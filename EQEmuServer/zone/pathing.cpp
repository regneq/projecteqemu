/*  EQEMu:  Everquest Server Emulator
Copyright (C) 2001-2002  EQEMu Development Team (http://eqemu.org)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
	  You should have received a copy of the GNU General Public License
	  along with this program; if not, write to the Free Software
	  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "../common/debug.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../common/files.h"
#include "../common/emu_opcodes.h"
#include "../common/eq_packet_structs.h"
#include "client.h"
#include "zone_profile.h"
#include "map.h"
#include "pathing.h"
#include "zone.h"
#include "../common/MiscFunctions.h"
#ifdef WIN32
#define snprintf	_snprintf
#endif

#ifndef WIN32
//comment this out if your worried about zone boot times and your not using valgrind
#define SLOW_AND_CRAPPY_MAKES_VALGRIND_HAPPY
#endif


/*

Fear guide points. these are not used directly by the fear pathing
code. They are used by `apathing` to act as hint points to 
provide more valid points for it to process on

CREATE TABLE fear_hints (
	id INT AUTO_INCREMENT PRIMARY KEY,
	zone VARCHAR(16) NOT NULL,
	x FLOAT NOT NULL,
	y FLOAT NOT NULL,
	z FLOAT NOT NULL,
	forced TINYINT NOT NULL DEFAULT 0,
	disjoint TINYINT NOT NULL DEFAULT 0,
	UNIQUE KEY(zone,x,y,z)
);

*/


extern Zone* zone;

//#define OPTIMIZE_FEAR_QT_LOOKUPS

//#define DEBUG_SEEK 1
//#define DEBUG_NEXT
//#define DEBUG_BEST_Z 1
#define DEBUG_PATHING

//quick functions to clean up vertex code.
#define Vmin3(o, a, b, c) ((a.o<b.o)? (a.o<c.o?a.o:c.o) : (b.o<c.o?b.o:c.o))
#define Vmax3(o, a, b, c) ((a.o>b.o)? (a.o>c.o?a.o:c.o) : (b.o>c.o?b.o:c.o))

PathManager* PathManager::LoadPathFile(const char* in_zonename) {
	FILE *fp = NULL;
	char zBuf[64];
	char cWork[256];
	PathManager* ret = 0;
	
	//have to convert to lower because the short names im getting
	//are not all lower anymore, copy since strlwr edits the str.
	strncpy(zBuf, in_zonename, 64);
	zBuf[63] = '\0';
	
	snprintf(cWork, 250, MAP_DIR "/%s.path", strlwr(zBuf));
	
	if ((fp = fopen( cWork, "rb" ))) {
		ret = new PathManager();
		if(ret->loadPaths(fp)) {
			printf("Path File %s loaded.\n", cWork);
		} else {
			printf("Path File %s loading failed.\n", cWork);
		}
		fclose(fp);
	}
	else {
		printf("Path File %s not found.\n", cWork);
	}
	return ret;
}

PathManager::PathManager() {
	
	m_Nodes = 0;
	m_Links = 0;
	nodes = NULL;
	links = NULL;
	m_QTNodes = 0;
	m_NodeLists = 0;
	QTNodes = NULL;
	nodelists = NULL;
	path_finding = NULL;
}

