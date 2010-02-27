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
#include "spdat.h"
#include "masterentity.h"
#include "../common/packet_dump.h"
#include "../common/moremath.h"
#include "../common/Item.h"
#include "worldserver.h"
#include "../common/skills.h"
#include "../common/bodytypes.h"
#include "../common/classes.h"
#include "../common/rulesys.h"
#include "spells.h"
#include "buff.h"
#include <math.h>
#include <assert.h>
#include <sstream>
#include <algorithm>
#ifndef WIN32
#include <stdlib.h>
#include "../common/unix.h"
#endif
#ifdef _GOTFRAGS
	#include "../common/packet_dump_file.h"
#endif

#include "StringIDs.h"

#ifdef EMBPERL
#include "embparser.h"
#endif

extern Zone* zone;
extern volatile bool ZoneLoaded;
extern bool spells_loaded;
extern WorldServer worldserver;
#if !defined(NEW_LoadSPDat) && !defined(DB_LoadSPDat)
	extern SPDat_Spell_Struct spells[SPDAT_RECORDS];
#endif


uint16 Mob::GetSpecializeSkillValue(const Spell *spell_to_cast) const {
	switch(spell_to_cast->GetSpell().skill) {
	case ABJURE:
		return(GetSkill(SPECIALIZE_ABJURE));
	case ALTERATION:
		return(GetSkill(SPECIALIZE_ALTERATION));
	case CONJURATION:
		return(GetSkill(SPECIALIZE_CONJURATION));
	case DIVINATION:
		return(GetSkill(SPECIALIZE_DIVINATION));
	case EVOCATION:
		return(GetSkill(SPECIALIZE_EVOCATION));
	default:
		//wtf...
		break;
	}
	return(0);
}

void Client::CheckSpecializeIncrease(int16 spell_id) {
	switch(spells[spell_id].skill) {
	case ABJURE:
		CheckIncreaseSkill(SPECIALIZE_ABJURE, NULL);
		break;
	case ALTERATION:
		CheckIncreaseSkill(SPECIALIZE_ALTERATION, NULL);
		break;
	case CONJURATION:
		CheckIncreaseSkill(SPECIALIZE_CONJURATION, NULL);
		break;
	case DIVINATION:
		CheckIncreaseSkill(SPECIALIZE_DIVINATION, NULL);
		break;
	case EVOCATION:
		CheckIncreaseSkill(SPECIALIZE_EVOCATION, NULL);
		break;
	default:
		//wtf...
		break;
	}
}

void Client::CheckSongSkillIncrease(int16 spell_id){
	switch(spells[spell_id].skill)
	{
	case SINGING:
		CheckIncreaseSkill(SINGING, NULL, -15);
		break;
	case PERCUSSION_INSTRUMENTS:
		if(this->itembonuses.percussionMod > 0) {
			if(GetRawSkill(PERCUSSION_INSTRUMENTS) > 0)	// no skill increases if not trained in the instrument
				CheckIncreaseSkill(PERCUSSION_INSTRUMENTS, NULL, -15);
			else
				Message_StringID(13,NO_INSTRUMENT_SKILL);	// tell the client that they need instrument training
		}
		else
			CheckIncreaseSkill(SINGING, NULL, -15);
		break;
	case STRINGED_INSTRUMENTS:
		if(this->itembonuses.stringedMod > 0) {
			if(GetRawSkill(STRINGED_INSTRUMENTS) > 0)
				CheckIncreaseSkill(STRINGED_INSTRUMENTS, NULL, -15);
			else
				Message_StringID(13,NO_INSTRUMENT_SKILL);
		}
		else
			CheckIncreaseSkill(SINGING, NULL, -15);
		break;
	case WIND_INSTRUMENTS:
		if(this->itembonuses.windMod > 0) {
			if(GetRawSkill(WIND_INSTRUMENTS) > 0)
				CheckIncreaseSkill(WIND_INSTRUMENTS, NULL, -15);
			else
				Message_StringID(13,NO_INSTRUMENT_SKILL);
		}
		else
			CheckIncreaseSkill(SINGING, NULL, -15);
		break;
	case BRASS_INSTRUMENTS:
		if(this->itembonuses.brassMod > 0) {
			if(GetRawSkill(BRASS_INSTRUMENTS) > 0)
				CheckIncreaseSkill(BRASS_INSTRUMENTS, NULL, -15);
			else
				Message_StringID(13,NO_INSTRUMENT_SKILL);
		}
		else
			CheckIncreaseSkill(SINGING, NULL, -15);
		break;
	default:
		break;
	}
}

/*
solar: returns true if spell is successful, false if it fizzled.
only works for clients, npcs shouldn't be fizzling..
neotokyo: new algorithm thats closer to live eq (i hope)
kathgar TODO: Add aa skills, item mods, reduced the chance to fizzle
*/
bool Mob::CheckFizzle(const Spell *spell_to_cast)
{
	return(true);
}

bool Client::CheckFizzle(const Spell *spell_to_cast)
{
	// GMs don't fizzle
	if (GetGM()) return(true);

	int no_fizzle_level = 0;
	if (GetAA(aaMasteryofthePast) || GetAA(aaMasteryofthePast2)) {
		switch (GetAA(aaMasteryofthePast) + GetAA(aaMasteryofthePast2)) {
			case 1:
				no_fizzle_level = 53;
				break;
			case 2:
				no_fizzle_level = 55;
				break;
			case 3:
				no_fizzle_level = 57;
				break;
		}
	} else {
		switch (GetAA(aaSpellCastingExpertise)) {
			case 1:
				no_fizzle_level = 19;
				break;
			case 2:
				no_fizzle_level = 34;
				break;
			case 3:
				no_fizzle_level = 51;
				break;
		}
	}
	if (spell_to_cast->GetSpell().classes[GetClass()-1] <= no_fizzle_level)
		return true;

	//is there any sort of focus that affects fizzling?


	// neotokyo: this is my try to get something going
	int par_skill;
	int act_skill;

	par_skill = spell_to_cast->GetSpell().classes[GetClass()-1] * 5 - 10;//IIRC even if you are lagging behind the skill levels you don't fizzle much
	/*par_skill = spells[spell_id].classes[GetClass()-1] * 5 + 5;*/
	if (par_skill > 235)
		par_skill = 235;

	par_skill += spell_to_cast->GetSpell().classes[GetClass()-1]; // maximum of 270 for level 65 spell

	act_skill = GetSkill(spell_to_cast->GetSpell().skill);
	act_skill += GetLevel(); // maximum of whatever the client can cheat

	//FatherNitwit: spell specialization
	float specialize = GetSpecializeSkillValue(spell_to_cast);
		//VERY rough success formula, needs research
	if(specialize > 0) {
		switch(GetAA(aaSpellCastingMastery)){
		case 1:
			specialize = specialize * 1.05;
			break;
		case 2:
			specialize = specialize * 1.15;
			break;
		case 3:
			specialize = specialize * 1.3;
			break;
		}
		if(((specialize/6.0f) + 15.0f) < MakeRandomFloat(0, 100)) {
			specialize *= SPECIALIZE_FIZZLE / 200;
		} else {
			specialize = 0.0f;
		}
	}

	// == 0 --> on par
	// > 0  --> skill is lower, higher chance of fizzle
	// < 0  --> skill is better, lower chance of fizzle
	// the max that diff can be is +- 235
	float diff = par_skill + spell_to_cast->GetSpell().basediff - act_skill;

	// if you have high int/wis you fizzle less, you fizzle more if you are stupid
	if (GetCasterClass() == 'W')
		diff -= (GetWIS() - 125) / 20.0;
	if (GetCasterClass() == 'I')
		diff -= (GetINT() - 125) / 20.0;

	// base fizzlechance is lets say 5%, we can make it lower for AA skills or whatever
	float basefizzle = 10;
	float fizzlechance = basefizzle - specialize + diff / 5.0;

	// always at least 5% chance to fail or succeed
	fizzlechance = fizzlechance < 5 ? 5 : (fizzlechance > 95 ? 95 : fizzlechance);
	float fizzle_roll = MakeRandomFloat(0, 100);

	mlog(SPELLS__CASTING, "Check Fizzle %s  spell %d  fizzlechance: %0.2f%%   diff: %0.2f  roll: %0.2f", GetName(), spell_to_cast->GetSpellID(), fizzlechance, diff, fizzle_roll);

	if(fizzle_roll > fizzlechance)
		return(true);
	return(false);
}

void Mob::ZeroCastingVars()
{
	// zero out the state keeping vars
	attacked_count = 0;
	casting_spell = NULL;
	casting_spell_finished = NULL;
}

void Mob::ZeroAndFreeCastingVars(bool all)
{
	// zero out the state keeping vars
	attacked_count = 0;
	safe_delete(casting_spell);
	if(all)
	{
		safe_delete(casting_spell_finished);
	}
}

void Mob::ZeroAndFreeSong()
{
	safe_delete(bard_song);
	bardsong_timer.Disable();
}

void Mob::InterruptSpell(int16 spellid)
{
	if (spellid == SPELL_UNKNOWN)
		spellid = casting_spell ? casting_spell->GetSpellID() : 0;

	InterruptSpell(0, 0x121, spellid);
}

// solar: color not used right now
void Mob::InterruptSpell(int16 message, int16 color, int16 spellid)
{
	EQApplicationPacket *outapp;
	int16 message_other;

	if (spellid == SPELL_UNKNOWN)
		spellid = casting_spell ? casting_spell->GetSpellID() : 0;

	if(casting_spell && IsNPC())
	{
		CastToNPC()->AI_Event_SpellCastFinished(false, casting_spell->GetSpellSlot());
	}

	mlog(SPELLS__CASTING, "Spell %d has been interrupted.", spellid);

	if(!spellid)
	{
		ZeroAndFreeCastingVars(false);
		return;
	}

	/*if (bard_song || (casting_spell ? casting_spell->IsBardSong() : 0))
	{
		_StopSong();
	}*/

	if(!message)
	{
		message = (casting_spell ? casting_spell->IsBardSong() : 0) ? SONG_ENDS_ABRUPTLY : INTERRUPT_SPELL;
	}

	// clients need some packets
	if (IsClient())
	{
		// the interrupt message
		outapp = new EQApplicationPacket(OP_InterruptCast, sizeof(InterruptCast_Struct));
		InterruptCast_Struct* ic = (InterruptCast_Struct*) outapp->pBuffer;
		ic->messageid = message;
		ic->spawnid = GetID();
		outapp->priority = 5;
		CastToClient()->QueuePacket(outapp);
		safe_delete(outapp);

		SendSpellBarEnable(spellid);
	}

	// notify people in the area

	// first figure out what message others should get
	switch(message)
	{
		case SONG_ENDS:
			message_other = SONG_ENDS_OTHER;
			break;
		case SONG_ENDS_ABRUPTLY:
			message_other = SONG_ENDS_ABRUPTLY_OTHER;
			break;
		case MISS_NOTE:
			message_other = MISSED_NOTE_OTHER;
			break;
		case SPELL_FIZZLE:
			message_other = SPELL_FIZZLE_OTHER;
			break;
		default:
			message_other = INTERRUPT_SPELL_OTHER;
	}

	// this is the actual message, it works the same as a formatted message
	outapp = new EQApplicationPacket(OP_InterruptCast, sizeof(InterruptCast_Struct) + strlen(GetCleanName()) + 1);
	InterruptCast_Struct* ic = (InterruptCast_Struct*) outapp->pBuffer;
	ic->messageid = message_other;
	ic->spawnid = GetID();
	strcpy(ic->message, GetCleanName());
	entity_list.QueueCloseClients(this, outapp, true, 200, 0, true, IsClient() ? FILTER_PCSPELLS : FILTER_NPCSPELLS);
	safe_delete(outapp);
	ZeroAndFreeCastingVars(false);
}

