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
#include "../common/debug.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <iostream>
using namespace std;

#ifdef WIN32
#include <process.h>
#else
#include <pthread.h>
#include "../common/unix.h"
#endif

#include "net.h"
#include "masterentity.h"
#include "worldserver.h"
#include "PlayerCorpse.h"
#include "../common/guilds.h"
#include "../common/packet_dump.h"
#include "../common/packet_functions.h"
#include "petitions.h"
#include "spdat.h"
#include "features.h"
#include "StringIDs.h"
#include "parser.h"
#include "../common/dbasync.h"
#include "guild_mgr.h"
#include "raids.h"

#ifdef WIN32
#define snprintf	_snprintf
#if (_MSC_VER < 1500)
	#define vsnprintf	_vsnprintf
#endif
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#endif

extern Zone* zone;
extern volatile bool ZoneLoaded;
extern WorldServer worldserver;
extern NetConnection net;
extern int32 numclients;
#if !defined(NEW_LoadSPDat) && !defined(DB_LoadSPDat)
	extern SPDat_Spell_Struct spells[SPDAT_RECORDS];
#endif
extern bool spells_loaded;
extern PetitionList petition_list;
extern DBAsync *dbasync;

extern char  errorname[32];
extern int16 adverrornum;

Entity::Entity() {
	id = 0;
	pDBAsyncWorkID = 0;
}

Entity::~Entity() {
	dbasync->CancelWork(pDBAsyncWorkID);
}

//void Entity::SetID(int16 set_id) {
//	id = set_id;
//}

