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
#include <math.h>
#ifndef WIN32
#include <stdlib.h>
#include "../common/unix.h"
#endif
#if !defined(NEW_LoadSPDat) && !defined(DB_LoadSPDat)
	extern SPDat_Spell_Struct spells[SPDAT_RECORDS];
#endif
extern bool spells_loaded;

///////////////////////////////////////////////////////////////////////////////
// spell property testing functions

bool IsTargetableAESpell(int16 spell_id) {
	bool bResult = false;

	if (IsValidSpell(spell_id) && spells[spell_id].targettype == ST_AETarget) {
		bResult = true;
	}

	return bResult;
}

bool Spell::IsTargetableAESpell() const
{
	if (raw_spell.targettype == ST_AETarget) 
	{
		return true;
	}

	return false;
}


bool IsLifetapSpell(int16 spell_id)
{
	return
	(
		IsValidSpell(spell_id) &&
		(
			spells[spell_id].targettype == ST_Tap ||
			(
				spell_id == 2115	// Ancient: Lifebane
			)
		)
	);
}

bool Spell::IsLifetapSpell() const
{
	return (raw_spell.targettype == ST_Tap || raw_spell.id == 2115);
}

bool IsSummonSpell(int16 spellid) {
	for (int o = 0; o < EFFECT_COUNT; o++)
	{
		int32 tid = spells[spellid].effectid[o];
		if(tid == SE_SummonPet || tid == SE_SummonItem || tid == SE_SummonPC)
		{
			return true;
		}
	}
	return false;
}

bool Spell::IsSummonSpell() const 
{
	for (int o = 0; o < EFFECT_COUNT; o++)
	{
		int32 tid = raw_spell.effectid[o];
		if(tid == SE_SummonPet || tid == SE_SummonItem || tid == SE_SummonPC)
		{
			return true;
		}
	}
	return false;
}

bool IsDamageSpell(int16 spellid) {
	for (int o = 0; o < EFFECT_COUNT; o++)
	{
		int32 tid = spells[spellid].effectid[o];
		if((tid == SE_CurrentHPOnce || tid == SE_CurrentHP) && spells[spellid].targettype != ST_Tap && spells[spellid].buffduration < 1 && spells[spellid].base[o] < 0)
		{
			return true;
		}
	}
	return false;
}

bool Spell::IsDamageSpell() const
{
	for (int o = 0; o < EFFECT_COUNT; o++)
	{
		int32 tid = raw_spell.effectid[o];
		if((tid == SE_CurrentHPOnce || tid == SE_CurrentHP) && raw_spell.targettype != ST_Tap && raw_spell.buffduration < 1 && raw_spell.base[o] < 0)
		{
			return true;
		}
	}
	return false;
}

bool IsSlowSpell(int16 spell_id)
{
	int i;
	const SPDat_Spell_Struct &sp = spells[spell_id];

	for(i = 0; i < EFFECT_COUNT; i++)
	{
		if
		(
			sp.effectid[i] == SE_AttackSpeed			// attack speed effect
			 && sp.base[i] < 100		// less than 100%
		)
			return true;
	}

	return false;
}

bool Spell::IsSlowSpell() const
{
	for(int i = 0; i < EFFECT_COUNT; i++)
	{
		if(raw_spell.effectid[i] == SE_AttackSpeed && raw_spell.base[i] < 100)
			return true;
	}

	return false;
}

bool IsHasteSpell(int16 spell_id)
{
	int i;
	const SPDat_Spell_Struct &sp = spells[spell_id];

	for(i = 0; i < EFFECT_COUNT; i++)
	{
		if(sp.effectid[i] == SE_AttackSpeed)
			return(sp.base[i] < 100);
	}

	return false;
}

bool Spell::IsHasteSpell() const 
{
	for(int i = 0; i < EFFECT_COUNT; i++)
	{
		if(raw_spell.effectid[i] == SE_AttackSpeed && raw_spell.base[i] > 100)
			return true;
	}

	return false;
}

