/*  EQEMu:  Everquest Server Emulator
Copyright (C) 2001-2004  EQEMu Development Team (http://eqemu.org)

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
#include "../common/packet_dump.h"
#include "../common/moremath.h"
#include "../common/Item.h"
#include "worldserver.h"
#include "../common/skills.h"
#include "../common/bodytypes.h"
#include "../common/classes.h"
#include "../common/rulesys.h"
#include <math.h>
#include <assert.h>
#ifndef WIN32
#include <stdlib.h>
#include "../common/unix.h"
#endif

#include "StringIDs.h"

void Mob::CalcBonuses()
{	
	CalcSpellBonuses(&spellbonuses);

	CalcMaxHP();
	CalcMaxMana();
	SetAttackTimer();
	
	rooted = FindType(SE_Root);
}

void NPC::CalcBonuses()
{
	Mob::CalcBonuses();

	if(RuleB(NPC, UseItemBonusesForNonPets)){
		memset(&itembonuses, 0, sizeof(StatBonuses));
		CalcItemBonuses(&itembonuses);
	}
	else{
		if(GetOwner()){
			memset(&itembonuses, 0, sizeof(StatBonuses));
			CalcItemBonuses(&itembonuses);
		}
	}
}

void Client::CalcBonuses()
{
	_ZP(Client_CalcBonuses);
	memset(&itembonuses, 0, sizeof(StatBonuses));
	CalcItemBonuses(&itembonuses);
	CalcEdibleBonuses(&itembonuses);
	
	RecalcWeight();
	
	CalcSpellBonuses(&spellbonuses);
	
	CalcAC();
	CalcATK();
	CalcHaste();
	
	CalcSTR();
	CalcSTA();
	CalcDEX();
	CalcAGI();
	CalcINT();
	CalcWIS();
	CalcCHA();
	
	CalcMR();
	CalcFR();
	CalcDR();
	CalcPR();
	CalcCR();
	
	CalcMaxHP();
	CalcMaxMana();
	CalcMaxEndurance();
	
	rooted = FindType(SE_Root);
}

int Client::CalcRecommendedLevelBonus(int8 level, uint8 reclevel, int basestat)
{
	if( (reclevel > 0) && (level < reclevel) )
	{
		sint32 statmod = (level * 10000 / reclevel) * basestat;
	
		if( statmod < 0 )
		{
			statmod -= 5000;
			return (statmod/10000);
		}
		else
		{
			statmod += 5000;
			return (statmod/10000);
		}
	}

	return 0;
}

void Client::CalcItemBonuses(StatBonuses* newbon) {
	//memset assumed to be done by caller.
	
	int i;
	//should not include 21 (SLOT_AMMO)
	for (i=0; i<21; i++) {
		const ItemInst* inst = m_inv[i];
		if(inst == 0)
			continue;
		AddItemBonuses(inst, newbon);
	}
	
	//tribute items
	for (i = 400; i < 404; i++) {
		const ItemInst* inst = m_inv[i];
		if(inst == 0)
			continue;
		AddItemBonuses(inst, newbon);
	}
	
	//caps
	if(newbon->ManaRegen > RuleI(Character, ItemManaRegenCap))
		newbon->ManaRegen = RuleI(Character, ItemManaRegenCap);
	if(newbon->HPRegen > RuleI(Character, ItemHealthRegenCap))
		newbon->HPRegen = RuleI(Character, ItemHealthRegenCap);
	
	
	SetAttackTimer();
}
		
void Client::AddItemBonuses(const ItemInst *inst, StatBonuses* newbon, bool isAug) {
	if(!inst || !inst->IsType(ItemClassCommon))
	{
		return;
	}
	if (GetLevel() < inst->GetItem()->ReqLevel)
	{
		return;
	}
	if (!inst->IsEquipable(GetBaseRace(),GetClass()))
	{
		return;
	}
	if(inst->GetAugmentType()==0 && isAug == true)
	{
		return;
	}

	const Item_Struct *item = inst->GetItem();
	if( GetLevel() >= item->RecLevel )
	{
		newbon->AC += item->AC;
		newbon->HP += item->HP;
		newbon->Mana += item->Mana;
		newbon->Endurance += item->Endur;
		newbon->STR += item->AStr;
		newbon->STA += item->ASta;
		newbon->DEX += item->ADex;
		newbon->AGI += item->AAgi;
		newbon->INT += item->AInt;
		newbon->WIS += item->AWis;
		newbon->CHA += item->ACha;
		
		newbon->MR += item->MR;
		newbon->FR += item->FR;
		newbon->CR += item->CR;
		newbon->PR += item->PR;
		newbon->DR += item->DR;
	}
	else
	{
		int lvl = GetLevel();
		int reclvl = item->RecLevel;

		newbon->AC += CalcRecommendedLevelBonus( lvl, reclvl, item->AC );
		newbon->HP += CalcRecommendedLevelBonus( lvl, reclvl, item->HP );
		newbon->Mana += CalcRecommendedLevelBonus( lvl, reclvl, item->Mana );
		newbon->Endurance += CalcRecommendedLevelBonus( lvl, reclvl, item->Endur );
		newbon->STR += CalcRecommendedLevelBonus( lvl, reclvl, item->AStr );
		newbon->STA += CalcRecommendedLevelBonus( lvl, reclvl, item->ASta );
		newbon->DEX += CalcRecommendedLevelBonus( lvl, reclvl, item->ADex );
		newbon->AGI += CalcRecommendedLevelBonus( lvl, reclvl, item->AAgi );
		newbon->INT += CalcRecommendedLevelBonus( lvl, reclvl, item->AInt );
		newbon->WIS += CalcRecommendedLevelBonus( lvl, reclvl, item->AWis );
		newbon->CHA += CalcRecommendedLevelBonus( lvl, reclvl, item->ACha );

		newbon->MR += CalcRecommendedLevelBonus( lvl, reclvl, item->MR );
		newbon->FR += CalcRecommendedLevelBonus( lvl, reclvl, item->FR );
		newbon->CR += CalcRecommendedLevelBonus( lvl, reclvl, item->CR );
		newbon->PR += CalcRecommendedLevelBonus( lvl, reclvl, item->PR );
		newbon->DR += CalcRecommendedLevelBonus( lvl, reclvl, item->DR );
	}
	
	//FatherNitwit: New style haste, shields, and regens
	if(newbon->haste < (sint8)item->Haste) {
		newbon->haste = item->Haste;
	}
	if(item->Regen > 0) {
		newbon->HPRegen += item->Regen;
	}
	if(item->ManaRegen > 0) {
		newbon->ManaRegen += item->ManaRegen;
	}
	if(item->Attack > 0) {
		newbon->ATK += item->Attack;
	}
	if(item->EnduranceRegen > 0){
		newbon->EnduranceRegen += item->EnduranceRegen;
	}
	if(item->DamageShield > 0) {
		if((newbon->DamageShield + item->DamageShield) > RuleI(Character, ItemDamageShieldCap))
			newbon->DamageShield = RuleI(Character, ItemDamageShieldCap);
		else
			newbon->DamageShield += item->DamageShield;
	}
	if(item->SpellShield > 0) {
		if((newbon->SpellDamageShield + item->SpellShield) > RuleI(Character, ItemSpellShieldingCap))
			newbon->SpellDamageShield = RuleI(Character, ItemSpellShieldingCap);
		else
			newbon->SpellDamageShield += item->SpellShield;
	}
	if(item->Shielding > 0) {
		if((newbon->MeleeMitigation + item->Shielding) > RuleI(Character, ItemShieldingCap))
			newbon->MeleeMitigation = RuleI(Character, ItemShieldingCap);
		else
			newbon->MeleeMitigation += item->Shielding;
	}
	if(item->StunResist > 0) {
		if((newbon->StunResist + item->StunResist) > RuleI(Character, ItemStunResistCap))
			newbon->StunResist = RuleI(Character, ItemStunResistCap);
		else
			newbon->StunResist += item->StunResist;
	}
	if(item->StrikeThrough > 0) {
		if((newbon->StrikeThrough + item->StrikeThrough) > RuleI(Character, ItemStrikethroughCap))
			newbon->StrikeThrough = RuleI(Character, ItemStrikethroughCap);
		else
			newbon->StrikeThrough += item->StrikeThrough;
	}
	if(item->Avoidance > 0) {
		if((newbon->AvoidMeleeChance + item->Avoidance) > RuleI(Character, ItemAvoidanceCap))
			newbon->AvoidMeleeChance = RuleI(Character, ItemAvoidanceCap);
		else
			newbon->AvoidMeleeChance += item->Avoidance;
	}
	if(item->Accuracy > 0) {
		if((newbon->HitChance + item->Accuracy) > RuleI(Character, ItemAccuracyCap))
			newbon->HitChance = RuleI(Character, ItemAccuracyCap);
		else
			newbon->HitChance += item->Accuracy;
	}
	if(item->CombatEffects > 0) {
		if((newbon->ProcChance + item->CombatEffects) > RuleI(Character, ItemCombatEffectsCap))
			newbon->ProcChance = RuleI(Character, ItemCombatEffectsCap);
		else
			newbon->ProcChance += item->CombatEffects;
	}
	if(item->DotShielding > 0) {
		if((newbon->DoTShielding + item->DotShielding) > RuleI(Character, ItemDoTShieldingCap))
			newbon->DoTShielding = RuleI(Character, ItemDoTShieldingCap);
		else
			newbon->DoTShielding += item->DotShielding;
	}
	else if (item->Worn.Effect>0 && (item->Worn.Type == ET_WornEffect)) { // latent effects
		ApplySpellsBonuses(item->Worn.Effect, item->Worn.Level, newbon);
	}
	switch(item->BardType)
	{
	case 51: /* All (e.g. Singing Short Sword) */
		{
			if(item->BardValue > newbon->singingMod)
				newbon->singingMod = item->BardValue;
			if(item->BardValue > newbon->brassMod)
				newbon->brassMod = item->BardValue;
			if(item->BardValue > newbon->stringedMod)
				newbon->stringedMod = item->BardValue;
			if(item->BardValue > newbon->percussionMod)
				newbon->percussionMod = item->BardValue;
			if(item->BardValue > newbon->windMod)
				newbon->windMod = item->BardValue;
			break;
		}
	case 50: /* Singing */
		{
			if(item->BardValue > newbon->singingMod)
				newbon->singingMod = item->BardValue;
			break;
		}
	case 23: /* Wind */
		{
			if(item->BardValue > newbon->windMod)
				newbon->windMod = item->BardValue;
			break;
		}
	case 24: /* stringed */
		{
			if(item->BardValue > newbon->stringedMod)
				newbon->stringedMod = item->BardValue;
			break;
		}
	case 25: /* brass */
		{
			if(item->BardValue > newbon->brassMod)
				newbon->brassMod = item->BardValue;
			break;
		}
	case 26: /* Percussion */
		{
			if(item->BardValue > newbon->percussionMod)
				newbon->percussionMod = item->BardValue;
			break;
		}
	}
	
	if (item->SkillModValue != 0 && item->SkillModType < HIGHEST_SKILL){
		if (newbon->skillmod[item->SkillModType] < item->SkillModValue)
			newbon->skillmod[item->SkillModType] = (sint8)item->SkillModValue;
	}

	int i;
	for(i = 0; i < MAX_AUGMENT_SLOTS; i++) {
		AddItemBonuses(inst->GetAugment(i),newbon,true);
	}
}

