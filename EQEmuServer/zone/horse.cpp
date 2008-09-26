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
#include "masterentity.h"
#include "../common/Item.h"
#include "../common/linked_list.h"
#include <math.h>
#include <assert.h>

map<int16, const NPCType *> Horse::horse_types;
LinkedList<NPCType *> horses_auto_delete;

/*
	After writting all this crap as an NPC, I think it would be possible
	to inherit from mob. I left it this way since it better facilitates
	loading horses from the database in the future.
*/

Horse::Horse(Client *_owner, int16 spell_id, float x, float y, float z, float heading) 
 : NPC(GetHorseType(spell_id), NULL, x, y, z, heading)
{
	//give the horse its proper name.
	strncpy(name, _owner->GetCleanName(), 55);
	name[55] = '\0';
	strcat(name,"`s_Mount00");
	
	owner = _owner;
}
	
void Horse::FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho) {
	NPC::FillSpawnStruct(ns, ForWho);
	
//	ns->spawn.texture = NPCTypedata->mount_color;
	ns->spawn.petOwnerId = 0;
	
	//dunno why we do these, they should allready be set right.
	ns->spawn.runspeed = NPCTypedata->runspeed;
}
	
bool Horse::IsHorseSpell(int16 spell_id) {
	//written in terms of a function which does a ton more work
	//than we need to to figure out if this is a horse spell.
	//the logic is that people calling this function will post
	//likely immediately summon the horse, so we need the extra anyways.
	return(GetHorseType(spell_id) != NULL);
}

const NPCType *Horse::GetHorseType(int16 spell_id) {
	if(horse_types.count(spell_id) == 1)
		return(horse_types[spell_id]);
	//cache invalid spell IDs as NULL entries
	const NPCType *ret;
	horse_types[spell_id] = ret = BuildHorseType(spell_id);
	return(ret);
}

