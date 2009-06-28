#include "../common/debug.h"
#include <string.h>
#include <math.h>
#include <list>
#include <algorithm>
#include <sstream>
#include "pathing.h"
#include "../common/files.h"
#include "../common/MiscFunctions.h"
#include "doors.h"
#include "client.h"
#include "zone.h"

#ifdef WIN32
#define snprintf        _snprintf
#endif

//#define PATHDEBUG
#define ABS(x) ((x)<0?-(x):(x))

extern Zone *zone;

float VertexDistance(VERTEX a, VERTEX b)
{
	_ZP(Pathing_VertexDistance);

	float xdist = a.x - b.x;
	float ydist = a.y - b.y;
	float zdist = a.z - b.z;
	return sqrtf(xdist * xdist + ydist * ydist + zdist * zdist);
}

float VertexDistanceNoRoot(VERTEX a, VERTEX b)
{
	_ZP(Pathing_VertexDistanceNoRoot);

	float xdist = a.x - b.x;
	float ydist = a.y - b.y;
	float zdist = a.z - b.z;
	return xdist * xdist + ydist * ydist + zdist * zdist;

}

PathManager* PathManager::LoadPathFile(const char* ZoneName)
{

	FILE *PathFile = NULL;

	char LowerCaseZoneName[64];

	char ZonePathFileName[256];

	PathManager* Ret = NULL;

	strn0cpy(LowerCaseZoneName, ZoneName, 64);
	
	strlwr(LowerCaseZoneName);

	snprintf(ZonePathFileName, 250, MAP_DIR "/%s.path", LowerCaseZoneName);

	if((PathFile = fopen(ZonePathFileName, "rb")))
	{
		Ret = new PathManager();

		if(Ret->loadPaths(PathFile))
		{
			LogFile->write(EQEMuLog::Status, "Path File %s loaded.", ZonePathFileName);

		}
		else
		{
			LogFile->write(EQEMuLog::Error, "Path File %s failed to load.", ZonePathFileName);
			safe_delete(Ret);
		}
		fclose(PathFile);
	}
	else
	{
		LogFile->write(EQEMuLog::Error, "Path File %s not found.", ZonePathFileName);
	}

	return Ret;
}

PathManager::PathManager()
{
	PathNodes = NULL;

	ClosedListFlag = NULL;
}

PathManager::~PathManager()
{

	safe_delete_array(PathNodes);

	safe_delete_array(ClosedListFlag);
}

bool PathManager::loadPaths(FILE *PathFile)
{

	char Magic[10];

	fread(&Magic, 9, 1, PathFile);

	if(strncmp(Magic, "EQEMUPATH", 9))
	{
		LogFile->write(EQEMuLog::Error, "Bad Magic String in .path file.");
		return false;
	}

	fread(&Head, sizeof(Head), 1, PathFile);

	LogFile->write(EQEMuLog::Status, "Path File Header: Version %ld, PathNodes %ld",
		       Head.version, Head.PathNodeCount);

	if(Head.version != 2)
	{
		LogFile->write(EQEMuLog::Error, "Unsupported path file version.");
		return false;
	}

	PathNodes = new PathNode[Head.PathNodeCount];
	
	fread(PathNodes, sizeof(PathNode), Head.PathNodeCount, PathFile);

	ClosedListFlag = new int[Head.PathNodeCount];

#ifdef PATHDEBUG
	PrintPathing();
#endif
	return true;
}

void PathManager::PrintPathing()
{

	for(int i = 0; i < Head.PathNodeCount; ++i)
	{
		printf("PathNode: %2d id %2d. (%8.3f, %8.3f, %8.3f), BestZ: %8.3f\n",
		       i, PathNodes[i].id, PathNodes[i].v.x, PathNodes[i].v.y, PathNodes[i].v.z, PathNodes[i].bestz);
		       

		if(PathNodes[i].Neighbours[0].id == -1)
		{
			printf("  NO NEIGHBOURS.\n");
			continue;
		}

		for(int j=0; j<PATHNODENEIGHBOURS; j++)
		{
			if(PathNodes[i].Neighbours[j].id == -1)
				break;

			printf("  Neighbour: %2d, Distance %8.3f", PathNodes[i].Neighbours[j].id,
			       PathNodes[i].Neighbours[j].distance);

			if(PathNodes[i].Neighbours[j].Teleport)
				printf("    ***** TELEPORT *****");

			if(PathNodes[i].Neighbours[j].DoorID >= 0)
				printf("    ***** via door %i *****", PathNodes[i].Neighbours[j].DoorID);

			printf("\n");
		}
	}
}

VERTEX PathManager::GetPathNodeCoordinates(int NodeNumber, bool BestZ)
{
	VERTEX Result;

	if(NodeNumber < Head.PathNodeCount)
	{
		Result = PathNodes[NodeNumber].v;

		if(!BestZ)
			return Result;

		Result.z = PathNodes[NodeNumber].bestz;
	}

	return Result;
	
}