bool Mob::DetermineSpellTargets(Spell *spell_to_cast, Mob *&spell_target, Mob *&ae_center, CastAction_type &CastAction)
{
	/*
	The basic types of spells:

	Single target - some might be undead only, self only, etc, but these
	all affect the target of the caster.

	AE around caster - these affect entities close to the caster, and have
	no target.

	AE around target - these have a target, and affect the target as well as
	entities close to the target.

	AE on location - this is a tricky one that is cast on a mob target but
	has a special AE duration that keeps it recasting every 2.5 sec on the
	same location.  These work the same as AE around target spells, except
	the target is a special beacon that's created when the spell is cast

	Group - the caster is always affected, but there's more
		targetgroupbuffs on - these affect the target and the target's group.
		targetgroupbuffs off - no target, affects the caster's group.

	Group Teleport - the caster plus his group are affected.  these cannot
	be targeted.

	I think the string ID SPELL_NEED_TAR is wrong, it dosent seem to show up.
	*/

	// during this switch, this variable gets set to one of these things
	// and that causes the spell to be executed differently

	bodyType target_bt = BT_Humanoid;
	SpellTargetType targetType = spell_to_cast->GetSpell().targettype;
	bodyType mob_body = spell_target ? spell_target->GetBodyType() : BT_Humanoid;

	if(spell_to_cast->IsPlayerIllusionSpell()
		&& spell_target != NULL // null ptr crash safeguard
		&& !spell_target->IsNPC() // still self only if NPC targetted
		&& IsClient()
		&& IsGrouped() // still self only if not grouped
		&& CastToClient()->CheckAAEffect(aaEffectProjectIllusion)){
			mlog(AA__MESSAGE, "Project Illusion overwrote target caster: %s spell id: %d was ON", GetName(), spell_to_cast->GetSpellID());
			targetType = ST_GroupClient;
	}

	switch (targetType)
	{
		//single target spells
		case ST_Self:
		{
			spell_target = this;
			CastAction = SingleTarget;
			break;
		}

		case ST_TargetOptional:
		{
			if(!spell_target)
				spell_target = this;
			CastAction = SingleTarget;
			break;
		}

		// target required for these
		case ST_Undead: {
			if(!spell_target || (
				spell_target->GetBodyType() != BT_SummonedUndead
				&& spell_target->GetBodyType() != BT_Undead
				&& spell_target->GetBodyType() != BT_Vampire
				)
			)
			{
				//invalid target
				mlog(SPELLS__CASTING_ERR, "Spell %d canceled: invalid target of body type %d (undead)", spell_to_cast->GetSpellID(), spell_target->GetBodyType());
				Message_StringID(13,SPELL_NEED_TAR);
				return false;
			}
			CastAction = SingleTarget;
			break;
		}

		case ST_Summoned: {
			int8 body_type = spell_target ? spell_target->GetBodyType() : 0;
			if(!spell_target || (body_type != BT_Summoned && body_type != BT_Summoned2 && body_type != BT_Summoned3))
			{
				//invalid target
				mlog(SPELLS__CASTING_ERR, "Spell %d canceled: invalid target of body type %d (summoned)", spell_to_cast->GetSpellID(), spell_target->GetBodyType());
				Message_StringID(13,SPELL_NEED_TAR);
				return false;
			}
			CastAction = SingleTarget;
			break;
		}

		case ST_SummonedPet:
		{
			int8 body_type = spell_target ? spell_target->GetBodyType() : 0;
			if(!spell_target || (spell_target != GetPet()) ||
			   (body_type != BT_Summoned && body_type != BT_Summoned2 && body_type != BT_Summoned3))
			{
				mlog(SPELLS__CASTING_ERR, "Spell %d canceled: invalid target of body type %d (summoned pet)",
					spell_to_cast->GetSpellID(), body_type);

				Message_StringID(13, SPELL_NEED_TAR);

				return false;
			}
			CastAction = SingleTarget;
			break;
		}
		//single body type target spells...
		//this is a little hackish, but better than duplicating code IMO
		case ST_Plant: if(target_bt == BT_Humanoid) target_bt = BT_Plant;
		case ST_Dragon: if(target_bt == BT_Humanoid) target_bt = BT_Dragon;
		case ST_Giant: if(target_bt == BT_Humanoid) target_bt = BT_Giant;
		case ST_Animal: if(target_bt == BT_Humanoid) target_bt = BT_Animal;
		
		// check for special case body types (Velious dragons/giants)
		if(mob_body == BT_RaidGiant) mob_body = BT_Giant;
		if(mob_body == BT_VeliousDragon) mob_body = BT_Dragon;

		{
			if(!spell_target || mob_body != target_bt)
			{
				//invalid target
				mlog(SPELLS__CASTING_ERR, "Spell %d canceled: invalid target of body type %d (want body Type %d)", spell_to_cast->GetSpellID(), spell_target->GetBodyType(), target_bt);
				if(!spell_target)
					Message_StringID(13,SPELL_NEED_TAR);
				else
					Message_StringID(13,CANNOT_AFFECT_NPC);
				return false;
			}
			CastAction = SingleTarget;
			break;
		}

		case ST_Tap:
		case ST_LDoNChest_Cursed:
		case ST_Target: {
			if(spell_to_cast->IsLDoNObjectSpell())
			{
				if(!spell_target)
				{
					mlog(SPELLS__CASTING_ERR, "Spell %d canceled: invalid target (ldon object)", spell_to_cast->GetSpellID());
					Message_StringID(13,SPELL_NEED_TAR);
					return false;
				}
				else
				{
					if(!spell_target->IsNPC())
					{
						mlog(SPELLS__CASTING_ERR, "Spell %d canceled: invalid target (normal)", spell_to_cast->GetSpellID());
						Message_StringID(13,SPELL_NEED_TAR);
						return false;
					}

					if(spell_target->GetClass() != LDON_TREASURE)
					{
						mlog(SPELLS__CASTING_ERR, "Spell %d canceled: invalid target (normal)", spell_to_cast->GetSpellID());
						Message_StringID(13,SPELL_NEED_TAR);
						return false;
					}
				}
			}

			if(!spell_target)
			{
				mlog(SPELLS__CASTING_ERR, "Spell %d canceled: invalid target (normal)", spell_to_cast->GetSpellID());
				Message_StringID(13,SPELL_NEED_TAR);
				return false;	// can't cast these unless we have a target
			}
			CastAction = SingleTarget;
			break;
		}

		case ST_Corpse:
		{
			if(!spell_target || !spell_target->IsPlayerCorpse())
			{
				mlog(SPELLS__CASTING_ERR, "Spell %d canceled: invalid target (corpse)", spell_to_cast->GetSpellID());
				int32 message = ONLY_ON_CORPSES;
				if(!spell_target) message = SPELL_NEED_TAR;
				else if(!spell_target->IsCorpse()) message = ONLY_ON_CORPSES;
				else if(!spell_target->IsPlayerCorpse()) message = CORPSE_NOT_VALID;
				Message_StringID(13, message);
				return false;
			}
			CastAction = SingleTarget;
			break;
		}
		case ST_Pet:
		{
			spell_target = GetPet();
			if(!spell_target)
			{
				mlog(SPELLS__CASTING_ERR, "Spell %d canceled: invalid target (no pet)", spell_to_cast->GetSpellID());
				Message_StringID(13,NO_PET);
				return false;	// can't cast these unless we have a target
			}
			CastAction = SingleTarget;
			break;
		}

		// AE spells
		case ST_AEBard:
		case ST_AECaster:
		{
			spell_target = NULL;
			ae_center = this;
			CastAction = AECaster;
			break;
		}

		case ST_UndeadAE:  //should only affect undead...
		case ST_AETarget:
		{
			if(!spell_target)
			{
				mlog(SPELLS__CASTING_ERR, "Spell %d canceled: invalid target (AOE)", spell_to_cast->GetSpellID());
				Message_StringID(13,SPELL_NEED_TAR);
				return false;
			}
			ae_center = spell_target;
			CastAction = AETarget;
			break;
		}

		// Group spells
		case ST_GroupTeleport:
		case ST_Group:
		{
			if(IsClient() && CastToClient()->TGB() && spell_to_cast->IsTGBCompatibleSpell()) {
				if(!target)	//target the group of our target, if we have a target, else our own
					spell_target = this;
				else
					spell_target = target;
			} else {
				spell_target = this;
			}
			CastAction = GroupSpell;
			break;
		}
		case ST_GroupClient:
		{
			if(!spell_target){
				mlog(SPELLS__CASTING_ERR, "Spell %d canceled: invalid target (Group: Single Target Client Only)", spell_to_cast->GetSpellID());
				Message_StringID(13,SPELL_NEED_TAR);
				return false;
			}
			if(spell_target != this){
				if(IsClient() && IsGrouped()){
					Group *g = entity_list.GetGroupByMob(this);
					if(g && g->IsGroupMember(spell_target) && spell_target != this){
						CastAction = SingleTarget;
					}
					else{
						mlog(SPELLS__CASTING_ERR, "Spell %d canceled: Attempted to cast a Single Target Group spell on a member not in the group.", spell_to_cast->GetSpellID());
						Message_StringID(13, TARGET_GROUP_MEMBER);
						return false;
					}
				}
				else{
					mlog(SPELLS__CASTING_ERR, "Spell %d canceled: Attempted to cast a Single Target Group spell on a member not in the group.", spell_to_cast->GetSpellID());
					Message_StringID(13, TARGET_GROUP_MEMBER);
					return false;
				}
			}
			else
				CastAction = SingleTarget;
			break;
		}

		default:
		{
			mlog(SPELLS__CASTING_ERR, "I dont know Target Type: %d   Spell: (%d) %s", spell_to_cast->GetSpell().targettype, spell_to_cast->GetSpellID(), spell_to_cast->GetSpell().name);
			Message(0, "I dont know Target Type: %d   Spell: (%d) %s", spell_to_cast->GetSpell().targettype, spell_to_cast->GetSpellID(), spell_to_cast->GetSpell().name);
			CastAction = CastActUnknown;
			break;
		}
	}
	return(true);
}

int Mob::SendActionSpellPacket(Spell *spell_to_cast, Mob *spell_target, int caster_level)
{
	EQApplicationPacket *action_packet = new EQApplicationPacket(OP_Action, sizeof(Action_Struct));
	Action_Struct* action = (Action_Struct*) action_packet->pBuffer;
	uint32 sequence = 0;

	if(IsClient() && CastToClient()->GMHideMe())
	{
		action->source = spell_target->GetID();
	}
	else
	{
		action->source = GetID();
	}

	if(spell_to_cast->IsEffectInSpell(SE_BindSight))
	{
		action->target = GetID();
	}
	else
	{
		action->target = spell_target->GetID();
	}

	action->level = caster_level;
	action->type = 231;
	action->spell = spell_to_cast->GetSpellID();
	action->sequence = GetHeading() * 2;
	action->instrument_mod = GetInstrumentMod(spell_to_cast);
	action->buff_unknown = 0;
	sequence = action->sequence;

	if(spell_target->IsClient())
	{
		spell_target->CastToClient()->QueuePacket(action_packet);
	}

	if(IsClient())
	{
		CastToClient()->QueuePacket(action_packet);
	}

	entity_list.QueueCloseClients(spell_target, action_packet, true, 200, this, true, spell_target->IsClient() ? FILTER_PCSPELLS : FILTER_NPCSPELLS);
	safe_delete(action_packet);
	return sequence;
}

void Mob::SendActionSpellPacket(Spell *spell_to_cast, Mob *spell_target, uint32 sequence, int caster_level, int mode)
{
	EQApplicationPacket *action_packet = new EQApplicationPacket(OP_Action, sizeof(Action_Struct));
	Action_Struct* action = (Action_Struct*) action_packet->pBuffer;

	if(IsClient() && CastToClient()->GMHideMe())
	{
		action->source = spell_target->GetID();
	}
	else
	{
		action->source = GetID();
	}

	if(spell_to_cast->IsEffectInSpell(SE_BindSight))
	{
		action->target = GetID();
	}
	else
	{
		action->target = spell_target->GetID();
	}

	action->level = caster_level;
	action->type = 231;
	action->spell = spell_to_cast->GetSpellID();
	action->sequence = sequence;
	action->instrument_mod = GetInstrumentMod(spell_to_cast);
	action->buff_unknown = mode;

	if(spell_target->IsClient())
	{
		spell_target->CastToClient()->QueuePacket(action_packet);
	}

	if(IsClient())
	{
		CastToClient()->QueuePacket(action_packet);
	}

	//entity_list.QueueCloseClients(spell_target, action_packet, true, 200, this, true, spell_target->IsClient() ? FILTER_PCSPELLS : FILTER_NPCSPELLS);
	safe_delete(action_packet);
}

void Mob::SendCombatDamageSpellPacket(Spell *spell_to_cast, Mob *spell_target, int sequence)
{
	EQApplicationPacket *message_packet;

	message_packet = new EQApplicationPacket(OP_Damage, sizeof(CombatDamage_Struct));
	CombatDamage_Struct *cd = (CombatDamage_Struct *)message_packet->pBuffer;
	cd->type = 231;
	cd->spellid = spell_to_cast->GetSpellID();
	cd->sequence = sequence;
	cd->damage = 0;

	if(IsClient() && CastToClient()->GMHideMe())
	{
		cd->source = spell_target->GetID();
	}
	else
	{
		cd->source = GetID();
	}

	if(spell_to_cast->IsEffectInSpell(SE_BindSight))
	{
		cd->target = GetID();
	}
	else
	{
		cd->target = spell_target->GetID();
	}

	if(!spell_to_cast->IsEffectInSpell(SE_BindAffinity))
	{
		entity_list.QueueCloseClients(spell_target, message_packet, false, 200, 0, true, spell_target->IsClient() ? FILTER_PCSPELLS : FILTER_NPCSPELLS);
	}

	safe_delete(message_packet);
}

float Mob::DoSpellOnTargetResistCheck(Spell *spell_to_cast, Mob *spell_target)
{
	float spell_effectiveness = 0;
	if(spell_to_cast->IsResistableSpell())
	{
		spell_effectiveness = spell_target->ResistSpell(spell_to_cast->GetSpell().resisttype, spell_to_cast, this);
		if(spell_effectiveness < 100)
		{
			if(spell_effectiveness == 0 || !spell_to_cast->IsPartialCapableSpell())
			{
				mlog(SPELLS__RESISTS, "Spell %d was completely resisted by %s", spell_to_cast->GetSpellID(), spell_target->GetName());
				Message_StringID(MT_Shout, TARGET_RESISTED, spell_to_cast->GetSpell().name);
				spell_target->Message_StringID(MT_Shout, YOU_RESIST, spell_to_cast->GetSpell().name);

				if(spell_target->IsAIControlled())
				{
					sint32 aggro = CheckAggroAmount(spell_to_cast);
					if(aggro > 0)
					{
						if(!spell_to_cast->IsHarmonySpell())
						{
							spell_target->AddToHateList(this, aggro);
						}
						else
							if(!PassCharismaCheck(this, spell_target, spell_to_cast))
								spell_target->AddToHateList(this, aggro);
					}
					else
					{
						sint32 newhate = spell_target->GetHateAmount(this) + aggro;
						if(newhate < 1)
						{
							spell_target->SetHate(this, 1);
						}
						else
						{
							spell_target->SetHate(this, newhate);
						}
					}
				}
				return 0;
			}
		}
	}
	else
	{
		spell_effectiveness = 100;
	}

	return spell_effectiveness;
}

