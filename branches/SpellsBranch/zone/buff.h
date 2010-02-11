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

namespace BuffStorage
{
	enum BuffStorageType
	{
		BUFF_ST_CHARACTER = 0,
		BUFF_ST_PET,
		BUFF_ST_SUSPENDED_PET,
		BUFF_ST_BOT,
	};

#pragma pack(1)
	struct buff_header
	{
	//256 versions
	//More than we will ever need unless we're stupid
	/* 000 */ uint8 version_identifier;
	
	//65535 possible buffs
	//More than we will ever need
	/* 001 */ uint16 number_of_buffs;
	};
#pragma pack()

	namespace Version1
	{
#pragma pack(1)
		struct buff_entry
		{
			uint8 buff_slot;
			uint32 spell_id;
			uint32 duration;
			bool is_perm_illusion;
			bool is_client;
			sint32 corruption_remaining_charges;
			sint32 poison_remaining_charges;
			sint32 disease_remaining_charges;
			sint32 curse_remaining_charges;
			sint32 general_remaining_charges;
			sint32 melee_shield_remaining;
			sint32 magic_shield_remaining;
			sint32 melee_shield_reduction;
			sint32 magic_shield_reduction;
			sint32 attacks_negated;
			uint8 death_save_chance;
			uint8 caster_aa_rank;
			uint32 instrument_mod;
			uint32 caster_level;
			uint32 spell_slot;
			uint32 spell_slot_inventory;
			uint32 spell_class_type;
			bool is_custom_spell;
		};
#pragma pack()
	}
}

class Spell;

class Buff
{
public:
	Buff(Spell* spell, uint32 duration);
	~Buff(){ safe_delete(buff_spell); }

	void SetDurationRemaining(uint32 duration) { spell_duration_remaining = duration; }
	uint32 GetDurationRemaining() const { return spell_duration_remaining; }
	bool IsPermanentDuration() const { return is_perm_duration; }

	void SetIsClientBuff(bool c) { is_client = c; }
	bool IsClientBuff() const { return is_client; }

	void SetPermanentIllusion(bool b) { is_perm_illusion = b; }
	bool IsPermanentIllusion() const { return is_perm_illusion; }
	
	//The first four are for counters via dispel the last is for the numhits modifier in the spell field.
	sint32 GetRemainingChargesCorruption() const { return corruption_remaining_charges; }
	sint32 GetRemainingChargesPoison() const { return poison_remaining_charges; }
	sint32 GetRemainingChargesDisease() const { return disease_remaining_charges; }
	sint32 GetRemainingChargesCurse() const { return curse_remaining_charges; }
	sint32 GetRemainingCharges() const { return general_remaining_charges; }
	
	void AddRemainingChargesCorruption(sint32 charges) { corruption_remaining_charges += charges; }
	void AddRemainingChargesPoison(sint32 charges) { poison_remaining_charges += charges; }
	void AddRemainingChargesDisease(sint32 charges) { disease_remaining_charges += charges; }
	void AddRemainingChargesCurse(sint32 charges) { curse_remaining_charges += charges; }
	void AddRemainingCharges(sint32 charges) { general_remaining_charges += charges; }
	
	void SetRemainingChargesCorruption(sint32 charges) { corruption_remaining_charges = charges; }
	void SetRemainingChargesPoison(sint32 charges) { poison_remaining_charges = charges; }
	void SetRemainingChargesDisease(sint32 charges) { disease_remaining_charges = charges; }
	void SetRemainingChargesCurse(sint32 charges) { curse_remaining_charges = charges; }
	void SetRemainingCharges(sint32 charges) { general_remaining_charges = charges; }

	void SetMeleeShield(sint32 shield) { melee_shield_remaining = shield; }
	void SetMeleeShieldReduction(sint32 percentage) { melee_shield_reduction = percentage; }
	sint32 GetMeleeShield() const { return melee_shield_remaining; }
	sint32 GetMeleeShieldReduction() const { return melee_shield_reduction; }

	void SetMagicShield(sint32 shield) { magic_shield_remaining = shield; }
	void SetMagicShieldReduction(sint32 percentage) { magic_shield_reduction = percentage; }
	sint32 GetMagicShield() const { return magic_shield_remaining; }
	sint32 GetMagicShieldReduction() const { return magic_shield_reduction; }

	void SetAttacksNegated(sint32 attack_num) { attacks_negated = attack_num; }	
	sint32 GetAttacksNegated() { return attacks_negated; }

	void SetDeathSaveChance(uint8 chance) { death_save_chance = chance; }
	uint8 GetDeathSaveChance() const { return death_save_chance; }

	void SetCasterAARank(uint8 rank) { caster_aa_rank = rank; }
	uint8 GetCasterAARank() const { return caster_aa_rank; }

	void SetInstrumentMod(uint32 mod) { instrument_mod = mod; }
	uint32 GetInstrumentMod() const { return instrument_mod; }

	const Spell* GetSpell() const { return buff_spell; }

	uint32 GetCasterID() const;


protected:
	uint32 spell_duration_remaining;
	bool is_perm_duration;	
	bool is_client;	
	bool is_perm_illusion;

	sint32 corruption_remaining_charges;
	sint32 poison_remaining_charges;
	sint32 disease_remaining_charges;
	sint32 curse_remaining_charges;
	sint32 general_remaining_charges;
	
	sint32 melee_shield_remaining;
	sint32 magic_shield_remaining;
	sint32 melee_shield_reduction;
	sint32 magic_shield_reduction;
	sint32 attacks_negated;
	uint8 death_save_chance;
	uint8 caster_aa_rank;
	uint32 instrument_mod;

	Spell *buff_spell;
};

#endif