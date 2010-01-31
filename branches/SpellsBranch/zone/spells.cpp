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


/*

	solar:	General outline of spell casting process
	
	1.
		a)	Client clicks a spell bar gem, ability, or item.  client_process.cpp
		gets the op and calls CastSpell() with all the relevant info including
		cast time.

		b)  NPC does CastSpell() from AI

	2.
		a)	CastSpell() determines there is a cast time and sets some state keeping
		flags to be used to check the progress of casting and finish it later.

		b)	CastSpell() sees there's no cast time, and calls CastedSpellFinished()
		Go to step 4.

	3.
		SpellProcess() notices that the spell casting timer which was set by
		CastSpell() is expired, and calls CastedSpellFinished()

	4.
		CastedSpellFinished() checks some timed spell specific things, like
		wether to interrupt or not, due to movement or melee.  If successful
		SpellFinished() is called.

	5.
		SpellFinished() checks some things like LoS, reagents, target and
		figures out what's going to get hit by this spell based on its type.

	6.
		a)	Single target spell, SpellOnTarget() is called.

		b)	AE spell, Entity::AESpell() is called.

		c)	Group spell, Group::CastGroupSpell()/SpellOnTarget() is called as
		needed.

	7.
		SpellOnTarget() may or may not call SpellEffect() to cause effects to
		the target

	8.
		If this was timed, CastedSpellFinished() will restore the client's
		spell bar gems.


	Most user code should call CastSpell(), with a 0 casting time if needed,
	and not SpellFinished().

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
#ifndef WIN32
//	#include <pthread.h>
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
#if !defined(NEW_LoadSPDat) && !defined(DB_LoadSPDat)
	extern SPDat_Spell_Struct spells[SPDAT_RECORDS];
#endif
extern bool spells_loaded;
extern WorldServer worldserver;
uchar blah[]={0x0D,0x00,0x00,0x00,0x01,0x00,0x00,0x00};
uchar blah2[]={0x12,0x00,0x00,0x00,0x16,0x01,0x00,0x00};


// solar: this is run constantly for every mob
void Mob::SpellProcess()
{
	if(casting_spell)
	{
		if(casting_spell->IsCastTimerFinished())
		{
			casting_spell->StopCastTimer();
			CastedSpellFinished(&casting_spell);
		}
	}

	if(spell_recovery_timer)
	{
		if(spell_recovery_timer->Check(false))
		{
			safe_delete(spell_recovery_timer);
		}
	}

	if(buff_tic_timer)
	{
		if(buff_tic_timer->Check())
		{
			BuffProcess();
		}
	}
}

void NPC::SpellProcess()
{
	Mob::SpellProcess();
	
	if(GetSwarmInfo()){
		if(!GetSwarmInfo()->owner)
		{
			Depop();
		}

		if(GetSwarmInfo()->duration->Check(false))
		{
			Depop();
		}

		Mob *targMob = entity_list.GetMob(GetSwarmInfo()->target);
		if(GetSwarmInfo()->target != 0)
        {
			if(!targMob || (targMob && targMob->IsCorpse()))
				Depop();
		}
	}
}

bool Mob::CastSpell(int16 spell_id, int16 target_id, int16 slot,
	sint32 cast_time, sint32 mana_cost, int32* spell_will_finish, int32 item_slot)
{
	if(!IsValidSpell(spell_id))
		return false;

	Spell *pass_spell = new Spell(spell_id, this, entity_list.GetMobID(target_id), slot, cast_time, mana_cost);
	pass_spell->SetInventorySpellSlot(item_slot);
	return CastSpell(&pass_spell, spell_will_finish);
}

bool Mob::CastSpell(Spell **casted_spell_ptr, int32* spell_will_finish)
{
	_ZP(Mob_CastSpell);
	Spell *casted_spell = *casted_spell_ptr;

	mlog(SPELLS__CASTING, "Mob::CastSpell called for %s with spell (%d): %s on entity %s, slot %d, time %d, mana %d from item slot %d", GetName(), 
		casted_spell->GetSpellID(), spells[casted_spell->GetSpellID()].name, 
		casted_spell->GetTarget() != NULL ? casted_spell->GetTarget()->GetName() : "NULL TARGET", 
		casted_spell->GetSpellSlot(), casted_spell->GetCastTime(), casted_spell->GetManaCost(), casted_spell->GetInventorySpellSlot());

	uint32 spell_id = casted_spell->GetSpellID();
	if(!ValidateStartSpellCast(casted_spell))
	{
		safe_delete(*casted_spell_ptr);
		return false;
	}
	
	if(spell_recovery_timer)
	{
		Message(13, "You have not recovered...");
		safe_delete(*casted_spell_ptr);
		return false;
	}
	
	if(casted_spell->GetSpellSlot() < MAX_PP_MEMSPELL && !CheckFizzle(casted_spell))
	{
		int fizzle_msg = IsBardSong(casted_spell->GetSpellID()) ? MISS_NOTE : SPELL_FIZZLE;
		InterruptSpell(fizzle_msg, 0x121, casted_spell->GetSpellID());
		
		uint32 use_mana = ((spells[casted_spell->GetSpellID()].mana) / 4);
		mlog(SPELLS__CASTING_ERR, "Spell casting canceled: fizzled. %d mana has been consumed", use_mana);
		
		// fizzle 1/4 the mana away
		SetMana(GetMana() - use_mana);
		safe_delete(*casted_spell_ptr);
		return false;
	}

	if (HasActiveSong()) 
	{
		mlog(SPELLS__BARDS, "Casting a new spell/song while singing a song. Killing old song %d.", bard_song->GetSpellID());
        _StopSong();
    }

	/*------------------------------
	Added to prevent MQ2 
	exploitation of equipping 
	normally-unequippable items 
	with effects and clicking them
	for benefits. - ndnet
	---------------------------------*/
	if(casted_spell->GetInventorySpellSlot() && IsClient() && ((casted_spell->GetSpellSlot() == USE_ITEM_SPELL_SLOT) || (casted_spell->GetSpellSlot() == POTION_BELT_SPELL_SLOT)))
	{
		ItemInst *itm = CastToClient()->GetInv().GetItem(casted_spell->GetInventorySpellSlot());
		int bitmask = 1;
		bitmask = bitmask << (CastToClient()->GetClass() - 1);
		if( itm && itm->GetItem()->Classes != 65535 && (itm->GetItem()->Click.Type == ET_EquipClick) && !( itm->GetItem()->Classes & bitmask ) ){
			// They are casting a spell on an item that requires equipping but shouldn't let them equip it
			LogFile->write(EQEMuLog::Error, "HACKER: %s (account: %s) attempted to click an equip-only effect on item %s (id: %d) which they shouldn't be able to equip!", CastToClient()->GetCleanName(), CastToClient()->AccountName(), itm->GetItem()->Name, itm->GetItem()->ID);
			database.SetHackerFlag(CastToClient()->AccountName(), CastToClient()->GetCleanName(), "Clicking equip-only item with an invalid class");
			safe_delete(*casted_spell_ptr);
			return false;
		}
	}
	return(DoCastSpell(casted_spell_ptr, spell_will_finish));
}

bool Mob::DoCastSpell(Spell **casted_spell_ptr, int32* spell_will_finish)
{
	_ZP(Mob_DoCastSpell);
	Spell *casted_spell = *casted_spell_ptr;
	sint32 orig_cast_time;

	SaveSpellLoc();
	mlog(SPELLS__CASTING, "Casting %d Started at (%.3f,%.3f,%.3f)", casted_spell->GetSpellID(), spell_x, spell_y, spell_z);

	if(casted_spell->GetCastTime() <= -1)
	{
		sint32 new_cast_time = orig_cast_time = casted_spell->GetSpell().cast_time;
		if(new_cast_time > 0)
		{
			new_cast_time = GetActSpellCasttime(casted_spell, new_cast_time);
		}
		casted_spell->SetCastTime(new_cast_time);
	}
	else
	{
		orig_cast_time = casted_spell->GetCastTime();
	}

	if(casted_spell->GetManaCost() <= -1) 
	{
		sint32 new_mana_cost = casted_spell->GetSpell().mana;
		new_mana_cost = GetActSpellCost(casted_spell, new_mana_cost);
		casted_spell->SetManaCost(new_mana_cost);
	}

	if(IsClient() && CastToClient()->CheckAAEffect(aaEffectMassGroupBuff) && casted_spell->IsGroupSpell())
	{
		casted_spell->SetManaCost(casted_spell->GetManaCost() * 2);
	}

	if(casted_spell->GetManaCost() > 0 && casted_spell->GetSpellSlot() != 10)
	{
		sint32 current_mana = GetMana();
		sint32 max_mana = GetMaxMana();
		if(current_mana < casted_spell->GetSpell().mana)
		{
			//this is a special case for NPCs with no mana...
			if(IsNPC() && current_mana == max_mana)
			{
				casted_spell->SetManaCost(0);
			} 
			else 
			{
				mlog(SPELLS__CASTING_ERR, "Spell Error not enough mana spell=%d my mana=%d cost=%d\n", GetName(), casted_spell->GetSpellID(), current_mana, casted_spell->GetSpell().mana);
				if(IsClient()) 
				{
					Message_StringID(13, INSUFFICIENT_MANA);
					InterruptSpell();
				} 
				else 
				{
					InterruptSpell(0, 0, 0);	//the 0 args should cause no messages
				}

				safe_delete(*casted_spell_ptr);
				return false;
			}
		}
	}

	if(casted_spell->GetManaCost() > GetMana())
	{
		casted_spell->SetManaCost(GetMana());
	}
	

	if((casted_spell->IsGroupSpell() || 
		casted_spell->GetSpell().targettype == ST_Self ||
		casted_spell->GetSpell().targettype == ST_AECaster ||
		casted_spell->GetSpell().targettype == ST_TargetOptional) && casted_spell->GetTarget() == NULL)
	{
		mlog(SPELLS__CASTING, "Spell %d auto-targeted the caster. Group? %d, target type %d", casted_spell->GetSpellID(), casted_spell->IsGroupSpell(), casted_spell->GetSpell().targettype);
		casted_spell->SetTarget(this);
	}

	// we checked for spells not requiring targets above
	if(casted_spell->GetTarget() == NULL) 
	{
		mlog(SPELLS__CASTING_ERR, "Spell Error: no target. spell=%d\n", GetName(), casted_spell->GetSpellID());
		if(IsClient()) 
		{
			Message_StringID(13, SPELL_NEED_TAR);
			InterruptSpell();
		} 
		else 
		{
			InterruptSpell(0, 0, 0);
		}

		safe_delete(*casted_spell_ptr);
		return false;
	}

	if(casted_spell->GetCastTime() != 0)
	{
		casted_spell->StartCastTimer(casted_spell->GetCastTime());
	}

	if(IsAIControlled())
	{
		SetRunAnimSpeed(0);
		Mob *sp_tar = casted_spell->GetTarget();
		if(this != sp_tar)
		{
			FaceTarget(sp_tar);
		}
	}

	if(spell_will_finish)
	{
		*spell_will_finish = Timer::GetCurrentTime() + casted_spell->GetCastTime() + 100;
	}

	mlog(SPELLS__CASTING, "Sending spell casting packet...");
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_BeginCast, sizeof(BeginCast_Struct));
	BeginCast_Struct* begin_cast = (BeginCast_Struct*)outapp->pBuffer;
	begin_cast->caster_id = GetID();
	begin_cast->spell_id = casted_spell->GetSpellID();
	begin_cast->cast_time = casted_spell->GetCastTime();
	outapp->priority = 3;
	entity_list.QueueCloseClients(this, outapp, false, 200, 0, true);
	safe_delete(outapp);

	if(casted_spell->GetCastTime() == 0)
	{
		CastedSpellFinished(&casted_spell);
		safe_delete(*casted_spell_ptr);
		return true;
	}

	casting_spell = casted_spell;
	return true;
}

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
	
	if (bard_song || (casting_spell ? casting_spell->IsBardSong() : 0))
	{
		_StopSong();
	}
	
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