void Mob::DoSpellOnTargetRecourse(Spell *spell_on_target, Mob *spell_target)
{
	int recourse_spell_id = spell_on_target->GetSpell().RecourseLink;
	if(!recourse_spell_id)
	{
		return;
	}

	Spell *recourse_spell = new Spell(recourse_spell_id, this, spell_target);
	if(recourse_spell->GetSpell().targettype == ST_Group || recourse_spell->GetSpell().targettype == ST_GroupTeleport)
	{
		if(IsGrouped())
		{
			Group *g = entity_list.GetGroupByMob(this);
			g->CastGroupSpell(this, recourse_spell);
		}
		else if(IsRaidGrouped() && IsClient())
		{
			Raid *r = entity_list.GetRaidByClient(CastToClient());
			int32 gid = 0xFFFFFFFF;
			gid = r->GetGroup(GetName());
			if(gid < 12)
			{
				r->CastGroupSpell(this, recourse_spell, gid);
			}
			else
			{
				SpellOnTarget(recourse_spell, this);
#ifdef GROUP_BUFF_PETS
				if (GetPet())
					SpellOnTarget(recourse_spell, GetPet());
#endif
			}
		}
		else if(HasOwner())
		{
			Mob *owner = GetOwner();
			if(owner->IsGrouped())
			{
				Group *g = entity_list.GetGroupByMob(owner);
				g->CastGroupSpell(this, recourse_spell);
			}
			else if(owner->IsRaidGrouped() && owner->IsClient())
			{
				Raid *r = entity_list.GetRaidByClient(owner->CastToClient());
				int32 gid = r->GetGroup(owner->GetName());
				if(gid < 12)
				{
					r->CastGroupSpell(this, recourse_spell, gid);
				}
				else
				{
					SpellOnTarget(recourse_spell, owner);
					SpellOnTarget(recourse_spell, this);
				}
			}
			else
			{
				SpellOnTarget(recourse_spell, owner);
				SpellOnTarget(recourse_spell, this);
			}
		}
		else
		{
			SpellOnTarget(recourse_spell, this);
#ifdef GROUP_BUFF_PETS
			if (GetPet())
				SpellOnTarget(recourse_spell, GetPet());
#endif
		}
	}
	else
	{
		SpellOnTarget(recourse_spell, this);
	}
}

void Mob::SendKnockBackPacket(Mob *caster, int push_up, int push_back)
{
	if(IsClient())
	{
		CastToClient()->SetKnockBackExemption(true);
		EQApplicationPacket* outapp_push = new EQApplicationPacket(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
		PlayerPositionUpdateServer_Struct* spu = (PlayerPositionUpdateServer_Struct*)outapp_push->pBuffer;

		double look_heading = caster->CalculateHeadingToTarget(GetX(), GetY());
		look_heading /= 256;
		look_heading *= 360;
		if(look_heading > 360)
			look_heading -= 360;

		//x and y are crossed mkay
		double new_x = push_back * sin(double(look_heading * 3.141592 / 180.0));
		double new_y = push_back * cos(double(look_heading * 3.141592 / 180.0));

		spu->spawn_id	= GetID();
		spu->x_pos		= FloatToEQ19(GetX());
		spu->y_pos		= FloatToEQ19(GetY());
		spu->z_pos		= FloatToEQ19(GetZ());
		spu->delta_x	= NewFloatToEQ13(new_x);
		spu->delta_y	= NewFloatToEQ13(new_y);
		spu->delta_z	= NewFloatToEQ13(push_up);
		spu->heading	= FloatToEQ19(GetHeading());
		spu->padding0002	=0;
		spu->padding0006	=7;
		spu->padding0014	=0x7f;
		spu->padding0018	=0x5df27;
		spu->animation = 0;
		spu->delta_heading = NewFloatToEQ13(0);
		outapp_push->priority = 6;
		entity_list.QueueClients(this, outapp_push, true);
		CastToClient()->FastQueuePacket(&outapp_push);
	}
}

void Corpse::CastRezz(int16 spellid, Mob* Caster){

	_log(SPELLS__REZ, "Corpse::CastRezz spellid %i, Rezzed() is %i, rezzexp is %i", spellid,Rezzed(),rezzexp);

	if(Rezzed()){
		if(Caster && Caster->IsClient())
			Caster->Message(13,"This character has already been resurrected.");
		return;
	}

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_RezzRequest, sizeof(Resurrect_Struct));
	Resurrect_Struct* rezz = (Resurrect_Struct*) outapp->pBuffer;
      // Why are we truncating these names to 30 characters ?
	memcpy(rezz->your_name,this->orgname,30);
	memcpy(rezz->corpse_name,this->name,30);
	memcpy(rezz->rezzer_name,Caster->GetName(),30);
	rezz->zone_id = zone->GetZoneID();
	rezz->instance_id = zone->GetInstanceID();
	rezz->spellid = spellid;
	rezz->x = this->x_pos;
	rezz->y = this->y_pos;
	rezz->z = this->z_pos;
	rezz->unknown000 = 0x00000000;
	rezz->unknown020 = 0x00000000;
	rezz->unknown088 = 0x00000000;
	// We send this to world, because it needs to go to the player who may not be in this zone.
	worldserver.RezzPlayer(outapp, rezzexp, OP_RezzRequest);
	_pkt(SPELLS__REZ, outapp);
	safe_delete(outapp);
}

// checks if 'this' can be affected by spell_id from caster
// returns true if the spell should fail, false otherwise
bool Mob::IsImmuneToSpell(Spell *spell_to_cast, Mob *caster)
{
	_ZP(Mob_IsImmuneToSpell);
	int effect_index;

	if(caster == NULL)
		return(false);

	//TODO: this function loops through the effect list for
	//this spell like 10 times, this could easily be consolidated
	//into one loop through with a switch statement.

	mlog(SPELLS__RESISTS, "Checking to see if we are immune to spell %d cast by %s", spell_to_cast->GetSpellID(), caster->GetName());

	if(spell_to_cast->IsEffectInSpell(SE_Mez))
	{
		if(SpecAttacks[UNMEZABLE]) {
			mlog(SPELLS__RESISTS, "We are immune to Mez spells.");
			caster->Message_StringID(MT_Shout, CANNOT_MEZ);
			AddToHateList(caster);
			return true;
		}

		// check max level for spell
		effect_index = spell_to_cast->GetSpellEffectIndex(SE_Mez);
		assert(effect_index >= 0);
		if(GetLevel() > spell_to_cast->GetSpell().max[effect_index])
		{
			mlog(SPELLS__RESISTS, "Our level (%d) is higher than the limit of this Mez spell (%d)", GetLevel(), spell_to_cast->GetSpell().max[effect_index]);
			caster->Message_StringID(MT_Shout, CANNOT_MEZ_WITH_SPELL);
			return true;
		}
	}

	// slow and haste spells
	if(SpecAttacks[UNSLOWABLE] && spell_to_cast->IsEffectInSpell(SE_AttackSpeed))
	{
		mlog(SPELLS__RESISTS, "We are immune to Slow spells.");
		caster->Message_StringID(MT_Shout, IMMUNE_ATKSPEED);
		return true;
	}

	// client vs client fear
	if(spell_to_cast->IsEffectInSpell(SE_Fear))
	{
		effect_index = spell_to_cast->GetSpellEffectIndex(SE_Fear);
		if(SpecAttacks[UNFEARABLE]) {
			mlog(SPELLS__RESISTS, "We are immune to Fear spells.");
			caster->Message_StringID(MT_Shout, IMMUNE_FEAR);
			return true;
		} else if(IsClient() && caster->IsClient() && (caster->CastToClient()->GetGM() == false))
		{
			mlog(SPELLS__RESISTS, "Clients cannot fear eachother!");
			caster->Message_StringID(MT_Shout, IMMUNE_FEAR);
			return true;
		}
		else if(GetLevel() > spell_to_cast->GetSpell().max[effect_index] && spell_to_cast->GetSpell().max[effect_index] != 0)
		{
			mlog(SPELLS__RESISTS, "Level is %d, cannot be feared by this spell.", GetLevel());
			caster->Message_StringID(MT_Shout, SPELL_NO_EFFECT);
			return true;
		}
	}

	if(spell_to_cast->IsEffectInSpell(SE_Charm))
	{
		if(SpecAttacks[UNCHARMABLE])
		{
			mlog(SPELLS__RESISTS, "We are immune to Charm spells.");
			caster->Message_StringID(MT_Shout, CANNOT_CHARM);
			return true;
		}

		if(this == caster)
		{
			mlog(SPELLS__RESISTS, "You are immune to your own charms.");
			caster->Message(MT_Shout, "You cannot charm yourself.");
			return true;
		}

		//let npcs cast whatever charm on anyone
		if(!caster->IsNPC())
		{
			// check level limit of charm spell
			effect_index = spell_to_cast->GetSpellEffectIndex(SE_Charm);
			assert(effect_index >= 0);
			if(GetLevel() > spell_to_cast->GetSpell().max[effect_index] && spell_to_cast->GetSpell().max[effect_index] != 0)
			{
				mlog(SPELLS__RESISTS, "Our level (%d) is higher than the limit of this Charm spell (%d)", GetLevel(), spell_to_cast->GetSpell().max[effect_index]);
				caster->Message_StringID(MT_Shout, CANNOT_CHARM_YET);
				return true;
			}
		}
	}

	if
	(
		spell_to_cast->IsEffectInSpell(SE_Root) ||
		spell_to_cast->IsEffectInSpell(SE_MovementSpeed)
	)
	{
		if(SpecAttacks[UNSNAREABLE]) {
			mlog(SPELLS__RESISTS, "We are immune to Snare spells.");
			caster->Message_StringID(MT_Shout, IMMUNE_MOVEMENT);
			return true;
		}
	}

	if(spell_to_cast->IsLifetapSpell())
	{
		if(this == caster)
		{
			mlog(SPELLS__RESISTS, "You cannot lifetap yourself.");
			caster->Message_StringID(MT_Shout, CANT_DRAIN_SELF);
			return true;
		}
	}

	if(spell_to_cast->IsEffectInSpell(SE_Sacrifice))
	{
		if(this == caster)
		{
			mlog(SPELLS__RESISTS, "You cannot sacrifice yourself.");
			caster->Message_StringID(MT_Shout, CANNOT_SAC_SELF);
			return true;
		}
	}

	mlog(SPELLS__RESISTS, "No immunities to spell %d found.", spell_to_cast->GetSpellID());

	return false;
}