void Client::CalcEdibleBonuses(StatBonuses* newbon) {
#if EQDEBUG >= 11
    cout<<"Client::CalcEdibleBonuses(StatBonuses* newbon)"<<endl;
#endif
  // Search player slots for skill=14(food) and skill=15(drink)
  	uint32 i;
  	
	bool food = false;
	bool drink = false;
	for (i = 22; i <= 29; i++)
	{
		if (food && drink)
			break;
		const ItemInst* inst = GetInv().GetItem(i);
		if (inst && inst->GetItem() && inst->IsType(ItemClassCommon)) {
			const Item_Struct *item=inst->GetItem();
			if (item->ItemType == ItemTypeFood && !food)
				food = true;
			else if (item->ItemType == ItemTypeDrink && !drink)
				drink = true;
			else
				continue;
			AddItemBonuses(inst, newbon);
		}
	}
	for (i = 251; i <= 330; i++)
	{
		if (food && drink)
			break;
		const ItemInst* inst = GetInv().GetItem(i);
		if (inst && inst->GetItem() && inst->IsType(ItemClassCommon)) {
			const Item_Struct *item=inst->GetItem();
			if (item->ItemType == ItemTypeFood && !food)
				food = true;
			else if (item->ItemType == ItemTypeDrink && !drink)
				drink = true;
			else
				continue;
			AddItemBonuses(inst, newbon);
		}
	}
}

