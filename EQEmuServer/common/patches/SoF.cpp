
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

namespace SoF {

static const char *name = "SoF";
static OpcodeManager *opcodes = NULL;
static Strategy struct_strategy;

char *SerializeItem(const ItemInst *inst, sint16 slot_id, uint32 *length, uint8 depth);
	
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
			eq2->u13 = 0;
			eq2->u14 = 0;
			eq2->u15 = 0xFF;
			eq2->deity = emu->deity[r];
			eq2->zone = emu->zone[r];
			eq2->u18 = 1;
			eq2->u19 = 0xFF;
			eq2->race = emu->race[r];
			eq2->gohome = emu->gohome[r];
			eq2->class_ = emu->class_[r];
			eq2->eyecolor1 = emu->eyecolor1[r];
			eq2->beardcolor = emu->beardcolor[r];
			eq2->eyecolor2 = emu->eyecolor2[r];
			eq2->secondary = emu->secondary[r];
			eq2->primary = emu->primary[r];
			eq2->u29 = 0;
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
	OUT(spellid);
	OUT(spell_type);
	OUT(spell_refresh);
	OUT(classes);
	OUT(berserker);
	OUT(max_level);
	OUT(last_id);
	OUT(next_id);
	OUT(cost2);
	OUT(unknown80[0]);
	//OUT(unknown80[1]);
	//eq->unknown80 = 0;
	eq->unknown0088 = 3;
	eq->unknown0092 = -1;
	eq->unknown0096 = 0;
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
		OUT(item_material[r]);
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
//	OUT(unknown13072[12]);
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
	OUT(expansions);
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
0x78,0x03,0x00,0x00,0x1A,0x04,0x00,0x00,0x1A,0x04,0x00,0x00,0x19,0x00,0x00,0x00,
0x19,0x00,0x00,0x00,0x19,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,
0x0F,0x00,0x00,0x00,0x1F,0x85,0xEB,0x3E,0x33,0x33,0x33,0x3F,0x09,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x14
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
	eq->unknown890 = 1;
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

		eq->deity = emu->deity;
		eq->gender = emu->gender;
		for(k = 0; k < 9; k++) {
			eq->equipment[k].equip0 = emu->equipment[k];
			eq->equipment[k].equip1 = 0;
			eq->equipment[k].itemId = 0;
			eq->colors[k].color = emu->colors[k].color;
		}
		eq->guildID = emu->guildID;
		eq->class_ = emu->class_;
		eq->gm = emu->gm;
		eq->helm = emu->helm;
		eq->runspeed = emu->runspeed;
		eq->light = emu->light;
		eq->level = emu->level;
		eq->race = emu->race;
		strcpy(eq->suffix, emu->suffix);
		eq->bodytype = emu->bodytype;
		eq->curHp = emu->curHp;
		strcpy(eq->lastName, emu->lastName);
		strcpy(eq->title, emu->title);
		eq->NPC = emu->NPC;
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
		eq->anon = emu->anon;
		eq->walkspeed = emu->walkspeed;

		eq->targetable = 1; //New Field - Force NPCs to Targetable for now
		eq->showname = 1; //New Field - Toggles Name Display on or off - 0 = off, 1 = on
		eq->linkdead = 0; //New Field - Toggles LD on or off after name - 0 = off, 1 = on

