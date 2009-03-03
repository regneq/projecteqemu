/*  EQEMu:  Everquest Server Emulator
Copyright (C) 2001-2004  EQEMu Development Team (http://eqemu.org)

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
#include "spdat.h"
#include "masterentity.h"
#include "../common/packet_dump.h"
#include "../common/moremath.h"
#include "../common/Item.h"
#include "zonedb.h"
#include "worldserver.h"
#include "../common/skills.h"
#include "../common/bodytypes.h"
#include "../common/classes.h"
#include "pets.h"
#include <math.h>
#include <assert.h>
#ifndef WIN32
#include <stdlib.h>
#include "../common/unix.h"
#endif

#include "StringIDs.h"

///////////////////////////////////////////////////////////////////////////////
// pet related functions

const char *GetRandPetName()
{
	static const char *petnames[] = { "Gabeker","Gann","Garanab","Garn","Garer","Gartik",
            "Gebann","Gebekn","Gekn","Geraner","Gobeker","Gonobtik","Jabantik",
            "Jasarab","Jasober","Jeker","Jenaner","Jenarer","Jobantik",
            "Jobekn","Jonartik","Kabann","Kabartik","Karn","Kasarer","Kasekn",
            "Kebekn","Keber","Kebtik","Kenantik","Kenn","Kentik","Kibekab",
            "Kobarer","Kobobtik","Konaner","Konarer","Konekn","Konn","Labann",
            "Lararer","Lasobtik","Lebantik","Lebarab","Libantik","Libtik",
            "Lobn","Lobtik","Lonaner","Lonobtik","Varekab","Vaseker","Vebobab",
            "Venarn","Venekn","Vener","Vibobn","Vobtik","Vonarer","Vonartik",
            "Xabtik","Xarantik","Xarar","Xarer","Xeber","Xebn","Xenartik",
            "Xeratik","Xesekn","Xonartik","Zabantik","Zaber","Zabn","Zabeker","Zanab",
            "Zaner","Zenann","Zonarer","Zonarn" };
	int r = MakeRandomInt(0, (sizeof(petnames)/sizeof(const char *))-1);
	printf("Pet being created: %s\n",petnames[r]); // DO NOT COMMENT THIS OUT!
	return petnames[r];
}

//not used anymore
/*int CalcPetHp(int levelb, int classb, int STA)
{
	int multiplier = 0;
	int base_hp = 0;
	switch(classb) {
		case WARRIOR:{
			if (levelb < 20)
				multiplier = 22;
			else if (levelb < 30)
				multiplier = 23;
			else if (levelb < 40)
				multiplier = 25;
			else if (levelb < 53)
				multiplier = 27;
			else if (levelb < 57)
				multiplier = 28;
			else
				multiplier = 30;
			break;
		}
		case DRUID:
		case CLERIC:
		case SHAMAN:{
			multiplier = 15;
			break;
		}
		case PALADIN:
		case SHADOWKNIGHT:{
			if (levelb < 35)
				multiplier = 21;
			else if (levelb < 45)
				multiplier = 22;
			else if (levelb < 51)
				multiplier = 23;
			else if (levelb < 56)
				multiplier = 24;
			else if (levelb < 60)
				multiplier = 25;
			else
				multiplier = 26;
			break;
		}
		case MONK:
		case BARD:
		case ROGUE:
		case BEASTLORD:{
			if (levelb < 51)
				multiplier = 18;
			else if (levelb < 58)
				multiplier = 19;
			else
				multiplier = 20;
			break;
		}
		case RANGER:{
			if (levelb < 58)
				multiplier = 20;
			else
				multiplier = 21;
			break;
		}
		case MAGICIAN:
		case WIZARD:
		case NECROMANCER:
		case ENCHANTER:{
			multiplier = 12;
			break;
		}
		default:{
			if (levelb < 35)
				multiplier = 21;
			else if (levelb < 45)
				multiplier = 22;
			else if (levelb < 51)
				multiplier = 23;
			else if (levelb < 56)
				multiplier = 24;
			else if (levelb < 60)
				multiplier = 25;
			else
				multiplier = 26;
			break;
		}
	}

	if (multiplier == 0)
	{
		LogFile->write(EQEMuLog::Error, "Multiplier == 0 in CalcPetHp,using Generic....");;
		multiplier=12;
	}

	base_hp = 5 + (multiplier*levelb) + ((multiplier*levelb*STA) + 1)/300;
	return base_hp;
}
*/