//
// solar: spell resists:
// returns an effectiveness index from 0 to 100.  for most spells, 100 means
// it landed, and anything else means it was resisted; however there are some
// spells that can be partially effective, and this value can be used there.
//
float Mob::ResistSpell(int8 resist_type, const Spell *spell_to_cast, Mob *caster)
{
	int caster_level, target_level, resist;
	float roll, fullchance, resistchance;

	//this is checked here instead of in the Immune code so it only applies to detrimental spells
	if(SpecAttacks[IMMUNE_MAGIC]) {
		mlog(SPELLS__RESISTS, "We are immune to magic, so we fully resist the spell %d", spell_to_cast
			? spell_to_cast->GetSpellID() : 0xFFFF);
		return(0);
	}

	if(resist_type == RESIST_NONE) {
		//unresistable...
		mlog(SPELLS__RESISTS, "The spell %d is unresistable (type %d)",
			spell_to_cast ? spell_to_cast->GetSpellID() : 0xFFFF, resist_type);
		return(100);
	}

	if(SpecAttacks[IMMUNE_CASTING_FROM_RANGE])
	{
		if(caster)
		{
			if(!caster->CombatRange(this))
				return(0);
		}
	}

	target_level = GetLevel();

	if(!spell_to_cast) {
		caster_level = caster->GetLevel();
	} else {
		caster_level = caster ? caster->GetCasterLevel() : target_level;
	}

	// if NPC target and more than X levels above caster, it's always resisted
	if(IsNPC() && target_level - caster_level > RuleI(Spells, AutoResistDiff)) {
		mlog(SPELLS__RESISTS, "We are %d levels above the caster, which is higher than the %d level"
			" auto-resist gap. Fully resisting.",  target_level - caster_level, RuleI(Spells, AutoResistDiff));
 		return 0;
	}

	//check for buff/item/aa based fear moditifers
	//still working on this...
	if (spell_to_cast && spell_to_cast->IsEffectInSpell(SE_Fear)) {
		sint16 rchance = 0;
		switch (GetAA(aaFearResistance))
		{
			case 1:
				rchance += 5;
				break;
			case 2:
				rchance += 15;
				break;
			case 3:
				rchance += 25;
				break;
		}
		rchance += itembonuses.ResistFearChance + spellbonuses.ResistFearChance;

		if(GetAA(aaFearless) || (IsClient() && CastToClient()->CheckAAEffect(aaEffectWarcry)))
			rchance = 100;

		//I dont think these should get factored into standard spell resist...
		if(MakeRandomInt(0, 99) < rchance) {
			mlog(SPELLS__RESISTS, "Had a %d chance of resisting the fear spell %d, and succeeded.",
				rchance, spell_to_cast->GetSpellID());
			return(0);
		}
		mlog(SPELLS__RESISTS, "Had a %d chance of resisting the fear spell %d, and failed.",
			rchance, spell_to_cast->GetSpellID());
	}

	switch(resist_type) {
	case RESIST_MAGIC:
		resist = GetMR();
		break;

	case RESIST_FIRE:
		resist = GetFR();
		break;

	case RESIST_COLD:
		resist = GetCR();
		break;

	case RESIST_POISON:
		resist = GetPR();
		break;

	case RESIST_DISEASE:
		resist = GetDR();
		break;

	// Hvitrev: Primsatic = average of all resists
	case RESIST_PRISMATIC:
		resist = (GetDR()+GetMR()+GetFR()+GetCR()+GetPR())/5;
		break;

	// Hvitrev: Chromatic = lowest of all resists
	case RESIST_CHROMATIC: {
		sint16 tempresist = GetMR();
		sint16 tempresist2 = GetFR();

		if ( tempresist < tempresist2 )
			resist = tempresist;
		else
			resist = tempresist2;
		tempresist = GetCR();
		if ( tempresist < resist )
			resist = tempresist;
		tempresist = GetDR();
		if ( tempresist < resist )
			resist = tempresist;
		tempresist = GetPR();
		if ( tempresist < resist )
			resist = tempresist;
		break;
	}

	// solar: I don't know how to calculate this stuff
	case RESIST_PHYSICAL:
	default:
		resist = GetMR();
		break;
	}

	// value in spell to adjust base resist by
	if(spell_to_cast)
		resist += spell_to_cast->GetSpell().ResistDiff;

	//This is our base resist chance given no resists and no level diff, set to a modest 2% by default
	resistchance = RuleR(Spells, ResistChance);

	//changed this again, just straight 8.5 resist points per level above you
	float lvldiff = caster_level - target_level;
	resist += (RuleI(Spells, ResistPerLevelDiff) * (-lvldiff) / 10);

	/*The idea is we come up with 3 ranges of numbers and a roll between 0 and 100
	[[[Empty Space above the resistchance line]]] - If the roll lands up here the spell wasn't resisted, the lower the resist chance the larger this range is
	[[[Space between resistchance line and full resist chance line]]] - If the roll ends up here then the spell is resisted but only partially, we take the roll in porportion to where it landed in this range to det how
	high the partial should be, for example if we rolled barely over the full resist chance line then it would result in a low partial but if we barely missed the spell not resisting then it would result in a very high partial
	The higher the resist the larger this range will be.
	[[[Space below the full resist chance line]]] - If the roll ends up down here then the spell was resisted fully, the higher the resist the larger this range will be.
	*/

	//default 0.40: 500 resist = 200% Base resist while 40 resist = 16% resist base.
	//Set ResistMod lower to require more resist points per percentage point of resistance.
	resistchance += resist * RuleR(Spells, ResistMod);
	resistchance += spellbonuses.ResistSpellChance + itembonuses.ResistSpellChance;

	if(caster && spell_to_cast && caster->IsClient())
	{
		sint32 focusResist = caster->CastToClient()->GetFocusEffect(focusResistRate, spell_to_cast);
		resistchance = (resistchance * (100-focusResist) / 100);
	}

#ifdef EQBOTS

	if(caster && caster->IsBot())
	{
		if(IsValidSpell(spell_id))
		{
			sint32 focusResist = caster->GetBotFocusEffect(botfocusResistRate, spell_id);
			resistchance = (resistchance * (100-focusResist) / 100);
		}
	}

#endif //EQBOTS

	//Resist chance makes up the upper limit of our partial range
	//Fullchance makes up the lower limit of our partial range
	if(spell_to_cast)
	{
		if(spell_to_cast->IsEffectInSpell(SE_Fear))
		{
			fullchance = (resistchance * (1 - RuleR(Spells, PartialHitChanceFear))); //default 0.25
		}
		else
		{
			fullchance = (resistchance * (1 - RuleR(Spells, PartialHitChance))); //default 0.7
		}
	}
	else
	{
		fullchance = (resistchance * (1 - RuleR(Spells, PartialHitChance))); //default 0.7
	}

	roll = MakeRandomFloat(0, 100);

	mlog(SPELLS__RESISTS, "Spell %d: Resist Amount: %d, ResistChance: %.2f, Resist Bonuses: %.2f",
		spell_to_cast->GetSpellID(), resist, resistchance, (spellbonuses.ResistSpellChance + itembonuses.ResistSpellChance));

	if (roll > resistchance)
	{
		mlog(SPELLS__RESISTS, "Spell %d: Roll of %.2f > resist chance of %.2f, no resist",
			spell_to_cast ? spell_to_cast->GetSpellID() : 0xFFFF, roll, resistchance);
		return(100);
	}
	else
	{
		if (roll <= fullchance)
 		{
			mlog(SPELLS__RESISTS, "Spell %d: Roll of %.2f <= fullchance %.2f, fully resisted",
				spell_to_cast ? spell_to_cast->GetSpellID() : 0xFFFF, roll, fullchance);
			return(0);
		}
		else
		{
			mlog(SPELLS__RESISTS, "Spell %d: Roll of %.2f > fullchance %.2f, partially resisted, returned %.2f",
				spell_to_cast? spell_to_cast->GetSpellID() : 0xFFFF, roll,
				fullchance, (100 * ((roll-fullchance)/(resistchance-fullchance))));
			//Remove the lower range so it doesn't throw off the proportion.
			return(100 * ((roll-fullchance)/(resistchance-fullchance)));
		}
	}
}

float Mob::GetAOERange(Spell *spell_to_cast) {
	float range;

	range = spell_to_cast->GetSpell().aoerange;
	if(range == 0)	//for TGB spells, they prolly do not have an aoe range
		range = spell_to_cast->GetSpell().range;
	if(range == 0)
		range = 10;	//something....

	float mod = 0;
	if (IsClient()) {
		if(spell_to_cast->IsBardSong()) {
			switch (GetAA(aaExtendedNotes) + GetAA(aaExtendedNotes2))
			{
				case 1:
					mod += range * 0.10;
					break;
				case 2:
					mod += range * 0.15;
					break;
				case 3:
				case 4:
				case 5:
				case 6:
					mod += range * 0.25;
					break;
			}
			switch (GetAA(aaSionachiesCrescendo) + GetAA(aaSionachiesCrescendo2))
			{
				case 1:
					mod += range * 0.05;
					break;
				case 2:
					mod += range * 0.10;
					break;
				case 3:
				case 4:
				case 5:
				case 6:
					mod += range * 0.15;
					break;
			}
			range += mod;
		}

		range = CastToClient()->GetActSpellRange(spell_to_cast, range);
	}

	return(range);
}

///////////////////////////////////////////////////////////////////////////////
// 'other' functions

void Mob::Spin() {
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_Action, sizeof(Action_Struct));
	outapp->pBuffer[0] = 0x0B;
	outapp->pBuffer[1] = 0x0A;
	outapp->pBuffer[2] = 0x0B;
	outapp->pBuffer[3] = 0x0A;
	outapp->pBuffer[4] = 0xE7;
	outapp->pBuffer[5] = 0x00;
	outapp->pBuffer[6] = 0x4D;
	outapp->pBuffer[7] = 0x04;
	outapp->pBuffer[8] = 0x00;
	outapp->pBuffer[9] = 0x00;
	outapp->pBuffer[10] = 0x00;
	outapp->pBuffer[11] = 0x00;
	outapp->pBuffer[12] = 0x00;
	outapp->pBuffer[13] = 0x00;
	outapp->pBuffer[14] = 0x00;
	outapp->pBuffer[15] = 0x00;
	outapp->pBuffer[16] = 0x00;
	outapp->pBuffer[17] = 0x00;
	outapp->pBuffer[18] = 0xD4;
	outapp->pBuffer[19] = 0x43;
	outapp->pBuffer[20] = 0x00;
	outapp->pBuffer[21] = 0x00;
	outapp->pBuffer[22] = 0x00;
	outapp->priority = 5;
	CastToClient()->QueuePacket(outapp);
	safe_delete(outapp);
}

void Mob::SendSpellBarDisable()
{
	if (!IsClient())
		return;

	CastToClient()->MemorizeSpell(0, SPELLBAR_UNLOCK, memSpellSpellbar);
}

// solar: this puts the spell bar back into a usable state fast
void Mob::SendSpellBarEnable(int16 spell_id)
{
	if(!IsClient())
		return;

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_ManaChange, sizeof(ManaChange_Struct));
	ManaChange_Struct* manachange = (ManaChange_Struct*)outapp->pBuffer;
	manachange->new_mana = GetMana();
	manachange->spell_id = spell_id;
	manachange->stamina = CastToClient()->GetEndurance();
	outapp->priority = 6;
	CastToClient()->QueuePacket(outapp);
	safe_delete(outapp);
}

void Mob::Stun(int duration)
{
	//make sure a shorter stun does not overwrite a longer one.
	if(stunned && stunned_timer.GetRemainingTime() > uint32(duration))
		return;

	if(casting_spell)
		InterruptSpell();

	if(duration > 0)
	{
		stunned = true;
		stunned_timer.Start(duration);
	}
}

void Mob::UnStun() {
	if(stunned && stunned_timer.Enabled()) {
		stunned = false;
		stunned_timer.Disable();
	}
}

// Hogie - Stuns "this"
void Client::Stun(int duration)
{
	Mob::Stun(duration);

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_Stun, sizeof(Stun_Struct));
	Stun_Struct* stunon = (Stun_Struct*) outapp->pBuffer;
	stunon->duration = duration;
	outapp->priority = 5;
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::UnStun() {
	Mob::UnStun();

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_Stun, sizeof(Stun_Struct));
	Stun_Struct* stunon = (Stun_Struct*) outapp->pBuffer;
	stunon->duration = 0;
	outapp->priority = 5;
	QueuePacket(outapp);
	safe_delete(outapp);
}

void NPC::Stun(int duration) {
	Mob::Stun(duration);
	SetRunAnimSpeed(0);
	SendPosition();
}

void NPC::UnStun() {
	Mob::UnStun();
	SetRunAnimSpeed(this->GetRunspeed());
	SendPosition();
}

void Mob::Mesmerize()
{
	mezzed = true;

	if (casting_spell)
		InterruptSpell();
}