list<int> PathManager::FindRoute(int startID, int endID)
{
	_ZP(Pathing_FindRoute_FromNodes);

	_log(PATHING__DEBUG, "FindRoute from node %i to %i", startID, endID);

	memset(ClosedListFlag, 0, sizeof(int) * Head.PathNodeCount);

	list<AStarNode> OpenList, ClosedList;

	list<int>Route;

	AStarNode AStarEntry, CurrentNode;

	AStarEntry.PathNodeID = startID;
	AStarEntry.Parent = -1;
	AStarEntry.HCost = 0;
	AStarEntry.GCost = 0;
	AStarEntry.Teleport = false;

	OpenList.push_back(AStarEntry);

	while(OpenList.size() > 0)
	{
		// The OpenList is maintained in sorted order, lowest to highest cost.

		CurrentNode = (*OpenList.begin());

		ClosedList.push_back(CurrentNode);
		
		ClosedListFlag[CurrentNode.PathNodeID] = true;

		OpenList.pop_front();

		for(int i = 0; i < PATHNODENEIGHBOURS; ++i)
		{
			if(PathNodes[CurrentNode.PathNodeID].Neighbours[i].id == -1)
				break;

			if(PathNodes[CurrentNode.PathNodeID].Neighbours[i].id == CurrentNode.Parent)
				continue;

			if(PathNodes[CurrentNode.PathNodeID].Neighbours[i].id == endID)
			{
				Route.push_back(CurrentNode.PathNodeID);

				Route.push_back(endID);
			
				list<AStarNode>::iterator RouteIterator;

				while(CurrentNode.PathNodeID != startID)
				{
					for(RouteIterator = ClosedList.begin(); RouteIterator != ClosedList.end(); ++RouteIterator)
					{
						if((*RouteIterator).PathNodeID == CurrentNode.Parent)
						{
							if(CurrentNode.Teleport)
								Route.insert(Route.begin(), -1);

							CurrentNode = (*RouteIterator);
							
							Route.insert(Route.begin(), CurrentNode.PathNodeID);

							break;
						}
					}
				}

				return Route;
			}
			if(ClosedListFlag[PathNodes[CurrentNode.PathNodeID].Neighbours[i].id])
				continue;

			AStarEntry.PathNodeID = PathNodes[CurrentNode.PathNodeID].Neighbours[i].id;

			AStarEntry.Parent = CurrentNode.PathNodeID;

			AStarEntry.Teleport = PathNodes[CurrentNode.PathNodeID].Neighbours[i].Teleport;

			// HCost is the estimated cost to get from this node to the end.
			AStarEntry.HCost = VertexDistance(PathNodes[PathNodes[CurrentNode.PathNodeID].Neighbours[i].id].v,
											PathNodes[endID].v);

			AStarEntry.GCost = CurrentNode.GCost + PathNodes[CurrentNode.PathNodeID].Neighbours[i].distance;

			float FCost = AStarEntry.HCost +  AStarEntry.GCost;
#ifdef PATHDEBUG	
			printf("Node: %i, Open Neighbour %i has HCost %8.3f, GCost %8.3f (Total Cost: %8.3f)\n",
					CurrentNode.PathNodeID, 
					PathNodes[CurrentNode.PathNodeID].Neighbours[i].id,
					AStarEntry.HCost,
					AStarEntry.GCost,
					AStarEntry.HCost + AStarEntry.GCost);
#endif

			bool AlreadyInOpenList = false;

			list<AStarNode>::iterator OpenListIterator, InsertionPoint = OpenList.end();

			for(OpenListIterator = OpenList.begin(); OpenListIterator != OpenList.end(); ++OpenListIterator)
			{
				if((*OpenListIterator).PathNodeID == PathNodes[CurrentNode.PathNodeID].Neighbours[i].id)
				{
					AlreadyInOpenList = true;

					float GCostToNode = CurrentNode.GCost +  PathNodes[CurrentNode.PathNodeID].Neighbours[i].distance;

					if(GCostToNode < (*OpenListIterator).GCost)
					{
						(*OpenListIterator).Parent = CurrentNode.PathNodeID;

						(*OpenListIterator).GCost = GCostToNode;

						(*OpenListIterator).Teleport = PathNodes[CurrentNode.PathNodeID].Neighbours[i].Teleport;
					}
					break;
				}
				else if((InsertionPoint == OpenList.end()) && (((*OpenListIterator).HCost + (*OpenListIterator).GCost) > FCost))
				{
					InsertionPoint = OpenListIterator;
				}
			}
			if(!AlreadyInOpenList)
				OpenList.insert(InsertionPoint, AStarEntry);
		}

	}
	_log(PATHING__DEBUG, "Unable to find a route.");
	return Route;

}

bool CheckLOSBetweenPoints(VERTEX start, VERTEX end) {

	VERTEX hit;
	FACE *face;

	if((zone->map) && (zone->map->LineIntersectsZone(start, end, 1, &hit, &face))) return false;

	return true;
}

bool SortPathNodesByDistance(PathNodeSortStruct n1, PathNodeSortStruct n2)
{
	return n1.Distance < n2.Distance;
}