Client* Entity::CastToClient() {
	if(this==0x00){
		cout << "CastToClient error (NULL)" << endl;
		DebugBreak();
		return 0;
	}
#ifdef _EQDEBUG
	if(!IsClient()) {
		cout << "CastToClient error (not client?)" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<Client*>(this);
}

NPC* Entity::CastToNPC() {
#ifdef _EQDEBUG
	if(!IsNPC()) {	
		cout << "CastToNPC error" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<NPC*>(this);
}

Mob* Entity::CastToMob() {
#ifdef _EQDEBUG
	if(!IsMob()) {	
		cout << "CastToMob error" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<Mob*>(this);
}


Trap* Entity::CastToTrap()
{
#ifdef DEBUG
	if(!IsTrap())
	{
		//cout << "CastToTrap error" << endl;
		return 0;
	}
#endif
	return static_cast<Trap*>(this);
}

Corpse* Entity::CastToCorpse() {
#ifdef _EQDEBUG
	if(!IsCorpse()) {	
		cout << "CastToCorpse error" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<Corpse*>(this);
}
Object* Entity::CastToObject() {
#ifdef _EQDEBUG
	if(!IsObject()) {	
		cout << "CastToObject error" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<Object*>(this);
}

/*Group* Entity::CastToGroup() {
#ifdef _EQDEBUG
	if(!IsGroup()) {	
		cout << "CastToGroup error" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<Group*>(this);
}*/

Doors* Entity::CastToDoors() {
return static_cast<Doors*>(this);
}

Beacon* Entity::CastToBeacon() {
	return static_cast<Beacon*>(this);
}




const Client* Entity::CastToClient() const {
	if(this==0x00){
		cout << "CastToClient error (NULL)" << endl;
		DebugBreak();
		return 0;
	}
#ifdef _EQDEBUG
	if(!IsClient()) {
		cout << "CastToClient error (not client?)" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<const Client*>(this);
}

const NPC* Entity::CastToNPC() const {
#ifdef _EQDEBUG
	if(!IsNPC()) {	
		cout << "CastToNPC error" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<const NPC*>(this);
}

const Mob* Entity::CastToMob() const {
#ifdef _EQDEBUG
	if(!IsMob()) {	
		cout << "CastToMob error" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<const Mob*>(this);
}


const Trap* Entity::CastToTrap() const {
#ifdef DEBUG
	if(!IsTrap())
	{
		//cout << "CastToTrap error" << endl;
		return 0;
	}
#endif
	return static_cast<const Trap*>(this);
}

const Corpse* Entity::CastToCorpse() const {
#ifdef _EQDEBUG
	if(!IsCorpse()) {	
		cout << "CastToCorpse error" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<const Corpse*>(this);
}

const Object* Entity::CastToObject() const {
#ifdef _EQDEBUG
	if(!IsObject()) {	
		cout << "CastToObject error" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<const Object*>(this);
}

const Doors* Entity::CastToDoors() const {
return static_cast<const Doors*>(this);
}

const Beacon* Entity::CastToBeacon() const {
	return static_cast<const Beacon*>(this);
}

#ifdef BOTS
Bot* Entity::CastToBot() {
#ifdef _EQDEBUG
	if(!IsBot()) {	
		cout << "CastToBot error" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<Bot*>(this);
}
#endif

EntityList::EntityList() {
	last_insert_id = 0;
}

EntityList::~EntityList() {
	//must call this before the list is destroyed, or else it will try to
	//delete the NPCs in the list, which it cannot do.
	RemoveAllLocalities();
}

int32 EntityList::GetZoneEntityID() {
	if(last_insert_id > 1500)
		last_insert_id = 0;

	int32 getid=last_insert_id;

	while(1) {
		getid++;

		if (GetByEntityID(getid) == 0) {
			last_insert_id = getid;
			return getid;
		}
	}
}

int32 EntityList::GetGlobalEntityID(Client* client) {
	int32 Result = 0;

	if(client) {
		int32 tempGlobalEntityID = 0;

		tempGlobalEntityID = GetGlobalEntityIDByObjectTypeAndTableID(1, client->CharacterID());

		if(tempGlobalEntityID == 0) {
			std::string errorMessage;
			char* Query = 0;
			char TempErrorMessageBuffer[MYSQL_ERRMSG_SIZE];

			if(!database.RunQuery(Query, MakeAnyLenString(&Query, "insert into globalentity (GlobalEntityTypeID, GlobalObjectTableID) VALUES (1, %i)", client->CharacterID()), TempErrorMessageBuffer, 0, 0, &tempGlobalEntityID)) {
				errorMessage = std::string(TempErrorMessageBuffer);
				// TODO: Write this error message to zone error log
			}
			else {
				Result = tempGlobalEntityID;
			}
		}
		else
			Result = tempGlobalEntityID;
	}

	return Result;
}

//int16 EntityList::GetGlobalEntityID(Group* group) {
//	int16 Result = 0;
//
//	if(group) {
//		//
//	}
//
//	return Result;
//}
//
//int16 EntityList::GetGlobalEntityID(Raid* raid) {
//	int16 Result = 0;
//
//	if(raid) {
//		//
//	}
//
//	return Result;
//}

int32 EntityList::GetGlobalEntityIDByObjectTypeAndTableID(int16 entityObjectType, int32 objectTableID) {
	int32 Result = 0;

	if(entityObjectType > 0 && objectTableID > 0) {
		char* Query = 0;
		char TempErrorMessageBuffer[MYSQL_ERRMSG_SIZE];
		MYSQL_RES* DatasetResult;
		MYSQL_ROW DataRow;

		if(!database.RunQuery(Query, MakeAnyLenString(&Query, "select ID from globalentity where GlobalEntityTypeID = %i and GlobalObjectTableID = %i", entityObjectType, objectTableID), TempErrorMessageBuffer, &DatasetResult)) {
			// TODO: Write error message to zone error log
			//*errorMessage = std::string(TempErrorMessageBuffer);
		}
		else {
			while(DataRow = mysql_fetch_row(DatasetResult)) {
				Result = atoi(DataRow[0]);
				break;
			}

			mysql_free_result(DatasetResult);
		}

		safe_delete(Query);
	}

	return Result;
}

bool EntityList::CanAddHateForMob(Mob *p) {
	bool Result = false;

	if(p) {
		int mobCount = 0;

		list<NPC*> ClonedNPCList = npc_list;

		for(list<NPC*>::iterator npcListItr = ClonedNPCList.begin(); npcListItr != ClonedNPCList.end(); npcListItr++) {
			NPC* npcMob = *npcListItr;

			if(npcMob) {
				if(npcMob->IsOnHatelist(p)) {
					if(mobCount < 3)
						mobCount++;
					else
						break;
				}
			}
		}

		if(mobCount < 3)
			Result = true;
	}

	return Result;
}

void EntityList::AddClient(Client* client) {
	int16 TempEntityId = GetGlobalEntityID(client);
	
	if(TempEntityId > 0) {
		client->SetID(TempEntityId);

		// Push this object to it's maps
		mob_entityid_map.insert(MobMapPair(client->GetID(), client));
		mob_name_map.insert(MobMapByNamePair(std::string(client->GetName()), client));

		client_entityid_map.insert(ClientMapPair(client->GetID(), client));
		client_characterid_map.insert(ClientMapPair(client->CharacterID(), client));
		client_name_map.insert(ClientMapByNamePair(std::string(client->GetName()), client));

		// Push this object to it's lists
		client_list.push_back(client);
		mob_list.push_back(client);
	}
}


void EntityList::TrapProcess() {
	if(numclients < 1)
		return;
	_ZP(EntityList_TrapProcess);
	LinkedListIterator<Trap*> iterator(trap_list);
	iterator.Reset();
	int32 count=0;
	while(iterator.MoreElements())
	{
		count++;
		if(!iterator.GetData()->Process()){
			iterator.RemoveCurrent();
		}
		else
			iterator.Advance();
	}
	if(count==0)
		net.trap_timer.Disable();//No traps in list, disable until one is added
}


// Debug function -- checks to see if group_list has any NULL entries.
// Meant to be called after each group-related function, in order
// to track down bugs.
void EntityList::CheckGroupList (const char *fname, const int fline)
{
	list<Group *>::iterator it;

	for (it = group_list.begin(); it != group_list.end(); it++)
	{
		if (*it == NULL)
		{
			LogFile->write(EQEMuLog::Error, "NULL group, %s:%i", fname, fline);
		}
	}
}

void EntityList::GroupProcess() {
	list<Group *>::iterator iterator;
	int32 count = 0;

	if(numclients < 1)
		return;
	_ZP(EntityList_GroupProcess);

	iterator = group_list.begin();
	while(iterator != group_list.end())
	{
		count++;
		(*iterator)->Process();
		/*
		if(!iterator.GetData()->Process()){
			iterator.RemoveCurrent();
		}
		else
			iterator.Advance();
		*/
		iterator++;
	}
	if(count == 0)
		net.group_timer.Disable();//No groups in list, disable until one is added

#if EQDEBUG >= 5
	CheckGroupList (__FILE__, __LINE__);
#endif
}

void EntityList::QueueToGroupsForNPCHealthAA(Mob* sender, const EQApplicationPacket* app)
{

	list<Group *>::iterator iterator = group_list.begin();

	_ZP(EntityList_QueueToGroupsForNPCHealthAA);

	while(iterator != group_list.end())
	{
		(*iterator)->QueueHPPacketsForNPCHealthAA(sender, app);
		iterator++;
	}
}

void EntityList::RaidProcess() {
	list<Raid *>::iterator iterator;
	int32 count = 0;

	if(numclients < 1)
		return;
	_ZP(EntityList_RaidProcess);

	iterator = raid_list.begin();
	while(iterator != raid_list.end())
	{
		count++;
		(*iterator)->Process();
		iterator++;
	}
	if(count == 0)
		net.raid_timer.Disable();//No groups in list, disable until one is added
}

void EntityList::DoorProcess() {
#ifdef IDLE_WHEN_EMPTY
	if(numclients < 1)
		return;
#endif
	_ZP(EntityList_DoorProcess);
	LinkedListIterator<Doors*> iterator(door_list);
	iterator.Reset();
	int32 count=0;
	while(iterator.MoreElements())
	{
		count++;
		if(!iterator.GetData()->Process()){
			iterator.RemoveCurrent();
		}
		else
			iterator.Advance();
	}
	if (count==0)
		net.door_timer.Disable();//No doors in list, disable until one is added
}

void EntityList::ObjectProcess() {
	_ZP(EntityList_ObjectProcess);
	LinkedListIterator<Object*> iterator(object_list);
	iterator.Reset();
	int32 count=0;
	while(iterator.MoreElements())
	{
		count++;
		if(!iterator.GetData()->Process()){
			iterator.RemoveCurrent();
		}
		else
			iterator.Advance();
	}
	if(count==0)
		net.object_timer.Disable();//No objects in list, disable until one is added
}

void EntityList::CorpseProcess() {
	_ZP(EntityList_CorpseProcess);
	
	int32 count=0;

	list<Corpse*> ClonedCorpseList = corpse_list;

	for(list<Corpse*>::iterator itr = ClonedCorpseList.begin(); itr != ClonedCorpseList.end(); itr++) {
		count++;

		if(!(*itr)->Process()) {
			//itr = corpse_list.erase(itr);
			//safe_delete(*itr);

			/*if(itr == corpse_list.end())
				break;*/
			corpse_list.remove(*itr);
			safe_delete(*itr);
		}
	}

	if(count == 0)
		net.corpse_timer.Disable();	//No corpses in list, disable until one is added
}

void EntityList::MobProcess() {
#ifdef IDLE_WHEN_EMPTY
	if(numclients < 1)
		return;
#endif
	_ZP(EntityList_MobProcess);
	list<Mob*> ClonedMobList = mob_list;

	for(list<Mob*>::iterator itr = ClonedMobList.begin(); itr != ClonedMobList.end(); itr++) {
		Mob* mob = *itr;
		
		if(!mob)
			continue;

		if(!mob->Process()){
			if(mob->IsNPC()) {
				entity_list.RemoveNPC(mob->CastToNPC()->GetID());
			}
#ifdef BOTS
			else if(mob->IsBot()) {
				entity_list.RemoveBot(mob->CastToBot()->GetID());
			}
#endif
			else {
#ifdef WIN32
				struct in_addr	in;
				in.s_addr = mob->CastToClient()->GetIP();
				cout << "Dropping client: Process=false, ip=" << inet_ntoa(in) << ", port=" << mob->CastToClient()->GetPort() << endl;
#endif
				zone->StartShutdownTimer();
				Group *g = GetGroupByMob(mob);
				if(g) {
					LogFile->write(EQEMuLog::Error, "About to delete a client still in a group.");
					g->DelMember(mob);
				}
				Raid *r = entity_list.GetRaidByClient(mob->CastToClient());
				if(r) {
					LogFile->write(EQEMuLog::Error, "About to delete a client still in a raid.");
					r->MemberZoned(mob->CastToClient());
				}
				entity_list.RemoveClient(mob->GetID());
			}
			
			mob_list.remove(mob);
			//itr = mob_list.erase(itr);
			safe_delete(mob);
			mob = 0;

			/*if(itr == mob_list.end())
				break;*/
		}
	}
}

void EntityList::BeaconProcess() {
	_ZP(EntityList_BeaconProcess);

	list<Beacon*> ClonedBeaconList = beacon_list;

	for(list<Beacon*>::iterator itr = ClonedBeaconList.begin(); itr != ClonedBeaconList.end(); itr++) {
		Beacon* beacon = *itr;
		
		if(!beacon->Process()) {
			beacon_list.remove(beacon);
			// itr = beacon_list.erase(itr);
			safe_delete(beacon);

			/*if(itr == beacon_list.end())
				break;*/
		}
	}
}


void EntityList::AddGroup(Group* group) {
	if(group == NULL)	//this seems to be happening somehow...
		return;
	
	int32 gid = worldserver.NextGroupID();
	if(gid == 0) {
		LogFile->write(EQEMuLog::Error, "Unable to get new group ID from world server. group is going to be broken.");
		return;
	}
	
	AddGroup(group, gid);
#if EQDEBUG >= 5
	CheckGroupList (__FILE__, __LINE__);
#endif
}


void EntityList::AddGroup(Group* group, int32 gid) {
	group->SetID(gid);
	//group_list.Insert(group);
	group_list.push_back(group);
	if(!net.group_timer.Enabled())
		net.group_timer.Start();
#if EQDEBUG >= 5
	CheckGroupList (__FILE__, __LINE__);
#endif
}

void EntityList::AddRaid(Raid* raid) {
	if(raid == NULL)
		return;
	
	int32 gid = worldserver.NextGroupID();
	if(gid == 0) {
		LogFile->write(EQEMuLog::Error, "Unable to get new group ID from world server. group is going to be broken.");
		return;
	}
	
	AddRaid(raid, gid);
}
void EntityList::AddRaid(Raid* raid, int32 gid) {
	raid->SetID(gid);
	raid_list.push_back(raid);
	if(!net.raid_timer.Enabled())
		net.raid_timer.Start();
}


void EntityList::AddCorpse(Corpse* corpse, int32 in_id) {
	if (corpse == 0)
		return;
	
	if (in_id == 0xFFFFFFFF)
		corpse->SetID(GetZoneEntityID());
	else
		corpse->SetID(in_id);

	corpse->CalcCorpseName();

	// Add to corpse containers
	corpse_list.push_back(corpse);
	corpse_entityid_map.insert(CorpseMapPair(corpse->GetID(), corpse));

	if(!net.corpse_timer.Enabled())
		net.corpse_timer.Start();
}

void EntityList::AddNPC(NPC* npc, bool SendSpawnPacket, bool dontqueue) {
	npc->SetID(GetZoneEntityID());
	
	if (SendSpawnPacket) {
		if (dontqueue) { // aka, SEND IT NOW BITCH!
			EQApplicationPacket* app = new EQApplicationPacket;
			npc->CreateSpawnPacket(app,npc);
			QueueClients(npc, app);
			safe_delete(app);
			parse->Event(EVENT_SPAWN, npc->GetNPCTypeID(), 0, npc, NULL);
		}
		else {
			NewSpawn_Struct* ns = new NewSpawn_Struct;
			memset(ns, 0, sizeof(NewSpawn_Struct));
			npc->FillSpawnStruct(ns, 0);	// Not working on player newspawns, so it's safe to use a ForWho of 0
			AddToSpawnQueue(npc->GetID(), &ns);
			safe_delete(ns);
			parse->Event(EVENT_SPAWN, npc->GetNPCTypeID(), 0, npc, NULL);
		}
	}
	
	npc_list.push_back(npc);
	npc_entityid_map.insert(NPCMapPair(npc->GetID(), npc));
	npc_npcid_map.insert(NPCMapPair(npc->GetNPCTypeID(), npc));

	mob_list.push_back(npc);
	mob_entityid_map.insert(MobMapPair(npc->GetID(), npc));
	mob_name_map.insert(MobMapByNamePair(std::string(npc->GetName()), npc));
}

void EntityList::AddObject(Object* obj, bool SendSpawnPacket) {
	obj->SetID(GetZoneEntityID()); 
	if (SendSpawnPacket) {
		EQApplicationPacket app;
		obj->CreateSpawnPacket(&app);
		#if (EQDEBUG >= 6)
			DumpPacket(&app);
		#endif
		QueueClients(0, &app,false);
	}
	object_list.Insert(obj);
	if(!net.object_timer.Enabled())
		net.object_timer.Start();
}

void EntityList::AddDoor(Doors* door) {
	door->SetEntityID(GetZoneEntityID());
	door_list.Insert(door);
	if(!net.door_timer.Enabled())
		net.door_timer.Start();
}

void EntityList::AddTrap(Trap* trap) {
	trap->SetID(GetZoneEntityID());
	trap_list.Insert(trap);
	if(!net.trap_timer.Enabled())
		net.trap_timer.Start();
}

void EntityList::AddBeacon(Beacon *beacon) {
	beacon->SetID(GetZoneEntityID());
	beacon_list.push_back(beacon);
}

void EntityList::AddToSpawnQueue(int16 entityid, NewSpawn_Struct** ns) {
	int32 count;
	if((count=(client_list.size()))==0)
		return;
	SpawnQueue.Append(*ns);
	NumSpawnsOnQueue++;
	if (tsFirstSpawnOnQueue == 0xFFFFFFFF)
		tsFirstSpawnOnQueue = Timer::GetCurrentTime();
	*ns = 0; // make it so the calling function cant fuck us and delete the data =)
}

void EntityList::CheckSpawnQueue() {
	// Send the stuff if the oldest packet on the queue is older than 50ms -Quagmire
	if (tsFirstSpawnOnQueue != 0xFFFFFFFF && (Timer::GetCurrentTime() - tsFirstSpawnOnQueue) > 50) {
		//if (NumSpawnsOnQueue <= 5) {
			LinkedListIterator<NewSpawn_Struct*> iterator(SpawnQueue);
			EQApplicationPacket* outapp = 0;
			
			iterator.Reset();
			while(iterator.MoreElements()) {
				outapp = new EQApplicationPacket;
				Mob::CreateSpawnPacket(outapp, iterator.GetData());
//				cout << "Sending spawn packet: " << iterator.GetData()->spawn.name << endl;
				QueueClients(0, outapp);
				safe_delete(outapp);
				iterator.RemoveCurrent();
			}
			//sending Spawns like this after zone in causes the client to freeze...
		/*}
		else {
			int32 spawns_per_pack = MAX_SPAWNS_PER_PACKET;
			if(NumSpawnsOnQueue < spawns_per_pack)
				spawns_per_pack = NumSpawnsOnQueue;

			BulkZoneSpawnPacket* bzsp = new BulkZoneSpawnPacket(0, spawns_per_pack);
			LinkedListIterator<NewSpawn_Struct*> iterator(SpawnQueue);
			
			iterator.Reset();
			while(iterator.MoreElements()) {
				bzsp->AddSpawn(iterator.GetData());
				iterator.RemoveCurrent();
			}
			safe_delete(bzsp);
		}*/
		
		tsFirstSpawnOnQueue = 0xFFFFFFFF;
		NumSpawnsOnQueue = 0;
	}
}

Doors* EntityList::FindDoor(int8 door_id)
{
	if (door_id == 0)
		return 0;

	LinkedListIterator<Doors*> iterator(door_list);
	iterator.Reset();

	while(iterator.MoreElements())
	{
		Doors* door=iterator.GetData();
		if (door->GetDoorID() == door_id)
		{
			return door;
		}
		iterator.Advance();
	}
	return 0;
}

Object* EntityList::FindObject(int32 object_id)
{
	LinkedListIterator<Object*> iterator(object_list);
	iterator.Reset();

	while(iterator.MoreElements())
	{
		Object* object=iterator.GetData();
		if (object->GetDBID() == object_id)
		{
			return object;
		}
		iterator.Advance();
	}
	return NULL;
}

Object* EntityList::FindNearbyObject(float x, float y, float z, float radius)
{
	LinkedListIterator<Object*> iterator(object_list);
	iterator.Reset();

	float ox;
	float oy;
	float oz;

	while(iterator.MoreElements())
	{
		Object* object=iterator.GetData();
		
		object->GetLocation(&ox, &oy, &oz);

		ox = (x < ox) ? (ox - x) : (x - ox);
		oy = (y < oy) ? (oy - y) : (y - oy);
		oz = (z < oz) ? (oz - z) : (z - oz);

		if ((ox <= radius) && (oy <= radius) && (oz <= radius))
		{
			return object;
		}
		iterator.Advance();
	}


	return NULL;
}


bool EntityList::MakeDoorSpawnPacket(EQApplicationPacket* app)
{
	int32 count = door_list.Count();
	if( !count || count>500)
		return false;
	int32 length = count * sizeof(Door_Struct);
	uchar* packet_buffer = new uchar[length];
	memset(packet_buffer, 0, length);
	uchar* ptr = packet_buffer;
	Doors *door;
	LinkedListIterator<Doors*> iterator(door_list);
	Door_Struct nd;
	iterator.Reset();
	while(iterator.MoreElements()){
		door = iterator.GetData();
		if(door && strlen(door->GetDoorName()) > 3){
			memset(&nd, 0, sizeof(nd));
			memcpy(nd.name, door->GetDoorName(), 32);
			nd.xPos = door->GetX();
			nd.yPos = door->GetY();
			nd.zPos = door->GetZ();
			nd.heading = door->GetHeading();
			nd.incline = door->GetIncline();
			nd.size = door->GetSize();
			nd.doorId = door->GetDoorID();				
			nd.opentype = door->GetOpenType();
			nd.state_at_spawn = door->GetInvertState() ? !door->IsDoorOpen() : door->IsDoorOpen();
			nd.invert_state = door->GetInvertState();
			nd.door_param = door->GetDoorParam();	
			memcpy(ptr, &nd, sizeof(nd));
			ptr+=sizeof(nd);
			*(ptr-1)=0x01;
			*(ptr-3)=0x01;
		}
		iterator.Advance();
	}

#if EQDEBUG >= 5
//	LogFile->write(EQEMuLog::Debug, "MakeDoorPacket() packet length:%i qty:%i ", length, qty);
#endif
	app->SetOpcode(OP_SpawnDoor);
	app->size = length;
	app->pBuffer = packet_buffer;
	return true;	
}

Entity* EntityList::GetEntityMob(int32 id) {
	Entity* Result = 0;

	if(!mob_entityid_map.empty()) {
		MobMap::iterator mobItr = mob_entityid_map.begin();

		mobItr = mob_entityid_map.find(id);

		if(mobItr != mob_entityid_map.end())
			Result = mobItr->second;
	}


	return Result;
}

Entity* EntityList::GetEntityMob(const char *name) {
	Entity* Result = 0;
	std::string firstName(name);

	if(!firstName.empty() && !mob_list.empty()) {
		MobNameMap::iterator mobItr = mob_name_map.begin();
	
		mobItr = mob_name_map.find(firstName);

		if(mobItr != mob_name_map.end())
			Result = mobItr->second;
	}

	return Result;

	/*if (name == 0)
		return 0;

	LinkedListIterator<Mob*> iterator(mob_list);
	
	for(iterator.Reset(); iterator.MoreElements(); iterator.Advance())
	{
		if (strcasecmp(iterator.GetData()->GetName(), name) == 0)
		{
			return iterator.GetData();
		}
	}
	return 0;*/
}

Entity* EntityList::GetEntityDoor(int32 id) {
	LinkedListIterator<Doors*> iterator(door_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->GetID() == id)
		{
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return 0;
}

Entity* EntityList::GetEntityCorpse(int32 id) {
	Entity* Result = 0;

	if(!corpse_entityid_map.empty() && id > 0) {
		CorpseMap::iterator corpseItr = corpse_entityid_map.begin();

		corpseItr = corpse_entityid_map.find(id);

		if(corpseItr != corpse_entityid_map.end())
			Result = corpseItr->second;
	}


	return Result;
}

Entity* EntityList::GetEntityCorpse(const char *name) {
	Entity* Result = 0;
	std::string tempName(name);

	if(!tempName.empty()) {
		list<Corpse*> ClonedCorpseList = corpse_list;

		for(list<Corpse*>::iterator itr = ClonedCorpseList.begin(); itr != ClonedCorpseList.end(); itr++) {
			Corpse* c = *itr;

			if(c) {
				if (strcmp(name, tempName.c_str()) == 0) {
					Result = c;
					break;
				}
			}
		}
	}

	return Result;
}

Entity* EntityList::GetEntityTrap(int32 id){
	LinkedListIterator<Trap*> iterator(trap_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->GetID() == id)
		{
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return 0;
}

Entity* EntityList::GetEntityObject(int32 id){
	LinkedListIterator<Object*> iterator(object_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->GetID() == id)
		{
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return 0;
}
/*
Entity* EntityList::GetEntityGroup(int16 id){
	LinkedListIterator<Group*> iterator(group_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->GetID() == id)
		{
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return 0;
}
*/
Entity* EntityList::GetEntityBeacon(int32 id) {
	Entity* Result = 0;

	if(id > 0 && !beacon_entityid_map.empty()) {
		BeaconMap::iterator beaconItr = beacon_entityid_map.begin();

		beaconItr = beacon_entityid_map.find(id);

		if(beaconItr != beacon_entityid_map.end())
			Result = beaconItr->second;
	}

	return Result;
}

Entity* EntityList::GetByEntityID(int32 get_id) {
	Entity* ent=0;

	if((ent=entity_list.GetEntityMob(get_id))!=0)
		return ent;
	else if((ent=entity_list.GetEntityDoor(get_id))!=0)
		return ent;
	else if((ent=entity_list.GetEntityCorpse(get_id))!=0)
		return ent;
	//	else if((ent=entity_list.GetEntityGroup(get_id))!=0)
	//		return ent;
	else if((ent=entity_list.GetEntityObject(get_id))!=0)
		return ent;
	else if((ent=entity_list.GetEntityTrap(get_id))!=0)
		return ent;
	else if((ent=entity_list.GetEntityBeacon(get_id))!=0)
		return ent;
	else
		return 0;
}

Mob* EntityList::GetMob(int32 get_id)
{
	Entity* ent=0;

	if (get_id == 0)
		return 0;

	if((ent=entity_list.GetEntityMob(get_id))!=0)
		return ent->CastToMob();
	else if((ent=entity_list.GetEntityCorpse(get_id))!=0)
		return ent->CastToMob();

	return 0;
}

Mob* EntityList::GetMob(const char* name)
{
	Entity* ent=0;

	if (name == 0)
		return 0;

	if((ent=entity_list.GetEntityMob(name))!=0)
		return ent->CastToMob();
	else if((ent=entity_list.GetEntityCorpse(name))!=0)
		return ent->CastToMob();

	return 0;
}

Mob* EntityList::GetMobByNpcTypeID(int32 get_id) {
	Mob* Result = 0;

	if(get_id > 0 && !npc_npcid_map.empty()) {
		NPCMap::iterator npcItr = npc_npcid_map.begin();

		npcItr = npc_npcid_map.find(get_id);

		if(npcItr != npc_npcid_map.end())
			Result = npcItr->second;
	}

	return Result;
}

//int16 EntityList::GetFreeID()
//{
//	if(last_insert_id > 1500)
//		last_insert_id = 0;
//	int16 getid=last_insert_id;
//	while(1)
//	{
//		getid++;
//		if (GetID(getid) == 0)
//		{
//			last_insert_id = getid;
//			return getid;
//		}
//	}
//}

// if no language skill is specified, sent with 100 skill
void EntityList::ChannelMessage(Mob* from, int8 chan_num, int8 language, const char* message, ...) {
	ChannelMessage(from, chan_num, language, 100, message);
}

void EntityList::ChannelMessage(Mob* from, int8 chan_num, int8 language, int8 lang_skill, const char* message, ...) {
	va_list argptr;
	char buffer[4096];
	
	va_start(argptr, message);
	vsnprintf(buffer, 4096, message, argptr);
	va_end(argptr);

	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* client = *itr;

		if(client) {
			eqFilterType filter = FilterNone;
			
			if(chan_num==3)//shout
				filter=FILTER_SHOUT;
			else if(chan_num==4) //auction
				filter=FILTER_AUCTION;

			if (chan_num != 8 || client->Dist(*from) < 200) // Only say is limited in range
			{
				if(filter==FilterNone || client->GetFilter(filter)!=FilterHide)
					client->ChannelMessageSend(from->GetName(), 0, chan_num, language, lang_skill, buffer);
			}
		}
	}
}

void EntityList::ChannelMessageSend(Mob* to, int8 chan_num, int8 language, const char* message, ...) {
	va_list argptr;
	char buffer[4096];
	va_start(argptr, message);
	vsnprintf(buffer, 4096, message, argptr);
	va_end(argptr);

	if(!client_entityid_map.empty() && to) {
		ClientMap::iterator clientItr = client_entityid_map.begin();

		clientItr = client_entityid_map.find(to->GetID());

		if(clientItr != client_entityid_map.end()) {
			Client* client = clientItr->second;

			if(client)
				client->ChannelMessageSend(0, 0, chan_num, language, buffer);
		}
	}
}

void EntityList::SendZoneSpawns(Client* client) {
	list<Mob*> ClonedMobList = mob_list;

	for(list<Mob*>::iterator itr = ClonedMobList.begin(); itr != ClonedMobList.end(); itr++) {
		Mob* ent = *itr;

		if(ent) {
			if(ent->InZone() || (ent->IsClient() && !ent->CastToClient()->GMHideMe(client))) {
				EQApplicationPacket* app = new EQApplicationPacket();
				ent->CreateSpawnPacket(app);	// TODO: Use zonespawns opcode instead
				client->QueuePacket(app, true, Client::CLIENT_CONNECTED);
				safe_delete(app);
			}
		}
	}
}

void EntityList::SendZoneSpawnsBulk(Client* client) {
	if(!mob_list.empty()) {
		int32 maxspawns = 100;

		if(maxspawns > mob_list.size())
			maxspawns = mob_list.size();

		BulkZoneSpawnPacket* bzsp = new BulkZoneSpawnPacket(client, maxspawns);

		list<Mob*> ClonedMobList = mob_list;

		for(list<Mob*>::iterator itr = ClonedMobList.begin(); itr != ClonedMobList.end(); itr++) {
			Mob* spawn = *itr;

			if(spawn) {
				if(spawn->InZone()) {
					NewSpawn_Struct ns;

					if(spawn->IsClient() && spawn->CastToClient()->GMHideMe(client))
						continue;

					memset(&ns, 0, sizeof(NewSpawn_Struct));
					spawn->FillSpawnStruct(&ns, client);
					bzsp->AddSpawn(&ns);
				}
			}
		}

		safe_delete(bzsp);
	}
}

//this is a hack to handle a broken spawn struct
void EntityList::SendZonePVPUpdates(Client *to) {
	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* c = *itr;

		if(c) {
			if(c->GetPVP())
				c->SendAppearancePacket(AT_PVP, c->GetPVP(), true, false, to);
		}
	}
}

void EntityList::SendZoneCorpses(Client* client) {
	if(client) {
		list<Corpse*> ClonedCorpseList = corpse_list;

		for(list<Corpse*>::iterator itr = ClonedCorpseList.begin(); itr != ClonedCorpseList.end(); itr++) {
			Corpse* ent = *itr;

			if(ent) {
				EQApplicationPacket* app = new EQApplicationPacket;
				ent->CreateSpawnPacket(app);
				client->QueuePacket(app, true, Client::CLIENT_CONNECTED);
				safe_delete(app);
			}
		}
	}
}

void EntityList::SendZoneCorpsesBulk(Client* client) {
	//float rate = client->Connection()->GetDataRate();
	
	NewSpawn_Struct ns;
	int32 maxspawns=100;

	//rate = rate > 1.0 ? (rate < 10.0 ? rate : 10.0) : 1.0;
	//maxspawns = (int32)rate * SPAWNS_PER_POINT_DATARATE; // FYI > 10240 entities will cause BulkZoneSpawnPacket to throw exception

	if(client) {
		BulkZoneSpawnPacket* bzsp = new BulkZoneSpawnPacket(client, maxspawns);

		list<Corpse*> ClonedCorpseList = corpse_list;

		for(list<Corpse*>::iterator itr = ClonedCorpseList.begin(); itr != ClonedCorpseList.end(); itr++) {
			Corpse* spawn = *itr;

			if(spawn) {
				if(spawn->InZone()) {
					memset(&ns, 0, sizeof(NewSpawn_Struct));
					spawn->FillSpawnStruct(&ns, client);
					bzsp->AddSpawn(&ns);
				}
			}
		}

		safe_delete(bzsp);
	}
}

void EntityList::SendZoneObjects(Client* client)
{
	LinkedListIterator<Object*> iterator(object_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		EQApplicationPacket *app = new EQApplicationPacket;
		iterator.GetData()->CreateSpawnPacket(app);
		client->FastQueuePacket(&app);
		iterator.Advance();
	}
}

void EntityList::Save() {
	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* c = *itr;

		if(c) {
			c->Save();
		}
	}
}

void EntityList::ReplaceWithTarget(Mob* pOldMob, Mob* pNewTarget) {
	if(pOldMob && pNewTarget) {
		list<Mob*> ClonedMobList = mob_list;

		for(list<Mob*>::iterator itr = ClonedMobList.begin(); itr != ClonedMobList.end(); itr++) {
			Mob* m = *itr;

			if(m) {
				if(m->IsAIControlled()) {
					// replace the old mob with the new one
					if(m->RemoveFromHateList(pOldMob))
						m->AddToHateList(pNewTarget, 1, 0);
				}
			}
		}
	}
}

void EntityList::RemoveFromTargets(Mob* mob) {
	if(mob) {
		list<Mob*> ClonedMobList = mob_list;

		for(list<Mob*>::iterator itr = ClonedMobList.begin(); itr != ClonedMobList.end(); itr++) {
			Mob* m = *itr;

			if(m) {
				m->RemoveFromHateList(mob);
			}
		}
	}
}

void EntityList::QueueClientsByTarget(Mob* sender, const EQApplicationPacket* app, bool iSendToSender, Mob* SkipThisMob, bool ackreq) {
	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* c = *itr;

		if(c) {
			if ((iSendToSender || (c != sender && (c->GetTarget() == sender || (c->GetTarget() && c->GetTarget()->GetTarget() && c->GetTarget()->GetTarget() == sender)))) && c != SkipThisMob) {
					c->QueuePacket(app, ackreq);
			}
		}
	}
}

void EntityList::QueueCloseClients(Mob* sender, const EQApplicationPacket* app, bool ignore_sender, float dist, Mob* SkipThisMob, bool ackreq, eqFilterType filter) {
	if (sender == NULL) {
		QueueClients(sender, app, ignore_sender);
		return;
	}
	if(dist <= 0) {
		dist = 600;
	}
	float dist2 = dist * dist; //pow(dist, 2);
	
	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* ent = *itr;

		if(ent) {
			if ((!ignore_sender || ent != sender) && (ent != SkipThisMob)) {
				eqFilterMode filter2 = ent->GetFilter(filter);
				if(ent->Connected() && 
					(  filter==FilterNone 
					||  filter2 == FilterShow 
					|| (filter2 == FilterShowGroupOnly && (sender == ent || 
					(ent->GetGroup() && ent->GetGroup()->IsGroupMember(sender))))
					|| (filter2 == FilterShowSelfOnly && ent==sender))
					&& (ent->DistNoRoot(*sender) <= dist2)) {
						ent->QueuePacket(app, ackreq, Client::CLIENT_CONNECTED);
				}
			}
		}
	}
}

//sender can be null
void EntityList::QueueClients(Mob* sender, const EQApplicationPacket* app, bool ignore_sender, bool ackreq) {
	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* ent = *itr;

		if(ent) {
			if ((!ignore_sender || ent != sender)) {
				ent->QueuePacket(app, ackreq, Client::CLIENT_CONNECTED);
			}
		}
	}
}

/*
rewrite of all the queue close methods to use the update manager
void EntityList::FilterQueueCloseClients(int8 filter, int8 required, Mob* sender, const EQApplicationPacket* app, bool ignore_sender, float dist, Mob* SkipThisMob, bool ackreq){
	if(dist <= 0) {
		dist = 600;
	}

#ifdef PACKET_UPDATE_MANAGER
	EQApplicationPacket* tmp_app = app->Copy();
#else
	float dist2 = dist * dist; //pow(dist, 2);
#endif
	
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();
	while(iterator.MoreElements()) {

		Client* ent = iterator.GetData();
		int8 filterval=ent->GetFilter(filter);
		if(required==0)
			required=1;
		if(filterval==required){
			if ((!ignore_sender || ent != sender) && (ent != SkipThisMob)
		  		) {
#ifdef PACKET_UPDATE_MANAGER
				if(ent->Connected()) {
					ent->GetUpdateManager()->QueuePacket(tmp_app, ackreq, sender, ent->DistNoRoot(*sender));
				}
#else
				if(ent->Connected() &&  (ent->DistNoRoot(*sender) <= dist2 || dist == 0)) {
						ent->QueuePacket(app, ackreq);
				}
#endif
			}
		}
		iterator.Advance();
	}
#ifdef PACKET_UPDATE_MANAGER
	EQApplicationPacket::PacketUsed(&tmp_app);
#endif
}

void EntityList::QueueCloseClients(Mob* sender, const EQApplicationPacket* app, bool ignore_sender, float dist, Mob* SkipThisMob, bool ackreq,int8 filter) {
	if (sender == 0) {
		QueueClients(sender, app, ignore_sender);
		return;
	}
	if(dist <= 0) {
		dist = 600;
	}
#ifdef PACKET_UPDATE_MANAGER
	EQApplicationPacket* tmp_app = app->Copy();
#else
	float dist2 = dist * dist; //pow(dist, 2);
#endif

	
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();
	while(iterator.MoreElements()) {

		Client* ent = iterator.GetData();

		if ((!ignore_sender || ent != sender) && (ent != SkipThisMob)) {
			int8 filter2=ent->GetFilter(filter);
			if(ent->Connected() && 
				(filter==0 || (filter2==1 || 
				(filter2==99 && entity_list.GetGroupByClient(ent)!=0 && 
				 entity_list.GetGroupByClient(ent)->IsGroupMember(sender))
				 || (filter2==98 && ent==sender)))
#ifdef PACKET_UPDATE_MANAGER
			) {
				ent->GetUpdateManager()->QueuePacket(tmp_app, ackreq, sender, ent->DistNoRoot(*sender));
			}
#else
			&& (ent->DistNoRoot(*sender) <= dist2 || dist == 0)) {
				ent->QueuePacket(app, ackreq, Client::CLIENT_CONNECTED);
			}
#endif
		}
		iterator.Advance();
	}
#ifdef PACKET_UPDATE_MANAGER
	EQApplicationPacket::PacketUsed(&tmp_app);
#endif
}

void EntityList::QueueClients(Mob* sender, const EQApplicationPacket* app, bool ignore_sender, bool ackreq) {
	LinkedListIterator<Client*> iterator(client_list);
	
#ifdef PACKET_UPDATE_MANAGER
	EQApplicationPacket* tmp_app = app->Copy();
#endif
	
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Client* ent = iterator.GetData();

		if ((!ignore_sender || ent != sender))
		{
#ifdef PACKET_UPDATE_MANAGER
			ent->GetUpdateManager()->QueuePacket(tmp_app, ackreq, sender, ent->DistNoRoot(*sender));
#else
			ent->QueuePacket(app, ackreq, Client::CLIENT_CONNECTED);
#endif
		}
		iterator.Advance();
	}
#ifdef PACKET_UPDATE_MANAGER
	EQApplicationPacket::PacketUsed(&tmp_app);
#endif
}
*/

/*
void EntityList::QueueManaged(Mob* sender, const EQApplicationPacket* app, bool ignore_sender, bool ackreq) {
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Client* ent = iterator.GetData();

		if ((!ignore_sender || ent != sender))
		{
			ent->QueuePacket(app, ackreq, Client::CLIENT_CONNECTED);
		}
		iterator.Advance();
	}
}*/

void EntityList::QueueManaged(Mob* sender, const EQApplicationPacket* app, bool ignore_sender, bool ackreq) {
#ifdef PACKET_UPDATE_MANAGER
	EQApplicationPacket* tmp_app = app->Copy();
#endif

	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* ent = *itr;

		if(ent) {
			if ((!ignore_sender || ent != sender)) {
#ifdef PACKET_UPDATE_MANAGER
				ent->GetUpdateManager()->QueuePacket(tmp_app, ackreq, sender, ent->DistNoRoot(*sender));
#else
				ent->QueuePacket(app, ackreq, Client::CLIENT_CONNECTED);
#endif
			}
		}
	}

#ifdef PACKET_UPDATE_MANAGER
	EQApplicationPacket::PacketUsed(&tmp_app);
#endif
}

void EntityList::QueueClientsStatus(Mob* sender, const EQApplicationPacket* app, bool ignore_sender, int8 minstatus, int8 maxstatus) {
	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* ent = *itr;

		if(ent) {
			if ((!ignore_sender || ent != sender) && (ent->Admin() >= minstatus && ent->Admin() <= maxstatus)) {
				ent->QueuePacket(app);
			}
		}
	}
}

void EntityList::DuelMessage(Mob* winner, Mob* loser, bool flee) {
	if(winner && loser) {
		list<Client*> ClonedClientList = client_list;

		for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
			Client* cur = *itr;

			if(cur) {
				//might want some sort of distance check in here?
				if (cur != winner && cur != loser) {
					if (flee)
						cur->Message_StringID(15, DUEL_FLED, winner->GetName(),loser->GetName(),loser->GetName());
					else
						cur->Message_StringID(15, DUEL_FINISHED, winner->GetName(),loser->GetName());
				}
			}
		}
	}
}

Client* EntityList::GetClientByName(const char *checkname) {
	Client* Result = 0;
	std::string tempCheckName(checkname);

	if(!tempCheckName.empty() && !client_name_map.empty()) {
		ClientNameMap::iterator itr = client_name_map.begin();

		itr = client_name_map.find(tempCheckName);

		if(itr != client_name_map.end())
			Result = itr->second;
	}

	return Result;
}

Client* EntityList::GetClientByCharID(int32 iCharID) {
	Client* Result = 0;

	if(iCharID > 0 && !client_name_map.empty()) {
		ClientMap::iterator itr = client_characterid_map.begin();

		itr = client_characterid_map.find(iCharID);

		if(itr != client_characterid_map.end())
			Result = itr->second;
	}

	return Result;
}

Client* EntityList::GetClientByWID(int32 iWID) {
	Client* Result = 0;

	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* cur = *itr;

		if(cur) {
			if (cur->GetWID() == iWID) {
				Result = cur;
				break;
			}
		}
	}

	return Result;
}

Client* EntityList::GetRandomClient(float x, float y, float z, float Distance, Client* ExcludeClient) {
	Client* Result = 0;
	vector<Client*> ClientsInRange;

	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* cur = *itr;

		if(cur) {
			if((cur != ExcludeClient) && (cur->DistNoRoot(x, y, z) <= Distance))
				ClientsInRange.push_back(cur);
		}
	}

	if(!ClientsInRange.empty())
		Result = ClientsInRange[MakeRandomInt(0, ClientsInRange.size() - 1)];

	return Result;
}

Corpse*	EntityList::GetCorpseByOwner(Client* client) {
	Corpse* Result = 0;

	if(client) {
		if(!corpse_list.empty()) {
			list<Corpse*> ClonedCorpseList = corpse_list;

			for(list<Corpse*>::iterator itr = ClonedCorpseList.begin(); itr != ClonedCorpseList.end(); itr++) {
				Corpse* c = *itr;

				if(c) {
					if (strcmp(c->GetName(), client->GetName()) == 0) {
						Result = c;
						break;
					}
				}
			}
		}
	}

	return Result;
}

Corpse* EntityList::GetCorpseByID(int32 id) {
	Corpse* Result = 0;

	if(id > 0) {
		if(!corpse_entityid_map.empty()) {
			CorpseMap::iterator itr = corpse_entityid_map.begin();

			itr = corpse_entityid_map.find(id);

			if(itr != corpse_entityid_map.end())
				Result = itr->second;
		}
	}

	return Result;
}

Corpse* EntityList::GetCorpseByDBID(int32 dbid) {
	Corpse* Result = 0;

	if(dbid > 0) {
		list<Corpse*> ClonedCorpseList = corpse_list;

		for(list<Corpse*>::iterator itr = ClonedCorpseList.begin(); itr != ClonedCorpseList.end(); itr++) {
			Corpse* ent = *itr;

			if(ent) {
				if(ent->GetDBID() == dbid) {
					Result = ent;
					break;
				}
			}
		}
	}

	return Result;
}

Corpse* EntityList::GetCorpseByName(char* name) {
	Corpse* Result = 0;
	std::string tempName(name);

	if(!tempName.empty()) {
		if(!corpse_list.empty()) {
			list<Corpse*> ClonedCorpseList = corpse_list;

			for(list<Corpse*>::iterator itr = ClonedCorpseList.begin(); itr != ClonedCorpseList.end(); itr++) {
				Corpse* c = *itr;

				if(c) {
					if (strcmp(name, tempName.c_str()) == 0) {
						Result = c;
						break;
					}
				}
			}
		}
	}

	return Result;
}

void EntityList::RemoveAllCorpsesByCharID(int32 charid) {
	if(charid > 0) {
		list<Corpse*> ClonedCorpseList = corpse_list;

		for(list<Corpse*>::iterator itr = ClonedCorpseList.begin(); itr != ClonedCorpseList.end(); itr++) {
			Corpse* c = *itr;

			if(c) {
				if (c->GetCharID() == charid) {
					RemoveCorpse(c);
				}
			}
		}
	}
}

int EntityList::RezzAllCorpsesByCharID(int32 charid) {
	int RezzExp = 0;

	if(charid > 0 && !corpse_list.empty()) {
		list<Corpse*> ClonedCorpseList = corpse_list;

		for(list<Corpse*>::iterator itr = ClonedCorpseList.begin(); itr != ClonedCorpseList.end(); itr++) {
			Corpse* c = *itr;

			if(c) {
				if (c->GetCharID() == charid) {
					RezzExp += c->GetRezzExp();
					c->Rezzed(true);
					c->CompleteRezz();
				}
			}
		}
	}

	return RezzExp;
}

Group* EntityList::GetGroupByMob(Mob* mob) 
{ 
	list<Group *>::iterator iterator;

	iterator = group_list.begin();

	while(iterator != group_list.end())
	{ 
		if ((*iterator)->IsGroupMember(mob)) {
			return *iterator;
		}
		iterator++;
	} 
#if EQDEBUG >= 5
	CheckGroupList (__FILE__, __LINE__);
#endif
	return 0; 
}

Group* EntityList::GetGroupByLeaderName(char* leader){
	list<Group *>::iterator iterator;

	iterator = group_list.begin();

	while(iterator != group_list.end())
	{ 
		if (!strcmp((*iterator)->GetLeaderName(), leader)) {
			return *iterator;
		}
		iterator++; 
	}
#if EQDEBUG >= 5
	CheckGroupList (__FILE__, __LINE__);
#endif
	return 0;
}
Group* EntityList::GetGroupByID(int32 group_id){
	list<Group *>::iterator iterator;

	iterator = group_list.begin();

	while(iterator != group_list.end())
	{ 
		if ((*iterator)->GetID() == group_id) {
			return *iterator;
		}
		iterator++;
	}
#if EQDEBUG >= 5
	CheckGroupList (__FILE__, __LINE__);
#endif
	return 0;
}
Group* EntityList::GetGroupByClient(Client* client) 
{ 
	list <Group *>::iterator iterator;

	iterator = group_list.begin();

	while(iterator != group_list.end())
	{ 
		if ((*iterator)->IsGroupMember(client->CastToMob())) {
			return *iterator;
		}
		iterator++; 
	} 
#if EQDEBUG >= 5
	CheckGroupList (__FILE__, __LINE__);
#endif
	return 0; 
} 

Raid* EntityList::GetRaidByLeaderName(const char *leader){
	list<Raid *>::iterator iterator;

	iterator = raid_list.begin();

	while(iterator != raid_list.end())
	{ 
		if((*iterator)->GetLeader()){
			if(strcmp((*iterator)->GetLeader()->GetName(), leader) == 0){
				return *iterator;
	} 
}
		iterator++;
	} 
	return 0;
}
Raid* EntityList::GetRaidByID(int32 id){
	list<Raid *>::iterator iterator;

	iterator = raid_list.begin();

	while(iterator != raid_list.end())
	{ 
		if ((*iterator)->GetID() == id) {
			return *iterator;
		}
		iterator++;
	} 
	return 0;
}

Raid* EntityList::GetRaidByClient(Client* client) 
{ 
	list<Raid *>::iterator iterator;

	iterator = raid_list.begin();

	while(iterator != raid_list.end())
	{ 
		for(int x = 0; x < MAX_RAID_MEMBERS; x++)
		{
			if((*iterator)->members[x].member){
				if((*iterator)->members[x].member == client)
					return *iterator;
			}
		}
		iterator++;
	} 
	return 0; 
}

Raid* EntityList::GetRaidByMob(Mob* mob) { 
	list<Raid *>::iterator iterator;

	iterator = raid_list.begin();

	while(iterator != raid_list.end())
	{ 
		for(int x = 0; x < MAX_RAID_MEMBERS; x++)
		{
			// TODO: Implement support for Mob objects in Raid class
			/*if((*iterator)->members[x].member){
				if((*iterator)->members[x].member == mob)
					return *iterator;
			}*/
		}
		iterator++;
	} 
	return 0; 
} 

Client* EntityList::GetClientByAccID(int32 accid) { 
	Client* Result = 0;

	if(accid > 0 && !client_list.empty()) {
		list<Client*> ClonedClientList = client_list;

		for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
			Client* c = *itr;

			if(c) {
				if (c->AccountID() == accid) {
					Result = c;
					break;
				}
			}
		}
	}

	return Result;
}

Client* EntityList::GetClientByID(int32 id) { 
	Client* Result = 0;

	if(id > 0 && !client_entityid_map.empty()) {
		ClientMap::iterator itr = client_entityid_map.begin();

		itr = client_entityid_map.find(id);

		if(itr != client_entityid_map.end())
			Result = itr->second;
	}

	return Result;
} 

void EntityList::ChannelMessageFromWorld(const char* from, const char* to, int8 chan_num, int32 guild_id, int8 language, const char* message) {
	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* client = *itr;

		if(client) {
			if(chan_num == 0) {
				if(!client->IsInGuild(guild_id))
					continue;
				if(!guild_mgr.CheckPermission(guild_id, client->GuildRank(), GUILD_HEAR))
					continue;
				if(client->GetFilter(FILTER_GUILDSAY) == FilterHide)
					continue;
			}
			else if(chan_num == 5) {
				if(client->GetFilter(FILTER_OOC) == FilterHide)
					continue;
			}

			client->ChannelMessageSend(from, to, chan_num, language, message);
		}
	}
}

void EntityList::Message(int32 to_guilddbid, int32 type, const char* message, ...) {
	va_list argptr;
	char buffer[4096];

	va_start(argptr, message);
	vsnprintf(buffer, 4096, message, argptr);
	va_end(argptr);

	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* client = *itr;

		if(client) {
			if (to_guilddbid == 0 || client->IsInGuild(to_guilddbid))
				client->Message(type, buffer);
		}
	}
}