void Client::MakeBuffFadePacket(Buff* buff, int slot_id, bool send_message)
{
	SendBuffPacket(buff, slot_id, 1);

	if(send_message)
	{
		const char *fade_text = buff->GetSpell()->GetSpell().spell_fades;
		uint32 fade_text_len = strlen(fade_text);

		EQApplicationPacket *outapp = new EQApplicationPacket(OP_BuffFadeMsg, sizeof(BuffFadeMsg_Struct) + fade_text_len);
		BuffFadeMsg_Struct *bfm = (BuffFadeMsg_Struct *) outapp->pBuffer;
		bfm->color = MT_Spells;
		memcpy(bfm->msg, fade_text, fade_text_len);
		QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Client::MemSpell(int16 spell_id, int slot, bool update_client)
{
	if(slot >= MAX_PP_MEMSPELL || slot < 0)
		return;

	if(update_client)
	{
		if(m_pp.mem_spells[slot] != 0xFFFFFFFF)
			UnmemSpell(slot, update_client);
	}

	m_pp.mem_spells[slot] = spell_id;
	mlog(CLIENT__SPELLS, "Spell %d memorized into slot %d", spell_id, slot);

	if(update_client)
	{
		MemorizeSpell(slot, spell_id, memSpellMemorize);
	}
}

void Client::UnmemSpell(int slot, bool update_client)
{
	if(slot > MAX_PP_MEMSPELL || slot < 0)
		return;

	mlog(CLIENT__SPELLS, "Spell %d forgotten from slot %d", m_pp.mem_spells[slot], slot);
	m_pp.mem_spells[slot] = 0xFFFFFFFF;

	if(update_client)
	{
		MemorizeSpell(slot, m_pp.mem_spells[slot], memSpellForget);
	}
}

void Client::UnmemSpellAll(bool update_client)
{
	int i;

	for(i = 0; i < MAX_PP_MEMSPELL; i++)
		if(m_pp.mem_spells[i] != 0xFFFFFFFF)
			UnmemSpell(i, update_client);
}

void Client::ScribeSpell(int16 spell_id, int slot, bool update_client)
{
	if(slot >= MAX_PP_SPELLBOOK || slot < 0)
		return;

	if(update_client)
	{
		if(m_pp.spell_book[slot] != 0xFFFFFFFF)
			UnscribeSpell(slot, update_client);
	}

	m_pp.spell_book[slot] = spell_id;
	mlog(CLIENT__SPELLS, "Spell %d scribed into spell book slot %d", spell_id, slot);

	if(update_client)
	{
		MemorizeSpell(slot, spell_id, memSpellScribing);
	}
}

void Client::UnscribeSpell(int slot, bool update_client)
{
	if(slot >= MAX_PP_SPELLBOOK || slot < 0)
		return;

	mlog(CLIENT__SPELLS, "Spell %d erased from spell book slot %d", m_pp.spell_book[slot], slot);
	m_pp.spell_book[slot] = 0xFFFFFFFF;

	if(update_client)
	{
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_DeleteSpell, sizeof(DeleteSpell_Struct));
		DeleteSpell_Struct* del = (DeleteSpell_Struct*)outapp->pBuffer;
		del->spell_slot = slot;
		del->success = 1;
		QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Client::UnscribeSpellAll(bool update_client)
{
	int i;

	for(i = 0; i < MAX_PP_SPELLBOOK; i++)
	{
		if(m_pp.spell_book[i] != 0xFFFFFFFF)
			UnscribeSpell(i, update_client);
	}
}

void Client::UntrainDisc(int slot, bool update_client)
{
	if(slot >= MAX_PP_DISCIPLINES || slot < 0)
		return;

	mlog(CLIENT__SPELLS, "Discipline %d untrained from slot %d", m_pp.disciplines.values[slot], slot);
	m_pp.disciplines.values[slot] = 0;

	if(update_client)
	{
		SendDisciplineUpdate();
	}
}

void Client::UntrainDiscAll(bool update_client)
{
	int i;

	for(i = 0; i < MAX_PP_DISCIPLINES; i++)
	{
		if(m_pp.disciplines.values[i] != 0)
			UntrainDisc(i, update_client);
	}
}

int Client::GetNextAvailableSpellBookSlot(int starting_slot) {
	for (int i = starting_slot; i < MAX_PP_SPELLBOOK; i++) {	//using starting_slot should help speed this up when we're iterating through a bunch of spells
		if (!IsValidSpell(GetSpellByBookSlot(i)))
			return i;
	}

	return -1;	//default
}

int Client::FindSpellBookSlotBySpellID(int16 spellid) {
	for(int i = 0; i < MAX_PP_SPELLBOOK; i++) {
		if(m_pp.spell_book[i] == spellid)
			return i;
	}

	return -1;	//default
}

sint8 Mob::GetBuffSlotFromType(int8 type)
{
	for (int i = 0; i < BUFF_COUNT; i++)
	{
		if(buffs[i])
		{
			for (int j = 0; j < EFFECT_COUNT; j++)
			{
				if(buffs[i]->GetSpell()->GetSpell().effectid[j] == type)
					return i;
			}
		}
	}
    return -1;
}

bool Mob::AddProcToWeapon(int16 spell_id, bool bPerma, int16 iChance) {
	if(spell_id == SPELL_UNKNOWN)
		return(false);

	int i;
	if (bPerma) {
 		for (i = 0; i < MAX_PROCS; i++) {
			if (PermaProcs[i].spellID == SPELL_UNKNOWN) {
				PermaProcs[i].spellID = spell_id;
				PermaProcs[i].chance = iChance;
				PermaProcs[i].pTimer = NULL;
				mlog(SPELLS__PROCS, "Added permanent proc spell %d with chance %d to slot %d", spell_id, iChance, i);

				return true;
			}
		}
		mlog(SPELLS__PROCS, "Too many perma procs for %s", GetName());
    } else {
		for (i = 0; i < MAX_PROCS; i++) {
			if (SpellProcs[i].spellID == SPELL_UNKNOWN) {
				SpellProcs[i].spellID = spell_id;
				SpellProcs[i].chance = iChance;
				SpellProcs[i].pTimer = NULL;
				mlog(SPELLS__PROCS, "Added spell-granted proc spell %d with chance %d to slot %d", spell_id, iChance, i);
				return true;
			}
		}
		mlog(SPELLS__PROCS, "Too many procs for %s", GetName());
	}
    return false;
}

bool Mob::RemoveProcFromWeapon(int16 spell_id, bool bAll)
{
	for (int i = 0; i < MAX_PROCS; i++)
	{
		if (bAll || SpellProcs[i].spellID == spell_id)
		{
			SpellProcs[i].spellID = SPELL_UNKNOWN;
			SpellProcs[i].chance = 0;
			SpellProcs[i].pTimer = NULL;
			mlog(SPELLS__PROCS, "Removed proc %d from slot %d", spell_id, i);
		}
	}
    return true;
}

bool Mob::AddDefensiveProc(int16 spell_id, int16 iChance)
{
	if(spell_id == SPELL_UNKNOWN)
		return(false);

	int i;
	for (i = 0; i < MAX_PROCS; i++) {
		if (DefensiveProcs[i].spellID == SPELL_UNKNOWN) {
			DefensiveProcs[i].spellID = spell_id;
			DefensiveProcs[i].chance = iChance;
			DefensiveProcs[i].pTimer = NULL;
			mlog(SPELLS__PROCS, "Added spell-granted defensive proc spell %d with chance %d to slot %d", spell_id, iChance, i);
			return true;
		}
	}

    return false;
}

bool Mob::RemoveDefensiveProc(int16 spell_id, bool bAll)
{
	for (int i = 0; i < MAX_PROCS; i++) {
		if (bAll || DefensiveProcs[i].spellID == spell_id)
		{
			DefensiveProcs[i].spellID = SPELL_UNKNOWN;
			DefensiveProcs[i].chance = 0;
			DefensiveProcs[i].pTimer = NULL;
			mlog(SPELLS__PROCS, "Removed defensive proc %d from slot %d", spell_id, i);
		}
	}
    return true;
}

bool Mob::AddRangedProc(int16 spell_id, int16 iChance)
{
	if(spell_id == SPELL_UNKNOWN)
		return(false);

	for (int i = 0; i < MAX_PROCS; i++)
	{
		if (RangedProcs[i].spellID == SPELL_UNKNOWN)
		{
			RangedProcs[i].spellID = spell_id;
			RangedProcs[i].chance = iChance;
			RangedProcs[i].pTimer = NULL;
			mlog(SPELLS__PROCS, "Added spell-granted ranged proc spell %d with chance %d to slot %d", spell_id, iChance, i);
			return true;
		}
	}

    return false;
}

bool Mob::RemoveRangedProc(int16 spell_id, bool bAll)
{
	for (int i = 0; i < MAX_PROCS; i++)
	{
		if (bAll || RangedProcs[i].spellID == spell_id)
		{
			RangedProcs[i].spellID = SPELL_UNKNOWN;
			RangedProcs[i].chance = 0;
			RangedProcs[i].pTimer = NULL;
			mlog(SPELLS__PROCS, "Removed ranged proc %d from slot %d", spell_id, i);
		}
	}
    return true;
}

// solar: this is checked in a few places to decide wether special bard
// behavior should be used.
bool Mob::UseBardSpellLogic(int16 spell_id, int slot)
{
	if(spell_id == SPELL_UNKNOWN)
		spell_id = casting_spell ? casting_spell->GetSpellID() : 0;

	if(slot == -1)
		slot = casting_spell ? casting_spell->GetSpellSlot() : 0;

	// should we treat this as a bard singing?
	return
	(
		spell_id != 0 &&
		spell_id != SPELL_UNKNOWN &&
		slot != -1 &&
		GetClass() == BARD &&
		slot <= MAX_PP_MEMSPELL &&
		IsBardSong(spell_id)
	);
}

int Mob::GetCasterLevel()
{
	int level = GetLevel();
	level += spellbonuses.effective_casting_level;
	level += itembonuses.effective_casting_level;
	level += GetAA(aaJamFest);
	mlog(SPELLS__CASTING, "Determined effective casting level %d+%d+%d=%d", GetLevel(), spellbonuses.effective_casting_level, itembonuses.effective_casting_level, level);
	return(level);
}


//this method does NOT tell the client to stop singing the song.
//this is NOT the right way to stop a mob from singing, use InterruptSpell
//you should really know what your doing before you call this
void Mob::_StopSong()
{
	mlog(SPELLS__CASTING, "Song has been stopped.");
	if (IsClient() && (bard_song || (casting_spell && casting_spell->IsBardSong())))
	{
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_ManaChange, sizeof(ManaChange_Struct));
		ManaChange_Struct* manachange = (ManaChange_Struct*)outapp->pBuffer;
		manachange->new_mana = cur_mana;
		if (!bard_song)
			manachange->spell_id = casting_spell ? casting_spell->GetSpellID() : 0;
		else
			manachange->spell_id = bard_song->GetSpellID();
		manachange->stamina = CastToClient()->GetEndurance();
		if (CastToClient()->Hungry())
			manachange->stamina = 0;
		CastToClient()->QueuePacket(outapp);
		safe_delete(outapp);
	}
	safe_delete(bard_song);
	bardsong_timer.Disable();
}

void Mob::SendPetBuffsToClient()
{
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_PetBuffWindow,sizeof(PetBuff_Struct));
	PetBuff_Struct* pbs=(PetBuff_Struct*)outapp->pBuffer;
	memset(outapp->pBuffer,0,outapp->size);
	pbs->petid = GetID();

	int PetBuffCount = 0;
	int max_slots = GetMaxTotalSlots();
	for(int buffslot = 0; buffslot < max_slots; buffslot++)
	{
		if(buffs[buffslot])
		{
			pbs->spellid[PetBuffCount] = buffs[buffslot]->GetSpell()->GetSpellID();
			pbs->ticsremaining[PetBuffCount] = buffs[buffslot]->GetDurationRemaining();
			PetBuffCount++;
		}
	}

	pbs->buffcount=PetBuffCount;
	GetOwner()->CastToClient()->QueuePacket(outapp);
	safe_delete(outapp);
}

bool Mob::ValidateStartSpellCast(const Spell *spell_to_cast)
{
	bool return_value = true;
	if(!IsValidSpell(spell_to_cast->GetSpellID()) || casting_spell != NULL)
	{
		mlog(SPELLS__CASTING_ERR, "Spell casting canceled: not able to cast now. Spell is either not valid or we already have a spell casting.");
		return_value = false;
	}
	else if(IsStunned() || IsFeared() || IsMezzed())
	{
		mlog(SPELLS__CASTING_ERR, "Spell casting canceled: not able to cast now. The mob is not in control of it self and therefor cannot cast.");
		return_value = false;
	}
	else if(IsSilenced())
	{
		Message_StringID(13, SILENCED_STRING);
		mlog(SPELLS__CASTING_ERR, "Spell casting canceled: not able to cast now. The mob is not able to speak and therefor cannot cast.");
		return_value = false;
	}
	else if(DivineAura())
	{
		mlog(SPELLS__CASTING_ERR, "Spell casting canceled: not able to cast now. The mob is invulnerable and therefor cannot cast.");
		return_value = false;
	}

	if(return_value && (IsDetrimentalSpell(spell_to_cast->GetSpellID()) && !zone->CanDoCombat()))
	{
		mlog(SPELLS__CASTING_ERR, "Spell casting canceled: this is a detrimental spell in a non-combat flagged zone.");
		Message_StringID(13, SPELL_WOULDNT_HOLD);
		return_value = false;
	}

	if(return_value == false)
	{
		ValidateSpellCastFinish(spell_to_cast);
	}
	return return_value;
}

void Client::ValidateSpellCastFinish(const Spell *spell_to_cast)
{
	SendSpellBarEnable(spell_to_cast->GetSpellID());
}

void NPC::ValidateSpellCastFinish(const Spell *spell_to_cast)
{
	AI_Event_SpellCastFinished(false, spell_to_cast->GetSpellSlot());
}

bool Mob::DoChannelCheck(bool &did_regain_conc)
{
	// if has been attacked, or moved while casting
	// check for regain concentration
	if(attacked_count > 0 || GetX() != GetSpellX() || GetY() != GetSpellY())
	{
		// modify the chance based on how many times they were hit
		// but cap it so it's not that large a factor
		if(attacked_count > 15) attacked_count = 15;

		float channelchance, distance_moved, d_x, d_y, distancemod;

		if(IsClient())
		{
			// max 93% chance at 252 skill
			channelchance = 30 + GetSkill(CHANNELING) / 400.0f * 100;
			channelchance -= attacked_count * 2;
			channelchance += channelchance * (GetAA(aaChanellingFocus) * 5) / 100;
			channelchance += channelchance * (GetAA(aaInternalMetronome) * 5) / 100;
		}
		else
		{
			// NPCs are just hard to interrupt, otherwise they get pwned
			channelchance = 45 + GetLevel();
			channelchance -= attacked_count;
		}

		// solar: as you get farther from your casting location,
		// it gets squarely harder to regain concentration
		if(GetX() != GetSpellX() || GetY() != GetSpellY())
		{
			d_x = fabs(fabs(GetX()) - fabs(GetSpellX()));
			d_y = fabs(fabs(GetY()) - fabs(GetSpellY()));
			if(d_x < 5 && d_y < 5)
			{
				//avoid the square root...
				distance_moved = d_x * d_x + d_y * d_y;
				// if you moved 1 unit, that's 25% off your chance to regain.
				// if you moved 2, you lose 100% off your chance
				distancemod = distance_moved * 25;
				channelchance -= distancemod;
			}
			else
			{
				channelchance = 0;
			}
		}

		mlog(SPELLS__CASTING, "Checking Interruption: spell x: %f  spell y: %f  cur x: %f  cur y: %f channelchance %f channeling skill %d\n", GetSpellX(), GetSpellY(), GetX(), GetY(), channelchance, GetSkill(CHANNELING));

		if(MakeRandomFloat(0, 100) > channelchance)
		{
			mlog(SPELLS__CASTING_ERR, "Casting of %d canceled: interrupted.", casting_spell ? casting_spell->GetSpellID() : 0xFFFF);
			InterruptSpell();
			did_regain_conc = false;
			return false;
		}

		// if we got here, we regained concentration
		Message_StringID(MT_Spells, REGAIN_AND_CONTINUE);
		entity_list.MessageClose_StringID(this, true, 200, MT_Spells, OTHER_REGAIN_CAST, this->GetCleanName());
		did_regain_conc = true;
		return true;
	}
	else
	{
		did_regain_conc = false;
		return true;
	}
}

bool Client::DoComponentCheck(Spell *spell_to_cast, bool bard_song_mode)
{
	int reg_focus = CastToClient()->GetFocusEffect(focusReagentCost, spell_to_cast);
	if(MakeRandomInt(0, 100) <= reg_focus)
	{
		mlog(SPELLS__CASTING, "Spell %d: Reagent focus item prevented reagent consumption (%d chance)", spell_to_cast->GetSpellID(), reg_focus);
	}
	else
	{
		if(reg_focus > 0)
		{
			mlog(SPELLS__CASTING, "Spell %d: Reagent focus item failed to prevent reagent consumption (%d chance)", spell_to_cast->GetSpellID(), reg_focus);
		}

		int component, component_count, inv_slot_id;
		for(int t_count = 0; t_count < 4; t_count++)
		{
			component = spell_to_cast->GetSpell().components[t_count];
			component_count = spell_to_cast->GetSpell().component_counts[t_count];
			if(component == -1)
			{
				continue;
			}

			if(bard_song_mode)
			{
				bool has_instrument = true;
				int inst_component = spell_to_cast->GetSpell().NoexpendReagent[0];
				switch(inst_component)
				{
					// no instrument required, go to next component
				case -1:
					continue;

					// percussion songs (13000 = hand drum)
				case 13000:
					if(itembonuses.percussionMod == 0)
					{
						has_instrument = false;
						Message_StringID(13, SONG_NEEDS_DRUM);	// send an error message if missing
					}
					break;

					// wind songs (13001 = wooden flute)
				case 13001:
					if(itembonuses.windMod == 0)
					{
						has_instrument = false;
						Message_StringID(13, SONG_NEEDS_WIND);
					}
					break;

					// string songs (13011 = lute)
				case 13011:
					if(itembonuses.stringedMod == 0)
					{
						has_instrument = false;
						Message_StringID(13, SONG_NEEDS_STRINGS);
					}
					break;

					// brass songs (13012 = horn)
				case 13012:
					if(itembonuses.brassMod == 0)
					{
						has_instrument = false;
						Message_StringID(13, SONG_NEEDS_BRASS);
					}
					break;

				default:	// some non-instrument component.  Let it go, but record it in the log
					mlog(SPELLS__CASTING_ERR, "Something odd happened: Song %d required component %s", spell_to_cast->GetSpellID(), component);
				}

				if(!has_instrument)
				{
					// if the instrument is missing, log it and interrupt the song
					mlog(SPELLS__CASTING_ERR, "Song %d: Canceled. Missing required instrument %s", spell_to_cast->GetSpellID(), component);
					if(GetGM())
					{
						Message(0, "Your GM status allows you to finish casting even though you're missing a required instrument.");
					}
					else
					{
						InterruptSpell();
						return false;
					}
				}
			}
			else
			{
				if(GetInv().HasItem(component, component_count, invWhereWorn|invWherePersonal) == -1) // item not found
				{
					Message_StringID(13, MISSING_SPELL_COMP);

					const Item_Struct *item = database.GetItem(component);
					if(item)
					{
						Message_StringID(13, MISSING_SPELL_COMP_ITEM, item->Name);
						mlog(SPELLS__CASTING_ERR, "Spell %d: Canceled. Missing required reagent %s (%d)", spell_to_cast->GetSpellID(), item->Name, component);
					}
					else
					{
						char TempItemName[64];
						strcpy((char*)&TempItemName, "UNKNOWN");
						mlog(SPELLS__CASTING_ERR, "Spell %d: Canceled. Missing required reagent %s (%d)", spell_to_cast->GetSpellID(), TempItemName, component);
					}

					if(GetGM())
					{
						Message(0, "Your GM status allows you to finish casting even though you're missing required components.");
					}
					else
					{
						InterruptSpell();
						return false;
					}

				}
				else
				{
					mlog(SPELLS__CASTING_ERR, "Spell %d: Consuming %d of spell component item id %d", spell_to_cast->GetSpellID(), component, component_count);
					// Components found, Deleteing
					// now we go looking for and deleting the items one by one
					for(int s = 0; s < component_count; s++)
					{
						inv_slot_id = GetInv().HasItem(component, 1, invWhereWorn|invWherePersonal);
						if(inv_slot_id != -1)
						{
							DeleteItemInInventory(inv_slot_id, 1, true);
						}
						else
						{
							// some kind of error in the code if this happens
							Message(13, "ERROR: reagent item disappeared while processing?");
						}
					}
				}
			}
		}
	}
	return true;
}

void Client::SendBuffPacket(Buff *buff, uint32 buff_index, uint32 buff_mode, uint32 action)
{
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_Buff, sizeof(SpellBuffFade_Struct));
	SpellBuffFade_Struct* sbf = (SpellBuffFade_Struct*) outapp->pBuffer;

	sbf->entityid = GetID();
	sbf->slot = 2;
	sbf->spellid = buff->GetSpell()->GetSpellID();
	sbf->slotid = buff_index;
	sbf->effect = buff->GetInstrumentMod();
	sbf->level = buff->GetSpell()->GetCasterLevel();
	sbf->bufffade = buff_mode; //0x03 = create new buff, 0x01 = buff fade, 0x00 = update current buff
	sbf->duration = buff->GetDurationRemaining();
	sbf->unknown016 = action;
	QueuePacket(outapp);

	if(buff_mode == 1)
	{
		sbf->spellid = 0xFFFFFFFF;
		QueuePacket(outapp);
	}
	safe_delete(outapp);
}

