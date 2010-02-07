/*  EQEMu:  Everquest Server Emulator
  Copyright (C) 2001-2010  EQEMu Development Team (http://www.eqemulator.org)

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
#include "spells.h"
#include "buff.h"
#include "StringIDs.h"
#include "worldserver.h"
#include "../common/packet_dump.h"
#include "../common/Item.h"
#include "../common/skills.h"
#include "../common/bodytypes.h"
#include "../common/classes.h"
#include "../common/rulesys.h"
#include <math.h>
#include <assert.h>
#include <sstream>
#include <algorithm>

#ifdef EMBPERL
#include "embparser.h"
#endif

#ifndef WIN32
#include <stdlib.h>
#include "../common/unix.h"
#endif

extern Zone* zone;
extern volatile bool ZoneLoaded;
extern bool spells_loaded;
extern WorldServer worldserver;
#if !defined(NEW_LoadSPDat) && !defined(DB_LoadSPDat)
	extern SPDat_Spell_Struct spells[SPDAT_RECORDS];
#endif

/*

	General outline of spell casting process
	
	1.
		a)	Client clicks a spell bar gem, ability, or item.  client_process.cpp
		gets the op and calls CastSpell() with all the relevant info including
		cast time.

		b)  NPC does CastSpell() from AI

	2.
		a)	CastSpell() determines there is a cast time and set the passed spell to the 
		SpellProcess() to check it state later.

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
		mlog(SPELLS__CASTING, "Spell %d: AE duration beacon created, entity id %d", spell_to_cast->GetSpellID(), beacon->GetID());
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
			SendKnockBackPacket(caster, spell_to_cast->GetSpell().pushup, spell_to_cast->GetSpell().pushback);
		}
	}

	//Send the action packet to everyone
	int seq = SendActionSpellPacket(spell_to_cast, this, caster->GetCasterLevel());

	//Do the spell on target
	caster->SpellOnTarget(spell_to_cast, this);
}

///////////////////////////////////////////////////////////////////////////////
// spell effect related functions
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
	
	if(spell_target->IsAIControlled() && spell_to_cast->IsDetrimentalSpell() && 
		!spell_to_cast->IsHarmonySpell() && !spell_to_cast->IsEffectInSpell(SE_BindSight)) 
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
			spell_target->SendKnockBackPacket(this, spell_to_cast->GetSpell().pushup, spell_to_cast->GetSpell().pushback);
		}
	}
	
	
	//TODO: Move Exemptions from here to SpellEffect as well as needed action 0x04 packets.
	//Shadow Step -> Target
	//Bind Affinity -> Caster and Target
	
	mlog(SPELLS__CASTING, "Cast of %d by %s on %s complete successfully.", spell_to_cast->GetSpellID(), GetName(), spell_target->GetName());	
	return true;
}