void EntityList::QueueClientsGuild(Mob* sender, const EQApplicationPacket* app, bool ignore_sender, int32 guild_id){
	// TODO: This method does not use the "sender" or the "ignore_sender" parameter values
	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* client = *itr;

		if(client) {
			if (client->IsInGuild(guild_id))
				client->QueuePacket(app);
		}
	}
}

void EntityList::MessageStatus(int32 to_guild_id, int to_minstatus, int32 type, const char* message, ...) {
	va_list argptr;
	char buffer[4096];

	va_start(argptr, message);
	vsnprintf(buffer, 4096, message, argptr);
	va_end(argptr);

	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* client = *itr;

		if(client) {
			if ((to_guild_id == 0 || client->IsInGuild(to_guild_id)) && client->Admin() >= to_minstatus)
				client->Message(type, buffer);
		}
	}
}

// works much like MessageClose, but with formatted strings
void EntityList::MessageClose_StringID(Mob *sender, bool skipsender, float dist, int32 type, int32 string_id, const char* message1,const char* message2,const char* message3,const char* message4,const char* message5,const char* message6,const char* message7,const char* message8,const char* message9) {
	float dist2 = dist * dist;

	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* c = *itr;

		if(c) {
			if(c && c->DistNoRoot(*sender) <= dist2 && (!skipsender || c != sender))
				c->Message_StringID(type, string_id, message1, message2, message3, message4, message5, message6, message7, message8, message9);
		}
	}
}