void Mob::FreeSpell(Spell **spell)
{
	safe_delete(*spell);
}

Spell *Mob::CreateSpell(uint32 spell_id, Mob* caster, Mob* target)
{
	Spell *ret_val = new Spell(spell_id, caster, target);
	return ret_val;
}

Spell::Spell(uint32 spell_id, Mob* caster, Mob* target, uint32 slot, uint32 cast_time, uint32 mana_cost)
{
	caster_id = caster ? caster->GetID() : 0;
	caster_level = caster ? caster->GetCasterLevel() : 0;
	target_id = target ? target->GetID() : 0;
	spell_slot = slot;
	this->cast_time = cast_time;
	this->mana_cost = mana_cost;
	cast_timer = NULL;
	timer_id = -1;
	timer_id_duration = -1;
	spell_class_type = SC_NORMAL;
	spell_slot_inventory = 0xFFFFFFFF;
	custom_data = false;
	cast_timer = NULL;

	const SPDat_Spell_Struct &spell = spells[spell_id];
	memcpy((void*)&raw_spell, (const void*)&spell, sizeof(SPDat_Spell_Struct));
	raw_spell.id = spell_id;
	id = spell_id;
}

Spell::Spell()
{
	cast_timer = NULL;
	spell_class_type = SC_NORMAL;
	custom_data = true;
	id = 0xffffcccc;
}

Spell::Spell(SPDat_Spell_Struct *new_spell, uint32 caster_level, uint32 slot, uint32 inventory_slot, uint32 spell_type)
{
	memcpy((void*)&raw_spell, (const void*)new_spell, sizeof(SPDat_Spell_Struct));
	caster_id = 0;
	caster_level = caster_level;
	target_id = 0;
	spell_slot = slot;
	this->cast_time = -1;
	this->mana_cost = -1;
	cast_timer = NULL;
	timer_id = -1;
	timer_id_duration = -1;
	spell_class_type = (SpellClass)spell_type;
	spell_slot_inventory = inventory_slot;
	custom_data = true;
	cast_timer = NULL;
	id = new_spell->id;
}

Spell::~Spell()
{
	safe_delete(cast_timer);
}

void Spell::StartCastTimer(uint32 duration)
{
	if(cast_timer)
	{
		cast_timer->Start(duration);
	}
	else
	{
		cast_timer = new Timer(duration, true);
	}
}

bool Spell::IsCastTimerFinished() const
{
	if(cast_timer)
	{
		return cast_timer->Check(false);
	}
	else
	{
		return false;
	}
}

void Spell::StopCastTimer()
{
	safe_delete(cast_timer);
}

void Spell::SetCaster(Mob *c)
{
	caster_id = c->GetID();
}

Mob *Spell::GetCaster() const
{
	return entity_list.GetMob(caster_id);
}

void Spell::SetTarget(Mob *t)
{
	target_id = t->GetID();
}

Mob *Spell::GetTarget() const
{
	return entity_list.GetMob(target_id);
}

Spell* Spell::CopySpell()
{
	Spell *return_value = new Spell();
	return_value->caster_level = this->caster_level;
	return_value->caster_id = this->caster_id;
	return_value->target_id = this->target_id;
	return_value->spell_slot = this->spell_slot;
	return_value->spell_slot_inventory = this->spell_slot_inventory;
	return_value->cast_time = this->cast_time;
	return_value->mana_cost = this->mana_cost;
	return_value->timer_id = this->timer_id;
	return_value->timer_id_duration = this->timer_id_duration;
	return_value->spell_class_type = this->spell_class_type;
	return_value->custom_data = this->custom_data;

	if(this->cast_timer)
	{
		return_value->cast_timer = new Timer(this->cast_timer->GetRemainingTime());
	}

	memcpy((void*)&return_value->raw_spell, (const void*)&this->raw_spell, sizeof(SPDat_Spell_Struct));
	return_value->raw_spell.id = GetSpellID();
	id = GetSpellID();
	return return_value;
}

