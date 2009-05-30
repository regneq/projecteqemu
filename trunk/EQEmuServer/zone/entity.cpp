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

void Entity::SetID(int16 set_id) {
	id = set_id;
}

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



EntityList::EntityList() {
	last_insert_id = 0;
}

EntityList::~EntityList() {
	//must call this before the list is destroyed, or else it will try to
	//delete the NPCs in the list, which it cannot do.
	RemoveAllLocalities();
}

bool EntityList::CanAddHateForMob(Mob *p) {
    LinkedListIterator<NPC*> iterator(npc_list);
    int count = 0;

    iterator.Reset();
    while( iterator.MoreElements())
    {
        NPC *npc=iterator.GetData();
        if (npc->IsOnHatelist(p))
            count++;
        // no need to continue if we already hit the limit
        if (count > 3)
            return false;
        iterator.Advance();
    }

    if (count <= 2)
        return true;
    return false;
}

void EntityList::AddClient(Client* client) {
	client->SetID(GetFreeID());
	client_list.Insert(client);
	mob_list.Insert(client);
	if(!client_list.dont_delete)
		client_list.dont_delete=true;
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
	LinkedListIterator<Corpse*> iterator(corpse_list);
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
		net.corpse_timer.Disable();//No corpses in list, disable until one is added
}