void EntityList::Message_StringID(Mob *sender, bool skipsender, int32 type, int32 string_id, const char* message1,const char* message2,const char* message3,const char* message4,const char* message5,const char* message6,const char* message7,const char* message8,const char* message9) {
	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* c = *itr;

		if(c) {
			if(c && (!skipsender || c != sender))
				c->Message_StringID(type, string_id, message1, message2, message3, message4, message5, message6, message7, message8, message9);
		}
	}
}

void EntityList::MessageClose(Mob* sender, bool skipsender, float dist, int32 type, const char* message, ...) {
	va_list argptr;
	char buffer[4096];

	va_start(argptr, message);
	vsnprintf(buffer, 4095, message, argptr);
	va_end(argptr);

	float dist2 = dist * dist;

	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* c = *itr;

		if(c) {
			if (c->DistNoRoot(*sender) <= dist2 && (!skipsender || c != sender)) {
				c->Message(type, buffer);
			}
		}
	}
}

#if 0	// solar: old code, see Mob functions: Say, Say_StringID, Shout, Emote
void EntityList::NPCMessage(Mob* sender, bool skipsender, float dist, int32 type, const char* message, ...) { 
   va_list argptr; 
   char buffer[4096]; 
   char *findzero; 
    int  stripzero; 
   va_start(argptr, message); 
   vsnprintf(buffer, 4095, message, argptr); 
   va_end(argptr); 
    findzero = strstr( buffer, "0" ); 
    stripzero = (int)(findzero - buffer + 2); 
   if (stripzero > 2 && stripzero<4096) //Incase its not an npc, you dont want to crash the zone 
      strncpy(buffer + stripzero," ",1); 
   float dist2 = dist * dist; 
   char *tmp = new char[strlen(buffer)];
   memset(tmp,0x0,sizeof(tmp));
   LinkedListIterator<Client*> iterator(client_list); 
   if (dist2==0) {
      iterator.Reset(); 
      while(iterator.MoreElements()) 
      {  
		Client* client = iterator.GetData()->CastToClient(); 
		client->Message(type, buffer); 
		iterator.Advance(); 
      } 
   } 
   else { 
      iterator.Reset(); 
      while(iterator.MoreElements()) 
      { 
         if (iterator.GetData()->DistNoRoot(*sender) <= dist2 && (!skipsender || iterator.GetData() != sender)) { 
            iterator.GetData()->Message(type, buffer); 
         } 
         iterator.Advance(); 
      } 
   }

   if (sender->GetTarget() && sender->GetTarget()->IsNPC() && buffer)
   {
	   strcpy(tmp,strstr(buffer,"says"));
	   tmp[strlen(tmp) - 1] = '\0';
	   while (*tmp)
	   {
    	   tmp++;
		   if (*tmp == '\'') { tmp++; break; }
	   }
	   if (tmp)
		parse->Event(EVENT_SAY, sender->GetTarget()->GetNPCTypeID(), tmp, sender->GetTarget(), sender);
   }

} 
#endif

void EntityList::RemoveAllMobs() {
	list<Mob*> ClonedMobList = mob_list;

	for(list<Mob*>::iterator itr = ClonedMobList.begin(); itr != ClonedMobList.end(); itr++) {
		RemoveMob(*itr);
	}
}

void EntityList::RemoveAllClients() {
	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		RemoveClient(*itr);
	}
}

void EntityList::RemoveAllNPCs() {
	list<NPC*> ClonedNPCList = npc_list;

	for(list<NPC*>::iterator itr = ClonedNPCList.begin(); itr != ClonedNPCList.end(); itr++) {
		RemoveNPC(*itr);
	}

	npc_limit_list.clear();
}

void EntityList::RemoveAllGroups(){
	while (group_list.size())
		group_list.pop_front();
#if EQDEBUG >= 5
	CheckGroupList (__FILE__, __LINE__);
#endif
}

void EntityList::RemoveAllRaids(){
	while (raid_list.size())
		raid_list.pop_front();
}

void EntityList::RemoveAllDoors(){
	LinkedListIterator<Doors*> iterator(door_list);
	iterator.Reset();
	while(iterator.MoreElements())
		iterator.RemoveCurrent();
}

void EntityList::RemoveAllCorpses() {
	list<Corpse*> ClonedCorpseList = corpse_list;

	for(list<Corpse*>::iterator itr = ClonedCorpseList.begin(); itr != ClonedCorpseList.end(); itr++) {
		RemoveCorpse(*itr);
	}
}

void EntityList::RemoveAllObjects(){
	LinkedListIterator<Object*> iterator(object_list);
	iterator.Reset();
	while(iterator.MoreElements())
		iterator.RemoveCurrent();
}
void EntityList::RemoveAllTraps(){
	LinkedListIterator<Trap*> iterator(trap_list);
	iterator.Reset();
	while(iterator.MoreElements())
		iterator.RemoveCurrent();
}

bool EntityList::RemoveMob(int32 delete_id) {
	bool Result = false;

	if(delete_id > 0) {
		MobMap::iterator mobItr = mob_entityid_map.begin();

		mobItr = mob_entityid_map.find(delete_id);

		if(mobItr != mob_entityid_map.end()) {
			Mob* delete_mob = mobItr->second;

			// Call RemoveMob(Mob*) to remove this object from all "mob" lists now that we have the object reference
			Result = RemoveMob(delete_mob);
		}
	}

	return Result;
}

bool EntityList::RemoveMob(Mob *delete_mob) {
	bool Result = false;

	if(delete_mob) {
		list<Mob*>::iterator itr = mob_list.begin();

		// Remove the object reference from the Mob entity id map container
		mob_entityid_map.erase(delete_mob->GetID());

		// Remove the object reference from the Mob name map container
		mob_name_map.erase(delete_mob->GetName());

		// Remove the object reference from the Mob list container
		mob_list.remove(delete_mob);

		// Delete the referenced object
		//safe_delete(delete_mob);
		//delete_mob = 0;

		Result = true;
	}

	return Result;
}

bool EntityList::RemoveNPC(int32 delete_id) {
	bool Result = false;

	if(delete_id > 0) {
		NPCMap::iterator npcItr = npc_entityid_map.begin();

		npcItr = npc_entityid_map.find(delete_id);

		if(npcItr != npc_entityid_map.end()) {
			NPC* delete_npc = npcItr->second;

			// Call RemoveNPC(NPC*) to remove this object from all "NPC" lists now that we have the object reference
			Result = RemoveNPC(delete_npc);
		}
	}

	return Result;
}