list<int> PathManager::FindRoute(VERTEX Start, VERTEX End)
{

	_ZP(Pathing_FindRoute_FromVertices);

	_log(PATHING__DEBUG, "FindRoute(%8.3f, %8.3f, %8.3f,  %8.3f, %8.3f, %8.3f)", Start.x, Start.y, Start.z, End.x, End.y, End.z);

	list<int> noderoute;

	float CandidateNodeRangeXY = RuleR(Pathing, CandidateNodeRangeXY);

	float CandidateNodeRangeZ = RuleR(Pathing, CandidateNodeRangeZ);

	// Find the nearest PathNode the Start has LOS to.
	//
	//
	int ClosestPathNodeToStart = -1;

	list<PathNodeSortStruct> SortedByDistance;

	PathNodeSortStruct TempNode;

	for(int i = 0 ; i < Head.PathNodeCount; ++i)
	{
		if((ABS(Start.x - PathNodes[i].v.x) <= CandidateNodeRangeXY) &&
		   (ABS(Start.y - PathNodes[i].v.y) <= CandidateNodeRangeXY) &&
		   (ABS(Start.z - PathNodes[i].v.z) <= CandidateNodeRangeZ))
		{
		   	TempNode.id = i;
			TempNode.Distance = VertexDistanceNoRoot(Start, PathNodes[i].v);
			SortedByDistance.push_back(TempNode);

		}
	}
	
	SortedByDistance.sort(SortPathNodesByDistance);

	for(list<PathNodeSortStruct>::iterator Iterator = SortedByDistance.begin(); Iterator != SortedByDistance.end(); ++Iterator)
	{
		_log(PATHING__DEBUG, "Checking Reachability of Node %i from Start Position.", PathNodes[(*Iterator).id].id);

		if(!zone->map->LineIntersectsZone(Start, PathNodes[(*Iterator).id].v, 1.0f, NULL, NULL))
		{
			ClosestPathNodeToStart = (*Iterator).id;
			break;
		}
	}

	if(ClosestPathNodeToStart <0 ) {
		_log(PATHING__DEBUG, "No LOS to any starting Path Node within range.");
		return noderoute;
	}

	_log(PATHING__DEBUG, "Closest Path Node To Start: %2d", ClosestPathNodeToStart);

	// Find the nearest PathNode the end point has LOS to

	int ClosestPathNodeToEnd = -1;

	SortedByDistance.clear();

	for(int i = 0 ; i < Head.PathNodeCount; ++i)
	{
		if((ABS(End.x - PathNodes[i].v.x) <= CandidateNodeRangeXY) &&
		   (ABS(End.y - PathNodes[i].v.y) <= CandidateNodeRangeXY) &&
		   (ABS(End.z - PathNodes[i].v.z) <= CandidateNodeRangeZ))
		{
		   	TempNode.id = i;
			TempNode.Distance = VertexDistanceNoRoot(End, PathNodes[i].v);
			SortedByDistance.push_back(TempNode);
		}
	}
	
	SortedByDistance.sort(SortPathNodesByDistance);

	for(list<PathNodeSortStruct>::iterator Iterator = SortedByDistance.begin(); Iterator != SortedByDistance.end(); ++Iterator)
	{
		_log(PATHING__DEBUG, "Checking Reachability of Node %i from End Position.", PathNodes[(*Iterator).id].id);
		_log(PATHING__DEBUG, " (%8.3f, %8.3f, %8.3f) to (%8.3f, %8.3f, %8.3f)",
			End.x, End.y, End.z,
			PathNodes[(*Iterator).id].v.x, PathNodes[(*Iterator).id].v.y, PathNodes[(*Iterator).id].v.z);

		if(!zone->map->LineIntersectsZone(End, PathNodes[(*Iterator).id].v, 1.0f, NULL, NULL))
		{
			ClosestPathNodeToEnd = (*Iterator).id;
			break;
		}
	}
	
	if(ClosestPathNodeToEnd < 0) {
		_log(PATHING__DEBUG, "No LOS to any end Path Node within range.");
		return noderoute;
	}

	_log(PATHING__DEBUG, "Closest Path Node To End: %2d", ClosestPathNodeToEnd);

	if(ClosestPathNodeToStart == ClosestPathNodeToEnd)
	{
		noderoute.push_back(ClosestPathNodeToStart);
		return noderoute;
	}
	noderoute = FindRoute(ClosestPathNodeToStart, ClosestPathNodeToEnd);

	int NodesToAttemptToCull = RuleI(Pathing, CullNodesFromStart);

	if(NodesToAttemptToCull > 0)
	{
		int CulledNodes = 0;
		
		list<int>::iterator First, Second;

		while((noderoute.size() >= 2) && (CulledNodes < NodesToAttemptToCull))
		{
			First = noderoute.begin();

			Second = First;

			++Second;

			if((*Second) < 0)
				break;

			if(!zone->map->LineIntersectsZone(Start, PathNodes[(*Second)].v, 1.0f, NULL, NULL))
			{
				noderoute.erase(First);

				++CulledNodes;
			}
			else
				break;
		}
	}
				
	NodesToAttemptToCull = RuleI(Pathing, CullNodesFromEnd);

	if(NodesToAttemptToCull > 0)
	{
		int CulledNodes = 0;
		
		list<int>::iterator First, Second;

		while((noderoute.size() >= 2) && (CulledNodes < NodesToAttemptToCull))
		{
			First = noderoute.end();

			--First;

			Second = First;

			--Second;

			if((*Second) < 0)
				break;

			if(!zone->map->LineIntersectsZone(End, PathNodes[(*Second)].v, 1.0f, NULL, NULL))
			{
				noderoute.erase(First);

				++CulledNodes;
			}
			else
				break;
		}
	}

	return noderoute;
}