void EntityList::MobProcess() {
#ifdef IDLE_WHEN_EMPTY
	if(numclients < 1)
		return;
#endif
	_ZP(EntityList_MobProcess);
	LinkedListIterator<Mob*> iterator(mob_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(!iterator.GetData())
		{
			iterator.Advance();
			continue;
		}
		if(!iterator.GetData()->Process()){
			Mob* mob=iterator.GetData();
			if(mob->IsNPC())
				entity_list.RemoveNPC(mob->CastToNPC()->GetID());
			else{
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
			iterator.RemoveCurrent();
		}
		else
			iterator.Advance();
	}
}

void EntityList::BeaconProcess() {
	_ZP(EntityList_BeaconProcess);
	LinkedListIterator<Beacon *> iterator(beacon_list);
	int count;

	for(iterator.Reset(), count = 0; iterator.MoreElements(); count++)
	{
		if(!iterator.GetData()->Process())
			iterator.RemoveCurrent();
		else
			iterator.Advance();
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
		corpse->SetID(GetFreeID());
	else
		corpse->SetID(in_id);
	corpse->CalcCorpseName();
	corpse_list.Insert(corpse);
	if(!net.corpse_timer.Enabled())
		net.corpse_timer.Start();
}

void EntityList::AddNPC(NPC* npc, bool SendSpawnPacket, bool dontqueue) {
	npc->SetID(GetFreeID());
	
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
	
	npc_list.Insert(npc);
	if(!npc_list.dont_delete)
		npc_list.dont_delete=true;
	mob_list.Insert(npc);
};
void EntityList::AddObject(Object* obj, bool SendSpawnPacket) {
	obj->SetID(GetFreeID()); 
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
};

void EntityList::AddDoor(Doors* door) {
	door->SetEntityID(GetFreeID());
	door_list.Insert(door);
	if(!net.door_timer.Enabled())
		net.door_timer.Start();
}

void EntityList::AddTrap(Trap* trap) {
	trap->SetID(GetFreeID());
	trap_list.Insert(trap);
	if(!net.trap_timer.Enabled())
		net.trap_timer.Start();
}

void EntityList::AddBeacon(Beacon *beacon)
{
	beacon->SetID(GetFreeID());
	beacon_list.Insert(beacon);
}

void EntityList::AddToSpawnQueue(int16 entityid, NewSpawn_Struct** ns) {
	int32 count;
	if((count=(client_list.Count()))==0)
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
Entity* EntityList::GetEntityMob(int16 id){
	LinkedListIterator<Mob*> iterator(mob_list);
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
Entity* EntityList::GetEntityMob(const char *name)
{
	if (name == 0)
		return 0;

	LinkedListIterator<Mob*> iterator(mob_list);
	
	for(iterator.Reset(); iterator.MoreElements(); iterator.Advance())
	{
		if (strcasecmp(iterator.GetData()->GetName(), name) == 0)
		{
			return iterator.GetData();
		}
	}
	return 0;
}
Entity* EntityList::GetEntityDoor(int16 id){
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
Entity* EntityList::GetEntityCorpse(int16 id){
	LinkedListIterator<Corpse*> iterator(corpse_list);
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
Entity* EntityList::GetEntityCorpse(const char *name)
{
	if (name == 0)
		return 0;

	LinkedListIterator<Corpse*> iterator(corpse_list);
	
	for(iterator.Reset(); iterator.MoreElements(); iterator.Advance())
	{
		if (strcasecmp(iterator.GetData()->GetName(), name) == 0)
		{
			return iterator.GetData();
		}
	}
	return 0;
}

Entity* EntityList::GetEntityTrap(int16 id){
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

Entity* EntityList::GetEntityObject(int16 id){
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
Entity* EntityList::GetEntityBeacon(int16 id) {
	LinkedListIterator<Beacon*> iterator(beacon_list);
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
Entity* EntityList::GetID(int16 get_id)
{
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

Mob* EntityList::GetMob(int16 get_id)
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

Mob* EntityList::GetMobByNpcTypeID(int32 get_id)
{
	if (get_id == 0)
		return 0;
	LinkedListIterator<Mob*> iterator(mob_list);
	
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->GetNPCTypeID() == get_id)
		{
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return 0;
}

int16 EntityList::GetFreeID()
{
	if(last_insert_id > 1500)
		last_insert_id = 0;
	int16 getid=last_insert_id;
	while(1)
	{
		getid++;
		if (GetID(getid) == 0)
		{
			last_insert_id = getid;
			return getid;
		}
	}
}

// if no language skill is specified, sent with 100 skill
void EntityList::ChannelMessage(Mob* from, int8 chan_num, int8 language, const char* message, ...) {
	ChannelMessage(from, chan_num, language, 100, message);
}

void EntityList::ChannelMessage(Mob* from, int8 chan_num, int8 language, int8 lang_skill, const char* message, ...) {
	LinkedListIterator<Client*> iterator(client_list);
	va_list argptr;
	char buffer[4096];
	
	va_start(argptr, message);
	vsnprintf(buffer, 4096, message, argptr);
	va_end(argptr);
	
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Client* client = iterator.GetData();
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
		iterator.Advance();
	}
}

void EntityList::ChannelMessageSend(Mob* to, int8 chan_num, int8 language, const char* message, ...) {
	LinkedListIterator<Client*> iterator(client_list);
	va_list argptr;
	char buffer[4096];
	va_start(argptr, message);
	vsnprintf(buffer, 4096, message, argptr);
	va_end(argptr);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Client* client = iterator.GetData();
		if (client->GetID() == to->GetID()) {
			client->ChannelMessageSend(0, 0, chan_num, language, buffer);
			break;
		}
		iterator.Advance();
	}
}

void EntityList::SendZoneSpawns(Client* client)
{
	LinkedListIterator<Mob*> iterator(mob_list);
	
	EQApplicationPacket* app;
	iterator.Reset();
	while(iterator.MoreElements()) {
		Mob* ent = iterator.GetData();
		if (!( ent->InZone() ) || (ent->IsClient() && ent->CastToClient()->GMHideMe(client))) {
			iterator.Advance();
			continue;
		}
		app = new EQApplicationPacket;
		iterator.GetData()->CastToMob()->CreateSpawnPacket(app); // TODO: Use zonespawns opcode instead
        client->QueuePacket(app, true, Client::CLIENT_CONNECTED);
		safe_delete(app);
		iterator.Advance();
	}	
}

void EntityList::SendZoneSpawnsBulk(Client* client)
{
	//float rate = client->Connection()->GetDataRate();
	LinkedListIterator<Mob*> iterator(mob_list);
	NewSpawn_Struct ns;
	Mob *spawn;
	int32 maxspawns=100;

	//rate = rate > 1.0 ? (rate < 10.0 ? rate : 10.0) : 1.0;
	//maxspawns = (int32)rate * SPAWNS_PER_POINT_DATARATE; // FYI > 10240 entities will cause BulkZoneSpawnPacket to throw exception
	if(maxspawns > mob_list.Count())
		maxspawns = mob_list.Count();
	BulkZoneSpawnPacket* bzsp = new BulkZoneSpawnPacket(client, maxspawns);
	for(iterator.Reset(); iterator.MoreElements(); iterator.Advance())
	{
		spawn = iterator.GetData();
		if(spawn && spawn->InZone())
		{
			if(spawn->IsClient() && spawn->CastToClient()->GMHideMe(client))
				continue;
			memset(&ns, 0, sizeof(NewSpawn_Struct));
			spawn->FillSpawnStruct(&ns, client);
			bzsp->AddSpawn(&ns);
		}
	}
	safe_delete(bzsp);
}

//this is a hack to handle a broken spawn struct
void EntityList::SendZonePVPUpdates(Client *to) {
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();
	while(iterator.MoreElements()) {
		Client *c = iterator.GetData();
		if(c->GetPVP())
			c->SendAppearancePacket(AT_PVP, c->GetPVP(), true, false, to);
		iterator.Advance();
	}
}

void EntityList::SendZoneCorpses(Client* client)
{
	EQApplicationPacket* app;
	LinkedListIterator<Corpse*> iterator(corpse_list);
	
	for(iterator.Reset(); iterator.MoreElements(); iterator.Advance())
	{
		Corpse *ent = iterator.GetData();
		app = new EQApplicationPacket;
		ent->CreateSpawnPacket(app);
		client->QueuePacket(app, true, Client::CLIENT_CONNECTED);
		safe_delete(app);
	}	
}

void EntityList::SendZoneCorpsesBulk(Client* client) {
	//float rate = client->Connection()->GetDataRate();
	LinkedListIterator<Corpse*> iterator(corpse_list);
	NewSpawn_Struct ns;
	Corpse *spawn;
	int32 maxspawns=100;

	//rate = rate > 1.0 ? (rate < 10.0 ? rate : 10.0) : 1.0;
	//maxspawns = (int32)rate * SPAWNS_PER_POINT_DATARATE; // FYI > 10240 entities will cause BulkZoneSpawnPacket to throw exception
	BulkZoneSpawnPacket* bzsp = new BulkZoneSpawnPacket(client, maxspawns);
	
	for(iterator.Reset(); iterator.MoreElements(); iterator.Advance())
	{
		spawn = iterator.GetData();
		if(spawn && spawn->InZone())
		{
			memset(&ns, 0, sizeof(NewSpawn_Struct));
			spawn->FillSpawnStruct(&ns, client);
			bzsp->AddSpawn(&ns);
		}
	}
	safe_delete(bzsp);
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

void EntityList::Save()
{
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();
	while(iterator.MoreElements())
	{
		iterator.GetData()->Save();
		iterator.Advance();
	}	
}

void EntityList::ReplaceWithTarget(Mob* pOldMob, Mob*pNewTarget)
{
	if(!pNewTarget)
		return;
	LinkedListIterator<Mob*> iterator(mob_list);
	
	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->IsAIControlled()) {
            // replace the old mob with the new one
			if (iterator.GetData()->RemoveFromHateList(pOldMob))
                    iterator.GetData()->AddToHateList(pNewTarget, 1, 0);
		}
		iterator.Advance();
	}
}

void EntityList::RemoveFromTargets(Mob* mob)
{
	LinkedListIterator<Mob*> iterator(mob_list);
	
	iterator.Reset();
	while(iterator.MoreElements()) {
		iterator.GetData()->RemoveFromHateList(mob);
		iterator.Advance();
	}	
}

void EntityList::QueueClientsByTarget(Mob* sender, const EQApplicationPacket* app, bool iSendToSender, Mob* SkipThisMob, bool ackreq) {
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();
	while(iterator.MoreElements()) {
		if ((iSendToSender || (iterator.GetData() != sender && (iterator.GetData()->GetTarget() == sender || 
			(iterator.GetData()->GetTarget() && iterator.GetData()->GetTarget()->GetTarget() && iterator.GetData()->GetTarget()->GetTarget() == sender)))) 
			&& iterator.GetData() != SkipThisMob) {
			iterator.GetData()->QueuePacket(app, ackreq);
		}
		iterator.Advance();
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
	
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();
	while(iterator.MoreElements()) {

		Client* ent = iterator.GetData();

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
		iterator.Advance();
	}
}

//sender can be null
void EntityList::QueueClients(Mob* sender, const EQApplicationPacket* app, bool ignore_sender, bool ackreq) {
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


void EntityList::QueueClientsStatus(Mob* sender, const EQApplicationPacket* app, bool ignore_sender, int8 minstatus, int8 maxstatus)
{
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if ((!ignore_sender || iterator.GetData() != sender) && (iterator.GetData()->Admin() >= minstatus && iterator.GetData()->Admin() <= maxstatus))
		{
			iterator.GetData()->QueuePacket(app);
		}
		iterator.Advance();
	}	
}

void EntityList::DuelMessage(Mob* winner, Mob* loser, bool flee) {
	LinkedListIterator<Client*> iterator(client_list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		Client *cur = iterator.GetData();
		//might want some sort of distance check in here?
		if (cur != winner && cur != loser)
		{
			if (flee)
				cur->Message_StringID(15, DUEL_FLED, winner->GetName(),loser->GetName(),loser->GetName());
			else
				cur->Message_StringID(15, DUEL_FINISHED, winner->GetName(),loser->GetName());
		}
		iterator.Advance();
	}
}

Client* EntityList::GetClientByName(const char *checkname) {
	LinkedListIterator<Client*> iterator(client_list); 
	
	iterator.Reset(); 
	while(iterator.MoreElements()) 
	{ 
		if (strcasecmp(iterator.GetData()->GetName(), checkname) == 0) {
			return iterator.GetData();
		}
		iterator.Advance(); 
	} 
	return 0; 
}

Client* EntityList::GetClientByCharID(int32 iCharID) {
	LinkedListIterator<Client*> iterator(client_list); 
	
	iterator.Reset(); 
	while(iterator.MoreElements()) { 
		if (iterator.GetData()->CharacterID() == iCharID) {

			return iterator.GetData();
		}
		iterator.Advance(); 
	} 
	return 0; 
}

Client* EntityList::GetClientByWID(int32 iWID) {
	LinkedListIterator<Client*> iterator(client_list); 
	
	iterator.Reset(); 
	while(iterator.MoreElements()) {  
		if (iterator.GetData()->GetWID() == iWID) {
			return iterator.GetData();
		} 
		iterator.Advance(); 
	} 
	return 0; 
}

Corpse*	EntityList::GetCorpseByOwner(Client* client){
	LinkedListIterator<Corpse*> iterator(corpse_list); 
	
	iterator.Reset(); 
	while(iterator.MoreElements()) 
	{ 
		if (iterator.GetData()->IsPlayerCorpse()) 
		{ 
			if (strcasecmp(iterator.GetData()->GetOwnerName(), client->GetName()) == 0) {
				return iterator.GetData();
			}
		} 
		iterator.Advance(); 
	} 
	return 0; 
}
Corpse* EntityList::GetCorpseByID(int16 id){
	LinkedListIterator<Corpse*> iterator(corpse_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->id == id) {
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return 0;
}

	Corpse* EntityList::GetCorpseByDBID(int32 dbid){
	LinkedListIterator<Corpse*> iterator(corpse_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->GetDBID() == dbid) {
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return 0;
}

Corpse* EntityList::GetCorpseByName(char* name){
	LinkedListIterator<Corpse*> iterator(corpse_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (strcmp(iterator.GetData()->GetName(),name)==0) {
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return 0;
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

#ifdef EQBOTS

// EQoffline
BotRaids* EntityList::GetBotRaidByMob(Mob *mr) {
	list<BotRaids *>::iterator iterator;
	iterator = botraid_list.begin();
	while(iterator != botraid_list.end()) {
		if((*iterator)->IsBotRaidMember(mr)) {
			return *iterator;
		}
		iterator++;
	}
	return 0;
}

void EntityList::AddBotRaid(BotRaids* br) {
	if(br == NULL)
		return;
	
	int16 gid = GetFreeID();
	if(gid == 0) {
		LogFile->write(EQEMuLog::Error, "Unable to get new Raid ID from world server. Raid is going to be broken.");
		return;
	}
	
	AddBotRaid(br, gid);
}

void EntityList::AddBotRaid(BotRaids* br, int16 gid) {
	br->SetBotRaidID(gid);
	botraid_list.push_back(br);
}

bool EntityList::RemoveBotRaid(int16 delete_id) {
	list<BotRaids *>::iterator iterator;
	iterator = botraid_list.begin();
	while(iterator != botraid_list.end()) {
		if((*iterator)->GetBotRaidID() == delete_id) {
			botraid_list.remove (*iterator);
			return true;
		}
		iterator++;
	}
	return false;
}

#endif //EQBOTS

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

Client* EntityList::GetClientByAccID(int32 accid) 
{ 
	LinkedListIterator<Client*> iterator(client_list); 
	
	iterator.Reset(); 
	while(iterator.MoreElements()) 
	{ 
		if (iterator.GetData()->AccountID() == accid) {
			return iterator.GetData();
		}
		iterator.Advance(); 
	} 
	return 0; 
} 
Client* EntityList::GetClientByID(int16 id) { 
	LinkedListIterator<Client*> iterator(client_list); 
	
	iterator.Reset(); 
	while(iterator.MoreElements()) 
	{ 
		if (iterator.GetData()) 
		{ 
			if (iterator.GetData()->GetID() == id) {
				return iterator.GetData();
			}
		} 
		iterator.Advance(); 
	} 
	return 0; 
} 

void EntityList::ChannelMessageFromWorld(const char* from, const char* to, int8 chan_num, int32 guild_id, int8 language, const char* message) {
	
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();
	for(; iterator.MoreElements(); iterator.Advance())
	{
		Client* client = iterator.GetData();
		if(chan_num == 0) {
			if(!client->IsInGuild(guild_id))
				continue;
			if(!guild_mgr.CheckPermission(guild_id, client->GuildRank(), GUILD_HEAR))
				continue;
			if(client->GetFilter(FILTER_GUILDSAY) == FilterHide)
				continue;
		} else if(chan_num == 5) {
			if(client->GetFilter(FILTER_OOC) == FilterHide)
				continue;
		}
		client->ChannelMessageSend(from, to, chan_num, language, message);
	}
}

void EntityList::Message(int32 to_guilddbid, int32 type, const char* message, ...) {
	va_list argptr;
	char buffer[4096];
	
	va_start(argptr, message);
	vsnprintf(buffer, 4096, message, argptr);
	va_end(argptr);
	
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Client* client = iterator.GetData();
		if (to_guilddbid == 0 || client->IsInGuild(to_guilddbid))
			client->Message(type, buffer);
		iterator.Advance();
	}
}

void EntityList::QueueClientsGuild(Mob* sender, const EQApplicationPacket* app, bool ignore_sender, int32 guild_id){
	LinkedListIterator<Client*> iterator(client_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Client* client = iterator.GetData()->CastToClient();
		if (client->IsInGuild(guild_id))
			client->QueuePacket(app);
		iterator.Advance();
	}
}

void EntityList::MessageStatus(int32 to_guild_id, int to_minstatus, int32 type, const char* message, ...) {
	va_list argptr;
	char buffer[4096];
	
	va_start(argptr, message);
	vsnprintf(buffer, 4096, message, argptr);
	va_end(argptr);
	
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();
	while(iterator.MoreElements()) {
		Client* client = iterator.GetData();
		if ((to_guild_id == 0 || client->IsInGuild(to_guild_id)) && client->Admin() >= to_minstatus)
			client->Message(type, buffer);
		iterator.Advance();
	}
}

// works much like MessageClose, but with formatted strings
void EntityList::MessageClose_StringID(Mob *sender, bool skipsender, float dist, int32 type, int32 string_id, const char* message1,const char* message2,const char* message3,const char* message4,const char* message5,const char* message6,const char* message7,const char* message8,const char* message9)
{
	Client *c;
	LinkedListIterator<Client*> iterator(client_list);
	float dist2 = dist * dist;
	
	
	for(iterator.Reset(); iterator.MoreElements(); iterator.Advance())
	{
		c = iterator.GetData();
		if(c && c->DistNoRoot(*sender) <= dist2 && (!skipsender || c != sender))
			c->Message_StringID(type, string_id, message1, message2, message3, message4, message5, message6, message7, message8, message9);
	}
}

void EntityList::Message_StringID(Mob *sender, bool skipsender, int32 type, int32 string_id, const char* message1,const char* message2,const char* message3,const char* message4,const char* message5,const char* message6,const char* message7,const char* message8,const char* message9)
{
	Client *c;
	LinkedListIterator<Client*> iterator(client_list);
	
	
	for(iterator.Reset(); iterator.MoreElements(); iterator.Advance())
	{
		c = iterator.GetData();
		if(c && (!skipsender || c != sender))
			c->Message_StringID(type, string_id, message1, message2, message3, message4, message5, message6, message7, message8, message9);
	}
}

void EntityList::MessageClose(Mob* sender, bool skipsender, float dist, int32 type, const char* message, ...) {
	va_list argptr;
	char buffer[4096];
	
	va_start(argptr, message);
	vsnprintf(buffer, 4095, message, argptr);
	va_end(argptr);
	
	float dist2 = dist * dist;
	
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->DistNoRoot(*sender) <= dist2 && (!skipsender || iterator.GetData() != sender)) {
			iterator.GetData()->Message(type, buffer);
		}
		iterator.Advance();
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

void EntityList::RemoveAllMobs(){
	LinkedListIterator<Mob*> iterator(mob_list);
	iterator.Reset();
	while(iterator.MoreElements())
		iterator.RemoveCurrent();
}
void EntityList::RemoveAllClients(){
	LinkedListIterator<Client*> iterator(client_list);
	iterator.Reset();
	while(iterator.MoreElements())
		iterator.RemoveCurrent(false);
}
void EntityList::RemoveAllNPCs(){
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	while(iterator.MoreElements()) {

#ifdef EQBOTS

		if(iterator.GetData()->IsBot()) {
			database.CleanBotLeaderEntries(iterator.GetData()->GetNPCTypeID());
		}

#endif //EQBOTS

		iterator.RemoveCurrent(false);
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
void EntityList::RemoveAllCorpses(){
	LinkedListIterator<Corpse*> iterator(corpse_list);
	iterator.Reset();
	while(iterator.MoreElements())
		iterator.RemoveCurrent();
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
bool EntityList::RemoveMob(int16 delete_id){
	if(delete_id==0)
		return true;
	LinkedListIterator<Mob*> iterator(mob_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(iterator.GetData()->GetID()==delete_id){
			if(iterator.GetData()->IsNPC())
				entity_list.RemoveNPC(delete_id);
			else if(iterator.GetData()->IsClient())
				entity_list.RemoveClient(delete_id);
			iterator.RemoveCurrent();
			return true;
		}
		iterator.Advance();
	}
	return false;
}

bool EntityList::RemoveMob(Mob *delete_mob) {
	if(delete_mob==0)
		return true;
	LinkedListIterator<Mob*> iterator(mob_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(iterator.GetData()==delete_mob){
			iterator.RemoveCurrent();
			return true;
		}
		iterator.Advance();
	}
	return false;
}

bool EntityList::RemoveNPC(int16 delete_id){
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(iterator.GetData()->GetID()==delete_id){
			//make sure its proximity is removed
			RemoveProximity(iterator.GetData()->GetID());
			//take it out of the list
			iterator.RemoveCurrent(false);//Already Deleted
			//take it out of our limit list
			if(npc_limit_list.count(delete_id) == 1)
				npc_limit_list.erase(delete_id);
			return true;
		}
		iterator.Advance();
	}
	return false;
}
bool EntityList::RemoveClient(int16 delete_id){
	LinkedListIterator<Client*> iterator(client_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(iterator.GetData()->GetID()==delete_id){
			iterator.RemoveCurrent(false);//Already Deleted
			return true;
		}
		iterator.Advance();
	}
	return false;
}

bool EntityList::RemoveClient(Client *delete_client){
	LinkedListIterator<Client*> iterator(client_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(iterator.GetData()==delete_client){
			iterator.RemoveCurrent(false);//Already Deleted
			return true;
		}
		iterator.Advance();
	}
	return false;
}

bool EntityList::RemoveObject(int16 delete_id){
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
bool EntityList::RemoveTrap(int16 delete_id){
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
bool EntityList::RemoveDoor(int16 delete_id){
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
bool EntityList::RemoveCorpse(int16 delete_id){
	LinkedListIterator<Corpse*> iterator(corpse_list);
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
	if ((!worldserver.Connected()) || !ZoneLoaded)
		return;
	LinkedListIterator<Client*> iterator(client_list);
	int32 tmpNumUpdates = numclients + 5;
	ServerPacket* pack = 0;
	ServerClientListKeepAlive_Struct* sclka = 0;
	if (!iSendFullUpdate) {
		pack = new ServerPacket(ServerOP_ClientListKA, sizeof(ServerClientListKeepAlive_Struct) + (tmpNumUpdates * 4));
		sclka = (ServerClientListKeepAlive_Struct*) pack->pBuffer;
	}
	
	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->InZone()) {
			if (iSendFullUpdate) {
				iterator.GetData()->UpdateWho();
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
				sclka->wid[sclka->numupdates] = iterator.GetData()->GetWID();
				sclka->numupdates++;
			}
		}
		iterator.Advance();
	}
	if (!iSendFullUpdate) {
		pack->size = sizeof(ServerClientListKeepAlive_Struct) + (sclka->numupdates * 4);
		worldserver.SendPacket(pack);
		safe_delete(pack);
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

#ifdef EQBOTS

	else if(entity_list.RemoveBotRaid(id))
		return;

#endif //EQBOTS

	else 
		entity_list.RemoveObject(id);
}

void EntityList::Process()
{
	_ZP(EntityList_Process);
	CheckSpawnQueue();
}

void EntityList::CountNPC(int32* NPCCount, int32* NPCLootCount, int32* gmspawntype_count) {
	LinkedListIterator<NPC*> iterator(npc_list);
	*NPCCount = 0;
	*NPCLootCount = 0;
	
	iterator.Reset();
	while(iterator.MoreElements())	
	{
		(*NPCCount)++;
		(*NPCLootCount) += iterator.GetData()->CastToNPC()->CountLoot();
		if (iterator.GetData()->CastToNPC()->GetNPCTypeID() == 0)
			(*gmspawntype_count)++;
		iterator.Advance();
	}
}

void EntityList::DoZoneDump(ZSDump_Spawn2* spawn2_dump, ZSDump_NPC* npc_dump, ZSDump_NPC_Loot* npcloot_dump, NPCType* gmspawntype_dump) {
	int32 spawn2index = 0;
	int32 NPCindex = 0;
	int32 NPCLootindex = 0;
	int32 gmspawntype_index = 0;
	
	if (npc_dump != 0) {
		LinkedListIterator<NPC*> iterator(npc_list);
		NPC* npc = 0;
		iterator.Reset();
		while(iterator.MoreElements())	
		{
			npc = iterator.GetData()->CastToNPC();
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
			iterator.Advance();
		}
	}
	if (spawn2_dump != 0)
		zone->DumpAllSpawn2(spawn2_dump, &spawn2index);
}

void EntityList::Depop(bool StartSpawnTimer) {
	LinkedListIterator<NPC*> iterator(npc_list);
	
	iterator.Reset();
	for(; iterator.MoreElements(); iterator.Advance())
	{
		NPC *it = iterator.GetData();
		Mob *own = it->GetOwner();
		//do not depop player's pets...
		if(it && own && own->IsClient())
			continue;
		it->Depop(StartSpawnTimer);
	}
}

void EntityList::SendTraders(Client* client){
	LinkedListIterator<Client*> iterator(client_list);
	iterator.Reset();
	Client* trader;
	while(iterator.MoreElements()) {
		trader=iterator.GetData();
		if(trader->IsTrader())
			client->SendTraderPacket(trader);

		if(trader->IsBuyer())
			client->SendBuyerPacket(trader);

		iterator.Advance();
	}
}

void EntityList::RemoveFromHateLists(Mob* mob, bool settoone) {
	LinkedListIterator<NPC*> iterator(npc_list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->CheckAggro(mob)) {
			if (!settoone)
			{
				iterator.GetData()->RemoveFromHateList(mob);
			}
			else
			{
				iterator.GetData()->SetHate(mob,1);
			}
		}
		iterator.Advance();
	}
}


// Currently, a new packet is sent per entity.
// @todo: Come back and use FLAG_COMBINED to pack
// all updates into one packet.
void EntityList::SendPositionUpdates(Client* client, int32 cLastUpdate, float range, Entity* alwayssend, bool iSendEvenIfNotChanged) {
	range = range * range;
	LinkedListIterator<Mob*> iterator(mob_list);
	
	EQApplicationPacket* outapp = 0;
	PlayerPositionUpdateServer_Struct* ppu = 0;
	Mob* mob = 0;
	
	iterator.Reset();
	while(iterator.MoreElements()) {
		if (outapp == 0) {
			outapp = new EQApplicationPacket(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
			ppu = (PlayerPositionUpdateServer_Struct*)outapp->pBuffer;
		}
		mob = iterator.GetData()->CastToMob();
		if (!mob->IsCorpse() 
			&& (iterator.GetData() != client)
			&& (mob->IsClient() || iSendEvenIfNotChanged || (mob->LastChange() >= cLastUpdate)) 
			&& (!iterator.GetData()->IsClient() || !iterator.GetData()->CastToClient()->GMHideMe(client))
			) {
				if (range == 0 || (iterator.GetData() == alwayssend) || (mob->DistNoRootNoZ(*client) <= range)) {
					mob->MakeSpawnUpdate(ppu);
			}
		}
		if(mob && mob->IsClient() && mob->GetID()>0)
/*#ifdef PACKET_UPDATE_MANAGER
			client->GetUpdateManager()->QueuePacket(outapp, false, mob, mob->DistNoRoot(*client));
#else*/
			client->QueuePacket(outapp, false, Client::CLIENT_CONNECTED);
//#endif
		safe_delete(outapp);
		outapp = 0;	
		iterator.Advance();
	}
	
	safe_delete(outapp);
}

char* EntityList::MakeNameUnique(char* name) {
	bool used[300];
	memset(used, 0, sizeof(used));
	name[61] = 0; name[62] = 0; name[63] = 0;

	LinkedListIterator<Mob*> iterator(mob_list);

	iterator.Reset();
	int len = strlen(name);
	while(iterator.MoreElements()) {
		if (iterator.GetData()->IsMob()) {
			if (strncasecmp(iterator.GetData()->CastToMob()->GetName(), name, len) == 0) {
				if (Seperator::IsNumber(&iterator.GetData()->CastToMob()->GetName()[len])) {
					used[atoi(&iterator.GetData()->CastToMob()->GetName()[len])] = true;
				}
			}
		}
		iterator.Advance();
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
	LinkedListIterator<NPC*> iterator(npc_list);
	int32 x = 0;
	int32 z = 0;
	char sName[36];
	
	iterator.Reset();
	client->Message(0, "NPCs in the zone:");
	if(searchtype == 0) {
		while(iterator.MoreElements()) {
			client->Message(0, "  %5d: %s", iterator.GetData()->GetID(), iterator.GetData()->GetName());
			x++;
			z++;
			iterator.Advance();
		}
	}
	else if(searchtype == 1) {
		client->Message(0, "Searching by name method. (%s)",arg1);
		char* tmp = new char[strlen(arg1) + 1];
		strcpy(tmp, arg1);
		strupr(tmp);
		while(iterator.MoreElements()) {
			z++;
			strcpy(sName, iterator.GetData()->GetName());
			strupr(sName);
			if (strstr(sName, tmp)) {
				client->Message(0, "  %5d: %s", iterator.GetData()->GetID(), iterator.GetData()->GetName());
				x++;
			}
			iterator.Advance();
		}
		safe_delete_array(tmp);
	}
	else if(searchtype == 2) {
		client->Message(0, "Searching by number method. (%s %s)",arg1,arg2);
		while(iterator.MoreElements()) {
			z++;
			if ((iterator.GetData()->GetID() >= atoi(arg1)) && (iterator.GetData()->GetID() <= atoi(arg2)) && (atoi(arg1) <= atoi(arg2))) {
				client->Message(0, "  %5d: %s", iterator.GetData()->GetID(), iterator.GetData()->GetName());
				x++;
			}
			iterator.Advance();
		}
	}
	client->Message(0, "%d npcs listed. There is a total of %d npcs in this zone.", x, z);
}

void EntityList::ListNPCCorpses(Client* client) {
	LinkedListIterator<Corpse*> iterator(corpse_list);
	int32 x = 0;
	
	iterator.Reset();
	client->Message(0, "NPC Corpses in the zone:");
	while(iterator.MoreElements()) {
		if (iterator.GetData()->IsNPCCorpse()) {
			client->Message(0, "  %5d: %s", iterator.GetData()->GetID(), iterator.GetData()->GetName());
			x++;
		}
		iterator.Advance();
	}
	client->Message(0, "%d npc corpses listed.", x);
}

void EntityList::ListPlayerCorpses(Client* client) {
	LinkedListIterator<Corpse*> iterator(corpse_list);
	int32 x = 0;
	
	iterator.Reset();
	client->Message(0, "Player Corpses in the zone:");
	while(iterator.MoreElements()) {
		if (iterator.GetData()->IsPlayerCorpse()) {
			client->Message(0, "  %5d: %s", iterator.GetData()->GetID(), iterator.GetData()->GetName());
			x++;
		}
		iterator.Advance();
	}
	client->Message(0, "%d player corpses listed.", x);
}

// returns the number of corpses deleted. A negative number indicates an error code.
sint32 EntityList::DeleteNPCCorpses() {
	LinkedListIterator<Corpse*> iterator(corpse_list);
	sint32 x = 0;
	
	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->IsNPCCorpse()) {
			iterator.GetData()->Depop();
			x++;
		}
		iterator.Advance();
	}
	return x;
}

// returns the number of corpses deleted. A negative number indicates an error code.
sint32 EntityList::DeletePlayerCorpses() {
	LinkedListIterator<Corpse*> iterator(corpse_list);
	sint32 x = 0;
	
	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->IsPlayerCorpse()) {
			iterator.GetData()->CastToCorpse()->Delete();
			x++;
		}
		iterator.Advance();
	}
	return x;
}
void EntityList::SendPetitionToAdmins(){
	LinkedListIterator<Client*> iterator(client_list);
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_PetitionUpdate,sizeof(PetitionUpdate_Struct));
	PetitionUpdate_Struct* pcus = (PetitionUpdate_Struct*) outapp->pBuffer;
	pcus->petnumber = 0;		// Petition Number
	pcus->color = 0;
	pcus->status = 0xFFFFFFFF;
	pcus->senttime = 0;
	strcpy(pcus->accountid, "");
	strcpy(pcus->gmsenttoo, "");
	pcus->quetotal=0;
	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->CastToClient()->Admin() >= 80)
			iterator.GetData()->CastToClient()->QueuePacket(outapp);
		iterator.Advance();
	}
	safe_delete(outapp);
}
void EntityList::SendPetitionToAdmins(Petition* pet) {
	LinkedListIterator<Client*> iterator(client_list);
	
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
	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->CastToClient()->Admin() >= 80) {
			if (pet->CheckedOut())
				strcpy(pcus->gmsenttoo, "");
			else
				strcpy(pcus->gmsenttoo, iterator.GetData()->CastToClient()->GetName());
			iterator.GetData()->CastToClient()->QueuePacket(outapp);
		}
		iterator.Advance();
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
	LinkedListIterator<Client*> iterator(client_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->CastToClient()->Admin() >= 100) {
			int x = 0;
			for (x=0;x<64;x++) {
				pet->petnumber = x;
				iterator.GetData()->CastToClient()->QueuePacket(outapp);
			}
		}
		iterator.Advance();
	}
	safe_delete(outapp);
	return;
}

void EntityList::WriteEntityIDs() {
	LinkedListIterator<Mob*> iterator(mob_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		cout << "ID: " << iterator.GetData()->GetID() << "  Name: " << iterator.GetData()->GetName() << endl;
		iterator.Advance();
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

void EntityList::DoubleAggro(Mob* who)
{
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->CheckAggro(who))
		  iterator.GetData()->SetHate(who,iterator.GetData()->CastToNPC()->GetHateAmount(who),iterator.GetData()->CastToNPC()->GetHateAmount(who)*2);
		iterator.Advance();
	}
}

void EntityList::HalveAggro(Mob* who)
{
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->CastToNPC()->CheckAggro(who))
		  iterator.GetData()->CastToNPC()->SetHate(who,iterator.GetData()->CastToNPC()->GetHateAmount(who)/2);
		iterator.Advance();
	}
}


void EntityList::Evade(Mob *who)
{
	uint32 flatval = who->GetLevel() * 13;
	int amt = 0;
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->CastToNPC()->CheckAggro(who)){
			amt = iterator.GetData()->CastToNPC()->GetHateAmount(who);
			amt -= flatval;
			if(amt > 0)
				iterator.GetData()->CastToNPC()->SetHate(who, amt);
			else
				iterator.GetData()->CastToNPC()->SetHate(who, 0);
		}
		iterator.Advance();
	}
}

//removes "targ" from all hate lists, including feigned, in the zone
void EntityList::ClearAggro(Mob* targ) {
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->CheckAggro(targ))
			iterator.GetData()->RemoveFromHateList(targ);
		iterator.GetData()->RemoveFromFeignMemory(targ->CastToClient()); //just in case we feigned
		iterator.Advance();
	}
}

void EntityList::ClearFeignAggro(Mob* targ)
{
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->CheckAggro(targ))
		{
			iterator.GetData()->RemoveFromHateList(targ);
			// EverHood 6/24/06
			// For client targets if the mob that hated us is 35+ 
			// there is a 3 outta 5 chance he adds us to feign memory
			if(targ->IsClient()){
				if (iterator.GetData()->GetLevel() >= 35){
					if(MakeRandomInt(1,100)<=60){
						iterator.GetData()->AddFeignMemory(targ->CastToClient());
					}
				}
			}
		}
		iterator.Advance();
	}
}
// EverHood 6/17/06
void EntityList::ClearZoneFeignAggro(Client* targ)
{
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		iterator.GetData()->RemoveFromFeignMemory(targ);
		iterator.Advance();
	}
}

