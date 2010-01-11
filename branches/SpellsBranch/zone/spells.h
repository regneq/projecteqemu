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

enum SpellAttribute
{
	SA_NONE,
};

class Spell
{
public:
	Spell(uint32 spell_id, Mob* caster, Mob* target, uint32 slot = 10, uint32 cast_time = -1, uint32 mana_cost = -1);
	~Spell();
	
	void SetSpellID(uint32 id) { spell_id = id; }
	uint32 GetSpellID() const { return spell_id; }

	void SetCaster(Mob *c);
	Mob *GetCaster() const;

	void SetTarget(Mob *t);
	Mob *GetTarget() const;

	void SetSpellSlot(sint32 slot) { spell_slot = slot; }
	sint32 GetSpellSlot() const { return spell_slot; }
	
	void SetManaCost(sint32 cost) { mana_cost = cost; }
	sint32 GetManaCost() const { return mana_cost; }

	void SetCastTime(sint32 time) { cast_time = time; }
	sint32 GetCastTime() const { return cast_time; }

	void SetInventorySpellSlot(uint32 slot) { spell_slot_inventory = slot; }
	uint32 GetInventorySpellSlot() const { return spell_slot_inventory; }

	//The timer id this mob/player will trigger when it finishes, 0 = ignored.
	void SetTimerID(sint32 timer) { timer_id = timer; }
	sint32 GetTimerID() const { return timer_id; }

	void SetTimerIDDuration(sint32 duration) { timer_id_duration = duration; }
	sint32 GetTimerIDDuration() const { return timer_id_duration; }
	
	Buff *CreateBuff() { return NULL; }

	void StartCastTimer(uint32 duration);
	bool IsCastTimerFinished() const;
	void StopCastTimer();

protected:
	uint32 spell_id;
	uint16 caster_level;
	uint32 caster_id;
	uint32 target_id;
	uint32 spell_slot;
	uint32 spell_slot_inventory;
	sint32 cast_time;
	sint32 mana_cost;
	Timer * cast_timer;
	sint32 timer_id;
	sint32 timer_id_duration;

	std::map<SpellAttribute, uint32> sa_container_uint32;
	std::map<SpellAttribute, sint32> sa_container_sint32;
	std::map<SpellAttribute, double> sa_container_double;
};

#endif