bool IsHarmonySpell(int16 spell_id)
{
	return (IsEffectInSpell(spell_id, SE_Harmony) || IsEffectInSpell(spell_id, SE_Lull));
}

bool Spell::IsHarmonySpell() const
{
	return (IsEffectInSpell(SE_Harmony) || IsEffectInSpell(SE_Lull));
}

bool IsGroupOnlySpell(int16 spell_id)
{
	return IsValidSpell(spell_id) && spells[spell_id].goodEffect == 2;
}

bool Spell::IsGroupOnlySpell() const
{
	return raw_spell.goodEffect == 2;
}

bool IsBeneficialSpell(int16 spell_id)
{
	// EverHood - These spells are actually detrimental
	if(spells[spell_id].goodEffect == 1){
		SpellTargetType tt = spells[spell_id].targettype;
		if(tt != ST_Self || tt != ST_Pet){
			if(IsEffectInSpell(spell_id, SE_CancelMagic))
				return false;
		}		
		if(tt == ST_Target || tt == ST_AETarget || tt == ST_Animal || tt == ST_Undead || tt == ST_Pet) {
			int16 sai = spells[spell_id].SpellAffectIndex;
			if(spells[spell_id].resisttype == RESIST_MAGIC){
				if(sai == SAI_Calm || sai == SAI_Dispell_Sight || sai == SAI_Memory_Blur || sai == SAI_Calm_Song)
					return false;
			}else{
				// Bind Sight and Cast Sight
				if(sai == SAI_Dispell_Sight && spells[spell_id].skill == 18)
					return false;
			}
		}
	}
	return spells[spell_id].goodEffect != 0 || IsGroupSpell(spell_id);
}

bool Spell::IsBeneficialSpell() const
{
	if(raw_spell.goodEffect == 1)
	{
		SpellTargetType tt = raw_spell.targettype;
		if(tt != ST_Self || tt != ST_Pet)
		{
			if(IsEffectInSpell(SE_CancelMagic))
			{
				return false;
			}
		}		
		if(tt == ST_Target || tt == ST_AETarget || tt == ST_Animal || tt == ST_Undead || tt == ST_Pet) 
		{
			int16 sai = raw_spell.SpellAffectIndex;
			if(raw_spell.resisttype == RESIST_MAGIC)
			{
				if(sai == SAI_Calm || sai == SAI_Dispell_Sight || sai == SAI_Memory_Blur || sai == SAI_Calm_Song)
				{
					return false;
				}
			}
			else
			{
				// Bind Sight and Cast Sight
				if(sai == SAI_Dispell_Sight && raw_spell.skill == 18)
				{
					return false;
				}
			}
		}
	}
	return raw_spell.goodEffect != 0 || IsGroupSpell();
}

bool IsDetrimentalSpell(int16 spell_id)
{
	return !IsBeneficialSpell(spell_id);
}

bool Spell::IsDetrimentalSpell() const
{
	return !IsBeneficialSpell();
}

bool IsSummonPetSpell(int16 spell_id)
{
	return
	(
		IsEffectInSpell(spell_id, SE_SummonPet) ||
		IsEffectInSpell(spell_id, SE_SummonBSTPet)
	);
}

bool Spell::IsSummonPetSpell() const
{
	return (IsEffectInSpell(SE_SummonPet) || IsEffectInSpell(SE_SummonBSTPet));
}

bool IsBlindSpell(int16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_Blind);
}

bool IsAEDurationSpell(int16 spell_id)
{
	return IsValidSpell(spell_id) &&
		(spells[spell_id].targettype == ST_AETarget || spells[spell_id].targettype == ST_UndeadAE )
		&& spells[spell_id].AEDuration !=0;
}

bool Spell::IsAEDurationSpell() const
{
	return (raw_spell.targettype == ST_AETarget || raw_spell.targettype == ST_UndeadAE)	&& raw_spell.AEDuration != 0;
}