bool EntityList::RemoveNPC(NPC *delete_npc) {
	bool Result = false;

	if(delete_npc) {
		// Remove the object reference from the NPC entity id map container
		npc_entityid_map.erase(delete_npc->GetID());

		// Remove the object reference from the NPC id map container
		npc_npcid_map.erase(delete_npc->GetNPCTypeID());

		// Remove the object reference from the npc list container
		npc_list.remove(delete_npc);

		mob_entityid_map.erase(delete_npc->GetID());

		mob_name_map.erase(std::string(delete_npc->GetName()));

		mob_list.remove(delete_npc);

		// Delete the referenced object
		//safe_delete(delete_npc);
		//delete_npc = 0;

		Result = true;
	}

	return Result;
}

bool EntityList::RemoveClient(int32 delete_id) {
	bool Result = false;

	if(delete_id > 0) {
		ClientMap::iterator itr = client_entityid_map.begin();
		
		itr = client_entityid_map.find(delete_id);

		if(itr != client_entityid_map.end()) {
			Client* delete_mob = itr->second;

			// Call RemoveClient(Client*) to remove this object from all "client" lists now that we have the object reference
			Result = RemoveClient(delete_mob);
		}
	}

	return Result;
}

bool EntityList::RemoveClient(Client *delete_client) {
	bool Result = false;

	if(delete_client) {
		// Remove the object reference from the Client entity id map container
		client_entityid_map.erase(delete_client->GetID());

		// Remove the object reference from the Client character id map container
		client_characterid_map.erase(delete_client->CharacterID());

		// Remove the object reference from the Client name map container
		client_name_map.erase(std::string(delete_client->GetName()));

		// Remove the object reference from the npc list container
		client_list.remove(delete_client);

		// Delete the referenced object
		//safe_delete(delete_client);
		//delete_client = 0;

		Result = true;
	}

	return Result;
}

bool EntityList::RemoveObject(int32 delete_id) {
	LinkedListIterator<Object*> iterator(object_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(iterator.GetData()->GetID()==delete_id){
			iterator.RemoveCurrent();
			return true;
		}
		iterator.Advance();
	}
	return false;
}

bool EntityList::RemoveTrap(int32 delete_id) {
	LinkedListIterator<Trap*> iterator(trap_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(iterator.GetData()->GetID()==delete_id){
			iterator.RemoveCurrent();
			return true;
		}
		iterator.Advance();
	}
	return false;
}

bool EntityList::RemoveDoor(int32 delete_id) {
	LinkedListIterator<Doors*> iterator(door_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(iterator.GetData()->GetID()==delete_id){
			iterator.RemoveCurrent();
			return true;
		}
		iterator.Advance();
	}
	return false;
}

bool EntityList::RemoveCorpse(int32 delete_id) {
	bool Result = false;

	if(delete_id > 0) {
		CorpseMap::iterator corpseItr = corpse_entityid_map.begin();

		corpseItr = corpse_entityid_map.find(delete_id);

		if(corpseItr != corpse_entityid_map.end()) {
			Corpse* delete_corpse = corpseItr->second;

			// Call RemoveCorpse(Corpse*) to remove this object from all "corpse" lists now that we have the object reference
			Result = RemoveCorpse(delete_corpse);
		}
	}

	return Result;
}

bool EntityList::RemoveCorpse(Corpse* delete_corpse) {
	bool Result = false;

	if(delete_corpse) {
		// Remove the object reference from the corpse entity id map container
		corpse_entityid_map.erase(delete_corpse->GetID());

		// Remove the object reference from the corpse list container
		corpse_list.remove(delete_corpse);

		// Delete the referenced object
		//safe_delete(delete_corpse);
		//delete_corpse = 0;

		Result = true;
	}

	return Result;
}

bool EntityList::RemoveGroup(int32 delete_id){
	list<Group *>::iterator iterator;

	iterator = group_list.begin();

	while(iterator != group_list.end())
	{
		if((*iterator)->GetID() == delete_id) {
			group_list.remove (*iterator);
#if EQDEBUG >= 5
	CheckGroupList (__FILE__, __LINE__);
#endif
			return true;
		}
		iterator++;
	}
#if EQDEBUG >= 5
	CheckGroupList (__FILE__, __LINE__);
#endif
	return false;
}

bool EntityList::RemoveRaid(int32 delete_id){
	list<Raid *>::iterator iterator;

	iterator = raid_list.begin();

	while(iterator != raid_list.end())
	{
		if((*iterator)->GetID() == delete_id) {
			raid_list.remove (*iterator);
			return true;
		}
		iterator++;
	}
	return false;
}

void EntityList::Clear()
{
	RemoveAllClients();
	entity_list.RemoveAllTraps(); //we can have child npcs so we go first
	entity_list.RemoveAllNPCs();
	entity_list.RemoveAllMobs();
	entity_list.RemoveAllCorpses();
	entity_list.RemoveAllGroups();
	entity_list.RemoveAllDoors();
	entity_list.RemoveAllObjects();
	entity_list.RemoveAllRaids();
	entity_list.RemoveAllLocalities();
	last_insert_id = 0;
}

void EntityList::UpdateWho(bool iSendFullUpdate) {
	if(worldserver.Connected() || ZoneLoaded) {
		int32 tmpNumUpdates = numclients + 5;
		ServerPacket* pack = 0;
		ServerClientListKeepAlive_Struct* sclka = 0;
		
		if (!iSendFullUpdate) {
			pack = new ServerPacket(ServerOP_ClientListKA, sizeof(ServerClientListKeepAlive_Struct) + (tmpNumUpdates * 4));
			sclka = (ServerClientListKeepAlive_Struct*) pack->pBuffer;
		}

		list<Client*> ClonedClientList = client_list;

		for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
			Client* c = *itr;

			if(c) {
				if(c->InZone()) {
					if(iSendFullUpdate) {
						c->UpdateWho();
					}
					else {
						if (sclka->numupdates >= tmpNumUpdates) {
							tmpNumUpdates += 10;
							int8* tmp = pack->pBuffer;
							pack->pBuffer = new int8[sizeof(ServerClientListKeepAlive_Struct) + (tmpNumUpdates * 4)];
							memset(pack->pBuffer, 0, sizeof(ServerClientListKeepAlive_Struct) + (tmpNumUpdates * 4));
							memcpy(pack->pBuffer, tmp, pack->size);
							pack->size = sizeof(ServerClientListKeepAlive_Struct) + (tmpNumUpdates * 4);
							safe_delete_array(tmp);
						}
						sclka->wid[sclka->numupdates] = c->GetWID();
						sclka->numupdates++;
					}
				}
			}
		}

		if (!iSendFullUpdate) {
			pack->size = sizeof(ServerClientListKeepAlive_Struct) + (sclka->numupdates * 4);
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
	}
}

void EntityList::RemoveEntity(int16 id)
{
	if (id == 0)
		return;
	if(entity_list.RemoveMob(id))
		return;
	else if(entity_list.RemoveCorpse(id))
		return;
	else if(entity_list.RemoveDoor(id))
		return;
	else if(entity_list.RemoveGroup(id))
		return;
	else if(entity_list.RemoveTrap(id))
		return;

#ifdef BOTS
	// This block of code is necessary to clean up bot objects
	else if(entity_list.RemoveBotRaid(id))
		return;
	else if(entity_list.RemoveBot(id))
		return;
#endif //BOTS

	else 
		entity_list.RemoveObject(id);
}

void EntityList::Process()
{
	_ZP(EntityList_Process);
	CheckSpawnQueue();
}

void EntityList::CountNPC(int32* NPCCount, int32* NPCLootCount, int32* gmspawntype_count) {
	*NPCCount = 0;
	*NPCLootCount = 0;
	
	list<NPC*> ClonedNPCList = npc_list;

	for(list<NPC*>::iterator itr = ClonedNPCList.begin(); itr != ClonedNPCList.end(); itr++) {
		NPC* n = *itr;

		if(n) {
			if(n->GetNPCTypeID() == 0)
				(*gmspawntype_count)++;
		}
	}
}

void EntityList::DoZoneDump(ZSDump_Spawn2* spawn2_dump, ZSDump_NPC* npc_dump, ZSDump_NPC_Loot* npcloot_dump, NPCType* gmspawntype_dump) {
	int32 spawn2index = 0;
	int32 NPCindex = 0;
	int32 NPCLootindex = 0;
	int32 gmspawntype_index = 0;
	
	if (npc_dump) {
		list<NPC*> ClonedNPCList = npc_list;

		for(list<NPC*>::iterator itr = ClonedNPCList.begin(); itr != ClonedNPCList.end(); itr++) {
			NPC* npc = *itr;

			if(npc) {
				if (spawn2_dump != 0)
					npc_dump[NPCindex].spawn2_dump_index = zone->DumpSpawn2(spawn2_dump, &spawn2index, npc->respawn2);

				npc_dump[NPCindex].npctype_id = npc->GetNPCTypeID();
				npc_dump[NPCindex].cur_hp = npc->GetHP();

				if (npc->IsCorpse()) {
					if (npc->CastToCorpse()->IsLocked())
						npc_dump[NPCindex].corpse = 2;
					else
						npc_dump[NPCindex].corpse = 1;
				}
				else
					npc_dump[NPCindex].corpse = 0;

				npc_dump[NPCindex].decay_time_left = 0xFFFFFFFF;
				npc_dump[NPCindex].x = npc->GetX();
				npc_dump[NPCindex].y = npc->GetY();
				npc_dump[NPCindex].z = npc->GetZ();
				npc_dump[NPCindex].heading = npc->GetHeading();
				npc_dump[NPCindex].copper = npc->copper;
				npc_dump[NPCindex].silver = npc->silver;
				npc_dump[NPCindex].gold = npc->gold;
				npc_dump[NPCindex].platinum = npc->platinum;

				if (npcloot_dump != 0)
					npc->DumpLoot(NPCindex, npcloot_dump, &NPCLootindex);

				if (gmspawntype_dump != 0) {
					if (npc->GetNPCTypeID() == 0) {
						memcpy(&gmspawntype_dump[gmspawntype_index], npc->NPCTypedata, sizeof(NPCType));
						npc_dump[NPCindex].gmspawntype_index = gmspawntype_index;
						gmspawntype_index++;
					}
				}

				NPCindex++;
			}
		}
	}

	if (spawn2_dump != 0)
		zone->DumpAllSpawn2(spawn2_dump, &spawn2index);
}

void EntityList::Depop(bool StartSpawnTimer) {
	list<NPC*> ClonedNPCList = npc_list;

	for(list<NPC*>::iterator itr = ClonedNPCList.begin(); itr != ClonedNPCList.end(); itr++) {
		NPC* npc = *itr;

		if(npc) {
			Mob* owner = npc->GetOwner();

			if(owner) {
				if(owner->IsClient()) {
					continue;
				}
				else {
					npc->Depop(StartSpawnTimer);
				}
			}
		}
	}
}

void EntityList::SendTraders(Client* client) {
	if(client) {
		list<Client*> ClonedClientList = client_list;

		for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
			Client* trader = *itr;

			if(trader) {
				if(trader->IsTrader())
					client->SendTraderPacket(trader);

				if(trader->IsBuyer())
					client->SendBuyerPacket(trader);
			}
		}
	}
}

void EntityList::RemoveFromHateLists(Mob* mob, bool settoone) {
	list<NPC*> ClonedNPCList = npc_list;

	for(list<NPC*>::iterator itr = ClonedNPCList.begin(); itr != ClonedNPCList.end(); itr++) {
		NPC* npc = *itr;

		if(npc) {
			if(npc->CheckAggro(mob)) {
				if(!settoone)
					npc->RemoveFromHateList(mob);
				else
					npc->SetHate(mob, 1);
			}
		}
	}
}

void EntityList::RemoveDebuffs(Mob* caster) {
	list<Mob*> ClonedMobList = mob_list;

	for(list<Mob*>::iterator itr = ClonedMobList.begin(); itr != ClonedMobList.end(); itr++) {
		Mob* mob = *itr;

		if(mob) {
			mob->BuffFadeDetrimentalByCaster(caster);
		}
	}
}


