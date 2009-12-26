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

#include "../common/types.h"
#include "../common/timer.h"
#include "spdat.h"
#include <vector>

struct SpellEffect
{
	uint32 effect_id;
	uint32 base1;
	uint32 base2;
	uint32 max;
	uint32 formula;
};

class Spell
{
public:
	Spell(uint32 spell_id, Mob* caster, Mob* target, uint32 slot = 10, uint32 cast_time = -1, uint32 mana_cost = -1);
	~Spell();
	
	void SetSpellID(uint32 id) { spell_id = id; }
	uint32 GetSpellID() const { return spell_id; }

	//The spell will automatically fill in effects on construction but if you wish to manually modify, add or remove effects constructs are available to do so.
	void AddEffect(uint8 slot, uint32 effect_id, uint32 base1, uint32 base2, uint32 max, uint32 formula)
	{ 
		effect_container[slot]->effect_id = effect_id;
		effect_container[slot]->base1 = base1; 
		effect_container[slot]->base2 = base2; 
		effect_container[slot]->max = max; 
		effect_container[slot]->formula = formula; 
	}
	void DestroyEffect(uint8 slot) { effect_container[slot]->effect_id = SE_Blank; }
	const SpellEffect * GetEffect(uint32 slot) const { return effect_container[slot]; }
	
	void SetCaster(Mob *c) { caster = c; }
	Mob *GetCaster() const { return caster; }

	void SetTarget(Mob *t) { target = t; }
	Mob *GetTarget() const { return target; }

	void SetSpellSlot(sint32 slot) { spell_slot = slot; }
	sint32 GetSpellSlot() const { return spell_slot; }

	void SetInventorySpellSlot(uint32 slot) { spell_slot_inventory = slot; }
	uint32 GetInventorySpellSlot() const { return spell_slot_inventory; }
	
	void SetManaCost(sint32 cost) { mana_cost = cost; }
	sint32 GetManaCost() const { return mana_cost; }

	void SetCastTime(sint32 time) { cast_time = time; }
	sint32 GetCastTime() const { return cast_time; }

	void SetDuration(uint32 duration) { spell_duration = duration; }
	uint32 GetDuration() const { return spell_duration; }

	//The timer id this mob/player will trigger when it finishes, 0 = ignored.
	void SetTimerID(uint32 timer) { timer_id = timer; }
	uint32 GetTimerID() const { return timer_id; }

	void SetTimerDuration(uint32 time) { timer_duration = time; }
	uint32 GetTimerDuration() const { return timer_duration; }

	//A permanant duration spell is supposed to last forever and never tic down, the client does this via a special duration send
	void SetPermanantDuration(bool d) { is_perm_duration = d; if(d) { tic_timer->Disable(); } }
	bool IsPermanantDuration() const { return is_perm_duration; }
	
	//The first four are for counters via dispel the last is for the numhits modifier in the spell field.
	sint32 GetRemainingChargesMagic() const { return magic_remaining_charges; }
	sint32 GetRemainingChargesPoison() const { return poison_remaining_charges; }
	sint32 GetRemainingChargesDisease() const { return disease_remaining_charges; }
	sint32 GetRemainingChargesCurse() const { return curse_remaining_charges; }
	sint32 GetRemainingCharges() const { return general_remaining_charges; }
	
	void AddRemainingChargesMagic(sint32 charges) { magic_remaining_charges += charges; }
	void AddRemainingChargesPoison(sint32 charges) { poison_remaining_charges += charges; }
	void AddRemainingChargesDisease(sint32 charges) { disease_remaining_charges += charges; }
	void AddRemainingChargesCurse(sint32 charges) { curse_remaining_charges += charges; }
	void AddRemainingCharges(sint32 charges) { general_remaining_charges += charges; }
	
	void SetRemainingChargesMagic(sint32 charges) { magic_remaining_charges = charges; }
	void SetRemainingChargesPoison(sint32 charges) { poison_remaining_charges = charges; }
	void SetRemainingChargesDisease(sint32 charges) { disease_remaining_charges = charges; }
	void SetRemainingChargesCurse(sint32 charges) { curse_remaining_charges = charges; }
	void SetRemainingCharges(sint32 charges) { general_remaining_charges = charges; }

	void SetDeathSaveChance(uint8 chance) { death_save_chance = chance; }
	uint8 GetDeathSaveChance() const { return death_save_chance; }

	void SetCasterAARank(uint8 rank) { caster_aa_rank = rank; }
	uint8 GetCasterAARank() const { return caster_aa_rank; }
	
protected:
	uint32 spell_id;
	uint16 caster_level;
	Mob *caster;
	Mob *target;
	uint32 spell_slot;
	uint32 spell_slot_inventory;
	sint32 mana_cost;
	sint32 cast_time;
	uint32 spell_duration;
	Timer * cast_timer;
	Timer * tic_timer;
	bool is_perm_duration;
	bool is_perm_illusion;
	uint32 timer_id;
	uint32 timer_duration;
	
	sint32 magic_remaining_charges;
	sint32 poison_remaining_charges;
	sint32 disease_remaining_charges;
	sint32 curse_remaining_charges;
	sint32 general_remaining_charges;
	
	uint32 melee_shield_remaining;
	uint32 magic_shield_remaining;
	uint8 death_save_chance;
	uint8 caster_aa_rank;
	
	std::vector<SpellEffect*> effect_container;
	std::list<Mob*> target_container;
};

#endif