const char* DigitToWord(int i)
{
	switch(i) {
		case 0:
			return "zero";
		case 1:
			return "one";
		case 2:
			return "two";
		case 3:
			return "three";
		case 4:
			return "four";
		case 5:
			return "five";
		case 6:
			return "six";
		case 7:
			return "seven";
		case 8:
			return "eight";
		case 9:
			return "nine";
	}
	return "";
}

void PathManager::SpawnPathNodes()
{

	for(int i = 0; i < Head.PathNodeCount; ++i)
	{
		NPCType* npc_type = new NPCType;
		memset(npc_type, 0, sizeof(NPCType));

		if(i < 10)
			sprintf(npc_type->name, "%s", DigitToWord(i));
		else if(i < 100)
			sprintf(npc_type->name, "%s_%s", DigitToWord(i/10), DigitToWord(i % 10));
		else
			sprintf(npc_type->name, "%s_%s_%s", DigitToWord(i/100), DigitToWord((i % 100)/10), 
				DigitToWord(((i % 100) %10)));
		npc_type->cur_hp = 4000000;
		npc_type->max_hp = 4000000;
		npc_type->race = 151;
		npc_type->gender = 2;
		npc_type->class_ = 9;
		npc_type->deity= 1;
		npc_type->level = 75;
		npc_type->npc_id = 0;
		npc_type->loottable_id = 0;
		npc_type->texture = 1;
		npc_type->light = 0;
		npc_type->runspeed = 0;
		npc_type->d_meele_texture1 = 1;
		npc_type->d_meele_texture2 = 1;
		npc_type->merchanttype = 1;
		npc_type->bodytype = 1;
	
		npc_type->STR = 150;
		npc_type->STA = 150;
		npc_type->DEX = 150;
		npc_type->AGI = 150;
		npc_type->INT = 150;
		npc_type->WIS = 150;
		npc_type->CHA = 150;

		npc_type->findable = 1;
	
		NPC* npc = new NPC(npc_type, 0, PathNodes[i].v.x, PathNodes[i].v.y, PathNodes[i].v.z, 0, FlyMode1);
		npc->GiveNPCTypeData(npc_type);
	
		entity_list.AddNPC(npc, true, true);
	}
}

void PathManager::MeshTest()
{
	// This will test connectivity between all path nodes

	int TotalTests = 0;
	int NoConnections = 0;

	printf("Beginning Pathmanager connectivity tests.\n"); fflush(stdout);

	for(int i = 0; i < Head.PathNodeCount; ++i)
	{
		for(int j = 0; j < Head.PathNodeCount; ++j)
		{
			if(j == i)
				continue;
			
			list<int> Route = FindRoute(i, j);

			if(Route.size() == 0)
			{
				++NoConnections;
				printf("FindRoute(%i, %i) **** NO ROUTE FOUND ****\n", i, j);
			}
			++TotalTests;	
		}
	}
	printf("Executed %i route searches.\n", TotalTests);
	printf("Failed to find %i routes.\n", NoConnections);
	fflush(stdout);
}

