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
#include <vector>

struct SpellEffect
{
	uint8 slot;
	uint32 effect_id;
	uint32 base1;
	uint32 base2;
	uint32 max;
	uint32 formula;
};

class Spell
{
public:
	Spell(uint32 spell_id);
	Spell(uint32 spell_id, uint32 duration);
	Spell(uint32 spell_id, uint32 duration, uint32 caster_level);
	~Spell();
	
	//The spell will automatically fill in effects on construction but if you wish to manually modify, add or remove effects constructs are available to do so.
	void AddEffect(uint8 slot, uint32 effect_id, uint32 base1, uint32 base2, uint32 max, uint32 formula);
	void UpdateEffect(uint8 slot, uint32 effect_id, uint32 base1, uint32 base2, uint32 max, uint32 formula);
	void DestroyEffect(uint8 slot);
	const SpellEffect * GetEffect(uint32 slot) const;
	
	//The names should be fairly self-explanitory.
	void SetDuration(uint32 duration);
	uint32 GetDuration() const;

	//The timer id this mob/player will trigger when it finishes, 0 = ignored.
	void SetTimerID(uint32 timer);
	uint32 GetTimerID() const;

	//A permanant duration spell is supposed to last forever and never tic down, the client does this via a special duration send
	void SetPermanantDuration(bool d);
	bool IsPermanantDuration() const;
	
	//The first four are for counters via dispel the last is for the numhits modifier in the spell field.
	sint32 GetRemainingChargesMagic() const;
	sint32 GetRemainingChargesPoison() const;
	sint32 GetRemainingChargesDisease() const;
	sint32 GetRemainingChargesCurse() const;
	sint32 GetRemainingCharges() const;
	
	void AddRemainingChargesMagic(sint32 charges);
	void AddRemainingChargesPoison(sint32 charges);
	void AddRemainingChargesDisease(sint32 charges);
	void AddRemainingChargesCurse(sint32 charges);
	void AddRemainingCharges(sint32 charges);
	
	void SetRemainingChargesMagic(sint32 charges);
	void SetRemainingChargesPoison(sint32 charges);
	void SetRemainingChargesDisease(sint32 charges);
	void SetRemainingChargesCurse(sint32 charges);
	void SetRemainingCharges(sint32 charges);

	void SetDeathSaveChance(uint8 chance);
	uint8 GetDeathSaveChance() const;

	void SetCasterAARank(uint8 rank);
	uint8 GetCasterAARank() const;
	
protected:
	uint32 spell_id;
	uint16 caster_level;
	Mob *caster;

	Timer * tic_timer;
	bool is_perm_duration;
	bool is_perm_illusion;
	uint32 timer_id;
	
	sint32 magic_charges_remaining;
	sint32 poison_charges_remaining;
	sint32 disease_charges_remaining;
	sint32 curse_charges_remaining;
	sint32 general_charges_remaining;
	
	uint32 melee_shield_remaining;
	uint32 magic_shield_remaining;
	uint8 death_save_chance;
	uint8 caster_aa_rank;
	
	std::vector<SpellEffect*> effect_container;
	std::list<Mob*> target_container;
};

#endif