bool PathManager::loadPaths(FILE *fp) {
#ifndef INVERSEXY
#warning Path files do not work without inverted XY
	return(false);
#endif

	PathFile_Header head;
	if(fread(&head, sizeof(head), 1, fp) != 1) {
		//map read error.
		return(false);
	}
	if(head.version != PATHFILE_VERSION) {
		//invalid version... if there really are multiple versions,
		//a conversion routine could be possible.
		printf("Invalid path file version 0x%lx, we want 0x%lx\n", head.version, PATHFILE_VERSION);
		return(false);
	}
	
	printf("Path header: %lu nodes, %lu links, %u QT nodes, %lu nodelists\n", head.node_count, head.link_count, head.qtnode_count, head.nodelist_count);
	
	m_Nodes = head.node_count;
	m_Links = head.link_count;
	m_QTNodes = head.qtnode_count;
	m_NodeLists = head.nodelist_count;
	
	nodes = new PathNode_Struct[m_Nodes];
	links = new PathLink_Struct[m_Links];
	QTNodes = new PathTree_Struct[m_QTNodes];
	nodelists = new PathNodeRef[m_NodeLists];
	path_finding = new PathLinkOffsetRef[m_Nodes*m_Nodes];
	
	
	//this was changed to this loop from the single read because valgrind was
	//hanging on this read otherwise... I dont pretend to understand it.
#ifdef SLOW_AND_CRAPPY_MAKES_VALGRIND_HAPPY
	unsigned long r;
	for(r = 0; r < m_Nodes; r++) {
		if(fread(nodes+r, sizeof(PathNode_Struct), 1, fp) != 1) {
			printf("Unable to read %lu nodes from path file, got %lu.\n", m_Nodes, r);
			return(false);
		}
	}
#else
	unsigned long count;
	if((count=fread(nodes, sizeof(PathNode_Struct), m_Nodes , fp)) != m_Nodes) {
		printf("Unable to read %lu nodes from path file, got %lu.\n", m_Nodes, count);
		return(false);
	}
#endif
	
#ifdef SLOW_AND_CRAPPY_MAKES_VALGRIND_HAPPY
	for(r = 0; r < m_Links; r++) {
		if(fread(links+r, sizeof(PathLink_Struct), 1, fp) != 1) {
			printf("Unable to read %lu links from path file, got %lu.\n", m_Links, r);
			return(false);
		}
	}
#else
	if((count=fread(links, sizeof(PathLink_Struct), m_Links , fp)) != m_Links) {
		printf("Unable to read %lu links from path file, got %lu.\n", m_Links, count);
		return(false);
	}
#endif
	
#ifdef SLOW_AND_CRAPPY_MAKES_VALGRIND_HAPPY
	for(r = 0; r < m_QTNodes; r++) {
		if(fread(QTNodes+r, sizeof(PathTree_Struct), 1, fp) != 1) {
			printf("Unable to read %lu qt nodes from map file, got %lu.\n", m_QTNodes, r);
			return(false);
		}
	}
#else
	if((count=fread(QTNodes, sizeof(PathTree_Struct), m_QTNodes, fp)) != m_QTNodes) {
		printf("Unable to read %lu qt nodes from path file.\n", m_Nodes);
		return(false);
	}
#endif
	
#ifdef SLOW_AND_CRAPPY_MAKES_VALGRIND_HAPPY
	for(r = 0; r < m_NodeLists; r++) {
		if(fread(nodelists+r, sizeof(PathNodeRef), 1, fp) != 1) {
			printf("Unable to read %lu node lists from path file, got %lu.\n", m_NodeLists, r);
			return(false);
		}
	}
#else
	if((count=fread(nodelists, sizeof(PathNodeRef), m_NodeLists, fp)) != m_NodeLists) {
		printf("Unable to read %lu node lists from path file. Got %lu.\n", m_NodeLists, count);
		return(false);
	}
#endif
	
#ifdef SLOW_AND_CRAPPY_MAKES_VALGRIND_HAPPY
	int nodes2 = m_Nodes*m_Nodes;
	for(r = 0; r < nodes2; r++) {
		if(fread(path_finding+r, sizeof(PathLinkOffsetRef), 1, fp) != 1) {
			printf("Unable to read %lu faces from map file, got %lu.\n", nodes2, r);
			return(false);
		}
	}
#else
	if((count=fread(path_finding, sizeof(PathLinkOffsetRef), m_Nodes*m_Nodes, fp)) != m_Nodes*m_Nodes) {
		printf("Unable to read %lu path finding matrix from path file. Got %lu.\n", m_Nodes*m_Nodes, count);
		return(false);
	}
#endif
	
	return(true);
}

PathManager::~PathManager() {
	safe_delete_array(nodes);
	safe_delete_array(links);
	safe_delete_array(QTNodes);
	safe_delete_array(nodelists);
	safe_delete_array(path_finding);
}