VERTEX Mob::UpdatePath(float ToX, float ToY, float ToZ, float Speed, bool &WaypointChanged, bool &NodeReached)
{
	_ZP(Pathing_UpdatePath);

	WaypointChanged = false;

	NodeReached = false;

	VERTEX NodeLoc;

	VERTEX From(GetX(), GetY(), GetZ());

	VERTEX HeadPosition(From.x, From.y,  From.z + (GetSize() < 6.0 ? 6 : GetSize()) * HEAD_POSITION);

	VERTEX To(ToX, ToY, ToZ);

	bool SameDestination = (To == PathingDestination);

	int NextNode;

	if(To == From)
		return To;

	mlog(PATHING__DEBUG, "UpdatePath. From(%8.3f, %8.3f, %8.3f) To(%8.3f, %8.3f, %8.3f)", From.x, From.y, From.z, To.x, To.y, To.z);

	if(From == PathingLastPosition)
	{
		++PathingLoopCount;

		if((PathingLoopCount > 5) && !IsRooted())
		{
			mlog(PATHING__DEBUG, "appears to be stuck. Teleporting them to next position.", GetName());

			if(Route.size() == 0)
			{
				Teleport(To);

				WaypointChanged = true;

				PathingLoopCount = 0;

				return To;
			}
			NodeLoc = zone->pathing->GetPathNodeCoordinates(Route.front());

			Route.pop_front();

			++PathingTraversedNodes;

			Teleport(NodeLoc);

			WaypointChanged = true;

			PathingLoopCount = 0;

			return NodeLoc;
		}
	}
	else
	{
		PathingLoopCount = 0;

		PathingLastPosition = From;
	}
		
	if(Route.size() > 0)
	{

		// If we are already pathing, and the destination is the same as before ...
		if(SameDestination)
		{
			mlog(PATHING__DEBUG, "  Still pathing to the same destination.");

			// Get the coordinates of the first path node we are going to.
			NextNode = Route.front();

			NodeLoc = zone->pathing->GetPathNodeCoordinates(NextNode);

			// May need to refine this as rounding errors may mean we never have equality
			// We have reached the path node.
			if(NodeLoc == From)
			{
				mlog(PATHING__DEBUG, "  Arrived at node %i", NextNode);	

				NodeReached = true;

				PathingLastNodeVisited = Route.front();
				// We only check for LOS again after traversing more than 1 node, otherwise we can get into
				// a loop where we have a hazard and so run to a path node far enough away from the hazard, and
				// then run right back towards the same hazard again.
				//
				// An exception is when we are about to head for the last node. We always check LOS then. This
				// is because we are seeking a path to the node nearest to our target. This node may be behind the
				// target, and we may run past the target if we don't check LOS at this point.
				int RouteSize = Route.size();

				mlog(PATHING__DEBUG, "Route size is %i", RouteSize);

				if((RouteSize == 2)
				   || ((PathingTraversedNodes >= RuleI(Pathing, MinNodesTraversedForLOSCheck))
				   && (RouteSize <= RuleI(Pathing, MinNodesLeftForLOSCheck))
				   && PathingLOSCheckTimer->Check()))
				{
					mlog(PATHING__DEBUG, "  Checking distance to target.");
					float Distance = VertexDistanceNoRoot(From, To);

					mlog(PATHING__DEBUG, "  Distance between From and To (NoRoot) is %8.3f", Distance);

					if((Distance <= RuleR(Pathing, MinDistanceForLOSCheckShort))
					   && (ABS(From.z - To.z) <= RuleR(Pathing, ZDiffThreshold)))
					{
						if(!zone->map->LineIntersectsZone(HeadPosition, To, 1.0f, NULL, NULL))
							PathingLOSState = HaveLOS;
						else
							PathingLOSState = NoLOS;
						mlog(PATHING__DEBUG, "  LOS stats is %s", (PathingLOSState == HaveLOS) ? "HaveLOS" : "NoLOS");	

						if((PathingLOSState == HaveLOS) && zone->pathing->NoHazards(From, To, Speed))
						{
							mlog(PATHING__DEBUG, "  No hazards. Running directly to target.");
							Route.clear();

							return To;
						}
						else
						{
							mlog(PATHING__DEBUG, "  Continuing on node path.");	
						}
					}
					else
						PathingLOSState = UnknownLOS;
				}
				// We are on the same route, no LOS (or not checking this time, so pop off the node we just reached
				//
				Route.pop_front();

				++PathingTraversedNodes;

				WaypointChanged = true;

				// If there are more nodes on the route, return the coords of the next node
				if(Route.size() > 0)
				{
					NextNode = Route.front();

					if(NextNode == -1)
					{
						// -1 indicates a teleport to the next node
						Route.pop_front();

						if(Route.size() == 0)
						{
							mlog(PATHING__DEBUG, "Missing node after teleport.");
							return To;
						}

						NextNode = Route.front();

						NodeLoc = zone->pathing->GetPathNodeCoordinates(NextNode);

						Teleport(NodeLoc);

						mlog(PATHING__DEBUG, "  TELEPORTED to %8.3f, %8.3f, %8.3f\n", NodeLoc.x, NodeLoc.y, NodeLoc.z);

						Route.pop_front();

						if(Route.size() == 0)
							return To;

						NextNode = Route.front();
					}
					zone->pathing->OpenDoors(PathingLastNodeVisited, NextNode, this);

					mlog(PATHING__DEBUG, "  Now moving to node %i", NextNode);

					return  zone->pathing->GetPathNodeCoordinates(NextNode);
				}
				else
				{	
					// we have run all the nodes, all that is left is the direct path from the last node
					// to the destination
					mlog(PATHING__DEBUG, "  Reached end of node path, running direct to target.");

					return To;
				}
			}
			// At this point, we are still on the previous path, but not reached a node yet.
			// The route shouldn't be empty, but check anyway.
			//
			int RouteSize = Route.size();

			if((PathingTraversedNodes >= RuleI(Pathing, MinNodesTraversedForLOSCheck))
			   && (RouteSize <= RuleI(Pathing, MinNodesLeftForLOSCheck))
			   && PathingLOSCheckTimer->Check())
			{
				mlog(PATHING__DEBUG, "  Checking distance to target.");

				float Distance = VertexDistanceNoRoot(From, To);

				mlog(PATHING__DEBUG, "  Distance between From and To (NoRoot) is %8.3f", Distance);

				if((Distance <= RuleR(Pathing, MinDistanceForLOSCheckShort))
				   && (ABS(From.z - To.z) <= RuleR(Pathing, ZDiffThreshold)))
				{
					if(!zone->map->LineIntersectsZone(HeadPosition, To, 1.0f, NULL, NULL))
						PathingLOSState = HaveLOS;
					else
						PathingLOSState = NoLOS;
					mlog(PATHING__DEBUG, "  LOS stats is %s", (PathingLOSState == HaveLOS) ? "HaveLOS" : "NoLOS");	

					if((PathingLOSState == HaveLOS) && zone->pathing->NoHazards(From, To, Speed))
					{
						mlog(PATHING__DEBUG, "  No hazards. Running directly to target.");
						Route.clear();

						return To;
					}
					else
					{
						mlog(PATHING__DEBUG, "  Continuing on node path.");	
					}
				}
				else
					PathingLOSState = UnknownLOS;
			}
			return NodeLoc;
		}
		else
		{
			// We get here if we were already pathing, but our destination has now changed.
			//
			mlog(PATHING__DEBUG, "  Target has changed position.");
			// Update our record of where we are going to.
			PathingDestination = To;
			// Check if we now have LOS etc to the new destination.	
			if(PathingLOSCheckTimer->Check())
			{
				float Distance = VertexDistanceNoRoot(From, To);

				if((Distance <= RuleR(Pathing, MinDistanceForLOSCheckShort))
				   && (ABS(From.z - To.z) <= RuleR(Pathing, ZDiffThreshold)))
				{
					mlog(PATHING__DEBUG, "  Checking for short LOS at distance %8.3f.", Distance);
					if(!zone->map->LineIntersectsZone(HeadPosition, To, 1.0f, NULL, NULL))
						PathingLOSState = HaveLOS;
					else
						PathingLOSState = NoLOS;
	
					mlog(PATHING__DEBUG, "  LOS stats is %s", (PathingLOSState == HaveLOS) ? "HaveLOS" : "NoLOS");	
	
					if((PathingLOSState == HaveLOS) && zone->pathing->NoHazards(From, To, Speed))
					{
						mlog(PATHING__DEBUG, "  No hazards. Running directly to target.");
						Route.clear();
						return To;
					}
					else
					{
						mlog(PATHING__DEBUG, "  Continuing on node path.");	
					}
				}
			}

			// If the player is moving, we don't want to recalculate our route too frequently.
			//
			if(static_cast<int>(Route.size()) <= RuleI(Pathing, RouteUpdateFrequencyNodeCount))
			{
				if(!PathingRouteUpdateTimerShort->Check())
				{
					mlog(PATHING__DEBUG, "Short route update timer not yet expired.");
					return zone->pathing->GetPathNodeCoordinates(Route.front());
				}
				mlog(PATHING__DEBUG, "Short route update timer expired.");
			}
			else
			{
				if(!PathingRouteUpdateTimerLong->Check())
				{
					mlog(PATHING__DEBUG, "Long route update timer not yet expired.");
					return zone->pathing->GetPathNodeCoordinates(Route.front());
				}
				mlog(PATHING__DEBUG, "Long route update timer expired.");
			}

			// We are already pathing, destination changed, no LOS. Find the nearest node to our destination.
			int DestinationPathNode= zone->pathing->FindNearestPathNode(To);

			// Destination unreachable via pathing, return direct route.
			if(DestinationPathNode == -1)
			{
				mlog(PATHING__DEBUG, "  Unable to find path node for new destination. Running straight to target.");
				Route.clear();
				return To;
			}
			// If the nearest path node to our new destination is the same as for the previous
			// one, we will carry on on our path.
			if(DestinationPathNode == Route.back())
			{
				mlog(PATHING__DEBUG, "  Same destination Node (%i). Continue with current path.", DestinationPathNode);

				NodeLoc = zone->pathing->GetPathNodeCoordinates(Route.front());

				// May need to refine this as rounding errors may mean we never have equality
				// Check if we have reached a path node.
				if(NodeLoc == From)
				{	
					mlog(PATHING__DEBUG, "  Arrived at node %i, moving to next one.\n", Route.front());

					NodeReached = true;

					PathingLastNodeVisited = Route.front();
			
					Route.pop_front();

					++PathingTraversedNodes;

					WaypointChanged = true;

					if(Route.size() > 0)
					{
						NextNode = Route.front();

						if(NextNode == -1)
						{
							// -1 indicates a teleport to the next node
							Route.pop_front();

							if(Route.size() == 0)
							{
								mlog(PATHING__DEBUG, "Missing node after teleport.");
								return To;
							}
	
							NextNode = Route.front();

							NodeLoc = zone->pathing->GetPathNodeCoordinates(NextNode);

							Teleport(NodeLoc);

							mlog(PATHING__DEBUG, "  TELEPORTED to %8.3f, %8.3f, %8.3f\n", NodeLoc.x, NodeLoc.y, NodeLoc.z);

							Route.pop_front();

							if(Route.size() == 0)
								return To;

							NextNode = Route.front();
						}
						// Return the coords of our next path node on the route.
						mlog(PATHING__DEBUG, "  Now moving to node %i", NextNode);

						zone->pathing->OpenDoors(PathingLastNodeVisited, NextNode, this);

						return zone->pathing->GetPathNodeCoordinates(NextNode);
					}
					else
					{
						mlog(PATHING__DEBUG, "  Reached end of path grid. Running direct to target.");
						return To;
					}
				}
				return NodeLoc;
			}
			else 
			{
				mlog(PATHING__DEBUG, "  Target moved. End node is different. Clearing route.");

				Route.clear();
				// We will now fall through to get a new route.
			}

		}


	}
	mlog(PATHING__DEBUG, "  Our route list is empty.");

	if((SameDestination) && !PathingLOSCheckTimer->Check())
	{
		mlog(PATHING__DEBUG, "  Destination same as before, LOS check timer not reached. Returning To.");
		return To;
	}

	PathingLOSState = UnknownLOS;

	PathingDestination = To;

	WaypointChanged = true;

	float Distance = VertexDistanceNoRoot(From, To);

	if((Distance <= RuleR(Pathing, MinDistanceForLOSCheckLong))
	   && (ABS(From.z - To.z) <= RuleR(Pathing, ZDiffThreshold)))
	{	
		mlog(PATHING__DEBUG, "  Checking for long LOS at distance %8.3f.", Distance);
	
		if(!zone->map->LineIntersectsZone(HeadPosition, To, 1.0f, NULL, NULL))
			PathingLOSState = HaveLOS;
		else
			PathingLOSState = NoLOS;

		mlog(PATHING__DEBUG, "  LOS stats is %s", (PathingLOSState == HaveLOS) ? "HaveLOS" : "NoLOS");	
		
		if((PathingLOSState == HaveLOS) && zone->pathing->NoHazards(From, To, Speed))
		{
			mlog(PATHING__DEBUG, "Target is reachable. Running directly there.");
			return To;
		}
	}
	mlog(PATHING__DEBUG, "  Calculating new route to target.");

	Route = zone->pathing->FindRoute(From, To);

	PathingTraversedNodes = 0;

	if(Route.size() == 0)
	{
		mlog(PATHING__DEBUG, "  No route available, running direct.");

		return To;
	}

	if(SameDestination && (Route.front() == PathingLastNodeVisited))
	{
		mlog(PATHING__DEBUG, "  Probable loop detected. Same destination and Route.front() == PathingLastNodeVisited.");

		Route.clear();

		return To;
	}
	NodeLoc = zone->pathing->GetPathNodeCoordinates(Route.front());

	mlog(PATHING__DEBUG, "  New route determined, heading for node %i", Route.front());

	PathingLoopCount = 0;

	return NodeLoc;

}

