#ifndef PATHING_H
#define PATHING_H

#include "../common/types.h"
#include <stdio.h>


//this defines the version of path files that we read
#define PATHFILE_VERSION 0x02000000

//named types cause the emu is never supposed to know/care what they are
//grow this type if you wanna support more than 64k path nodes
//this will affect path files, so change their version. (aka dont change this)
typedef uint16 PathNodeRef;		//this is a quad tree node
typedef uint16 PathPointRef;	//this is really a path node ref
typedef uint32 PathLinkRef;
typedef uint8  PathLinkOffsetRef;	//offset into a node's link list

//special values for PathNodeRefs
#define PATH_LINK_NONE 0xFFFFFFFF
#define PATH_NODE_NONE 65534		//applies to nodes and points
#define PATH_ROOT_NODE 0
#define PATH_LINK_OFFSET_NONE 0xFF

/*

File format:
	PathFile_Header		head;
	PathNode_Struct		nodes[head.node_count];
	PathLink_Struct		links[head.link_count*2];
	PathTree_Struct		quadtree[head.qtnode_count];
	//we could prolly shrink this node list into uint16
	PathPointRef		nodelist[head.nodelist_count];
	PathLinkOffsetRef	path_finding[head.node_count*head.node_count];
*/

#pragma pack(1)

struct PathFile_Header {
	uint32 version;
	uint32 node_count;
	uint32 link_count;
	uint16 qtnode_count;
	uint32 nodelist_count;
};

//types are used here so each element is the right size
struct PathNode_Struct {
	float				x;
	float				y;
	float				z;
	PathLinkRef			link_offset;		//offset into the links array for our data
	PathLinkOffsetRef	link_count;			//how many links we have
	PathNodeRef			distance;			//distance from the tree's root		
};

struct PathLink_Struct {
	PathPointRef 	dest_node;		//what node is reachable from this link
	PathNodeRef		reach;			//# of nodes reachable after traversing this link
};


enum {  //node flags
	pathNodeFinal = 0x01
	//7 more bits if theres something to use them for...
};

typedef struct _PathTree_Struct {
	//bounding box of this node
	//there is no reason that these could not be unsigned
	//shorts other than we have to compare them to floats
	//all over the place, so they stay floats for now.
	float minx;
	float miny;
	float maxx;
	float maxy;
	
	unsigned char flags;
	//un-unioned because all nodes store a list
//	union {
		PathNodeRef nodes[4];	//index 0 means NULL, not root
		struct {
			unsigned long count;
			unsigned long offset;
		} nodelist;
//	};
} PathTree_Struct, FPNODE, *PFPNODE;

#pragma pack()



class PathManager;

class MobFearState {
	friend class PathManager;
public:
	MobFearState() {
		goal_node = PATH_NODE_NONE;
		last_link = PATH_LINK_NONE;
		state = stuckSomewhere;
	}
	
	//mob should move to these coordinates and then call NextFearPath
	float x;
	float y;
	float z;
	
	inline bool IsValidState() { return(state != stuckSomewhere && goal_node != PATH_NODE_NONE); }
	
protected:
	//stuff that only the fear manager needs to know about
	PathPointRef goal_node;
	PathLinkRef  last_link;
	
	enum {
		stuckSomewhere,
		runToTop,		//trying to run to the top of the tree
		runToBottom		//we have reached a peak, descend to a leaf
	} state;
};

class PathFindingState {
	friend class PathManager;
public:
	PathFindingState() {
		goal_node = PATH_NODE_NONE;
		dest_node = PATH_NODE_NONE;
	}
	
	//mob should move to these coordinates and then call NextPathFinding
	//this is the current mob position on call to InitPathFinding
	float x;
	float y;
	float z;
	
	//final desitnation, which should be reachable from dest_node
	float dest_x;
	float dest_y;
	float dest_z;
	
//	inline bool Finished() { return(x == dest_x && y == dest_y && z == dest_z); }
	inline bool Finished() { float tmp1, tmp2; tmp1=x-dest_x; tmp2=y-dest_y; return(tmp1*tmp1+tmp2*tmp2 < 1); }
	
protected:
	//stuff that only the path manager needs to know about
	PathPointRef goal_node;	//our short term goal
	PathPointRef dest_node;	//our long term goal
};



class PathManager {
public:
	static PathManager* LoadPathFile(const char* in_zonename);
	PathManager();
	~PathManager();
	
	bool loadPaths(FILE *fp);
	
	//the result is always final, except special PATH_NODE_NONE
	PathNodeRef SeekNode( PathNodeRef _node, float x, float y );
	PathNodeRef SeekNodeGuarantee( PathNodeRef _node, float x, float y );
	
	//find the node closest to location, optionally enforcing LOS
	typedef enum { IgnoreLOS, ForceLOS, EncourageLOS } losp;
	PathPointRef FindNearestNode(float x, float y, float z, losp los = ForceLOS);
	
	//Fear pathing methods:
	bool FindNearestFear(MobFearState *state, float x, float y, float z, bool check_los = true);
	bool NextFearPath(MobFearState *state);
	
	//Path Finding methods:
	bool InitPathFinding(PathFindingState *state, bool force_it = false);
	bool NextPathFinding(PathFindingState *state);
	
	
	inline PathNodeRef		GetRoot( ) { return PATH_ROOT_NODE; }
	
	inline uint32 CountNodes() { return(m_Nodes); }
	inline PathNode_Struct *GetNode(PathPointRef r) { return(r<m_Nodes?nodes+r:NULL); }
	
private:
	//path data
	uint32 m_Nodes;
	uint32 m_Links;
	PathNode_Struct *nodes;
	PathLink_Struct *links;
	PathLinkOffsetRef *path_finding;
	
	//quadtree data
	uint32 m_QTNodes;
	uint32 m_NodeLists;
	PathTree_Struct *QTNodes;
	PathPointRef *nodelists;
	
	//quick access to abstract away how we access these things
	inline PathTree_Struct *GetQTNode( PathNodeRef r ) { return( QTNodes + r ); }
	inline PathPointRef *GetNodeList(PathTree_Struct *qt) {
		return(qt->nodelist.offset>=m_NodeLists?NULL : nodelists + qt->nodelist.offset); }
	inline PathLink_Struct *GetLinks(PathLinkRef r) { return(r<m_Links?links+r:NULL); }
	inline PathLinkOffsetRef *GetPathFinding(PathNodeRef r) { return(path_finding + r*m_Nodes); }
	
	static inline float Dist2(float x1, float y1, float z1, float x2, float y2, float z2) {
		float tmp;
		float sum;
		tmp = x1 - x2;
		sum = tmp*tmp;
		tmp = y1 - y2;
		sum += tmp*tmp;
		tmp = z1 - z2;
		sum += tmp*tmp;
		return(sum);
	}
};


#endif