PathNodeRef PathManager::SeekNode( PathNodeRef node_r, float x, float y ) {
	if(node_r == PATH_NODE_NONE || node_r >= m_QTNodes) {
		return(PATH_NODE_NONE);
	}
	PFPNODE _node = &QTNodes[node_r];
#ifdef DEBUG_SEEK
printf("Seeking node for %u:(%.2f, %.2f) with root 0x%x.\n", node_r, x, y, _node);

printf("	Current Box: (%.2f -> %.2f, %.2f -> %.2f)\n", _node->minx, _node->maxx, _node->miny, _node->maxy);
#endif
	if( x>= _node->minx && x<= _node->maxx && y>= _node->miny && y<= _node->maxy ) {
		if( _node->flags & pathNodeFinal ) {
#ifdef DEBUG_SEEK
printf("Seeking node for %u:(%.2f, %.2f) with root 0x%x.\n", node_r, x, y, _node);
printf("	Final Node: (%.2f -> %.2f, %.2f -> %.2f)\n", _node->minx, _node->maxx, _node->miny, _node->maxy);
fflush(stdout);
printf("	Final node found with %d fear nodes.\n", _node->nodelist.count);
/*printf("	Faces:\n");
unsigned long *cfl = mFaceLists + _node->faces.offset;
unsigned long m;
for(m = 0; m < _node->faces.count; m++) {
	FACE *c = &mFinalFaces[ *cfl ];
	printf("	%lu (%.2f, %.2f, %.2f) (%.2f, %.2f, %.2f) (%.2f, %.2f, %.2f)\n",
				*cfl, c->a.x, c->a.y, c->a.z,
				c->b.x, c->b.y, c->b.z, 
				c->c.x, c->c.y, c->c.z);
	cfl++;
}*/
#endif
			return node_r;
		}
#ifdef DEBUG_SEEK
printf("	Kids: %u, %u, %u, %u\n", _node->nodes[0], _node->nodes[1], _node->nodes[2], _node->nodes[3]);
		
printf("	Contained In Box: (%.2f -> %.2f, %.2f -> %.2f)\n", _node->minx, _node->maxx, _node->miny, _node->maxy);
		
/*printf("	Node found has children.\n");
if(_node->node1 != NULL) {
	printf("\tNode: (%.2f -> %.2f, %.2f -> %.2f)\n", 
		_node->node1->minx, _node->node1->maxx, _node->node1->miny, _node->node1->maxy);
}
if(_node->node2 != NULL) {
	printf("\tNode: (%.2f -> %.2f, %.2f -> %.2f)\n", 
		_node->node2->minx, _node->node2->maxx, _node->node2->miny, _node->node2->maxy);
}
if(_node->node3 != NULL) {
	printf("\tNode: (%.2f -> %.2f, %.2f -> %.2f)\n", 
		_node->node3->minx, _node->node3->maxx, _node->node3->miny, _node->node3->maxy);
}
if(_node->node4 != NULL) {
	printf("\tNode: (%.2f -> %.2f, %.2f -> %.2f)\n", 
		_node->node4->minx, _node->node4->maxx, _node->node4->miny, _node->node4->maxy);
}*/
#endif
		//NOTE: could precalc these and store them in node headers

		PathNodeRef tmp = PATH_NODE_NONE;
#ifdef OPTIMIZE_FEAR_QT_LOOKUPS
		float midx = _node->minx + (_node->maxx - _node->minx) * 0.5;
		float midy = _node->miny + (_node->maxy - _node->miny) * 0.5;
		//follow ordering rules from map.h...
		if(x < midx) {
			if(y < midy) { //quad 3
				if(_node->nodes[2] != PATH_NODE_NONE)
					tmp = SeekNode( _node->nodes[2], x, y );
			} else {	//quad 2
				if(_node->nodes[2] != PATH_NODE_NONE)
					tmp = SeekNode( _node->nodes[1], x, y );
			}
		} else {
			if(y < midy) {  //quad 4
				if(_node->nodes[2] != PATH_NODE_NONE)
					tmp = SeekNode( _node->nodes[3], x, y );
			} else {	//quad 1
				if(_node->nodes[2] != PATH_NODE_NONE)
					tmp = SeekNode( _node->nodes[0], x, y );
			}
		}
		if( tmp != PATH_NODE_NONE ) return tmp;
#else
		tmp = SeekNode( _node->nodes[0], x, y );
		if( tmp != PATH_NODE_NONE ) return tmp;
		tmp = SeekNode( _node->nodes[1], x, y );
		if( tmp != PATH_NODE_NONE ) return tmp;
		tmp = SeekNode( _node->nodes[2], x, y );
		if( tmp != PATH_NODE_NONE ) return tmp;
		tmp = SeekNode( _node->nodes[3], x, y );
		if( tmp != PATH_NODE_NONE ) return tmp;
#endif

	}
#ifdef DEBUG_SEEK
printf(" No node found.\n");
#endif
	return(PATH_NODE_NONE);
}