bool IsPureNukeSpell(int16 spell_id)
{
	int i, effect_count = 0;

	if(!IsValidSpell(spell_id))
		return false;

	for(i = 0; i < EFFECT_COUNT; i++)
	{
		if(!IsBlankSpellEffect(spell_id, i))
			effect_count++;
	}

	return
	(
		spells[spell_id].effectid[0] == SE_CurrentHP &&
		effect_count == 1
	);
}

bool Spell::IsPureNukeSpell() const
{
	int i, effect_count = 0, last_index = 0;

	for(i = 0; i < EFFECT_COUNT; i++)
	{
		if(!IsBlankSpellEffect(i))
		{
			last_index = i;
			effect_count++;
		}
	}

	return
	(
		raw_spell.effectid[last_index] == SE_CurrentHP &&
		effect_count == 1
	);
}

bool IsPartialCapableSpell(int16 spell_id)
{
	if(IsPureNukeSpell(spell_id) || IsEffectInSpell(spell_id, SE_Fear) || IsEffectInSpell(spell_id, SE_Charm))
		return true;
	
	return false;
}

bool Spell::IsPartialCapableSpell() const
{
	return (IsPureNukeSpell() || IsEffectInSpell(SE_Fear) || IsEffectInSpell(SE_Charm));
}

bool IsResistableSpell(int16 spell_id)
{
	// solar: for now only detrimental spells are resistable.  later on i will
	// add specific exceptions for the beneficial spells that are resistable
	if(IsDetrimentalSpell(spell_id))
	{
		return true;
	}

	return false;
}

bool Spell::IsResistableSpell() const
{
	return IsDetrimentalSpell();
}

// solar: checks if this spell affects your group
bool IsGroupSpell(int16 spell_id)
{
	return
	(
		IsValidSpell(spell_id) &&
		(
			spells[spell_id].targettype == ST_AEBard ||
			spells[spell_id].targettype == ST_Group || 
			spells[spell_id].targettype == ST_GroupTeleport
		)
	);
}

bool Spell::IsGroupSpell() const
{
	return
	(
		raw_spell.targettype == ST_AEBard ||
		raw_spell.targettype == ST_Group || 
		raw_spell.targettype == ST_GroupTeleport
	);
}

// solar: checks if this spell can be targeted
bool IsTGBCompatibleSpell(int16 spell_id)
{
	return
	(
		IsValidSpell(spell_id) &&
		(
			!IsDetrimentalSpell(spell_id) &&
			spells[spell_id].buffduration != 0 &&
			!IsBardSong(spell_id) &&
			!IsEffectInSpell(spell_id, SE_Illusion)
		)
	);
}

bool Spell::IsTGBCompatibleSpell() const
{
	return (!IsDetrimentalSpell() && raw_spell.buffduration != 0 && !IsBardSong() && !IsEffectInSpell(SE_Illusion));
}

bool IsBardSong(int16 spell_id)
{
	return
	(
		IsValidSpell(spell_id) &&
		spells[spell_id].classes[BARD - 1] < 255
	);
}

bool Spell::IsBardSong() const
{
	return (raw_spell.classes[BARD - 1] < 255);
}

bool IsEffectInSpell(int16 spellid, int effect)
{
	int j;

	if(!IsValidSpell(spellid))
		return false;

	for(j = 0; j < EFFECT_COUNT; j++)
		if(spells[spellid].effectid[j] == effect) 
			return true;

	return false;
}

bool Spell::IsEffectInSpell(int effect) const
{
	for(int j = 0; j < EFFECT_COUNT; j++)
	{
		if(raw_spell.effectid[j] == effect)
		{
			return true;
		}
	}
	return false;
}

// solar: arguments are spell id and the index of the effect to check.
// this is used in loops that process effects inside a spell to skip
// the blanks
bool IsBlankSpellEffect(int16 spellid, int effect_index)
{
	int effect, base, formula;

	effect = spells[spellid].effectid[effect_index];
	base = spells[spellid].base[effect_index];
	formula = spells[spellid].formula[effect_index];

	return
	(
		effect == SE_Blank ||	// blank marker
		(	// spacer
			effect == SE_CHA && 
			base == 0 &&
			formula == 100
		)
		||
		effect == SE_StackingCommand_Block ||	// these are only used by stacking code
		effect == SE_StackingCommand_Overwrite
	);
}

