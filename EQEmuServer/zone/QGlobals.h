#ifndef __QGLOBALS__H
#define __QGLOBALS__H

#include <map>
#include <string>
#include "../common/timer.h"

struct QGlobal
{
	QGlobal() { }
	QGlobal(std::string g_name, uint32 c_id, uint32 n_id, uint32 z_id, std::string n_value, uint32 expire_date) 
		: name(g_name), char_id(c_id), npc_id(n_id), zone_id(z_id), value(n_value), expdate(expire_date) { }
	std::string name;
	std::string value;
	uint32 npc_id;
	uint32 char_id;
	uint32 zone_id;
	uint32 expdate;
};

class QGlobalCache
{
public:
	void AddGlobal(uint32 id, QGlobal global);
	//void RemoveGlobal(std::string name);
	std::map<uint32, QGlobal> GetMap() { return qGlobalBucket; }
	static std::map<uint32, QGlobal> Combine(std::map<uint32, QGlobal> cacheA, std::map<uint32, QGlobal> cacheB, uint32 npcID, uint32 charID, uint32 zoneID);
	
	void LoadByNPCID(uint32 npcID); //npc
	void LoadByCharID(uint32 charID); //client
	void LoadByZoneID(uint32 zoneID); //zone
	void LoadByGlobalContext(); //zone
protected:
	std::map<uint32, QGlobal> qGlobalBucket;
};

#endif