//this looks for our QT node, if it cannot find our, it
//will find one close to us
PathNodeRef PathManager::SeekNodeGuarantee( PathNodeRef node_r, float x, float y ) {
	if(node_r == PATH_NODE_NONE || node_r >= m_QTNodes) {
		return(PATH_NODE_NONE);
	}
	
	PFPNODE _node = &QTNodes[node_r];
	if( x>= _node->minx && x<= _node->maxx && y>= _node->miny && y<= _node->maxy ) {
		if( _node->flags & pathNodeFinal ) {
			return node_r;
		}
		//NOTE: could precalc these and store them in node headers

		PathNodeRef tmp = PATH_NODE_NONE;
#ifdef OPTIMIZE_FEAR_QT_LOOKUPS
		float midx = _node->minx + (_node->maxx - _node->minx) * 0.5;
		float midy = _node->miny + (_node->maxy - _node->miny) * 0.5;
		//follow ordering rules from map.h...
		if(x < midx) {
			if(y < midy) { //quad 3
				if(_node->nodes[2] != PATH_NODE_NONE)
					tmp = SeekNode( _node->nodes[2], x, y );
			} else {	//quad 2
				if(_node->nodes[2] != PATH_NODE_NONE)
					tmp = SeekNode( _node->nodes[1], x, y );
			}
		} else {
			if(y < midy) {  //quad 4
				if(_node->nodes[2] != PATH_NODE_NONE)
					tmp = SeekNode( _node->nodes[3], x, y );
			} else {	//quad 1
				if(_node->nodes[2] != PATH_NODE_NONE)
					tmp = SeekNode( _node->nodes[0], x, y );
			}
		}
		if( tmp != PATH_NODE_NONE ) return tmp;
#else
		tmp = SeekNode( _node->nodes[0], x, y );
		if( tmp != PATH_NODE_NONE ) return tmp;
		tmp = SeekNode( _node->nodes[1], x, y );
		if( tmp != PATH_NODE_NONE ) return tmp;
		tmp = SeekNode( _node->nodes[2], x, y );
		if( tmp != PATH_NODE_NONE ) return tmp;
		tmp = SeekNode( _node->nodes[3], x, y );
		if( tmp != PATH_NODE_NONE ) return tmp;
#endif
		
		//if we get here we didnt find the node in our children,
		//so use ourself as the node.
		return(node_r);
	}
#ifdef DEBUG_SEEK
printf(" No node found.\n");
#endif
	return(PATH_NODE_NONE);
}

/*

A quadtree is not the ideal structure for finding the closest
node, because if you are close to a boundary, you will not search
nodes which are just over the boundary, even if they are much closer.

Also, the sparse nature of the grid points might lead to several gaps
in the quadtree which contain no nodes

to help fix this, I modified the quadtree to store a node list
at each level, this allows us to get a node list even if our current
node dosent exist/is empty.

I also dont just store nodes WITHIN this QT node, but also nodes
within a defined range, so if you stay under that range, you will always
find the true closest node. (FEAR_MAXIMUM_DISTANCE)

This also means that if we cannot find our QT node, that there are no
nodes within FEAR_MAXIMUM_DISTANCE from the mob, so you might wanna 
just call it quits.

*/

bool PathManager::FindNearestFear(MobFearState *state, float x, float y, float z, bool check_los) {
	if(state == NULL)
		return(false);

	PathPointRef bestref;
	
	bestref = FindNearestNode(x, y, z, check_los?ForceLOS:EncourageLOS);
	if(bestref == PATH_NODE_NONE)
		return(false);
	
	PathNode_Struct *bestn = GetNode(bestref);
	//could prolly assume this is not null..
	if(bestn == NULL)
		return(false);
	
	//start with running to bottom since it will stay
	//localized longer, once they get to top, they really only
	//run along one path from top to bottom.
	state->state = MobFearState::runToBottom;
	state->goal_node = bestref;
	state->last_link = PATH_LINK_NONE;
	
	state->x = bestn->x;
	state->y = bestn->y;
	state->z = bestn->z;
	
#ifdef DEBUG_SEEK
printf("Found fear node: (%.3f,%.3f,%.3f)\n", bestn->x, bestn->y, bestn->z);
#endif
	
	return(true);
}

