/*  EQEMu:  Everquest Server Emulator
Copyright (C) 2001-2010  EQEMu Development Team (http://eqemu.org)

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

#ifndef EQEMU_SPELLS_H
#define EQEMU_SPELLS_H

#include "../common/debug.h"
#include "../common/types.h"
#include "../common/timer.h"
#include "spdat.h"
#include "buff.h"
#include <string>
#include <map>

enum SpellClass
{
	SC_NORMAL = 0,
	SC_DISC,
	SC_AA,
	SC_PROC,
};

class Spell
{
public:
	Spell();
	Spell(uint32 spell_id, Mob* caster, Mob* target, uint32 slot = 10, uint32 cast_time = -1, uint32 mana_cost = -1);
	Spell(SPDat_Spell_Struct *new_spell, uint32 caster_level = 0, uint32 slot = 10, uint32 inventory_slot = 0xFFFFFFFF, uint32 spell_type = 0);
	~Spell();
	
	void SetSpellID(uint32 id) {  raw_spell.id = id; }
	uint32 GetSpellID() const { return raw_spell.id; }

	void SetCaster(Mob *c);
	Mob *GetCaster() const;
	uint32 GetCasterID() const { return caster_id; }

	void SetTarget(Mob *t);
	Mob *GetTarget() const;
	uint32 GetTargetID() const { return target_id; }

	void SetSpellSlot(sint32 slot) { spell_slot = slot; }
	sint32 GetSpellSlot() const { return spell_slot; }
	
	void SetManaCost(sint32 cost) { mana_cost = cost; }
	sint32 GetManaCost() const { return mana_cost; }

	void SetCastTime(sint32 time) { cast_time = time; }
	sint32 GetCastTime() const { return cast_time; }

	void SetInventorySpellSlot(uint32 slot) { spell_slot_inventory = slot; }
	uint32 GetInventorySpellSlot() const { return spell_slot_inventory; }

	//The timer id this mob/player will trigger when it finishes, 0 = ignored.
	void SetTimerID(uint32 timer) { timer_id = timer; }
	uint32 GetTimerID() const { return timer_id; }

	void SetTimerIDDuration(uint32 duration) { timer_id_duration = duration; }
	uint32 GetTimerIDDuration() const { return timer_id_duration; }

	void SetSpellType(SpellClass type) { spell_class_type = type; }
	SpellClass GetSpellType() const { return spell_class_type; }

	uint32 GetCasterLevel() const { return caster_level; }
	void SetCasterLevel(uint32 level) { caster_level = level; }

	bool IsCustomSpell() const { return custom_data; }
	void SetCustomSpell(bool c) { custom_data = c; }
	
	Spell* CopySpell();

	void StartCastTimer(uint32 duration);
	bool IsCastTimerFinished() const;
	void StopCastTimer();

	const SPDat_Spell_Struct GetSpell() const { return raw_spell; }
	SPDat_Spell_Struct &GetSpellEditable() { return raw_spell; }

	//These aren't fast because of a lot of string comparisons
	//but they're designed ONLY for the quest interface
	//If you're using these in raw C++ code: you're doing it wrong.
	//Use GetSpell() & GetSpellEditable() instead.
	std::string GetSpellAttribute(std::string field) const;
	void SetSpellAttribute(std::string attribute, std::string field);

	bool IsTargetableAESpell() const; //impl
	bool IsLifetapSpell() const; //impl
	bool IsSlowSpell() const; //impl
	bool IsHasteSpell() const; //impl
	bool IsHarmonySpell() const; //impl
	bool IsGroupOnlySpell() const; //impl
	bool IsBeneficialSpell() const; //impl
	bool IsDetrimentalSpell() const; //impl
	bool IsSummonPetSpell() const; //impl
	bool IsAEDurationSpell() const; //impl
	bool IsPureNukeSpell() const; //impl
	bool IsPartialCapableSpell() const; //impl
	bool IsResistableSpell() const; //impl
	bool IsGroupSpell() const; //impl
	bool IsTGBCompatibleSpell() const; //impl
	bool IsBardSong() const; //impl
	bool IsEffectInSpell(int effect) const; //impl
	bool IsBlankSpellEffect(int effect_index) const; //impl
	bool IsSummonSpell() const; //impl
	bool IsDamageSpell() const; //impl
	bool BeneficialSpell() const; //impl
	int GetSpellEffectIndex(int effect) const; //impl
	int GetMinLevel() const; //impl
	sint32 CalculatePoisonCounters() const; //impl
	sint32 CalculateDiseaseCounters() const; //impl
	sint32 CalculateCurseCounters() const; //impl
	bool IsDiscipline() const; //impl
	bool IsResurrectionEffects() const; //impl
	bool IsManaTapSpell() const; //impl
	bool IsFullDeathSaveSpell() const; // impl
	bool IsPlayerIllusionSpell() const; //impl
	bool IsLDoNObjectSpell() const; //impl
	bool IsCompleteHealSpell() const; //impl
	uint32 GetMorphTrigger() const;

protected:
	uint32 caster_level;
	uint32 caster_id;
	uint32 target_id;
	uint32 spell_slot;
	uint32 spell_slot_inventory;
	sint32 cast_time;
	sint32 mana_cost;
	Timer * cast_timer;
	uint32 timer_id;
	uint32 timer_id_duration;
	SpellClass spell_class_type;
	bool custom_data;

	SPDat_Spell_Struct raw_spell;
};

#endif