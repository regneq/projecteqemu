/*  EQEMu:  Everquest Server Emulator
	Copyright (C) 2001-2003  EQEMu Development Team (http://eqemulator.net)

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
#ifndef EQEMU_DATABASE_H
#define EQEMU_DATABASE_H

#define AUTHENTICATION_TIMEOUT	60
#define INVALID_ID				0xFFFFFFFF

#include "debug.h"
#include "types.h"
#include "dbcore.h"
#include "linked_list.h"
/*#include "eq_packet_structs.h"
#include "EQStream.h"
#include "guilds.h"
#include "MiscFunctions.h"
#include "Mutex.h"
#include "Item.h"
#include "extprofile.h"*/
#include <string>
#include <vector>
#include <map>
using namespace std;

//atoi is not int32 or uint32 safe!!!!
#define atoul(str) strtoul(str, NULL, 10)

//class Spawn;
class Corpse;
class Spawn2;
class NPC;
class SpawnGroupList;
class Petition;
class Client;
struct Combine_Struct;
//struct Faction;
//struct FactionMods;
//struct FactionValue;
struct ZonePoint;
struct NPCType;
class Inventory;
class ItemInst;

struct EventLogDetails_Struct {
	int32	id;
	char	accountname[64];
	int32	account_id;
	sint16	status;
	char	charactername[64];
	char	targetname[64];
	char	timestamp[64];
	char	descriptiontype[64];
	char	details[128];
};

struct CharacterEventLog_Struct {
int32	count;
int8	eventid;
EventLogDetails_Struct eld[255];
};


// Added By Hogie 
// INSERT into variables (varname,value) values('decaytime [minlevel] [maxlevel]','[number of seconds]');
// IE: decaytime 1 54 = Levels 1 through 54
//     decaytime 55 100 = Levels 55 through 100
// It will always put the LAST time for the level (I think) from the Database
struct npcDecayTimes_Struct {
	int16 minlvl;
	int16 maxlvl;
	int32 seconds;
};
// Added By Hogie -- End

struct VarCache_Struct {
	char varname[26];	// varname is char(25) in database
	char value[0];
};

struct PlayerProfile_Struct;
struct GuildRankLevel_Struct;
struct GuildRanks_Struct;
struct ExtendedProfile_Struct;
struct GuildMember_Struct;
class PTimerList;

class Database : public DBcore {
public:
	Database();
	Database(const char* host, const char* user, const char* passwd, const char* database,int32 port);
	bool Connect(const char* host, const char* user, const char* passwd, const char* database,int32 port);
	~Database();
	
	
//	void	ExtraOptions();
	

	/*
	 * General Character Related Stuff
	 */
	bool	MoveCharacterToZone(const char* charname, const char* zonename);
	bool	MoveCharacterToZone(const char* charname, const char* zonename,int32 zoneid);
	bool	MoveCharacterToZone(int32 iCharID, const char* iZonename);
	bool	UpdateName(const char* oldname, const char* newname);
	bool	SetHackerFlag(const char* accountname, const char* charactername, const char* hacked);
	bool	SetMQDetectionFlag(const char* accountname, const char* charactername, const char* hacked, const char* zone);
	bool	AddToNameFilter(const char* name);
	bool	ReserveName(int32 account_id, char* name);
	bool	CreateCharacter(uint32 account_id, char* name, int16 gender, int16 race, int16 class_, int8 str, int8 sta, int8 cha, int8 dex, int8 int_, int8 agi, int8 wis, int8 face);
	bool	StoreCharacter(uint32 account_id, PlayerProfile_Struct* pp, Inventory* inv, ExtendedProfile_Struct *ext);
	bool	DeleteCharacter(char* name);
	int8	CopyCharacter(const char* oldname, const char* newname, int32 acctid);
	
	/*
	 * General Information Getting Queries
	 */
	bool	CheckNameFilter(const char* name);
	bool	CheckUsedName(const char* name);
	int32	GetAccountIDByChar(const char* charname, int32* oCharID = 0);
	uint32	GetAccountIDByChar(uint32 char_id);
	int32	GetAccountIDByName(const char* accname, sint16* status = 0, int32* lsid = 0);
	void	GetAccountName(int32 accountid, char* name, int32* oLSAccountID = 0);
	void	GetCharName(int32 char_id, char* name);
	int32	GetCharacterInfo(const char* iName, int32* oAccID = 0, int32* oZoneID = 0, float* oX = 0, float* oY = 0, float* oZ = 0);
	int32	GetCharacterID(const char *name);
	bool	CheckBannedIPs(const char* loginIP); //Lieka Edit:  Check incomming connection against banned IP table.
 	bool	AddBannedIP(char* bannedIP, const char* notes); //Lieka Edit:  Add IP address to the Banned_IPs table.
	bool	CheckGMIPs(const char* loginIP, int32 account_id);
	bool	AddGMIP(char* ip_address, char* name);

	/*
	 * Account Related
	 */
	int32	GetMiniLoginAccount(char* ip);
	void	GetAccountFromID(int32 id, char* oAccountName, sint16* oStatus);
	int32	CheckLogin(const char* name, const char* password, sint16* oStatus = 0);
	sint16	CheckStatus(int32 account_id);
	int32	CreateAccount(const char* name, const char* password, sint16 status, int32 lsaccount_id = 0);
	bool	DeleteAccount(const char* name);
 	bool	SetAccountStatus(const char* name, sint16 status);
	bool	SetLocalPassword(uint32 accid, const char* password);
	int32	GetAccountIDFromLSID(int32 iLSID, char* oAccountName = 0, sint16* oStatus = 0);
	bool	UpdateLiveChar(char* charname,int32 lsaccount_id);
	bool	GetLiveChar(int32 account_id, char* cname);
	int8	GetAgreementFlag(int32 acctid);
	void	SetAgreementFlag(int32 acctid);
	