int PathManager::FindNearestPathNode(VERTEX Position)
{

	// Find the nearest PathNode we have LOS to.
	//
	//

	float CandidateNodeRangeXY = RuleR(Pathing, CandidateNodeRangeXY);

	float CandidateNodeRangeZ = RuleR(Pathing, CandidateNodeRangeZ);

	int ClosestPathNodeToStart = -1;

	list<PathNodeSortStruct> SortedByDistance;

	PathNodeSortStruct TempNode;

	for(int i = 0 ; i < Head.PathNodeCount; ++i)
	{
		if((ABS(Position.x - PathNodes[i].v.x) <= CandidateNodeRangeXY) &&
		   (ABS(Position.y - PathNodes[i].v.y) <= CandidateNodeRangeXY) &&
		   (ABS(Position.z - PathNodes[i].v.z) <= CandidateNodeRangeZ))
		{
		   	TempNode.id = i;
			TempNode.Distance = VertexDistanceNoRoot(Position, PathNodes[i].v);
			SortedByDistance.push_back(TempNode);

		}
	}
	
	SortedByDistance.sort(SortPathNodesByDistance);

	for(list<PathNodeSortStruct>::iterator Iterator = SortedByDistance.begin(); Iterator != SortedByDistance.end(); ++Iterator)
	{
		_log(PATHING__DEBUG, "Checking Reachability of Node %i from Start Position.", PathNodes[(*Iterator).id].id);

		if(!zone->map->LineIntersectsZone(Position, PathNodes[(*Iterator).id].v, 1.0f, NULL, NULL))
		{
			ClosestPathNodeToStart = (*Iterator).id;
			break;
		}
	}

	if(ClosestPathNodeToStart <0 ) {
		_log(PATHING__DEBUG, "No LOS to any starting Path Node within range.");
		return -1;
	}
	return ClosestPathNodeToStart;
}

