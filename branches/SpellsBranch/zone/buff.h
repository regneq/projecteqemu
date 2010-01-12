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

#ifndef EQEMU_BUFF_H
#define EQEMU_BUFF_H

#include "spells.h"

class Spell;

class Buff
{
public:
	//Buff(Spell* spell);
	//~Buff();

	void SetDurationRemaining(uint32 duration) { spell_duration_remaining = duration; }
	uint32 GetDurationRemaining() const { return spell_duration_remaining; }

	//A permanant duration spell is supposed to last forever and never tic down, the client does this via a special duration send
	void SetPermanantDuration(bool d) { is_perm_duration = d; }
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

	void SetMeleeShield(sint32 shield) { melee_shield_remaining = shield; }
	void AddMeleeShield(sint32 shield) { melee_shield_remaining += shield; }
	sint32 GetMeleeShield() const { return melee_shield_remaining; }

	void SetMagicShield(sint32 shield) { magic_shield_remaining = shield; }
	void AddMagicShield(sint32 shield) { magic_shield_remaining += shield; }
	sint32 GetMagicShield() const { return magic_shield_remaining; }

	void SetDeathSaveChance(uint8 chance) { death_save_chance = chance; }
	uint8 GetDeathSaveChance() const { return death_save_chance; }

	void SetCasterAARank(uint8 rank) { caster_aa_rank = rank; }
	uint8 GetCasterAARank() const { return caster_aa_rank; }

	const Spell* GetSpell() const { return spell; }

protected:
	uint32 spell_duration_remaining;
	bool is_perm_duration;
	bool is_perm_illusion;

	sint32 magic_remaining_charges;
	sint32 poison_remaining_charges;
	sint32 disease_remaining_charges;
	sint32 curse_remaining_charges;
	sint32 general_remaining_charges;
	
	sint32 melee_shield_remaining;
	sint32 magic_shield_remaining;
	uint8 death_save_chance;
	uint8 caster_aa_rank;

	Spell *spell;
};

#endif