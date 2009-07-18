#ifndef PATHING_H
#define PATHING_H
#include <algorithm>
#include "map.h"
#include "../common/timer.h"
#include <list>
#include <vector>
#include <algorithm>

using namespace std;

class Client;

#define PATHNODENEIGHBOURS 50

#pragma pack(1)

struct AStarNode
{
	int PathNodeID;
	int Parent;
	float HCost;
	float GCost;
	bool Teleport;
};

struct NeighbourNode {
	sint16 id;
	float distance;
	uint8 Teleport;
	sint16 DoorID;
};

struct PathNode {
	uint16 id;
	VERTEX v;
	float bestz;
	NeighbourNode Neighbours[PATHNODENEIGHBOURS];
};

struct PathFileHeader {
	long version;
	long PathNodeCount;
};

#pragma pack()

struct PathNodeSortStruct
{
	int id;
	float Distance;
};

enum LOSType{ UnknownLOS, HaveLOS, NoLOS };

class PathManager {

public:
	PathManager();
	~PathManager();


	static PathManager *LoadPathFile(const char *ZoneName);
	bool loadPaths(FILE *fp);
	void PrintPathing();
	list<int> FindRoute(VERTEX Start, VERTEX End);
	list<int> FindRoute(int startID, int endID);

	VERTEX GetPathNodeCoordinates(int NodeNumber, bool BestZ = true);
	bool CheckLosFN(VERTEX myloc, VERTEX oloc);
	void SpawnPathNodes();
	void MeshTest();
	int FindNearestPathNode(VERTEX Position);
	bool NoHazards(VERTEX From, VERTEX To);
	void OpenDoors(int Node1, int Node2, Mob* ForWho);

	PathNode* FindPathNodeByCoordinates(float x, float y, float z);
	void ShowPathNodeNeighbours(Client *c);
	int GetRandomPathNode();

private:
	PathFileHeader Head;
	PathNode *PathNodes;

	int *ClosedListFlag;
};

	
#endif