void Mob::MakePet(int16 spell_id, const char* pettype, const char *petname) {
	//see if we are a special type of pet (for command filtering)
	PetType type = petOther;
	if(strncmp(pettype, "Familiar", 8) == 0) {
		type = petFamiliar;
	} else if(strncmp(pettype, "Animation", 9) == 0) {
		type = petAnimation;
	}
	
	if(HasPet())
		return;

	
	//lookup our pets table record for this type
	PetRecord record;
	if(!database.GetPetEntry(pettype, &record)) {
		Message(13, "Unable to find data for pet %s", pettype);
		LogFile->write(EQEMuLog::Error, "Unable to find data for pet %s, check pets table.", pettype);
		return;
	}
	
	//find the NPC data for the specified NPC type
	const NPCType *base = database.GetNPCType(record.npc_type);
	if(base == NULL) {
		Message(13, "Unable to load NPC data for pet %s", pettype);
		LogFile->write(EQEMuLog::Error, "Unable to load NPC data for pet %s (NPC ID %d), check pets and npc_types tables.", pettype, record.npc_type);
		return;
	}
	
	//we copy the npc_type data because we need to edit it a bit
	NPCType *npc_type = new NPCType;
	memcpy(npc_type, base, sizeof(NPCType));
	
	if (this->IsClient() && CastToClient()->GetFocusEffect(focusPetPower, spell_id) > 0)
	{
		npc_type->max_hp *= 1.20;
		npc_type->cur_hp = npc_type->max_hp;
		npc_type->AC *= 1.20;
		npc_type->level += 1;
		npc_type->min_dmg = (npc_type->min_dmg * 110 / 100);
		npc_type->max_dmg = (npc_type->max_dmg * 110 / 100);
		npc_type->size *= 1.15;
	}

	switch (GetAA(aaElementalDurability))
	{
	case 1:
		npc_type->max_hp *= 1.02;
		npc_type->cur_hp = npc_type->max_hp;
		break;
	case 2:
		npc_type->max_hp *= 1.05;
		npc_type->cur_hp = npc_type->max_hp;
		break;
	case 3:
		npc_type->max_hp *= 1.10;
		npc_type->cur_hp = npc_type->max_hp;
		break;
	}

	//TODO: think about regen (engaged vs. not engaged)
	
	if(petname != NULL) {
		strncpy(npc_type->name, petname, 64);
	} else if (strncmp("Familiar", pettype, 8) == 0) {
		strcpy(npc_type->name, this->GetName());
		npc_type->name[19] = '\0';
		strcat(npc_type->name, "`s_familiar");
	} else if (this->IsClient()) {
		//clients get a random pet name
		strcpy(npc_type->name, GetRandPetName());
	} else {
		strcpy(npc_type->name, this->GetCleanName());
		npc_type->name[25] = '\0';
		strcat(npc_type->name, "`s_pet");
	}
	
	
	//handle beastlord pet appearance
	if(strncmp(pettype, "BLpet", 5) == 0) {
		switch ( GetBaseRace() ) {
		case VAHSHIR: npc_type->race = TIGER; npc_type->size *= 0.8f; break;
		case TROLL: npc_type->race = ALLIGATOR; npc_type->size *= 2.5f; break;
		case OGRE: npc_type->race = BEAR; npc_type->texture=3; npc_type->gender=2; break;
		case BARBARIAN: npc_type->race = WOLF; npc_type->texture=2; break;
		case IKSAR: npc_type->race = WOLF; npc_type->texture=1; npc_type->gender=1; npc_type->size *= 2.0f; break;
		default: npc_type->race = WOLF; npc_type->texture=0; break;
		}
	}
	
	//this takes ownership of the npc_type data
	Pet *npc = new Pet(npc_type, this, type, spell_id);

#ifdef EQBOTS

	if(IsBot()) {
		npc->SetTaunting(false);
		npc->BotOwner = this->BotOwner;
		npc->SetOwnerID(this->GetID());
		if(IsBotRaiding()) {
			npc->SetBotRaidID(GetBotRaidID());
		}
	}

#endif //EQBOTS

	entity_list.AddNPC(npc);
	SetPetID(npc->GetID());
}
/* Angelox: This is why the pets ghost - pets were being spawned too far away from its npc owner and some
into walls or objects (+10), this sometimes creates the "ghost" effect. I changed to +2 (as close as I 
could get while it still looked good). I also noticed this can happen if an NPC is spawned on the same spot of another or in a related bad spot.*/
Pet::Pet(NPCType *type_data, Mob *owner, PetType type, int16 spell_id)
: NPC(type_data, 0, owner->GetX()+2, owner->GetY()+2, owner->GetZ(), owner->GetHeading())
{
	GiveNPCTypeData(type_data);
	typeofpet = type;
	SetOwnerID(owner->GetID());
	SetPetSpellID(spell_id);
	taunting = true;
}