//state is an input parameter here so I have have to take a ton of args
bool PathManager::InitPathFinding(PathFindingState *state, bool force_it) {
	if(state == NULL)
		return(false);
	if(zone->map == NULL)
		return(false);
	
	//first try the obvious choice which is a straight path
	//see if we have line of sight
	VERTEX start, end, hit;
	start.x = state->x;
	start.y = state->y;
	start.z = state->z + 5.0;
	end.x = state->dest_x;
	end.y = state->dest_y;
	end.z = state->dest_z + 5.0;
	
	if(!zone->map->LineIntersectsZone(start, end, 0.5, &hit, NULL)) {
		//we have LOS, no path finding needed.
		state->x = state->dest_x;
		state->y = state->dest_y;
		state->z = state->dest_z;
		return(true);
	}
	
	
	PathPointRef bestref, bestdest;
	
	bestref = FindNearestNode(state->x, state->y, state->z, force_it?EncourageLOS:ForceLOS);
	if(bestref == PATH_NODE_NONE)
		return(false);
	
	bestdest = FindNearestNode(state->dest_x, state->dest_y, state->dest_z, force_it?EncourageLOS:ForceLOS);
	if(bestdest == PATH_NODE_NONE)
		return(false);
	
	PathNode_Struct *bestn = GetNode(bestref);
	//could prolly assume this is not null..
	if(bestn == NULL)
		return(false);
	
	//we found both nodes, we should be able to path this.
	//set up our state
	state->goal_node = bestref;
	state->dest_node = bestdest;
	
	state->x = bestn->x;
	state->y = bestn->y;
	state->z = bestn->z;
	
#ifdef DEBUG_PATHING
	LogFile->write(EQEMuLog::Debug, "Init Pathing: start=%d, dest=%d", state->goal_node, state->dest_node);
#endif
	
	return(true);
}

bool PathManager::NextPathFinding(PathFindingState *state) {
	if(state == NULL)
		return(false);
	
	//assume we have reached goal_node
	
	//see if were completely done
	if(state->Finished()) {
#ifdef DEBUG_PATHING
	LogFile->write(EQEMuLog::Debug, "Next Pathing: completely done");
#endif
		return(false);
	}
	
	//see if we have reached our last node, just move to dest now.
	if(state->goal_node == state->dest_node) {
#ifdef DEBUG_PATHING
	LogFile->write(EQEMuLog::Debug, "Next Pathing: last node at %d", state->goal_node);
#endif
		state->x = state->dest_x;
		state->y = state->dest_y;
		state->z = state->dest_z;
		return(true);
	}
	
	//we need to find our next node now...
	PathNode_Struct *gnode = GetNode(state->goal_node);
	if(gnode == NULL)
		return(false);
	
	//ask the pathing info for the next link to take to get
	//to our destination node from our current node.
	
	//get our array of links to take to reach all nodes
	PathLinkOffsetRef *offsets = GetPathFinding(state->goal_node);
	if(offsets == NULL)
		return(false);
	
	//get the link number to reach dest_node from our link array
	PathLinkOffsetRef off = offsets[state->dest_node];
	
	if(off == PATH_LINK_OFFSET_NONE) {
		//this means the node is unreachable from here.. should print error
		return(false);
	}
	
	//get our current array of links
	PathLink_Struct *links = GetLinks(gnode->link_offset);
	if(links == NULL)
		return(false);
	
	//get the link that the path finding code tells us for this node
	PathLink_Struct *link = links + off;
	
	if(link->dest_node == PATH_NODE_NONE)
		return(false);
	
	PathNode_Struct *next_node = GetNode(link->dest_node);
	if(next_node == NULL)
		return(false);
	
	if(state->goal_node == link->dest_node) {
		//..um.. WTF... why???
#ifdef DEBUG_PATHING
		LogFile->write(EQEMuLog::Debug, "Cycle Detected at %d (%.3f,%.3f,%.3f) going to %d", link->dest_node, next_node->x, next_node->y, next_node->z, state->dest_node);
#endif
		state->x = state->dest_x;
		state->y = state->dest_y;
		state->z = state->dest_z;
		return(true);
	}
	
#ifdef DEBUG_PATHING
	LogFile->write(EQEMuLog::Debug, "Next Pathing: to %d (%.3f,%.3f,%.3f) via link %d", link->dest_node, next_node->x, next_node->y, next_node->z, off);
#endif
	
	//set our next state.
	state->goal_node = link->dest_node;
	state->x = next_node->x;
	state->y = next_node->y;
	state->z = next_node->z;
	
	return(true);
}