void Mob::CalcSpellBonuses(StatBonuses* newbon)
{
	int i;

	memset(newbon, 0, sizeof(StatBonuses));
	newbon->AggroRange = -1;
	newbon->AssistRange = -1;

	for(i = 0; i < BUFF_COUNT; i++) {
		if(buffs[i].spellid != SPELL_UNKNOWN)
			ApplySpellsBonuses(buffs[i].spellid, buffs[i].casterlevel, newbon, buffs[i].casterid);
	}
	
	//this prolly suffer from roundoff error slightly...
	newbon->AC = newbon->AC * 10 / 34;	//ratio determined impirically from client.
}

void Mob::ApplySpellsBonuses(int16 spell_id, int8 casterlevel, StatBonuses* newbon, int16 casterId)
{
	int i, effect_value;
	Mob *caster = NULL;

	if(!IsValidSpell(spell_id))
		return;
	
	if(casterId > 0)
		caster = entity_list.GetClientByID(casterId);
	
	for (i = 0; i < EFFECT_COUNT; i++)
	{
		if(IsBlankSpellEffect(spell_id, i))
			continue;

		effect_value = CalcSpellEffectValue(spell_id, i, casterlevel, caster);

		switch (spells[spell_id].effectid[i])
		{

			case SE_Harmony:
			{
				// neotokyo: Harmony effect as buff - kinda tricky
				// harmony could stack with a lull spell, which has better aggro range
				// take the one with less range in any case
				if
				(
					newbon->AggroRange == -1 ||
					effect_value < newbon->AggroRange
				)
				{
					newbon->AggroRange = effect_value;
				}
				break;
			}

			case SE_ChangeFrenzyRad:
			{
				if
				(
					newbon->AssistRange == -1 ||
					effect_value < newbon->AssistRange
				)
				{
					newbon->AssistRange = effect_value;
				}
				break;
			}

			case SE_AttackSpeed:
			{
				if ((effect_value - 100) > 0) { // Haste
					if (newbon->haste < 0) break; // Slowed - Don't apply haste
					if ((effect_value - 100) > newbon->haste) {
						newbon->haste = effect_value - 100;
					}
				} else if ((effect_value - 100) < 0) { // Slow
					if ((effect_value - 100) < newbon->haste) {
						newbon->haste = effect_value - 100;
					}
				}
				break;
			}

 			case SE_AttackSpeed2:
			{
				if ((effect_value - 100) > 0) { // Haste V2 - Stacks with V1 but does not Overcap
					if ((effect_value - 100) > newbon->hastetype2) {
						newbon->hastetype2 = effect_value - 100;
					}
				}
				break;
 			}
 
 			case SE_AttackSpeed3:
 			{
				if (effect_value > 0) { // Haste V3 - Stacks and Overcaps
					if (effect_value > newbon->hastetype3) {
						newbon->hastetype3 = effect_value;
					}
				}
				break;
 			}

			case SE_TotalHP:
			{
				newbon->HP += effect_value;
				break;
			}

			case SE_ManaPool:
			{
				newbon->Mana += effect_value;
				break;
			}

			case SE_Stamina:
			{
				newbon->EnduranceReduction += effect_value;
				break;
			}
			
			case SE_ArmorClass:
			{
				newbon->AC += effect_value;
				break;
			}

			case SE_ATK:
			{
				newbon->ATK += effect_value;
				break;
			}

			case SE_STR:
			{
				newbon->STR += effect_value;
				break;
			}

			case SE_DEX:
			{
				newbon->DEX += effect_value;
				break;
			}

			case SE_AGI:
			{
				newbon->AGI += effect_value;
				break;
			}

			case SE_STA:
			{
				newbon->STA += effect_value;
				break;
			}

			case SE_INT:
			{
				newbon->INT += effect_value;
				break;
			}

			case SE_WIS:
			{
				newbon->WIS += effect_value;
				break;
			}

			case SE_CHA:
			{
				if (spells[spell_id].base[i] != 0) {
					newbon->CHA += effect_value;
				}
				break;
			}

			case SE_AllStats:
			{
				newbon->STR += effect_value;
				newbon->DEX += effect_value;
				newbon->AGI += effect_value;
				newbon->STA += effect_value;
				newbon->INT += effect_value;
				newbon->WIS += effect_value;
				newbon->CHA += effect_value;
				break;
			}

			case SE_ResistFire:
			{
				newbon->FR += effect_value;
				break;
			}

			case SE_ResistCold:
			{
				newbon->CR += effect_value;
				break;
			}

			case SE_ResistPoison:
			{
				newbon->PR += effect_value;
				break;
			}

			case SE_ResistDisease:
			{
				newbon->DR += effect_value;
				break;
			}

			case SE_ResistMagic:
			{
				newbon->MR += effect_value;
				break;
			}

			case SE_ResistAll:
			{
				newbon->MR += effect_value;
				newbon->DR += effect_value;
				newbon->PR += effect_value;
				newbon->CR += effect_value;
				newbon->FR += effect_value;
				break;
			}

			case SE_CastingLevel:	// Brilliance of Ro
			{
				newbon->effective_casting_level += effect_value;
				break;
			}

			case SE_MovementSpeed:
			{
				newbon->movementspeed += effect_value;
				break;
			}

			case SE_DamageShield:
			{
				newbon->DamageShield += effect_value;
				newbon->DamageShieldSpellID = spell_id;
				break;
			}
			
			case SE_SpellDamageShield:
			{
				newbon->SpellDamageShield += effect_value;
				break;
			}

			case SE_ReverseDS:
			{
				newbon->ReverseDamageShield += effect_value;
				break;
			}

			case SE_Reflect:
			{
				newbon->reflect_chance += effect_value;
				break;
			}

			case SE_SingingSkill:
			{
				//newbon->skillmod[SINGING] += effect_value;
				if(effect_value > newbon->singingMod)
					newbon->singingMod = effect_value;
				break;
			}
			
			case SE_ChangeAggro:
			{
				newbon->hatemod += effect_value;
				break;
			}
			case SE_MeleeMitigation:
			{
				//for some reason... this value is negative for increased mitigation
				newbon->MeleeMitigation -= effect_value;
				break;
			}
			
			/*
				Assuming that none of these chances stack... they just pick the highest
				
				perhaps some smarter logic is needed here to handle the case
				where there is a chance increase and a decrease
				because right now, the increase will completely offset the decrease...
				
			*/
			case SE_CriticalHitChance:
			{
				if(newbon->CriticalHitChance < effect_value)
					newbon->CriticalHitChance = effect_value;
				break;
			}
				
			case SE_CrippBlowChance:
			{
				if(newbon->CrippBlowChance < effect_value)
					newbon->CrippBlowChance = effect_value;
				break;
			}
				
			case SE_AvoidMeleeChance:
			{
				//multiplier is to be compatible with item effects
				//watching for overflow too
				effect_value = effect_value<3000? effect_value * 10 : 30000;
				if(newbon->AvoidMeleeChance < effect_value)
					newbon->AvoidMeleeChance = effect_value;
				break;
			}
				
			case SE_RiposteChance:
			{
				if(newbon->RiposteChance < effect_value)
					newbon->RiposteChance = effect_value;
				break;
			}
				
			case SE_DodgeChance:
			{
				if(newbon->DodgeChance < effect_value)
					newbon->DodgeChance = effect_value;
				break;
			}
				
			case SE_ParryChance:
			{
				if(newbon->ParryChance < effect_value)
					newbon->ParryChance = effect_value;
				break;
			}
				
			case SE_DualWeildChance:
			{
				if(newbon->DualWeildChance < effect_value)
					newbon->DualWeildChance = effect_value;
				break;
			}
				
			case SE_DoubleAttackChance:
			{
				if(newbon->DoubleAttackChance < effect_value)
					newbon->DoubleAttackChance = effect_value;
				break;
			}
				
			case SE_MeleeLifetap:
			{
				newbon->MeleeLifetap = true;
				break;
			}
				
			case SE_AllInstrunmentMod:
			{
				if(effect_value > newbon->singingMod)
					newbon->singingMod = effect_value;
				if(effect_value > newbon->brassMod)
					newbon->brassMod = effect_value;
				if(effect_value > newbon->percussionMod)
					newbon->percussionMod = effect_value;
				if(effect_value > newbon->windMod)
					newbon->windMod = effect_value;
				if(effect_value > newbon->stringedMod)
					newbon->stringedMod = effect_value;
				break;
			}
				
			case SE_ResistSpellChance:
			{
				if(newbon->ResistSpellChance < effect_value)
					newbon->ResistSpellChance = effect_value;
				break;
			}
				
			case SE_ResistFearChance:
			{
				if(newbon->ResistFearChance < effect_value)
					newbon->ResistFearChance = effect_value;
				break;
			}
				
 			case SE_HundredHands:
 			{
				if(IsBeneficialSpell(spell_id)){ //If it's a beneficial spell we switch it cause
					effect_value *= -1; //of the way it's stored by sony, negative for both ben and det spells
				}
				effect_value = effect_value > 120 ? 120 : (effect_value < -120 ? -120 : effect_value);
				newbon->HundredHands = newbon->HundredHands > effect_value ? newbon->HundredHands : effect_value;
 				break;
 			}
				
			case SE_MeleeSkillCheck:
			{
				if(newbon->MeleeSkillCheck < effect_value) {
					newbon->MeleeSkillCheck = effect_value;
					newbon->MeleeSkillCheckSkill = spells[spell_id].base2[i]==-1?255:spells[spell_id].base2[i];
				}
				break;
			}
				
			case SE_HitChance:
			{
				//multiplier is to be compatible with item effects
				//watching for overflow too
				effect_value = effect_value<2000? effect_value * 15 : 30000;
				if(newbon->HitChance < effect_value) {
					newbon->HitChance = effect_value;
					newbon->HitChanceSkill = spells[spell_id].base2[i]==-1?255:spells[spell_id].base2[i];
				}
				break;
			}
				
			case SE_DamageModifier:
			{
				if(newbon->DamageModifier < effect_value) {
					newbon->DamageModifier = effect_value;
					newbon->DamageModifierSkill = spells[spell_id].base2[i]==-1?255:spells[spell_id].base2[i];
				}
				break;
			}
				
			case SE_MinDamageModifier:
			{
				if(newbon->MinDamageModifier < effect_value)
					newbon->MinDamageModifier = effect_value;
				break;
			}
				
			case SE_StunResist:
			{
				if(newbon->StunResist < effect_value)
					newbon->StunResist = effect_value;
				break;
			}
				
			case SE_ProcChance:
			{
				//multiplier is to be compatible with item effects
				//watching for overflow too
				effect_value = effect_value<3000? effect_value * 10 : 30000;
				if(newbon->ProcChance < effect_value)
					newbon->ProcChance = effect_value;
				break;
			}
				
			case SE_ExtraAttackChance:
			{
				if(newbon->ExtraAttackChance < effect_value)
					newbon->ExtraAttackChance = effect_value;
				break;
			}
				
		}
	}
}