bool ZoneDatabase::GetPetEntry(const char *pet_type, PetRecord *into) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;
	
	if (RunQuery(query, MakeAnyLenString(&query, 
		"SELECT npcID,temp FROM pets WHERE type='%s'", pet_type), errbuf, &result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			
			into->npc_type = atoi(row[0]);
			into->temporary = atoi(row[1]);
			
			mysql_free_result(result);
			return(true);
		}
		mysql_free_result(result);
	}
	else {
		LogFile->write(EQEMuLog::Error, "Error in GetPetEntry query '%s': %s", query,  errbuf);
		safe_delete_array(query);
	}
	return(false);
}

Mob* Mob::GetPet() {
	if(GetPetID() == 0)
		return(NULL);
	
	Mob* tmp = entity_list.GetMob(GetPetID());
	if(tmp == NULL) {
		SetPetID(0);
		return(NULL);
	}
	
	if(tmp->GetOwnerID() != GetID()) {
		SetPetID(0);
		return(NULL);
	}
	
	return(tmp);
}

void Mob::SetPet(Mob* newpet) {
	Mob* oldpet = GetPet();
	if (oldpet) {
		oldpet->SetOwnerID(0);
	}
	if (newpet == NULL) {
		SetPetID(0);
	} else {
		SetPetID(newpet->GetID());
		Mob* oldowner = entity_list.GetMob(newpet->GetOwnerID());
		if (oldowner)
			oldowner->SetPetID(0);
		newpet->SetOwnerID(this->GetID());
	}
}

void Mob::SetPetID(int16 NewPetID) {
	if (NewPetID == GetID() && NewPetID != 0)
		return;
	petid = NewPetID;
}

void NPC::GetPetState(SpellBuff_Struct *pet_buffs, int32 *items, char *name) {
	//save the pet name
	strncpy(name, GetCleanName(), 64);
	name[63] = '\0';
	
	//save their items
	int i;
	memset(items, 0, sizeof(int32)*MAX_MATERIALS);
	i = 0;
	
	ItemList::iterator cur,end;
	cur = itemlist.begin();
	end = itemlist.end();
	for(; cur != end; cur++) {
		ServerLootItem_Struct* item = *cur;
		items[i] = item->item_id;
		i++;
		if (i >= MAX_MATERIALS)
			break;
		//dont need to save anything else... since these items only
		//exist for the pet, nobody else can get at them AFAIK
	}
	
	//save their buffs.
	for (i=0; i < BUFF_COUNT; i++) {
		if (buffs[i].spellid != SPELL_UNKNOWN) {
			pet_buffs[i].spellid = buffs[i].spellid;
// solar: fix this if buffs struct is fixed
			pet_buffs[i].slotid = i+1/*2*/;
			pet_buffs[i].duration = buffs[i].ticsremaining;
			pet_buffs[i].level = buffs[i].casterlevel;
			pet_buffs[i].effect = 10;
			pet_buffs[i].persistant_buff = buffs[i].persistant_buff;
			pet_buffs[i].reserved = 0;
		}
		else {
			pet_buffs[i].spellid = SPELL_UNKNOWN;
			pet_buffs[i].duration = 0;
			pet_buffs[i].level = 0;
			pet_buffs[i].effect = 0;
			pet_buffs[i].persistant_buff = 0;
			pet_buffs[i].reserved = 0;
		}
	}
}