PathPointRef PathManager::FindNearestNode(float x, float y, float z, losp los) {
	if(zone->map == NULL)
		return(PATH_NODE_NONE);
	
	PathNodeRef qtnodeR;
	qtnodeR = SeekNodeGuarantee(GetRoot(), x, y);
	if(qtnodeR == PATH_NODE_NONE) {
		//we have no node from garuntee... this is bad
		return(PATH_NODE_NONE);
	}
	
	//get our real node
	PathTree_Struct *qtnode = GetQTNode(qtnodeR);
	
	//grab our node list, which is qtnode->nodelist.count long
	PathPointRef *nlist = GetNodeList(qtnode);
	
#ifdef DEBUG_SEEK
printf("QT Node starts at %d and has %d points.\n", qtnode->nodelist.offset, qtnode->nodelist.count);
#endif
	PathNode_Struct *curn;
	PathPointRef bestref = PATH_NODE_NONE, bestlref = PATH_NODE_NONE;
	float cur_dist, best_dist2, bestl_dist2;
	VERTEX p1, p2, liz_res;
	int32 r;
	
	p1.x = x;
	p1.y = y;
	p1.z = z + 6.0;
	
	//loop through each node in the list, and find the closest we can see
	best_dist2 = 9999999e111;
	bestl_dist2 = 9999999e111;
	for(r = 0; r < qtnode->nodelist.count; r++, nlist++) {
		curn = GetNode(*nlist);
		if(curn == NULL)
			continue;
#ifdef DEBUG_SEEK
printf("\nTrying node %d at (%.3f,%.3f,%.3f) ", *nlist, curn->x, curn->y, curn->z);
#endif
		
		//make sure its the closest
		cur_dist = Dist2(x, y, z, curn->x, curn->y, curn->z);
		if(cur_dist >= best_dist2) {
			if(los == ForceLOS || cur_dist >= bestl_dist2)
				continue;
		}
#ifdef DEBUG_SEEK
printf("Maybe..(%.3f<%.3f)", cur_dist, best_dist2);
#endif
		
		//make sure we can see it...
		p2.x = curn->x;
		p2.y = curn->y;
		p2.z = curn->z + 6.0;
		if(los != IgnoreLOS && zone->map->LineIntersectsZone(p1, p2, 0.5, &liz_res)) {
			//we are not ignoring los, and we dont have los.
			if(los == EncourageLOS && cur_dist < bestl_dist2) {
				//we might have to use the closest nos LOS node, and
				//this is a new best.
				bestl_dist2 = cur_dist;
				bestlref = *nlist;
			}
			continue;
		}
#ifdef DEBUG_SEEK
printf("New Best");
#endif
		
		best_dist2 = cur_dist;
		bestref = *nlist;
	}
#ifdef DEBUG_SEEK
printf("\nBest node: %d at dist2 %f\n", bestref, best_dist2);
#endif
	if(bestref == PATH_NODE_NONE && los == EncourageLOS) {
		//we have failed to locate any LOS nodes, and the caller
		//specified that it would take a non-LOS node if this happened
		bestref = bestlref;
	}
	
	return(bestref);
}

