
#include "../debug.h"
#include "SoF.h"
#include "../opcodemgr.h"
#include "../logsys.h"
#include "../EQStreamIdent.h"
#include "../crc32.h"

#include "../eq_packet_structs.h"
#include "../MiscFunctions.h"
#include "../Item.h"
#include "SoF_structs.h"
#include "../rulesys.h"

#include <iostream>
#include <sstream>

namespace SoF {

static const char *name = "SoF";
static OpcodeManager *opcodes = NULL;
static Strategy struct_strategy;

char* SerializeItem(const ItemInst *inst, sint16 slot_id, uint32 *length, uint8 depth);
	
void Register(EQStreamIdentifier &into) {
	//create our opcode manager if we havent already
	if(opcodes == NULL) {
		//TODO: get this file name from the config file
		string opfile = "patch_";
		opfile += name;
		opfile += ".conf";
		//load up the opcode manager.
		//TODO: figure out how to support shared memory with multiple patches...
		opcodes = new RegularOpcodeManager();
		if(!opcodes->LoadOpcodes(opfile.c_str())) {
			_log(NET__OPCODES, "Error loading opcodes file %s. Not registering patch %s.", opfile.c_str(), name);
			return;
		}
	}
	
	//ok, now we have what we need to register.
	
	EQStream::Signature signature;
	string pname;
	
	//register our world signature.
	pname = string(name) + "_world";
	signature.ignore_eq_opcode = 0;
	signature.first_length = sizeof(structs::LoginInfo_Struct);
	signature.first_eq_opcode = opcodes->EmuToEQ(OP_SendLoginInfo);
	into.RegisterPatch(signature, pname.c_str(), &opcodes, &struct_strategy);
	
	//register our zone signature.
	pname = string(name) + "_zone";
	signature.ignore_eq_opcode = opcodes->EmuToEQ(OP_AckPacket);
	signature.first_length = sizeof(structs::ClientZoneEntry_Struct);
	signature.first_eq_opcode = opcodes->EmuToEQ(OP_ZoneEntry);
	into.RegisterPatch(signature, pname.c_str(), &opcodes, &struct_strategy);
	
	
	
	_log(NET__IDENTIFY, "Registered patch %s", name);
}

void Reload() {
	
	//we have a big problem to solve here when we switch back to shared memory
	//opcode managers because we need to change the manager pointer, which means
	//we need to go to every stream and replace it's manager.
	
	if(opcodes != NULL) {
		//TODO: get this file name from the config file
		string opfile = "patch_";
		opfile += name;
		opfile += ".conf";
		if(!opcodes->ReloadOpcodes(opfile.c_str())) {
			_log(NET__OPCODES, "Error reloading opcodes file %s for patch %s.", opfile.c_str(), name);
			return;
		}
		_log(NET__OPCODES, "Reloaded opcodes for patch %s", name);
	}
}



Strategy::Strategy()
: StructStrategy()
{
	//all opcodes default to passthrough.
	#include "SSRegister.h"
	#include "SoF_ops.h"
}

std::string Strategy::Describe() const {
       std::string r;
	r += "Patch ";
	r += name;
	return(r);
}
 


#include "SSDefine.h"

ENCODE(OP_OpenNewTasksWindow) {

	AvailableTaskHeader_Struct*	__emu_AvailableTaskHeader;
	AvailableTaskData1_Struct* 	__emu_AvailableTaskData1;
	AvailableTaskData2_Struct* 	__emu_AvailableTaskData2;
	AvailableTaskTrailer_Struct* 	__emu_AvailableTaskTrailer;

	structs::AvailableTaskHeader_Struct*	__eq_AvailableTaskHeader;
	structs::AvailableTaskData1_Struct* 	__eq_AvailableTaskData1;
	structs::AvailableTaskData2_Struct* 	__eq_AvailableTaskData2;
	structs::AvailableTaskTrailer_Struct* 	__eq_AvailableTaskTrailer;

	EQApplicationPacket *in = *p;
	*p = NULL;
	
	unsigned char *__emu_buffer = in->pBuffer;

	__emu_AvailableTaskHeader = (AvailableTaskHeader_Struct*)__emu_buffer;

	// For each task, SoF has an extra uint32 and what appears to be space for a null terminated string.
	//
	in->size = in->size + (__emu_AvailableTaskHeader->TaskCount * 5);

	in->pBuffer = new unsigned char[in->size];

	unsigned char *__eq_buffer = in->pBuffer;

	__eq_AvailableTaskHeader = (structs::AvailableTaskHeader_Struct*)__eq_buffer;

	char *__eq_ptr, *__emu_Ptr;

	// Copy Header
	//
	//

	__eq_AvailableTaskHeader->TaskCount = __emu_AvailableTaskHeader->TaskCount;
	__eq_AvailableTaskHeader->unknown1 = __emu_AvailableTaskHeader->unknown1;
	__eq_AvailableTaskHeader->TaskGiver = __emu_AvailableTaskHeader->TaskGiver;

	__emu_Ptr = (char *) __emu_AvailableTaskHeader + sizeof(AvailableTaskHeader_Struct);
	__eq_ptr = (char *) __eq_AvailableTaskHeader + sizeof(structs::AvailableTaskHeader_Struct);

	for(uint32 i=0; i<__emu_AvailableTaskHeader->TaskCount; i++) {

		__emu_AvailableTaskData1 = (AvailableTaskData1_Struct*)__emu_Ptr;
		__eq_AvailableTaskData1 = (structs::AvailableTaskData1_Struct*)__eq_ptr;

		__eq_AvailableTaskData1->TaskID = __emu_AvailableTaskData1->TaskID;
		// This next unknown seems to affect the colour of the task title. 0x3f80000 is what I have seen
		// in Live packets. Changing it to 0x3f000000 makes the title red.
		__eq_AvailableTaskData1->unknown1 = 0x3f800000; 
		__eq_AvailableTaskData1->TimeLimit = __emu_AvailableTaskData1->TimeLimit;
		__eq_AvailableTaskData1->unknown2 = __emu_AvailableTaskData1->unknown2;

		__emu_Ptr += sizeof(AvailableTaskData1_Struct);
		__eq_ptr += sizeof(structs::AvailableTaskData1_Struct);

		strcpy(__eq_ptr, __emu_Ptr); // Title

		__emu_Ptr += strlen(__emu_Ptr) + 1;
		__eq_ptr += strlen(__eq_ptr) + 1;

		strcpy(__eq_ptr, __emu_Ptr); // Description

		__emu_Ptr += strlen(__emu_Ptr) + 1;
		__eq_ptr += strlen(__eq_ptr) + 1;

		__eq_ptr[0] = 0;
		__eq_ptr += strlen(__eq_ptr) + 1;

		__emu_AvailableTaskData2 = (AvailableTaskData2_Struct*)__emu_Ptr;
		__eq_AvailableTaskData2 = (structs::AvailableTaskData2_Struct*)__eq_ptr;

		__eq_AvailableTaskData2->unknown1 = __emu_AvailableTaskData2->unknown1;
		__eq_AvailableTaskData2->unknown2 = __emu_AvailableTaskData2->unknown2;
		__eq_AvailableTaskData2->unknown3 = __emu_AvailableTaskData2->unknown3;
		__eq_AvailableTaskData2->unknown4 = __emu_AvailableTaskData2->unknown4;

		__emu_Ptr += sizeof(AvailableTaskData2_Struct);
		__eq_ptr += sizeof(structs::AvailableTaskData2_Struct);

		strcpy(__eq_ptr, __emu_Ptr); // Unknown string

		__emu_Ptr += strlen(__emu_Ptr) + 1;
		__eq_ptr += strlen(__eq_ptr) + 1;

		strcpy(__eq_ptr, __emu_Ptr); // Unknown string

		__emu_Ptr += strlen(__emu_Ptr) + 1;
		__eq_ptr += strlen(__eq_ptr) + 1;

		__emu_AvailableTaskTrailer = (AvailableTaskTrailer_Struct*)__emu_Ptr;
		__eq_AvailableTaskTrailer = (structs::AvailableTaskTrailer_Struct*)__eq_ptr;

		__eq_AvailableTaskTrailer->ItemCount = __emu_AvailableTaskTrailer->ItemCount;
		__eq_AvailableTaskTrailer->unknown1 = __emu_AvailableTaskTrailer->unknown1;
		__eq_AvailableTaskTrailer->unknown2 = __emu_AvailableTaskTrailer->unknown2;
		__eq_AvailableTaskTrailer->StartZone = __emu_AvailableTaskTrailer->StartZone;

		__emu_Ptr += sizeof(AvailableTaskTrailer_Struct);
		__eq_ptr += sizeof(structs::AvailableTaskTrailer_Struct);

		strcpy(__eq_ptr, __emu_Ptr); // Unknown string

		__emu_Ptr += strlen(__emu_Ptr) + 1;
		__eq_ptr += strlen(__eq_ptr) + 1;
	}

	delete[] __emu_buffer;
	
	dest->FastQueuePacket(&in, ack_req);
}


ENCODE(OP_SendCharInfo) {
	ENCODE_LENGTH_EXACT(CharacterSelect_Struct);
	SETUP_VAR_ENCODE(CharacterSelect_Struct);
	
	
	//EQApplicationPacket *packet = *p;
	//const CharacterSelect_Struct *emu = (CharacterSelect_Struct *) packet->pBuffer;

	int char_count;
	int namelen = 0;
	for(char_count = 0; char_count < 10; char_count++) {
		if(emu->name[char_count][0] == '\0')
			break;
		if(strcmp(emu->name[char_count], "<none>") == 0)
			break;
		namelen += strlen(emu->name[char_count]);
    }

	int total_length = sizeof(structs::CharacterSelect_Struct)
		+ char_count * sizeof(structs::CharacterSelectEntry_Struct)
		+ namelen;

	ALLOC_VAR_ENCODE(structs::CharacterSelect_Struct, total_length);
	
	//unsigned char *eq_buffer = new unsigned char[total_length];
	//structs::CharacterSelect_Struct *eq_head = (structs::CharacterSelect_Struct *) eq_buffer;
	
	eq->char_count = char_count;
	eq->total_chars = 10;

	unsigned char *bufptr = (unsigned char *) eq->entries;
	int r;
	for(r = 0; r < char_count; r++) {
		{	//pre-name section...
			structs::CharacterSelectEntry_Struct *eq2 = (structs::CharacterSelectEntry_Struct *) bufptr;
			eq2->level = emu->level[r];
			eq2->haircolor = emu->haircolor[r];
			eq2->gender = emu->gender[r];
			strcpy(eq2->name, emu->name[r]);
		}
		//adjust for name.
		bufptr += strlen(emu->name[r]);
		{	//post-name section...
			structs::CharacterSelectEntry_Struct *eq2 = (structs::CharacterSelectEntry_Struct *) bufptr;
			eq2->beard = emu->beard[r];
			eq2->hair = emu->hair[r];
			eq2->face = emu->face[r];
			int k;
			for(k = 0; k < MAX_MATERIALS; k++) {
				eq2->equip[k].equip0 = emu->equip[r][k];
				eq2->equip[k].equip1 = 0;
				eq2->equip[k].itemid = 0;
				eq2->equip[k].color.color = emu->cs_colors[r][k].color;
			}
			eq2->secondary = emu->secondary[r];
			eq2->primary = emu->primary[r];
			eq2->tutorial = emu->tutorial[r]; // was u15
			eq2->u15 = 0xff;
			eq2->deity = emu->deity[r];
			eq2->zone = emu->zone[r];
			eq2->u19 = 0xFF;
			eq2->race = emu->race[r];
			eq2->gohome = emu->gohome[r];
			eq2->class_ = emu->class_[r];
			eq2->eyecolor1 = emu->eyecolor1[r];
			eq2->beardcolor = emu->beardcolor[r];
			eq2->eyecolor2 = emu->eyecolor2[r];
			eq2->u13 = 0; // Appears to be Drakkin Related
			eq2->u14 = 0; // Appears to be Drakkin Related
			eq2->u29 = 0; // Appears to be Drakkin Related
		}
		bufptr += sizeof(structs::CharacterSelectEntry_Struct);
	}
	
	FINISH_ENCODE();
	
}

ENCODE(OP_ZoneServerInfo) {
	SETUP_DIRECT_ENCODE(ZoneServerInfo_Struct, ZoneServerInfo_Struct);
	OUT_str(ip);
	OUT(port);
	FINISH_ENCODE();
	
	//this is SUCH bullshit to be doing from down here. but the 
	// new client requires us to close immediately following this
	// packet, so do it.
	//dest->Close();
}

//hack hack hack
ENCODE(OP_SendZonepoints) {
	ENCODE_LENGTH_ATLEAST(ZonePoints);
	
	SETUP_VAR_ENCODE(ZonePoints);
	ALLOC_VAR_ENCODE(structs::ZonePoints, __packet->size);
	
	memcpy(eq, emu, __packet->size);
	
	FINISH_ENCODE();
//	unknown0xxx[24];
	//this is utter crap... the client is waiting for this
	//certain 0 length opcode to come after the reqclientspawn
	//stuff... so this is a dirty way to put it in there. 
	// this needs to be done better

	//EQApplicationPacket hack_test(OP_PetitionUnCheckout, 0);
	//dest->QueuePacket(&hack_test);

}

ENCODE(OP_SendAATable) {
	ENCODE_LENGTH_ATLEAST(SendAA_Struct);
	
	SETUP_VAR_ENCODE(SendAA_Struct);
	ALLOC_VAR_ENCODE(structs::SendAA_Struct, sizeof(structs::SendAA_Struct) + emu->total_abilities*sizeof(structs::AA_Ability));
	
	OUT(id);
	eq->unknown004 = 1;
	eq->hotkey_sid = (emu->hotkey_sid==4294967295UL)?0:(emu->id - emu->current_level + 1);
	eq->hotkey_sid2 = (emu->hotkey_sid2==4294967295UL)?0:(emu->id - emu->current_level + 1);
	eq->title_sid = emu->id - emu->current_level + 1;
	eq->desc_sid = emu->id - emu->current_level + 1;
	OUT(class_type);
	OUT(cost);
	OUT(seq);
	OUT(current_level);
	OUT(prereq_skill);
	OUT(prereq_minpoints);
	OUT(type);
	if (emu->type < 4)
		eq->type = emu->type;
	if (emu->type > 4)
		eq->type = 1;
	OUT(spellid);
	OUT(spell_type);
	OUT(spell_refresh);
	OUT(classes);
	OUT(berserker);
	OUT(max_level);
	OUT(last_id);
	OUT(next_id);
	OUT(cost2);
	//memset(eq->unknown80, 0x00, sizeof(eq->unknown80));
	eq->aa_expansion = emu->type;
	eq->unknown0092 = -1;
	//eq->unknown0096 = 0;
	OUT(total_abilities);
	unsigned int r;
	for(r = 0; r < emu->total_abilities; r++) {
		OUT(abilities[r].skill_id);
		OUT(abilities[r].base1);
		OUT(abilities[r].base2);
		OUT(abilities[r].slot);
	}
	FINISH_ENCODE();
}

ENCODE(OP_LeadershipExpUpdate) {
	SETUP_DIRECT_ENCODE(LeadershipExpUpdate_Struct, structs::LeadershipExpUpdate_Struct);
	OUT(group_leadership_exp);
	OUT(group_leadership_points);
	OUT(raid_leadership_exp);
	OUT(raid_leadership_points);
	FINISH_ENCODE();
}

ENCODE(OP_PlayerProfile) {
	SETUP_DIRECT_ENCODE(PlayerProfile_Struct, structs::PlayerProfile_Struct);
	
	uint32 r;
	
	eq->available_slots=0xffffffff;
	memset(eq->unknown4184, 0xff, sizeof(eq->unknown4184));
	memset(eq->unknown04396, 0xff, sizeof(eq->unknown04396));
	
//	OUT(checksum);
	OUT(gender);
	OUT(race);
	OUT(class_);
//	OUT(unknown00016);
	OUT(level);
	eq->level1 = emu->level;
//	OUT(unknown00022[2]);
	for(r = 0; r < 5; r++) {
		OUT(binds[r].zoneId);
		OUT(binds[r].x);
		OUT(binds[r].y);
		OUT(binds[r].z);
		OUT(binds[r].heading);
	}
	OUT(deity);
	OUT(intoxication);
	OUT_array(spellSlotRefresh, structs::MAX_PP_MEMSPELL);
	OUT(points); // Relocation Test
//	OUT(unknown0166[4]);
	OUT(haircolor);
	OUT(beardcolor);
	OUT(eyecolor1);
	OUT(eyecolor2);
	OUT(hairstyle);
	OUT(beard);
//	OUT(unknown00178[10]);
	for(r = 0; r < 9; r++) {
		eq->equipment[r].equip0 = emu->item_material[r];
		eq->equipment[r].equip1 = 0;
		eq->equipment[r].itemId = 0;
		//eq->colors[r].color = emu->colors[r].color;
	}
	for(r = 0; r < 7; r++) {
		OUT(item_tint[r].color);
	}
//	OUT(unknown00224[48]);
	//NOTE: new client supports 359 AAs, our internal rep 
	//only supports 239..
	for(r = 0; r < structs::MAX_PP_AA_ARRAY; r++) {
		OUT(aa_array[r].AA);
		OUT(aa_array[r].value);
	}
//	OUT(unknown02220[4]);
	OUT(mana);
	OUT(cur_hp);
	OUT(STR);
	OUT(STA);
	OUT(CHA);
	OUT(AGI);
	OUT(INT);
	OUT(DEX);
	OUT(WIS);
	OUT(face);
//	OUT(unknown02264[47]);
	OUT_array(spell_book, structs::MAX_PP_SPELLBOOK);
//	OUT(unknown4184[128]);
	OUT_array(mem_spells, structs::MAX_PP_MEMSPELL);
//	OUT(unknown04396[32]);
	OUT(platinum);
	OUT(gold);
	OUT(silver);
	OUT(copper);
	OUT(platinum_cursor);
	OUT(gold_cursor);
	OUT(silver_cursor);
	OUT(copper_cursor);
	OUT_array(skills, structs::MAX_PP_SKILL);
//	OUT(unknown04760[236]);
	OUT(toxicity);
	OUT(thirst_level);
	OUT(hunger_level);
	for(r = 0; r < structs::BUFF_COUNT; r++) {
		OUT(buffs[r].slotid);
		OUT(buffs[r].level);
		OUT(buffs[r].bard_modifier);
		OUT(buffs[r].effect);
		OUT(buffs[r].spellid);
		OUT(buffs[r].duration);
		OUT(buffs[r].dmg_shield_remaining);
		OUT(buffs[r].persistant_buff);
		OUT(buffs[r].reserved);
		OUT(buffs[r].player_id);
	}
	//NOTE: new client supports 100 disciplines, our internal rep 
	//only supports 50..
	for(r = 0; r < 50; r++) {
		OUT(disciplines.values[r]);
	}
//	OUT(unknown05008[360]);
//	OUT_array(recastTimers, structs::MAX_RECAST_TYPES);
	OUT(endurance);
	OUT(aapoints_spent);
	OUT(aapoints);
//	OUT(unknown06160[4]);
	//NOTE: new client supports 20 bandoliers, our internal rep 
	//only supports 4..
	for(r = 0; r < 4; r++) {
		OUT_str(bandoliers[r].name);
		uint32 k;
		for(k = 0; k < structs::MAX_PLAYER_BANDOLIER_ITEMS; k++) {
			OUT(bandoliers[r].items[k].item_id);
			OUT(bandoliers[r].items[k].icon);
			OUT_str(bandoliers[r].items[k].item_name);
		}
	}
//	OUT(unknown07444[5120]);
	for(r = 0; r < structs::MAX_POTIONS_IN_BELT; r++) {
		OUT(potionbelt.items[r].item_id);
		OUT(potionbelt.items[r].icon);
		OUT_str(potionbelt.items[r].item_name);
	}
//	OUT(unknown12852[8]);
//	OUT(unknown12864[76]);
	OUT_str(name);
	OUT_str(last_name);
	OUT(guild_id);
	OUT(birthday);
	OUT(lastlogin);
	OUT(timePlayedMin);
	OUT(pvp);
	OUT(anon);
	OUT(gm);
	OUT(guildrank);
//	OUT(unknown13054[12]);
	OUT(exp);
//	OUT(unknown13072[8]);
	OUT(timeentitledonaccount);
	OUT_array(languages, structs::MAX_PP_LANGUAGE);
//	OUT(unknown13109[7]);
	OUT(y); //reversed x and y
	OUT(x);
	OUT(z);
	OUT(heading);
//	OUT(unknown13132[4]);
	OUT(platinum_bank);
	OUT(gold_bank);
	OUT(silver_bank);
	OUT(copper_bank);
	OUT(platinum_shared);
//	OUT(unknown13156[84]);
	//OUT(expansions);
	eq->expansions = 16383;
//	OUT(unknown13244[12]);
	OUT(autosplit);
//	OUT(unknown13260[16]);
	OUT(zone_id);
	OUT(zoneInstance);
	for(r = 0; r < structs::MAX_GROUP_MEMBERS; r++) {
		OUT_str(groupMembers[r]);
	}
	strcpy(eq->groupLeader, emu->groupMembers[0]);
//	OUT_str(groupLeader);
//	OUT(unknown13728[660]);
//	OUT(leadAAActive);
//	OUT(unknown14392[4]);
	OUT(ldon_points_guk);
	OUT(ldon_points_mir);
	OUT(ldon_points_mmc);
	OUT(ldon_points_ruj);
	OUT(ldon_points_tak);
	OUT(ldon_points_available);
//	OUT(unknown14420[132]);
	OUT(tribute_time_remaining);
	OUT(career_tribute_points);
//	OUT(unknown7208);
	OUT(tribute_points);
//	OUT(unknown7216);
	OUT(tribute_active);
	for(r = 0; r < structs::MAX_PLAYER_TRIBUTES; r++) {
		OUT(tributes[r].tribute);
		OUT(tributes[r].tier);
	}
//	OUT(unknown14616[8]);
	OUT(group_leadership_exp);
//	OUT(unknown14628);
	OUT(raid_leadership_exp);
	OUT(group_leadership_points);
	OUT(raid_leadership_points);
	OUT_array(leader_abilities.ranks, structs::MAX_LEADERSHIP_AA_ARRAY);
//	OUT(unknown14772[128]);
	OUT(air_remaining);
//	OUT(unknown14904[4608]);
	OUT(expAA);
//	OUT(unknown19516[40]);
	OUT(currentRadCrystals);
	OUT(careerRadCrystals);
	OUT(currentEbonCrystals);
	OUT(careerEbonCrystals);
	OUT(groupAutoconsent);
	OUT(raidAutoconsent);
	OUT(guildAutoconsent);
//	OUT(unknown19575[5]);
//	OUT(showhelm);
	eq->showhelm = 1;
//	OUT(unknown19584[4]);
//	OUT(unknown19588);


const uint8 bytes[] = {
0xa3,0x02,0x00,0x00,0x95,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x19,0x00,0x00,0x00,
0x19,0x00,0x00,0x00,0x19,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,
0x0F,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x1F,0x85,0xEB,0x3E,0x33,0x33,0x33,0x3F,
0x04,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x07,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

	memcpy(eq->unknown12864, bytes, sizeof(bytes));
		

	
	//set the checksum...
	CRC32::SetEQChecksum(__packet->pBuffer, sizeof(structs::PlayerProfile_Struct)-4);
	
	FINISH_ENCODE();
}

ENCODE(OP_NewZone) {
	SETUP_DIRECT_ENCODE(NewZone_Struct, structs::NewZone_Struct);
	OUT_str(char_name);
	OUT_str(zone_short_name);
	OUT_str(zone_long_name);
	OUT(ztype);
	int r;
	for(r = 0; r < 4; r++) {
		OUT(fog_red[r]);
		OUT(fog_green[r]);
		OUT(fog_blue[r]);
		OUT(fog_minclip[r]);
		OUT(fog_maxclip[r]);
	}
	OUT(gravity);
	OUT(time_type);
	for(r = 16; r < 48; r++) {
		eq->unknown521[r] = 0xFF;	//observed
	}
	OUT(sky);
	OUT(zone_exp_multiplier);
	OUT(safe_y);
	OUT(safe_x);
	OUT(safe_z);
	OUT(max_z);
	OUT(underworld);
	OUT(minclip);
	OUT(maxclip);
	OUT_str(zone_short_name2);
	OUT(zone_id);
	OUT(zone_instance);
	
    /*fill in some unknowns with observed values, hopefully it will help */
	eq->unknown796 = -1;
	eq->unknown840 = 600;
	eq->unknown876 = 50;
	eq->unknown880 = 10;
	eq->unknown884 = 1;
	eq->unknown885 = 0;
	eq->unknown886 = 1;
	eq->unknown887 = 0;
	eq->unknown888 = 0;
	eq->unknown889 = 0;
	eq->fall_damage = 0;	// 0 = Fall Damage on, 1 = Fall Damage off
	eq->unknown891 = 0;
	eq->unknown892 = 180;
	eq->unknown896 = 180;
	eq->unknown900 = 180;
	eq->unknown904 = 2;
	eq->unknown908 = 2;

	FINISH_ENCODE();
}

ENCODE(OP_NewSpawn) {  ENCODE_FORWARD(OP_ZoneSpawns); }
ENCODE(OP_ZoneEntry){  ENCODE_FORWARD(OP_ZoneSpawns); }
ENCODE(OP_ZoneSpawns) {
	//consume the packet
	EQApplicationPacket *in = *p;
	*p = NULL;
	
	//store away the emu struct
	unsigned char *__emu_buffer = in->pBuffer;
	Spawn_Struct *emu = (Spawn_Struct *) __emu_buffer;
	
	//determine and verify length
	int entrycount = in->size / sizeof(Spawn_Struct);
	if(entrycount == 0 || (in->size % sizeof(Spawn_Struct)) != 0) {
		_log(NET__STRUCTS, "Wrong size on outbound %s: Got %d, expected multiple of %d", opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(Spawn_Struct));
		delete in;
		return;
	}
	
	//make the EQ struct.
	in->size = sizeof(structs::Spawn_Struct)*entrycount;
	in->pBuffer = new unsigned char[in->size];
	structs::Spawn_Struct *eq = (structs::Spawn_Struct *) in->pBuffer;
	
	//zero out the packet. We could avoid this memset by setting all fields (including unknowns)
	//in the loop.
	memset(in->pBuffer, 0, in->size);
	
	//do the transform...
	int r;
	int k;
	for(r = 0; r < entrycount; r++, eq++, emu++) {

		eq->showname = 1; //New Field - Toggles Name Display on or off - 0 = off, 1 = on
		eq->linkdead = 0; //New Field - Toggles LD on or off after name - 0 = off, 1 = on
		eq->showhelm = emu->showhelm;

		eq->deity = emu->deity;
		eq->gender = emu->gender;
		for(k = 0; k < 9; k++) {
			eq->equipment[k].equip0 = emu->equipment[k];
			eq->equipment[k].equip1 = 0;
			eq->equipment[k].itemId = 0;
			eq->colors[k].color = emu->colors[k].color;
		}
		eq->guildID = emu->guildID;

		eq->StandState = 0x64;
		
		eq->class_ = emu->class_;
		eq->gm = emu->gm;
		eq->helm = emu->helm;
		eq->runspeed = emu->runspeed;
		eq->light = emu->light;
		eq->level = emu->level;
		eq->lfg = emu->lfg;
		eq->race = emu->race;
		strcpy(eq->suffix, emu->suffix);
		eq->findable = emu->findable;
		eq->bodytype = emu->bodytype;
		eq->equip_chest2 = emu->equip_chest2;
		eq->curHp = emu->curHp;
		strcpy(eq->lastName, emu->lastName);
		eq->eyecolor1 = emu->eyecolor1;
		strcpy(eq->title, emu->title);
		eq->beard = emu->beard;
		eq->targetable = 1; //New Field - Toggle Targetable on or off - 0 = off, 1 = on
		eq->NPC = emu->NPC;
		eq->targetable_with_hotkey = 1;//New Field - Toggle Targetable on or off - 0 = off, 1 = on
		eq->x = emu->x;
		eq->deltaX = emu->deltaX;
		eq->deltaY = emu->deltaY;
		eq->z = emu->z;
		eq->deltaHeading = emu->deltaHeading;
		eq->y = emu->y;
		eq->deltaZ = emu->deltaZ;
		eq->animation = emu->animation;
		eq->heading = emu->heading;
		eq->spawnId = emu->spawnId;
		strcpy(eq->name, emu->name);
		eq->petOwnerId = emu->petOwnerId;
		eq->pvp = 0;	// 0 = non-pvp colored name, 1 = red pvp name
		for(k = 0; k < 9; k++) {
			eq->colors[k].color = emu->colors[k].color;
		}
		eq->anon = emu->anon;
		eq->face = emu->face;
		eq->size = emu->size;
		eq->walkspeed = emu->walkspeed;

		//Hack Test for finding more fields in the Struct:
		//memset(eq->unknown0001, 0x01, sizeof(eq->unknown0001));	// 
		//memset(eq->unknown0006, 0x01, sizeof(eq->unknown0006));	// 
		//memset(eq->unknown0011, 0x02, sizeof(eq->unknown0011));	// 
		//memset(eq->unknown0048, 0x01, sizeof(eq->unknown0048));	// 
		//eq->unknown0820 = 1;										// Stand State - Stand/Sit/Crouch
		//eq->unknown0059 = 1;										// - west bug?
		//memset(eq->unknown0074, 0x01, sizeof(eq->unknown0074));	// 
		//memset(eq->unknown0077, 0x01, sizeof(eq->unknown0077));	// 
		//memset(eq->unknown00771, 0x00, sizeof(eq->unknown00771));	// 
		//memset(eq->unknown00772, 0x02, sizeof(eq->unknown00772));	// 
		//memset(eq->unknown0078, 0x00, sizeof(eq->unknown0078));	// 
		//memset(eq->unknown0079, 0x01, sizeof(eq->unknown0079));	// 
		//memset(eq->unknown0080, 0x01, sizeof(eq->unknown0080));	// 
		//memset(eq->unknown0106, 0x01, sizeof(eq->unknown0106));	// 
		//memset(eq->unknown0107, 0x01, sizeof(eq->unknown0107));	// - Flymode
		//memset(eq->unknown01071, 0x01, sizeof(eq->unknown01071));	//
		//memset(eq->unknown0108, 0x00, sizeof(eq->unknown0108));	// - LFG and Hair/Beard
		//memset(eq->unknown01081, 0x01, sizeof(eq->unknown01081));	// 
		//memset(eq->unknown01082, 0x00, sizeof(eq->unknown01082));	//
		//memset(eq->unknown0110, 0x01, sizeof(eq->unknown0110));	// 
		//memset(eq->unknown01101, 0x00, sizeof(eq->unknown01101));	// 
		//eq->unknown0111 = 2;										// 
		//memset(eq->unknown0154, 0x01, sizeof(eq->unknown0154));	// - freeze in place?
		//memset(eq->unknown0263, 0x01, sizeof(eq->unknown0263));	// - no player character visible?
		//memset(eq->unknown0281, 0x01, sizeof(eq->unknown0281));	// 
		//memset(eq->unknown0308, 0x02, sizeof(eq->unknown0308));	// 
		//memset(eq->unknown0309, 0x02, sizeof(eq->unknown0309));	// 
		//memset(eq->unknown442, 0x01, sizeof(eq->unknown442));		// - crash?
		//memset(eq->unknown0760, 0x03, sizeof(eq->unknown0760));	// 
		//eq->unknown0760 = 0;
		//eq->unknown0761 = 1;
		//eq->unknown0762 = 2;
		//eq->unknown0763 = 3;
		//eq->unknown0764 = 4;
		//memset(eq->unknown0496, 0x02, sizeof(eq->unknown0496));	// 

	}
	
	
	//kill off the emu structure and send the eq packet.
	delete[] __emu_buffer;
	
	_log(NET__ERROR, "Sending zone spawns");
	_hex(NET__ERROR, in->pBuffer, in->size);
	
	dest->FastQueuePacket(&in, ack_req);
}

ENCODE(OP_ItemLinkResponse) {  ENCODE_FORWARD(OP_ItemPacket); }
ENCODE(OP_ItemPacket) {
	//consume the packet
	EQApplicationPacket *in = *p;
	*p = NULL;
	
	unsigned char *__emu_buffer = in->pBuffer;
	ItemPacket_Struct *old_item_pkt=(ItemPacket_Struct *)__emu_buffer;
	InternalSerializedItem_Struct *int_struct=(InternalSerializedItem_Struct *)(old_item_pkt->SerializedItem);

	uint32 length;
	char *serialized=SerializeItem((ItemInst *)int_struct->inst,int_struct->slot_id,&length,0);

	if (!serialized) {
		_log(NET__STRUCTS, "Serialization failed on item slot %d.",int_struct->slot_id);
		delete in;
		return;
	}
	in->size = length+4;
	in->pBuffer = new unsigned char[in->size];
	ItemPacket_Struct *new_item_pkt=(ItemPacket_Struct *)in->pBuffer;
	new_item_pkt->PacketType=old_item_pkt->PacketType;
	memcpy(new_item_pkt->SerializedItem,serialized,length);

	delete[] __emu_buffer;
	safe_delete_array(serialized);
	dest->FastQueuePacket(&in, ack_req);
}

ENCODE(OP_CharInventory) {
	//consume the packet
	EQApplicationPacket *in = *p;
	*p = NULL;

	if(in->size == 0) {
		in->size = 4;
		in->pBuffer = new uchar[in->size];
		*((uint32 *) in->pBuffer) = 0;
		dest->FastQueuePacket(&in, ack_req);
		return;
	}
	
	//store away the emu struct
	unsigned char *__emu_buffer = in->pBuffer;

	int itemcount = in->size / sizeof(InternalSerializedItem_Struct);
	if(itemcount == 0 || (in->size % sizeof(InternalSerializedItem_Struct)) != 0) {
		_log(NET__STRUCTS, "Wrong size on outbound %s: Got %d, expected multiple of %d", opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(InternalSerializedItem_Struct));
		delete in;
		return;
	}
	InternalSerializedItem_Struct *eq = (InternalSerializedItem_Struct *) in->pBuffer;
	
	in->size = 4;
	in->pBuffer = new uchar[in->size];
	*((uint32 *) in->pBuffer) = 0;
	dest->FastQueuePacket(&in, ack_req);

	//EQApplicationPacket * outapp = new EQApplicationPacket((const EmuOpcode)0x78Cd);

	int r;
	char* serialized = NULL;
	uint32 length = 0;
	for(r = 0; r < itemcount; r++, eq++) 
	{
		length = 0;
		serialized = NULL;
        serialized = SerializeItem((const ItemInst*)eq->inst,eq->slot_id,&length,0);
		if(serialized)
		{
			EQApplicationPacket * outapp = new EQApplicationPacket(OP_ItemPacket, length+4);
			uint32 * type = (uint32*)outapp->pBuffer;
			*type = ItemPacketTrade;
			memcpy(outapp->pBuffer+4, serialized, length);

			_log(NET__ERROR, "Sending item to client");
			_hex(NET__ERROR, outapp->pBuffer, outapp->size);

			dest->FastQueuePacket(&outapp);
			delete[] serialized;
			serialized = NULL;
			if((const ItemInst*)eq->inst,eq->slot_id >= 22 && (const ItemInst*)eq->inst,eq->slot_id <= 30)
			{
				for(int x = 0; x < 10; ++x)
				{
					const ItemInst* subitem = ((const ItemInst*)eq->inst)->GetItem(x);
					if(subitem)
					{
						uint32 sub_length;
						serialized = NULL;
						serialized = SerializeItem(subitem, (((eq->slot_id+3)*10)+x+1), &sub_length, 0);
						if(serialized)
						{
							EQApplicationPacket * suboutapp = new EQApplicationPacket(OP_ItemPacket, sub_length+4);
							uint32 * subtype = (uint32*)suboutapp->pBuffer;
							*subtype = ItemPacketTrade;
							memcpy(suboutapp->pBuffer+4, serialized, sub_length);
							_log(NET__ERROR, "Sending sub item to client");
							_hex(NET__ERROR, suboutapp->pBuffer, suboutapp->size);
							dest->FastQueuePacket(&suboutapp);
							delete[] serialized;
							serialized = NULL;
						}
					}
				}
			}
			else if((const ItemInst*)eq->inst,eq->slot_id >= 2000 && (const ItemInst*)eq->inst,eq->slot_id <= 2023)
			{
				for(int x = 0; x < 10; ++x)
				{
					const ItemInst* subitem = ((const ItemInst*)eq->inst)->GetItem(x);
					if(subitem)
					{
						uint32 sub_length;
						serialized = NULL;
						serialized = SerializeItem(subitem, (((eq->slot_id-2000)*10)+2030+x+1), &sub_length, 0);
						if(serialized)
						{
							EQApplicationPacket * suboutapp = new EQApplicationPacket(OP_ItemPacket, sub_length+4);
							uint32 * subtype = (uint32*)suboutapp->pBuffer;
							*subtype = ItemPacketTrade;
							memcpy(suboutapp->pBuffer+4, serialized, sub_length);
							_log(NET__ERROR, "Sending sub item to client");
							_hex(NET__ERROR, suboutapp->pBuffer, suboutapp->size);
							dest->FastQueuePacket(&suboutapp);
							delete[] serialized;
							serialized = NULL;
						}
					}
				}
			}
			else if((const ItemInst*)eq->inst,eq->slot_id >= 2500 && (const ItemInst*)eq->inst,eq->slot_id <= 2501)
			{
				for(int x = 0; x < 10; ++x)
				{
					const ItemInst* subitem = ((const ItemInst*)eq->inst)->GetItem(x);
					if(subitem)
					{
						uint32 sub_length;
						serialized = NULL;
						serialized = SerializeItem(subitem, (((eq->slot_id-2500)*10)+2530+x+1), &sub_length, 0);
						if(serialized)
						{
							EQApplicationPacket * suboutapp = new EQApplicationPacket(OP_ItemPacket, sub_length+4);
							uint32 * subtype = (uint32*)suboutapp->pBuffer;
							*subtype = ItemPacketTrade;
							memcpy(suboutapp->pBuffer+4, serialized, sub_length);
							_log(NET__ERROR, "Sending sub item to client");
							_hex(NET__ERROR, suboutapp->pBuffer, suboutapp->size);
							dest->FastQueuePacket(&suboutapp);
							delete[] serialized;
							serialized = NULL;
						}
					}
				}
			}
		}
	}

	//Proper way below crashing
	//Workaround above
	//Goal: get the item struct good enough that we don't need the workaround.
	/*uchar *data = NULL;
	uchar *dataptr = NULL;
	uchar *tempdata = NULL;

	//do the transform...
	int r;

	data = new uchar[4];
	uint32 *item_opcode;
	item_opcode = (uint32*)data;
	*item_opcode = 0x69;//0x35;


	uint32 total_length = 4;
	uint32 length = 0;

	char* serialized = NULL;
	for(r = 0; r < itemcount; r++, eq++) 
	{
		length = 0;
		serialized = NULL;
        serialized = SerializeItem((const ItemInst*)eq->inst,eq->slot_id,&length,0);
		if(serialized)
		{
			tempdata = data;
			data = NULL;
			data = new uchar[total_length+length];
			memcpy(data, tempdata, total_length);
			memcpy(data+total_length, serialized, length);
			
			total_length += length;
			delete[] tempdata;
			tempdata = NULL;
			delete[] serialized;
			serialized = NULL;

			if((const ItemInst*)eq->inst,eq->slot_id >= 22 && (const ItemInst*)eq->inst,eq->slot_id < 30)
			{
				for(int x = 0; x < 10; ++x)
				{
					serialized = NULL;
					uint32 sub_length = 0;
					const ItemInst* subitem = ((const ItemInst*)eq->inst)->GetItem(x);
					if(subitem)
					{
						serialized = SerializeItem(subitem, (((eq->slot_id+3)*10)+x+1), &sub_length, 0);
						if(serialized)
						{
							tempdata = data;
							data = NULL;
							data = new uchar[total_length+sub_length];
							memcpy(data, tempdata, total_length);
							memcpy(data+total_length, serialized, sub_length);
							total_length += length;
							delete[] tempdata;
							tempdata = NULL;
							delete[] serialized;
							serialized = NULL;
						}
					}
				}
			}

		}
		else
		{
			_log(NET__ERROR, "Serialization failed on item slot %d during OP_CharInventory.  Item skipped.",eq->slot_id);
		}
	}

	in->size = total_length;
	in->pBuffer = new unsigned char[in->size];
	memcpy(in->pBuffer, data, in->size);

	delete[] __emu_buffer;

	_log(NET__ERROR, "Sending inventory to client");
	_hex(NET__ERROR, in->pBuffer, in->size);

	dest->FastQueuePacket(&in, ack_req);*/
}


ENCODE(OP_GuildMemberList) {
	//consume the packet
	EQApplicationPacket *in = *p;
	*p = NULL;
	
	//store away the emu struct
	unsigned char *__emu_buffer = in->pBuffer;
	Internal_GuildMembers_Struct *emu = (Internal_GuildMembers_Struct *) in->pBuffer;
	
	
	
	//make a new EQ buffer.
	int32 pnl = strlen(emu->player_name);
	uint32 length = sizeof(structs::GuildMembers_Struct) + pnl + 
		emu->count*sizeof(structs::GuildMemberEntry_Struct)
		+ emu->name_length + emu->note_length;
	in->pBuffer = new uint8[length];
	in->size = length;
	//no memset since we fill every byte.
	
	uint8 *buffer;
	buffer = in->pBuffer;
	
	//easier way to setup GuildMembers_Struct
	//set prefix name
	strcpy((char *)buffer, emu->player_name);
	buffer += pnl;
	*buffer = '\0';
	buffer++;
	
	//add member count.
	*((uint32 *) buffer) = htonl( emu->count );
	buffer += sizeof(uint32);
	
	if(emu->count > 0) {
		Internal_GuildMemberEntry_Struct *emu_e = emu->member;
		const char *emu_name = (const char *) (__emu_buffer + 
				sizeof(Internal_GuildMembers_Struct) + //skip header
				emu->count * sizeof(Internal_GuildMemberEntry_Struct)	//skip static length member data
				);
		const char *emu_note = (emu_name +
				emu->name_length + //skip name contents
				emu->count	//skip string terminators
				);
		
		structs::GuildMemberEntry_Struct *e = (structs::GuildMemberEntry_Struct *) buffer;
		
		uint32 r;
		for(r = 0; r < emu->count; r++, emu_e++) {
			
			//the order we set things here must match the struct

//nice helper macro
/*#define SlideStructString(field, str) \
		strcpy(e->field, str.c_str()); \
		e = (GuildMemberEntry_Struct *) ( ((uint8 *)e) + str.length() )*/
#define SlideStructString(field, str) \
		{ \
			int sl = strlen(str); \
			strcpy(e->field, str); \
			e = (structs::GuildMemberEntry_Struct *) ( ((uint8 *)e) + sl ); \
			str += sl + 1; \
		}
#define PutFieldN(field) \
		e->field = htonl(emu_e->field)
			
			SlideStructString( name, emu_name );
			PutFieldN(level);
			PutFieldN(banker);
			PutFieldN(class_);
			PutFieldN(rank);
			PutFieldN(time_last_on);
			PutFieldN(tribute_enable);
			PutFieldN(total_tribute);
			PutFieldN(last_tribute);
			e->unknown_one = htonl(1);
			SlideStructString( public_note, emu_note );
			e->zoneinstance = 0;
			e->zone_id = htons(emu_e->zone_id);
			
			
#undef SlideStructString
#undef PutFieldN
			
			e++;
		}
	}
	

	delete[] __emu_buffer;

	dest->FastQueuePacket(&in, ack_req);
}


ENCODE(OP_SpawnDoor) {
	SETUP_VAR_ENCODE(Door_Struct);
	int door_count = __packet->size/sizeof(Door_Struct);
	int total_length = door_count * sizeof(structs::Door_Struct);
	ALLOC_VAR_ENCODE(structs::Door_Struct, total_length);
	int r;
	for(r = 0; r < door_count; r++) {
		strcpy(eq[r].name, emu[r].name);
		eq[r].xPos = emu[r].xPos;
		eq[r].yPos = emu[r].yPos;
		eq[r].zPos = emu[r].zPos;
		eq[r].heading = emu[r].heading;
		eq[r].incline = emu[r].incline;
		eq[r].size = emu[r].size;
		eq[r].doorId = emu[r].doorId;
		eq[r].opentype = emu[r].opentype;
		eq[r].state_at_spawn = emu[r].state_at_spawn;
		eq[r].invert_state = emu[r].invert_state;
		eq[r].door_param = emu[r].door_param;
		eq[r].unknown0076 = 0;
		eq[r].unknown0077 = 1; // Both must be 1 to allow clicking doors
		eq[r].unknown0078 = 0;
		eq[r].unknown0079 = 1; // Both must be 1 to allow clicking doors
		eq[r].unknown0080 = 0;
		eq[r].unknown0081 = 0;
		eq[r].unknown0082 = 0;
	}
	FINISH_ENCODE();
}

ENCODE(OP_GroundSpawn) {
	ENCODE_LENGTH_EXACT(Object_Struct);
	SETUP_DIRECT_ENCODE(Object_Struct, structs::Object_Struct);
	OUT(drop_id);
	OUT(zone_id);
	OUT(zone_instance);
	OUT(heading);
	OUT(x);
	OUT(y);
	OUT(z);
	OUT_str(object_name);
	OUT(object_type);
	OUT(spawn_id);

    /*fill in some unknowns with observed values, hopefully it will help */
	eq->unknown020 = 0;
	eq->unknown024 = 0;
	eq->size = 1;	//This forces all objects to standard size for now
	eq->unknown088 = 0;
	memset(eq->unknown096, 0xFF, sizeof(eq->unknown096));
	FINISH_ENCODE();
}

ENCODE(OP_ManaChange) {
	ENCODE_LENGTH_EXACT(ManaChange_Struct);
	SETUP_DIRECT_ENCODE(ManaChange_Struct, structs::ManaChange_Struct);
	OUT(new_mana);
	OUT(stamina);
	OUT(spell_id);
	FINISH_ENCODE();
}

ENCODE(OP_Illusion) {
	ENCODE_LENGTH_EXACT(Illusion_Struct);
	SETUP_DIRECT_ENCODE(Illusion_Struct, structs::Illusion_Struct);
	OUT(spawnid);
	OUT_str(charname);
	OUT(race);
	OUT(unknown006[0]);
	OUT(unknown006[1]);
	OUT(gender);
	OUT(texture);
	eq->unknown008 = 0;		//
	eq->unknown009 = 0;		//
	OUT(helmtexture);
	eq->unknown010 = 0;		//
	eq->unknown011 = 0;		//
	eq->unknown012 = 0;		//
	OUT(face);				// This doesn't set face yet.  Illusion function needs to be corrected
	eq->hairstyle = 0;		// hairstyle
	eq->haircolor = 1;		// haircolor
	eq->beard = 1;			// beard
	eq->beardcolor = 1;		// beardcolor
	FINISH_ENCODE();
}

ENCODE(OP_WearChange) {
	ENCODE_LENGTH_EXACT(WearChange_Struct);
	SETUP_DIRECT_ENCODE(WearChange_Struct, structs::WearChange_Struct);
	OUT(spawn_id);
	OUT(material);
	OUT(color.color);
	OUT(wear_slot_id);
	FINISH_ENCODE();
}


ENCODE(OP_ClientUpdate) {
	ENCODE_LENGTH_EXACT(PlayerPositionUpdateServer_Struct);
	SETUP_DIRECT_ENCODE(PlayerPositionUpdateServer_Struct, structs::PlayerPositionUpdateServer_Struct);
	OUT(spawn_id);
	OUT(x_pos);
	OUT(delta_x);
	OUT(delta_y);
	OUT(z_pos);
	OUT(delta_heading);
	OUT(y_pos);
	OUT(delta_z);
	OUT(animation);
	OUT(heading);
	FINISH_ENCODE();
}

ENCODE(OP_ExpansionInfo) {
	ENCODE_LENGTH_EXACT(ExpansionInfo_Struct);
	SETUP_DIRECT_ENCODE(ExpansionInfo_Struct, structs::ExpansionInfo_Struct);
	OUT(Expansions);
	FINISH_ENCODE();
}

ENCODE(OP_LogServer) {
	ENCODE_LENGTH_EXACT(LogServer_Struct);
 	SETUP_DIRECT_ENCODE(LogServer_Struct, structs::LogServer_Struct);
 	strcpy(eq->worldshortname, emu->worldshortname);
 
 	OUT(enablevoicemacros);
 	OUT(enablemail);
 
 	// These next two need to be set like this for the Tutorial Button to work.
 	eq->unknown263[0] = 0;
 	eq->unknown263[2] = 1;
 
 	FINISH_ENCODE();
}

ENCODE(OP_Damage) {
	ENCODE_LENGTH_EXACT(CombatDamage_Struct);
	SETUP_DIRECT_ENCODE(CombatDamage_Struct, structs::CombatDamage_Struct);
	OUT(target);
	OUT(source);
	OUT(type);
	OUT(spellid);
	OUT(damage);
	eq->sequence = emu->sequence;
	FINISH_ENCODE();
}

ENCODE(OP_Consider) {
	ENCODE_LENGTH_EXACT(Consider_Struct);
	SETUP_DIRECT_ENCODE(Consider_Struct, structs::Consider_Struct);
	OUT(playerid);
	OUT(targetid);
	OUT(faction);
	OUT(level);
	OUT(pvpcon);
	FINISH_ENCODE();
}

ENCODE(OP_Action) {
	ENCODE_LENGTH_EXACT(Action_Struct);
	SETUP_DIRECT_ENCODE(Action_Struct, structs::Action_Struct);
	OUT(target);
	OUT(source);
	OUT(level);
	OUT(instrument_mod);
	eq->sequence = emu->sequence;
	OUT(type);
	//OUT(damage);
	OUT(spell);
	eq->level2 = emu->level;
	OUT(buff_unknown); // if this is 4, a buff icon is made
	//eq->unknown0036 = -1;
	//eq->unknown0040 = -1;
	//eq->unknown0044 = -1;
	FINISH_ENCODE();
}

ENCODE(OP_Buff) {
	ENCODE_LENGTH_EXACT(SpellBuffFade_Struct);
	SETUP_DIRECT_ENCODE(SpellBuffFade_Struct, structs::SpellBuffFade_Struct);
	OUT(entityid);
	OUT(slot);
	OUT(level);
	OUT(effect);
	//eq->unknown7 = 10;
	OUT(spellid);
	OUT(duration);
	OUT(slotid);
	OUT(bufffade);
	FINISH_ENCODE();
}

ENCODE(OP_CancelTrade) {
	ENCODE_LENGTH_EXACT(CancelTrade_Struct);
	SETUP_DIRECT_ENCODE(CancelTrade_Struct, structs::CancelTrade_Struct);
	OUT(fromid);
	OUT(action);
	FINISH_ENCODE();
}

ENCODE(OP_InterruptCast) {
	ENCODE_LENGTH_EXACT(InterruptCast_Struct);
	SETUP_DIRECT_ENCODE(InterruptCast_Struct, structs::InterruptCast_Struct);
	OUT(spawnid);
	OUT(messageid);
	FINISH_ENCODE();
}


ENCODE(OP_RespondAA) {
	ENCODE_LENGTH_EXACT(AATable_Struct);
	SETUP_DIRECT_ENCODE(AATable_Struct, structs::AATable_Struct);
	
	//int aa_spent_total = 0;
	eq->aa_spent = 500; //aa_spent_total;

	int r;
	for(r = 0; r < structs::MAX_PP_AA_ARRAY; r++) {
		OUT(aa_list[r].aa_skill);
		OUT(aa_list[r].aa_value);
		//eq->aa_list[r].aa_value = emu->aa_list[r].aa_value;
		//if (emu->aa_list[r].aa_value)
		//	aa_spent_total += emu->aa_list[r].aa_value;
		eq->aa_list[r].unknown08 = 0;
	}
	FINISH_ENCODE();
}

ENCODE(OP_ShopPlayerSell) {
	ENCODE_LENGTH_EXACT(Merchant_Purchase_Struct);
	SETUP_DIRECT_ENCODE(Merchant_Purchase_Struct, structs::Merchant_Purchase_Struct);
	OUT(npcid);

	int slot_id;
	slot_id = emu->itemslot;

	if(slot_id >= 21 && slot_id < 50)
		slot_id += 1;
	else if(slot_id >= 251 && slot_id < 351)
		slot_id += 11;
	
	eq->itemslot = slot_id;

	OUT(quantity);
	OUT(price);
	FINISH_ENCODE();
}

ENCODE(OP_DeleteItem) {
	ENCODE_LENGTH_EXACT(DeleteItem_Struct);
	SETUP_DIRECT_ENCODE(DeleteItem_Struct, structs::DeleteItem_Struct);
	if(emu->from_slot >= 21 && emu->from_slot < 50)
	{
		eq->from_slot = emu->from_slot + 1;
	}
	else if(emu->from_slot >= 251 && emu->from_slot < 351)
	{
		eq->from_slot = emu->from_slot + 11;
	}
	else if(emu->from_slot >= 2031 && emu->from_slot < 2270)
	{
		eq->from_slot = emu->from_slot + 1;
	}
	else if(emu->from_slot >= 2531 && emu->from_slot < 2550)
	{
		eq->from_slot = emu->from_slot + 1;
	}
	else
	{
		OUT(from_slot);
	}

	if(emu->to_slot >= 21 && emu->to_slot < 50)
	{
		eq->to_slot = emu->to_slot + 1;
	}
	else if(emu->to_slot >= 251 && emu->to_slot < 351)
	{
		eq->to_slot = emu->to_slot + 11;
	}
	else if(emu->to_slot >= 2031 && emu->to_slot < 2270)
	{
		eq->to_slot = emu->to_slot + 1;
	}
	else if(emu->to_slot >= 2531 && emu->to_slot < 2550)
	{
		eq->to_slot = emu->to_slot + 1;
	}
	else
	{
		OUT(to_slot);
	}
	OUT(number_in_stack);
	FINISH_ENCODE();
}

ENCODE(OP_DeleteCharge) {  ENCODE_FORWARD(OP_MoveItem); }
ENCODE(OP_MoveItem) {
	ENCODE_LENGTH_EXACT(MoveItem_Struct);
	SETUP_DIRECT_ENCODE(MoveItem_Struct, structs::MoveItem_Struct);
	if(emu->from_slot >= 21 && emu->from_slot < 50)
	{
		eq->from_slot = emu->from_slot + 1;
	}
	else if(emu->from_slot >= 251 && emu->from_slot < 351)
	{
		eq->from_slot = emu->from_slot + 11;
	}
	else if(emu->from_slot >= 2031 && emu->from_slot < 2270)
	{
		eq->from_slot = emu->from_slot + 1;
	}
	else if(emu->from_slot >= 2531 && emu->from_slot < 2550)
	{
		eq->from_slot = emu->from_slot + 1;
	}
	else
	{
		OUT(from_slot);
	}

	if(emu->to_slot >= 21 && emu->to_slot < 50)
	{
		eq->to_slot = emu->to_slot + 1;
	}
	else if(emu->to_slot >= 251 && emu->to_slot < 351)
	{
		eq->to_slot = emu->to_slot + 11;
	}
	else if(emu->to_slot >= 2031 && emu->to_slot < 2270)
	{
		eq->to_slot = emu->to_slot + 1;
	}
	else if(emu->to_slot >= 2531 && emu->to_slot < 2550)
	{
		eq->to_slot = emu->to_slot + 1;
	}
	else
	{
		OUT(to_slot);
	}
	OUT(number_in_stack);
	FINISH_ENCODE();
}

ENCODE(OP_ItemVerifyReply) {
	ENCODE_LENGTH_EXACT(ItemVerifyReply_Struct);
	SETUP_DIRECT_ENCODE(ItemVerifyReply_Struct, structs::ItemVerifyReply_Struct);
	if(emu->slot >= 21 && emu->slot < 50)
	{
		eq->slot = emu->slot + 1;
	}
	else
	{
		OUT(slot);
	}
	OUT(spell);
	OUT(target);
	FINISH_ENCODE();
}

DECODE(OP_ItemVerifyRequest) {
	DECODE_LENGTH_EXACT(structs::ItemVerifyRequest_Struct);
	SETUP_DIRECT_DECODE(ItemVerifyRequest_Struct, structs::ItemVerifyRequest_Struct);
	if(eq->slot >= 22 && eq->slot < 51)
	{
		emu->slot = eq->slot - 1;
	}
	else if(eq->slot == 21)
	{
		emu->slot = 22;		//some power source slot TODO
	}
	else
	{
		IN(slot);
	}
	IN(target);
	FINISH_DIRECT_DECODE();
}

DECODE(OP_Consume) {
	DECODE_LENGTH_EXACT(structs::Consume_Struct);
	SETUP_DIRECT_DECODE(Consume_Struct, structs::Consume_Struct);
	if(eq->slot >= 22 && eq->slot < 51)
	{
		emu->slot = eq->slot - 1;
	}
	else if(eq->slot == 21)
	{
		emu->slot = 22;		//some power source slot TODO
	}
	else
	{
		IN(slot);
	}
	IN(auto_consumed);
	//IN(c_unknown1);
	IN(type);
	//IN(unknown13);
	FINISH_DIRECT_DECODE();
}

DECODE(OP_CastSpell) {
	DECODE_LENGTH_EXACT(structs::CastSpell_Struct);
	SETUP_DIRECT_DECODE(CastSpell_Struct, structs::CastSpell_Struct);
	IN(slot);
	IN(spell_id);

	if(eq->inventoryslot >= 22 && eq->inventoryslot < 51)
	{
		emu->inventoryslot = eq->inventoryslot - 1;
	}
	else if(eq->inventoryslot == 21)
	{
		emu->inventoryslot = 22;		//some power source slot TODO
	}
	else
	{
		IN(inventoryslot);
	}
	IN(target_id);
	//IN(cs_unknown);
	FINISH_DIRECT_DECODE();
}

DECODE(OP_MoveItem)
{
	DECODE_LENGTH_EXACT(structs::MoveItem_Struct);
	SETUP_DIRECT_DECODE(MoveItem_Struct, structs::MoveItem_Struct);

	_log(NET__ERROR, "Moved item from %u to %u", eq->from_slot, eq->to_slot);

	if(eq->from_slot >= 22 && eq->from_slot < 51)
	{
		emu->from_slot = eq->from_slot - 1;
	}
	else if(eq->from_slot >= 251 && eq->from_slot < 351)
	{
		emu->from_slot = eq->from_slot - 11;
	}
	else if(eq->from_slot >= 2032 && eq->from_slot < 2271)
	{
		emu->from_slot = eq->from_slot - 1;
	}
	else if(eq->from_slot >= 2532 && eq->from_slot < 2551)
	{
		emu->from_slot = eq->from_slot - 1;
	}
	else if(eq->from_slot == 21)
	{
		emu->from_slot = 22;//some power source slot TODO
	}
	else
	{
		IN(from_slot);
	}

	if(eq->to_slot >= 22 && eq->to_slot < 51)
	{
		emu->to_slot = eq->to_slot - 1;
	}
	else if(eq->to_slot >= 251 && eq->to_slot < 351)
	{
		emu->to_slot = eq->to_slot - 11;
	}
	else if(eq->to_slot >= 2031 && eq->to_slot < 2271)
	{
		emu->to_slot = eq->to_slot - 1;
	}
	else if(eq->to_slot >= 2532 && eq->to_slot < 2551)
	{
		emu->to_slot = eq->to_slot - 1;
	}
	else if(eq->to_slot == 21)
	{
		emu->to_slot = 22;//some power source slot TODO
	}
	else
	{
		IN(to_slot);
	}
	IN(number_in_stack);

	FINISH_DIRECT_DECODE();
}

DECODE(OP_ItemLinkClick) {
	DECODE_LENGTH_EXACT(structs::ItemViewRequest_Struct);
	SETUP_DIRECT_DECODE(ItemViewRequest_Struct, structs::ItemViewRequest_Struct);
	MEMSET_IN(ItemViewRequest_Struct);
	
	IN(item_id);
	int r;
	for (r = 0; r < 5; r++) {
		IN(augments[r]);
	}
	IN(link_hash);
	IN(icon);
	
	FINISH_DIRECT_DECODE();
}

DECODE(OP_SetServerFilter) {
	DECODE_LENGTH_EXACT(structs::SetServerFilter_Struct);
	SETUP_DIRECT_DECODE(SetServerFilter_Struct, structs::SetServerFilter_Struct);
	int r;
	for(r = 0; r < 29; r++) {
		IN(filters[r]);
	}
	FINISH_DIRECT_DECODE();
}

DECODE(OP_ConsiderCorpse) { DECODE_FORWARD(OP_Consider); }
DECODE(OP_Consider) {
	DECODE_LENGTH_EXACT(structs::Consider_Struct);
	SETUP_DIRECT_DECODE(Consider_Struct, structs::Consider_Struct);
	IN(playerid);
	IN(targetid);
	IN(faction);
	IN(level);
	//emu->cur_hp = 1;
	//emu->max_hp = 2;
	//emu->pvpcon = 0;
	FINISH_DIRECT_DECODE();
}


DECODE(OP_WearChange) {
	DECODE_LENGTH_EXACT(structs::WearChange_Struct);
	SETUP_DIRECT_DECODE(WearChange_Struct, structs::WearChange_Struct);
	IN(spawn_id);
	IN(material);
	IN(color.color);
	IN(wear_slot_id);
	FINISH_DIRECT_DECODE();
}


DECODE(OP_ClientUpdate) {
    // for some odd reason, there is an extra byte on the end of this on occasion.. 
	DECODE_LENGTH_ATLEAST(structs::PlayerPositionUpdateClient_Struct);
	SETUP_DIRECT_DECODE(PlayerPositionUpdateClient_Struct, structs::PlayerPositionUpdateClient_Struct);
	IN(spawn_id);
	IN(sequence);
	IN(x_pos);
	IN(y_pos);
	IN(z_pos);
	IN(heading);
	IN(delta_x);
	IN(delta_y);
	IN(delta_z);
	IN(delta_heading);
	IN(animation);
	FINISH_DIRECT_DECODE();
}


DECODE(OP_CharacterCreate) {
	DECODE_LENGTH_EXACT(structs::CharCreate_Struct);
	SETUP_DIRECT_DECODE(CharCreate_Struct, structs::CharCreate_Struct);
	IN(class_);
	IN(beardcolor);
	IN(beard);
	IN(haircolor);
	IN(gender);
	IN(race);

	if(RuleB(World, EnableTutorialButton) && eq->tutorial)
		emu->start_zone = RuleI(World, TutorialZoneID);
	else
		emu->start_zone = eq->start_zone;
		
	IN(hairstyle);
	IN(deity);
	IN(STR);
	IN(STA);
	IN(AGI);
	IN(DEX);
	IN(WIS);
	IN(INT);
	IN(CHA);
	IN(face);
	IN(eyecolor1);
	IN(eyecolor2);
	FINISH_DIRECT_DECODE();
}


DECODE(OP_WhoAllRequest) {
	DECODE_LENGTH_EXACT(structs::Who_All_Struct);
	SETUP_DIRECT_DECODE(Who_All_Struct, structs::Who_All_Struct);

	memcpy(emu->whom, eq->whom, sizeof(emu->whom));
	IN(wrace);
	IN(wclass);
	IN(lvllow);
	IN(lvlhigh);
	IN(gmlookup);
	
	FINISH_DIRECT_DECODE();
}

DECODE(OP_GroupFollow) {
	DECODE_LENGTH_EXACT(structs::GroupFollow_Struct);
	SETUP_DIRECT_DECODE(GroupGeneric_Struct, structs::GroupFollow_Struct);
	memcpy(emu->name1, eq->name1, sizeof(emu->name1));
	memcpy(emu->name2, eq->name2, sizeof(emu->name2));
	FINISH_DIRECT_DECODE();
}

DECODE(OP_GroupFollow2) {
	DECODE_LENGTH_EXACT(structs::GroupFollow_Struct);
	SETUP_DIRECT_DECODE(GroupGeneric_Struct, structs::GroupFollow_Struct);
	memcpy(emu->name1, eq->name1, sizeof(emu->name1));
	memcpy(emu->name2, eq->name2, sizeof(emu->name2));
	FINISH_DIRECT_DECODE();
}

DECODE(OP_Buff) {
	DECODE_LENGTH_EXACT(structs::SpellBuffFade_Struct);
	SETUP_DIRECT_DECODE(SpellBuffFade_Struct, structs::SpellBuffFade_Struct);
	IN(entityid);
	IN(slot);
	IN(level);
	IN(effect);
	IN(spellid);
	IN(duration);
	IN(slotid);
	IN(bufffade);
	FINISH_DIRECT_DECODE();
}

DECODE(OP_ShopPlayerSell) {
	DECODE_LENGTH_EXACT(structs::Merchant_Purchase_Struct);
	SETUP_DIRECT_DECODE(Merchant_Purchase_Struct, structs::Merchant_Purchase_Struct);
	IN(npcid);

	int slot_id;
	slot_id = eq->itemslot;

	if(slot_id >= 22 && slot_id < 51)
		slot_id -= 1;
	else if(slot_id >= 262 && slot_id < 351)
		slot_id -= 11;

	emu->itemslot = slot_id;

	IN(quantity);
	IN(price);
	FINISH_DIRECT_DECODE();
}

DECODE(OP_Save) {
	DECODE_LENGTH_EXACT(structs::Save_Struct);
	SETUP_DIRECT_DECODE(Save_Struct, structs::Save_Struct);
	memcpy(emu->unknown00, eq->unknown00, sizeof(emu->unknown00));
	FINISH_DIRECT_DECODE();
}

DECODE(OP_FindPersonRequest) {
	DECODE_LENGTH_EXACT(structs::FindPersonRequest_Struct);
	SETUP_DIRECT_DECODE(FindPersonRequest_Struct, structs::FindPersonRequest_Struct);
	IN(npc_id);
	IN(client_pos.x);
	IN(client_pos.y);
	IN(client_pos.z);
	FINISH_DIRECT_DECODE();
}

int32 NextItemInstSerialNumber = 1;
int32 MaxInstances = 2000000000;

static inline sint32 GetNextItemInstSerialNumber() {

	if(NextItemInstSerialNumber >= MaxInstances)
		NextItemInstSerialNumber = 1;
	else
		NextItemInstSerialNumber++;

	return NextItemInstSerialNumber;
}


char* SerializeItem(const ItemInst *inst, sint16 slot_id, uint32 *length, uint8 depth) {
	char *serialization = NULL;
	uint8 null_term = 0;
	bool stackable = inst->IsStackable();
	uint32 merchant_slot = inst->GetMerchantSlot();
	sint16 charges = inst->GetCharges();

	
	std::stringstream ss(std::stringstream::in | std::stringstream::out | std::stringstream::binary);
	ss.clear();

	const Item_Struct *item = inst->GetItem();
	SoF::structs::ItemSerializationHeader hdr;
	hdr.stacksize = stackable ? charges : 1;
	hdr.unknown004 = 0;

	if(slot_id >= 22 && slot_id < 50) // Ammo and Main Inventory
		slot_id += 1;
	else if(slot_id >= 251 && slot_id < 351) // Inventory Bg Slots
		slot_id += 11;
	else if(slot_id >= 2031 && slot_id < 2270) // Bank Bag Slots
		slot_id += 1;
	else if(slot_id >= 2531 && slot_id < 2550) // Shared Bank Bag Slots
		slot_id += 1;

	if(slot_id == 21)
		slot_id = 22;

	// It looks like Power Source is slot 21 and Ammo got bumped to slot 22
	// This will have to be changed somehow to handle slot 21 for Power Source items

	hdr.slot = (merchant_slot == 0) ? slot_id : merchant_slot;
	hdr.price = inst->GetPrice();
	hdr.merchant_slot = (merchant_slot == 0) ? 1 : inst->GetMerchantCount();
	hdr.unknown020 = 0;
	hdr.instance_id = (merchant_slot == 0) ? inst->GetSerialNumber() : merchant_slot;
	hdr.inst_nodrop = inst->IsInstNoDrop() ? 1 : 0;
	hdr.potion_type = (stackable ? ((inst->GetItem()->ItemType == ItemTypePotion) ? 1 : 0) : charges);
	hdr.charges = charges;
	hdr.unknown040 = 0;
	hdr.unknown044 = 0;
	hdr.unknown048 = 0;
	hdr.unknown052 = 0;
	hdr.unknown056 = 0;
	hdr.unknown060 = 0;
	hdr.unknown061 = 0;
	hdr.ItemClass = item->ItemClass;

	ss.write((const char*)&hdr, sizeof(SoF::structs::ItemSerializationHeader));

	if(strlen(item->Name) > 0)
	{
		ss.write(item->Name, strlen(item->Name));
		ss.write((const char*)&null_term, sizeof(uint8));
	}
	else
	{
		ss.write((const char*)&null_term, sizeof(uint8));
	}

	if(strlen(item->Lore) > 0)
	{
		ss.write(item->Lore, strlen(item->Lore));
		ss.write((const char*)&null_term, sizeof(uint8));
	}
	else
	{
		ss.write((const char*)&null_term, sizeof(uint8));
	}

	if(strlen(item->IDFile) > 0)
	{
		ss.write(item->IDFile, strlen(item->IDFile));
		ss.write((const char*)&null_term, sizeof(uint8));
	}
	else
	{
		ss.write((const char*)&null_term, sizeof(uint8));
	}

	SoF::structs::ItemBodyStruct ibs;
	memset(&ibs, 0, sizeof(SoF::structs::ItemBodyStruct));

	uint32 adjusted_slots = item->Slots;

	if(item->Slots & (1 << 21))
	{
		adjusted_slots -= (1 << 21);
		adjusted_slots += (1 << 22);
	}

	ibs.id = item->ID;
	ibs.weight = item->Weight;
	ibs.norent = item->NoRent;
	ibs.nodrop = item->NoDrop;
	ibs.attune = item->Attuneable;
	ibs.size = item->Size;
	ibs.slots = adjusted_slots;
	ibs.price = item->Price;
	ibs.icon = item->Icon;
	ibs.unknown1 = 1;
	ibs.unknown2 = 1;
	ibs.BenefitFlag = item->BenefitFlag;
	ibs.tradeskills = item->Tradeskills;
	ibs.CR = item->CR;
	ibs.DR = item->DR;
	ibs.PR = item->PR;
	ibs.MR = item->MR;
	ibs.FR = item->FR;
	ibs.Corruption = 0; //NYI
	ibs.AStr = item->AStr;
	ibs.ASta = item->ASta;
	ibs.AAgi = item->AAgi;
	ibs.ADex = item->ADex;
	ibs.ACha = item->ACha;
	ibs.AInt = item->AInt;
	ibs.AWis = item->AWis;

	ibs.HP = item->HP;
	ibs.Mana = item->Mana;
	ibs.Endur = item->Endur;
	ibs.AC = item->AC;
	ibs.regen = item->Regen;
	ibs.mana_regen = item->ManaRegen;
	ibs.end_regen = item->EnduranceRegen;
	ibs.Classes = item->Classes;
	ibs.Races = item->Races;
	ibs.Deity = item->Deity;
	ibs.SkillModValue = item->SkillModValue;
	ibs.unknown6 = 0;
	ibs.SkillModType = item->SkillModType;
	ibs.BaneDmgRace = item->BaneDmgRace;
	ibs.BaneDmgBody = item->BaneDmgBody;
	ibs.BaneDmgRaceAmt = item->BaneDmgRaceAmt;
	ibs.BaneDmgAmt = item->BaneDmgAmt;
	ibs.Magic = item->Magic;
	ibs.CastTime_ = item->CastTime_;
	ibs.ReqLevel = item->ReqLevel;
	ibs.RecLevel = item->RecLevel;
	ibs.RecSkill = item->RecSkill;
	ibs.BardType = item->BardType;
	ibs.BardValue = item->BardValue;
	ibs.Light = item->Light;
	ibs.Delay = item->Delay;
	ibs.ElemDmgType = item->ElemDmgType;
	ibs.ElemDmgAmt = item->ElemDmgAmt;
	ibs.Range = item->Range;
	ibs.Damage = item->Damage;
	ibs.Color = item->Color;
	ibs.ItemType = item->ItemType;
	ibs.Material = item->Material;
	ibs.unknown7 = 0;
	ibs.unknown8 = 0;
	ibs.SellRate = 0;

	ibs.CombatEffects = item->CombatEffects;
	ibs.Shielding = item->Shielding;
	ibs.StunResist = item->StunResist;
	ibs.StrikeThrough = item->StrikeThrough;
	ibs.ExtraDmgSkill = item->ExtraDmgSkill;
	ibs.ExtraDmgAmt = item->ExtraDmgAmt;
	ibs.SpellShield = item->SpellShield;
	ibs.Avoidance = item->Avoidance;
	ibs.Accuracy = item->Accuracy;
	ibs.FactionAmt1 = item->FactionAmt1;
	ibs.FactionMod1 = item->FactionMod1;
	ibs.FactionAmt2 = item->FactionAmt2;
	ibs.FactionMod2 = item->FactionMod2;
	ibs.FactionAmt3 = item->FactionAmt3;
	ibs.FactionMod3 = item->FactionMod3;
	ibs.FactionAmt4 = item->FactionAmt4;
	ibs.FactionMod4 = item->FactionMod4;

	ss.write((const char*)&ibs, sizeof(SoF::structs::ItemBodyStruct));

	//charm text
	if(strlen(item->CharmFile) > 0)
	{
		ss.write((const char*)item->CharmFile, strlen(item->CharmFile));
		ss.write((const char*)&null_term, sizeof(uint8));
	}
	else
	{
		ss.write((const char*)&null_term, sizeof(uint8));
	}

	SoF::structs::ItemSecondaryBodyStruct isbs;
	memset(&isbs, 0, sizeof(SoF::structs::ItemSecondaryBodyStruct));

	isbs.augtype = item->AugType;
	isbs.augrestrict = item->AugRestrict;

	for(int x = 0; x < 5; ++x)
	{
		isbs.augslots[x].type = item->AugSlotType[x];
		isbs.augslots[x].visible = item->AugSlotVisible[x];
		isbs.augslots[x].unknown = item->AugSlotUnk2[x];
	}

	isbs.ldonpoint_type = 0;
	isbs.ldontheme = item->LDoNTheme;
	isbs.ldonprice = item->LDoNPrice;
	isbs.unk098 = 70;
	isbs.ldonsold = item->LDoNSold;

	isbs.bagtype = item->BagType;
	isbs.bagslots = item->BagSlots;
	isbs.bagsize = item->BagSize;
	isbs.wreduction = item->BagWR;

	isbs.book = item->Book;
	isbs.booktype = item->BookType;

	ss.write((const char*)&isbs, sizeof(SoF::structs::ItemSecondaryBodyStruct));

	if(strlen(item->Filename) > 0)
	{
		ss.write((const char*)item->Filename, strlen(item->Filename));
		ss.write((const char*)&null_term, sizeof(uint8));
	}
	else
	{
		ss.write((const char*)&null_term, sizeof(uint8));
	}

	SoF::structs::ItemTiertaryBodyStruct itbs;
	memset(&itbs, 0, sizeof(SoF::structs::ItemTiertaryBodyStruct));
	
	itbs.loregroup = item->LoreGroup;
	itbs.artifact = item->ArtifactFlag;
	itbs.pendinglore = item->PendingLoreFlag;
	itbs.favor = item->Favor;
	itbs.fvnodrop = item->FVNoDrop;
	itbs.dotshield = item->DotShielding;
	itbs.atk = item->Attack;
	itbs.haste = item->Haste;
	itbs.damage_shield = item->DamageShield;
	itbs.guildfavor = item->GuildFavor;
	itbs.augdistil = item->AugDistiller;
	itbs.no_pet = item->NoPet;

	itbs.potion_belt_enabled = item->PotionBelt;
	itbs.potion_belt_slots = item->PotionBeltSlots;
	itbs.no_transfer = item->NoTransfer;
	itbs.stacksize = stackable ? item->StackSize : 0;

	itbs.click_effect.effect = item->Click.Effect;
	itbs.click_effect.type = item->Click.Type;
	itbs.click_effect.level = item->Click.Level;
	itbs.click_effect.max_charges = item->MaxCharges;
	itbs.click_effect.cast_time = item->CastTime;
	itbs.click_effect.recast = item->RecastDelay;
	itbs.click_effect.recast_type = item->RecastType;

	itbs.proc_effect.effect = item->Proc.Effect;
	itbs.proc_effect.level2 = item->Proc.Level2;
	itbs.proc_effect.type = item->Proc.Type;
	itbs.proc_effect.level = item->Proc.Level;
	itbs.proc_effect.procrate = item->ProcRate;

	itbs.worn_effect.effect = item->Worn.Effect;
	itbs.worn_effect.level2 = item->Worn.Level2;
	itbs.worn_effect.type = item->Worn.Type;
	itbs.worn_effect.level = item->Worn.Level;

	itbs.focus_effect.effect = item->Focus.Effect;
	itbs.focus_effect.level2 = item->Focus.Level2;
	itbs.focus_effect.type = item->Focus.Type;
	itbs.focus_effect.level = item->Focus.Level;

	itbs.scroll_effect.effect = item->Scroll.Effect;
	itbs.scroll_effect.level2 = item->Scroll.Level2;
	itbs.scroll_effect.type = item->Scroll.Type;
	itbs.scroll_effect.level = item->Scroll.Level;

	itbs.quest_item = item->QuestItemFlag;
	itbs.unknown15 = 0xffffffff;

	ss.write((const char*)&itbs, sizeof(SoF::structs::ItemTiertaryBodyStruct));

	char* item_serial = new char[ss.tellp()];
	memset(item_serial, 0, ss.tellp());
	memcpy(item_serial, ss.str().c_str(), ss.tellp());

	*length = ss.tellp();
	return item_serial;
}

} //end namespace SoF