bool PathManager::NoHazards(VERTEX From, VERTEX To, float Speed)
{
	_ZP(Pathing_NoHazards);

	// Test the Z coordinate at the mid point.
	//
	VERTEX MidPoint((From.x + To.x) / 2, (From.y + To.y) / 2, From.z);

	float NewZ = zone->map->FindBestZ(MAP_ROOT_NODE, MidPoint, NULL, NULL);

	if(ABS(NewZ - From.z) > RuleR(Pathing, ZDiffThreshold))
	{
		_log(PATHING__DEBUG, "  HAZARD DETECTED moving from %8.3f, %8.3f, %8.3f to %8.3f, %8.3f, %8.3f. Z Change is %8.3f",
			From.x, From.y, From.z, MidPoint.x, MidPoint.y, MidPoint.z, NewZ - From.z);

		return false;
	}
	else
	{
		_log(PATHING__DEBUG, "No HAZARD DETECTED moving from %8.3f, %8.3f, %8.3f to %8.3f, %8.3f, %8.3f. Z Change is %8.3f",
			From.x, From.y, From.z, MidPoint.x, MidPoint.y, MidPoint.z, NewZ - From.z);
	}

	return true;
}

void Mob::PrintRoute()
{

	printf("Route is : ");
	
	list<int>::iterator Iterator;

	for(Iterator = Route.begin(); Iterator !=Route.end(); ++Iterator)
	{
		printf("%i, ", (*Iterator));
	}

	printf("\n");

}