bool PathManager::NextFearPath(MobFearState *state) {
#ifdef DEBUG_NEXT
	printf("Finding next node from node %d, via link %d. (%.3f,%.3f,%.3f)\n", state->goal_node, state->last_link, state->x, state->y, state->z);
#endif
	if(!state->IsValidState()) {
		return(false);
	} else if(state->state == MobFearState::runToBottom) {
		//assume we have reached our goal node, find our next goal
		
		//the goal of running to bottom is to increase our distance
		//from the root node, while taking the path which has the
		//greatest reach
#ifdef DEBUG_NEXT
	printf("Starting 'To Bottom' search\n");
#endif
		
		PathNode_Struct *gnode = GetNode(state->goal_node);
		if(gnode == NULL)
			return(false);
		
		PathLink_Struct *links = GetLinks(gnode->link_offset);
		int r;
		
		PathNode_Struct *look_node;
		PathNodeRef longest = 0;
		PathPointRef long_node = PATH_NODE_NONE;
		PathLinkRef long_link = PATH_LINK_NONE;
		for(r = 0; r < gnode->link_count; r++, links++) {
			if(state->last_link == (gnode->link_offset + r))
				continue;		//never traverse the same link we just came from.
			//make sure its further away than where we are
			look_node = GetNode(links->dest_node);
			if(look_node == NULL)
				continue;		//this shouldent happen.
#ifdef DEBUG_NEXT
			printf("  Checking... d=%d, old_d=%d, best=%d, cur=%d.\n", look_node->distance, gnode->distance, longest, links->reach);
#endif
			//if this node is closer than our current node, skip it
			if(look_node->distance <= gnode->distance)
				continue;	//only looking for further nodes.
			
			if(links->reach > longest) {
				longest = links->reach;
				long_node = links->dest_node;
				long_link = gnode->link_offset + r;
			} else if(links->reach == longest) {
				//try to provide some variance at junction nodes
				//same length longest, give it a 50% chance
				if(MakeRandomInt(0, 99) >= 50)
					continue;
				//it passed, crown this as the new longest
				longest = links->reach;
				long_node = links->dest_node;
				long_link = gnode->link_offset + r;
			} else if(links->reach > (longest - 5)) {
				//try to provide some variance at junction nodes
				//close to the longest, give it a chance based on closeness
				if(MakeRandomInt(0, 99) >= 10*(longest-links->reach))
					continue;
				//it passed, crown this as the new longest
				longest = links->reach;
				long_node = links->dest_node;
				long_link = gnode->link_offset + r;
				
			}
		}
		
		//this means we skipped all links cause they were closer to root
		if(long_node == PATH_NODE_NONE) {
			//we reached the top, start heading back down...
			state->state = MobFearState::runToTop;
			//call ourselves again to get a real node...
			//I dont THINK infinite recursion is posible... lets hope not (:
			//a node should never be top or bottom unless its alont
			if(m_Links < 2)
				return(false);	//watch for empty grids
			return(NextFearPath(state));
		}
		
		PathNode_Struct *bestn = NULL;
		bestn = GetNode(long_node);
		if(bestn == NULL)
			return(false);
		
		//not a criteria for state change anymore
/*		if(state->last_link != PATH_LINK_NONE) {
			//Get the link
			PathLink_Struct *last_link = GetLinks(state->last_link);
#ifdef DEBUG_NEXT
	printf("  Last link reach was %d, new link is %d\n", last_link->reach, longest);
#endif
		}*/
		
		
		//only go to the next node if we did not change state.
		//if we changed, let them pick next time we get called.
		if(state->state == MobFearState::runToBottom) {
			state->last_link = long_link;
			state->goal_node = long_node;
		
			state->x = bestn->x;
			state->y = bestn->y;
			state->z = bestn->z;

#ifdef DEBUG_NEXT
		printf("To Bottom picked %d, via link %d. (%.3f,%.3f,%.3f)\n", state->goal_node, state->last_link, state->x, state->y, state->z);
#endif
		}
#ifdef DEBUG_NEXT
else {
			printf("To Top changed state.\n");
}
#endif
		
		return(true);
	} else if(state->state == MobFearState::runToTop) {
		//assume we have reached our goal node, find our next goal
		
		//our goal is to minimize our distance. The root of the tree
		//can reach a minimum number of nodes down each branch
		//beacuse it divides them equally on each branch.
		
#ifdef DEBUG_NEXT
	printf("Starting 'To Top' search\n");
#endif		
		PathNode_Struct *gnode = GetNode(state->goal_node);
		if(gnode == NULL)
			return(false);
		
		PathLink_Struct *links = GetLinks(gnode->link_offset);
		int r;
		
		PathNode_Struct *look_node;
		PathNodeRef shortest = 0xFFFF;
		PathPointRef short_node = PATH_NODE_NONE;
		PathLinkRef short_link = PATH_LINK_NONE;
		for(r = 0; r < gnode->link_count; r++, links++) {
			if(state->last_link == (gnode->link_offset + r))
				continue;		//never traverse the same link we just came from.
			//make sure its further away than where we are
			look_node = GetNode(links->dest_node);
			if(look_node == NULL)
				continue;	//this shouldent happen...
#ifdef DEBUG_NEXT
			printf("  Checking...best=%d, cur=%d.\n", shortest, look_node->distance);
#endif
			
			if(look_node->distance < shortest) {
				shortest = look_node->distance;
				short_node = links->dest_node;
				short_link = gnode->link_offset + r;
			}
			//do we want to add variance for going up? seems like we might
			//end up screwing up our ascent
			/* else if(links->distance == shortest) {
				//try to provide some variance at junction nodes
				//same length shortest, give it a 50% chance
				if(MakeRandomInt(0, 99) >= 50)
					continue;
				//it passed, crown this as the new shortest
				shortest = links->distance;
				short_node = links->dest_node;
				short_link = gnode->link_offset + r;
			}*/
		}
		if(short_node == PATH_NODE_NONE)
			return(false);
		
		PathNode_Struct *bestn = NULL;
		bestn = GetNode(short_node);
		if(bestn == NULL)
			return(false);
		
/*		if(state->last_link != PATH_LINK_NONE) {
			//Get the link
			PathLink_Struct *last_link = GetLinks(state->last_link);
#ifdef DEBUG_NEXT
	printf("  Last link shortest was %d, new link is %d\n", last_link->distance, shortest);
#endif
			if(shortest > last_link->distance) {
				//we reached the top, start heading back down...
				state->state = MobFearState::runToBottom;
			}
		}*/
#ifdef DEBUG_NEXT
	printf("  Last link shortest was %d, new link is %d\n", gnode->distance, shortest);
#endif
		if(shortest > gnode->distance) {
			//we reached the top, start heading back down...
			//call ourselves again to get a real node...
			state->state = MobFearState::runToBottom;
			if(m_Links < 2)
				return(false);	//watch for empty grids
			return(NextFearPath(state));
		}
		
		//we reached the top, start heading back down...
		if(shortest == 1) {
			//call ourselves again to get a real node...
			state->state = MobFearState::runToBottom;
			if(m_Links < 2)
				return(false);	//watch for empty grids
			return(NextFearPath(state));
		}
		
		//only go to the next node if we did not change state.
		//if we changed, let them pick next time we get called.
		if(state->state == MobFearState::runToTop) {
		
			state->last_link = short_link;
			state->goal_node = short_node;
		
			state->x = bestn->x;
			state->y = bestn->y;
			state->z = bestn->z;

#ifdef DEBUG_NEXT
			printf("To Top picked %d, via link %d. (%.3f,%.3f,%.3f)\n", state->goal_node, state->last_link, state->x, state->y, state->z);
#endif
		}
#ifdef DEBUG_NEXT
else {
			printf("To Top changed state.\n");
}
#endif
		
		return(true);
	}
	
	return(false);
}



