#include "../common/debug.h"
#include "../common/MiscFunctions.h"
#include "QGlobals.h"
#include "zonedb.h"

void QGlobalCache::AddGlobal(uint32 id, QGlobal global)
{
	std::map<uint32, QGlobal>::iterator iter = qGlobalBucket.find(id);
	if(iter != qGlobalBucket.end())
	{
		qGlobalBucket.erase(iter);
	}
	qGlobalBucket[id] = global;
}

void QGlobalCache::RemoveGlobal(std::string name, uint32 npcID, uint32 charID, uint32 zoneID)
{
	std::map<uint32, QGlobal>::iterator iter = qGlobalBucket.begin();
	while(iter != qGlobalBucket.end())
	{
		if(name.compare(iter->second.name) == 0)
		{
			if((npcID == iter->second.npc_id || iter->second.npc_id == 0) && (charID == iter->second.char_id || iter->second.char_id == 0) && (zoneID == iter->second.zone_id) || iter->second.zone_id == 0)
			{
				qGlobalBucket.erase(iter);
				return;
			}
		}
		++iter;
	}
}

std::map<uint32, QGlobal> QGlobalCache::Combine(std::map<uint32, QGlobal> cacheA, std::map<uint32, QGlobal> cacheB, uint32 npcID, uint32 charID, uint32 zoneID)
{
	std::map<uint32, QGlobal> returnCache;
	std::map<uint32, QGlobal>::iterator iter = cacheA.begin();
	while(iter != cacheA.end())
	{
		QGlobal cur = iter->second;

		if((cur.npc_id == npcID || cur.npc_id == 0) && (cur.char_id == charID || cur.char_id == 0) && (cur.zone_id == zoneID || cur.zone_id == 0))
		{

			if(Timer::GetTimeSeconds() < cur.expdate)
			{
				returnCache[iter->first] = cur;
			}
		}
		++iter;
	}
	
	iter = cacheB.begin();
	while(iter != cacheB.end())
	{
		QGlobal cur = iter->second;

		if((cur.npc_id == npcID || cur.npc_id == 0) && (cur.char_id == charID || cur.char_id == 0) && (cur.zone_id == zoneID || cur.zone_id == 0))
		{
			if(Timer::GetTimeSeconds() < cur.expdate)
			{
				returnCache[iter->first] = cur;
			}
		}
		++iter;
	}
	
	return returnCache;
}

void QGlobalCache::PurgeExpiredGlobals()
{
	if(!qGlobalBucket.size())
		return;

	std::map<uint32, QGlobal>::iterator iter = qGlobalBucket.begin();
	while(iter != qGlobalBucket.end())
	{
		QGlobal cur = iter->second;
		if(Timer::GetTimeSeconds() > cur.expdate)
		{
			qGlobalBucket.erase(iter);
			iter = qGlobalBucket.begin();
		}
		++iter;
	}
}

void QGlobalCache::LoadByNPCID(uint32 npcID)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (database.RunQuery(query, MakeAnyLenString(&query, "select id, name, charid, npcid, zoneid, value, expdate"
		" from quest_globals where npcid = %d", npcID), errbuf, &result))
	{
		while((row = mysql_fetch_row(result)))
		{
			AddGlobal(atoi(row[0]), QGlobal(std::string(row[1]), atoi(row[2]), atoi(row[3]), atoi(row[4]), row[5], row[6]?atoi(row[6]):0xFFFFFFFF));
		}
		mysql_free_result(result);
	}
	safe_delete_array(query);
}

void QGlobalCache::LoadByCharID(uint32 charID)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (database.RunQuery(query, MakeAnyLenString(&query, "select id, name, charid, npcid, zoneid, value, expdate from"
		" quest_globals where charid = %d && npcid = 0", charID), errbuf, &result))
	{
		while((row = mysql_fetch_row(result)))
		{
			AddGlobal(atoi(row[0]), QGlobal(std::string(row[1]), atoi(row[2]), atoi(row[3]), atoi(row[4]), row[5], row[6]?atoi(row[6]):0xFFFFFFFF));
		}
		mysql_free_result(result);
	}
	safe_delete_array(query);
}

void QGlobalCache::LoadByZoneID(uint32 zoneID)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (database.RunQuery(query, MakeAnyLenString(&query, "select id, name, charid, npcid, zoneid, value, expdate from quest_globals"
		" where zoneid = %d && npcid = 0 && charid = 0", zoneID), errbuf, &result))
	{
		while((row = mysql_fetch_row(result)))
		{
			AddGlobal(atoi(row[0]), QGlobal(std::string(row[1]), atoi(row[2]), atoi(row[3]), atoi(row[4]), row[5], row[6]?atoi(row[6]):0xFFFFFFFF));
		}
		mysql_free_result(result);
	}
	safe_delete_array(query);
}
void QGlobalCache::LoadByGlobalContext()
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (database.RunQuery(query, MakeAnyLenString(&query, "select id, name, charid, npcid, zoneid, value, expdate from quest_globals"
		" where zoneid = 0 && npcid = 0 && charid = 0"), errbuf, &result))
	{
		while((row = mysql_fetch_row(result)))
		{
			AddGlobal(atoi(row[0]), QGlobal(std::string(row[1]), atoi(row[2]), atoi(row[3]), atoi(row[4]), row[5], row[6]?atoi(row[6]):0xFFFFFFFF));
		}
		mysql_free_result(result);
	}
	safe_delete_array(query);
}