// Currently, a new packet is sent per entity.
// @todo: Come back and use FLAG_COMBINED to pack
// all updates into one packet.
void EntityList::SendPositionUpdates(Client* client, int32 cLastUpdate, float range, Entity* alwayssend, bool iSendEvenIfNotChanged) {
	range = range * range;
	EQApplicationPacket* outapp = 0;
	PlayerPositionUpdateServer_Struct* ppu = 0;

	list<Mob*> ClonedMobList = mob_list;

	for(list<Mob*>::iterator itr = ClonedMobList.begin(); itr != ClonedMobList.end(); itr++) {
		Mob* mob = *itr;

		if(mob) {
			if (outapp == 0) {
				outapp = new EQApplicationPacket(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
				ppu = (PlayerPositionUpdateServer_Struct*)outapp->pBuffer;
			}

			if (!mob->IsCorpse() 
				&& (mob != client)
				&& (mob->IsClient() || iSendEvenIfNotChanged || (mob->LastChange() >= cLastUpdate)) 
				&& (!mob->IsClient() || !mob->CastToClient()->GMHideMe(client))
				) {
					if (range == 0 || (mob == alwayssend) || (mob->DistNoRootNoZ(*client) <= range)) {
						mob->MakeSpawnUpdate(ppu);
					}
			}

			if(mob && mob->IsClient() && mob->GetID() > 0)
/*#ifdef PACKET_UPDATE_MANAGER
				client->GetUpdateManager()->QueuePacket(outapp, false, mob, mob->DistNoRoot(*client));
#else*/
				client->QueuePacket(outapp, false, Client::CLIENT_CONNECTED);
//#endif
			safe_delete(outapp);
			outapp = 0;	
		}
	}

	safe_delete(outapp);
}

char* EntityList::MakeNameUnique(char* name) {
	bool used[300];
	
	memset(used, 0, sizeof(used));
	name[61] = 0; name[62] = 0; name[63] = 0;
	
	int len = strlen(name);

	list<Mob*> ClonedMobList = mob_list;

	for(list<Mob*>::iterator itr = ClonedMobList.begin(); itr != ClonedMobList.end(); itr++) {
		Mob* mob = *itr;

		if(mob) {
			if (strncasecmp(mob->GetName(), name, len) == 0) {
				if (Seperator::IsNumber(&mob->GetName()[len])) {
					used[atoi(&mob->GetName()[len])] = true;
				}
			}
		}
	}

	for (int i=0; i < 300; i++) {
		if (!used[i]) {
#ifdef WIN32
			snprintf(name, 64, "%s%03d", name, i);
#else
			//glibc clears destination of snprintf
			//make a copy of name before snprintf--misanthropicfiend
			char temp_name[64];
			strn0cpy(temp_name, name, 64);
			snprintf(name, 64, "%s%03d", temp_name, i);
#endif
			return name;
		}
	}

	LogFile->write(EQEMuLog::Error, "Fatal error in EntityList::MakeNameUnique: Unable to find unique name for '%s'", name);
	char tmp[64] = "!";
	strn0cpy(&tmp[1], name, sizeof(tmp) - 1);
	strcpy(name, tmp);

	return MakeNameUnique(name);
}

char* EntityList::RemoveNumbers(char* name) {
	char	tmp[64];
	memset(tmp, 0, sizeof(tmp));
	int k = 0;
	for (unsigned int i=0; i<strlen(name) && i<sizeof(tmp); i++) {
		if (name[i] < '0' || name[i] > '9')
			tmp[k++] = name[i];
	}
	strn0cpy(name, tmp, sizeof(tmp));
	return name;
}

void EntityList::ListNPCs(Client* client, const char* arg1, const char* arg2, int8 searchtype) {
	if (arg1 == 0)
		searchtype = 0;
	else if (arg2 == 0 && searchtype >= 2)
		searchtype = 0;

	int32 x = 0;
	int32 z = 0;
	char sName[36];
	
	client->Message(0, "NPCs in the zone:");
	
	if(searchtype == 0) {
		list<NPC*> ClonedNPCList = npc_list;

		for(list<NPC*>::iterator itr = ClonedNPCList.begin(); itr != ClonedNPCList.end(); itr++) {
			NPC* npc = *itr;

			if(npc) {
				client->Message(0, "  %5d: %s", npc->GetID(), npc->GetName());
				x++;
				z++;
			}
		}
	}
	else if(searchtype == 1) {
		client->Message(0, "Searching by name method. (%s)",arg1);
		char* tmp = new char[strlen(arg1) + 1];
		strcpy(tmp, arg1);
		strupr(tmp);

		list<NPC*> ClonedNPCList = npc_list;

		for(list<NPC*>::iterator itr = ClonedNPCList.begin(); itr != ClonedNPCList.end(); itr++) {
			NPC* npc = *itr;

			if(npc) {
				z++;
				strcpy(sName, npc->GetName());
				strupr(sName);
				if (strstr(sName, tmp)) {
					client->Message(0, "  %5d: %s", npc->GetID(), npc->GetName());
					x++;
				}
			}
		}

		safe_delete_array(tmp);
	}
	else if(searchtype == 2) {
		client->Message(0, "Searching by number method. (%s %s)",arg1,arg2);

		list<NPC*> ClonedNPCList = npc_list;

		for(list<NPC*>::iterator itr = ClonedNPCList.begin(); itr != ClonedNPCList.end(); itr++) {
			NPC* npc = *itr;

			if(npc) {
				z++;
				if ((npc->GetID() >= atoi(arg1)) && (npc->GetID() <= atoi(arg2)) && (atoi(arg1) <= atoi(arg2))) {
					client->Message(0, "  %5d: %s", npc->GetID(), npc->GetName());
					x++;
				}
			}
		}
	}

	client->Message(0, "%d npcs listed. There is a total of %d npcs in this zone.", x, z);
}

void EntityList::ListNPCCorpses(Client* client) {
	if(client) {
		int32 x = 0;

		client->Message(0, "NPC Corpses in the zone:");

		list<Corpse*> ClonedCorpseList = corpse_list;

		for(list<Corpse*>::iterator itr = ClonedCorpseList.begin(); itr != ClonedCorpseList.end(); itr++) {
			Corpse* corpse = *itr;

			if(corpse) {
				if(corpse->IsNPCCorpse()) {
					client->Message(0, "  %5d: %s", corpse->GetID(), corpse->GetName());
					x++;
				}
			}
		}

		client->Message(0, "%d npc corpses listed.", x);
	}
}

void EntityList::ListPlayerCorpses(Client* client) {
	if(client) {
		int32 x = 0;

		client->Message(0, "NPC Corpses in the zone:");

		list<Corpse*> ClonedCorpseList = corpse_list;

		for(list<Corpse*>::iterator itr = ClonedCorpseList.begin(); itr != ClonedCorpseList.end(); itr++) {
			Corpse* corpse = *itr;

			if(corpse) {
				if(corpse->IsPlayerCorpse()) {
					client->Message(0, "  %5d: %s", corpse->GetID(), corpse->GetName());
					x++;
				}
			}
		}

		client->Message(0, "%d player corpses listed.", x);
	}
}

void EntityList::FindPathsToAllNPCs() {
	if(zone->pathing) {
		list<NPC*> ClonedNPCList = npc_list;

		for(list<NPC*>::iterator itr = ClonedNPCList.begin(); itr != ClonedNPCList.end(); itr++) {
			NPC* npc = *itr;

			if(npc) {
				VERTEX Node0 = zone->pathing->GetPathNodeCoordinates(0, false);
				VERTEX Dest(npc->GetX(), npc->GetY(), npc->GetZ());
				list<int> Route = zone->pathing->FindRoute(Node0, Dest);

				if(Route.size() == 0)
					printf("Unable to find a route to %s\n", npc->GetName());
				else
					printf("Found a route to %s\n", npc->GetName());
			}
		}

		fflush(stdout);
	}
}

// returns the number of corpses deleted. A negative number indicates an error code.
sint32 EntityList::DeleteNPCCorpses() {
	sint32 Result = 0;

	list<Corpse*> ClonedCorpseList = corpse_list;

	for(list<Corpse*>::iterator itr = ClonedCorpseList.begin(); itr != ClonedCorpseList.end(); itr++) {
		Corpse* corpse = *itr;

		if(corpse) {
			if (corpse->IsNPCCorpse()) {
				corpse->Depop();
				Result++;
			}
		}
	}

	return Result;
}

// returns the number of corpses deleted. A negative number indicates an error code.
sint32 EntityList::DeletePlayerCorpses() {
	sint32 Result = 0;

	list<Corpse*> ClonedCorpseList = corpse_list;

	for(list<Corpse*>::iterator itr = ClonedCorpseList.begin(); itr != ClonedCorpseList.end(); itr++) {
		Corpse* corpse = *itr;

		if(corpse) {
			if (corpse->IsPlayerCorpse()) {
				corpse->Delete();
				Result++;
			}
		}
	}

	return Result;
}

void EntityList::SendPetitionToAdmins() {
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_PetitionUpdate,sizeof(PetitionUpdate_Struct));
	PetitionUpdate_Struct* pcus = (PetitionUpdate_Struct*) outapp->pBuffer;
	pcus->petnumber = 0;		// Petition Number
	pcus->color = 0;
	pcus->status = 0xFFFFFFFF;
	pcus->senttime = 0;
	strcpy(pcus->accountid, "");
	strcpy(pcus->gmsenttoo, "");
	pcus->quetotal=0;

	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* client = *itr;

		if(client) {
			if (client->Admin() >= 80)
				client->QueuePacket(outapp);
		}
	}

	safe_delete(outapp);
}

void EntityList::SendPetitionToAdmins(Petition* pet) {
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_PetitionUpdate,sizeof(PetitionUpdate_Struct));
	PetitionUpdate_Struct* pcus = (PetitionUpdate_Struct*) outapp->pBuffer;
	pcus->petnumber = pet->GetID();		// Petition Number
	if (pet->CheckedOut()) {
		pcus->color = 0x00;
		pcus->status = 0xFFFFFFFF;
		pcus->senttime = pet->GetSentTime();
		strcpy(pcus->accountid, "");
		strcpy(pcus->gmsenttoo, "");
	}
	else {
		pcus->color = pet->GetUrgency();	// 0x00 = green, 0x01 = yellow, 0x02 = red
		pcus->status = pet->GetSentTime();
		pcus->senttime = pet->GetSentTime();			// 4 has to be 0x1F
		strcpy(pcus->accountid, pet->GetAccountName());
		strcpy(pcus->charname, pet->GetCharName());
	}

	pcus->quetotal = petition_list.GetTotalPetitions();

	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* client = *itr;

		if(client) {
			if (client->Admin() >= 80) {
				if (pet->CheckedOut())
					strcpy(pcus->gmsenttoo, "");
				else
					strcpy(pcus->gmsenttoo, client->GetName());

				client->QueuePacket(outapp);
			}
		}
	}

	safe_delete(outapp);
}

void EntityList::ClearClientPetitionQueue() {
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_PetitionUpdate,sizeof(PetitionUpdate_Struct));
	PetitionUpdate_Struct* pet = (PetitionUpdate_Struct*) outapp->pBuffer;
	pet->color = 0x00;
	pet->status = 0xFFFFFFFF;
	pet->senttime = 0;
	strcpy(pet->accountid, "");
	strcpy(pet->gmsenttoo, "");
	strcpy(pet->charname, "");
	pet->quetotal = petition_list.GetTotalPetitions();

	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* client = *itr;

		if(client) {
			if (client->Admin() >= 100) {
				int x = 0;
				for (x=0; x<64; x++) {
					pet->petnumber = x;
					client->QueuePacket(outapp);
				}
			}
		}
	}

	safe_delete(outapp);
}

void EntityList::WriteEntityIDs() {
	list<Mob*> ClonedMobList = mob_list;

	for(list<Mob*>::iterator itr = ClonedMobList.begin(); itr != ClonedMobList.end(); itr++) {
		Mob* mob = *itr;

		if(mob) {
			cout << "ID: " << mob->GetID() << "  Name: " << mob->GetName() << endl;
		}
	}
}

BulkZoneSpawnPacket::BulkZoneSpawnPacket(Client* iSendTo, int32 iMaxSpawnsPerPacket) {
	data = 0;
	pSendTo = iSendTo;
	pMaxSpawnsPerPacket = iMaxSpawnsPerPacket;
#ifdef _EQDEBUG
	if (pMaxSpawnsPerPacket <= 0 || pMaxSpawnsPerPacket > MAX_SPAWNS_PER_PACKET) {
		// ok, this *cant* be right =p
		ThrowError("Error in BulkZoneSpawnPacket::BulkZoneSpawnPacket(): pMaxSpawnsPerPacket outside range that makes sense");
	}
#endif
}

BulkZoneSpawnPacket::~BulkZoneSpawnPacket() {
	SendBuffer();
	safe_delete_array(data)
}

bool BulkZoneSpawnPacket::AddSpawn(NewSpawn_Struct* ns) {
	if (!data) {
		data = new NewSpawn_Struct[pMaxSpawnsPerPacket];
		memset(data, 0, sizeof(NewSpawn_Struct) * pMaxSpawnsPerPacket);
		index = 0;
	}
	memcpy(&data[index], ns, sizeof(NewSpawn_Struct));
	index++;
	if (index >= pMaxSpawnsPerPacket) {
		SendBuffer();
		return true;
	}
	return false;
}

void BulkZoneSpawnPacket::SendBuffer() {
	if (!data)
		return;
	
	int32 tmpBufSize = (index * sizeof(NewSpawn_Struct));
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_ZoneSpawns, (unsigned char *)data, tmpBufSize);
	
	if (pSendTo) {
		pSendTo->FastQueuePacket(&outapp);
	} else {
		entity_list.QueueClients(0, outapp);
		safe_delete(outapp);
	}
	memset(data, 0, sizeof(NewSpawn_Struct) * pMaxSpawnsPerPacket);
	index = 0;
}

void EntityList::DoubleAggro(Mob* who) {
	list<NPC*> ClonedNPCList = npc_list;

	for(list<NPC*>::iterator itr = ClonedNPCList.begin(); itr != ClonedNPCList.end(); itr++) {
		NPC* npc = *itr;

		if(npc) {
			if (npc->CheckAggro(who))
				npc->SetHate(who, npc->GetHateAmount(who), npc->GetHateAmount(who) * 2);
		}
	}
}

void EntityList::HalveAggro(Mob* who) {
	list<NPC*> ClonedNPCList = npc_list;

	for(list<NPC*>::iterator itr = ClonedNPCList.begin(); itr != ClonedNPCList.end(); itr++) {
		NPC* npc = *itr;

		if(npc) {
			if (npc->CheckAggro(who))
				npc->SetHate(who, npc->GetHateAmount(who) / 2);
		}
	}
}

void EntityList::Evade(Mob *who) {
	uint32 flatval = who->GetLevel() * 13;
	int amt = 0;

	list<NPC*> ClonedNPCList = npc_list;

	for(list<NPC*>::iterator itr = ClonedNPCList.begin(); itr != ClonedNPCList.end(); itr++) {
		NPC* npc = *itr;

		if(npc) {
			if (npc->CheckAggro(who)) {
				amt = npc->GetHateAmount(who);
				amt -= flatval;
				
				if(amt > 0)
					npc->SetHate(who, amt);
				else
					npc->SetHate(who, 0);
			}
		}
	}
}

//removes "targ" from all hate lists, including feigned, in the zone
void EntityList::ClearAggro(Mob* targ) {
	if(targ) {
		list<NPC*> ClonedNPCList = npc_list;

		for(list<NPC*>::iterator itr = ClonedNPCList.begin(); itr != ClonedNPCList.end(); itr++) {
			NPC* npc = *itr;

			if(npc) {
				if (npc->CheckAggro(targ))
					npc->RemoveFromFeignMemory(targ->CastToClient());	//just in case we feigned
			}
		}
	}
}

void EntityList::ClearFeignAggro(Mob* targ) {
	if(targ) {
		list<NPC*> ClonedNPCList = npc_list;

		for(list<NPC*>::iterator itr = ClonedNPCList.begin(); itr != ClonedNPCList.end(); itr++) {
			NPC* npc = *itr;

			if(npc) {
				if (npc->CheckAggro(targ)) {
					if(npc->SpecAttacks[IMMUNE_FEIGN_DEATH]) {
						npc->SetHate(targ, 0);
						continue;
					}

					npc->RemoveFromHateList(targ);
					// EverHood 6/24/06
					// For client targets if the mob that hated us is 35+ 
					// there is a 3 outta 5 chance he adds us to feign memory
					if(targ->IsClient()){
						if (npc->GetLevel() >= 35){
							if(MakeRandomInt(1,100) <= 60){
								npc->AddFeignMemory(targ->CastToClient());
							}
						}
					}
				}
			}
		}
	}
}

// EverHood 6/17/06
void EntityList::ClearZoneFeignAggro(Client* targ) {
	list<NPC*> ClonedNPCList = npc_list;

	for(list<NPC*>::iterator itr = ClonedNPCList.begin(); itr != ClonedNPCList.end(); itr++) {
		NPC* npc = *itr;

		if(npc) {
			npc->RemoveFromFeignMemory(targ);
		}
	}
}

void EntityList::AggroZone(Mob* who, int hate) {
	list<NPC*> ClonedNPCList = npc_list;

	for(list<NPC*>::iterator itr = ClonedNPCList.begin(); itr != ClonedNPCList.end(); itr++) {
		NPC* npc = *itr;

		if(npc) {
			npc->AddToHateList(who, hate);
		}
	}
}

// Signal Quest command function
void EntityList::SignalMobsByNPCID(int32 snpc, int signal_id) {
	list<NPC*> ClonedNPCList = npc_list;

	for(list<NPC*>::iterator itr = ClonedNPCList.begin(); itr != ClonedNPCList.end(); itr++) {
		NPC* npc = *itr;

		if(npc) {
			if (npc->GetNPCTypeID() == snpc) {
				npc->SignalNPC(signal_id);
			}
		}
	}
}


bool EntityList::MakeTrackPacket(Client* client) {
	int32 distance = 0;
	float MobDistance;

	if(client->GetClass() == DRUID)
		distance = (client->GetSkill(TRACKING)*10);
	else if(client->GetClass() == RANGER)
		distance = (client->GetSkill(TRACKING)*12);
	else if(client->GetClass() == BARD)
		distance = (client->GetSkill(TRACKING)*7); 
	if(distance <= 0)
		return false;
	if(distance<300)
		distance=300;
	
	int32 spe= 0;
	bool ret = false;
	
	spe = mob_list.size() + 50;

	uchar* buffer1 = new uchar[sizeof(Track_Struct)];
	Track_Struct* track_ent = (Track_Struct*) buffer1;
	
	uchar* buffer2 = new uchar[sizeof(Track_Struct)*spe];
	Tracking_Struct* track_array = (Tracking_Struct*) buffer2;
	memset(track_array, 0, sizeof(Track_Struct)*spe);
	
	int32 array_counter = 0;
	
	list<Mob*> ClonedMobList = mob_list;

	for(list<Mob*>::iterator itr = ClonedMobList.begin(); itr != ClonedMobList.end(); itr++) {
		Mob* mob = *itr;

		if(mob) {
			if((MobDistance = mob->DistNoZ(*client)) <= distance) {
				if(mob->IsTrackable()) {
					memset(track_ent, 0, sizeof(Track_Struct));
					Mob* cur_entity = mob;
					track_ent->entityid = cur_entity->GetID();
					track_ent->distance = MobDistance;
					memcpy(&track_array->Entrys[array_counter], track_ent, sizeof(Track_Struct));
					array_counter++;
				}
			}
		}
	}

	if(array_counter <= spe) {
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_Track,sizeof(Track_Struct)*(array_counter));
		memcpy(outapp->pBuffer, track_array,sizeof(Track_Struct)*(array_counter));
		outapp->priority = 6;
		client->QueuePacket(outapp);
		safe_delete(outapp);
		ret = true;
	}
	else {
		LogFile->write(EQEMuLog::Status, "ERROR: Unable to transmit a Tracking_Struct packet. Mobs in zone = %i. Mobs in packet = %i", array_counter, spe);
	}
	
	safe_delete_array(buffer1);
	safe_delete_array(buffer2);
	
	return ret;
}