void Mob::CastedSpellFinished(Spell **casted_spell_ptr)
{
	_ZP(Mob_CastedSpellFinished);
	Spell *casted_spell = *casted_spell_ptr;

	if(IsClient() && casted_spell->GetSpellSlot() != USE_ITEM_SPELL_SLOT && casted_spell->GetSpellSlot() != POTION_BELT_SPELL_SLOT && casted_spell->GetSpell().recast_time > 1000) 
	{
		if(!CastToClient()->GetPTimers().Expired(&database, pTimerSpellStart + casted_spell->GetSpellID(), false)) 
		{
			Message_StringID(13, SPELL_RECAST);
			mlog(SPELLS__CASTING_ERR, "Casting of %d canceled: spell reuse timer not expired", casted_spell->GetSpellID());
			InterruptSpell();
			return;
		}
	}

	if(IsClient() && ((casted_spell->GetSpellSlot() == USE_ITEM_SPELL_SLOT) || (casted_spell->GetSpellSlot() == POTION_BELT_SPELL_SLOT)))
	{
		ItemInst *itm = CastToClient()->GetInv().GetItem(casted_spell->GetInventorySpellSlot());
		if(itm && itm->GetItem()->RecastDelay > 0)
		{
			if(!CastToClient()->GetPTimers().Expired(&database, (pTimerItemStart + itm->GetItem()->RecastType), false)) 
			{
				Message_StringID(13, SPELL_RECAST);
				mlog(SPELLS__CASTING_ERR, "Casting of %d canceled: item spell reuse timer not expired", casted_spell->GetSpellID());
				InterruptSpell();
				return;
			}
		}
	}

	bool bard_song_mode = false;
	bool regain_conc = false;
	Mob *spell_target = casted_spell->GetTarget();

	// here we do different things if this is a bard casting a bard song from
	// a spell bar slot
	if(GetClass() == BARD) // bard's can move when casting any spell...
	{
		if (IsBardSong(casted_spell->GetSpellID())) 
		{
			if(casted_spell->GetSpell().buffduration == 0xFFFF || casted_spell->GetSpell().recast_time != 0) 
			{
				mlog(SPELLS__BARDS, "Bard song %d not applying bard logic because duration or recast is wrong: dur=%d, recast=%d", casted_spell->GetSpell().buffduration, casted_spell->GetSpell().recast_time);
			} 
			else
			{
				//Set our bard song...
				bard_song = casted_spell->CopySpell();
				bardsong_timer.Start(6000);
				mlog(SPELLS__BARDS, "Bard song %d started: slot %d", bard_song->GetSpellID(), bard_song->GetSpellSlot());
				bard_song_mode = true;
			}
		}
	}
	else // not bard, check channeling
	{
		if(!DoChannelCheck(regain_conc))
		{
			return;
		}
	}

	if(!DoComponentCheck(casted_spell, bard_song_mode))
	{
		return;
	}

	if(IsClient() && ((casted_spell->GetSpellSlot() == USE_ITEM_SPELL_SLOT) || (casted_spell->GetSpellSlot() == POTION_BELT_SPELL_SLOT)) && casted_spell->GetInventorySpellSlot() != 0xFFFFFFFF)
	{
		const ItemInst* inst = CastToClient()->GetInv()[casted_spell->GetInventorySpellSlot()];
		if (inst && inst->IsType(ItemClassCommon))
		{
			//const Item_Struct* item = inst->GetItem();
			sint16 charges = inst->GetItem()->MaxCharges;
			if(charges > -1) 
			{	// charged item, expend a charge
				mlog(SPELLS__CASTING, "Spell %d: Consuming a charge from item %s (%d) which had %d/%d charges.", casted_spell->GetSpell(), inst->GetItem()->Name, inst->GetItem()->ID, inst->GetCharges(), inst->GetItem()->MaxCharges);
				CastToClient()->DeleteItemInInventory(casted_spell->GetInventorySpellSlot(), 1, true);
			} 
			else 
			{
				mlog(SPELLS__CASTING, "Spell %d: Cast from unlimited charge item %s (%d) (%d charges)", casted_spell->GetSpell(), inst->GetItem()->Name, inst->GetItem()->ID, inst->GetItem()->MaxCharges);
			}
		}
		else
		{
			mlog(SPELLS__CASTING_ERR, "Item used to cast spell %d was missing from inventory slot %d after casting!", casted_spell->GetSpell(), casted_spell->GetInventorySpellSlot());
			Message(0, "Error: item not found for inventory slot #%i", casted_spell->GetInventorySpellSlot());
			InterruptSpell();
			return;
		}
	}

	//This casting_spell_finished stuff doesn't seem to make much sense on the
	//outside but it's extremely important for the proper function of spell 
	//effects with stuns in them.
	casting_spell_finished = casting_spell;
	casting_spell = NULL;
	if(!SpellFinished(casted_spell))
	{
		mlog(SPELLS__CASTING_ERR, "Casting of %d canceled: SpellFinished returned false.", casted_spell->GetSpellID());
		//The above block disabled interrupts, lets re-enable it for one last one.
		casting_spell = casting_spell_finished;
		casting_spell_finished = NULL;
		InterruptSpell();
		return;
	}

	#ifdef EMBPERL
	if(this->IsClient()) 
	{
		if(((PerlembParser*)parse)->PlayerHasQuestSub("EVENT_CAST") ) {
			char temp[64];
			sprintf(temp, "%d", casted_spell->GetSpellID());
			((PerlembParser*)parse)->Event(EVENT_CAST, 0, temp, (NPC*)NULL, CastToClient());
		}
	}
	#endif

	uint32 spell_slot = casted_spell->GetSpellSlot();
	uint32 spell_id = casted_spell->GetSpellID();
	SkillType spell_skill = casted_spell->GetSpell().skill;

	if(bard_song_mode)
	{
		if(IsClient())
		{
			CastToClient()->CheckSongSkillIncrease(spell_id);
		}

		mlog(SPELLS__CASTING, "Bard song %d should be started", spell_id);
		ZeroAndFreeCastingVars();
	}
	else
	{
		if(IsClient())
		{
			Client *c = CastToClient();
			SendSpellBarEnable(spell_id);
			c->MemorizeSpell(spell_slot, spell_id, memSpellSpellbar);
			SetMana(GetMana());

			// skills
			if(spell_slot < MAX_PP_MEMSPELL)
			{
				c->CheckIncreaseSkill(spell_skill, NULL);
				c->CheckIncreaseSkill(CHANNELING, NULL, regain_conc ? 5 : 0);
				c->CheckSpecializeIncrease(spell_id);	
			}
		}
		spell_recovery_timer = new Timer(100);

		// there should be no casting going on now
		ZeroAndFreeCastingVars();

		//TODO:
		// set the rapid recast timer for next time around

		mlog(SPELLS__CASTING, "Spell casting of %d is finished.", spell_id);
	}
}