void NPC::SetPetState(SpellBuff_Struct *pet_buffs, int32 *items) {
	//restore their buffs...
	
	int i;
	for (i = 0; i < BUFF_COUNT; i++) {
		for(int z = 0; z < BUFF_COUNT; z++) {
		// check for duplicates
			if(buffs[z].spellid != SPELL_UNKNOWN && buffs[z].spellid == pet_buffs[i].spellid) {
				buffs[z].spellid = SPELL_UNKNOWN;
				pet_buffs[i].spellid = 0xFFFFFFFF;
			}
		}
		
		if (pet_buffs[i].spellid <= (int32)SPDAT_RECORDS && pet_buffs[i].spellid != 0 && pet_buffs[i].duration > 0) {
			if(pet_buffs[i].level == 0 || pet_buffs[i].level > 100)
				pet_buffs[i].level = 1;
			buffs[i].spellid			= pet_buffs[i].spellid;
			buffs[i].ticsremaining		= pet_buffs[i].duration;
			buffs[i].casterlevel		= pet_buffs[i].level;
			buffs[i].casterid			= 0;
			buffs[i].durationformula	= spells[buffs[i].spellid].buffdurationformula;
			buffs[i].poisoncounters		= CalculatePoisonCounters(pet_buffs[i].spellid);
			buffs[i].diseasecounters	= CalculateDiseaseCounters(pet_buffs[i].spellid);
			buffs[i].cursecounters		= CalculateCurseCounters(pet_buffs[i].spellid);
			buffs[i].numhits			= spells[pet_buffs[i].spellid].numhits;
		}
		else {
			buffs[i].spellid = SPELL_UNKNOWN;
			pet_buffs[i].spellid = 0xFFFFFFFF;
			pet_buffs[i].slotid = 0;
			pet_buffs[i].level = 0;
			pet_buffs[i].duration = 0;
			pet_buffs[i].effect = 0;
		}
	}
	for (int j1=0; j1 < BUFF_COUNT; j1++) {
		if (buffs[j1].spellid <= (int32)SPDAT_RECORDS) {
			for (int x1=0; x1 < EFFECT_COUNT; x1++) {
				switch (spells[buffs[j1].spellid].effectid[x1]) {
					case SE_Charm:
					case SE_Rune:
					case SE_Illusion:
						buffs[j1].spellid = SPELL_UNKNOWN;
						pet_buffs[j1].spellid = SPELLBOOK_UNKNOWN;
						pet_buffs[j1].slotid = 0;
						pet_buffs[j1].level = 0;
						pet_buffs[j1].duration = 0;
						pet_buffs[j1].effect = 0;
						x1 = EFFECT_COUNT;
						break;
					// We can't send appearance packets yet, put down at CompleteConnect
				}
			}
		}
	}
	
	//restore their equipment...
	for(i = 0; i < MAX_MATERIALS; i++) {
		if(items[i] == 0)
			continue;
		
		const Item_Struct* item2 = database.GetItem(items[i]);
		if (item2 && item2->NoDrop != 0) {
			//dont bother saving item charges for now, NPCs never use them
			//and nobody should be able to get them off the corpse..?
			AddLootDrop(item2, &itemlist, 0, true, true);
		}
	}
}