bool Spell::IsBlankSpellEffect(int effect_index) const
{
	int effect, base, formula;

	effect = raw_spell.effectid[effect_index];
	base = raw_spell.base[effect_index];
	formula = raw_spell.formula[effect_index];

	return
	(
		effect == SE_Blank ||	// blank marker
		(	// spacer
			effect == SE_CHA && 
			base == 0 &&
			formula == 100
		)
		||
		effect == SE_StackingCommand_Block ||	// these are only used by stacking code
		effect == SE_StackingCommand_Overwrite
	);
}

// solar: checks some things about a spell id, to see if we can proceed
bool IsValidSpell(int16 spellid)
{
	return
	(
		spells_loaded &&
		spellid != 0 &&
		spellid != 1 &&
		spellid != 0xFFFF &&
		spellid < SPDAT_RECORDS &&
		spells[spellid].player_1[0]
	);
}

//returns the lowest level of any caster which can use the spell
int GetMinLevel(int16 spell_id) {
	int r;
	int min = 255;
	const SPDat_Spell_Struct &spell = spells[spell_id];
	for(r = 0; r < PLAYER_CLASS_COUNT; r++) {
		if(spell.classes[r] < min)
			min = spell.classes[r];
	}
	
	//if we can't cast the spell return 0
	//just so it wont screw up calculations used in other areas of the code
	if(min == 255)
		return 0;
	else
		return(min);
}

int Spell::GetMinLevel() const
{
	int r;
	int min = 255;
	for(r = 0; r < PLAYER_CLASS_COUNT; r++) 
	{
		if(raw_spell.classes[r] < min)
			min = raw_spell.classes[r];
	}
	
	//if we can't cast the spell return 0
	//just so it wont screw up calculations used in other areas of the code
	if(min == 255)
		return 0;
	else
		return(min);
}

// solar: this will find the first occurance of effect.  this is handy
// for spells like mez and charm, but if the effect appears more than once
// in a spell this will just give back the first one.
int GetSpellEffectIndex(int16 spell_id, int effect)
{
	int i;

	if(!IsValidSpell(spell_id))
		return -1;

	for(i = 0; i < EFFECT_COUNT; i++)
	{
		if(spells[spell_id].effectid[i] == effect)
			return i;
	}

	return -1;
}

int Spell::GetSpellEffectIndex(int effect) const
{
	for(int i = 0; i < EFFECT_COUNT; i++)
	{
		if(raw_spell.effectid[i] == effect)
			return i;
	}

	return -1;
}

// solar: returns the level required to use the spell if that class/level
// can use it, 0 otherwise
// note: this isn't used by anything right now
int CanUseSpell(int16 spellid, int classa, int level)
{
	int level_to_use;
	
	if(!IsValidSpell(spellid) || classa >= PLAYER_CLASS_COUNT)
		return 0;

	level_to_use = spells[spellid].classes[classa - 1];

	if
	(
		level_to_use &&
		level_to_use != 255 &&
		level >= level_to_use
	)
		return level_to_use;

	return 0;
}



bool BeneficialSpell(int16 spell_id)
{
	if (spell_id <= 0 || spell_id >= SPDAT_RECORDS 
		/*|| spells[spell_id].stacking == 27*/ )
	{
		return true;
	}
	switch (spells[spell_id].goodEffect)
	{
		case 1:
		case 3:
			return true;
	}
	return false;
}

bool Spell::BeneficialSpell() const
{
	if (GetSpellID() <= 0 || GetSpellID() >= SPDAT_RECORDS 
		/*|| spells[spell_id].stacking == 27*/ )
	{
		return true;
	}
	switch (raw_spell.goodEffect)
	{
		case 1:
		case 3:
			return true;
	}
	return false;
}