std::string Spell::GetSpellAttribute(std::string field) const
{
	//transform the entire inc string using int tolower(int __C)
	std::transform(field.begin(), field.end(), field.begin(), (int (*)(int))tolower);
	std::stringstream ss(stringstream::in | stringstream::out);
	if(field.length() < 2)
	{
		ss << "UNKNOWN FIELD";
		return ss.str();
	}

	size_t string_loc;
	switch (field[0])
	{
		case 'a':
			{
				string_loc = field.find("aoerange");
				if(string_loc != string::npos)
				{
					ss << raw_spell.aoerange;
					return ss.str();
				}

				string_loc = field.find("aeduration");
				if(string_loc != string::npos)
				{
					ss << raw_spell.AEDuration;
					return ss.str();
				}

				string_loc = field.find("activated");
				if(string_loc != string::npos)
				{
					ss << raw_spell.Activated;
					return ss.str();
				}
			}
			break;
		case 'b':
			{
				if(field[1] == 'a')
				{
					string_loc = field.find("base2");
					if(string_loc != string::npos)
					{
						//get index
						if(field[field.length()-1] != ']')
						{
							ss << "UNKNOWN FIELD";
							return ss.str();
						}

						if(field[field.length()-4] == '[')
						{
							string index = field.substr(field.length()-3, 2);
							ss << raw_spell.base2[atoi(index.c_str())];
							return ss.str();
						}
						else if(field[field.length()-3] == '[')
						{
							string index = field.substr(field.length()-2, 1);
							ss << raw_spell.base2[atoi(index.c_str())];
							return ss.str();
						}
					}
					string_loc = field.find("base");
					if(string_loc != string::npos)
					{
						//get index
						if(field[field.length()-1] != ']')
						{
							ss << "UNKNOWN FIELD";
							return ss.str();
						}

						if(field[field.length()-4] == '[')
						{
							string index = field.substr(field.length()-3, 2);
							ss << raw_spell.base[atoi(index.c_str())];
							return ss.str();
						}
						else if(field[field.length()-3] == '[')
						{
							string index = field.substr(field.length()-2, 1);
							ss << raw_spell.base[atoi(index.c_str())];
							return ss.str();
						}
					}
				}
				else if(field[1] == 'u')
				{
					string_loc = field.find("buffdurationformula");
					if(string_loc != string::npos)
					{
						ss << raw_spell.buffdurationformula;
						return ss.str();
					}
					string_loc = field.find("buffduration");
					if(string_loc != string::npos)
					{
						ss << raw_spell.buffduration;
						return ss.str();
					}
				}
				else
				{
					string_loc = field.find("bonushate");
					if(string_loc != string::npos)
					{
						ss << raw_spell.bonushate;
						return ss.str();
					}
				}
			}
			break;
		case 'c':
			{
				if(field[1] == 'a')
				{
					string_loc = field.find("cast_time");
					if(string_loc != string::npos)
					{
						ss << raw_spell.cast_time;
						return ss.str();
					}
					string_loc = field.find("cast_on_you");
					if(string_loc != string::npos)
					{
						ss << raw_spell.cast_on_you;
						return ss.str();
					}
					string_loc = field.find("cast_on_other");
					if(string_loc != string::npos)
					{
						ss << raw_spell.cast_on_other;
						return ss.str();
					}
					string_loc = field.find("castinganim");
					if(string_loc != string::npos)
					{
						ss << raw_spell.CastingAnim;
						return ss.str();
					}
					string_loc = field.find("can_mgb");
					if(string_loc != string::npos)
					{
						ss << raw_spell.can_mgb;
						return ss.str();
					}
				}
				else if(field[1] == 'o')
				{
					string_loc = field.find("components");
					if(string_loc != string::npos)
					{
						//get index
						if(field[field.length()-1] != ']')
						{
							ss << "UNKNOWN FIELD";
							return ss.str();
						}

						if(field[field.length()-4] == '[')
						{
							string index = field.substr(field.length()-3, 2);
							ss << raw_spell.components[atoi(index.c_str())];
							return ss.str();
						}
						else if(field[field.length()-3] == '[')
						{
							string index = field.substr(field.length()-2, 1);
							ss << raw_spell.components[atoi(index.c_str())];
							return ss.str();
						}
					}

					string_loc = field.find("component_counts");
					if(string_loc != string::npos)
					{
						//get index
						if(field[field.length()-1] != ']')
						{
							ss << "UNKNOWN FIELD";
							return ss.str();
						}

						if(field[field.length()-4] == '[')
						{
							string index = field.substr(field.length()-3, 2);
							ss << raw_spell.component_counts[atoi(index.c_str())];
							return ss.str();
						}
						else if(field[field.length()-3] == '[')
						{
							string index = field.substr(field.length()-2, 1);
							ss << raw_spell.component_counts[atoi(index.c_str())];
							return ss.str();
						}
					}
				}
				else
				{
					string_loc = field.find("classes");
					if(string_loc != string::npos)
					{
						//get index
						if(field[field.length()-1] != ']')
						{
							ss << "UNKNOWN FIELD";
							return ss.str();
						}

						if(field[field.length()-4] == '[')
						{
							string index = field.substr(field.length()-3, 2);
							ss << raw_spell.classes[atoi(index.c_str())];
							return ss.str();
						}
						else if(field[field.length()-3] == '[')
						{
							string index = field.substr(field.length()-2, 1);
							ss << raw_spell.classes[atoi(index.c_str())];
							return ss.str();
						}
						else
						{
							return ss.str();
						}
					}
				}
			}
			break;
		case 'd':
			{
				if(field[1] == 'e')
				{
					string_loc = field.find("deletable");
					if(string_loc != string::npos)
					{
						ss << raw_spell.deletable;
						return ss.str();
					}
					string_loc = field.find("descnum");
					if(string_loc != string::npos)
					{
						ss << raw_spell.descnum;
						return ss.str();
					}
					string_loc = field.find("deities");
					if(string_loc != string::npos)
					{
						//get index
						if(field[field.length()-1] != ']')
						{
							ss << "UNKNOWN FIELD";
							return ss.str();
						}

						if(field[field.length()-4] == '[')
						{
							string index = field.substr(field.length()-3, 2);
							ss << raw_spell.deities[atoi(index.c_str())];
							return ss.str();
						}
						else if(field[field.length()-3] == '[')
						{
							string index = field.substr(field.length()-2, 1);
							ss << raw_spell.deities[atoi(index.c_str())];
							return ss.str();
						}
					}

				}
				else
				{
					string_loc = field.find("dot_stacking_exempt");
					if(string_loc != string::npos)
					{
						ss << raw_spell.dot_stacking_exempt;
						return ss.str();
					}
					string_loc = field.find("damageshieldtype");
					if(string_loc != string::npos)
					{
						ss << raw_spell.DamageShieldType;
						return ss.str();
					}
				}
			}
			break;
		case 'e':
			{
				if(field[1] == 'f')
				{
					string_loc = field.find("effectdescnum");
					if(string_loc != string::npos)
					{
						ss << raw_spell.effectdescnum;
						return ss.str();
					}
					string_loc = field.find("effectid");
					if(string_loc != string::npos)
					{
						//get index
						if(field[field.length()-1] != ']')
						{
							ss << "UNKNOWN FIELD";
							return ss.str();
						}

						if(field[field.length()-4] == '[')
						{
							string index = field.substr(field.length()-3, 2);
							ss << raw_spell.effectid[atoi(index.c_str())];
							return ss.str();
						}
						else if(field[field.length()-3] == '[')
						{
							string index = field.substr(field.length()-2, 1);
							ss << raw_spell.effectid[atoi(index.c_str())];
							return ss.str();
						}
					}
				}
				else if(field[1] == 'n')
				{
					string_loc = field.find("endurcost");
					if(string_loc != string::npos)
					{
						ss << raw_spell.EndurCost;
						return ss.str();
					}
					string_loc = field.find("endurtimerindex");
					if(string_loc != string::npos)
					{
						ss << raw_spell.EndurTimerIndex;
						return ss.str();
					}
					string_loc = field.find("endurupkeep");
					if(string_loc != string::npos)
					{
						ss << raw_spell.EndurUpkeep;
						return ss.str();
					}
				}
				else
				{
					string_loc = field.find("environmenttype");
					if(string_loc != string::npos)
					{
						ss << raw_spell.EnvironmentType;
						return ss.str();
					}
				}
			}
			break;
		case 'f':
			{
				string_loc = field.find("formula");
				if(string_loc != string::npos)
				{
					//get index
					if(field[field.length()-1] != ']')
					{
						ss << "UNKNOWN FIELD";
						return ss.str();
					}

					if(field[field.length()-4] == '[')
					{
						string index = field.substr(field.length()-3, 2);
						ss << raw_spell.formula[atoi(index.c_str())];
						return ss.str();
					}
					else if(field[field.length()-3] == '[')
					{
						string index = field.substr(field.length()-2, 1);
						ss << raw_spell.formula[atoi(index.c_str())];
						return ss.str();
					}
				}
			}
			break;
		case 'g':
			{
				string_loc = field.find("goodeffect");
				if(string_loc != string::npos)
				{
					ss << raw_spell.goodEffect;
					return ss.str();
				}
			}
			break;
		case 'h':
			{
				string_loc = field.find("hateadded");
				if(string_loc != string::npos)
				{
					ss << raw_spell.HateAdded;
					return ss.str();
				}
			}
			break;
		case 'i':
			{
				string_loc = field.find("id");
				if(string_loc != string::npos)
				{
					ss << raw_spell.id;
					return ss.str();
				}
				string_loc = field.find("icon");
				if(string_loc != string::npos)
				{
					ss << raw_spell.icon;
					return ss.str();
				}
			}
			break;
		case 'l':
			{
				string_loc = field.find("lighttype");
				if(string_loc != string::npos)
				{
					ss << raw_spell.LightType;
					return ss.str();
				}
			}
			break;
		case 'm':
			{
				string_loc = field.find("mana");
				if(string_loc != string::npos)
				{
					ss << raw_spell.mana;
					return ss.str();
				}
				string_loc = field.find("memicon");
				if(string_loc != string::npos)
				{
					ss << raw_spell.memicon;
					return ss.str();
				}
				string_loc = field.find("max");
				if(string_loc != string::npos)
				{
					//get index
					if(field[field.length()-1] != ']')
					{
						ss << "UNKNOWN FIELD";
						return ss.str();
					}

					if(field[field.length()-4] == '[')
					{
						string index = field.substr(field.length()-3, 2);
						ss << raw_spell.max[atoi(index.c_str())];
						return ss.str();
					}
					else if(field[field.length()-3] == '[')
					{
						string index = field.substr(field.length()-2, 1);
						ss << raw_spell.max[atoi(index.c_str())];
						return ss.str();
					}
				}
			}
			break;
		case 'n':
			{
				string_loc = field.find("numhits");
				if(string_loc != string::npos)
				{
					ss << raw_spell.numhits;
					return ss.str();
				}
				string_loc = field.find("no_dispel");
				if(string_loc != string::npos)
				{
					ss << raw_spell.no_dispel;
					return ss.str();
				}
				string_loc = field.find("newicon");
				if(string_loc != string::npos)
				{
					ss << raw_spell.new_icon;
					return ss.str();
				}
				string_loc = field.find("name");
				if(string_loc != string::npos)
				{
					ss << raw_spell.name;
					return ss.str();
				}
				string_loc = field.find("npc_category");
				if(string_loc != string::npos)
				{
					ss << raw_spell.npc_category;
					return ss.str();
				}
				string_loc = field.find("npc_usefulness");
				if(string_loc != string::npos)
				{
					ss << raw_spell.npc_usefulness;
					return ss.str();
				}

				string_loc = field.find("noexpendreagent");
				if(string_loc != string::npos)
				{
					//get index
					if(field[field.length()-1] != ']')
					{
						ss << "UNKNOWN FIELD";
						return ss.str();
					}

					if(field[field.length()-4] == '[')
					{
						string index = field.substr(field.length()-3, 2);
						ss << raw_spell.NoexpendReagent[atoi(index.c_str())];
						return ss.str();
					}
					else if(field[field.length()-3] == '[')
					{
						string index = field.substr(field.length()-2, 1);
						ss << raw_spell.NoexpendReagent[atoi(index.c_str())];
						return ss.str();
					}
				}
			}
			break;
		case 'o':
			{
				string_loc = field.find("other_casts");
				if(string_loc != string::npos)
				{
					ss << raw_spell.other_casts;
					return ss.str();
				}
			}
			break;
		case 'p':
			{
				if(field[1] == 'v')
				{
					string_loc = field.find("pvpresistbase");
					if(string_loc != string::npos)
					{
						ss << raw_spell.pvpresistbase;
						return ss.str();
					}
					string_loc = field.find("pvpresistcalc");
					if(string_loc != string::npos)
					{
						ss << raw_spell.pvpresistcalc;
						return ss.str();
					}
					string_loc = field.find("pvpresistcap");
					if(string_loc != string::npos)
					{
						ss << raw_spell.pvpresistcap;
						return ss.str();
					}
				}
				else if(field[1] == 'u')
				{
					string_loc = field.find("pushup");
					if(string_loc != string::npos)
					{
						ss << raw_spell.pushup;
						return ss.str();
					}
					string_loc = field.find("pushback");
					if(string_loc != string::npos)
					{
						ss << raw_spell.pushback;
						return ss.str();
					}
				}
				else
				{
					string_loc = field.find("player");
					if(string_loc != string::npos)
					{
						ss << raw_spell.player_1;
						return ss.str();
					}
				}
			}
			break;
		case 'r':
			{
				if(field[1] == 'e')
				{
					string_loc = field.find("resisttype");
					if(string_loc != string::npos)
					{
						ss << raw_spell.resisttype;
						return ss.str();
					}
					string_loc = field.find("resistdiff");
					if(string_loc != string::npos)
					{
						ss << raw_spell.ResistDiff;
						return ss.str();
					}
					string_loc = field.find("recourselink");
					if(string_loc != string::npos)
					{
						ss << raw_spell.RecourseLink;
						return ss.str();
					}
					string_loc = field.find("recast_time");
					if(string_loc != string::npos)
					{
						ss << raw_spell.recast_time;
						return ss.str();
					}
					string_loc = field.find("recovery_time");
					if(string_loc != string::npos)
					{
						ss << raw_spell.recovery_time;
						return ss.str();
					}
				}
				else
				{
					string_loc = field.find("range");
					if(string_loc != string::npos)
					{
						ss << raw_spell.range;
						return ss.str();
					}
				}
			}
			break;
		case 's':
			{
				if(field[1] == 'p')
				{
					string_loc = field.find("spellgroup");
					if(string_loc != string::npos)
					{
						ss << raw_spell.spellgroup;
						return ss.str();
					}
					string_loc = field.find("spellaffectindex");
					if(string_loc != string::npos)
					{
						ss << raw_spell.SpellAffectIndex;
						return ss.str();
					}
					string_loc = field.find("spellanim");
					if(string_loc != string::npos)
					{
						ss << raw_spell.spellanim;
						return ss.str();
					}
					string_loc = field.find("spell_category");
					if(string_loc != string::npos)
					{
						ss << raw_spell.spell_category;
						return ss.str();
					}
					string_loc = field.find("spell_fades");
					if(string_loc != string::npos)
					{
						ss << raw_spell.spell_fades;
						return ss.str();
					}
				}
				else
				{
					string_loc = field.find("skill");
					if(string_loc != string::npos)
					{
						ss << raw_spell.skill;
						return ss.str();
					}

					string_loc = field.find("short_buff_box");
					if(string_loc != string::npos)
					{
						ss << raw_spell.short_buff_box;
						return ss.str();
					}
				}
			}
			break;
		case 't':
			{
				if(field[1] == 'a')
				{
					string_loc = field.find("targettype");
					if(string_loc != string::npos)
					{
						ss << raw_spell.targettype;
						return ss.str();
					}
					string_loc = field.find("targetanim");
					if(string_loc != string::npos)
					{
						ss << raw_spell.TargetAnim;
						return ss.str();
					}
				}
				else
				{
					string_loc = field.find("teleport_zone");
					if(string_loc != string::npos)
					{
						ss << raw_spell.teleport_zone;
						return ss.str();
					}
					string_loc = field.find("timeofday");
					if(string_loc != string::npos)
					{
						ss << raw_spell.TimeOfDay;
						return ss.str();
					}
					string_loc = field.find("traveltype");
					if(string_loc != string::npos)
					{
						ss << raw_spell.TravelType;
						return ss.str();
					}
					string_loc = field.find("typedescnum");
					if(string_loc != string::npos)
					{
						ss << raw_spell.typedescnum;
						return ss.str();
					}
				}
			}
			break;
		case 'u':
			{
				string_loc = field.find("uninterruptable");
				if(string_loc != string::npos)
				{
					ss << raw_spell.uninterruptable;
					return ss.str();
				}
			}
			break;
		case 'y':
			{
				string_loc = field.find("you_cast");
				if(string_loc != string::npos)
				{
					ss << raw_spell.you_cast;
					return ss.str();
				}
			}
		case 'z':
			{
				string_loc = field.find("zonetype");
				if(string_loc != string::npos)
				{
					ss << raw_spell.zonetype;
					return ss.str();
				}
			}
			break;
		default:
			ss << "UNKNOWN FIELD";
			return ss.str();
	}

	ss << "UNKNOWN FIELD";
	return ss.str();
}