		//Hack Test for finding more fields in the Struct:
		//memset(eq->unknown0001, 0x01, sizeof(eq->unknown0001)); // 16 - No Visible Change? 22
		//memset(eq->unknown0005, 0x01, sizeof(eq->unknown0005)); // 15 - No Visible Change? 22
		//memset(eq->unknown0008, 0x01, sizeof(eq->unknown0008)); // 13
		//memset(eq->unknown0048, 0x01, sizeof(eq->unknown0048)); // 12 - No Visible Change? 22
		//eq->unknown0820 = 1;	//Stand State - Stand/Sit/Crouch
		//eq->unknown0059 = 1; // 1 Turned off on 6 - west bug?
		//memset(eq->unknown0074, 0x01, sizeof(eq->unknown0074)); // 16 - No Visible Change? 22
		//memset(eq->unknown0077, 0x01, sizeof(eq->unknown0077)); // 19 - Flymode 22
		//memset(eq->unknown00771, 0x00, sizeof(eq->unknown00771)); // 20 - No Visible Change? 22
		//memset(eq->unknown00772, 0x02, sizeof(eq->unknown00772)); // 20 - No Visible Change? 22
		//memset(eq->unknown0078, 0x00, sizeof(eq->unknown0078)); // 18
		//eq->unknown0078 = 1;
		//memset(eq->unknown0079, 0x01, sizeof(eq->unknown0079)); // 18 - No Visible Change? 22
		//memset(eq->unknown0080, 0x01, sizeof(eq->unknown0080)); // 18 - No Visible Change? 22
		//memset(eq->unknown0106, 0x01, sizeof(eq->unknown0106)); // 11 - No Visible Change? 22
		//memset(eq->unknown0107, 0x01, sizeof(eq->unknown0107)); // 20 21 - Flymode 22
		//memset(eq->unknown0108, 0x01, sizeof(eq->unknown0108)); // 20 - LFG and Hair/Beard 22
		//memset(eq->unknown0110, 0x01, sizeof(eq->unknown0110)); // 20 21 
		//memset(eq->unknown01101, 0x00, sizeof(eq->unknown01101)); // 20
		//eq->unknown0111 = 1; // 1 - No Visible Change?
		//eq->unknown0613 = 0; //was bodytype
		//memset(eq->unknown0154, 0x01, sizeof(eq->unknown0154)); // 2 - freeze in place?
		//memset(eq->unknown0263, 0x01, sizeof(eq->unknown0263)); // 1 - no player character visible?
		//memset(eq->unknown0281, 0x01, sizeof(eq->unknown0281)); // 2 3
		//eq->unknown0307 = 1; // 9 10 11 - No Visible Change?
		//memset(eq->unknown0308, 0x01, sizeof(eq->unknown0308)); // 22 - No Visible Change? 22
		//memset(eq->unknown0309, 0x01, sizeof(eq->unknown0309)); // 8 - No Visible Change? 22
		//memset(eq->unknown442, 0x01, sizeof(eq->unknown442)); // 6 - crash?
		//eq->unknown443 = 1; // 1 turned off on 9 - No Visible Change? 22
		//memset(eq->unknown0760, 0x01, sizeof(eq->unknown0760)); // 4 avatar height?
		//eq->unknown0779 = 0; // 1 - int32 avatar height?
		//memset(eq->unknown0496, 0x01, sizeof(eq->unknown0496)); // 4 5
		

		// 1 all set to one shows you at floor level
		// 2 seems to cause invis and freeze in place and west bug
		// 3 causes no player character, but spawns work and has west bug
		// 4 looks just like 2 accept you can move and are at floor level
		// 5 Looks like 3
		// 6 Cuased a crash
		// 7 west bug gone 
		// 8 Looks normal accept no player character
		// 9 Targetable mobs!!! Still no player character...
		// 10 Narrowing down bodytype
		// 11 Non-targetable again and narrowing down bodytype
		// 12 Bodytype location identified!!!  Targeting works...  Nother other visible changes
		// 13 Can target self but not spawns.  West bug again
		// 14 Names showing and also LD showing!!!  Still West Bug though...
		// 15 Show Names field now identified.  
		// 16 No visible change
		// 17 Looks like helm is in there somewhere...  Only 134 bytes to figure it out :P
		// 18 helm location identified
		// 19 fly mode on
		// 20 fly mode on, LFG on, very slow walking, and hair changed
		// 21 LFG on, hair changed
		// 22 Makes everyone human with a ! in front of their name, target ring is off and con color is grey...



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
	
	//store away the emu struct
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
	in->size = length+5;	// ItemPacketType + Serialization + \0
	in->pBuffer = new unsigned char[in->size];
	ItemPacket_Struct *new_item_pkt=(ItemPacket_Struct *)in->pBuffer;
	new_item_pkt->PacketType=old_item_pkt->PacketType;
	memcpy(new_item_pkt->SerializedItem,serialized,length+1);