const NPCType *Horse::BuildHorseType(int16 spell_id) {


	// Spell: 2862 Tan Rope
	// Spell: 2863 Tan Leather
	// Spell: 2864 Tan Silken
	// Spell: 2865 Brown Chain
	// Spell: 2866 Tan Ornate Chain
	// Spell: 2867 White Rope
	// Spell: 2868 White Leather
	// Spell: 2869 White Silken
	// Spell: 2870 White Chain
	// Spell: 2871 White Ornate Chain
	// Spell: 2872 Black Rope
	// Spell: 2919 Tan Rope
	// Spell: 2918 Guide
	// Spell: 2917 Black Chain,		
	
	char mount_color=0;
	
	NPCType* npc_type = new NPCType;
	memset(npc_type, 0, sizeof(NPCType));
	strcpy(npc_type->name,"Unclaimed_Mount");	//this should never get used
      strcpy(npc_type->npc_attacks,"ABH");
	npc_type->cur_hp = 1; 
	npc_type->max_hp = 1; 
	npc_type->race = 216;
	npc_type->gender = (spell_id >= 3813 && spell_id <= 3832) ? 1 : 0; // Drogmor's are female horses. Yuck.
	npc_type->class_ = 1; 
	npc_type->deity= 1;
	npc_type->level = 1;
	npc_type->npc_id = 0;
	npc_type->loottable_id = 0;

	switch(spell_id) {
		case 2862:
			mount_color=0;  // Brown horse
			npc_type->runspeed=MOUNT_SLOW1_RUN;
			break;
		case 2863:
			mount_color=0;  // Brown horse
			npc_type->runspeed=MOUNT_SLOW2_RUN;
			break;
		case 2864:
			mount_color=0;  // Brown horse
			npc_type->runspeed=MOUNT_RUN1_RUN;
			break;
		case 2865:
			mount_color=0;  // Brown horse
			npc_type->runspeed=MOUNT_RUN2_RUN;
			break;
		case 2866:
			mount_color=0;  // Brown horse
			npc_type->runspeed=MOUNT_FAST_RUN;
			break;
		case 2867:
			mount_color=1;  // White horse
			npc_type->runspeed=MOUNT_SLOW1_RUN;
			break;
		case 2868:
			mount_color=1;  // White horse
			npc_type->runspeed=MOUNT_SLOW2_RUN;
			break;
		case 2869:
			mount_color=1;  // White horse
			npc_type->runspeed=MOUNT_RUN1_RUN;
			break;
		case 2870:
			mount_color=1;  // White horse
			npc_type->runspeed=MOUNT_RUN2_RUN;
			break;
		case 2871:
			mount_color=1;  // White horse
			npc_type->runspeed=MOUNT_FAST_RUN;
			break;
		case 2872:
			mount_color=2;  // Black horse
			npc_type->runspeed=MOUNT_SLOW1_RUN;
			break;
		case 2873:
			mount_color=2;  // Black horse
			npc_type->runspeed=MOUNT_SLOW2_RUN;
			break;
		case 2916:
			mount_color=2;  // Black horse
			npc_type->runspeed=MOUNT_RUN1_RUN;
			break;
		case 2917:
			mount_color=2;  // Black horse
			npc_type->runspeed=MOUNT_RUN2_RUN;
			break;
		case 2918:
			mount_color=2;  // Black horse
			npc_type->runspeed=MOUNT_FAST_RUN;
			break;
		case 2919:
			mount_color=3;  // Tan horse
			npc_type->runspeed=MOUNT_SLOW1_RUN;
			break;
		case 2920:
			mount_color=3;  // Tan horse
			npc_type->runspeed=MOUNT_SLOW2_RUN;
			break;
		case 2921:
			mount_color=3;  // Tan horse
			npc_type->runspeed=MOUNT_RUN1_RUN;
			break;
		case 2922:
			mount_color=3;  // Tan horse
			npc_type->runspeed=MOUNT_RUN2_RUN;
			break;
		case 2923:
			mount_color=3;  // Tan horse
			npc_type->runspeed=MOUNT_FAST_RUN;
			break;
		case 3813:
			mount_color=0;  // White drogmor
			npc_type->runspeed=MOUNT_SLOW1_RUN;
			break;
		case 3814:
			mount_color=0;  // White drogmor
			npc_type->runspeed=MOUNT_SLOW2_RUN;
			break;
		case 3815:
			mount_color=0;  // White drogmor
			npc_type->runspeed=MOUNT_RUN1_RUN;
			break;
		case 3816:
			mount_color=0;  // White drogmor
			npc_type->runspeed=MOUNT_RUN2_RUN;
			break;
		case 3817:
			mount_color=0;  // White drogmor
			npc_type->runspeed=MOUNT_FAST_RUN;
			break;
		case 3818:
			mount_color=1;  // Black drogmor
			npc_type->runspeed=MOUNT_SLOW1_RUN;
			break;
		case 3819:
			mount_color=1;  // Black drogmor
			npc_type->runspeed=MOUNT_SLOW2_RUN;
			break;
		case 3820:
			mount_color=1;  // Black drogmor
			npc_type->runspeed=MOUNT_RUN1_RUN;
			break;
		case 3821:
			mount_color=1;  // Black drogmor
			npc_type->runspeed=MOUNT_RUN2_RUN;
			break;
		case 3822:
			mount_color=1;  // Black drogmor
			npc_type->runspeed=MOUNT_FAST_RUN;
			break;
		case 3823:
			mount_color=2;  // Green drogmor
			npc_type->runspeed=MOUNT_SLOW1_RUN;
			break;
		case 3824:
			mount_color=2;  // Green drogmor
			npc_type->runspeed=MOUNT_SLOW2_RUN;
			break;
		case 3825:
			mount_color=2;  // Green drogmor
			npc_type->runspeed=MOUNT_RUN1_RUN;
			break;
		case 3826:
			mount_color=2;  // Green drogmor
			npc_type->runspeed=MOUNT_RUN2_RUN;
			break;
		case 3827:
			mount_color=2;  // Green drogmor
			npc_type->runspeed=MOUNT_FAST_RUN;
			break;
		case 3828:
			mount_color=3;  // Red drogmor
			npc_type->runspeed=MOUNT_SLOW1_RUN;
			break;
		case 3829:
			mount_color=3;  // Red drogmor
			npc_type->runspeed=MOUNT_SLOW2_RUN;
			break;
		case 3830:
			mount_color=3;  // Red drogmor
			npc_type->runspeed=MOUNT_RUN1_RUN;
			break;
		case 3831:
			mount_color=3;  // Red drogmor
			npc_type->runspeed=MOUNT_RUN2_RUN;
			break;
		case 3832:
			mount_color=3;  // Red drogmor
			npc_type->runspeed=MOUNT_FAST_RUN;
			break;
		case 2874:
			npc_type->runspeed=MOUNT_FAST_RUN;
			mount_color=1;
			break;
		case 2875:
			npc_type->runspeed=MOUNT_FAST_RUN;
			mount_color=2;
			break;			
		default:
/*			Message(13,"I dont know what mount spell this is! (%i)", spell_id);
			mount_color= 0;  // Brown horse
			npc_type->walkspeed=MOUNT_SLOW1_WALK;
			npc_type->runspeed=MOUNT_SLOW1_RUN;*/
			LogFile->write(EQEMuLog::Error, "Unknown mount spell id %d", spell_id);
			safe_delete(npc_type);
			return(NULL);
			break;
	}
	npc_type->texture = mount_color;

	npc_type->light = 0;
	npc_type->STR = 75;
	npc_type->STA = 75;
	npc_type->DEX = 75;
	npc_type->AGI = 75;
	npc_type->INT = 75;
	npc_type->WIS = 75;
	npc_type->CHA = 75;
	
	horses_auto_delete.Insert(npc_type);
	
	return(npc_type);
}