void Spell::SetSpellAttribute(std::string attribute, std::string field)
{
	//transform the entire inc string using int tolower(int __C)
	std::transform(field.begin(), field.end(), field.begin(), (int (*)(int))tolower);
	std::stringstream ss(stringstream::in | stringstream::out);

	if(field.length() < 2)
	{
		return;
	}

	size_t string_loc;
	switch (field[0])
	{
		case 'a':
			{
				string_loc = field.find("aoerange");
				if(string_loc != string::npos)
				{
					raw_spell.aoerange = atof(attribute.c_str());
					SetCustomSpell(true);
					return;
				}

				string_loc = field.find("aeduration");
				if(string_loc != string::npos)
				{
					raw_spell.AEDuration = atoul(attribute.c_str());
					SetCustomSpell(true);
					return;
				}

				string_loc = field.find("activated");
				if(string_loc != string::npos)
				{
					raw_spell.Activated = atoi(attribute.c_str());
					SetCustomSpell(true);
					return;
				}
			}
			break;
		case 'b':
			{
				if(field[1] == 'a')
				{
					string_loc = field.find("base2");
					if(string_loc != string::npos)
					{
						//get index
						if(field[field.length()-1] != ']')
						{
							return;
						}

						if(field[field.length()-4] == '[')
						{
							string index = field.substr(field.length()-3, 2);
							raw_spell.base2[atoi(index.c_str())] = atoi(attribute.c_str());
							SetCustomSpell(true);
							return;
						}
						else if(field[field.length()-3] == '[')
						{
							string index = field.substr(field.length()-2, 1);
							raw_spell.base2[atoi(index.c_str())] = atoi(attribute.c_str());
							SetCustomSpell(true);
							return;
						}
					}
					string_loc = field.find("base");
					if(string_loc != string::npos)
					{
						//get index
						if(field[field.length()-1] != ']')
						{
							return;
						}

						if(field[field.length()-4] == '[')
						{
							string index = field.substr(field.length()-3, 2);
							raw_spell.base[atoi(index.c_str())] = atoi(attribute.c_str());
							SetCustomSpell(true);
							return;
						}
						else if(field[field.length()-3] == '[')
						{
							string index = field.substr(field.length()-2, 1);
							raw_spell.base[atoi(index.c_str())] = atoi(attribute.c_str());
							SetCustomSpell(true);
							return;
						}
					}
				}
				else if(field[1] == 'u')
				{
					string_loc = field.find("buffdurationformula");
					if(string_loc != string::npos)
					{
						raw_spell.buffdurationformula = atoul(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("buffduration");
					if(string_loc != string::npos)
					{
						raw_spell.buffduration = atoul(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
				}
				else
				{
					string_loc = field.find("bonushate");
					if(string_loc != string::npos)
					{
						raw_spell.bonushate = atoi(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
				}
			}
			break;
		case 'c':
			{
				if(field[1] == 'a')
				{
					string_loc = field.find("cast_time");
					if(string_loc != string::npos)
					{
						raw_spell.cast_time = atoul(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("cast_on_you");
					if(string_loc != string::npos)
					{
						strcpy(raw_spell.cast_on_you, attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("cast_on_other");
					if(string_loc != string::npos)
					{
						strcpy(raw_spell.cast_on_other, attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("castinganim");
					if(string_loc != string::npos)
					{
						raw_spell.CastingAnim = atoul(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("can_mgb");
					if(string_loc != string::npos)
					{
						raw_spell.can_mgb = atoi(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
				}
				else if(field[1] == 'o')
				{
					string_loc = field.find("components");
					if(string_loc != string::npos)
					{
						//get index
						if(field[field.length()-1] != ']')
						{
							return;
						}

						if(field[field.length()-4] == '[')
						{
							string index = field.substr(field.length()-3, 2);
							raw_spell.components[atoi(index.c_str())] = atoi(attribute.c_str());
							SetCustomSpell(true);
							return;
						}
						else if(field[field.length()-3] == '[')
						{
							string index = field.substr(field.length()-2, 1);
							raw_spell.components[atoi(index.c_str())] = atoi(attribute.c_str());
							SetCustomSpell(true);
							return;
						}
					}

					string_loc = field.find("component_counts");
					if(string_loc != string::npos)
					{
						//get index
						if(field[field.length()-1] != ']')
						{
							return;
						}

						if(field[field.length()-4] == '[')
						{
							string index = field.substr(field.length()-3, 2);
							raw_spell.component_counts[atoi(index.c_str())] = atoi(attribute.c_str());
							SetCustomSpell(true);
							return;
						}
						else if(field[field.length()-3] == '[')
						{
							string index = field.substr(field.length()-2, 1);
							raw_spell.component_counts[atoi(index.c_str())] = atoi(attribute.c_str());
							SetCustomSpell(true);
							return;
						}
					}
				}
				else
				{
					string_loc = field.find("classes");
					if(string_loc != string::npos)
					{
						//get index
						if(field[field.length()-1] != ']')
						{
							return;
						}

						if(field[field.length()-4] == '[')
						{
							string index = field.substr(field.length()-3, 2);
							raw_spell.classes[atoi(index.c_str())] = atoul(attribute.c_str());
							SetCustomSpell(true);
							return;
						}
						else if(field[field.length()-3] == '[')
						{
							string index = field.substr(field.length()-2, 1);
							raw_spell.classes[atoi(index.c_str())] = atoul(attribute.c_str());
							SetCustomSpell(true);
							return;
						}
					}
				}
			}
			break;
		case 'd':
			{
				if(field[1] == 'e')
				{
					string_loc = field.find("deletable");
					if(string_loc != string::npos)
					{
						raw_spell.deletable = atoi(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("descnum");
					if(string_loc != string::npos)
					{
						raw_spell.descnum = atoi(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("deities");
					if(string_loc != string::npos)
					{
						//get index
						if(field[field.length()-1] != ']')
						{
							return;
						}

						if(field[field.length()-4] == '[')
						{
							string index = field.substr(field.length()-3, 2);
							raw_spell.deities[atoi(index.c_str())] = atoi(attribute.c_str());
							SetCustomSpell(true);
							return;
						}
						else if(field[field.length()-3] == '[')
						{
							string index = field.substr(field.length()-2, 1);
							raw_spell.deities[atoi(index.c_str())] = atoi(attribute.c_str());
							SetCustomSpell(true);
							return;
						}
					}

				}
				else
				{
					string_loc = field.find("dot_stacking_exempt");
					if(string_loc != string::npos)
					{
						raw_spell.dot_stacking_exempt = atoi(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("damageshieldtype");
					if(string_loc != string::npos)
					{
						raw_spell.DamageShieldType = atoul(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
				}
			}
			break;
		case 'e':
			{
				if(field[1] == 'f')
				{
					string_loc = field.find("effectdescnum");
					if(string_loc != string::npos)
					{
						raw_spell.effectdescnum = atoi(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("effectid");
					if(string_loc != string::npos)
					{
						//get index
						if(field[field.length()-1] != ']')
						{
							return;
						}

						if(field[field.length()-4] == '[')
						{
							string index = field.substr(field.length()-3, 2);
							raw_spell.effectid[atoi(index.c_str())] = atoi(attribute.c_str());
							SetCustomSpell(true);
							return;
						}
						else if(field[field.length()-3] == '[')
						{
							string index = field.substr(field.length()-2, 1);
							raw_spell.effectid[atoi(index.c_str())] = atoi(attribute.c_str());
							SetCustomSpell(true);
							return;
						}
					}
				}
				else if(field[1] == 'n')
				{
					string_loc = field.find("endurcost");
					if(string_loc != string::npos)
					{
						raw_spell.EndurCost = atoi(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("endurtimerindex");
					if(string_loc != string::npos)
					{
						raw_spell.EndurTimerIndex = atoi(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("endurupkeep");
					if(string_loc != string::npos)
					{
						raw_spell.EndurUpkeep = atoi(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
				}
				else
				{
					string_loc = field.find("environmenttype");
					if(string_loc != string::npos)
					{
						raw_spell.EnvironmentType = atoul(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
				}
			}
			break;
		case 'f':
			{
				string_loc = field.find("formula");
				if(string_loc != string::npos)
				{
					//get index
					if(field[field.length()-1] != ']')
					{
						return;
					}

					if(field[field.length()-4] == '[')
					{
						string index = field.substr(field.length()-3, 2);
						raw_spell.formula[atoi(index.c_str())] = atoul(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					else if(field[field.length()-3] == '[')
					{
						string index = field.substr(field.length()-2, 1);
						raw_spell.formula[atoi(index.c_str())] = atoul(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
				}
			}
			break;
		case 'g':
			{
				string_loc = field.find("goodeffect");
				if(string_loc != string::npos)
				{
					raw_spell.goodEffect = atoi(attribute.c_str());
					SetCustomSpell(true);
					return;
				}
			}
			break;
		case 'h':
			{
				string_loc = field.find("hateadded");
				if(string_loc != string::npos)
				{
					raw_spell.HateAdded = atoi(attribute.c_str());
					SetCustomSpell(true);
					return;
				}
			}
			break;
		case 'i':
			{
				string_loc = field.find("id");
				if(string_loc != string::npos)
				{
					raw_spell.id = atoi(attribute.c_str());
					SetCustomSpell(true);
					return;
				}
				string_loc = field.find("icon");
				if(string_loc != string::npos)
				{
					raw_spell.icon = atoul(attribute.c_str());
					SetCustomSpell(true);
					return;
				}
			}
			break;
		case 'l':
			{
				string_loc = field.find("lighttype");
				if(string_loc != string::npos)
				{
					raw_spell.LightType = atoi(attribute.c_str());
					SetCustomSpell(true);
					return;
				}
			}
			break;
		case 'm':
			{
				string_loc = field.find("mana");
				if(string_loc != string::npos)
				{
					raw_spell.mana = atoul(attribute.c_str());
					SetCustomSpell(true);
					return;
				}
				string_loc = field.find("memicon");
				if(string_loc != string::npos)
				{
					raw_spell.memicon = atoul(attribute.c_str());
					SetCustomSpell(true);
					return;
				}
				string_loc = field.find("max");
				if(string_loc != string::npos)
				{
					//get index
					if(field[field.length()-1] != ']')
					{
						return;
					}

					if(field[field.length()-4] == '[')
					{
						string index = field.substr(field.length()-3, 2);
						raw_spell.max[atoi(index.c_str())] = atoi(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					else if(field[field.length()-3] == '[')
					{
						string index = field.substr(field.length()-2, 1);
						raw_spell.max[atoi(index.c_str())] = atoi(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
				}
			}
			break;
		case 'n':
			{
				string_loc = field.find("numhits");
				if(string_loc != string::npos)
				{
					raw_spell.numhits = atoi(attribute.c_str());
					SetCustomSpell(true);
					return;
				}
				string_loc = field.find("no_dispel");
				if(string_loc != string::npos)
				{
					raw_spell.no_dispel = atoi(attribute.c_str());
					SetCustomSpell(true);
					return;
				}
				string_loc = field.find("newicon");
				if(string_loc != string::npos)
				{
					raw_spell.new_icon = atoi(attribute.c_str());
					SetCustomSpell(true);
					return;
				}
				string_loc = field.find("name");
				if(string_loc != string::npos)
				{
					strcpy(raw_spell.name, attribute.c_str());
					SetCustomSpell(true);
					return;
				}
				string_loc = field.find("npc_category");
				if(string_loc != string::npos)
				{
					raw_spell.npc_category = atoul(attribute.c_str());
					SetCustomSpell(true);
					return;
				}
				string_loc = field.find("npc_usefulness");
				if(string_loc != string::npos)
				{
					raw_spell.npc_usefulness = atoul(attribute.c_str());
					SetCustomSpell(true);
					return;
				}

				string_loc = field.find("noexpendreagent");
				if(string_loc != string::npos)
				{
					//get index
					if(field[field.length()-1] != ']')
					{
						return;
					}

					if(field[field.length()-4] == '[')
					{
						string index = field.substr(field.length()-3, 2);
						raw_spell.NoexpendReagent[atoi(index.c_str())] = atoi(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					else if(field[field.length()-3] == '[')
					{
						string index = field.substr(field.length()-2, 1);
						raw_spell.NoexpendReagent[atoi(index.c_str())] = atoi(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
				}
			}
			break;
		case 'o':
			{
				string_loc = field.find("other_casts");
				if(string_loc != string::npos)
				{
					strcpy(raw_spell.other_casts, attribute.c_str());
					SetCustomSpell(true);
					return;
				}
			}
			break;
		case 'p':
			{
				if(field[1] == 'v')
				{
					string_loc = field.find("pvpresistbase");
					if(string_loc != string::npos)
					{
						raw_spell.pvpresistbase = atoi(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("pvpresistcalc");
					if(string_loc != string::npos)
					{
						raw_spell.pvpresistcalc = atoi(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("pvpresistcap");
					if(string_loc != string::npos)
					{
						raw_spell.pvpresistcap = atoi(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
				}
				else if(field[1] == 'u')
				{
					string_loc = field.find("pushup");
					if(string_loc != string::npos)
					{
						raw_spell.pushup = atof(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("pushback");
					if(string_loc != string::npos)
					{
						raw_spell.pushback = atof(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
				}
				else
				{
					string_loc = field.find("player");
					if(string_loc != string::npos)
					{
						strcpy(raw_spell.player_1, attribute.c_str());
						SetCustomSpell(true);
						return;
					}
				}
			}
			break;
		case 'r':
			{
				if(field[1] == 'e')
				{
					string_loc = field.find("resisttype");
					if(string_loc != string::npos)
					{
						raw_spell.resisttype = atoi(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("resistdiff");
					if(string_loc != string::npos)
					{
						raw_spell.ResistDiff = atoi(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("recourselink");
					if(string_loc != string::npos)
					{
						raw_spell.RecourseLink = atoul(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("recast_time");
					if(string_loc != string::npos)
					{
						raw_spell.recast_time = atoul(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("recovery_time");
					if(string_loc != string::npos)
					{
						raw_spell.recovery_time = atoul(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
				}
				else
				{
					string_loc = field.find("range");
					if(string_loc != string::npos)
					{
						raw_spell.range = atof(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
				}
			}
			break;
		case 's':
			{
				if(field[1] == 'p')
				{
					string_loc = field.find("spellgroup");
					if(string_loc != string::npos)
					{
						raw_spell.spellgroup = atoul(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("spellaffectindex");
					if(string_loc != string::npos)
					{
						raw_spell.SpellAffectIndex = atoul(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("spellanim");
					if(string_loc != string::npos)
					{
						raw_spell.spellanim = atoi(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("spell_category");
					if(string_loc != string::npos)
					{
						raw_spell.spell_category = atoi(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("spell_fades");
					if(string_loc != string::npos)
					{
						strcpy(raw_spell.spell_fades, attribute.c_str());
						SetCustomSpell(true);
						return;
					}
				}
				else
				{
					string_loc = field.find("skill");
					if(string_loc != string::npos)
					{
						raw_spell.skill = (SkillType)atoi(attribute.c_str());
						SetCustomSpell(true);
						return;
					}

					string_loc = field.find("short_buff_box");
					if(string_loc != string::npos)
					{
						raw_spell.short_buff_box = atoi(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
				}
			}
			break;
		case 't':
			{
				if(field[1] == 'a')
				{
					string_loc = field.find("targettype");
					if(string_loc != string::npos)
					{
						raw_spell.targettype = (SpellTargetType)atoi(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("targetanim");
					if(string_loc != string::npos)
					{
						raw_spell.TargetAnim = atoul(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
				}
				else
				{
					string_loc = field.find("teleport_zone");
					if(string_loc != string::npos)
					{
						strcpy(raw_spell.teleport_zone, attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("timeofday");
					if(string_loc != string::npos)
					{
						raw_spell.TimeOfDay = atoi(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("traveltype");
					if(string_loc != string::npos)
					{
						raw_spell.TravelType = atoul(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
					string_loc = field.find("typedescnum");
					if(string_loc != string::npos)
					{
						raw_spell.typedescnum = atoi(attribute.c_str());
						SetCustomSpell(true);
						return;
					}
				}
			}
			break;
		case 'u':
			{
				string_loc = field.find("uninterruptable");
				if(string_loc != string::npos)
				{
					raw_spell.uninterruptable = atoi(attribute.c_str());
					SetCustomSpell(true);
					return;
				}
			}
			break;
		case 'y':
			{
				string_loc = field.find("you_cast");
				if(string_loc != string::npos)
				{
					strcpy(raw_spell.you_cast, attribute.c_str());
					SetCustomSpell(true);
					return;
				}
			}
		case 'z':
			{
				string_loc = field.find("zonetype");
				if(string_loc != string::npos)
				{
					raw_spell.zonetype = atoi(attribute.c_str());
					SetCustomSpell(true);
					return;
				}
			}
			break;
		default:
			return;
	}

	return;
}