void EntityList::MessageGroup(Mob* sender, bool skipclose, int32 type, const char* message, ...) {
	va_list argptr;
	char buffer[4096];

	va_start(argptr, message);
	vsnprintf(buffer, 4095, message, argptr);
	va_end(argptr);

	float dist2 = 100;

	if (skipclose)
		dist2 = 0;

	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* client = *itr;

		if(client) {
			if (client != sender && (client->Dist(*sender) <= dist2 || client->GetGroup() == sender->CastToClient()->GetGroup())) {
				client->Message(type, buffer);
			}
		}
	}
}


bool EntityList::Fighting(Mob* targ) {
	bool Result = false;

	list<NPC*> ClonedNPCList = npc_list;

	for(list<NPC*>::iterator itr = ClonedNPCList.begin(); itr != ClonedNPCList.end(); itr++) {
		NPC* npc = *itr;

		if(npc) {
			if (npc->CheckAggro(targ)) {
				Result = true;
				break;
			}
		}
	}

	return Result;
}

void EntityList::AddHealAggro(Mob* target, Mob* caster, int16 thedam) {
	int16 count = 0;

	list<NPC*> ClonedNPCList = npc_list;

	for(list<NPC*>::iterator itr = ClonedNPCList.begin(); itr != ClonedNPCList.end(); itr++) {
		NPC* npc = *itr;

		if(npc) {
			if(!npc->CheckAggro(target))
				continue;

			if (!npc->IsMezzed() && !npc->IsStunned() && !npc->IsFeared()) {
				++count;
			}
		}
	}

	if(thedam > 1) {
		if(count > 0)
			thedam = (thedam / count);

		if(thedam < 1)
			thedam = 1;
	}

	ClonedNPCList = npc_list;

	for(list<NPC*>::iterator itr = ClonedNPCList.begin(); itr != ClonedNPCList.end(); itr++) {
		NPC* npc = *itr;

		if(npc) {
			if(!npc->CheckAggro(target)){
				continue;
			}

			if (!npc->IsMezzed() && !npc->IsStunned() && !npc->IsFeared()) {
				if(npc->IsPet()){
					if(caster){
						if(npc->CheckAggro(caster)) {
							npc->AddToHateList(caster, thedam);
						}
					}
				}
				else {
					if(caster) {
						if(npc->CheckAggro(caster)) {
							npc->AddToHateList(caster, thedam);
						}
						else {
							npc->AddToHateList(caster, thedam*0.33);
						}
					}
				}
			}
		}
	}
}

void EntityList::OpenDoorsNear(NPC* who)
{
	LinkedListIterator<Doors*> iterator(door_list);
	iterator.Reset();
	while(iterator.MoreElements()) {
		Doors *cdoor = iterator.GetData();
		if(cdoor && !cdoor->IsDoorOpen()) {
			float zdiff = who->GetZ() - cdoor->GetZ();
			if(zdiff < 0)
				zdiff = 0 - zdiff;
			float curdist = 0;
			float tmp = who->GetX() - cdoor->GetX();
			curdist += tmp * tmp;
			tmp = who->GetY() - cdoor->GetY();
			curdist += tmp * tmp;
			if (zdiff < 10 && curdist <= 100) {
				cdoor->NPCOpen(who);
			}
		}
		iterator.Advance();
	}
}

void EntityList::SendAlarm(Trap* trap, Mob* currenttarget, int8 kos) {
	if(trap && currenttarget) {
		float val2 = trap->effectvalue * trap->effectvalue;

		list<NPC*> ClonedNPCList = npc_list;

		for(list<NPC*>::iterator itr = ClonedNPCList.begin(); itr != ClonedNPCList.end(); itr++) {
			NPC* npc = *itr;

			if(npc) {
				float curdist = 0;
				float tmp = npc->GetX() - trap->x;
				curdist += tmp*tmp;
				tmp = npc->GetY() - trap->y;
				curdist += tmp * tmp;
				tmp = npc->GetZ() - trap->z;
				curdist += tmp * tmp;
				if (!npc->GetOwner() && 
					/*!cur->CastToMob()->dead && */
					!npc->IsEngaged() && 
					curdist <= val2 )
				{
					if(kos) {
						int8 factioncon = currenttarget->GetReverseFactionCon(npc);

						if(factioncon == FACTION_THREATENLY || factioncon == FACTION_SCOWLS) {
							npc->AddToHateList(currenttarget, 1);
						}
					}
					else {
						npc->AddToHateList(currenttarget, 1);
					}
				}
			}
		}
	}
}

void EntityList::AddProximity(NPC *proximity_for) {
	RemoveProximity(proximity_for->GetID());
	
	proximity_list.Insert(proximity_for);
	
	proximity_for->proximity = new NPCProximity;
}

bool EntityList::RemoveProximity(int32 delete_npc_id) {
	LinkedListIterator<NPC*> iterator(proximity_list);
	iterator.Reset();
	while(iterator.MoreElements()) {
		NPC *d = iterator.GetData();
		if(d->GetID() == delete_npc_id) {
			safe_delete(d->proximity);
			iterator.RemoveCurrent(false);
			return true;
		}
		iterator.Advance();
	}
	return false;
}

void EntityList::RemoveAllLocalities() {
	LinkedListIterator<NPC*> iterator(proximity_list);
	iterator.Reset();
	while(iterator.MoreElements())
		iterator.RemoveCurrent(false);
}

void EntityList::ProcessMove(Client *c, float x, float y, float z) {
	/*
		We look through each proximity, looking to see if last_* was in(out)
		the proximity, and the new supplied coords are out(in)...
	*/
	LinkedListIterator<NPC*> iterator(proximity_list);
	
	float last_x = c->ProximityX();
	float last_y = c->ProximityY();
	float last_z = c->ProximityZ();
	
	for(iterator.Reset(); iterator.MoreElements(); iterator.Advance()) {
		NPC *d = iterator.GetData();
		NPCProximity *l = d->proximity;
		if(l == NULL)
			continue;
		
		//check both bounding boxes, if either coords pairs
		//cross a boundary, send the event.
		bool old_in = true;
		bool new_in = true;
		if(   last_x < l->min_x || last_x > l->max_x
		   || last_y < l->min_y || last_y > l->max_y
		   || last_z < l->min_z || last_z > l->max_z ) {
			old_in = false;
		}
		if(   x < l->min_x || x > l->max_x
		   || y < l->min_y || y > l->max_y
		   || z < l->min_z || z > l->max_z ) {
			new_in = false;
		}
		
		if(old_in && !new_in) {
			//we were in the proximity, we are no longer, send event exit
			parse->Event(EVENT_EXIT, d->GetNPCTypeID(), "", d, c);
		} else if(new_in && !old_in) {
			//we were not in the proximity, we are now, send enter event
			parse->Event(EVENT_ENTER, d->GetNPCTypeID(), "", d, c);
		}
	}
	
}

void EntityList::ProcessProximitySay(const char *Message, Client *c, int8 language) {

	if(!Message || !c)
		return;

	LinkedListIterator<NPC*> iterator(proximity_list);

	for(iterator.Reset(); iterator.MoreElements(); iterator.Advance()) {
		NPC *d = iterator.GetData();
		NPCProximity *l = d->proximity;
		if(l == NULL || !l->say)
			continue;

		if(   c->GetX() < l->min_x || c->GetX() > l->max_x
		   || c->GetY() < l->min_y || c->GetY() > l->max_y
		   || c->GetZ() < l->min_z || c->GetZ() > l->max_z )
			continue;

		parse->Event(EVENT_PROXIMITY_SAY, d->GetNPCTypeID(), Message, d, c, language);
	}
}

void EntityList::SaveAllClientsTaskState() {
	if(taskmanager) {
		list<Client*> ClonedClientList = client_list;

		for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
			Client* client = *itr;

			if(client) {
				if(client->IsTaskStateLoaded()) {
					client->SaveTaskState();
				}
			}
		}
	}
}

void EntityList::ReloadAllClientsTaskState(int TaskID) {
	if(taskmanager) {
		list<Client*> ClonedClientList = client_list;

		for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
			Client* client = *itr;

			if(client) {
				if(client->IsTaskStateLoaded()) {
					// If we have been passed a TaskID, only reload the client state if they have
					// that Task active.
					if((!TaskID) || (TaskID && client->IsTaskActive(TaskID))) {
						_log(TASKS__CLIENTLOAD, "Reloading Task State For Client %s", client->GetName());
						client->RemoveClientTaskState();
						client->LoadClientTaskState();
						taskmanager->SendActiveTasksToClient(client);
					}
				}
			}
		}
	}
}

bool EntityList::IsMobInZone(Mob *who) {
	bool Result = false;

	list<Mob*> ClonedMobList = mob_list;

	for(list<Mob*>::iterator itr = ClonedMobList.begin(); itr != ClonedMobList.end(); itr++) {
		Mob* mob = *itr;

		if(mob == who) {
			Result = true;
			break;
		}
	}

	return Result;
}

/*
Code to limit the ammount of certain NPCs in a given zone.
Primarily used to make a named mob unique within the zone, but written
to be more generic allowing limits larger than 1.

Maintain this stuff in a seperate list since the number
of limited NPCs will most likely be much smaller than the number
of NPCs in the entire zone.
*/
void EntityList::LimitAddNPC(NPC *npc) {
	if(!npc)
		return;
	
	SpawnLimitRecord r;
	
	int16 eid = npc->GetID();
	r.spawngroup_id = npc->GetSp2();
	r.npc_type = npc->GetNPCTypeID();
	
	npc_limit_list[eid] = r;
}

void EntityList::LimitRemoveNPC(NPC *npc) {
	if(!npc)
		return;
	
	int16 eid = npc->GetID();
	npc_limit_list.erase(eid);
}

//check a limit over the entire zone.
//returns true if the limit has not been reached
bool EntityList::LimitCheckType(int32 npc_type, int count) {
	if(count < 1)
		return(true);
	
	map<int32, SpawnLimitRecord>::iterator cur,end;
	cur = npc_limit_list.begin();
	end = npc_limit_list.end();
	
	for(; cur != end; cur++) {
		if(cur->second.npc_type == npc_type) {
			count--;
			if(count == 0) {
				return(false);
			}
		}
	}
	return(true);
}

//check limits on an npc type in a given spawn group.
//returns true if the limit has not been reached
bool EntityList::LimitCheckGroup(int32 spawngroup_id, int count) {
	if(count < 1)
		return(true);
	
	map<int32, SpawnLimitRecord>::iterator cur,end;
	cur = npc_limit_list.begin();
	end = npc_limit_list.end();
	
	for(; cur != end; cur++) {
		if(cur->second.spawngroup_id == spawngroup_id) {
			count--;
			if(count == 0) {
				return(false);
			}
		}
	}
	return(true);
}

//check limits on an npc type in a given spawn group, and
//checks limits on the entire zone in one pass.
//returns true if neither limit has been reached
bool EntityList::LimitCheckBoth(int32 npc_type, int32 spawngroup_id, int group_count, int type_count) {
	if(group_count < 1 && type_count < 1)
		return(true);
	
	map<int32, SpawnLimitRecord>::iterator cur,end;
	cur = npc_limit_list.begin();
	end = npc_limit_list.end();
	
	for(; cur != end; cur++) {
		if(cur->second.npc_type == npc_type) {
			type_count--;
			if(type_count == 0) {
				return(false);
			}
		}
		if(cur->second.spawngroup_id == spawngroup_id) {
			group_count--;
			if(group_count == 0) {
				return(false);
			}
		}
	}
	return(true);
}

void EntityList::RadialSetLogging(Mob *around, bool enabled, bool clients, bool non_clients, float range) {
	if(around) {
		float range2 = range * range;

		list<Mob*> ClonedMobList = mob_list;

		for(list<Mob*>::iterator itr = ClonedMobList.begin(); itr != ClonedMobList.end(); itr++) {
			Mob* mob = *itr;

			if(mob) {
				if(mob->IsClient()) {
					if(!clients)
						continue;
				} 
				else {
					if(!non_clients)
						continue;
				}

				if(around->DistNoRoot(*mob) > range2)
					continue;

				if(enabled)
					mob->EnableLogging();
				else
					mob->DisableLogging();
			}
		}
	}
}

void EntityList::UpdateHoTT(Mob* target) {
	if(target) {
		list<Client*> ClonedClientList = client_list;

		for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
			Client* client = *itr;

			if(client) {
				if(client->GetTarget() == target) {
					if (target->GetTarget()) 
						client->SetHoTT(target->GetTarget()->GetID());
					else
						client->SetHoTT(0);
				}
			}
		}
	}
}

void EntityList::DestroyTempPets(Mob *owner) {
	list<NPC*> ClonedNPCList = npc_list;

	for(list<NPC*>::iterator itr = ClonedNPCList.begin(); itr != ClonedNPCList.end(); itr++) {
		NPC* npc = *itr;

		if(npc) {
			if(npc->GetSwarmInfo()) {
				if(npc->GetSwarmInfo()->owner == owner) {
					npc->Depop();
				}
			}
		}
	}
}

bool Entity::CheckCoordLosNoZLeaps(float cur_x, float cur_y, float cur_z, float trg_x, float trg_y, float trg_z, float perwalk) {
	if(zone->zonemap == NULL) {
		return(true);
	}
	VERTEX myloc;
	VERTEX oloc;
	VERTEX hit;

	myloc.x = cur_x;
	myloc.y = cur_y;
	myloc.z = cur_z+5;

	oloc.x = trg_x;
	oloc.y = trg_y;
	oloc.z = trg_z+5;

	if (myloc.x == oloc.x && myloc.y == oloc.y && myloc.z == oloc.z)
		return true;

	FACE *onhit;

	if (!zone->zonemap->LineIntersectsZoneNoZLeaps(myloc,oloc,perwalk,&hit,&onhit))
		return true;

	return false;
}

void EntityList::QuestJournalledSayClose(Mob *sender, Client *QuestInitiator, float dist, const char* mobname, const char* message) {
	float dist2 = dist * dist;

	// Send the message to the quest initiator such that the client will enter it into the NPC Quest Journal
	if(QuestInitiator) {

		char *buf = new char[strlen(mobname) + strlen(message) + 10];
		sprintf(buf, "%s says, '%s'", mobname, message);
		QuestInitiator->QuestJournalledMessage(mobname, buf);
		safe_delete_array(buf);
	}

	// Use the old method for all other nearby clients
	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* client = *itr;

		if(client) {
			if((client != QuestInitiator) && client->DistNoRoot(*sender) <= dist2)
				client->Message_StringID(10, GENERIC_SAY, mobname, message);
		}
	}
}