void NPC::CalcItemBonuses(StatBonuses *newbon)
{
	if(newbon){

		for(int i = 0; i < 8; i++){
			const Item_Struct *cur = database.GetItem(equipment[i]);
			if(cur){
				//basic stats
				newbon->AC += cur->AC;
				newbon->HP += cur->HP;
				newbon->Mana += cur->Mana;
				newbon->Endurance += cur->Endur;
				newbon->STR += cur->AStr;
				newbon->STA += cur->ASta;
				newbon->DEX += cur->ADex;
				newbon->AGI += cur->AAgi;
				newbon->INT += cur->AInt;
				newbon->WIS += cur->AWis;
				newbon->CHA += cur->ACha;
				newbon->MR += cur->MR;
				newbon->FR += cur->FR;
				newbon->CR += cur->CR;
				newbon->PR += cur->PR;
				newbon->DR += cur->DR;
				

				//more complex stats
				if(cur->Regen > 0) {
					newbon->HPRegen += cur->Regen;
				}
				if(cur->ManaRegen > 0) {
					newbon->ManaRegen += cur->ManaRegen;
				}
				if(cur->Attack > 0) {
					newbon->ATK += cur->Attack;
				}
				if(cur->DamageShield > 0) {
					newbon->DamageShield += cur->DamageShield;
				}
				if(cur->SpellShield > 0) {
					newbon->SpellDamageShield += cur->SpellShield;
				}
				if(cur->Shielding > 0) {
					newbon->MeleeMitigation += cur->Shielding;
				}
				if(cur->StunResist > 0) {
					newbon->StunResist += cur->StunResist;
				}
				if(cur->StrikeThrough > 0) {
					newbon->StrikeThrough += cur->StrikeThrough;
				}
				if(cur->Avoidance > 0) {
					newbon->AvoidMeleeChance += cur->Avoidance;
				}
				if(cur->Accuracy > 0) {
					newbon->HitChance += cur->Accuracy;
				}
				if(cur->CombatEffects > 0) {
					newbon->ProcChance += cur->CombatEffects;
				}
				if (cur->Worn.Effect>0 && (cur->Worn.Type == ET_WornEffect)) { // latent effects
					ApplySpellsBonuses(cur->Worn.Effect, cur->Worn.Level, newbon);
				}
			}
		}
	
	}
}