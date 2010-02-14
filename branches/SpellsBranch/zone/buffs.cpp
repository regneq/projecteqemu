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
#include "../common/rulesys.h"
#ifndef WIN32
#include <stdlib.h>
#include "../common/unix.h"
#endif

///////////////////////////////////////////////////////////////////////////////
// buff related functions
// returns how many _ticks_ the buff will last.
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
	if(!buffs)
	{
		return NULL;
	}

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

bool Mob::WillSpellHold(Spell *spell_to_cast, Mob *spell_target)
{
	if(!(IsClient() && CastToClient()->GetGM()) && !spell_to_cast->IsHarmonySpell()) // GMs can cast on anything
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

void Mob::SetBuffCount(uint32 new_buff_count)
{
	if(current_buff_count == 0 && new_buff_count > 0)
	{
		buff_tic_timer = new Timer(6000);
	}
	else if(current_buff_count > 0 && new_buff_count == 0)
	{
		safe_delete(buff_tic_timer);
	}
	current_buff_count = new_buff_count; 
}

void Client::SaveBuffs(uint8 mode)
{
	//We only save the first 25 buffs.
	//The pp doesn't support more than 25 buffs so
	//we don't try to save anymore than that for now.
	//(Maybe we can find a field for spells/disc later!)
	uint32 total_size = sizeof(BuffStorage::buff_header);
	uint32 buff_count = 0;
	for(int i = 0; i < 25; i++)
	{
		if(buffs[i])
		{
			total_size += sizeof(BuffStorage::Version1::buff_entry);
			if(buffs[i]->GetSpell()->IsCustomSpell())
			{
				total_size += sizeof(SPDat_Spell_Struct);
			}
			buff_count++;
		}
	}

	char *data = new char[total_size];
	uint32 offset = 0;

	BuffStorage::buff_header header;
	header.version_identifier = 1;
	header.number_of_buffs = buff_count;

	memcpy(data + offset, &header, sizeof(BuffStorage::buff_header));
	offset += sizeof(BuffStorage::buff_header);

	for(int i = 0; i < 25; i++)
	{
		if(buffs[i])
		{
			BuffStorage::Version1::buff_entry buff_entry;
			buff_entry.buff_slot = i;
			buff_entry.spell_id = buffs[i]->GetSpell()->GetSpellID();
			buff_entry.duration = buffs[i]->GetDurationRemaining();
			buff_entry.is_perm_illusion = buffs[i]->IsPermanentIllusion();
			buff_entry.is_client = buffs[i]->IsClientBuff();
			buff_entry.corruption_remaining_charges = buffs[i]->GetRemainingChargesCorruption();
			buff_entry.poison_remaining_charges = buffs[i]->GetRemainingChargesPoison();
			buff_entry.disease_remaining_charges = buffs[i]->GetRemainingChargesDisease();
			buff_entry.curse_remaining_charges = buffs[i]->GetRemainingChargesCurse();
			buff_entry.general_remaining_charges = buffs[i]->GetRemainingCharges();
			buff_entry.melee_shield_remaining = buffs[i]->GetMeleeShield();
			buff_entry.melee_shield_reduction = buffs[i]->GetMeleeShieldReduction();
			buff_entry.magic_shield_remaining = buffs[i]->GetMagicShield();
			buff_entry.magic_shield_reduction = buffs[i]->GetMagicShieldReduction();
			buff_entry.attacks_negated = buffs[i]->GetAttacksNegated();
			buff_entry.death_save_chance = buffs[i]->GetDeathSaveChance();
			buff_entry.caster_aa_rank = buffs[i]->GetCasterAARank();
			buff_entry.instrument_mod = buffs[i]->GetInstrumentMod();
			buff_entry.caster_level = buffs[i]->GetSpell()->GetCasterLevel();
			buff_entry.spell_slot = buffs[i]->GetSpell()->GetSpellSlot();
			buff_entry.spell_slot_inventory = buffs[i]->GetSpell()->GetInventorySpellSlot();
			buff_entry.spell_class_type = buffs[i]->GetSpell()->GetSpellType();	
			buff_entry.is_custom_spell = buffs[i]->GetSpell()->IsCustomSpell();
			memcpy(data + offset, &buff_entry, sizeof(BuffStorage::Version1::buff_entry));
			offset += sizeof(BuffStorage::Version1::buff_entry);

			if(buff_entry.is_custom_spell)
			{
				memcpy(data + offset, &buffs[i]->GetSpell()->GetSpell(), sizeof(SPDat_Spell_Struct));
				offset += sizeof(SPDat_Spell_Struct);
			}
		}
	}
	database.SetBuff(CharacterID(), BuffStorage::BUFF_ST_CHARACTER, data, total_size);
}

void NPC::SaveBuffs(uint8 mode)
{
	Mob *owner = GetOwner();
	
	if(owner)
	{
		if(!owner->IsClient())
		{
			return;
		}
	}
	else
	{
		return;
	}

	uint32 total_size = sizeof(BuffStorage::buff_header);
	uint32 buff_count = 0;
	for(int i = 0; i < 25; i++)
	{
		if(buffs[i])
		{
			total_size += sizeof(BuffStorage::Version1::buff_entry);
			if(buffs[i]->GetSpell()->IsCustomSpell())
			{
				total_size += sizeof(SPDat_Spell_Struct);
			}
			buff_count++;
		}
	}

	char *data = new char[total_size];
	uint32 offset = 0;

	BuffStorage::buff_header header;
	header.version_identifier = 1;
	header.number_of_buffs = buff_count;

	memcpy(data + offset, &header, sizeof(BuffStorage::buff_header));
	offset += sizeof(BuffStorage::buff_header);

	uint32 max_count = GetMaxTotalSlots();
	for(int i = 0; i < max_count; i++)
	{
		if(buffs[i])
		{
			BuffStorage::Version1::buff_entry buff_entry;
			buff_entry.buff_slot = i;
			buff_entry.spell_id = buffs[i]->GetSpell()->GetSpellID();
			buff_entry.duration = buffs[i]->GetDurationRemaining();
			buff_entry.is_perm_illusion = buffs[i]->IsPermanentIllusion();
			buff_entry.is_client = buffs[i]->IsClientBuff();
			buff_entry.corruption_remaining_charges = buffs[i]->GetRemainingChargesCorruption();
			buff_entry.poison_remaining_charges = buffs[i]->GetRemainingChargesPoison();
			buff_entry.disease_remaining_charges = buffs[i]->GetRemainingChargesDisease();
			buff_entry.curse_remaining_charges = buffs[i]->GetRemainingChargesCurse();
			buff_entry.general_remaining_charges = buffs[i]->GetRemainingCharges();
			buff_entry.melee_shield_remaining = buffs[i]->GetMeleeShield();
			buff_entry.melee_shield_reduction = buffs[i]->GetMeleeShieldReduction();
			buff_entry.magic_shield_remaining = buffs[i]->GetMagicShield();
			buff_entry.magic_shield_reduction = buffs[i]->GetMagicShieldReduction();
			buff_entry.attacks_negated = buffs[i]->GetAttacksNegated();
			buff_entry.death_save_chance = buffs[i]->GetDeathSaveChance();
			buff_entry.caster_aa_rank = buffs[i]->GetCasterAARank();
			buff_entry.instrument_mod = buffs[i]->GetInstrumentMod();
			buff_entry.caster_level = buffs[i]->GetSpell()->GetCasterLevel();
			buff_entry.spell_slot = buffs[i]->GetSpell()->GetSpellSlot();
			buff_entry.spell_slot_inventory = buffs[i]->GetSpell()->GetInventorySpellSlot();
			buff_entry.spell_class_type = buffs[i]->GetSpell()->GetSpellType();	
			buff_entry.is_custom_spell = buffs[i]->GetSpell()->IsCustomSpell();
			memcpy(data + offset, &buff_entry, sizeof(BuffStorage::Version1::buff_entry));
			offset += sizeof(BuffStorage::Version1::buff_entry);

			if(buff_entry.is_custom_spell)
			{
				memcpy(data + offset, &buffs[i]->GetSpell()->GetSpell(), sizeof(SPDat_Spell_Struct));
				offset += sizeof(SPDat_Spell_Struct);
			}
		}
	}
	database.SetBuff(owner->CastToClient()->CharacterID(), mode == 0 ? BuffStorage::BUFF_ST_PET : BuffStorage::BUFF_ST_SUSPENDED_PET,
		data, total_size);
}

void Client::LoadBuffs(uint8 mode)
{
	char *data = database.GetBuff(CharacterID(), BuffStorage::BUFF_ST_CHARACTER);
	if(data)
	{
		BuffStorage::buff_header *header = (BuffStorage::buff_header *)data;
		switch(header->version_identifier)
		{
		case 1:
			{
				LoadBuffsVersion1(data);
				break;
			}
		default:
			{
			}
		}
		safe_delete_array(data);
	}
}

void NPC::LoadBuffs(uint8 mode)
{
	Mob *owner = GetOwner();
	
	if(owner)
	{
		if(!owner->IsClient())
		{
			return;
		}
	}
	else
	{
		return;
	}

	char *data = database.GetBuff(owner->CastToClient()->CharacterID(), mode == 0 ? BuffStorage::BUFF_ST_PET : BuffStorage::BUFF_ST_SUSPENDED_PET);
	if(data)
	{
		BuffStorage::buff_header *header = (BuffStorage::buff_header *)data;
		switch(header->version_identifier)
		{
		case 1:
			{
				LoadBuffsVersion1(data);
				break;
			}
		default:
			{
			}
		}
		safe_delete_array(data);
	}
}

void Mob::LoadBuffsVersion1(char *data)
{
	char *data_ptr = data;
	BuffStorage::buff_header *header = (BuffStorage::buff_header *)data_ptr;
	data_ptr += sizeof(BuffStorage::buff_header);

	for(int i = 0; i < header->number_of_buffs; i++)
	{
		BuffStorage::Version1::buff_entry *buff_entry = (BuffStorage::Version1::buff_entry *) data_ptr;

		if(buff_entry->is_custom_spell == 0)
		{
			Spell *new_spell = new Spell(buff_entry->spell_id, NULL, this, buff_entry->spell_slot);
			new_spell->SetInventorySpellSlot(buff_entry->spell_slot_inventory);
			new_spell->SetCasterLevel(buff_entry->caster_level);
			new_spell->SetSpellType((SpellClass)buff_entry->spell_class_type);
			buffs[buff_entry->buff_slot] = new Buff(new_spell, buff_entry->duration);
			buffs[buff_entry->buff_slot]->SetIsClientBuff(buff_entry->is_client);
			buffs[buff_entry->buff_slot]->SetPermanentIllusion(buff_entry->is_perm_illusion);
			buffs[buff_entry->buff_slot]->SetRemainingChargesCorruption(buff_entry->corruption_remaining_charges);
			buffs[buff_entry->buff_slot]->SetRemainingChargesPoison(buff_entry->poison_remaining_charges);
			buffs[buff_entry->buff_slot]->SetRemainingChargesDisease(buff_entry->disease_remaining_charges);
			buffs[buff_entry->buff_slot]->SetRemainingChargesCurse(buff_entry->curse_remaining_charges);
			buffs[buff_entry->buff_slot]->SetRemainingCharges(buff_entry->general_remaining_charges);
			buffs[buff_entry->buff_slot]->SetMeleeShield(buff_entry->melee_shield_remaining);
			buffs[buff_entry->buff_slot]->SetMagicShield(buff_entry->magic_shield_remaining);
			buffs[buff_entry->buff_slot]->SetMeleeShieldReduction(buff_entry->melee_shield_reduction);
			buffs[buff_entry->buff_slot]->SetMagicShieldReduction(buff_entry->magic_shield_reduction);
			buffs[buff_entry->buff_slot]->SetAttacksNegated(buff_entry->attacks_negated);
			buffs[buff_entry->buff_slot]->SetDeathSaveChance(buff_entry->death_save_chance);
			buffs[buff_entry->buff_slot]->SetCasterAARank(buff_entry->caster_aa_rank);
			buffs[buff_entry->buff_slot]->SetInstrumentMod(buff_entry->instrument_mod);
			
			safe_delete(new_spell);
			data_ptr += sizeof(BuffStorage::Version1::buff_entry);
		}
		else
		{
			data_ptr += sizeof(BuffStorage::Version1::buff_entry);
			SPDat_Spell_Struct *spell_data = (SPDat_Spell_Struct *)data_ptr;
			
			Spell *new_spell = new Spell(spell_data, buff_entry->caster_level, buff_entry->spell_slot, 
				buff_entry->spell_slot_inventory, buff_entry->spell_class_type);
			new_spell->SetTarget(this);
			buffs[buff_entry->buff_slot] = new Buff(new_spell, buff_entry->duration);
			buffs[buff_entry->buff_slot]->SetIsClientBuff(buff_entry->is_client);
			buffs[buff_entry->buff_slot]->SetPermanentIllusion(buff_entry->is_perm_illusion);
			buffs[buff_entry->buff_slot]->SetRemainingChargesCorruption(buff_entry->corruption_remaining_charges);
			buffs[buff_entry->buff_slot]->SetRemainingChargesPoison(buff_entry->poison_remaining_charges);
			buffs[buff_entry->buff_slot]->SetRemainingChargesDisease(buff_entry->disease_remaining_charges);
			buffs[buff_entry->buff_slot]->SetRemainingChargesCurse(buff_entry->curse_remaining_charges);
			buffs[buff_entry->buff_slot]->SetRemainingCharges(buff_entry->general_remaining_charges);
			buffs[buff_entry->buff_slot]->SetMeleeShield(buff_entry->melee_shield_remaining);
			buffs[buff_entry->buff_slot]->SetMagicShield(buff_entry->magic_shield_remaining);
			buffs[buff_entry->buff_slot]->SetMeleeShieldReduction(buff_entry->melee_shield_reduction);
			buffs[buff_entry->buff_slot]->SetMagicShieldReduction(buff_entry->magic_shield_reduction);
			buffs[buff_entry->buff_slot]->SetAttacksNegated(buff_entry->attacks_negated);
			buffs[buff_entry->buff_slot]->SetDeathSaveChance(buff_entry->death_save_chance);
			buffs[buff_entry->buff_slot]->SetCasterAARank(buff_entry->caster_aa_rank);
			buffs[buff_entry->buff_slot]->SetInstrumentMod(buff_entry->instrument_mod);

			safe_delete(new_spell);
			data_ptr += sizeof(SPDat_Spell_Struct);
		}
	}
}

Buff::Buff(Spell *spell, uint32 duration)
{
	spell_duration_remaining = duration;
	is_perm_illusion = false;
	corruption_remaining_charges = 0;
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