	/*
	 * Groups
	 */
	int32	GetGroupID(const char* name);
	void	SetGroupID(const char* name, int32 id, int32 charid);
	void	ClearGroup(int32 gid = 0);
	char*	GetGroupLeaderForLogin(const char* name,char* leaderbuf);
	
	void	SetGroupLeaderName(int32 gid, const char* name);
	char	*GetGroupLeaderName(int32 gid, char* leaderbuf);
	void	ClearGroupLeader(int32 gid = 0);

	/*
	 * Raids
	 */
	void	ClearRaid(int32 rid = 0);
	void	ClearRaidDetails(int32 rid = 0);
	int32	GetRaidID(const char* name);

	/*
	 * Database Varaibles
	 */
	bool	GetVariable(const char* varname, char* varvalue, int16 varvalue_len);
	bool	SetVariable(const char* varname, const char* varvalue);
	bool	LoadVariables();
	int32	LoadVariables_MQ(char** query);
	bool	LoadVariables_result(MYSQL_RES* result);
	
	/*
	 * General Queries
	 */
	bool	LoadZoneNames();
	bool	GetZoneLongName(const char* short_name, char** long_name, char* file_name = 0, float* safe_x = 0, float* safe_y = 0, float* safe_z = 0, int32* graveyard_id = 0, int32* maxclients = 0);
	bool	GetZoneGraveyard(const int32 graveyard_id, int32* graveyard_zoneid = 0, float* graveyard_x = 0, float* graveyard_y = 0, float* graveyard_z = 0, float* graveyard_heading = 0);
	int32	GetZoneGraveyardID(int32 zone_id);
	int32	GetZoneID(const char* zonename);
	const char*	GetZoneName(int32 zoneID, bool ErrorUnknown = false);
	int8	GetServerType();
	bool	GetSafePoints(const char* short_name, float* safe_x = 0, float* safe_y = 0, float* safe_z = 0, sint16* minstatus = 0, int8* minlevel = 0, char *flag_needed = NULL);
	bool	GetSafePoints(int32 zoneID, float* safe_x = 0, float* safe_y = 0, float* safe_z = 0, sint16* minstatus = 0, int8* minlevel = 0, char *flag_needed = NULL) { return GetSafePoints(GetZoneName(zoneID), safe_x, safe_y, safe_z, minstatus, minlevel, flag_needed); }
	int8	GetSkillCap(int8 skillid, int8 in_race, int8 in_class, int16 in_level);
	int8	GetRaceSkill(int8 skillid, int8 in_race);
	
#ifdef EQBOTS

	//franck-adds: EQoffline ------------------
	// 1:
	void	AddBot(int32 mobidtmp);
	void	RemoveBot(int32 mobidtmp);
	
	// 2:
	bool	DeleteBot(int32 mobid);
	int		GetBotStatus(int32 mobidtmp);
	void	SetBotLeader(int32 mobidtmp, int32 leaderid);
	int		GetBotLeader(int32 mobidtmp);
	int		GetBotOwner(int32 mobid);
	void	SetBotOwner(int32 mobid, int32 ownerid);
	void	UpdateBotOwner(int32 accountid, int32 ownerid);
	void	CleanBotLeader(int32 leaderid);
	void    CleanBotLeaderEntries(int32 mobidtmp);
	int		CountBots(int32 leaderid);

    // 3:
    // a)  Get and Set an item in the bot inventory
    bool	BotHasAnItemInSlot(int32 botid, int32 slot);			// return true if it has an item in the invent and false if not
    int		GetBotItemBySlot(int32 botid, int32 slot);				// return the item ID in the given slot
    void	SetBotItemInSlot(int32 botid, int32 slot, int32 itemid);// add an item the in the slot
    void	RemoveBotItemBySlot(int32 botid, int32 slot);			// remove an item in the given slot

	// c) How many items do the bots have
	int		GetBotItemsNumber(int32 botid);							// return the number of items that the bots have

#endif //EQBOTS
	
	bool	LoadPTimers(uint32 charid, PTimerList &into);
	void	ClearPTimers(uint32 charid);
	void	ClearMerchantTemp();
	int32	GetCharInstFlagNum(int32 charID);
	int32	GetCharInstZOrgID(int32 charID);
	int32   GetInstZoneID(int32 zoneID, const char* charName);
	void	DeleteInstZone(int32 instZoneID);
	int32   GetDfltInstZFlag();
	void	setCharInstFlag(int charID, int  orgZoneID, int instFlag);
	void	setGroupInstFlagNum(int charID, int orgZoneID, int instFlag);
	void	setRaidInstFlagNum(int charID, int orgZoneID, int instFlag);
	void	incrCurInstFlagNum(int instFlag);
	int	getCurInstFlagNum();
	void	SetLFP(int32 CharID, bool LFP); 
	void	SetLFG(int32 CharID, bool LFG); 
	

protected:
	void	HandleMysqlError(int32 errnum);
	//bool	RunQuery(const char* query, int32 querylen, char* errbuf = 0, MYSQL_RES** result = 0, int32* affected_rows = 0, int32* errnum = 0, bool retry = true);
	
private:
	void DBInitVars();
	
	int32				max_zonename;
	char**				zonename_array;
	
	Mutex				Mvarcache;
	uint32				varcache_max;
	VarCache_Struct**	varcache_array;
	uint32				varcache_lastupdate;
};

bool	FetchRowMap(MYSQL_RES *result, map<string,string> &rowmap);
#endif
