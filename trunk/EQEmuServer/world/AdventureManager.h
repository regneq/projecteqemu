#ifndef ADVENTURE_MANAGER_H
#define ADVENTURE_MANAGER_H

#include "../common/debug.h"
#include "../common/types.h"
#include "../common/timer.h"
#include "Adventure.h"
#include "AdventureTemplate.h"
#include <map>
#include <list>

using namespace std;

class AdventureManager
{
public:
	AdventureManager();
	~AdventureManager();

	void Process();

	bool LoadAdventureTemplates();
	bool LoadAdventureEntries();
	void LoadLeaderboardInfo();

	void CalculateAdventureRequestReply(const char *data);
	void PlayerClickedDoor(const char *player, int zone_id, int door_id);
	void TryAdventureCreate(const char *data);
	void GetAdventureData(Adventure *adv);
	void GetAdventureData(const char *name);
	void LeaveAdventure(const char *name);
	void IncrementCount(uint16 instance_id);
	void IncrementAssassinationCount(uint16 instance_id);
	void DoLeaderboardRequest(const char* player);
	void SendAdventureFinish(AdventureFinishEvent fe);
	void AddFinishedEvent(AdventureFinishEvent fe) { finished_list.push_back(fe); Save(); }
	bool PopFinishedEvent(const char *name, AdventureFinishEvent &fe);
	void Save();
	void Load();

	Adventure **GetFinishedAdventures(const char *player, int &count);
	Adventure *GetActiveAdventure(const char *player);
	AdventureTemplate *GetAdventureTemplate(int theme, int id);
	AdventureTemplate *GetAdventureTemplate(int id);
	void GetZoneData(uint16 instance_id);
protected:
	bool IsInExcludedZoneList(list<AdventureZones> excluded_zones, string zone_name, int version);
	bool IsInExcludedZoneInList(list<AdventureZoneIn> excluded_zone_ins, int zone_id, int door_object);

	map<uint32, AdventureTemplate*> adventure_templates;
	map<uint32, list<AdventureTemplate*> > adventure_entries;
	list<Adventure*> adventure_list;
	list<AdventureFinishEvent> finished_list;
	list<LeaderboardInfo> leaderboard_info;
	Timer *process_timer;
	Timer *save_timer;
	Timer *leaderboard_info_timer;
};

#endif