void Client::SummonHorse(int16 spell_id) {
	if (GetHorseId() != 0) {
		Message(13,"You already have a Horse.  Get off, Fatbutt!");
		return;
	}
	if(!Horse::IsHorseSpell(spell_id)) {
		LogFile->write(EQEMuLog::Error, "%s tried to summon an unknown horse, spell id %d", GetName(), spell_id);
		return;
	}
	
	// No Horse, lets get them one.
	
	Horse* horse = new Horse(this, spell_id, GetX(), GetY(), GetZ(), GetHeading());
	
	//we want to manage the spawn packet ourself.
	//another reason is we dont want quests executing on it.
	entity_list.AddNPC(horse, false);
	
	// Okay, lets say they have a horse now.
	
	
	EQApplicationPacket outapp;
	horse->CreateHorseSpawnPacket(&outapp, GetName(), GetID());
/*	// Doodman: Kludged in here instead of adding a field to PCType. FIXME!
	NewSpawn_Struct* ns=(NewSpawn_Struct*)outapp->pBuffer;
	ns->spawn.texture=mount_color;
	ns->spawn.pet_owner_id=0;
	ns->spawn.walkspeed=npc_type->walkspeed;
	ns->spawn.runspeed=npc_type->runspeed;
*/
	entity_list.QueueClients(horse, &outapp);
	
	
	int16 tmpID = horse->GetID();
	SetHorseId(tmpID);
	
}

void Client::SetHorseId(int16 horseid_in) {
	//if its the same, do nothing
	if(horseId == horseid_in)
		return;
	
	//otherwise it changed.
	//if we have a horse, get rid of it no matter what.
	if(horseId) {
		Mob *horse = entity_list.GetMob(horseId);
		if(horse != NULL)
			horse->Depop();
	}
	
	//now we take whatever they gave us.
	horseId = horseid_in;
}

void Mob::CreateHorseSpawnPacket(EQApplicationPacket* app, const char* ownername, uint16 ownerid, Mob* ForWho) {
	app->SetOpcode(OP_NewSpawn);
	app->pBuffer = new uchar[sizeof(NewSpawn_Struct)];
	app->size = sizeof(NewSpawn_Struct);
	memset(app->pBuffer, 0, sizeof(NewSpawn_Struct));
	NewSpawn_Struct* ns = (NewSpawn_Struct*)app->pBuffer;
	FillSpawnStruct(ns, ForWho);
	
#if (EQDEBUG >= 11)
	printf("Horse Spawn Packet - Owner: %s\n", ownername);
	DumpPacket(app);
#endif
}