sint32 CalculatePoisonCounters(int16 spell_id){
	if(!IsValidSpell(spell_id))
		return 0;

	sint32 Counters = 0;
	for(int i = 0; i < EFFECT_COUNT; i++)
	{
		if(spells[spell_id].effectid[i] == SE_PoisonCounter && spells[spell_id].base[i] > 0){
			Counters += spells[spell_id].base[i];
		}
	}
    return Counters;
}

sint32 Spell::CalculatePoisonCounters() const
{
	sint32 Counters = 0;
	for(int i = 0; i < EFFECT_COUNT; i++)
	{
		if(raw_spell.effectid[i] == SE_PoisonCounter && raw_spell.base[i] > 0)
		{
			Counters += raw_spell.base[i];
		}
	}
    return Counters;
}

sint32 CalculateDiseaseCounters(int16 spell_id){
	if(!IsValidSpell(spell_id))
		return 0;

	sint32 Counters = 0;
	for(int i = 0; i < EFFECT_COUNT; i++)
	{
		if(spells[spell_id].effectid[i] == SE_DiseaseCounter && spells[spell_id].base[i] > 0){
			Counters += spells[spell_id].base[i];
		}
	}
    return Counters;
}

sint32 Spell::CalculateDiseaseCounters() const
{
	sint32 Counters = 0;
	for(int i = 0; i < EFFECT_COUNT; i++)
	{
		if(raw_spell.effectid[i] == SE_DiseaseCounter && raw_spell.base[i] > 0)
		{
			Counters += raw_spell.base[i];
		}
	}
    return Counters;
}

sint32 CalculateCurseCounters(int16 spell_id){
	if(!IsValidSpell(spell_id))
		return 0;

	sint32 Counters = 0;
	for(int i = 0; i < EFFECT_COUNT; i++)
	{
		if(spells[spell_id].effectid[i] == SE_CurseCounter && spells[spell_id].base[i] > 0){
			Counters += spells[spell_id].base[i];
		}
	}
    return Counters;
}

sint32 Spell::CalculateCurseCounters() const
{
	sint32 Counters = 0;
	for(int i = 0; i < EFFECT_COUNT; i++)
	{
		if(raw_spell.effectid[i] == SE_CurseCounter && raw_spell.base[i] > 0)
		{
			Counters += raw_spell.base[i];
		}
	}
    return Counters;
}

bool IsDiscipline(int16 spell_id)
{
	if(!IsValidSpell(spell_id))
		return false;

	if(spells[spell_id].mana == 0 && (spells[spell_id].EndurCost || spells[spell_id].EndurUpkeep))
	{
		return true;
	}
	return false;
}

bool Spell::IsDiscipline() const
{
	if(raw_spell.mana == 0 && (raw_spell.EndurCost || raw_spell.EndurUpkeep))
	{
		return true;
	}
	return false;
}

bool IsResurrectionEffects(int16 spell_id) {
	bool Result = false;

	if(IsValidSpell(spell_id) && spell_id == 756)		// spell id 756 is Resurrection Effects spell
		Result = true;

	return Result;
}

bool Spell::IsResurrectionEffects() const
{
	return (GetSpellID() == 756);
}

bool IsManaTapSpell(int16 spell_id) {
	bool Result = false;

	if(IsValidSpell(spell_id)) {
		for(int i = 0; i < EFFECT_COUNT; i++) {
			if(spells[spell_id].effectid[i] == SE_CurrentMana && spells[spell_id].targettype == ST_Tap) {
				Result = true;
				break;
			}
		}
	}

	return Result;
}

bool Spell::IsManaTapSpell() const
{
	if(raw_spell.targettype == ST_Tap && IsEffectInSpell(SE_CurrentMana))
	{
		return true;
	}
	return false;
}

bool Spell::IsFullDeathSaveSpell() const
{
	return (raw_spell.id == 1546);
}