void EntityList::AggroZone(Mob* who, int hate) {
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		iterator.GetData()->AddToHateList(who, hate);
		iterator.Advance();
	}
}

// Signal Quest command function
void EntityList::SignalMobsByNPCID(int32 snpc, int signal_id)
{
	LinkedListIterator<NPC*> iterator(npc_list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		NPC *it = iterator.GetData();
		if (it->GetNPCTypeID() == snpc)
		{
			it->SignalNPC(signal_id);
		}
		iterator.Advance();
	}
}


bool EntityList::MakeTrackPacket(Client* client){
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
	
	spe = mob_list.Count() + 50;

	uchar* buffer1 = new uchar[sizeof(Track_Struct)];
	Track_Struct* track_ent = (Track_Struct*) buffer1;
	
	uchar* buffer2 = new uchar[sizeof(Track_Struct)*spe];
	Tracking_Struct* track_array = (Tracking_Struct*) buffer2;
	memset(track_array, 0, sizeof(Track_Struct)*spe);
	
	int32 array_counter = 0;
	
	LinkedListIterator<Mob*> iterator(mob_list);
	iterator.Reset();

	while(iterator.MoreElements())
	{
		if (iterator.GetData() && ((MobDistance = iterator.GetData()->DistNoZ(*client))<=distance))
		{
			if(iterator.GetData()->IsTrackable()) {
				memset(track_ent, 0, sizeof(Track_Struct));
				Mob* cur_entity = iterator.GetData();
				track_ent->entityid = cur_entity->GetID();
				track_ent->distance = MobDistance;
				memcpy(&track_array->Entrys[array_counter], track_ent, sizeof(Track_Struct));
				array_counter++;
			}
		}

		iterator.Advance();
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

	LinkedListIterator<Client*> iterator(client_list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData() != sender && (iterator.GetData()->Dist(*sender) <= dist2 || iterator.GetData()->GetGroup() == sender->CastToClient()->GetGroup())) {
			iterator.GetData()->Message(type, buffer);
		}
		iterator.Advance();
	}
}