//this assumes that the first point in the list is the player's
//current position, I dont know how well it works if its not.
void Client::SendPathPacket(vector<FindPerson_Point> &points) {
	if(points.size() < 2) {
		//empty length packet == not found.
		EQApplicationPacket outapp(OP_FindPersonReply, 0);
		QueuePacket(&outapp);
		return;
	}
	
	printf("Sending a path packet with %d nodes.\n", points.size());
	
	int len = sizeof(FindPersonResult_Struct) + (points.size()+1) * sizeof(FindPerson_Point);
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_FindPersonReply, len);
	FindPersonResult_Struct* fpr=(FindPersonResult_Struct*)outapp->pBuffer;
	
printf("%d*%d + %d = %d\n", points.size(), sizeof(FindPerson_Point), sizeof(FindPersonResult_Struct), len);
	vector<FindPerson_Point>::iterator cur, end;
	cur = points.begin();
	end = points.end();
	int r;
	for(r = 0; cur != end; cur++, r++) {
		fpr->path[r] = *cur;
printf("Point %d: (%f,%f,%f)\n", r, (*cur).x, (*cur).y, (*cur).z);
	}
	//put the last element into the destination field
	cur--;
	fpr->path[r] = *cur;
	fpr->dest = *cur;
//	memcpy(&fpr->dest, &fpr->path[r-1], sizeof(fpr->dest));
printf("Terminal Point %d: (%f,%f,%f)\n", r, fpr->dest.x, fpr->dest.y, fpr->dest.z);
	
	float *fa = (float *) outapp->pBuffer;
	for(r = 0; r < outapp->size/4; r++, fa++) {
		if(r%3 == 0)
			printf("\n");
		printf("%f ", *fa);
	}
	
	FastQueuePacket(&outapp);
	
	
}