bool IsPlayerIllusionSpell(int16 spell_id) {
	if(IsEffectInSpell(spell_id, SE_Illusion) && spells[spell_id].targettype == ST_Self)
		return true;
	else
		return false;

}

bool Spell::IsPlayerIllusionSpell() const
{
	if(this->IsEffectInSpell(SE_Illusion) && raw_spell.targettype == ST_Self)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int GetSpellEffectDescNum(int16 spell_id)
{
	if( (spell_id > 0) && (spell_id < SPDAT_RECORDS) ){
		return spells[spell_id].effectdescnum;
	} else {
		return -1;
	}
}

DmgShieldType GetDamageShieldType(int16 spell_id) 
{

	// If we have a DamageShieldType for this spell from the damageshieldtypes table, return that,
	// else, make a guess, based on the resist type. Default return value is DS_THORNS
	//
	if( (spell_id > 0) && (spell_id < SPDAT_RECORDS) ){

		_log(SPELLS__EFFECT_VALUES, "DamageShieldType for spell %i (%s) is %X\n", spell_id, 
			spells[spell_id].name, spells[spell_id].DamageShieldType); 

		if(spells[spell_id].DamageShieldType)
			return (DmgShieldType) spells[spell_id].DamageShieldType;

		switch(spells[spell_id].resisttype) {
			case RESIST_COLD:
				return DS_TORMENT;
			case RESIST_FIRE:
				return DS_BURN;
			case RESIST_DISEASE:
				return DS_DECAY;
			default:
				return DS_THORNS;
		}
	}

	return DS_THORNS;
}

bool IsLDoNObjectSpell(int16 spell_id) 
{
	if(IsEffectInSpell(spell_id, SE_AppraiseLDonChest) || 
		IsEffectInSpell(spell_id, SE_DisarmLDoNTrap) || 
		IsEffectInSpell(spell_id, SE_UnlockLDoNChest))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Spell::IsLDoNObjectSpell() const
{
	if(IsEffectInSpell(SE_AppraiseLDonChest) || 
		IsEffectInSpell(SE_DisarmLDoNTrap) || 
		IsEffectInSpell(SE_UnlockLDoNChest))
	{
		return true;
	}
	else
	{
		return false;
	}
}

sint32 GetSpellResistType(int16 spell_id)
{
	return spells[spell_id].resisttype;
}

sint32 GetSpellTargetType(int16 spell_id)
{
	return (sint32)spells[spell_id].targettype;
}

bool IsCompleteHealSpell(int16 spell_id) {
	
	if(spell_id == 13 || IsEffectInSpell(spell_id, SE_CompleteHeal) || IsEffectInSpell(spell_id, SE_PercentalHeal))
		return true;
	else
		return false;
}

bool Spell::IsCompleteHealSpell() const
{
	if(raw_spell.id == 13 || IsEffectInSpell(SE_CompleteHeal) || IsEffectInSpell(SE_PercentalHeal))
		return true;

	return false;
}

bool IsFastHealSpell(int16 spell_id) {
	const int MaxFastHealCastingTime = 2000;

	if(spells[spell_id].cast_time <= MaxFastHealCastingTime && spells[spell_id].effectid[0] == 0 && spells[spell_id].base[0] > 0)
		return true;
	else
		return false;
}

bool IsRegularSingleTargetHealSpell(int16 spell_id) {
	bool result = false;

	if(spells[spell_id].effectid[0] == 0 && spells[spell_id].base[0] > 0 && spells[spell_id].targettype == ST_Target
		&& !IsFastHealSpell(spell_id) && !IsCompleteHealSpell(spell_id) && !IsEffectInSpell(spell_id, SE_HealOverTime)) {
		result = true;
	}

	return result;
}

uint32 Spell::GetMorphTrigger() const
{
	for(int i = 0; i < EFFECT_COUNT; i++)
	{
		if(raw_spell.effectid[i] == SE_ImprovedSpellEffect)
		{
			return raw_spell.base[i];
		}
	}

	return 0xFFFFFFFF;
}