bool EntityList::Fighting(Mob* targ) {
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->CheckAggro(targ))
		{
			return true;
		}
		iterator.Advance();
	}
	return false;
}

void EntityList::AddHealAggro(Mob* target, Mob* caster, int16 thedam)
{
	LinkedListIterator<NPC*> iterator(npc_list);

	iterator.Reset();
	NPC *cur = NULL;
	int16 count = 0;
	while(iterator.MoreElements())
	{
		cur = iterator.GetData();
		iterator.Advance();

		if(!cur->CheckAggro(target))
		{
			iterator.Advance();
			continue;
		}
		if (!cur->IsMezzed() && !cur->IsStunned())
		{
			++count;
		}
	}

	if(thedam > 1)
	{
		if(count > 0)
			thedam = (thedam / count);

		if(thedam < 1)
			thedam = 1;
	}

	cur = NULL;
	iterator.Reset();
	while(iterator.MoreElements())
	{
		cur = iterator.GetData();
		if(!cur->CheckAggro(target)){
			iterator.Advance();
			continue;
		}

		if (!cur->IsMezzed() && !cur->IsStunned())
		{
			if(cur->IsPet()){
				if(caster){
					if(cur->CheckAggro(caster))
						cur->AddToHateList(caster, thedam);
				}
			}
			else{
				if(caster){
					if(!cur->CheckAggro(caster))
						cur->AddToHateList(caster, 1);
					else
						cur->AddToHateList(caster, thedam);
				}
			}
		}
		iterator.Advance();
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

void EntityList::SendAlarm(Trap* trap, Mob* currenttarget, int8 kos) 
{
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	
	float val2 = trap->effectvalue * trap->effectvalue;
	
	while(iterator.MoreElements())
	{
		NPC *cur = iterator.GetData();
		float curdist = 0;
		float tmp = cur->GetX() - trap->x;
		curdist += tmp*tmp;
		tmp = cur->GetY() - trap->y;
		curdist += tmp*tmp;
		tmp = cur->GetZ() - trap->z;
		curdist += tmp*tmp;
		if (!cur->GetOwner() && 
			/*!cur->CastToMob()->dead && */
			!cur->IsEngaged() && 
			curdist <= val2 )
		{
			if(kos)
			{
				int8 factioncon = currenttarget->GetReverseFactionCon(cur);
				if(factioncon == FACTION_THREATENLY || factioncon == FACTION_SCOWLS)
				{
					cur->AddToHateList(currenttarget,1);
				}
			}
			else
			{
				cur->AddToHateList(currenttarget,1);
			}
		}
		iterator.Advance();
	}
}

void EntityList::AddProximity(NPC *proximity_for) {
	RemoveProximity(proximity_for->GetID());
	
	proximity_list.Insert(proximity_for);
	
	proximity_for->proximity = new NPCProximity;
}

bool EntityList::RemoveProximity(int16 delete_npc_id) {
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

	if(!taskmanager) return;

	LinkedListIterator<Client*> iterator(client_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Client* client = iterator.GetData();
		if(client->IsTaskStateLoaded()) {
			client->SaveTaskState();
		}

		iterator.Advance();
	}
}

void EntityList::ReloadAllClientsTaskState(int TaskID) {

	if(!taskmanager) return;

	LinkedListIterator<Client*> iterator(client_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Client* client = iterator.GetData();
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
		iterator.Advance();
	}
}

bool EntityList::IsMobInZone(Mob *who) {
	LinkedListIterator<Mob*> iterator(mob_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(who == iterator.GetData())
			return(true);
		iterator.Advance();
	}
	return(false);
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
	
	map<int16, SpawnLimitRecord>::iterator cur,end;
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
	
	map<int16, SpawnLimitRecord>::iterator cur,end;
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
	
	map<int16, SpawnLimitRecord>::iterator cur,end;
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
	float range2 = range * range;
	
	LinkedListIterator<Mob*> iterator(mob_list);
	iterator.Reset();
	while(iterator.MoreElements()) {
		Mob* mob = iterator.GetData();
		if(mob->IsClient()) {
			if(!clients)
				continue;
		} else {
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

void EntityList::UpdateHoTT(Mob* target) {
	LinkedListIterator<Client*> iterator(client_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Client* c = iterator.GetData();
		if (c->GetTarget() == target) {
			if (target->GetTarget()) c->SetHoTT(target->GetTarget()->GetID());
			else c->SetHoTT(0);
		}
		iterator.Advance();
	}
}

void EntityList::DestroyTempPets(Mob *owner)
{
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		NPC* n = iterator.GetData();
		if(n->GetSwarmInfo())
		{
			if(n->GetSwarmInfo()->owner == owner)
			{
				n->Depop();
			}
		}
		iterator.Advance();
	}
}

bool Entity::CheckCoordLosNoZLeaps(float cur_x, float cur_y, float cur_z, float trg_x, float trg_y, float trg_z, float perwalk)
{
	if(zone->map == NULL) {
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

	if (!zone->map->LineIntersectsZoneNoZLeaps(myloc,oloc,perwalk,&hit,&onhit))
		return true;
	return false;
}

void EntityList::QuestJournalledSayClose(Mob *sender, Client *QuestInitiator, float dist, const char* mobname, const char* message)
{
       Client *c;
       LinkedListIterator<Client*> iterator(client_list);
       float dist2 = dist * dist;

       // Send the message to the quest initiator such that the client will enter it into the NPC Quest Journal
       if(QuestInitiator) {

               char *buf = new char[strlen(mobname) + strlen(message) + 10];
               sprintf(buf, "%s says, '%s'", mobname, message);
               QuestInitiator->QuestJournalledMessage(mobname, buf);
               safe_delete_array(buf);
       }
       // Use the old method for all other nearby clients
       for(iterator.Reset(); iterator.MoreElements(); iterator.Advance())
       {
               c = iterator.GetData();
               if(c && (c != QuestInitiator) && c->DistNoRoot(*sender) <= dist2)
                       c->Message_StringID(10, GENERIC_SAY, mobname, message);
       }
}

Corpse* EntityList::GetClosestCorpse(Mob* sender)
{
	if(!sender) 
		return NULL;

	uint32 dist = 4294967295;
	Corpse* nc = NULL;

	LinkedListIterator<Corpse*> iterator(corpse_list);
	iterator.Reset();

	while(iterator.MoreElements())
	{
		uint32 nd = ((iterator.GetData()->GetY() - sender->GetY()) * (iterator.GetData()->GetY() - sender->GetY())) + 
			((iterator.GetData()->GetX() - sender->GetX()) * (iterator.GetData()->GetX() - sender->GetX()));
		if(nd < dist){
			dist = nd;
			nc = iterator.GetData();

		}
		iterator.Advance();
	}
	return nc;
}

void EntityList::ForceGroupUpdate(int32 gid) {
	LinkedListIterator<Client*> iterator(client_list); 
	
	iterator.Reset(); 
	while(iterator.MoreElements()) {
		if(iterator.GetData()){
			Group *g = NULL;
			g = iterator.GetData()->GetGroup();
			if(g){
				if(g->GetID() == gid)
				{
					database.RefreshGroupFromDB(iterator.GetData());
				}
			}
		}
		iterator.Advance(); 
	} 
}

void EntityList::SendGroupLeave(int32 gid, const char *name) {
	LinkedListIterator<Client*> iterator(client_list); 
	iterator.Reset(); 
	while(iterator.MoreElements()) {
		Client *c = iterator.GetData();
		if(c){
			Group *g = NULL;
			g = c->GetGroup();
			if(g){
				if(g->GetID() == gid)
				{
					EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate,sizeof(GroupJoin_Struct));
					GroupJoin_Struct* gj = (GroupJoin_Struct*) outapp->pBuffer;	
					strcpy(gj->membername, name);
					gj->action = groupActLeave;
					strcpy(gj->yourname, name);
					Mob *Leader = g->GetLeader();
					if(Leader)
						Leader->CastToClient()->GetGroupAAs(&gj->leader_aas);
					c->QueuePacket(outapp);
					safe_delete(outapp);
					g->DelMemberOOZ(name);
					if(g->IsLeader(c) && c->IsLFP())
						c->UpdateLFP();
				}
			}
		}
		iterator.Advance(); 
	} 
}

void EntityList::SendGroupJoin(int32 gid, const char *name) {
	LinkedListIterator<Client*> iterator(client_list); 
	iterator.Reset(); 
	while(iterator.MoreElements()) {
		if(iterator.GetData()){
			Group *g = NULL;
			g = iterator.GetData()->GetGroup();
			if(g){
				if(g->GetID() == gid)
				{
					EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate,sizeof(GroupJoin_Struct));
					GroupJoin_Struct* gj = (GroupJoin_Struct*) outapp->pBuffer;	
					strcpy(gj->membername, name);
					gj->action = groupActJoin;
					strcpy(gj->yourname, iterator.GetData()->GetName());
					Mob *Leader = g->GetLeader();
					if(Leader)
						Leader->CastToClient()->GetGroupAAs(&gj->leader_aas);

					iterator.GetData()->QueuePacket(outapp);
					safe_delete(outapp);
				}
			}
		}
		iterator.Advance(); 
	} 
}

void EntityList::GroupMessage(int32 gid, const char *from, const char *message)
{
	LinkedListIterator<Client*> iterator(client_list); 
	iterator.Reset(); 
	while(iterator.MoreElements()) {
		if(iterator.GetData()){
			Group *g = NULL;
			g = iterator.GetData()->GetGroup();
			if(g){
				if(g->GetID() == gid)
				{
					iterator.GetData()->ChannelMessageSend(from, iterator.GetData()->GetName(),2,0,message);
				}
			}
		}
		iterator.Advance(); 
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
			object->Save();
			safe_delete(i);
		}
	}
}

Mob* EntityList::GetTargetForMez(Mob* caster)
{
	if(!caster)
		return NULL;

	LinkedListIterator<Mob*> iterator(mob_list); 
	iterator.Reset();
	//TODO: make this smarter and not mez targets being damaged by dots
	while(iterator.MoreElements()) {
		Mob* d = iterator.GetData();
		if(d){
			if(d == caster){ //caster can't pick himself
				iterator.Advance();
				continue;
			}

			if(caster->GetTarget() == d){ //caster can't pick his target
				iterator.Advance();			
				continue;
			}

			if(!caster->CheckAggro(d)){ //caster can't pick targets that aren't aggroed on himself
				iterator.Advance();
				continue;
			}

			if(caster->DistNoRoot(*d) > 22250){ //only pick targets within 150 range
				iterator.Advance();
				continue;
			}

			if(!caster->CheckLosFN(d)){ //this is wasteful but can't really think of another way to do it 
				iterator.Advance();		//that wont have us trying to los the same target every time
				continue;			   //it's only in combat so it's impact should be minimal.. but stil.
			}
			return d;
		}
		iterator.Advance();
	}
	return NULL;
}

void EntityList::SendZoneAppearance(Client *c)
{
	if(!c)
		return;

	LinkedListIterator<Mob*> iterator(mob_list); 
	iterator.Reset();
	while(iterator.MoreElements()) {
		Mob *cur = iterator.GetData();

		if(cur)
		{
			if(cur == c)
			{
				iterator.Advance();
				continue;
			}
			if(cur->GetAppearance() != ANIM_STAND)
			{
				cur->SendAppearancePacket(AT_Anim, cur->GetAppearanceValue(cur->GetAppearance()), false, true, c);
			}
		}
		iterator.Advance();
	}
}

void EntityList::ZoneWho(Client *c, Who_All_Struct* Who) {

	// This is only called for SoF clients, as regular /who is now handled server-side for that client.
	//
	int32 PacketLength = 0;

	int32 Entries = 0;

	LinkedListIterator<Client*> iterator(client_list); 
	
	iterator.Reset(); 

	while(iterator.MoreElements())  {

		Client *ClientEntry = iterator.GetData();

		iterator.Advance(); 

		if(ClientEntry) {

			if(ClientEntry->GMHideMe(c)) continue;

			if((Who->wrace != 0xFFFFFFFF) && (ClientEntry->GetRace() != Who->wrace)) continue;

			if((Who->wclass != 0xFFFFFFFF) && (ClientEntry->GetClass() != Who->wclass)) continue;

			if((Who->lvllow != 0xFFFFFFFF) && (ClientEntry->GetLevel() < Who->lvllow)) continue;

			if((Who->lvlhigh != 0xFFFFFFFF) && (ClientEntry->GetLevel() > Who->lvlhigh)) continue;

			if(Who->guildid != 0xFFFFFFFF) {

				if((Who->guildid == 0xFFFFFFFC) && !ClientEntry->IsTrader()) continue;

				if((Who->guildid == 0xFFFFFFFB) && !ClientEntry->IsBuyer()) continue;

				if(Who->guildid != ClientEntry->GuildID()) continue;
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

	iterator.Reset(); 

	while(iterator.MoreElements()) {

		Client *ClientEntry = iterator.GetData();

		iterator.Advance();

		if(ClientEntry) {

			if(ClientEntry->GMHideMe(c)) continue;

			if((Who->wrace != 0xFFFFFFFF) && (ClientEntry->GetRace() != Who->wrace)) continue;

			if((Who->wclass != 0xFFFFFFFF) && (ClientEntry->GetClass() != Who->wclass)) continue;

			if((Who->lvllow != 0xFFFFFFFF) && (ClientEntry->GetLevel() < Who->lvllow)) continue;

			if((Who->lvlhigh != 0xFFFFFFFF) && (ClientEntry->GetLevel() > Who->lvlhigh)) continue;

			if(Who->guildid != 0xFFFFFFFF) {

				if((Who->guildid == 0xFFFFFFFC) && !ClientEntry->IsTrader()) continue;

				if((Who->guildid == 0xFFFFFFFB) && !ClientEntry->IsBuyer()) continue;

				if(Who->guildid != ClientEntry->GuildID()) continue;
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

void EntityList::UnMarkNPC(int16 ID) 
{
	// Designed to be called from the Mob destructor, this method calls Group::UnMarkNPC for
	// each group to remove the dead mobs entity ID from the groups list of NPCs marked via the
	// Group Leadership AA Mark NPC ability.
	//
	LinkedListIterator<Client*> iterator(client_list); 

	iterator.Reset(); 

	while(iterator.MoreElements())
	{
		if(iterator.GetData())
		{
			Group *g = NULL;

			g = iterator.GetData()->GetGroup();

			if(g)
				g->UnMarkNPC(ID);
		}
		iterator.Advance(); 
	} 
}