void PathManager::OpenDoors(int Node1, int Node2, Mob *ForWho)
{

	_ZP(Pathing_OpenDoors);

	if(!ForWho || (Node1 >= Head.PathNodeCount) || (Node2 >= Head.PathNodeCount) || (Node1 < 0) || (Node2 < 0))
		return;

	for(int i = 0; i < PATHNODENEIGHBOURS; ++i)
	{
		if(PathNodes[Node1].Neighbours[i].id == -1)
			return;

		if(PathNodes[Node1].Neighbours[i].id != Node2)
			continue;

		if(PathNodes[Node1].Neighbours[i].DoorID >= 0)
		{
			Doors *d = entity_list.FindDoor(PathNodes[Node1].Neighbours[i].DoorID);

			if(d && !d->IsDoorOpen() )
			{
				_log(PATHING__DEBUG, "Opening door %i for %s", PathNodes[Node1].Neighbours[i].DoorID, ForWho->GetName());

				d->ForceOpen(ForWho);
			}
			return;
		}
	}
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
	
	int len = sizeof(FindPersonResult_Struct) + (points.size()+1) * sizeof(FindPerson_Point);
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_FindPersonReply, len);
	FindPersonResult_Struct* fpr=(FindPersonResult_Struct*)outapp->pBuffer;
	
	vector<FindPerson_Point>::iterator cur, end;
	cur = points.begin();
	end = points.end();
	unsigned int r;
	for(r = 0; cur != end; cur++, r++) {
		fpr->path[r] = *cur;

	}
	//put the last element into the destination field
	cur--;
	fpr->path[r] = *cur;
	fpr->dest = *cur;

	FastQueuePacket(&outapp);
	
	
}

PathNode* PathManager::FindPathNodeByCoordinates(float x, float y, float z)
{
	for(int i = 0; i < Head.PathNodeCount; ++i)
		if((PathNodes[i].v.x == x) && (PathNodes[i].v.y == y) && (PathNodes[i].v.z == z))
			return &PathNodes[i];

	return NULL;
}

int PathManager::GetRandomPathNode()
{
	return MakeRandomInt(0, Head.PathNodeCount - 1);

}

void PathManager::ShowPathNodeNeighbours(Client *c)
{
	if(!c || !c->GetTarget())
		return;


	PathNode *Node = zone->pathing->FindPathNodeByCoordinates(c->GetTarget()->GetX(), c->GetTarget()->GetY(), c->GetTarget()->GetZ());

	if(!Node)
	{
		c->Message(0, "Unable to find path node.");
		return;
	}
	c->Message(0, "Path node %4i", Node->id);

	for(int i = 0; i < Head.PathNodeCount; ++i)
	{
		char Name[64];

		if(PathNodes[i].id < 10)
			sprintf(Name, "%s000", DigitToWord(PathNodes[i].id));
		else if(PathNodes[i].id < 100)
			sprintf(Name, "%s_%s000", DigitToWord(PathNodes[i].id / 10), DigitToWord(PathNodes[i].id % 10));
		else
			sprintf(Name, "%s_%s_%s000", DigitToWord(PathNodes[i].id/100), DigitToWord((PathNodes[i].id % 100)/10), 
				DigitToWord(((PathNodes[i].id % 100) %10)));

		Mob *m = entity_list.GetMob(Name);

		if(m)
			m->SendIllusionPacket(151);
	}

	std::stringstream Neighbours;

	for(int i = 0; i < PATHNODENEIGHBOURS; ++i)
	{
		if(Node->Neighbours[i].id == -1)
			break;
		Neighbours << Node->Neighbours[i].id << ", ";	

		char Name[64];

		if(Node->Neighbours[i].id < 10)
			sprintf(Name, "%s000", DigitToWord(Node->Neighbours[i].id));
		else if(Node->Neighbours[i].id < 100)
			sprintf(Name, "%s_%s000", DigitToWord(Node->Neighbours[i].id / 10), DigitToWord(Node->Neighbours[i].id % 10));
		else
			sprintf(Name, "%s_%s_%s000", DigitToWord(Node->Neighbours[i].id/100), DigitToWord((Node->Neighbours[i].id % 100)/10), 
				DigitToWord(((Node->Neighbours[i].id % 100) %10)));

		Mob *m = entity_list.GetMob(Name);

		if(m)
			m->SendIllusionPacket(46);
	}
	c->Message(0, "Neighbours: %s", Neighbours.str().c_str());

}