bool Mob::DetermineSpellTargets(Spell *spell_to_cast, Mob *&spell_target, Mob *&ae_center, CastAction_type &CastAction) 
{
	/*
	solar: The basic types of spells:
	
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
		{
			if(!spell_target || spell_target->GetBodyType() != target_bt)
			{
				//invalid target
				mlog(SPELLS__CASTING_ERR, "Spell %d canceled: invalid target of body type %d (want body Type %d)", spell_to_cast->GetSpellID(), spell_target->GetBodyType(), target_bt);
				Message_StringID(13,SPELL_NEED_TAR);
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

		case ST_UndeadAE:	//should only affect undead...
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


bool Mob::SpellFinished(int16 spell_id, Mob *target, int16 slot, int16 mana_used, int32 inventory_slot)
{
	bool return_value;
	Spell *new_spell = new Spell(spell_id, this, target, slot, -1, mana_used);

	return_value = SpellFinished(new_spell);
	safe_delete(new_spell);

	return return_value;
}

bool Mob::SpellFinished(Spell *spell_to_cast)
{
	_ZP(Mob_SpellFinished);
	if(spell_to_cast->GetSpell().zonetype == 1 && !zone->CanCastOutdoor())
	{
		if(IsClient())
		{
			if(!CastToClient()->GetGM())
			{
				Message_StringID(13, CAST_OUTDOORS);
				return false;
			}
		}
	}

	if(spell_to_cast->IsEffectInSpell(SE_Levitate) && !zone->CanLevitate())
	{
		if(IsClient())
		{
			if(!CastToClient()->GetGM())
			{
				Message(13, "You can't levitate in this zone.");
				return false;
			}
		}
	}

	if(IsClient() && !CastToClient()->GetGM())
	{
		if(zone->IsSpellBlocked(spell_to_cast->GetSpellID(), GetX(), GetY(), GetZ()))
		{
			const char *msg = zone->GetSpellBlockedMessage(spell_to_cast->GetSpellID(), GetX(), GetY(), GetZ());
			if(msg)
			{
				Message(13, msg);
				return false;
			}
			else
			{
				Message(13, "You can't cast this spell here.");
				return false;
			}
		}
	}
		
	if(IsClient() && (zone->GetZoneID() == 183 || zone->GetZoneID() == 184) && CastToClient()->Admin() < 80)
	{
		if(spell_to_cast->IsEffectInSpell(SE_Gate) || spell_to_cast->IsEffectInSpell(SE_Translocate) ||	spell_to_cast->IsEffectInSpell(SE_Teleport))
		{
			Message(0, "The Gods brought you here, only they can send you away.");
			return false;
		}
	}

	Mob *ae_center = NULL;
	Mob *spell_target = spell_to_cast->GetTarget();
	CastAction_type CastAction;
	if(!DetermineSpellTargets(spell_to_cast, spell_target, ae_center, CastAction))
	{
		return(false);
	}
	
	mlog(SPELLS__CASTING, "Spell %d: target type %d, target %s, AE center %s", spell_to_cast->GetSpellID(), CastAction, spell_target ? spell_target->GetName() : "NONE", ae_center ? ae_center->GetName() : "NONE");
	
	if(spell_to_cast->IsAEDurationSpell()) 
	{
		// solar: the spells are AE target, but we aim them on a beacon
		Mob *beacon_loc =  spell_target ? spell_target : this;
		Beacon *beacon = new Beacon(beacon_loc, spell_to_cast->GetSpell().AEDuration);
		entity_list.AddBeacon(beacon);
		mlog(SPELLS__CASTING, "Spell %d: AE duration beacon created, entity id %d", spell_to_cast->GetSpellID(), beacon->GetName());
		spell_target = NULL;
		ae_center = beacon;
		CastAction = AECaster;
	}
	
	if(spell_target && spell_to_cast->IsDetrimentalSpell() && !CheckLosFN(spell_target) && !spell_to_cast->IsHarmonySpell())
	{
		mlog(SPELLS__CASTING, "Spell %d: cannot see target %s", spell_target->GetName());
		Message_StringID(13,CANT_SEE_TARGET);
		return false;
	}
	
	if(spell_target && spell_to_cast->IsManaTapSpell()) 
	{
		if(spell_target->GetCasterClass() == 'N') 
		{
			Message_StringID(13, TARGET_NO_MANA);
			return false;
		}
	}

	float range = spell_to_cast->GetSpell().range;
	if(IsClient() && CastToClient()->TGB() && spell_to_cast->IsTGBCompatibleSpell() && spell_to_cast->IsGroupSpell())
	{
		range = spell_to_cast->GetSpell().aoerange;
	}

	range = GetActSpellRange(spell_to_cast, range);


	if(spell_to_cast->IsPlayerIllusionSpell()
		&& IsClient()
		&& CastToClient()->CheckAAEffect(aaEffectProjectIllusion))
	{
		range = 100;
	}

	if(spell_target != NULL && spell_target != this) 
	{
		//casting a spell on somebody but ourself, make sure they are in range
		float dist2 = DistNoRoot(*spell_target);
		float range2 = range * range;
		if(dist2 > range2) {
			//target is out of range.
			mlog(SPELLS__CASTING, "Spell %d: Spell target is out of range (squared: %f > %f)", spell_to_cast->GetSpellID(), dist2, range2);
			Message_StringID(13, TARGET_OUT_OF_RANGE);
			return false;
		}
	}

	switch(CastAction)
	{
		default:
		case CastActUnknown:
		case SingleTarget:
		{
			if(spell_target == NULL) 
			{
				mlog(SPELLS__CASTING, "Spell %d: Targeted spell, but we have no target", spell_to_cast->GetSpellID());
				return(false);
			}
			SpellOnTarget(spell_to_cast, spell_target);

			if(spell_to_cast->IsPlayerIllusionSpell()
			&& IsClient()
			&& CastToClient()->CheckAAEffect(aaEffectProjectIllusion)){
				mlog(AA__MESSAGE, "Effect Project Illusion for %s on spell id: %d was ON", GetName(), spell_to_cast->GetSpellID());
				CastToClient()->DisableAAEffect(aaEffectProjectIllusion);
			}
			else{
				mlog(AA__MESSAGE, "Effect Project Illusion for %s on spell id: %d was OFF", GetName(), spell_to_cast->GetSpellID());
			}
			break;
		}

		case AECaster:
		case AETarget:
		{
			// we can't cast an AE spell without something to center it on
			assert(ae_center != NULL);

			if(ae_center->IsBeacon()) {
				// special ae duration spell
				ae_center->CastToBeacon()->AELocationSpell(this, spell_to_cast);
			} else {
				// regular PB AE or targeted AE spell - spell_target is null if PB
				if(spell_target)	// this must be an AETarget spell
				{
					// affect the target too
					SpellOnTarget(spell_to_cast, spell_target);
				}

				if(ae_center && ae_center == this && spell_to_cast->IsBeneficialSpell())
				{
					SpellOnTarget(spell_to_cast, this);
				}
				
				bool affect_caster = !IsNPC();	//NPC AE spells do not affect the NPC caster
				entity_list.AESpell(this, ae_center, spell_to_cast, affect_caster);
			}
			break;
		}

		case GroupSpell:
		{
			if(IsClient() && CastToClient()->CheckAAEffect(aaEffectMassGroupBuff))
			{
				SpellOnTarget(spell_to_cast, this);
				entity_list.AESpell(this, this, spell_to_cast, true);
				CastToClient()->DisableAAEffect(aaEffectMassGroupBuff);
			}
			else
			{
				// at this point spell_target is a member of the other group, or the
				// caster if they're not using TGB
				// NOTE: this will always hit the caster, plus the target's group so
				// it can affect up to 7 people if the targeted group is not our own
				if(spell_target->IsGrouped())
				{
					Group *target_group = entity_list.GetGroupByMob(spell_target);
					if(target_group)
					{
						target_group->CastGroupSpell(this, spell_to_cast);
					}
				}
				else if(spell_target->IsRaidGrouped() && spell_target->IsClient())
				{
					Raid *target_raid = entity_list.GetRaidByClient(spell_target->CastToClient());
					int32 gid = 0xFFFFFFFF;
					if(target_raid){
						gid = target_raid->GetGroup(spell_target->GetName());
						if(gid < 12)
						{
							target_raid->CastGroupSpell(this, spell_to_cast, gid);
						}
						else
						{
							SpellOnTarget(spell_to_cast, spell_target);
						}
					}
				}
				else
				{
					// if target is grouped, CastGroupSpell will cast it on the caster
					// too, but if not then we have to do that here.
					if(spell_target != this){
						SpellOnTarget(spell_to_cast, this);
	#ifdef GROUP_BUFF_PETS
						//pet too
						if (GetPet() && GetAA(aaPetAffinity) && !GetPet()->IsCharmed())
						{
							SpellOnTarget(spell_to_cast, GetPet());
						}
	#endif					
					}

					SpellOnTarget(spell_to_cast, spell_target);
	#ifdef GROUP_BUFF_PETS
					//pet too
					if (spell_target->GetPet() && GetAA(aaPetAffinity) && !spell_target->GetPet()->IsCharmed())
					{
						SpellOnTarget(spell_to_cast, spell_target->GetPet());
					}
	#endif
				}
			}
			break;
		}
	}

	DoAnim(spell_to_cast->GetSpell().CastingAnim, 0, true, IsClient() ? FILTER_PCSPELLS : FILTER_NPCSPELLS);

	// if this was a spell slot or an ability use up the mana for it
	// CastSpell already reduced the cost for it if we're a client with focus
	if(spell_to_cast->GetSpellSlot() != USE_ITEM_SPELL_SLOT  && spell_to_cast->GetSpellSlot() != POTION_BELT_SPELL_SLOT && spell_to_cast->GetManaCost() > 0)
	{
		mlog(SPELLS__CASTING, "Spell %d: consuming %d mana", spell_to_cast->GetSpellID(), spell_to_cast->GetManaCost());
		SetMana(GetMana() - spell_to_cast->GetManaCost());
	}

	if(IsClient())
	{
		uint32 recast_time = 0;
		uint32 recast_timer = 0;
		if(spell_to_cast->GetTimerID() > 0)
		{
			recast_timer = spell_to_cast->GetTimerID();
			recast_time = spell_to_cast->GetTimerIDDuration();
		}
		else
		{
			if((spell_to_cast->GetSpellSlot() == USE_ITEM_SPELL_SLOT) || (spell_to_cast->GetSpellSlot() == POTION_BELT_SPELL_SLOT))
			{
				ItemInst *itm = CastToClient()->GetInv().GetItem(spell_to_cast->GetInventorySpellSlot());
				if(itm && itm->GetItem()->RecastDelay > 0)
				{
					recast_timer = pTimerItemStart + itm->GetItem()->RecastType;
					recast_time = itm->GetItem()->RecastDelay;
				}
			}
			else
			{
				recast_timer = pTimerSpellStart + spell_to_cast->GetSpellID();
				recast_time = spell_to_cast->GetSpell().recast_time / 1000;
			}
		}

		if(recast_time > 0)
		{
			CastToClient()->GetPTimers().Start(recast_timer, recast_time);
			if(spell_to_cast->GetSpellType() == SC_AA)
			{
				time_t timestamp = time(NULL);
				CastToClient()->SendAATimer(recast_timer-pTimerAAStart, timestamp, timestamp);
			}
			else if(spell_to_cast->GetSpellType() == SC_DISC)
			{
				if(spell_to_cast->GetSpell().EndurTimerIndex < MAX_DISCIPLINE_TIMERS)
				{
					EQApplicationPacket *disc_packet = new EQApplicationPacket(OP_DisciplineTimer, sizeof(DisciplineTimer_Struct));
					DisciplineTimer_Struct *dts = (DisciplineTimer_Struct *)disc_packet->pBuffer;
					dts->TimerID = spell_to_cast->GetSpell().EndurTimerIndex;
					dts->Duration = spell_to_cast->GetSpell().recast_time / 1000;
					CastToClient()->QueuePacket(disc_packet);
					safe_delete(disc_packet);
				}
			}
		}
	}

	if(IsNPC())
		CastToNPC()->AI_Event_SpellCastFinished(true, spell_to_cast->GetSpellSlot());

	if (IsClient() && CastToClient()->MelodyIsActive()) {
		Client *c = CastToClient();
		if (spell_to_cast->IsBardSong()) {
			c->InterruptSpell(0, 0, 0);
			c->MelodyAdvanceSong();
			c->MelodyTrySong();
		}
	}

	return true;	
}

/*
 * handle bard song pulses...
 * 
 * we make several assumptions that SpellFinished does not:
 *   - there are no AEDuration (beacon) bard songs
 *   - there are no recourse spells on bard songs
 *   - there is no long recast delay on bard songs
 * 
 * return false to stop the song
 */
//TODO: FIXME
bool Mob::ApplyNextBardPulse(Spell *spell_to_cast) 
{
	Mob *spell_target = spell_to_cast->GetTarget();

	if(spell_to_cast->GetSpellSlot() == USE_ITEM_SPELL_SLOT) 
	{
		//bard songs should never come from items...
		mlog(SPELLS__BARDS, "Bard Song Pulse %d: Cast from an item, this is illegal. Killing song.", spell_to_cast->GetSpellID());
		return false;
	}
	
	//determine the type of spell target we have
	Mob *ae_center = NULL;
	CastAction_type CastAction;
	if(!DetermineSpellTargets(spell_to_cast, spell_target, ae_center, CastAction)) 
	{
		mlog(SPELLS__BARDS, "Bard Song Pulse %d: was unable to determine target. Stopping.", spell_to_cast->GetSpellID());
		return false;
	}
	
	if(ae_center != NULL && ae_center->IsBeacon())
	{
		mlog(SPELLS__BARDS, "Bard Song Pulse %d: Unsupported Beacon NPC AE spell", spell_to_cast->GetSpellID());
		return false;
	}
	
	//use mana, if this spell has a mana cost
	int mana_used = spell_to_cast->GetSpell().mana;
	if(mana_used > 0) 
	{
		if(mana_used > GetMana()) 
		{
			//ran out of mana... this calls StopSong() for us
			mlog(SPELLS__BARDS, "Ran out of mana while singing song %d", spell_to_cast->GetSpellID());
			return false;
		}
		
		mlog(SPELLS__CASTING, "Bard Song Pulse %d: consuming %d mana (have %d)", spell_to_cast->GetSpellID(), mana_used, GetMana());
		SetMana(GetMana() - mana_used);
	}
	
	
	// check line of sight to target if it's a detrimental spell
	if(spell_target && spell_to_cast->IsDetrimentalSpell() && !CheckLosFN(spell_target))
	{
		mlog(SPELLS__CASTING, "Bard Song Pulse %d: cannot see target %s", spell_to_cast->GetSpellID(), spell_target->GetName());
		Message_StringID(13, CANT_SEE_TARGET);
		return false;
	}
	
	//range check our target, if we have one and it is not us
	if(spell_target != NULL && spell_target != this) 
	{
		float range = GetActSpellRange(spell_to_cast, spell_to_cast->GetSpell().range);
		range *= range;
		float dist = DistNoRoot(*spell_target);
		if(dist > range) 
		{
			//target is out of range.
			mlog(SPELLS__BARDS, "Bard Song Pulse %d: Spell target is out of range (squared: %f > %f)", spell_to_cast->GetSpellID(), dist, range);
			Message_StringID(13, TARGET_OUT_OF_RANGE);
			return false;
		}
	}
	
	switch(CastAction)
	{
		default:
		case CastActUnknown:
		case SingleTarget:
		{
			if(spell_target == NULL) 
			{
				mlog(SPELLS__BARDS, "Bard Song Pulse %d: Targeted spell, but we have no target", spell_to_cast->GetSpellID());
				return false;
			}
			mlog(SPELLS__BARDS, "Bard Song Pulse: Targeted. spell %d, target %s", spell_to_cast->GetSpellID(), spell_target->GetName());
			spell_target->BardPulse(spell_to_cast, this);
			break;
		}

		case AECaster:
		{
			if(spell_to_cast->IsBeneficialSpell())
				SpellOnTarget(spell_to_cast, this);
			
			bool affect_caster = !IsNPC();	//NPC AE spells do not affect the NPC caster
			entity_list.AEBardPulse(this, this, spell_to_cast, affect_caster);
			break;		
		}
		case AETarget:
		{
			// we can't cast an AE spell without something to center it on
			if(ae_center == NULL) 
			{
				mlog(SPELLS__BARDS, "Bard Song Pulse %d: AE Targeted spell, but we have no target", spell_to_cast->GetSpellID());
				return(false);
			}

			// regular PB AE or targeted AE spell - spell_target is null if PB
			if(spell_target) 
			{	
				// this must be an AETarget spell
				// affect the target too
				spell_target->BardPulse(spell_to_cast, this);
				mlog(SPELLS__BARDS, "Bard Song Pulse: spell %d, AE target %s", spell_to_cast->GetSpellID(), spell_target->GetName());
			} 
			else 
			{
				mlog(SPELLS__BARDS, "Bard Song Pulse: spell %d, AE with no target", spell_to_cast->GetSpellID());
			}
			bool affect_caster = !IsNPC();	//NPC AE spells do not affect the NPC caster
			entity_list.AEBardPulse(this, ae_center, spell_to_cast, affect_caster);
			break;
		}

		case GroupSpell:
		{
			if(spell_target->IsGrouped()) 
			{
				mlog(SPELLS__BARDS, "Bard Song Pulse: spell %d, Group targeting group of %s", spell_to_cast->GetSpellID(), spell_target->GetName());
				Group *target_group = spell_target->GetGroup();
				if(target_group)
					target_group->GroupBardPulse(this, spell_to_cast);
			}
			else if(spell_target->IsRaidGrouped() && spell_target->IsClient()) 
			{
				mlog(SPELLS__BARDS, "Bard Song Pulse: spell %d, Raid group targeting raid group of %s", spell_to_cast->GetSpellID(), spell_target->GetName());
				Raid *r = spell_target->CastToClient()->GetRaid();
				if(r)
				{
					int32 gid = r->GetGroup(spell_target->GetName());
					if(gid < 12)
					{
						r->GroupBardPulse(this, spell_to_cast, gid);
					}
					else
					{
						BardPulse(spell_to_cast, this);
#ifdef GROUP_BUFF_PETS
						if (GetPet() && GetAA(aaPetAffinity) && !GetPet()->IsCharmed())
							GetPet()->BardPulse(spell_to_cast, this);
#endif
					}
				}
			}
			else 
			{
				mlog(SPELLS__BARDS, "Bard Song Pulse: spell %d, Group target without group. Affecting caster.", spell_to_cast->GetSpellID());
				BardPulse(spell_to_cast, this);
#ifdef GROUP_BUFF_PETS
				if (GetPet() && GetAA(aaPetAffinity) && !GetPet()->IsCharmed())
					GetPet()->BardPulse(spell_to_cast, this);
#endif
			}
			break;
		}
	}
	
	//TODO:
	//do we need to do this???
	DoAnim(spell_to_cast->GetSpell().CastingAnim, 0, true, IsClient() ? FILTER_PCSPELLS : FILTER_NPCSPELLS);
	if(IsClient())
		CastToClient()->CheckSongSkillIncrease(spell_to_cast->GetSpellID());

	return(true);
}

void Mob::BardPulse(Spell *spell_to_cast, Mob *caster) 
{
	Buff *found_spell = NULL;
	uint32 buff_index = 0xFFFFFFFF;
	int max_slots = GetMaxTotalSlots();
	for(int i = 0; i < max_slots; i++)
	{
		if(buffs[i])
		{
			Buff *current_buff = buffs[i];
			if(current_buff->GetSpell()->GetSpellID() != spell_to_cast->GetSpellID())
			{
				continue;
			}

			if(current_buff->GetSpell()->GetCasterID() != spell_to_cast->GetCasterID())
			{
				continue;
			}

			found_spell = current_buff;
			buff_index = i;
			break;
		}
	}

	if(found_spell)
	{
		found_spell->SetInstrumentMod(caster->GetInstrumentMod(spell_to_cast));

		//Add 3 tics to the buff.
		found_spell->SetDurationRemaining(found_spell->GetDurationRemaining() + 3);
		SendBuffPacket(found_spell, buff_index, 3);
	}

	//Knockback
	if(!spell_to_cast->IsEffectInSpell(SE_TossUp))
	{
		if(spell_to_cast->GetSpell().pushback > 0 || spell_to_cast->GetSpell().pushup > 0)
		{
			SendKnockBackPacket(spell_to_cast->GetSpell().pushback, spell_to_cast->GetSpell().pushup);
		}
	}

	//Send the action packet to everyone
	int seq = SendActionSpellPacket(spell_to_cast, this, caster->GetCasterLevel());

	//Do the spell on target
	caster->SpellOnTarget(spell_to_cast, this);
}

///////////////////////////////////////////////////////////////////////////////
// buff related functions

// solar: returns how many _ticks_ the buff will last.
// a tick is 6 seconds
// this is the place to figure out random duration buffs like fear and charm.
// both the caster and target mobs are passed in, so different behavior can
// even be created depending on the types of mobs involved
//
// right now this is just an outline, working on this..
int Mob::CalcBuffDuration(Mob *caster, Mob *target, const Spell *spell_to_cast, sint32 caster_level_override)
{
	int formula, duration;

	if(!caster && !target)
		return 0;
	
	// if we have at least one, we can make do, we'll just pretend they're the same
	if(!caster)
		caster = target;
	if(!target)
		target = caster;

	formula = spell_to_cast->GetSpell().buffdurationformula;
	duration = spell_to_cast->GetSpell().buffduration;

	//add one tic because we seem to fade at least one tic too soon
	int castlevel = caster->GetCasterLevel();
	if(caster_level_override > 0)
		castlevel = caster_level_override;

	int res = 1 + CalcBuffDuration_formula(castlevel, formula, duration);
	mlog(SPELLS__CASTING, "Spell %d: Casting level %d, formula %d, base_duration %d: result %d",
		spell_to_cast->GetSpellID(), castlevel, formula, duration, res);

	// enchanter mesmerization mastery aa
	if (caster->IsClient() && caster->CastToClient()->GetAA(aaMesmerizationMastery) > 0)
		res *= 1.35;

	return(res);
}

// the generic formula calculations
int CalcBuffDuration_formula(int level, int formula, int duration)
{
	int i;	// temp variable

	switch(formula)
	{
		case 0:	// not a buff
			return 0;

		case 1:	// solar: 2/7/04
			i = (int)ceil(level / 2.0f);
			return i < duration ? (i < 1 ? 1 : i) : duration;

		case 2:	// solar: 2/7/04
			i = (int)ceil(duration / 5.0f * 3);
			return i < duration ? (i < 1 ? 1 : i) : duration;

		case 3:	// solar: 2/7/04
			i = level * 30;
			return i < duration ? (i < 1 ? 1 : i) : duration;

		case 4:	// only used by 'LowerElement'
			return ((duration != 0) ? duration : 50);

		case 5:	// solar: 2/7/04
			i = duration;
			return i < 3 ? (i < 1 ? 1 : i) : 3;

		case 6:	// solar: 2/7/04
			i = (int)ceil(level / 2.0f);
			return i < duration ? (i < 1 ? 1 : i) : duration;

		case 7:	// solar: 2/7/04
			i = level;
			return (duration == 0) ? (i < 1 ? 1 : i) : duration;

		case 8:	// solar: 2/7/04
			i = level + 10;
			return i < duration ? (i < 1 ? 1 : i) : duration;

		case 9:	// solar: 2/7/04
			i = level * 2 + 10;
			return i < duration ? (i < 1 ? 1 : i) : duration;

		case 10:	// solar: 2/7/04
			i = level * 3 + 10;
			return i < duration ? (i < 1 ? 1 : i) : duration;

		case 11:	// solar: confirmed 2/7/04
			return duration;

		case 12:	// solar: 2/7/04
			return duration;

		case 50:	// solar: lucy says this is unlimited?
			return 72000;	// 5 days

		case 3600:
			return duration ? duration : 3600;

		default:
			LogFile->write(EQEMuLog::Debug, "CalcBuffDuration_formula: unknown formula %d", formula);
			return 0;
	}
}

// solar: helper function for AddBuff to determine stacking
// spellid1 is the spell already worn, spellid2 is the one trying to be cast
// returns:
// 0 if not the same type, no action needs to be taken
// 1 if spellid1 should be removed (overwrite)
// -1 if they can't stack and spellid2 should be stopped
//currently, a spell will not land if it would overwrite a better spell on any effect
//if all effects are better or the same, we overwrite, else we do nothing
int Mob::CheckStackConflict(const Spell* spell_1, const Spell *spell_2)
{
	const SPDat_Spell_Struct &sp1 = spell_1->GetSpell();
	const SPDat_Spell_Struct &sp2 = spell_2->GetSpell();
	
	int i, effect_1, effect_2, sp1_value, sp2_value;
	int blocked_effect, blocked_below_value, blocked_slot;
	int overwrite_effect, overwrite_below_value, overwrite_slot;

	mlog(SPELLS__STACKING, "Check Stacking on old %s (%d) @ lvl %d vs. new %s (%d) @ lvl %d", 
		sp1.name, sp1.id, spell_1->GetCasterLevel(), sp2.name, sp2.id, spell_2->GetCasterLevel());

	if(sp1.id == sp2.id)
	{
		if(sp1.id == 2751 || sp1.id == 2755)
		{
			mlog(SPELLS__STACKING, "Blocking spell because manaburn/lifeburn does not stack with itself");
			return -1;
		}
	}

	//resurrection effects wont count for overwrite/block stacking
	switch(sp1.id)
	{
	case 756:
	case 757:
	case 5249:
		return 0;
	}

	switch (sp2.id)
	{
	case 756:
	case 757:
	case 5249:
		return 0;
	}
	
	//One of these is a bard song and one isn't and they're both beneficial so they should stack.
	if(spell_1->IsBardSong() != spell_2->IsBardSong()) 
	{
		if(!spell_1->IsDetrimentalSpell() && !spell_2->IsDetrimentalSpell())
		{
			mlog(SPELLS__STACKING, "%s and %s are beneficial, and one is a bard song, no action needs to be taken", 
				sp1.name, sp2.name);
			return 0;
		}
	}


	//Check for special stacking block command in spell 1 against spell 2
	for(i = 0; i < EFFECT_COUNT; i++)
	{
		effect_1 = sp1.effectid[i];
		if(effect_1 == SE_StackingCommand_Block)
		{
			/*
			The logic here is if you're comparing the same spells they can't block each other
			from refreshing
			*/

			if(sp1.id == sp2.id)
				continue;

			blocked_effect = sp1.base[i];
			blocked_slot = sp1.formula[i] - 201;	//they use base 1 for slots, we use base 0
			blocked_below_value = sp1.max[i];
			
			if(sp2.effectid[blocked_slot] == blocked_effect)
			{
				sp2_value = CalcSpellEffectValue(spell_2, blocked_slot, spell_2->GetCasterLevel());
				
				mlog(SPELLS__STACKING, "%s (%d) blocks effect %d on slot %d below %d. New spell has value %d"
					" on that slot/effect. %s.", sp1.name, sp1.id, blocked_effect, blocked_slot, blocked_below_value, 
					sp2_value, (sp2_value < blocked_below_value) ? "Blocked" : "Not blocked");

				if(sp2_value < blocked_below_value)
				{
					mlog(SPELLS__STACKING, "Blocking spell because sp2_value < blocked_below_value");
					return -1;	// blocked
				}
			} 
			else 
			{
				mlog(SPELLS__STACKING, "%s (%d) blocks effect %d on slot %d below %d, but we do not have that"
					" effect on that slot. Ignored.",sp1.name, sp1.id, blocked_effect, blocked_slot, 
					blocked_below_value);
			}
		}
	}

	// check for special stacking overwrite in spell2 against effects in spell1
	for(i = 0; i < EFFECT_COUNT; i++)
	{
		effect_2 = sp2.effectid[i];
		if(effect_2 == SE_StackingCommand_Overwrite)
		{
			overwrite_effect = sp2.base[i];
			overwrite_slot = sp2.formula[i] - 201;	//they use base 1 for slots, we use base 0
			overwrite_below_value = sp2.max[i];
			if(sp1.effectid[overwrite_slot] == overwrite_effect)
			{
				sp1_value = CalcSpellEffectValue(spell_1, overwrite_slot, spell_1->GetCasterLevel());

				mlog(SPELLS__STACKING, "%s (%d) overwrites existing spell if effect %d on slot %d is below %d."
					" Old spell has value %d on that slot/effect. %s.", sp2.name, sp2.id, overwrite_effect, 
					overwrite_slot, overwrite_below_value, sp1_value, 
					(sp1_value < overwrite_below_value) ? "Overwriting" : "Not overwriting");
				
				if(sp1_value < overwrite_below_value)
				{
					mlog(SPELLS__STACKING, "Overwrite spell because sp1_value < overwrite_below_value");
					return 1;			// overwrite spell if its value is less
				}
			} 
			else
			{
				mlog(SPELLS__STACKING, "%s (%d) overwrites existing spell if effect %d on slot %d is below"
					" %d, but we do not have that effect on that slot. Ignored.",
					sp2.name, sp2.id, overwrite_effect, overwrite_slot, overwrite_below_value);

			}
		}
	}
	
	bool sp1_detrimental = spell_1->IsDetrimentalSpell();
	bool sp2_detrimental = spell_2->IsDetrimentalSpell();
	bool sp_det_mismatch;

	if(sp1_detrimental == sp2_detrimental)
		sp_det_mismatch = false;
	else
		sp_det_mismatch = true;
	
	// now compare matching effects
	// arbitration takes place if 2 spells have the same effect at the same
	// effect slot, otherwise they're stackable, even if it's the same effect
	bool will_overwrite = false;
	for(i = 0; i < EFFECT_COUNT; i++)
	{
		if(spell_1->IsBlankSpellEffect(i) ||spell_2->IsBlankSpellEffect(i))
			continue;

		effect_1 = sp1.effectid[i];
		effect_2 = sp2.effectid[i];

		//Effects which really aren't going to affect stacking.
		if(effect_1 == SE_CurrentHPOnce ||
			effect_1 == SE_CurseCounter	||
			effect_1 == SE_DiseaseCounter ||
			effect_1 == SE_PoisonCounter){
			continue;
			}

		
		//Quick check, are the effects the same, if so then
		//keep going else ignore it for stacking purposes.
		

		if(effect_1 != effect_2)
			continue;

		
		//If target is a npc and caster1 and caster2 exist
		//If Caster1 isn't the same as Caster2 and the effect is a DoT then ignore it.
		

		if(IsNPC() && spell_1->GetCasterID() != spell_2->GetCasterID()) 
		{
			if(effect_1 == SE_CurrentHP && sp1_detrimental && sp2_detrimental) 
			{
				continue;
				mlog(SPELLS__STACKING, "Both casters exist and are not the same, the "
					"effect is a detrimental dot, moving on");
			}
		}

		if(effect_1 == SE_CompleteHeal){ //SE_CompleteHeal never stacks or overwrites ever, always block.
			mlog(SPELLS__STACKING, "Blocking spell because complete heal never stacks or overwries");
			return (-1);
		}

		
		//If the effects are the same and
		//sp1 = beneficial & sp2 = detrimental or
		//sp1 = detrimental & sp2 = beneficial
		//Then this effect should be ignored for stacking purposes.
		

		if(sp_det_mismatch)
		{
			mlog(SPELLS__STACKING, "The effects are the same but the spell types are not, passing the effect");
			continue;
		}
		
		
		//If the spells aren't the same
		//and the effect is a dot we can go ahead and stack it
		
		if(effect_1 == SE_CurrentHP && sp1.id != sp2.id && sp1_detrimental && sp2_detrimental) 
		{
			mlog(SPELLS__STACKING, "The spells are not the same and it is a detrimental dot, passing");
			continue;
		}

		sp1_value = CalcSpellEffectValue(spell_1, i, spell_1->GetCasterLevel());
		sp2_value = CalcSpellEffectValue(spell_2, i, spell_2->GetCasterLevel());
		
		// some spells are hard to compare just on value.  attack speed spells
		// have a value that's a percentage for instance
		if
		(
			effect_1 == SE_AttackSpeed ||
			effect_1 == SE_AttackSpeed2 ||
			effect_1 == SE_AttackSpeed3
		)
		{
			sp1_value -= 100;
			sp2_value -= 100;
		}
		
		if(sp1_value < 0)
			sp1_value = 0 - sp1_value;
		if(sp2_value < 0)
			sp2_value = 0 - sp2_value;
		
		if(sp2_value < sp1_value) 
		{
			mlog(SPELLS__STACKING, "Spell %s (value %d) is not as good as %s (value %d). Rejecting %s.",
				sp2.name, sp2_value, sp1.name, sp1_value, sp2.name);
			return -1;	// can't stack
		}
		//we dont return here... a better value on this one effect dosent mean they are
		//all better...

		mlog(SPELLS__STACKING, "Spell %s (value %d) is not as good as %s (value %d). We will overwrite %s"
			" if there are no other conflicts.", sp1.name, sp1_value, sp2.name, sp2_value, sp1.name);
		will_overwrite = true;
	}
	
	//if we get here, then none of the values on the new spell are "worse"
	//so now we see if this new spell is any better, or if its not related at all
	if(will_overwrite) 
	{
		mlog(SPELLS__STACKING, "Stacking code decided that %s should overwrite %s.", sp2.name, sp1.name);
		return(1);
	}
	
	mlog(SPELLS__STACKING, "Stacking code decided that %s is not affected by %s.", sp2.name, sp1.name);
	return 0;
}

Buff *Mob::AddBuff(Mob *caster, Spell *spell_to_cast, sint32 &buff_slot, uint32 duration)
{
	if(duration == 0)
	{
		duration = CalcBuffDuration(caster, this, spell_to_cast);

		if(caster)
			duration = caster->GetActSpellDuration(spell_to_cast, duration);

		if(duration == 0) 
		{
			mlog(SPELLS__BUFFS, "Buff %d failed to add because its duration came back as 0.", spell_to_cast->GetSpellID());
			return NULL;
		}
	}

	buff_slot = GetFreeBuffSlot(spell_to_cast);
	if(buff_slot == -1)
	{
		return NULL;
	}

	if(current_buff_count == 0)
	{
		buff_tic_timer = new Timer(6000);
	}
	current_buff_count++;
	buffs[buff_slot] = new Buff(spell_to_cast, duration);
	//SendBuffPacket(buffs[buff_slot], buff_slot, 3);

	if(caster && caster->IsClient())
	{
		buffs[buff_slot]->SetIsClientBuff(true);
	}

	if(IsPet() && GetOwner() && GetOwner()->IsClient()) 
	{
		SendPetBuffsToClient();
	}

	CalcBonuses();
	return buffs[buff_slot];
}

int Mob::CanBuffStack(int16 spell_id, Mob *caster, bool iFailIfOverwrite)
{
	return CanBuffStack(&Spell(spell_id, caster, this), iFailIfOverwrite);
}

// Used by some MobAI stuff
// NOT USED BY SPELL CODE
// note that this should not be used for determining which slot to place a 
// buff into
// returns -1 on stack failure, -2 if all slots full, the slot number if the buff should overwrite another buff, or a free buff slot
int Mob::CanBuffStack(const Spell *spell_to_check, bool iFailIfOverwrite)
{
	
	int ret, first_free = -2;
	
	mlog(AI__BUFFS, "Checking if buff %d cast at level %d can stack on me.%s", spell_to_check->GetSpellID(), 
		spell_to_check->GetCasterLevel(), iFailIfOverwrite ? " failing if we would overwrite something" : "");
	
	int max_slots = GetMaxTotalSlots();
	for(int buffs_i = 0; buffs_i < max_slots; buffs_i++)
	{
		if(!buffs[buffs_i])
		{
			if(first_free == -2)
			{
				first_free = buffs_i;
			}
			continue;
		}

		if(buffs[buffs_i]->GetSpell()->GetSpellID() == spell_to_check->GetSpellID())
		{
			return -1;
		}

		ret = CheckStackConflict(buffs[buffs_i]->GetSpell(), spell_to_check);
		if(ret == 1)
		{
			if(iFailIfOverwrite) 
			{
				mlog(AI__BUFFS, "Buff %d would overwrite %d in slot %d, reporting stack failure", 
					spell_to_check->GetSpellID(), buffs[buffs_i]->GetSpell()->GetSpellID(), buffs_i);
				return(-1);
			}
		}
		else if(ret == -1)
		{
			mlog(AI__BUFFS, "Buff %d would conflict with %d in slot %d, reporting stack failure", 
				spell_to_check->GetSpellID(), buffs[buffs_i]->GetSpell()->GetSpellID(), buffs_i);
			return -1;
		}
	}
	
	mlog(AI__BUFFS, "Reporting that buff %d could successfully be placed into slot %d", spell_to_check->GetSpellID(), 
		first_free);

	return first_free;
}

///////////////////////////////////////////////////////////////////////////////
// spell effect related functions
//
// solar:
// this is actually applying a spell cast from 'this' on 'spelltar'
// it performs pvp checking and applies resists, etc then it
// passes it to SpellEffect which causes effects to the target
//
// this is called by these functions:
// Mob::SpellFinished
// Entity::AESpell (called by Mob::SpellFinished)
// Group::CastGroupSpell (called by Mob::SpellFinished)
//
// also note you can't interrupt the spell here. at this point it's going
// and if you don't want effects just return false.  interrupting here will
// break stuff
//
bool Mob::SpellOnTarget(uint16 spell_id, Mob* spell_target)
{
	Spell *spell_to_cast = new Spell(spell_id, this, spell_target);
	bool return_value = SpellOnTarget(spell_to_cast, spell_target);
	safe_delete(spell_to_cast);
	return return_value;
}

bool Mob::SpellOnTarget(Spell *spell_to_cast, Mob* spell_target)
{
	int16 caster_level = GetCasterLevel();
	
	int sequence = SendActionSpellPacket(spell_to_cast, spell_target, caster_level);
	
	if(spell_target->IsNPC())
	{
		char temp1[100];
		sprintf(temp1, "%d", spell_to_cast->GetSpellID());
		parse->Event(EVENT_CAST_ON, spell_target->GetNPCTypeID(), temp1, spell_target->CastToNPC(), this);
	}
	
	mlog(SPELLS__CASTING, "Casting spell %d on %s with effective caster level %d", spell_to_cast->GetSpellID(), spell_target->GetName(), caster_level);
	
	if(spell_target->GetInvul() || spell_target->DivineAura()) 
	{
		mlog(SPELLS__CASTING_ERR, "Casting spell %d on %s aborted: they are invulnerable.", spell_to_cast->GetSpellID(), spell_target->GetName());
		return false;
	}
	
	bodyType bt = spell_target->GetBodyType();
	if(bt == BT_NoTarget || bt == BT_NoTarget2) 
	{
		mlog(SPELLS__CASTING_ERR, "Casting spell %d on %s aborted: they are untargetable", spell_to_cast->GetSpellID(), spell_target->GetName());
		return false;
	}
	
	if(spell_to_cast->GetSpell().targettype == ST_UndeadAE)
	{
		if(spell_target->GetBodyType() != BT_SummonedUndead && 
			spell_target->GetBodyType() != BT_Undead && 
			spell_target->GetBodyType() != BT_Vampire)
		{
			mlog(SPELLS__CASTING_ERR, "Casting spell %d on %s aborted: they are not undead and this spell is undead target only.", spell_to_cast->GetSpellID(), spell_target->GetName());
			return false;
		}
	}
	
	//TODO: do something about double invis
	if(!WillSpellHold(spell_to_cast, spell_target))
	{
		return false;
	}
	
	if(spell_target->IsImmuneToSpell(spell_to_cast, this))
	{
		mlog(SPELLS__RESISTS, "Spell %d can't take hold due to immunity %s -> %s", spell_to_cast->GetSpellID(), GetName(), spell_target->GetName());
		return false;
	}
	
	float spell_effectiveness = DoSpellOnTargetResistCheck(spell_to_cast, spell_target);
	
	if(spell_effectiveness == 0)
	{
		mlog(SPELLS__RESISTS, "Spell %d can't take hold due to full resist %s -> %s", spell_to_cast->GetSpellID(), GetName(), spell_target->GetName());
		return false;
	}
	
	DoSpellOnTargetRecourse(spell_to_cast, spell_target);
	
	//Cazic Touch
	if(spell_to_cast->GetSpellID() == 982)
	{
		char target_name[64];
		strcpy(target_name, spell_target->GetCleanName());
		strupr(target_name);
		Shout("%s!", target_name);
	}
	
	if(spell_target->IsAIControlled() && spell_to_cast->IsDetrimentalSpell() && !spell_to_cast->IsHarmonySpell()) 
	{
		sint32 aggro_amount = CheckAggroAmount(spell_to_cast);
		mlog(SPELLS__CASTING, "Spell %d cast on %s generated %d hate", spell_to_cast->GetSpellID(), spell_target->GetName(), aggro_amount);
		if(aggro_amount > 0)
			spell_target->AddToHateList(this, aggro_amount);
		else{
			sint32 newhate = spell_target->GetHateAmount(this) + aggro_amount;
			if (newhate < 1) {
				spell_target->SetHate(this, 1);
			} else {
				spell_target->SetHate(this, newhate);
			}
		}
	}
	else if (spell_to_cast->IsBeneficialSpell())
		entity_list.AddHealAggro(spell_target, this, CheckHealAggroAmount(spell_to_cast, (spell_target->GetMaxHP() - spell_target->GetHP())));
		
	if(!spell_target->SpellEffect(this, spell_to_cast, sequence, spell_effectiveness))
	{
		mlog(SPELLS__CASTING_ERR, "Spell %d could not apply its effects %s -> %s\n", spell_to_cast->GetSpellID(), GetName(), spell_target->GetName());
		Message_StringID(MT_Shout, SPELL_NO_HOLD);
		return false;
	}
	
	if(!spell_to_cast->IsEffectInSpell(SE_TossUp))
	{
		if(spell_to_cast->GetSpell().pushback > 0 || spell_to_cast->GetSpell().pushup > 0)
		{
			SendKnockBackPacket(spell_to_cast->GetSpell().pushback, spell_to_cast->GetSpell().pushup);
		}
	}
	
	
	//TODO: Move Exemptions from here to SpellEffect as well as needed action 0x04 packets.
	//Shadow Step -> Target
	//Bind Affinity -> Caster and Target
	
	mlog(SPELLS__CASTING, "Cast of %d by %s on %s complete successfully.", spell_to_cast->GetSpellID(), GetName(), spell_target->GetName());	
	return true;
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

bool Mob::WillSpellHold(Spell *spell_to_cast, Mob *spell_target)
{
	if(!(IsClient() && CastToClient()->GetGM()) && !spell_to_cast->IsHarmonySpell())	// GMs can cast on anything
	{
		// Beneficial spells check
		if(spell_to_cast->IsBeneficialSpell())
		{
			if
			(
				IsClient() &&	//let NPCs do beneficial spells on anybody if they want, should be the job of the AI, not the spell code to prevent this from going wrong
				spell_target != this &&
				(
					!IsBeneficialAllowed(spell_target) ||
					(
						spell_to_cast->IsGroupOnlySpell() &&
						!(
							(entity_list.GetGroupByMob(this) &&
							entity_list.GetGroupByMob(this)->IsGroupMember(spell_target)) ||
							(spell_target == GetPet()) //should be able to cast grp spells on self and pet despite grped status.
						)
					)
				)
			)
			{
				mlog(SPELLS__CASTING_ERR, "Beneficial spell %d can't take hold %s -> %s, IBA? %d", spell_to_cast->GetSpellID(), GetName(), spell_target->GetName(), IsBeneficialAllowed(spell_target));
				Message_StringID(MT_Shout, SPELL_NO_HOLD);
				return false;
			}
		}
		else if(!IsAttackAllowed(spell_target, true) && !spell_to_cast->IsResurrectionEffects()) // Detrimental spells - PVP check
		{
			mlog(SPELLS__CASTING_ERR, "Detrimental spell %d can't take hold %s -> %s", spell_to_cast->GetSpellID(), GetName(), spell_target->GetName());
			spell_target->Message_StringID(MT_Shout, YOU_ARE_PROTECTED, GetCleanName());
			return false;
		}
	}
	return true;
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
	if(spell_on_target->GetSpell().targettype == ST_Group || spell_on_target->GetSpell().targettype == ST_GroupTeleport)
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
			if(GetOwner()->IsGrouped())
			{
				Group *g = entity_list.GetGroupByMob(GetOwner());
				g->CastGroupSpell(this, recourse_spell);
			}
			else if(GetOwner()->IsRaidGrouped() && GetOwner()->IsClient())
			{
				Raid *r = entity_list.GetRaidByClient(GetOwner()->CastToClient());
				int32 gid = r->GetGroup(GetOwner()->GetName());
				if(gid < 12)
				{
					r->CastGroupSpell(this, recourse_spell, gid);
				}
				else
				{
					SpellOnTarget(recourse_spell, GetOwner());
					SpellOnTarget(recourse_spell, this);
				}
			}
			else
			{
				SpellOnTarget(recourse_spell, GetOwner());
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

void Mob::SendKnockBackPacket(int push_up, int push_back)
{
	if(IsClient())
	{
		CastToClient()->SetKnockBackExemption(true);
		EQApplicationPacket* outapp_push = new EQApplicationPacket(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
		PlayerPositionUpdateServer_Struct* spu = (PlayerPositionUpdateServer_Struct*)outapp_push->pBuffer;

		double look_heading = CalculateHeadingToTarget(GetX(), GetY());
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

bool Mob::FindBuff(int16 spellid)
{
	int max_slots = GetMaxTotalSlots();
	for(int buffs_i = 0; buffs_i < max_slots; buffs_i++)
	{
		if(buffs[buffs_i])
		{
			if(buffs[buffs_i]->GetSpell()->GetSpellID() == spellid)
				return true;
		}
	}
	return false;
}

// solar: removes all buffs
void Mob::BuffFadeAll()
{
	int max_slots = GetMaxTotalSlots();
	for (int j = 0; j < max_slots; j++) 
	{
		if(buffs[j])
			BuffFadeBySlot(j, false);
	}
	//we tell BuffFadeBySlot not to recalc, so we can do it only once when were done
	CalcBonuses();
}

void Mob::BuffFadeDetrimental() 
{
	int max_slots = GetMaxTotalSlots();
	for (int j = 0; j < max_slots; j++) 
	{
		if(buffs[j]) 
		{
			if(buffs[j]->GetSpell()->IsDetrimentalSpell())
				BuffFadeBySlot(j, false);
		}
	}
}

void Mob::BuffFadeDetrimentalByCaster(Mob *caster)
{
	if(!caster)
		return;

	int max_slots = GetMaxTotalSlots();
	for (int j = 0; j < max_slots; j++) 
	{
		if(buffs[j]) 
		{
			if(buffs[j]->GetSpell()->IsDetrimentalSpell())
			{
				if(caster->GetID() == buffs[j]->GetCasterID())
				{
					BuffFadeBySlot(j, false);
				}
			}
		}
	}
}

// solar: removes the buff matching spell_id
void Mob::BuffFadeBySpellID(int16 spell_id)
{
	int max_slots = GetMaxTotalSlots();
	for (int j = 0; j < max_slots; j++)
	{
		if (buffs[j] && buffs[j]->GetSpell()->GetSpellID() == spell_id)
			BuffFadeBySlot(j, false);
	}
	
	//we tell BuffFadeBySlot not to recalc, so we can do it only once when were done
	CalcBonuses();
}

// solar: removes buffs containing effectid, skipping skipslot
void Mob::BuffFadeByEffect(int effectid, int skipslot)
{
	int i;

	int max_slots = GetMaxTotalSlots();
	for(i = 0; i < max_slots; i++)
	{
		if(!buffs[i])
			continue;

		if(buffs[i]->GetSpell()->IsEffectInSpell(effectid) && i != skipslot)
			BuffFadeBySlot(i, false);
	}

	//we tell BuffFadeBySlot not to recalc, so we can do it only once when were done
	CalcBonuses();
}

// solar: checks if 'this' can be affected by spell_id from caster
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

	if(spell_to_cast->IsMezSpell())
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

	if(spell_to_cast->IsCharmSpell())
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

	if(spell_to_cast->IsSacrificeSpell())
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
		mlog(SPELLS__RESISTS, "We are immune to magic, so we fully resist the spell %d", spell_to_cast->GetSpellID());
		return(0);
	}
	
	if(resist_type == RESIST_NONE) {
		//unresistable...
		mlog(SPELLS__RESISTS, "The spell %d is unresistable (type %d)", spell_to_cast->GetSpellID(), resist_type);
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
		mlog(SPELLS__RESISTS, "We are %d levels above the caster, which is higher than the %d level auto-resist gap. Fully resisting.",  target_level - caster_level, RuleI(Spells, AutoResistDiff));
 		return 0;
	}
	
	//check for buff/item/aa based fear moditifers
	//still working on this...
	if (spell_to_cast && spell_to_cast->IsFearSpell()) {
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
			mlog(SPELLS__RESISTS, "Had a %d chance of resisting the fear spell %d, and succeeded.", rchance, spell_to_cast->GetSpellID());
			return(0);
		}
		mlog(SPELLS__RESISTS, "Had a %d chance of resisting the fear spell %d, and failed.", rchance, spell_to_cast->GetSpellID());
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

	if(caster && caster->IsClient())
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
		if(spell_to_cast->IsFearSpell())
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
		mlog(SPELLS__RESISTS, "Spell %d: Roll of %.2f > resist chance of %.2f, no resist", spell_to_cast->GetSpellID(), roll, resistchance);
		return(100);
	}
	else
	{
		if (roll <= fullchance)
 		{
			mlog(SPELLS__RESISTS, "Spell %d: Roll of %.2f <= fullchance %.2f, fully resisted", spell_to_cast->GetSpellID(), roll, fullchance);
			return(0);
		}
		else
		{
			mlog(SPELLS__RESISTS, "Spell %d: Roll of %.2f > fullchance %.2f, partially resisted, returned %.2f", spell_to_cast->GetSpellID(), roll, fullchance, (100 * ((roll-fullchance)/(resistchance-fullchance))));
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
		
		//TODO: FIXME
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

// solar: add/update a spell in the client's spell bar
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

// solar: remove a spell from the client's spell bar
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

// solar: add a spell to client's spellbook
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

// solar: remove a spell from client's spellbook
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


bool Mob::FindType(int8 type, bool bOffensive, int16 threshold) 
{
	int max_slots = GetMaxTotalSlots();
	for (int i = 0; i < max_slots; i++) 
	{
		if (buffs[i]) 
		{
			for (int j = 0; j < EFFECT_COUNT; j++) 
			{
                // adjustments necessary for offensive npc casting behavior
                if (bOffensive) 
				{
					if(buffs[i]->GetSpell()->GetSpell().effectid[j] == type) 
					{
						sint16 value = CalcSpellEffectValue_formula(buffs[i]->GetSpell()->GetSpell().formula[j],
							buffs[i]->GetSpell()->GetSpell().base[j],
							buffs[i]->GetSpell()->GetSpell().max[j],
							buffs[i]->GetSpell()->GetCasterLevel(), 
							buffs[i]->GetSpell(),
							buffs[i]->GetDurationRemaining());
                        LogFile->write(EQEMuLog::Normal, "FindType: type = %d; value = %d; threshold = %d",
							type, value, threshold);
                        if (value < threshold)
                            return true;
                    }
                } 
				else 
				{
				    if(buffs[i]->GetSpell()->GetSpell().effectid[j] == type)
					    return true;
                }
			}
		}
	}
	return false;
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
	if (IsClient() && (bard_song || casting_spell ? casting_spell->IsBardSong() : 0))
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
		delete outapp;
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

void Mob::BuffModifyDurationBySpellID(int16 spell_id, sint32 newDuration)
{
	int max_slots = GetMaxTotalSlots();
	for(int i = 0; i < max_slots; ++i)
	{
		if(buffs[i])
		{
			if(buffs[i]->GetSpell()->GetSpellID() == spell_id)
			{
				buffs[i]->SetDurationRemaining(newDuration);
				SendBuffPacket(buffs[i], i, 0);
			}
		}
	}
}

int Client::GetCurrentBuffSlots() const
{
	return 15 + GetAA(aaMysticalAttuning);
}

int Client::GetCurrentSongSlots() const
{
	return 6 + GetAA(aaMysticalAttuning);
}

void Client::InitializeBuffSlots()
{
	int max_slots = GetMaxTotalSlots();
	buffs = new Buff*[max_slots];
	for(int x = 0; x < max_slots; ++x)
	{
		buffs[x] = NULL;
	}
	current_buff_count = 0;
	buff_tic_timer = NULL;
}

void Client::UninitializeBuffSlots()
{
	int max_slots = GetMaxTotalSlots();
	for(int x = 0; x < max_slots; ++x)
	{
		safe_delete(buffs[x]);
	}
	safe_delete_array(buffs);
}

void NPC::InitializeBuffSlots()
{
	int max_slots = GetMaxTotalSlots();
	buffs = new Buff*[max_slots];
	for(int x = 0; x < max_slots; ++x)
	{
		buffs[x] = NULL;
	}
	current_buff_count = 0;
	buff_tic_timer = NULL;
}

void NPC::UninitializeBuffSlots()
{
	int max_slots = GetMaxTotalSlots();
	for(int x = 0; x < max_slots; ++x)
	{
		safe_delete(buffs[x]);
	}
	safe_delete_array(buffs);
}

int Client::GetFreeBuffSlot(const Spell *spell_to_cast)
{
	//this isn't technically required since clients only have one spell slot for discs but since 
	//I was doing it anyway might as well make it easy to add another in the future *shrug*
	if(spell_to_cast->IsDiscipline())
	{
		int start = GetMaxBuffSlots() + GetMaxSongSlots();
		int end = start + GetCurrentDiscSlots();
		return CheckBuffSlotStackConflicts(spell_to_cast, start, end);
	}

	if(spell_to_cast->GetSpell().short_buff_box)
	{
		int start = GetMaxBuffSlots();
		int end = start + GetCurrentSongSlots();
		return CheckBuffSlotStackConflicts(spell_to_cast, start, end);
	}

	int start = 0;
	int end = GetCurrentBuffSlots();
	return CheckBuffSlotStackConflicts(spell_to_cast, start, end);
}

int NPC::GetFreeBuffSlot(const Spell *spell_to_cast)
{
	if(spell_to_cast->IsDiscipline())
	{
		int start = GetMaxBuffSlots() + GetMaxSongSlots();
		int end = start + GetCurrentDiscSlots();
		return CheckBuffSlotStackConflicts(spell_to_cast, start, end);
	}

	if(spell_to_cast->GetSpell().short_buff_box)
	{
		int start = GetMaxBuffSlots();
		int end = start + GetCurrentSongSlots();
		return CheckBuffSlotStackConflicts(spell_to_cast, start, end);
	}

	int start = 0;
	int end = GetCurrentBuffSlots();
	return CheckBuffSlotStackConflicts(spell_to_cast, start, end);
}

int Mob::CheckBuffSlotStackConflicts(const Spell* spell_to_cast, int start, int end)
{
	sint32 buff_slot = 0;
	bool will_overwrite = false;
	vector<int> overwrite_slots;
	int empty_slot = -1;
	
	for(buff_slot = start; buff_slot < end; buff_slot++)
	{
		Buff *cur_buff = buffs[buff_slot];
		if(cur_buff)
		{
			// there's a buff in this slot
			int ret = CheckStackConflict(cur_buff->GetSpell(), spell_to_cast);
			if(ret == -1) 
			{
				// stop the spell
				mlog(SPELLS__BUFFS, "Adding buff %d failed: stacking prevented by spell %d in slot %d with caster level %d",
					spell_to_cast->GetSpellID(), cur_buff->GetSpell()->GetSpellID(), buff_slot, cur_buff->GetSpell()->GetCasterLevel());
				return -1;
			}
			else if(ret == 1) 
			{
				// set a flag to indicate that there will be overwriting
				mlog(SPELLS__BUFFS, "Adding buff %d will overwrite spell %d in slot %d with caster level %d", 
					spell_to_cast->GetSpellID(), cur_buff->GetSpell()->GetSpellID(), buff_slot, cur_buff->GetSpell()->GetCasterLevel());
				will_overwrite = true;
				overwrite_slots.push_back(buff_slot);
			}
		}
		else
		{
			if(empty_slot == -1)
				empty_slot = buff_slot;
		}
	}
	
	mlog(SPELLS__BUFFS, "Checking where buff %d (cast level %d) can be added safely...",
		spell_to_cast->GetSpellID(), spell_to_cast->GetCasterLevel());

	// first we loop through everything checking that the spell
	// can stack with everything.  this is to avoid stripping the spells
	// it would overwrite, and then hitting a buff we can't stack with.
	// we also check if overwriting will occur.  this is so after this loop
	// we can determine if there will be room for this buff
	// we didn't find an empty slot to put it in, and it's not overwriting
	// anything so there must not be any room left.
 	if(empty_slot == -1 && !will_overwrite)
 	{
 		if(spell_to_cast->IsDetrimentalSpell()) //Sucks to be you, bye bye one of your buffs
 		{
 			for(buff_slot = start; buff_slot < end; buff_slot++)
 			{
 				Buff *cur_buff = buffs[buff_slot];
				if(cur_buff->GetSpell()->IsBeneficialSpell())
 				{
 					mlog(SPELLS__BUFFS, "No slot for detrimental buff %d, so we are overwriting a beneficial buff %d in slot %d", 
						spell_to_cast->GetSpellID(), cur_buff->GetSpell()->GetSpellID(), buff_slot);
 					BuffFadeBySlot(buff_slot, false);
 					empty_slot = buff_slot;
					break;
 				}
 			}
 			if(empty_slot == -1) 
			{
	 			mlog(SPELLS__BUFFS, "Unable to find a buff slot for detrimental buff %d", spell_to_cast->GetSpellID());
				return -1;
 			}
 		}
 		else 
		{
			mlog(SPELLS__BUFFS, "Unable to find a buff slot for beneficial buff %d", spell_to_cast->GetSpellID());
 			return -1;
 		}
 	}

	if(will_overwrite)
	{
		vector<int>::iterator cur, end;
		cur = overwrite_slots.begin();
		end = overwrite_slots.end();
		for(; cur != end; cur++) 
		{
			// strip spell
			BuffFadeBySlot(*cur, false);

			// if we hadn't found a free slot before, or if this is earlier
			// we use it
			if(empty_slot == -1 || *cur < empty_slot)
				empty_slot = *cur;
		}
	}
	
	return empty_slot;
}

bool Mob::ValidateStartSpellCast(const Spell *spell_to_cast)
{
	bool return_value = true;
	if(!IsValidSpell(spell_to_cast->GetSpellID()) || casting_spell != NULL)
	{
		printf("%d\n", spell_to_cast->GetSpellID());
		printf("%p\n", casting_spell);
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

	const SPDat_Spell_Struct &spell = spells[spell_id];
	memcpy((void*)&raw_spell, (const void*)&spell, sizeof(SPDat_Spell_Struct));
	raw_spell.id = spell_id;
}

Spell::Spell()
{
	cast_timer = NULL;
	spell_class_type = SC_NORMAL;
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
	if(this->cast_timer)
	{
		return_value->cast_timer = new Timer(this->cast_timer->GetRemainingTime());
	}

	memcpy((void*)&return_value->raw_spell, (const void*)&this->raw_spell, sizeof(SPDat_Spell_Struct));
	return_value->raw_spell.id = GetSpellID();
	return return_value;
}

Buff::Buff(Spell *spell, uint32 duration)
{
	spell_duration_remaining = duration;
	is_perm_illusion = false;
	magic_remaining_charges = 1;
	poison_remaining_charges = spell->CalculatePoisonCounters();
	disease_remaining_charges = spell->CalculateDiseaseCounters();
	curse_remaining_charges = spell->CalculateCurseCounters();
	general_remaining_charges = spell->GetSpell().numhits;	
	melee_shield_remaining = 0;
	magic_shield_remaining = 0;
	melee_shield_reduction = 0;
	magic_shield_reduction = 0;
	attacks_negated = 0;
	death_save_chance = 0;
	caster_aa_rank = 0;
	instrument_mod = 10;
	is_client = false;
	buff_spell = spell->CopySpell();
	is_perm_duration = (spell->GetSpell().buffdurationformula == 50);
}

uint32 Buff::GetCasterID() const
{
	return GetSpell()->GetCasterID();
}