Corpse* EntityList::GetClosestCorpse(Mob* sender) {
	Corpse* Result = 0;

	if(sender) {
		uint32 dist = 4294967295;

		list<Corpse*> ClonedCorpseList = corpse_list;

		for(list<Corpse*>::iterator itr = ClonedCorpseList.begin(); itr != ClonedCorpseList.end(); itr++) {
			Corpse* corpse = *itr;

			if(corpse) {
				uint32 nd = ((corpse->GetY() - sender->GetY()) * (corpse->GetY() - sender->GetY())) + ((corpse->GetX() - sender->GetX()) * (corpse->GetX() - sender->GetX()));
				
				if(nd < dist){
					dist = nd;
					Result = corpse;
					break;
				}
			}
		}
	}

	return Result;
}

void EntityList::ForceGroupUpdate(int32 gid) {
	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* client = *itr;

		if(client) {
			Group *g = NULL;
			g = client->GetGroup();
			if(g) {
				if(g->GetID() == gid) {
					database.RefreshGroupFromDB(client);
				}
			}
		}
	}
}

void EntityList::SendGroupLeave(int32 gid, const char *name) {
	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* client = *itr;

		if(client) {
			Group *g = NULL;

			g = client->GetGroup();

			if(g) {
				if(g->GetID() == gid) {
					EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate,sizeof(GroupJoin_Struct));
					GroupJoin_Struct* gj = (GroupJoin_Struct*) outapp->pBuffer;	
					strcpy(gj->membername, name);
					gj->action = groupActLeave;
					strcpy(gj->yourname, name);
					Mob *Leader = g->GetLeader();

					if(Leader)
						Leader->CastToClient()->GetGroupAAs(&gj->leader_aas);

					client->QueuePacket(outapp);
					safe_delete(outapp);
					g->DelMemberOOZ(name);

					if(g->IsLeader(client) && client->IsLFP())
						client->UpdateLFP();
				}
			}
		}
	}
}

void EntityList::SendGroupJoin(int32 gid, const char *name) {
	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* client = *itr;

		if(client) {
			Group *g = NULL;

			g = client->GetGroup();

			if(g) {
				if(g->GetID() == gid) {
					EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
					GroupJoin_Struct* gj = (GroupJoin_Struct*) outapp->pBuffer;	
					strcpy(gj->membername, name);
					gj->action = groupActJoin;
					strcpy(gj->yourname, client->GetName());
					Mob *Leader = g->GetLeader();
					
					if(Leader)
						Leader->CastToClient()->GetGroupAAs(&gj->leader_aas);
					
					client->QueuePacket(outapp);
					safe_delete(outapp);
				}
			}
		}
	}
}

void EntityList::GroupMessage(int32 gid, const char *from, const char *message) {
	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* client = *itr;

		if(client) {
			Group *g = NULL;

			g = client->GetGroup();

			if(g) {
				if(g->GetID() == gid) {
					client->ChannelMessageSend(from, client->GetName(), 2, 0, message);
				}
			}
		}
	}
}

void EntityList::CreateGroundObject(int32 itemid, float x, float y, float z, float heading, int32 decay_time)
{
	const Item_Struct* is = database.GetItem(itemid);
	if(is)
	{
		ItemInst *i = new ItemInst(is, is->MaxCharges);
		if(i)
		{
			Object* object = new Object(i,x,y,z,heading,decay_time);
			entity_list.AddObject(object, true);
			object->StartDecay();
			safe_delete(i);
		}
	}
}

Mob* EntityList::GetTargetForMez(Mob* caster) {
	Mob* Result = 0;

	if(caster) {
		list<Mob*> ClonedMobList = mob_list;

		for(list<Mob*>::iterator itr = ClonedMobList.begin(); itr != ClonedMobList.end(); itr++) {
			Mob* mob = *itr;

			if(mob) {
				if(mob == caster){ //caster can't pick himself
					continue;
				}

				if(caster->GetTarget() == mob){ //caster can't pick his target		
					continue;
				}

				if(!caster->CheckAggro(mob)){ //caster can't pick targets that aren't aggroed on himself
					continue;
				}

				if(caster->DistNoRoot(*mob) > 22250){ //only pick targets within 150 range
					continue;
				}

				if(!caster->CheckLosFN(mob)){ //this is wasteful but can't really think of another way to do it 
					//that wont have us trying to los the same target every time
					//it's only in combat so it's impact should be minimal.. but stil.
					continue;
				}
				
				Result = mob;
				break;
			}
		}
	}

	return Result;
}

void EntityList::SendZoneAppearance(Client *c) {
	if(c) {
		list<Mob*> ClonedMobList = mob_list;

		for(list<Mob*>::iterator itr = ClonedMobList.begin(); itr != ClonedMobList.end(); itr++) {
			Mob* cur = *itr;

			if(cur) {
				if(cur == c) {
					continue;
				}

				if(cur->GetAppearance() != ANIM_STAND) {
					cur->SendAppearancePacket(AT_Anim, cur->GetAppearanceValue(cur->GetAppearance()), false, true, c);
				}
			}
		}
	}
}

void EntityList::ZoneWho(Client *c, Who_All_Struct* Who) {
	// This is only called for SoF clients, as regular /who is now handled server-side for that client.
	int32 PacketLength = 0;
	int32 Entries = 0;

	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* ClientEntry = *itr;

		if(ClientEntry) {
			if(ClientEntry->GMHideMe(c))
				continue;

			if((Who->wrace != 0xFFFFFFFF) && (ClientEntry->GetRace() != Who->wrace))
				continue;

			if((Who->wclass != 0xFFFFFFFF) && (ClientEntry->GetClass() != Who->wclass))
				continue;

			if((Who->lvllow != 0xFFFFFFFF) && (ClientEntry->GetLevel() < Who->lvllow))
				continue;

			if((Who->lvlhigh != 0xFFFFFFFF) && (ClientEntry->GetLevel() > Who->lvlhigh))
				continue;

			if(Who->guildid != 0xFFFFFFFF) {
				if((Who->guildid == 0xFFFFFFFC) && !ClientEntry->IsTrader())
					continue;

				if((Who->guildid == 0xFFFFFFFB) && !ClientEntry->IsBuyer())
					continue;

				if(Who->guildid != ClientEntry->GuildID())
					continue;
			}

			Entries++;

			PacketLength = PacketLength + strlen(ClientEntry->GetName());

			if(strlen(guild_mgr.GetGuildName(ClientEntry->GuildID())) > 0) 
				PacketLength = PacketLength + strlen(guild_mgr.GetGuildName(ClientEntry->GuildID())) + 2;
		}
	}

	PacketLength = PacketLength + sizeof(WhoAllReturnStruct) + (47 * Entries);

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_WhoAllResponse, PacketLength);

	char *Buffer = (char *)outapp->pBuffer;

	WhoAllReturnStruct *WARS = (WhoAllReturnStruct *)Buffer;

	WARS->id = 0;

	WARS->playerineqstring = 5001;

	strcpy(WARS->line, "---------------------------");

	WARS->unknown35 = 0x0a;

	WARS->unknown36 = 0;
	
	switch(Entries) {
		case 0:
			WARS->playersinzonestring = 5029;
			break;
		case 1:
			WARS->playersinzonestring = 5028; // 5028 There is %1 player in EverQuest.
			break;
		default:
			WARS->playersinzonestring = 5036; // 5036 There are %1 players in EverQuest.
	}

	WARS->unknown44[0] = 0;

	WARS->unknown44[1] = 0;

	WARS->unknown52 = Entries;

	WARS->unknown56 = Entries;

	WARS->playercount = Entries;

	Buffer += sizeof(WhoAllReturnStruct);

	for(list<Client*>::iterator itr = client_list.begin(); itr != client_list.end(); itr++) {
		Client* ClientEntry = *itr;

		if(ClientEntry) {
			if(ClientEntry->GMHideMe(c)) 
				continue;

			if((Who->wrace != 0xFFFFFFFF) && (ClientEntry->GetRace() != Who->wrace)) 
				continue;

			if((Who->wclass != 0xFFFFFFFF) && (ClientEntry->GetClass() != Who->wclass)) 
				continue;

			if((Who->lvllow != 0xFFFFFFFF) && (ClientEntry->GetLevel() < Who->lvllow)) 
				continue;

			if((Who->lvlhigh != 0xFFFFFFFF) && (ClientEntry->GetLevel() > Who->lvlhigh)) 
				continue;

			if(Who->guildid != 0xFFFFFFFF) {
				if((Who->guildid == 0xFFFFFFFC) && !ClientEntry->IsTrader()) 
					continue;

				if((Who->guildid == 0xFFFFFFFB) && !ClientEntry->IsBuyer()) 
					continue;

				if(Who->guildid != ClientEntry->GuildID()) 
					continue;
			}

			string GuildName;

			if((ClientEntry->GuildID() != GUILD_NONE) && (ClientEntry->GuildID() > 0)) {
				GuildName = "<";

				GuildName += guild_mgr.GetGuildName(ClientEntry->GuildID());

				GuildName += ">";
			}

			int32 FormatMSGID=5025; // 5025 %T1[%2 %3] %4 (%5) %6 %7 %8 %9

			if(ClientEntry->GetAnon() == 1)
				FormatMSGID = 5024; // 5024 %T1[ANONYMOUS] %2 %3
			else if(ClientEntry->GetAnon() == 2)
				FormatMSGID = 5023; // 5023 %T1[ANONYMOUS] %2 %3 %4

			int32 PlayerClass = 0;

			int32 PlayerLevel = 0;

			int32 PlayerRace = 0;

			int32 ZoneMSGID = 0xFFFFFFFF;

			if(ClientEntry->GetAnon()==0) {
				PlayerClass = ClientEntry->GetClass();

				PlayerLevel = ClientEntry->GetLevel();

				PlayerRace = ClientEntry->GetRace();
			}				

			WhoAllPlayerPart1* WAPP1 = (WhoAllPlayerPart1*)Buffer;

			WAPP1->FormatMSGID = FormatMSGID;

			WAPP1->PIDMSGID = 0xFFFFFFFF;

			strcpy(WAPP1->Name, ClientEntry->GetName());

			Buffer += sizeof(WhoAllPlayerPart1) + strlen(WAPP1->Name);

			WhoAllPlayerPart2* WAPP2 = (WhoAllPlayerPart2*)Buffer;

			if(ClientEntry->Admin() >= 10)
				WAPP2->RankMSGID = 12312;
			else
				WAPP2->RankMSGID = 0xFFFFFFFF;

			strcpy(WAPP2->Guild, GuildName.c_str());

			Buffer += sizeof(WhoAllPlayerPart2) + strlen(WAPP2->Guild);

			WhoAllPlayerPart3* WAPP3 = (WhoAllPlayerPart3*)Buffer;

			WAPP3->Unknown80[0] = 0xFFFFFFFF;

			if(ClientEntry->IsLD())
				WAPP3->Unknown80[1] = 12313; // LinkDead
			else
				WAPP3->Unknown80[1] = 0xFFFFFFFF;

			WAPP3->ZoneMSGID = ZoneMSGID;

			WAPP3->Zone = 0;

			WAPP3->Class_ = PlayerClass;

			WAPP3->Level =  PlayerLevel;

			WAPP3->Race = PlayerRace;

			WAPP3->Account[0] = 0;

			Buffer += sizeof(WhoAllPlayerPart3);

			WhoAllPlayerPart4* WAPP4 = (WhoAllPlayerPart4*)Buffer;

			WAPP4->Unknown100 = 0;

			Buffer += sizeof(WhoAllPlayerPart4);
		}
	}

	c->QueuePacket(outapp);

	safe_delete(outapp);
}

void EntityList::UnMarkNPC(int16 ID) {
	// Designed to be called from the Mob destructor, this method calls Group::UnMarkNPC for
	// each group to remove the dead mobs entity ID from the groups list of NPCs marked via the
	// Group Leadership AA Mark NPC ability.
	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* client = *itr;

		if(client) {
			Group *g = NULL;

			g = client->GetGroup();

			if(g)
				g->UnMarkNPC(ID);
		}
	}
}

int32 EntityList::CheckNPCsClose(Mob *center) {
    int32 count = 0;

	list<NPC*> ClonedNPCList = npc_list;

	for(list<NPC*>::iterator itr = ClonedNPCList.begin(); itr != ClonedNPCList.end(); itr++) {
		NPC* current = *itr;

		if(current) {
			if(current == center) {
				continue;
			}

			if(current->IsPet()) {
				continue;
			}

			if(current->GetClass() == LDON_TREASURE) {
				continue;
			}

			if(current->GetBodyType() == BT_NoTarget || current->GetBodyType() == BT_Special) {
				continue;
			}

			float xDiff = current->GetX() - center->GetX();
			float yDiff = current->GetY() - center->GetY();
			float zDiff = current->GetZ() - center->GetZ();
			float dist = ((xDiff * xDiff) + (yDiff * yDiff) + (zDiff * zDiff));

			if(dist <= RuleR(Adventure, DistanceForRescueAccept)) {
				count++;
			}
		}
	}

	return count;
}

void EntityList::GateAllClients() {
	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* c = *itr;

		if(c) {
			c->GoToBind();
		}
	}
}

void EntityList::SendAdventureUpdate(int32 a_id) {
	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* c = *itr;

		if(c) {
			AdventureDetails *ad = c->GetCurrentAdventure();
			if(ad && ad->id == a_id) {
				c->SendAdventureDetail();
			}
		}
	}
}

void EntityList::AdventureMessage(int32 a_id, const char *msg) {
	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* c = *itr;

		if(c) {
			AdventureDetails *ad = c->GetCurrentAdventure();
			if(ad && ad->id == a_id) {
				c->Message(13, msg);
			}
		}
	}
}

void EntityList::AdventureFinish(int32 a_id, int8 win_lose, int32 points) {
	AdventureDetails *ad = NULL;

	std::map<uint32, AdventureDetails*>::iterator aa_iter;
	aa_iter = zone->active_adventures.find(a_id);

	if(aa_iter == zone->active_adventures.end())
		return;

	ad = aa_iter->second;

	if(!ad)
		return;

	if(!ad->ai)
		return;

	if(ad->status != 3) {
		ad->status = 3;

		list<Client*> ClonedClientList = client_list;

		for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
			Client* c = *itr;

			if(c) {
				AdventureDetails *ad = c->GetCurrentAdventure();

				if(ad && ad->id == a_id) {
					c->SendAdventureFinish(win_lose, points, ad->ai->theme);
				}
			}
		}
	}
}

void EntityList::AdventureDestroy(int32 a_id) {
	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* c = *itr;

		if(c) {
			AdventureDetails *ad = c->GetCurrentAdventure();
			if(ad && ad->id == a_id) {
				c->SendAdventureError("You are not currently in an adventure.");
				c->SetCurrentAdventure(NULL);
			}
		}
	}
}

void EntityList::AdventureCountUpdate(int32 a_id, int32 current, int32 total) {
	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* c = *itr;

		if(c) {
			AdventureDetails *ad = c->GetCurrentAdventure();
			if(ad && ad->id == a_id) {
				c->SendAdventureCountUpdate(current, total);
			}
		}
	}
}

void EntityList::SignalAllClients(int32 data) {
	list<Client*> ClonedClientList = client_list;

	for(list<Client*>::iterator itr = ClonedClientList.begin(); itr != ClonedClientList.end(); itr++) {
		Client* ent = *itr;

		if(ent) {
			ent->Signal(data);
		}
	}
}