	delete[] __emu_buffer;
	safe_delete_array(serialized);

	dest->FastQueuePacket(&in, ack_req);
}

ENCODE(OP_CharInventory) {
	//consume the packet
	EQApplicationPacket *in = *p;
	*p = NULL;
	//*
	if(in->size == 0 || true) {
		//send an empty packet for now...
		in->size = 4;
		in->pBuffer = new uchar[in->size];
		*((uint32 *) in->pBuffer) = 0;
		dest->FastQueuePacket(&in, ack_req);
		return;
	}
	//*/
	
	//store away the emu struct
	unsigned char *__emu_buffer = in->pBuffer;

	int itemcount = in->size / sizeof(InternalSerializedItem_Struct);
	if(itemcount == 0 || (in->size % sizeof(InternalSerializedItem_Struct)) != 0) {
		_log(NET__STRUCTS, "Wrong size on outbound %s: Got %d, expected multiple of %d", opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(InternalSerializedItem_Struct));
		delete in;
		return;
	}
	InternalSerializedItem_Struct *eq = (InternalSerializedItem_Struct *) in->pBuffer;
	
	//do the transform...
	int r;
	string serial_string;
	for(r = 0; r < itemcount; r++, eq++) {
		uint32 length;
		char *serialized=SerializeItem((const ItemInst*)eq->inst,eq->slot_id,&length,0);
		if (serialized) {
			serial_string.append(serialized,length+1);
			safe_delete_array(serialized);
		} else {
			_log(NET__STRUCTS, "Serialization failed on item slot %d during OP_CharInventory.  Item skipped.",eq->slot_id);
		}
	
	}

	in->size = serial_string.length();
	in->pBuffer = new unsigned char[in->size];
	memcpy(in->pBuffer,serial_string.c_str(),serial_string.length());

	delete[] __emu_buffer;

	dest->FastQueuePacket(&in, ack_req);
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
	//eq->unknown006 = 1; // These 2 make the name green with underscores and numbers after them
	//eq->unknown007 = 1; // They also make everything into a bald human
	OUT(gender);
	OUT(texture);
	OUT(face);
	eq->unknown011 = 1; // Size?
	OUT(helmtexture);
	//eq->unknown010 = 2; //
	eq->unknown012 = 1; // Maybe Size?  Seems like it may be an int16
	eq->unknown013 = 1; // Size?
	eq->unknown014 = 1; // face hair with color for both
	eq->unknown015 = 2; // face hair with color for both
	eq->unknown016 = 1; // face hair with color for both
	eq->unknown017 = 1; // face hair with color for both
	eq->unknown018 = 3; // beard
	eq->unknown019 = 1; // beard
	eq->unknown020 = 0; ////testing - This moves the spawn into the ground partially?
	//memset(eq->unknown022, 0x01, sizeof(eq->unknown022)); //testing
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




DECODE(OP_ItemLinkClick) {
	DECODE_LENGTH_EXACT(structs::ItemViewRequest_Struct);
	SETUP_DIRECT_DECODE(ItemViewRequest_Struct, structs::ItemViewRequest_Struct);
	MEMSET_IN(ItemViewRequest_Struct);
	
	IN(item_id);
	IN(link_hash);
	
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
	//IN(level);
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
	IN(start_zone);
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



int32 NextItemInstSerialNumber = 1;
int32 MaxInstances = 2000000000;

static inline sint32 GetNextItemInstSerialNumber() {

	if(NextItemInstSerialNumber >= MaxInstances)
		NextItemInstSerialNumber = 1;
	else
		NextItemInstSerialNumber++;

	return NextItemInstSerialNumber;
}



char *SerializeItem(const ItemInst *inst, sint16 slot_id, uint32 *length, uint8 depth) {
	char *serialization = NULL;
	char *instance = NULL;
	const char *protection=(const char *)"\\\\\\\\\\";
	char *sub_items[10] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
	bool stackable=inst->IsStackable();
	uint32 merchant_slot=inst->GetMerchantSlot();
	sint16 charges=inst->GetCharges();
	const Item_Struct *item=inst->GetItem();
	int i;
	uint32 sub_length;
	uint32 stack = stackable ? charges : 1;
	uint32 zero = 0;
	uint32 price = inst->GetPrice();
	uint32 slot = (merchant_slot==0) ? slot_id : merchant_slot;
	uint32 merchcount = (merchant_slot==0) ? 1 : inst->GetMerchantCount();
	uint32 serialnumber = GetNextItemInstSerialNumber();
	uint32 instnodrop = inst->IsInstNoDrop() ? 1 : 0;
	uint32 typepotion = (stackable ? ((inst->GetItem()->ItemType == ItemTypePotion) ? 1 : 0) : charges);

	
	//not sure how these truely shifted, as this dosent seem right.
   MakeAnyLenString(&instance,
	  //"%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|",
      "%i%i%i%i%i%i%i%i%i%i%i%i%i%i%i",
      stack,
      zero,
      price,
	  slot,
      merchcount,
      zero,
      serialnumber,
      instnodrop,
      typepotion,
      zero,
      zero,
      zero,
      zero,
      zero,
      zero

   );
   

	//not sure how these truely shifted, as this dosent seem right.
   /*
   MakeAnyLenString(&instance,
	  //"%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|",
      "%i%i%i%i%i%i%i%i%i%i%i%i%i%i%i",
      stackable ? charges : 1, //was 0
      0,
      inst->GetPrice(),
	  (merchant_slot==0) ? slot_id : merchant_slot,
      (merchant_slot==0) ? 1 : inst->GetMerchantCount(),
      0,
      GetNextItemInstSerialNumber(), //merchant_slot,   //instance ID, bullshit for now
      inst->IsInstNoDrop() ? 1 : 0,      //not sure where this field is
      (stackable ? ((inst->GetItem()->ItemType == ItemTypePotion) ? 1 : 0) : charges),
      0,
      0,
      0,
      0,
      0,
      0

   ); */

	for(i=0;i<10;i++) {
		ItemInst *sub=inst->GetItem(i);
		if (sub) {
			sub_items[i]=SerializeItem(sub,0,&sub_length,depth+1);
		}
	}

	
	*length=MakeAnyLenString(&serialization,
		"%.*s%s"	// For leading quotes (and protection) if a subitem;
		"%s"		// Instance data
		"%.*s\""	// Quotes (and protection, if needed) around static data
		"%i"		// item->ItemClass so we can do |%s instead of %s|
#define I(field) "%i"
#define C(field) "%s"
#define S(field) "%s"
#define F(field) "%f"
#include "SoF_itemfields.h"
		"%.*s\""	// Quotes (and protection, if needed) around static data
		"%s%s%s%s%s%s%s%s%s%s"	// Sub items
		"%.*s%s"	// For trailing quotes (and protection) if a subitem;
		,depth ? depth-1 : 0,protection,(depth) ? "\"" : ""
		,instance
		,depth,protection
		,item->ItemClass
#define I(field) ,item->field
#define C(field) ,field
#define S(field) ,item->field
#define F(field) ,item->field
#include "SoF_itemfields.h"
		,depth,protection
		,sub_items[0] ? sub_items[0] : ""
		,sub_items[1] ? sub_items[1] : ""
		,sub_items[2] ? sub_items[2] : ""
		,sub_items[3] ? sub_items[3] : ""
		,sub_items[4] ? sub_items[4] : ""
		,sub_items[5] ? sub_items[5] : ""
		,sub_items[6] ? sub_items[6] : ""
		,sub_items[7] ? sub_items[7] : ""
		,sub_items[8] ? sub_items[8] : ""
		,sub_items[9] ? sub_items[9] : ""
		,(depth) ? depth-1 : 0,protection,(depth) ? "\"" : ""
	);

	for(i=0;i<10;i++) {
		if (sub_items[i])
			safe_delete_array(sub_items[i]);
	}

	safe_delete_array(instance);

printf("ITEM: \n%s\n", serialization);
	
	return serialization;
}

} //end namespace SoF




