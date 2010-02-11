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
#include "spells.h"
#include "buff.h"
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


extern Zone* zone;
extern volatile bool ZoneLoaded;
#if !defined(NEW_LoadSPDat) && !defined(DB_LoadSPDat)
	extern SPDat_Spell_Struct spells[SPDAT_RECORDS];
#endif
extern bool spells_loaded;
extern WorldServer worldserver;

typedef bool (Mob::*SpellEffectProc)(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);
SpellEffectProc SpellEffectDispatch[SE_LAST_EFFECT + 1];

void MapSpellEffects()
{
	memset(SpellEffectDispatch, 0, sizeof(SpellEffectDispatch));
	SpellEffectDispatch[SE_CurrentHP] = &Mob::Handle_SE_CurrentHP;
	SpellEffectDispatch[SE_ArmorClass] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_ATK] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_MovementSpeed] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_STR] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_DEX] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_AGI] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_STA] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_INT] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_WIS] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_CHA] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_AttackSpeed] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_Invisibility] = &Mob::Handle_SE_Invisibility;
	SpellEffectDispatch[SE_SeeInvis] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_WaterBreathing] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_CurrentMana] = &Mob::Handle_SE_CurrentMana;
	SpellEffectDispatch[SE_Lull] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_AddFaction] = &Mob::Handle_SE_AddFaction;
	SpellEffectDispatch[SE_Blind] = &Mob::Handle_SE_Blind;
	SpellEffectDispatch[SE_Stun] = &Mob::Handle_SE_Stun;
	SpellEffectDispatch[SE_Charm] = &Mob::Handle_SE_Charm;
	SpellEffectDispatch[SE_Fear] = &Mob::Handle_SE_Fear;
	SpellEffectDispatch[SE_Stamina] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_BindAffinity] = &Mob::Handle_SE_BindAffinity;
	SpellEffectDispatch[SE_Gate] = &Mob::Handle_SE_Gate;
	SpellEffectDispatch[SE_CancelMagic] = &Mob::Handle_SE_CancelMagic;
	SpellEffectDispatch[SE_InvisVsUndead] = &Mob::Handle_SE_InvisVsUndead;
	SpellEffectDispatch[SE_InvisVsAnimals] = &Mob::Handle_SE_InvisVsAnimals;
	SpellEffectDispatch[SE_ChangeFrenzyRad] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_Mez] = &Mob::Handle_SE_Mez;
	SpellEffectDispatch[SE_SummonItem] = &Mob::Handle_SE_SummonItem;
	SpellEffectDispatch[SE_SummonPet] = &Mob::Handle_SE_SummonPet;
	SpellEffectDispatch[SE_DiseaseCounter] = &Mob::Handle_SE_DiseaseCounter;
	SpellEffectDispatch[SE_PoisonCounter] = &Mob::Handle_SE_PoisonCounter;
	SpellEffectDispatch[SE_DivineAura] = &Mob::Handle_SE_DivineAura;
	SpellEffectDispatch[SE_Destroy] = &Mob::Handle_SE_Destroy;
	SpellEffectDispatch[SE_ShadowStep] = &Mob::Handle_SE_ShadowStep;
	SpellEffectDispatch[SE_Lycanthropy] = NULL;
	SpellEffectDispatch[SE_ResistFire] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_ResistCold] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_ResistPoison] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_ResistDisease] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_ResistMagic] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_SenseDead] = &Mob::Handle_SE_Sense;
	SpellEffectDispatch[SE_SenseSummoned] = &Mob::Handle_SE_Sense;
	SpellEffectDispatch[SE_SenseAnimals] = &Mob::Handle_SE_Sense;
	SpellEffectDispatch[SE_Rune] = &Mob::Handle_SE_Rune;
	SpellEffectDispatch[SE_TrueNorth] = &Mob::Handle_SE_BlankWithPacket;
	SpellEffectDispatch[SE_Levitate] = &Mob::Handle_SE_Levitate;
	SpellEffectDispatch[SE_Illusion] = &Mob::Handle_SE_Illusion;
	SpellEffectDispatch[SE_DamageShield] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_Identify] = &Mob::Handle_SE_BlankWithPacket;
	SpellEffectDispatch[SE_WipeHateList] = &Mob::Handle_SE_WipeHateList;
	SpellEffectDispatch[SE_SpinTarget] = &Mob::Handle_SE_Stun;
	SpellEffectDispatch[SE_InfraVision] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_UltraVision] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_EyeOfZomm] = &Mob::Handle_SE_EyeOfZomm;
	SpellEffectDispatch[SE_ReclaimPet] = &Mob::Handle_SE_ReclaimPet;
	SpellEffectDispatch[SE_TotalHP] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_NecPet] = &Mob::Handle_SE_SummonPet;
	SpellEffectDispatch[SE_BindSight] = &Mob::Handle_SE_BindSight;
	SpellEffectDispatch[SE_FeignDeath] = &Mob::Handle_SE_FeignDeath;
	SpellEffectDispatch[SE_VoiceGraft] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_Sentinel] = &Mob::Handle_SE_Sentinel;
	SpellEffectDispatch[SE_LocateCorpse] = &Mob::Handle_SE_BlankWithPacket;
	SpellEffectDispatch[SE_AbsorbMagicAtt] = &Mob::Handle_SE_AbsorbMagicAtt;
	SpellEffectDispatch[SE_CurrentHPOnce] = &Mob::Handle_SE_CurrentHPOnce;
	SpellEffectDispatch[SE_Revive] = &Mob::Handle_SE_Revive;
	SpellEffectDispatch[SE_SummonPC] = &Mob::Handle_SE_SummonPC;
	SpellEffectDispatch[SE_Teleport] = &Mob::Handle_SE_Teleport;
	SpellEffectDispatch[SE_TossUp] = &Mob::Handle_SE_TossUp;
	SpellEffectDispatch[SE_WeaponProc] = &Mob::Handle_SE_WeaponProc;
	SpellEffectDispatch[SE_Harmony] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_MagnifyVision] = &Mob::Handle_SE_BlankWithPacket;
	SpellEffectDispatch[SE_Succor] = &Mob::Handle_SE_Succor;
	SpellEffectDispatch[SE_ModelSize] = &Mob::Handle_SE_ModelSize;
	SpellEffectDispatch[SE_Cloak] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_SummonCorpse] = &Mob::Handle_SE_SummonCorpse;
	SpellEffectDispatch[SE_Calm] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_StopRain] = &Mob::Handle_SE_StopRain;
	SpellEffectDispatch[SE_NegateIfCombat] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_Sacrifice] = &Mob::Handle_SE_Sacrifice;
	SpellEffectDispatch[SE_Silence] = &Mob::Handle_SE_Silence;
	SpellEffectDispatch[SE_ManaPool] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_AttackSpeed2] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_Root] = &Mob::Handle_SE_Root;
	SpellEffectDispatch[SE_HealOverTime] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_CompleteHeal] = &Mob::Handle_SE_CompleteHeal;
	SpellEffectDispatch[SE_Fearless] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_CallPet] = &Mob::Handle_SE_CallPet;
	SpellEffectDispatch[SE_Translocate] = &Mob::Handle_SE_Translocate;
	SpellEffectDispatch[SE_AntiGate] = NULL;
	SpellEffectDispatch[SE_SummonBSTPet] = &Mob::Handle_SE_SummonPet;
	SpellEffectDispatch[SE_Familiar] = &Mob::Handle_SE_SummonPet;
	SpellEffectDispatch[SE_SummonItemIntoBag] = &Mob::Handle_SE_SummonItemIntoBag;
	SpellEffectDispatch[SE_ResistAll] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_CastingLevel] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_SummonHorse] = &Mob::Handle_SE_SummonHorse;
	SpellEffectDispatch[SE_ChangeAggro] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_Hunger] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_CurseCounter] = &Mob::Handle_SE_CurseCounter;
	SpellEffectDispatch[SE_MagicWeapon] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_SingingSkill] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_AttackSpeed3] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_HealRate] = NULL;
	SpellEffectDispatch[SE_ReverseDS] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_Screech] = NULL;
	SpellEffectDispatch[SE_ImprovedDamage] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_ImprovedHeal] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_SpellResistReduction] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_IncreaseSpellHaste] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_IncreaseSpellDuration] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_IncreaseRange] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_SpellHateMod] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_ReduceReagentCost] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_ReduceManaCost] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_LimitMaxLevel] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_LimitResist] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_LimitTarget] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_LimitEffect] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_LimitSpellType] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_LimitSpell] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_LimitMinDur] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_LimitInstant] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_LimitMinLevel] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_LimitCastTime] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_Teleport2] = &Mob::Handle_SE_Teleport;
	SpellEffectDispatch[SE_PercentalHeal] = &Mob::Handle_SE_PercentalHeal;
	SpellEffectDispatch[SE_StackingCommand_Block] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_StackingCommand_Overwrite] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_DeathSave] = &Mob::Handle_SE_DeathSave;
	SpellEffectDispatch[SE_SuspendPet] = &Mob::Handle_SE_SuspendPet;
	SpellEffectDispatch[SE_TemporaryPets] = &Mob::Handle_SE_TemporaryPets;
	SpellEffectDispatch[SE_BalanceHP] = &Mob::Handle_SE_BalanceHP;
	SpellEffectDispatch[SE_DispelDetrimental] = &Mob::Handle_SE_DispelDetrimental;
	SpellEffectDispatch[SE_IllusionCopy] = &Mob::Handle_SE_IllusionCopy;
	SpellEffectDispatch[SE_SpellDamageShield] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_Reflect] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_AllStats] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_MakeDrunk] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_MitigateSpellDamage] = &Mob::Handle_SE_MitigateSpellDamage;
	SpellEffectDispatch[SE_MitigateMeleeDamage] = &Mob::Handle_SE_MitigateMeleeDamage;
	SpellEffectDispatch[SE_NegateAttacks] = &Mob::Handle_SE_NegateAttacks;
	SpellEffectDispatch[SE_AppraiseLDonChest] = &Mob::Handle_SE_AppraiseLDonChest;
	SpellEffectDispatch[SE_DisarmLDoNTrap] = &Mob::Handle_SE_DisarmLDoNTrap;
	SpellEffectDispatch[SE_UnlockLDoNChest] = &Mob::Handle_SE_UnlockLDoNChest;
	SpellEffectDispatch[SE_PetPowerIncrease] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_MeleeMitigation] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_CriticalHitChance] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_SpellCritChance] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_CrippBlowChance] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_AvoidMeleeChance] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_RiposteChance] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_DodgeChance] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_ParryChance] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_DualWeildChance] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_DoubleAttackChance] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_MeleeLifetap] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_AllInstrunmentMod] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_ResistSpellChance] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_ResistFearChance] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_HundredHands] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_MeleeSkillCheck] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_HitChance] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_DamageModifier] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_MinDamageModifier] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_IncreaseBlockChance] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_CurrentEndurance] = &Mob::Handle_SE_CurrentEndurance;
	SpellEffectDispatch[SE_EndurancePool] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_Amnesia] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_Hate2] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_SkillAttack] = &Mob::Handle_SE_SkillAttack;
	SpellEffectDispatch[SE_FadingMemories] = &Mob::Handle_SE_FadingMemories;
	SpellEffectDispatch[SE_StunResist] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_Strikethrough] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_SkillDamageTaken] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_CurrentEnduranceOnce] = &Mob::Handle_SE_CurrentEndurance;
	SpellEffectDispatch[SE_Taunt] = &Mob::Handle_SE_Taunt;
	SpellEffectDispatch[SE_ProcChance] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_RangedProc] = &Mob::Handle_SE_RangedProc;
	SpellEffectDispatch[SE_IllusionOther] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_MassGroupBuff] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_GroupFearImmunity] = &Mob::Handle_SE_GroupFearImmunity;
	SpellEffectDispatch[SE_Rampage] = &Mob::Handle_SE_Rampage;
	SpellEffectDispatch[SE_AETaunt] = &Mob::Handle_SE_AETaunt;
	SpellEffectDispatch[SE_FleshToBone] = &Mob::Handle_SE_FleshToBone;
	SpellEffectDispatch[SE_FadingMemories2] = &Mob::Handle_SE_FadingMemories;
	SpellEffectDispatch[SE_PetShield] = NULL; //pretty sure these three are AA effects
	SpellEffectDispatch[SE_AEMelee] = NULL;
	SpellEffectDispatch[SE_ProlongedDestruction] = NULL;
	SpellEffectDispatch[SE_MaxHPChange] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_Accuracy] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_PetCriticalHit] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_SlayUndead] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_SkillDamageAmount] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_Packrat] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_GiveDoubleRiposte] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_GiveDoubleAttack] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_TwoHandBash] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_ReduceSkillTimer] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_PersistantCasting] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_DivineSave] = &Mob::Handle_SE_Blank; //Handle this with bonuses
	SpellEffectDispatch[SE_ChannelingChance] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_GivePetGroupTarget] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_SetBreathLevel] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_SecondaryForte] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_SecondaryDmgInc] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_Blank] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_PetDiscipline] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_TripleBackstab] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_CombatStability] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_AddInstrumentMod] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_RaiseStatCap] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_TradeSkillMastery] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_HastenedAASkill] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_MasteryofPast] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_ExtraAttackChance] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_PetDiscipline2] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_ReduceTradeskillFail] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_BaseMovementSpeed] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_CastingLevel2] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_CriticalDoTChance] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_CriticalHealChance] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_Ambidexterity] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_FinishingBlow] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_Flurry] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_PetFlurry] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_SpellDamage] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_FocusCombatDurationMod] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_ImprovedSpellEffect] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_ChangeHeight] = &Mob::Handle_SE_ModelSize;
	SpellEffectDispatch[SE_SuspendMinion] = &Mob::Handle_SE_SuspendPet;
	SpellEffectDispatch[SE_Invisibility2] = &Mob::Handle_SE_Invisibility;
	SpellEffectDispatch[SE_InvisVsUndead2] = &Mob::Handle_SE_InvisVsUndead;
	SpellEffectDispatch[SE_GateToHomeCity] = &Mob::Handle_SE_GateToHomeCity;
	SpellEffectDispatch[SE_DefensiveProc] = &Mob::Handle_SE_DefensiveProc;
	SpellEffectDispatch[SE_BardAEDot] = &Mob::Handle_SE_BardAEDot;
	SpellEffectDispatch[SE_PercentXPIncrease] = &Mob::Handle_SE_Blank;
	SpellEffectDispatch[SE_SummonAndResAllCorpses] = &Mob::Handle_SE_SummonAndResAllCorpses;
}

// the spell can still fail here, if the buff can't stack
// in this case false will be returned, true otherwise
bool Mob::SpellEffect(Mob* caster, Spell *spell_to_cast, int action_sequence, float partial)
{
	_ZP(Mob_SpellEffect);
	ItemInst *summoned_item = NULL;
	Buff *new_buff = NULL;
	uint32 caster_level = 0;

	//calculate caster level
	if(caster && caster->IsClient() && GetCastedSpellInvSlot() > 0)
	{
		const ItemInst* inst = caster->CastToClient()->GetInv().GetItem(GetCastedSpellInvSlot());
		if(inst)
		{
			if(inst->GetItem()->Click.Level > 0)
			{
				caster_level = inst->GetItem()->Click.Level;
			}
			else
			{
				caster_level = caster ? caster->GetCasterLevel() : GetCasterLevel();
			}
		}
		else
			caster_level = caster ? caster->GetCasterLevel() : GetCasterLevel();
	}
	else
		caster_level = caster ? caster->GetCasterLevel() : GetCasterLevel();

	spell_to_cast->SetCasterLevel(caster_level);

	//Add the buff and junk
	sint32 buff_duration = CalcBuffDuration(caster, this, spell_to_cast, caster_level) - 1;
	if(caster)
		buff_duration = caster->GetActSpellDuration(spell_to_cast, buff_duration);

	sint32 buff_slot = -1;
	if(buff_duration > 0)
	{
		if(spell_to_cast->IsEffectInSpell(SE_BindSight))
		{
			if(caster)
			{
				new_buff = caster->AddBuff(caster, spell_to_cast, buff_slot, buff_duration);
				if(!new_buff)
				{
					mlog(SPELLS__EFFECT_VALUES, "Unable to apply buff for spell %s(%d) via Mob::AddBuff()", spell_to_cast->GetSpell().name, spell_to_cast->GetSpellID());
					SendActionSpellPacket(spell_to_cast, this, action_sequence, caster_level);
					SendCombatDamageSpellPacket(spell_to_cast, this, action_sequence);
					return false;
				}
			}
			else
			{
				mlog(SPELLS__EFFECT_VALUES, "Unable to apply buff for spell %s(%d) via Mob::AddBuff()", spell_to_cast->GetSpell().name, spell_to_cast->GetSpellID());
				SendActionSpellPacket(spell_to_cast, this, action_sequence, caster_level);
				SendCombatDamageSpellPacket(spell_to_cast, this, action_sequence);
				return false;
			}
		}
		else
		{
			new_buff = AddBuff(caster, spell_to_cast, buff_slot, buff_duration);
			if(!new_buff)
			{
				mlog(SPELLS__EFFECT_VALUES, "Unable to apply buff for spell %s(%d) via Mob::AddBuff()", spell_to_cast->GetSpell().name, spell_to_cast->GetSpellID());
				SendActionSpellPacket(spell_to_cast, this, action_sequence, caster_level);
				SendCombatDamageSpellPacket(spell_to_cast, this, action_sequence);
				return false;
			}
		}
	}

	//loop through all effects...
	bool need_04_packet = false;
	for(int i = 0; i < EFFECT_COUNT; i++)
	{
		if(spell_to_cast->GetSpell().effectid[i] >= SE_LAST_EFFECT)
		{
			mlog(SPELLS__EFFECT_VALUES, "Out of range spell effect: %s(%d) effect id %d at index %d", spell_to_cast->GetSpell().name, 
				spell_to_cast->GetSpellID(), spell_to_cast->GetSpell().effectid[i], i);
			Message(13, "Out of range spell effect: %s(%d) effect id %d at index %d", spell_to_cast->GetSpell().name, 
				spell_to_cast->GetSpellID(), spell_to_cast->GetSpell().effectid[i], i);
			continue;
		}

		SpellEffectProc proc = SpellEffectDispatch[spell_to_cast->GetSpell().effectid[i]];
		if(proc == NULL) 
		{
			mlog(SPELLS__EFFECT_VALUES, "Unhandled spell effect: %s(%d) effect id %d at index %d", spell_to_cast->GetSpell().name, 
				spell_to_cast->GetSpellID(), spell_to_cast->GetSpell().effectid[i], i);
			Message(13, "Unhandled spell effect: %s(%d) effect id %d at index %d", spell_to_cast->GetSpell().name, 
				spell_to_cast->GetSpellID(), spell_to_cast->GetSpell().effectid[i], i);
			continue;
		}

		if(!need_04_packet)
		{
			need_04_packet = (this->*proc)(spell_to_cast, caster, i, partial, &summoned_item, new_buff, buff_slot);
		}
		else
		{
			(this->*proc)(spell_to_cast, caster, i, partial, &summoned_item, new_buff, buff_slot);
		}
	}

	if(new_buff)
	{
		SendActionSpellPacket(spell_to_cast, this, action_sequence, caster_level, 4);
	}
	else
	{
		if(need_04_packet)
		{
			SendActionSpellPacket(spell_to_cast, this, action_sequence, caster_level, 4);
		}
		else
		{
			SendActionSpellPacket(spell_to_cast, this, action_sequence, caster_level, 0);
		}
	}

	SendCombatDamageSpellPacket(spell_to_cast, this, action_sequence);
	if(new_buff)
	{
		if(spell_to_cast->IsEffectInSpell(SE_BindSight))
		{
			action_sequence = caster->SendActionSpellPacket(spell_to_cast, caster, caster_level);
			caster->SendActionSpellPacket(spell_to_cast, this, action_sequence, caster_level, 4);
			caster->SendCombatDamageSpellPacket(spell_to_cast, this, action_sequence);
			caster->SendBuffPacket(new_buff, buff_slot, 4, action_sequence);
		}
		else
		{
			SendBuffPacket(new_buff, buff_slot, 4, action_sequence);
		}
	}

	if (summoned_item) 
	{
		if(IsClient())
		{
			Client *c = CastToClient();
			c->PushItemOnCursor(*summoned_item);
			c->SendItemPacket(SLOT_CURSOR, summoned_item, ItemPacketSummonItem);
		}
		safe_delete(summoned_item);
	}

	return true;
}

bool Mob::Handle_SE_Blank(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	return false;
}

bool Mob::Handle_SE_BlankWithPacket(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	return true;
}

bool Mob::Handle_SE_CurrentHP(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(!buff_in_use)
	{
		sint32 dmg = CalcSpellEffectValue(spell_to_cast, effect_id_index, spell_to_cast->GetCasterLevel(), caster, 0);

		if(dmg < 0)
		{
			dmg = (sint32) (dmg * partial / 100);
			
			if(caster)
				dmg = caster->GetActSpellDamage(spell_to_cast, dmg);

			dmg = -dmg;
			Damage(caster, dmg, spell_to_cast->GetSpellID(), spell_to_cast->GetSpell().skill, false, buff_slot, false);
		}
		else if(dmg > 0)
		{
			if(caster)
				dmg = caster->GetActSpellHealing(spell_to_cast, dmg);

			HealDamage(dmg, caster);
		}
	}

	return false;
}

bool Mob::Handle_SE_Invisibility(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	SetInvisible(true);
	return false;
}

bool Mob::Handle_SE_CurrentMana(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	sint32 effect_value = CalcSpellEffectValue(spell_to_cast, effect_id_index, spell_to_cast->GetCasterLevel(), caster, 0);
	if(spell_to_cast->IsManaTapSpell())
	{
		if(GetCasterClass() != 'N')
		{
			SetMana(GetMana() + effect_value);
			caster->SetMana(caster->GetMana() + abs(effect_value));
		}
	}
	else
	{
		if(!buff_in_use)
		{
			SetMana(GetMana() + effect_value);
		}
	}
	return false;
}

bool Mob::Handle_SE_AddFaction(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(caster && GetPrimaryFaction() > 0) 
	{
		//TODO: Is this right? Are they always base 0? Remember to look into it later.
		caster->AddFactionBonus(GetPrimaryFaction(), spell_to_cast->GetSpell().base[0]);
	}
	return false;
}

bool Mob::Handle_SE_Blind(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(spell_to_cast->GetSpell().base[effect_id_index] == 1)
		BuffFadeByEffect(SE_Blind);
	return false;
}

bool Mob::Handle_SE_Stun(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(SpecAttacks[UNSTUNABLE])
	{
		if(caster)
		{
			caster->Message_StringID(MT_Shout, IMMUNE_STUN);
		}
	}
	else
	{
		sint32 effect_value = CalcSpellEffectValue(spell_to_cast, effect_id_index, spell_to_cast->GetCasterLevel(), caster, 0);
		Stun(effect_value);
	}
	return false;
}

bool Mob::Handle_SE_Charm(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	//This is one of the more involved spell effects and needs both a caster and buff to be successful
	if(!caster)
		return false;

	if(!buff_in_use)
		return false;

	if(IsNPC())
	{
		CastToNPC()->SaveGuardSpotCharm();
	}

	InterruptSpell();
	entity_list.RemoveDebuffs(this);
	entity_list.RemoveFromTargets(this);
	WipeHateList();

	if (IsClient() && caster->IsClient())
	{
		caster->Message(0, "Unable to cast charm on a fellow player.");
		BuffFadeByEffect(SE_Charm);
		return false;
	} 
	else if(IsCorpse()) 
	{
		caster->Message(0, "Unable to cast charm on a corpse.");
		BuffFadeByEffect(SE_Charm);
		return false;
	}
	else if(caster->GetPet() != NULL && caster->IsClient())
	{
		caster->Message(0, "You cannot charm something when you already have a pet.");
		BuffFadeByEffect(SE_Charm);
		return false;
	}
	else if(GetOwner())
	{
		caster->Message(0, "You cannot charm someone else's pet!");
		BuffFadeByEffect(SE_Charm);
		return false;
	}

	Mob *my_pet = GetPet();
	if(my_pet)
	{
		my_pet->Kill();
	}

	caster->SetPet(this);
	SetOwnerID(caster->GetID());
	SetPetOrder(SPO_Follow);

	//Sends the pet update so the pet window will work for charmed pets
	if(caster->IsClient())
	{
		EQApplicationPacket *app = new EQApplicationPacket(OP_Charm, sizeof(Charm_Struct));
		Charm_Struct *ps = (Charm_Struct*)app->pBuffer;
		ps->owner_id = caster->GetID();
		ps->pet_id = this->GetID();
		ps->command = 1;
		entity_list.QueueClients(this, app);
		safe_delete(app);
		SendPetBuffsToClient();
	}

	//If we're a client we start our AI and freeze the client so it can't move
	if (IsClient()) 
	{
		AI_Start();
		SendAppearancePacket(14, 100, true, true);
	} 
	else if(IsNPC()) 
	{
		CastToNPC()->SetPetSpellID(0);	//not a pet spell.
	}

	bool b_break = false;

	// define spells with fixed duration
	// this is handled by the server, and not by the spell database
	switch(spell_to_cast->GetSpellID())
	{
	case 3371:	//call of the banshee
	case 1707:	//dictate
		b_break = true;
	}

	if (!b_break)
	{
		int resist_mod = partial + (GetCHA() / 25);
		resist_mod = resist_mod > 100 ? 100 : resist_mod;

		buff_in_use->SetDurationRemaining(resist_mod * buff_in_use->GetDurationRemaining() / 100);
	}

	if(IsClient())
	{
		if(buff_in_use->GetDurationRemaining() > RuleI(Character, MaxCharmDurationForPlayerCharacter))
			buff_in_use->SetDurationRemaining(RuleI(Character, MaxCharmDurationForPlayerCharacter));
	}
	return false;
}

bool Mob::Handle_SE_Fear(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(!buff_in_use)
		return false;

	buff_in_use->SetDurationRemaining(buff_in_use->GetDurationRemaining() * partial / 100);

	if(IsClient())
	{
		if(buff_in_use->GetDurationRemaining() > RuleI(Character, MaxFearDurationForPlayerCharacter))
			buff_in_use->SetDurationRemaining(RuleI(Character, MaxFearDurationForPlayerCharacter));
	}

	if(RuleB(Combat, EnableFearPathing))
	{
		if(IsClient())
		{
			AI_Start();
		}

		CalculateNewFearpoint();
		if(curfp) 
		{
			return false;
		}
	}
	else 
	{
		Stun(buff_in_use->GetDurationRemaining() * 6000 - (6000 - tic_timer.GetRemainingTime()));
	}
	return false;
}

bool Mob::Handle_SE_BindAffinity(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(IsClient())
	{
		if(CastToClient()->GetGM() || RuleB(Character, BindAnywhere))
		{
			CastToClient()->SetBindPoint();
			Save();
			return true;
		}
		else
		{
			if(!zone->CanBind())
			{
				Message_StringID(13, CANNOT_BIND);
				return false;
			}

			if(!zone->IsCity())
			{
				if(caster != this)
				{
					Message_StringID(13, CANNOT_BIND);
					return false;
				}
				else
				{
					CastToClient()->SetBindPoint();
					Save();
					return true;
				}
			}
			else
			{
				CastToClient()->SetBindPoint();
				Save();
				return true;
			}
		}
	}
	return false;
}

bool Mob::Handle_SE_Gate(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	Gate();
	return false;
}

bool Mob::Handle_SE_CancelMagic(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	int max_slots = GetMaxTotalSlots();
	for(int buffs_i = 0; buffs_i < max_slots; buffs_i++)
	{
		if(!buffs[buffs_i])
		{
			continue;
		}

		if(buffs[buffs_i]->GetRemainingChargesDisease() > 0 ||
			buffs[buffs_i]->GetRemainingChargesPoison() > 0 ||
			buffs[buffs_i]->GetRemainingChargesCurse() > 0)
		{
			continue;
		}

		if(!buffs[buffs_i]->IsPermanentDuration())
		{
			BuffFadeBySlot(buffs_i);
			return false;
		}
	}
	return false;
}

bool Mob::Handle_SE_InvisVsUndead(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	invisible_undead = true;
	return false;
}

bool Mob::Handle_SE_InvisVsAnimals(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	invisible_animals = true;
	return false;
}

bool Mob::Handle_SE_Mez(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	Mesmerize();
	return false;
}

bool Mob::Handle_SE_SummonItem(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	const Item_Struct *item = database.GetItem(spell_to_cast->GetSpell().base[effect_id_index]);
	if(IsClient())
	{
		Client *c = CastToClient();
		if(c->CheckLoreConflict(item))
		{
			Message_StringID(0,PICK_LORE);
			return false;
		}
		else
		{
			int charges;
			if(spell_to_cast->GetSpell().formula[effect_id_index] < 100)
			{
				charges = spell_to_cast->GetSpell().formula[effect_id_index];
			}
			else
			{
				charges = CalcSpellEffectValue_formula(spell_to_cast->GetSpell().formula[effect_id_index], 
					0, 20, spell_to_cast->GetCasterLevel(), spell_to_cast);
			}
			//This is hard to read, TODO: rewrite this to not suck so hard
			charges = (spell_to_cast->GetSpell().formula[effect_id_index] < 100) ? charges :
				(charges > 20) ? 20 : (spell_to_cast->GetSpell().max[effect_id_index] < 1) ? item->MaxCharges :
				spell_to_cast->GetSpell().max[effect_id_index];

			if(*summoned_item)
			{
				c->PushItemOnCursor(*(*summoned_item));
				c->SendItemPacket(SLOT_CURSOR, *summoned_item, ItemPacketSummonItem);
				safe_delete(*summoned_item);
			}
			*summoned_item = database.CreateItem(spell_to_cast->GetSpell().base[effect_id_index], charges);
		}
	}
	return false;
}

bool Mob::Handle_SE_SummonPet(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(GetPet())
	{
		Message_StringID(MT_Shout, ONLY_ONE_PET);
	}
	else
	{
		MakePet(spell_to_cast, spell_to_cast->GetSpell().teleport_zone);
	}
	return false;
}

bool Mob::Handle_SE_DiseaseCounter(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	sint32 effect_value = CalcSpellEffectValue(spell_to_cast, effect_id_index, spell_to_cast->GetCasterLevel(), caster, 0);
	if(effect_value > 0)
	{
		if(buff_in_use)
		{
			buff_in_use->SetRemainingChargesDisease(effect_value);
		}
	}
	else
	{
		effect_value = -effect_value;
		int max_slots = GetMaxTotalSlots();
		for(int buffs_i = 0; buffs_i < max_slots; buffs_i++)
		{
			if(!buffs[buffs_i])
			{
				continue;
			}

			if(buffs[buffs_i]->GetRemainingChargesDisease() == 0)
			{
				continue;
			}

			if(effect_value > buffs[buffs_i]->GetRemainingChargesDisease())
			{
				effect_value -= buffs[buffs_i]->GetRemainingChargesDisease();
				if(caster)
				{
					caster->Message(MT_Spells, "You have cured your target from %s!", buffs[buffs_i]->GetSpell()->GetSpell().name);
					BuffFadeBySlot(buffs_i);
				}
			}
			else
			{
				buffs[buffs_i]->SetRemainingChargesDisease(buffs[buffs_i]->GetRemainingChargesDisease() - effect_value);
				effect_value = 0; 
			}
		}
	}
	return false;						
}

bool Mob::Handle_SE_DivineAura(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	SetInvul(true);
	return false;
}

bool Mob::Handle_SE_Sense(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(IsClient())
	{
		CastToClient()->SetSenseExemption(true);
	}
	return true;
}

bool Mob::Handle_SE_Destroy(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(!IsNPC())
	{
		return false;
	}

	if(GetLevel() <= 52)
	{
		CastToNPC()->Depop();
	}
	else
	{
		Message(13, "Your target is too high level to be affected by this spell.");
	}
	return false;
}

bool Mob::Handle_SE_ShadowStep(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(IsClient())
	{
		CastToClient()->SetShadowStepExemption(true);
	}
	return true;
}

bool Mob::Handle_SE_PoisonCounter(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	sint32 effect_value = CalcSpellEffectValue(spell_to_cast, effect_id_index, spell_to_cast->GetCasterLevel(), caster, 0);
	if(effect_value > 0)
	{
		if(buff_in_use)
		{
			buff_in_use->SetRemainingChargesPoison(effect_value);
		}
	}
	else
	{
		effect_value = -effect_value;
		int max_slots = GetMaxTotalSlots();
		for(int buffs_i = 0; buffs_i < max_slots; buffs_i++)
		{
			if(!buffs[buffs_i])
			{
				continue;
			}

			if(buffs[buffs_i]->GetRemainingChargesPoison() == 0)
			{
				continue;
			}

			if(effect_value > buffs[buffs_i]->GetRemainingChargesPoison())
			{
				effect_value -= buffs[buffs_i]->GetRemainingChargesPoison();
				if(caster)
				{
					caster->Message(MT_Spells, "You have cured your target from %s!", buffs[buffs_i]->GetSpell()->GetSpell().name);
					BuffFadeBySlot(buffs_i);
				}
			}
			else
			{
				buffs[buffs_i]->SetRemainingChargesPoison(buffs[buffs_i]->GetRemainingChargesPoison() - effect_value);
				effect_value = 0; 
			}
		}
	}
	return false;
}

bool Mob::Handle_SE_Rune(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(buff_in_use)
	{
		sint32 effect_value = CalcSpellEffectValue(spell_to_cast, effect_id_index, spell_to_cast->GetCasterLevel(), caster, 0);
		if(effect_value > 0)
		{
			buff_in_use->SetMeleeShield(effect_value);
			SetHasRune(true);
		}
		else
		{
			mlog(SPELLS__EFFECT_VALUES, "Rune cast with <= 0 effect value.  We can't work with this value");
		}
	}
	return false;
}

bool Mob::Handle_SE_Levitate(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	SendAppearancePacket(AT_Levitate, 2, true, true);
	return false;
}

bool Mob::Handle_SE_Illusion(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(!buff_in_use)
	{
		return false;
	}

	const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();
	SendIllusionPacket(spell.base[effect_id_index], GetDefaultGender(spell.base[effect_id_index], GetGender()), spell.base2[effect_id_index]);

	switch(spell.base[effect_id_index])
	{
	case OGRE:
		SendAppearancePacket(AT_Size, 9);
		break;
	case TROLL:
		SendAppearancePacket(AT_Size, 8);
		break;
	case VAHSHIR:
	case BARBARIAN:
		SendAppearancePacket(AT_Size, 7);
		break;
	case HALF_ELF:
	case WOOD_ELF:
	case DARK_ELF:
	case FROGLOK:
		SendAppearancePacket(AT_Size, 5);
		break;
	case DWARF:
		SendAppearancePacket(AT_Size, 4);
		break;
	case HALFLING:
	case GNOME:
		SendAppearancePacket(AT_Size, 3);
		break;
	case WOLF:
		SendAppearancePacket(AT_Size, 2);
		break;
	default:
		SendAppearancePacket(AT_Size, 6);
	}

	for(int x = 0; x < 7; x++)
	{
		SendWearChange(x);
	}
	
	if(caster && caster->GetAA(aaPermanentIllusion))
		buff_in_use->SetPermanentIllusion(true);
	return false;
}

bool Mob::Handle_SE_WipeHateList(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	int wipe_chance = spell_to_cast->GetSpell().base[effect_id_index];
	if(MakeRandomInt(0, 99) < wipe_chance)
	{
		if(IsAIControlled())
		{
			WipeHateList();
		}
		Message(13, "Your mind fogs. Who are my friends? Who are my enemies?... it was all so clear a moment ago...");
	}
	return false;
}

bool Mob::Handle_SE_EyeOfZomm(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(caster && caster->IsClient())
	{
		char eye_name[64];
		snprintf(eye_name, sizeof(eye_name), "Eye_of_%s", caster->GetCleanName());
		int pet_duration = CalcBuffDuration(caster, this, spell_to_cast) * 6;
		caster->TemporaryPets(spell_to_cast, NULL, eye_name, pet_duration);
	}
	return false;
}

bool Mob::Handle_SE_ReclaimPet(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(IsNPC())
	{
		if(caster && caster->GetID() == GetOwnerID())
		{
			if(GetPetType() != petCharmed)
			{
				if(caster->GetAA(aaImprovedReclaimEnergy) || caster->GetAA(aaImprovedReclaimEnergy2))
				{
					caster->SetMana(caster->GetMana() + (GetLevel() * 8));
				}
				else
				{
					caster->SetMana(caster->GetMana() + (GetLevel() * 4));
				}

				caster->SetPetID(0);
				SetOwnerID(0);
			}
		}
	}
	return false;
}

bool Mob::Handle_SE_BindSight(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(caster && caster->IsClient())
	{
		caster->CastToClient()->SetBindSightTarget(this);
	}
	return false;
}

bool Mob::Handle_SE_FeignDeath(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(spell_to_cast->GetSpellID() == 2488)
		return false;

	if(IsClient())
	{
		CastToClient()->SetFeigned(true);
		SendAppearancePacket(AT_Anim, ANIM_DEATH);
	}

	return false;
}

bool Mob::Handle_SE_Sentinel(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(caster)
	{
		if(caster == this)
		{
			caster->Message_StringID(MT_Spells, SENTINEL_TRIG_YOU);
			return false;
		}
		else
		{
			caster->Message_StringID(MT_Spells, SENTINEL_TRIG_OTHER, GetCleanName());
		}
	}

	Message_StringID(MT_Spells, SENTINEL_TRIG_YOU);
	return false;
}

bool Mob::Handle_SE_AbsorbMagicAtt(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(buff_in_use)
	{
		sint32 effect_value = CalcSpellEffectValue(spell_to_cast, effect_id_index, spell_to_cast->GetCasterLevel(), caster, 0);
		if(effect_value > 0)
		{
			buff_in_use->SetMagicShield(effect_value);
			SetHasSpellRune(true);
		}
		else
		{
			mlog(SPELLS__EFFECT_VALUES, "Spell rune cast with <= 0 effect value.  We can't work with this value");
		}
	}	
	return false;
}

bool Mob::Handle_SE_CurrentHPOnce(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();
	sint32 effect_value = CalcSpellEffectValue(spell_to_cast, effect_id_index, spell_to_cast->GetCasterLevel(), caster, 0);
	if(caster)
	{
		if(spell_to_cast->GetSpellID() == 2751)
		{
			effect_value = -(caster->GetMana() * 3);
		}
		else if(spell_to_cast->GetSpellID() == 2751)
		{
			effect_value = -((caster->GetHP() * 3) / 2);
			caster->SetHP(1);
			bool feign_return = Handle_SE_FeignDeath(spell_to_cast, caster, effect_id_index, partial, summoned_item, buff_in_use, buff_slot);
		}
		else if(spell_to_cast->GetSpellID() == 2766)
		{
			effect_value -= (500 * caster->GetAA(aaImprovedConsumptionofSoul));
		}
	}

	if(effect_value < 0)
	{
		Damage(caster, -effect_value, spell.id, spell.skill, false, buff_slot, false);
	}
	else if(effect_value > 0)
	{
		HealDamage(effect_value, caster);
	}
	return false;
}

bool Mob::Handle_SE_Revive(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(!IsCorpse())
	{
		return false;
	}

	if(caster)
		mlog(SPELLS__REZ, "Corpse being rezzed using spell %i by %s", spell_to_cast->GetSpellID(), caster->GetName());

	CastToCorpse()->CastRezz(spell_to_cast->GetSpellID(), caster);

	return false;
}

bool Mob::Handle_SE_SummonPC(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(caster)
	{
		if(IsClient())
		{
			CastToClient()->MovePC(zone->GetZoneID(), zone->GetInstanceID(), caster->GetX(), caster->GetY(), caster->GetZ(), caster->GetHeading(), 2, SummonPC);
			entity_list.ClearAggro(this);
		}
		else
			caster->Message(13, "This spell can only be cast on players.");
	}

	return false;
}

bool Mob::Handle_SE_Teleport(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();
	float x, y, z, heading;
	const char *target_zone;

	x = spell.base[1];
	y = spell.base[0];
	z = spell.base[2];
	heading = spell.base[3];

	if(!strcmp(spell.teleport_zone, "same"))
	{
		target_zone = 0;
	}
	else
	{
		target_zone = spell.teleport_zone;
		if(IsNPC() && target_zone != zone->GetShortName())
		{
			if(!GetOwner())
			{
				CastToNPC()->Depop();
				return false;
			}
			else
			{
				if(!GetOwner()->IsClient())
					CastToNPC()->Depop();
				return false;
			}
		}
	}

	if(spell.effectid[effect_id_index] == SE_YetAnotherGate && caster->IsClient())
	{
		if(!caster)
			return false;

		x = caster->CastToClient()->GetBindX();
		y = caster->CastToClient()->GetBindY();
		z = caster->CastToClient()->GetBindZ();
		heading = caster->CastToClient()->GetBindHeading();
		CastToClient()->MovePC(caster->CastToClient()->GetBindZoneID(), 0, x, y, z, heading);
		return false;
	}

	if(IsClient())
	{
		if(!target_zone)
			CastToClient()->MovePC(zone->GetZoneID(), zone->GetInstanceID(), x, y, z, heading);
		else
			CastToClient()->MovePC(target_zone, x, y, z, heading);
	}
	else
	{
		if(!target_zone)
			GMMove(x, y, z, heading);
	}
	return false;
}

bool Mob::Handle_SE_TossUp(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();
	double toss_amt = (double)spell.base[effect_id_index];
	if(toss_amt < 0)
		toss_amt = -toss_amt;

	if(IsNPC())
	{
		Stun(toss_amt);
	}
	toss_amt = sqrt(toss_amt) - 2.0;

	if(toss_amt < 0.0)
		toss_amt = 0.0;

	if(toss_amt > 20.0)
		toss_amt = 20.0;

	if(IsClient())
	{
		CastToClient()->SetKnockBackExemption(true);
	}

	double look_heading = GetHeading();
	look_heading /= 256;
	look_heading *= 360;
	look_heading += 180;
	if(look_heading > 360)
		look_heading -= 360;

	//x and y are crossed mkay
	double new_x = spell.pushback * sin(double(look_heading * 3.141592 / 180.0));
	double new_y = spell.pushback * cos(double(look_heading * 3.141592 / 180.0));

	EQApplicationPacket* outapp_push = new EQApplicationPacket(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
	PlayerPositionUpdateServer_Struct* spu = (PlayerPositionUpdateServer_Struct*)outapp_push->pBuffer;

	spu->spawn_id = GetID();
	spu->x_pos = FloatToEQ19(GetX());
	spu->y_pos = FloatToEQ19(GetY());
	spu->z_pos = FloatToEQ19(GetZ());
	spu->delta_x = NewFloatToEQ13(new_x);
	spu->delta_y = NewFloatToEQ13(new_y);
	spu->delta_z = NewFloatToEQ13(toss_amt);
	spu->heading = FloatToEQ19(GetHeading());
	spu->padding0002 = 0;
	spu->padding0006 = 7;
	spu->padding0014 = 0x7f;
	spu->padding0018 = 0x5df27;
	spu->animation = 0;
	spu->delta_heading = NewFloatToEQ13(0);
	outapp_push->priority = 5;
	entity_list.QueueClients(this, outapp_push, true);
	if(IsClient())
		CastToClient()->FastQueuePacket(&outapp_push);
	else
		safe_delete(outapp_push);

	return false;
}

bool Mob::Handle_SE_WeaponProc(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	uint16 procid = GetProcID(spell_to_cast, effect_id_index);

	if(spell_to_cast->GetSpell().base2[effect_id_index] == 0)
	{
		AddProcToWeapon(procid, false, 100);
	}
	else
	{
		AddProcToWeapon(procid, false, spell_to_cast->GetSpell().base2[effect_id_index] + 100);
	}

	return false;
}

bool Mob::Handle_SE_Succor(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();
	float x, y, z, heading;
	const char *target_zone;

	x = spell.base[1];
	y = spell.base[0];
	z = spell.base[2];
	heading = spell.base[3];

	if(!strcmp(spell.teleport_zone, "same"))
	{
		target_zone = 0;
	}
	else
	{
		target_zone = spell.teleport_zone;
		if(IsNPC() && target_zone != zone->GetShortName())
		{
			if(!GetOwner())
			{
				CastToNPC()->Depop();
				return false;
			}
			else
			{
				if(!GetOwner()->IsClient())
					CastToNPC()->Depop();
				return false;
			}
		}
	}

	if(IsClient())
	{
		// Below are the spellid's for known evac/succor spells that send player
		// to the current zone's safe points.
		// Succor = 1567
		// Lesser Succor = 2183
		// Evacuate = 1628
		// Lesser Evacuate = 2184
		// Decession = 2558
		// Greater Decession = 3244
		// Egress = 1566

		if(!target_zone) 
		{
			if(IsClient())
				CastToClient()->MovePC(zone->GetZoneID(), zone->GetInstanceID(), x, y, z, heading, 0, EvacToSafeCoords);
			else
				GMMove(x, y, z, heading);
		}
		else 
		{
			if(IsClient())
				CastToClient()->MovePC(target_zone, x, y, z, heading);
		}
	}

	return false;
}

bool Mob::Handle_SE_ModelSize(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	//TODO: upper/lower limits for this
	sint32 effect_value = CalcSpellEffectValue(spell_to_cast, effect_id_index, spell_to_cast->GetCasterLevel(), caster, 0);
	ChangeSize(GetSize() * (effect_value / 100.0));
	return false;
}

bool Mob::Handle_SE_SummonCorpse(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(IsClient())
	{
		Client* target_client = 0;
		if(GetTarget())
			target_client = GetTarget()->CastToClient();
		else
			target_client = CastToClient();

		// We now have a valid target for this spell. Either the caster himself or a 
		//targetted player. Lets see if the target is in the group.
		Group* group = entity_list.GetGroupByClient(target_client);
		if(group) 
		{
			if(!group->IsGroupMember(target_client))
			{
				Message(13, "Your target must be a group member for this spell.");
				return false;
			}
		}
		else
		{
			if(target_client != CastToClient()) 
			{
				Message(13, "Your target must be a group member for this spell.");
				return false;
			}
		}

		// Now we should either be casting this on self or its being cast on a valid group member
		if(target_client) 
		{
			Corpse *corpse = entity_list.GetCorpseByOwner(target_client);
			if(corpse) 
			{
				if(target_client == CastToClient())
					Message_StringID(4, SUMMONING_CORPSE, target_client->GetCleanName());
				else
					Message_StringID(4, SUMMONING_CORPSE_OTHER, target_client->GetCleanName());
							
				corpse->Summon(CastToClient(), true);
			}
			else
			{
				// No corpse found in the zone
				Message_StringID(4, CORPSE_CANT_SENSE);
			}
		}
		else
		{
			Message_StringID(4, TARGET_NOT_FOUND);
			LogFile->write(EQEMuLog::Error, "%s attempted to cast spell id %u with spell effect SE_SummonCorpse, "
				"but could not cast target into a Client object.", GetCleanName(), spell_to_cast->GetSpellID());
		}
	}
	return false;
}

bool Mob::Handle_SE_StopRain(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	zone->zone_weather = 0;
	zone->weatherSend();
	return false;
}

bool Mob::Handle_SE_Sacrifice(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(IsClient() && caster->IsClient())
	{
		CastToClient()->SacrificeConfirm(caster->CastToClient());
	}
	return false;
}

bool Mob::Handle_SE_Silence(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	InterruptSpell();
	Silence(true);
	return false;
}

bool Mob::Handle_SE_Root(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	rooted = true;
	return false;
}

bool Mob::Handle_SE_CompleteHeal(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	bool in_use = false;
	int max_slots = GetMaxTotalSlots();
	for(int buffs_i = 0; buffs_i < max_slots; buffs_i++) 
	{
		if(buffs[buffs_i] && buffs[buffs_i]->GetSpell()->GetSpellID() == spell_to_cast->GetSpellID() && buffs_i != buff_slot) 
		{
			Message(0, "You must wait before you can be affected by this spell again.");
			in_use = true;
			break;
		}
	}
	
	if(in_use)
		return false;

	HealDamage(GetMaxHP(), caster);
	return false;
}

bool Mob::Handle_SE_CallPet(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(GetPet() && GetPet()->IsNPC())
	{
		GetPet()->CastToNPC()->GMMove(GetX(), GetY(), GetZ(), GetHeading());
	}
	return false;
}

bool Mob::Handle_SE_Translocate(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(IsClient())
	{
		if(caster) 
			CastToClient()->SendOPTranslocateConfirm(caster, spell_to_cast->GetSpellID());
	}
	return false;
}

bool Mob::Handle_SE_SummonItemIntoBag(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();
	const Item_Struct *item = database.GetItem(spell.base[effect_id_index]);

	uint8 slot;

	if(!*summoned_item || !(*summoned_item)->IsType(ItemClassContainer))
	{
		if(caster) 
			caster->Message(13, "SE_SummonItemIntoBag but no bag has been summoned!");
	} 
	else if((slot = (*summoned_item)->FirstOpenSlot()) == 0xff) 
	{
		if(caster) 
			caster->Message(13, "SE_SummonItemIntoBag but no room in summoned bag!");
	} 
	else if(IsClient()) 
	{
		if (CastToClient()->CheckLoreConflict(item))
		{
			Message_StringID(0, PICK_LORE);
		} 
		else 
		{
			int charges;
			if (spell.formula[effect_id_index] < 100)
			{
				charges = spell.formula[effect_id_index];
			}
			else //variable charges
			{
				charges = CalcSpellEffectValue_formula(spell.formula[effect_id_index], 0, 20, spell_to_cast->GetCasterLevel(), spell_to_cast);
			}
			
			charges = charges < 1 ? 1 : (charges > 20 ? 20 : charges);
			ItemInst *sub_item = database.CreateItem(spell.base[effect_id_index], charges);
			if(sub_item != NULL)
			{
				(*summoned_item)->PutItem(slot, *sub_item);
				safe_delete(sub_item);
			}
		}
	}

	return false;
}

bool Mob::Handle_SE_SummonHorse(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(IsClient())
	{
		//Todo: custom horses
		CastToClient()->SummonHorse(spell_to_cast->GetSpellID());
	}
	return false;
}

bool Mob::Handle_SE_CurseCounter(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	sint32 effect_value = CalcSpellEffectValue(spell_to_cast, effect_id_index, spell_to_cast->GetCasterLevel(), caster, 0);
	if(effect_value > 0)
	{
		if(buff_in_use)
		{
			buff_in_use->SetRemainingChargesCurse(effect_value);
		}
	}
	else
	{
		effect_value = -effect_value;
		int max_slots = GetMaxTotalSlots();
		for(int buffs_i = 0; buffs_i < max_slots; buffs_i++)
		{
			if(!buffs[buffs_i])
			{
				continue;
			}

			if(buffs[buffs_i]->GetRemainingChargesCurse() == 0)
			{
				continue;
			}

			if(effect_value > buffs[buffs_i]->GetRemainingChargesCurse())
			{
				effect_value -= buffs[buffs_i]->GetRemainingChargesCurse();
				if(caster)
				{
					caster->Message(MT_Spells, "You have cured your target from %s!", buffs[buffs_i]->GetSpell()->GetSpell().name);
					BuffFadeBySlot(buffs_i);
				}
			}
			else
			{
				buffs[buffs_i]->SetRemainingChargesCurse(buffs[buffs_i]->GetRemainingChargesCurse() - effect_value);
				effect_value = 0; 
			}
		}
	}
	return false;						
}

bool Mob::Handle_SE_PercentalHeal(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();
	sint32 val = spell.max[effect_id_index];

	if(caster)
		val = caster->GetActSpellHealing(spell_to_cast, val);

	sint32 mhp = GetMaxHP();
	sint32 cap = mhp * spell.base[effect_id_index] / 100;

	if(cap < val)
		val = cap;

	if(val > 0)
		HealDamage(val, caster);	
	return false;
}

bool Mob::Handle_SE_DeathSave(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(!buff_in_use)
	{
		return false;
	}

	const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();
	sint32 effect_value = CalcSpellEffectValue(spell_to_cast, effect_id_index, spell_to_cast->GetCasterLevel(), caster, 0);
	if(caster) 
	{
		int8 success_chance = 0;
		float base_chance = 0.00f;
		float bonus_chance = 0.00f;

		switch(effect_value) 
		{
		case 1: 
			{
				base_chance = 0.10f;
			}
			break;
		case 2: 
			{
				base_chance = 0.30f;
			}
			break;
		default: 
			{
				LogFile->write(EQEMuLog::Error, "Unknown SE_DeathSave effect value in spell: %s (%i).", spell.name, spell.id);
			}
		}

		switch(caster->GetAA(aaUnfailingDivinity)) 
		{
		case 1: 
			{
				bonus_chance = 0.10f;
				break;
			}
		case 2: 
			{
				bonus_chance = 0.10f;
				break;
			}
		case 3: 
			{
				bonus_chance = 0.10f;
				break;
			}
		}

		success_chance = (((float)GetCHA() * 0.0005f) + base_chance + bonus_chance) * 100;
		buff_in_use->SetDeathSaveChance(success_chance);
		buff_in_use->SetCasterAARank(caster->GetAA(aaUnfailingDivinity));
		SetDeathSaveChance(true);
	}
	return false;
}

bool Mob::Handle_SE_SuspendPet(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(IsClient())
		CastToClient()->SuspendMinion();
	return false;
}

bool Mob::Handle_SE_TemporaryPets(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(caster)
	{
		const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();
		if((spell.id != 6882) && (spell.id != 6884)) // Chaotic Jester/Steadfast Servant
		{
			char pet_name[64];
			snprintf(pet_name, sizeof(pet_name), "%s`s pet", caster->GetCleanName());
			caster->TemporaryPets(spell_to_cast, this, pet_name);
		}
		else
			caster->TemporaryPets(spell_to_cast, this, NULL);
	}
	return false;
}

bool Mob::Handle_SE_BalanceHP(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(!caster)
		return false;

	if(!caster->IsClient())
		return false;

	const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();
	Raid *r = entity_list.GetRaidByClient(caster->CastToClient());
	if(r)
	{
		int32 gid = 0xFFFFFFFF;
		gid = r->GetGroup(caster->GetName());
		if(gid < 11)
		{
			r->BalanceHP(spell.base[effect_id_index], gid);
			return false;
		}
	}

	Group *g = entity_list.GetGroupByClient(caster->CastToClient());

	if(!g)
		return false;

	g->BalanceHP(spell.base[effect_id_index]);
		return false;
}

bool Mob::Handle_SE_DispelDetrimental(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();
	int max_slots = GetMaxTotalSlots();
	for(int buffs_i = 0; buffs_i < max_slots; buffs_i++)
	{
		if(!buffs[buffs_i])
		{
			continue;
		}
		
		if(buffs[buffs_i]->GetRemainingChargesDisease() || 
			buffs[buffs_i]->GetRemainingChargesPoison() || 
			buffs[buffs_i]->GetRemainingChargesCurse())
		{
			continue;
		}

		if(buffs[buffs_i]->GetSpell()->IsDetrimentalSpell() && !buffs[buffs_i]->IsPermanentDuration())
		{
			BuffFadeBySlot(buffs_i);
			return false;
		}
	}

	return false;
}

bool Mob::Handle_SE_IllusionCopy(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();
	if(caster && caster->GetTarget())
	{
		Mob *t = caster->GetTarget();
		SendIllusionPacket(t->GetRace(), t->GetGender(), t->GetTexture());
		caster->SendAppearancePacket(AT_Size, t->GetSize());
		for(int x = 0; x < 7; x++)
		{
			caster->SendWearChange(x);
		}
	}

	return false;
}

bool Mob::Handle_SE_MitigateSpellDamage(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(!buff_in_use)
	{
		return false;
	}

	const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();
	buff_in_use->SetMagicShield(spell.max[effect_id_index]);
	buff_in_use->SetMagicShieldReduction(spell.base[effect_id_index]);
	SetHasPartialSpellRune(true);
	return false;
}

bool Mob::Handle_SE_MitigateMeleeDamage(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(!buff_in_use)
	{
		return false;
	}

	const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();
	buff_in_use->SetMeleeShield(spell.max[effect_id_index]);
	buff_in_use->SetMeleeShieldReduction(spell.base[effect_id_index]);
	SetHasPartialMeleeRune(true);
	return false;
}

bool Mob::Handle_SE_NegateAttacks(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(!buff_in_use)
	{
		return false;
	}

	sint32 effect_value = CalcSpellEffectValue(spell_to_cast, effect_id_index, spell_to_cast->GetCasterLevel(), caster, 0);
	buff_in_use->SetAttacksNegated(effect_value);
	return false;
}

bool Mob::Handle_SE_AppraiseLDonChest(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(IsNPC())
	{
		const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();
		int check = spell.max[0];
		int target = spell.targettype;
		if(target == ST_LDoNChest_Cursed)
		{
			if(caster && caster->IsClient())
			{
				caster->CastToClient()->HandleLDoNSenseTraps(CastToNPC(), check, LDoNTypeCursed);
			}
		}
		else if(target == ST_Target)
		{
			if(caster && caster->IsClient())
			{
				caster->CastToClient()->HandleLDoNSenseTraps(CastToNPC(), check, LDoNTypeMagical);
			}
		}
	}
	return false;
}

bool Mob::Handle_SE_DisarmLDoNTrap(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(IsNPC())
	{
		const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();
		int check = spell.max[0];
		int target = spell.targettype;
		if(target == ST_LDoNChest_Cursed)
		{
			if(caster && caster->IsClient())
			{
				caster->CastToClient()->HandleLDoNDisarm(CastToNPC(), check, LDoNTypeCursed);
			}
		}
		else if(target == ST_Target)
		{
			if(caster && caster->IsClient())
			{
				caster->CastToClient()->HandleLDoNDisarm(CastToNPC(), check, LDoNTypeMagical);
			}
		}
	}	
	return false;
}

bool Mob::Handle_SE_UnlockLDoNChest(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(IsNPC())
	{
		const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();
		int check = spell.max[0];
		int target = spell.targettype;
		if(target == ST_LDoNChest_Cursed)
		{
			if(caster && caster->IsClient())
			{
				caster->CastToClient()->HandleLDoNPickLock(CastToNPC(), check, LDoNTypeCursed);
			}
		}
		else if(target == ST_Target)
		{
			if(caster && caster->IsClient())
			{
				caster->CastToClient()->HandleLDoNPickLock(CastToNPC(), check, LDoNTypeMagical);
			}
		}
	}	
	return false;
}

bool Mob::Handle_SE_CurrentEndurance(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	sint32 effect_value = CalcSpellEffectValue(spell_to_cast, effect_id_index, spell_to_cast->GetCasterLevel(), caster, 0);
	if(IsClient()) 
	{
		CastToClient()->SetEndurance(CastToClient()->GetEndurance() + effect_value);
	}
	return false;
}

bool Mob::Handle_SE_SkillAttack(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	sint32 effect_value = CalcSpellEffectValue(spell_to_cast, effect_id_index, spell_to_cast->GetCasterLevel(), caster, 0);
	const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();
	const ItemInst* itm = NULL;
	sint32 t_dmg = 0; //total damage done
	int m_dmg = 1; //min damage done
	
	/* for this, spells[spell_id].base2[i] is the damage for the attack "weapon damage" */
				
	//if we are a client, do stuff involving clients attacks
	if(caster->IsClient())
	{
		//which skill are we using, get the damage based on that skill
		switch(spell.skill)
		{
		case THROWING:
			caster->CastToClient()->GetThrownDamage(spell.base[effect_id_index], t_dmg, m_dmg);
			break;
		case ARCHERY:
			itm = caster->CastToClient()->GetInv().GetItem(SLOT_RANGE);
			if(itm)
			{
				t_dmg = effect_value + itm->GetItem()->Damage * 2 + (itm->GetItem()->Damage * 
				(GetSkill(spell.skill) + spell.base[effect_id_index] + GetDEX()) / 225);
			}
			break;
		case BASH:
			itm = caster->CastToClient()->GetInv().GetItem(SLOT_SECONDARY);
			if(itm)
				t_dmg = effect_value + ((itm->GetItem()->AC / 4) + 1) * 2 + (((itm->GetItem()->AC / 4) + 1) 
				* (GetSkill(spell.skill) + GetSTR()) / 225);
			break;
		case KICK:
		case FLYING_KICK:
		case ROUND_KICK:
			itm = caster->CastToClient()->GetInv().GetItem(SLOT_FEET);
			if(itm)
				t_dmg = effect_value + ((itm->GetItem()->AC / 2) + 1) * 2 + (((itm->GetItem()->AC / 2) + 1) *
				(GetSkill(spell.skill) + GetSTR()) / 225);
			break;
		case HAND_TO_HAND:
		case EAGLE_STRIKE:
		case TIGER_CLAW:
			itm = caster->CastToClient()->GetInv().GetItem(SLOT_HANDS);
			if(itm)
				t_dmg = effect_value + ((itm->GetItem()->AC / 2) + 1) + (((itm->GetItem()->AC / 2) + 1) * 
				(GetSkill(spell.skill) + GetSTR()) / 225);
			break;
		default:
			itm = caster->CastToClient()->GetInv().GetItem(SLOT_PRIMARY);
			if(itm)
				t_dmg = effect_value + itm->GetItem()->Damage * 2 + (itm->GetItem()->Damage * 
				(GetSkill(spell.skill) + GetSTR()) / 225);
			break;
		}

	}

	if(t_dmg == 0)
		t_dmg = effect_value;
				
	//these are considered magical attacks, so we don't need to test that
	//if they are resistable that's been taken care of, all these discs have a 10000 hit chance so they auto hit, no need to test
	if(RuleB(Combat, UseIntervalAC))
		caster->DoSpecialAttackDamage(this, spell.skill, t_dmg, m_dmg);
	else
		caster->DoSpecialAttackDamage(this, spell.skill, MakeRandomInt(1, t_dmg), m_dmg);
	
	return false;
}

bool Mob::Handle_SE_FadingMemories(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();

	if(caster && caster->IsClient())
	{
		if(caster->GetClass() == BARD && caster->GetMana() < spell.mana)
		{
			caster->Message(13, "Insufficient Mana to use this effect.");
			return false;
		}
	}

	entity_list.RemoveFromTargets(caster);
	return false;
}

bool Mob::Handle_SE_Taunt(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(IsAIControlled() && caster)
	{
		sint32 effect_value = CalcSpellEffectValue(spell_to_cast, effect_id_index, spell_to_cast->GetCasterLevel(), caster, 0);
		if(MakeRandomInt(0, 99) < effect_value)
		{
			Mob* hate_top = GetHateMost();
			if(hate_top != caster)
			{
				sint32 new_hate = hate_list.GetEntHate(hate_top) - hate_list.GetEntHate(caster) + 
					(MakeRandomInt(5, 10) * caster->GetLevel());
				AddToHateList(caster, new_hate);
			}
		}
	}
	return false;
}

bool Mob::Handle_SE_RangedProc(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();
	uint16 proc_id = GetProcID(spell_to_cast, effect_id_index);
	AddRangedProc(proc_id, spell.base2[effect_id_index]);
	return false;
}

bool Mob::Handle_SE_GroupFearImmunity(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();
	if(IsClient())
	{
		Group *g = GetGroup();
		Raid *r = GetRaid();
		uint32 time = spell.base[effect_id_index] * 10;
		if(g)
		{
			for(int gi = 0; gi < 6; gi++)
			{
				if(g->members[gi] && g->members[gi]->IsClient())
				{
					g->members[gi]->CastToClient()->EnableAAEffect(aaEffectWarcry, time);
				}
			}
		}
		else if(r)
		{
			uint32 raid_group = r->GetGroup(GetName());
			if(raid_group = 0xFFFFFFFF)
			{
				CastToClient()->EnableAAEffect(aaEffectWarcry, time);
			}
			else
			{
				for(int ri = 0; ri < 72; ri++)
				{
					if(r->members[ri].member)
					{
						if(r->members[ri].GroupNumber == raid_group)
						{
							r->members[ri].member->EnableAAEffect(aaEffectWarcry, time);
						}
					}
				}
			}
		}
		else
		{
			CastToClient()->EnableAAEffect(aaEffectWarcry, time);
		}
	}

	return false;
}

bool Mob::Handle_SE_Rampage(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	entity_list.AEAttack(caster, 30);
	return false;
}

bool Mob::Handle_SE_AETaunt(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(caster)
		entity_list.AETaunt(caster);

	return false;
}

bool Mob::Handle_SE_FleshToBone(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	sint32 effect_value = CalcSpellEffectValue(spell_to_cast, effect_id_index, spell_to_cast->GetCasterLevel(), caster, 0);
	const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();

	if(IsClient())
	{
		ItemInst* trans_item = CastToClient()->GetInv().GetItem(SLOT_CURSOR);
		if(trans_item && trans_item->IsType(ItemClassCommon) && trans_item->IsStackable())
		{
			uint32 f_charges = trans_item->GetCharges();
			//Does it sound like meat... maybe should check if it looks like meat too...
			if(strstr(trans_item->GetItem()->Name, "meat") ||
				strstr(trans_item->GetItem()->Name, "Meat") ||
				strstr(trans_item->GetItem()->Name, "flesh") ||
				strstr(trans_item->GetItem()->Name, "Flesh") ||
				strstr(trans_item->GetItem()->Name, "parts") ||
				strstr(trans_item->GetItem()->Name, "Parts"))
			{
				CastToClient()->DeleteItemInInventory(SLOT_CURSOR, f_charges, true);
				CastToClient()->SummonItem(13073, f_charges);
			}
			else
			{
				Message(13, "You can only transmute flesh to bone.");
			}
		}
		else
		{
			Message(13, "You can only transmute flesh to bone.");
		}
	}

	return false;
}

bool Mob::Handle_SE_WakeTheDead(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(caster->IsClient())
	{
		int duration = 60;
		if(spell_to_cast->GetSpellID() == 3269)
			duration += 15;
		else if(spell_to_cast->GetSpellID() == 3270)
			duration += 30;

		caster->WakeTheDead(spell_to_cast, caster->GetTarget(), duration);
	}
	return false;
}

bool Mob::Handle_SE_GateToHomeCity(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(IsClient())
	{
		CastToClient()->GoToBind(4);
	}
	return false;
}

bool Mob::Handle_SE_DefensiveProc(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();
	uint16 proc_id = GetProcID(spell_to_cast, effect_id_index);
	AddDefensiveProc(proc_id, spell.base2[effect_id_index]);
	return false;
}

bool Mob::Handle_SE_BardAEDot(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(buff_in_use)
	{
		return false;
	}

	sint32 effect_value = CalcSpellEffectValue(spell_to_cast, effect_id_index, spell_to_cast->GetCasterLevel(), caster, 0);
	const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();

	sint32 dmg = effect_value;
	if(dmg < 0)
	{
		dmg = (sint32) (dmg * partial / 100);

		if(caster)
			dmg = caster->GetActSpellDamage(spell_to_cast, dmg);

		dmg = -dmg;
		Damage(caster, dmg, spell.id, spell.skill, false, buff_slot, false);
	} 
	else if(dmg > 0) 
	{
		if(caster)
			dmg = caster->GetActSpellHealing(spell_to_cast, dmg);
		HealDamage(dmg, caster);
	}
	return false;
}

bool Mob::Handle_SE_SummonAndResAllCorpses(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	if(IsClient())
	{
		CastToClient()->SummonAndRezzAllCorpses();
	}
	return false;
}

/*
bool Mob::Handle_(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot)
{
	sint32 effect_value = CalcSpellEffectValue(spell_to_cast, effect_id_index, spell_to_cast->GetCasterLevel(), caster, 0);
	const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();
	return false;
}
*/

int Mob::CalcSpellEffectValue(const Spell *spell_to_cast, int effect_id, int caster_level, Mob *caster, int ticsremaining)
{
	int formula, base, max, effect_value;

	if(effect_id < 0 || effect_id >= EFFECT_COUNT)
		return 0;

	formula = spell_to_cast->GetSpell().formula[effect_id];
	base = spell_to_cast->GetSpell().base[effect_id];
	max = spell_to_cast->GetSpell().max[effect_id];

	if(spell_to_cast->IsBlankSpellEffect(effect_id))
		return 0;

	effect_value = CalcSpellEffectValue_formula(formula, base, max, caster_level, spell_to_cast, ticsremaining);

	if(caster && spell_to_cast->IsBardSong() &&
	(spell_to_cast->GetSpell().effectid[effect_id] != SE_AttackSpeed) &&
	(spell_to_cast->GetSpell().effectid[effect_id] != SE_AttackSpeed2) &&
	(spell_to_cast->GetSpell().effectid[effect_id] != SE_AttackSpeed3)) {
		int oval = effect_value;
		int mod = caster->GetInstrumentMod(spell_to_cast);
		effect_value = effect_value * mod / 10;
		mlog(SPELLS__BARDS, "Effect value %d altered with bard modifier of %d to yeild %d", oval, mod, effect_value);
	}

	return(effect_value);
}

// solar: generic formula calculations
int Mob::CalcSpellEffectValue_formula(int formula, int base, int max, int caster_level, const Spell* spell_to_cast, int ticsremaining)
{
	/*	
	0 = base
	1 - 99 = base + level * formulaID
	100 = base
	101 = base + level / 2
	102 = base + level
	103 = base + level * 2
	104 = base + level * 3
	105 = base + level * 4
	106 ? base + level * 5
	107 ? min + level / 2
	108 = min + level / 3
	109 = min + level / 4
	110 = min + level / 5
	119 ? min + level / 8
	121 ? min + level / 4
	122 = splurt
	123 ?
	203 = stacking issues ? max
	205 = stacking issues ? 105
	0x77 = min + level / 8
	*/

	int result = 0, updownsign = 1, ubase = base;
	if(ubase < 0)
		ubase = 0 - ubase;

	// This updown thing might look messed up but if you look at the
	// spells it actually looks like some have a positive base and max where
	// the max is actually less than the base, hence they grow downward
	/*
	This seems to mainly catch spells where both base and max are negative.
	Strangely, damage spells  have a negative base and positive max, but
	snare has both of them negative, yet their range should work the same:
	(meaning they both start at a negative value and the value gets lower)
	*/
	if (max < base && max != 0)
	{
		// values are calculated down
		updownsign = -1;
	}
	else
	{
		// values are calculated up
		updownsign = 1;
	}

	mlog(SPELLS__EFFECT_VALUES, "CSEV: spell %d, formula %d, base %d, max %d, lvl %d. Up/Down %d",
		spell_to_cast->GetSpellID(), formula, base, max, caster_level, updownsign);

	switch(formula)
	{
		case 60:	//used in stun spells..?
		case 70:
			result = ubase/100; break;
		case   0:
		case 100:	// solar: confirmed 2/6/04
			result = ubase; break;
		case 101:	// solar: confirmed 2/6/04
			result = updownsign * (ubase + (caster_level / 2)); break;
		case 102:	// solar: confirmed 2/6/04
			result = updownsign * (ubase + caster_level); break;
		case 103:	// solar: confirmed 2/6/04
			result = updownsign * (ubase + (caster_level * 2)); break;
		case 104:	// solar: confirmed 2/6/04
			result = updownsign * (ubase + (caster_level * 3)); break;
		case 105:	// solar: confirmed 2/6/04
			result = updownsign * (ubase + (caster_level * 4)); break;

		case 107:
			//Used on Reckless Strength, I think it should decay over time
			result = updownsign * (ubase + (caster_level / 2)); break;
		case 108:
			result = updownsign * (ubase + (caster_level / 3)); break;
		case 109:	// solar: confirmed 2/6/04
			result = updownsign * (ubase + (caster_level / 4)); break;

		case 110:	// solar: confirmed 2/6/04
			//is there a reason we dont use updownsign here???
			result = ubase + (caster_level / 5); break;

		case 111:
            result = updownsign * (ubase + 6 * (caster_level - spell_to_cast->GetMinLevel())); break;
		case 112:
            result = updownsign * (ubase + 8 * (caster_level - spell_to_cast->GetMinLevel())); break;
		case 113:
            result = updownsign * (ubase + 10 * (caster_level - spell_to_cast->GetMinLevel())); break;
		case 114:
            result = updownsign * (ubase + 15 * (caster_level - spell_to_cast->GetMinLevel())); break;

        //these formula were updated according to lucy 10/16/04
		case 115:	// solar: this is only in symbol of transal
			result = ubase + 6 * (caster_level - spell_to_cast->GetMinLevel()); break;
		case 116:	// solar: this is only in symbol of ryltan
            result = ubase + 8 * (caster_level - spell_to_cast->GetMinLevel()); break;
		case 117:	// solar: this is only in symbol of pinzarn
            result = ubase + 12 * (caster_level - spell_to_cast->GetMinLevel()); break;
		case 118:	// solar: used in naltron and a few others
            result = ubase + 20 * (caster_level - spell_to_cast->GetMinLevel()); break;

		case 119:	// solar: confirmed 2/6/04
			result = ubase + (caster_level / 8); break;
		case 121:	// solar: corrected 2/6/04
			result = ubase + (caster_level / 3); break;
		case 122: {
			int ticdif = spell_to_cast->GetSpell().buffduration - (ticsremaining-1);
			if(ticdif < 0)
				ticdif = 0;
			result = -(11 + 11*ticdif);
			break;
		}
		case 123:	// solar: added 2/6/04
			result = MakeRandomInt(ubase, abs(max));
			break;

		//these are used in stacking effects... formula unknown
		case 201:
		case 203:
			result = max;
			break;
		default:
			if (formula < 100)
				result = ubase + (caster_level * formula);
			else
				LogFile->write(EQEMuLog::Debug, "Unknown spell effect value forumula %d", formula);
	}

	int oresult = result;

	// now check result against the allowed maximum
	if (max != 0)
	{
		if (updownsign == 1)
		{
			if (result > max)
				result = max;
		}
		else
		{
			if (result < max)
				result = max;
		}
	}

	// if base is less than zero, then the result need to be negative too
	if (base < 0 && result > 0)
		result *= -1;

	mlog(SPELLS__EFFECT_VALUES, "Result: %d (orig %d), cap %d %s", result, oresult, max, (base < 0 && result > 0)?"Inverted due to negative base":"");

	return result;
}

void Mob::BuffProcess() 
{
	int num_buffs = current_buff_count;
	int max_slots = GetMaxTotalSlots();
	for(int i = 0; i < max_slots; i++)
	{
		if(buffs[i])
		{
			DoBuffTic(buffs[i]);
			
			//because things like dmg/death in DoBuffTic can get rid of buffs.
			if(!buffs[i])
			{
				continue;
			}
			
			if(buffs[i]->GetDurationRemaining() != 0xFFFFFFFF)
			{
				buffs[i]->SetDurationRemaining(buffs[i]->GetDurationRemaining() - 1);
				if(buffs[i]->GetDurationRemaining() == 0)
				{
					if(buffs[i]->GetSpell()->IsEffectInSpell(SE_ImprovedSpellEffect))
					{
						uint32 morph_trigger = buffs[i]->GetSpell()->GetMorphTrigger();
						Mob *morph_caster = entity_list.GetMobID(buffs[i]->GetCasterID());

						if(morph_caster)
							morph_caster->SpellOnTarget(morph_trigger, this);
						else
							SpellOnTarget(morph_trigger, this);
					}
					else
					{
						BuffFadeBySlot(i, false);
					}
				}
			}
		}
	}

	if(num_buffs > 0)
	{
		CalcSpellBonuses(&spellbonuses);
		CalcMaxHP();
		CalcMaxMana();
		SetAttackTimer();
	}
}

void Mob::DoBuffTic(const Buff *buff_to_use) 
{
	_ZP(Mob_DoBuffTic);

	Mob *caster = buff_to_use->GetSpell()->GetCaster();
	int effect, effect_value;
	const SPDat_Spell_Struct &spell = buff_to_use->GetSpell()->GetSpell();

	for (int i = 0; i < EFFECT_COUNT; i++)
	{
		if(IsDead())
			return;

		if(buff_to_use->GetSpell()->IsBlankSpellEffect(i))
			continue;

		effect = spell.effectid[i];
		//I copied the calculation into each case which needed it instead of
		//doing it every time up here, since most buff effects dont need it

		switch(effect)
		{
		case SE_CurrentHP:
		{
			effect_value = CalcSpellEffectValue(buff_to_use->GetSpell(), i, 
				buff_to_use->GetSpell()->GetCasterLevel(), caster, buff_to_use->GetDurationRemaining());

			//TODO: account for AAs and stuff
			//dont know what the signon this should be... - makes sense
			if (caster && caster->IsClient() &&
				buff_to_use->GetSpell()->IsDetrimentalSpell() &&
				effect_value < 0) {
				sint32 modifier = 100;
				modifier += caster->CastToClient()->GetFocusEffect(focusImprovedDamage, buff_to_use->GetSpell());

				if(caster){
					if(caster->IsClient() && !caster->CastToClient()->GetFeigned()){
						AddToHateList(caster, -effect_value);
					}
					else if(!caster->IsClient())
					{
						if(!IsClient())
							AddToHateList(caster, -effect_value);
					}

					TryDotCritical(buff_to_use->GetSpell(), caster, effect_value);
				}
				effect_value = effect_value * modifier / 100;
			}

			if(effect_value < 0) {
				effect_value = -effect_value;
				Damage(caster, effect_value, spell.id, spell.skill, false, i, true);
			} 
			else if(effect_value > 0) 
			{
				//healing spell...
				//healing aggro would go here; removed for now
				if(caster)
					effect_value = caster->GetActSpellHealing(buff_to_use->GetSpell(), effect_value);
				HealDamage(effect_value, caster);
			}

			break;
		}
		case SE_HealOverTime:
		{
			effect_value = CalcSpellEffectValue(buff_to_use->GetSpell(), i, 
				buff_to_use->GetSpell()->GetCasterLevel(), caster, buff_to_use->GetDurationRemaining());

			//is this affected by stuff like GetActSpellHealing??
			HealDamage(effect_value, caster);
			//healing aggro would go here; removed for now
			break;
		}

		case SE_CurrentMana:
		{
			effect_value = CalcSpellEffectValue(buff_to_use->GetSpell(), i, 
				buff_to_use->GetSpell()->GetCasterLevel(), caster, buff_to_use->GetDurationRemaining());

			SetMana(GetMana() + effect_value);
			break;
		}

		case SE_CurrentEndurance: {
			if(IsClient()) {
				effect_value = CalcSpellEffectValue(buff_to_use->GetSpell(), i, 
				buff_to_use->GetSpell()->GetCasterLevel(), caster, buff_to_use->GetDurationRemaining());

				CastToClient()->SetEndurance(CastToClient()->GetEndurance() + effect_value);
			}
			break;
		}

		case SE_BardAEDot:
		{
			effect_value = CalcSpellEffectValue(buff_to_use->GetSpell(), i, 
				buff_to_use->GetSpell()->GetCasterLevel(), caster, buff_to_use->GetDurationRemaining());

			if (invulnerable || /*effect_value > 0 ||*/ DivineAura())
				break;

			if(effect_value < 0) 
			{
				effect_value = -effect_value;
				if(caster)
				{
					if(caster->IsClient() && !caster->CastToClient()->GetFeigned())
					{
						AddToHateList(caster, effect_value);
					}
					else if(!caster->IsClient())
						AddToHateList(caster, effect_value);
				}
				Damage(caster, effect_value, spell.id, spell.skill, false, i, true);
			} 
			else if(effect_value > 0) 
			{
				//healing spell...
				HealDamage(effect_value, caster);
				//healing aggro would go here; removed for now
			}
			break;
		}

		case SE_Hate2:
			{
			effect_value = CalcSpellEffectValue(buff_to_use->GetSpell(), i, 
				buff_to_use->GetSpell()->GetCasterLevel(), caster, buff_to_use->GetDurationRemaining());
			if(caster)
			{
				if(effect_value > 0)
				{
					if(caster)
					{
						if(caster->IsClient() && !caster->CastToClient()->GetFeigned())
						{
							AddToHateList(caster, effect_value);
						}
						else if(!caster->IsClient())
							AddToHateList(caster, effect_value);
					}
				}
				else
				{
					sint32 newhate = GetHateAmount(caster) + effect_value;
					if (newhate < 1) {
						SetHate(caster,1);
					} else {
						SetHate(caster,newhate);
					}
				}
			}
			break;
		}

		case SE_Charm: {
			if (!caster || !PassCharismaCheck(caster, this, buff_to_use->GetSpell())) 
			{
				BuffFadeByEffect(SE_Charm);
			}

			break;
		}

		case SE_Root: 
			{
			float SpellEffectiveness = ResistSpell(spell.resisttype, buff_to_use->GetSpell(), caster);
			if(SpellEffectiveness < 25) 
			{
				BuffFadeByEffect(SE_Root);
			}

			break;
		}

		case SE_Hunger: {
			// this procedure gets called 7 times for every once that the stamina update occurs so we add 1/7 of the subtraction.  
			// It's far from perfect, but works without any unnecessary buff checks to bog down the server.
			if(IsClient()) {
				CastToClient()->m_pp.hunger_level += 5;
				CastToClient()->m_pp.thirst_level += 5;
			}
			break;
		}
		case SE_Invisibility:
		case SE_InvisVsAnimals:
		case SE_InvisVsUndead:
			if(buff_to_use->GetDurationRemaining() > 3)
			{
				if(!buff_to_use->GetSpell()->IsBardSong())
				{
					double break_chance = 2.0;
					if(caster)
					{
						break_chance -= (2 * (((double)caster->GetSkill(DIVINATION) + ((double)caster->GetLevel() * 3.0)) / 650.0));
					}
					else
					{
						break_chance -= (2 * (((double)GetSkill(DIVINATION) + ((double)GetLevel() * 3.0)) / 650.0));
					}

					if(MakeRandomFloat(0.0, 100.0) < break_chance)
					{
						BuffModifyDurationBySpellID(spell.id, 3);
					}
				}
			}
		
		case SE_Invisibility2:
		case SE_InvisVsUndead2:
			if(buff_to_use->GetDurationRemaining() <= 3 && buff_to_use->GetDurationRemaining() > 1)
			{
				Message_StringID(MT_Spells, INVIS_BEGIN_BREAK);
			}
			break;
		default: {
			// do we need to do anyting here?
		}
		}
	}
}

// Removes the buff in the buff slot 'slot'
void Mob::BuffFadeBySlot(int slot, bool iRecalcBonuses)
{
	if(slot < 0 || slot > GetMaxTotalSlots())
	{
		return;
	}

	mlog(SPELLS__BUFFS, "Fading buff %d from slot %d", buffs[slot]->GetSpell()->GetSpellID(), slot);

	Mob *p = entity_list.GetMob(buffs[slot]->GetCasterID());
	if (p && p != this && !buffs[slot]->GetSpell()->IsBardSong())
	{
		Mob *notify = p;
		if(p->IsPet())
			notify = p->GetOwner();
		if(p) 
		{
			notify->Message_StringID(MT_Broadcasts, SPELL_WORN_OFF_OF,
				buffs[slot]->GetSpell()->GetSpell().name, GetCleanName());
		}
	}

	DoBuffWearOffEffect(buffs[slot], slot);
	MakeBuffFadePacket(buffs[slot], slot, true);
	safe_delete(buffs[slot]);
	current_buff_count--;

	if(current_buff_count == 0)
	{
		safe_delete(buff_tic_timer);
	}

	if(IsPet() && GetOwner() && GetOwner()->IsClient()) 
	{
		SendPetBuffsToClient();
	}

	if(iRecalcBonuses)
		CalcBonuses();
}

void Mob::DoBuffWearOffEffect(const Buff *buff_to_use, uint32 buff_slot)
{
	for (int i = 0; i < EFFECT_COUNT; i++)
	{
		if(buff_to_use->GetSpell()->IsBlankSpellEffect(i))
			continue;

		const SPDat_Spell_Struct &sp = buff_to_use->GetSpell()->GetSpell();

		switch(sp.effectid[i])
		{
			case SE_WeaponProc:
			{
				uint16 procid = GetProcID(buff_to_use->GetSpell(), i);
				RemoveProcFromWeapon(procid, false);
				break;
			}

			case SE_DefensiveProc:
			{
				uint16 procid = GetProcID(buff_to_use->GetSpell(), i);
				RemoveDefensiveProc(procid);
				break;
			}

			case SE_RangedProc:
			{
				uint16 procid = GetProcID(buff_to_use->GetSpell(), i);
				RemoveRangedProc(procid);
				break;
			}

			case SE_SummonHorse:
			{
				if(IsClient())
				{
					CastToClient()->SetHorseId(0);
				}
				break;
			}

			case SE_IllusionCopy:
			case SE_Illusion:
			{
				SendIllusionPacket(0, GetBaseGender());
				if(GetRace() == OGRE)
				{
					SendAppearancePacket(AT_Size, 9);
				}
				else if(GetRace() == TROLL)
				{
					SendAppearancePacket(AT_Size, 8);
				}
				else if(GetRace() == VAHSHIR || GetRace() == FROGLOK || GetRace() == BARBARIAN)
				{
					SendAppearancePacket(AT_Size, 7);
				}
				else if(GetRace() == HALF_ELF || GetRace() == WOOD_ELF || GetRace() == DARK_ELF)
				{
					SendAppearancePacket(AT_Size, 5);
				}
				else if(GetRace() == DWARF)
				{
					SendAppearancePacket(AT_Size, 4);
				}
				else if(GetRace() == HALFLING || GetRace() == GNOME)
				{
					SendAppearancePacket(AT_Size, 3);
				}
				else
				{
					SendAppearancePacket(AT_Size, 6);
				}
				for(int x = 0; x < 7; x++)
				{
					SendWearChange(x);
				}
				break;
			}

			case SE_Levitate:
			{
				if(!AffectedExcludingSlot(buff_slot, SE_Levitate))
					SendAppearancePacket(AT_Levitate, 0);
				break;
			}

			case SE_Invisibility2:
			case SE_Invisibility:
			{
				SetInvisible(false);
				break;
			}

			case SE_InvisVsUndead2:
			case SE_InvisVsUndead:
			{
				invisible_undead = false;
				break;
			}

			case SE_InvisVsAnimals:
			{
				invisible_animals = false;
				break;
			}

			case SE_Silence:
			{
				Silence(false);
				break;
			}

			case SE_DivineAura:
			{
				SetInvul(false);
				break;
			}

			case SE_Familiar:
			{
				Mob *mypet = GetPet();
				if (mypet)
				{
					if(mypet->IsNPC())
						mypet->CastToNPC()->Depop();
					SetPetID(0);
				}
				break;
			}

			case SE_Mez:
			{
				SendAppearancePacket(AT_Anim, ANIM_STAND);
				this->mezzed = false;
				break;
			}

			case SE_Charm:
			{
				if(IsNPC())
				{
					CastToNPC()->RestoreGuardSpotCharm();
				}
				Mob* tempmob = GetOwner();
				SetOwnerID(0);
				if(tempmob)
				{
					tempmob->SetPet(0);
				}
				if (IsAIControlled())
				{
					// clear the hate list of the mobs
					entity_list.ReplaceWithTarget(this, tempmob);
					WipeHateList();
					if(tempmob)
						AddToHateList(tempmob, 1, 0);
					SendAppearancePacket(AT_Anim, ANIM_STAND);
				}
				if(tempmob && tempmob->IsClient())
				{
					EQApplicationPacket *app = new EQApplicationPacket(OP_Charm, sizeof(Charm_Struct));
					Charm_Struct *ps = (Charm_Struct*)app->pBuffer;
					ps->owner_id = tempmob->GetID();
					ps->pet_id = this->GetID();
					ps->command = 0;
					entity_list.QueueClients(this, app);
					safe_delete(app);
				}

				if(IsClient())
				{
					InterruptSpell();
					if (this->CastToClient()->IsLD())
						AI_Start(CLIENT_LD_TIMEOUT);
					else
					{
						bool feared = FindType(SE_Fear);
						if(!feared)
							AI_Stop();
					}
				}
				break;
			}

			case SE_Root:
			{
				rooted = false;
				break;
			}

			case SE_Fear:
			{
				if(RuleB(Combat, EnableFearPathing))
				{
					if(IsClient())
					{
						bool charmed = FindType(SE_Charm);
						if(!charmed)
							AI_Stop();
					}

					if(curfp) 
					{
						curfp = false;
						break;
					}
				}
				else
				{
					UnStun();
				}
				break;
			}

			case SE_BindSight:
			{
				if(IsClient())
				{
					CastToClient()->SetBindSightTarget(NULL);
				}
				break;			
			}

			case SE_MovementSpeed:
			{
				if(IsClient())
				{
					Client *my_c = CastToClient();
					int32 cur_time = Timer::GetCurrentTime();
					if((cur_time - my_c->m_TimeSinceLastPositionCheck) > 1000)
					{
						float speed = (my_c->m_DistanceSinceLastPositionCheck * 100) / (float)(cur_time - my_c->m_TimeSinceLastPositionCheck);
						float runs = my_c->GetRunspeed();
						if(speed > (runs * RuleR(Zone, MQWarpDetectionDistanceFactor)))
						{
							if(!my_c->GetGMSpeed() && (runs >= my_c->GetBaseRunspeed() || (speed > (my_c->GetBaseRunspeed() * RuleR(Zone, MQWarpDetectionDistanceFactor)))))
							{
								if(my_c->IsShadowStepExempted())
								{
									if(my_c->m_DistanceSinceLastPositionCheck > 800)
									{
										my_c->CheatDetected(MQWarpShadowStep, my_c->GetX(), my_c->GetY(), my_c->GetZ());
									}
								}
								else if(my_c->IsKnockBackExempted())
								{
									//still potential to trigger this if you're knocked back off a 
									//HUGE fall that takes > 2.5 seconds
									if(speed > 30.0f)
									{
										my_c->CheatDetected(MQWarpKnockBack, my_c->GetX(), my_c->GetY(), my_c->GetZ());
									}
								}
								else if(!my_c->IsPortExempted())
								{
									if(!my_c->IsMQExemptedArea(zone->GetZoneID(), my_c->GetX(), my_c->GetY(), my_c->GetZ()))
									{
										if(speed > (runs * 2 * RuleR(Zone, MQWarpDetectionDistanceFactor)))
										{
											my_c->m_TimeSinceLastPositionCheck = cur_time;
											my_c->m_DistanceSinceLastPositionCheck = 0.0f;
											my_c->CheatDetected(MQWarp, my_c->GetX(), my_c->GetY(), my_c->GetZ());
											//my_c->Death(my_c, 10000000, SPELL_UNKNOWN, _1H_BLUNT);
										}
										else
										{
											my_c->CheatDetected(MQWarpLight, my_c->GetX(), my_c->GetY(), my_c->GetZ());
										}
									}
								}
							}
						}
					}
					my_c->m_TimeSinceLastPositionCheck = cur_time;
					my_c->m_DistanceSinceLastPositionCheck = 0.0f;
				}
			}
		}
	}
}

//given an item/spell's focus ID and the spell being cast, determine the focus ammount, if any
//assumes that spell_id is not a bard spell and that both ids are valid spell ids
sint16 Client::CalcFocusEffect(focusType type, int16 focus_id, const Spell *spell_to_cast) {

	const SPDat_Spell_Struct &focus_spell = spells[focus_id];
	const SPDat_Spell_Struct &spell = spell_to_cast->GetSpell();

	sint16 value = 0;
	int lvlModifier = 100;

	for (int i = 0; i < EFFECT_COUNT; i++) {
		switch (focus_spell.effectid[i]) {
		case SE_Blank:
			break;

		//check limits

		//missing limits:
		//SE_LimitTarget

		case SE_LimitResist:{
			if(focus_spell.base[i]){
				if(spell.resisttype != focus_spell.base[i])
					return(0);
			}
			break;
		}
		case SE_LimitInstant:{
			if(spell.buffduration)
				return(0);
			break;
		}

		case SE_LimitMaxLevel:{
			int lvldiff = (spell.classes[(GetClass()%16) - 1]) - focus_spell.base[i];

			if(lvldiff > 0){ //every level over cap reduces the effect by spell.base2[i] percent
				lvlModifier -= spell.base2[i]*lvldiff;
				if(lvlModifier < 1)
					return 0;
			}
			break;
		}

		case SE_LimitMinLevel:
			if (spell.classes[(GetClass()%16) - 1] < focus_spell.base[i])
				return(0);
			break;

		case SE_LimitCastTime:
			if (spell.cast_time < (uint16)focus_spell.base[i])
				return(0);
			break;

		case SE_LimitSpell:
			if(focus_spell.base[i] < 0) {	//exclude spell
				if (spell_to_cast->GetSpellID() == (focus_spell.base[i]*-1))
					return(0);
			} else {
				//this makes the assumption that only one spell can be explicitly included...
				if (spell_to_cast->GetSpellID()!= focus_spell.base[i])
					return(0);
			}
			break;

		case SE_LimitMinDur:
				if (focus_spell.base[i] > CalcBuffDuration_formula(GetLevel(), spell.buffdurationformula, spell.buffduration))
					return(0);
			break;

		case SE_LimitEffect:
			if(focus_spell.base[i] < 0){
				if(spell_to_cast->IsEffectInSpell(focus_spell.base[i]))
				{ 
					//we limit this effect, can't have
					return 0;
				}
			}
			else{
				if(focus_spell.base[i] == SE_SummonPet) //summoning haste special case
				{	//must have one of the three pet effects to qualify
					if(!spell_to_cast->IsEffectInSpell(SE_SummonPet) &&
						!spell_to_cast->IsEffectInSpell(SE_NecPet) &&
						!spell_to_cast->IsEffectInSpell(SE_SummonBSTPet))
					{
						return 0;
					}
				}
				else if(!spell_to_cast->IsEffectInSpell(focus_spell.base[i]))
				{ 
					//we limit this effect, must have
					return 0;
				}
			}
			break;


		case SE_LimitSpellType:
			switch( focus_spell.base[i] )
			{
				case 0:
					if (!spell_to_cast->IsDetrimentalSpell())
						return 0;
					break;
				case 1:
					if (!spell_to_cast->IsBeneficialSpell())
						return 0;
					break;
				default:
					LogFile->write(EQEMuLog::Normal, "CalcFocusEffect:  unknown limit spelltype %d", focus_spell.base[i]);
			}
			break;



		//handle effects

		case SE_ImprovedDamage:
			switch (focus_spell.max[i])
			{
				case 0:
					if (type == focusImprovedDamage && focus_spell.base[i] > value)
					{
						value = focus_spell.base[i];
					}
					break;
				case 1:
					if (type == focusImprovedCritical && focus_spell.base[i] > value)
					{
						value = focus_spell.base[i];
					}
					break;
				case 2:
					if (type == focusImprovedUndeadDamage && focus_spell.base[i] > value)
					{
						value = focus_spell.base[i];
					}
					break;
				case 3:
					if (type == 10 && focus_spell.base[i] > value)
					{
						value = focus_spell.base[i];
					}
					break;
				default: //Resist stuff
					if (type == (focusType)focus_spell.max[i] && focus_spell.base[i] > value)
					{
						value = focus_spell.base[i];
					}
					break;
			}
			break;
		case SE_ImprovedHeal:
			if (type == focusImprovedHeal && focus_spell.base[i] > value)
			{
				value = focus_spell.base[i];
			}
			break;
		case SE_IncreaseSpellHaste:
			if (type == focusSpellHaste && focus_spell.base[i] > value)
			{
				value = focus_spell.base[i];
			}
			break;
		case SE_IncreaseSpellDuration:
			if (type == focusSpellDuration && focus_spell.base[i] > value)
			{
				value = focus_spell.base[i];
			}
			break;
		case SE_IncreaseRange:
			if (type == focusRange && focus_spell.base[i] > value)
			{
				value = focus_spell.base[i];
			}
			break;
		case SE_ReduceReagentCost:
			if (type == focusReagentCost && focus_spell.base[i] > value)
			{
				value = focus_spell.base[i];
			}
			break;
		case SE_ReduceManaCost:
			if (type == focusManaCost)
			{
				if(focus_spell.base[i] > value)
				{
					value = focus_spell.base[i];
				}
			}
			break;
		case SE_PetPowerIncrease:
			if (type == focusPetPower && focus_spell.base[i] > value)
			{
				value = focus_spell.base[i];
			}
			break;
		case SE_SpellResistReduction:
			if (type == focusResistRate && focus_spell.base[i] > value)
			{
				value = focus_spell.base[i];
			}
			break;
		case SE_SpellHateMod:
			if (type == focusSpellHateMod)
			{
				if(value != 0)
				{
					if(value > 0)
					{
						if(focus_spell.base[i] > value)
						{
							value = focus_spell.base[i];
						}
					}
					else
					{
						if(focus_spell.base[i] < value)
						{
							value = focus_spell.base[i];
						}
					}
				}
				else
					value = focus_spell.base[i];
			}
			break;
#if EQDEBUG >= 6
		//this spits up a lot of garbage when calculating spell focuses
		//since they have all kinds of extra effects on them.
		default:
			LogFile->write(EQEMuLog::Normal, "CalcFocusEffect:  unknown effectid %d", focus_spell.effectid[i]);
#endif
		}
	}
	return(value*lvlModifier/100);
}

sint16 Client::GetFocusEffect(focusType type, const Spell *spell_to_cast) 
{
	if(spell_to_cast->IsBardSong())
		return 0;
	const Item_Struct* TempItem = 0;
	const Item_Struct* UsedItem = 0;
	sint16 Total = 0;
	sint16 realTotal = 0;

	//item focus
	for(int x=0; x<=21; x++)
	{
		TempItem = NULL;
		ItemInst* ins = GetInv().GetItem(x);
		if (!ins)
			continue;
		TempItem = ins->GetItem();
		if (TempItem && TempItem->Focus.Effect > 0 && TempItem->Focus.Effect != SPELL_UNKNOWN) {
			Total = CalcFocusEffect(type, TempItem->Focus.Effect, spell_to_cast);

			if (Total > 0 && realTotal >= 0 && Total > realTotal) {
				realTotal = Total;
				UsedItem = TempItem;
			} else if (Total < 0 && Total < realTotal) {
				realTotal = Total;
				UsedItem = TempItem;
			}
		}
		for(int y = 0; y < MAX_AUGMENT_SLOTS; ++y)
		{
			ItemInst *aug = NULL;
			aug = ins->GetAugment(y);
			if(aug)
			{
				const Item_Struct* TempItemAug = aug->GetItem();
				if (TempItemAug && TempItemAug->Focus.Effect > 0 && TempItemAug->Focus.Effect != SPELL_UNKNOWN) {
					Total = CalcFocusEffect(type, TempItemAug->Focus.Effect, spell_to_cast);
					if (Total > 0 && realTotal >= 0 && Total > realTotal) {
						realTotal = Total;
						UsedItem = TempItem;
					} else if (Total < 0 && Total < realTotal) {
						realTotal = Total;
						UsedItem = TempItem;
					}
				}
			}
		}
	}

	if (realTotal != 0 && UsedItem && spell_to_cast->GetSpell().buffduration == 0) {
		Message_StringID(MT_Spells, BEGINS_TO_GLOW, UsedItem->Name);
	}

	//Spell Focus
	sint16 Total2 = 0;
	sint16 realTotal2 = 0;

	int max_slots = GetMaxTotalSlots();
	for(int buffs_i = 0; buffs_i < max_slots; buffs_i++)
	{
		if(buffs[buffs_i])
		{
			uint32 focus_spell_id = buffs[buffs_i]->GetSpell()->GetSpellID();
			Total2 = CalcFocusEffect(type, focus_spell_id, spell_to_cast);
			if (Total2 > 0 && realTotal2 >= 0 && Total2 > realTotal2) 
			{
				realTotal2 = Total2;
			} 
			else if(Total2 < 0 && Total2 < realTotal2) 
			{
				realTotal2 = Total2;
			}
		}
	}

	if(type == focusReagentCost && spell_to_cast->IsSummonPetSpell() && GetAA(aaElementalPact))
		return 100;

	if(type == focusReagentCost && (spell_to_cast->IsEffectInSpell(SE_SummonItem) || spell_to_cast->IsEffectInSpell(SE_Sacrifice))){
		return 0;
	//Summon Spells that require reagents are typically imbue type spells, enchant metal, sacrifice and shouldn't be affected
	//by reagent conservation for obvious reasons.
	}

	return realTotal + realTotal2;
}

//for some stupid reason SK procs return theirs one base off...
uint16 Mob::GetProcID(const Spell* spell_to_use, uint8 effect_index) 
{
	bool sk = false;
	bool other = false;
	for(int x = 0; x < 16; x++)
	{
		if(x == 4)
		{
			if(spell_to_use->GetSpell().classes[4] < 255)
				sk = true;
		}
		else{
			if(spell_to_use->GetSpell().classes[x] < 255)
				other = true;
		}
	}
	
	if(sk && !other)
	{
		return(spell_to_use->GetSpell().base[effect_index] + 1);
	}
	else{
		return(spell_to_use->GetSpell().base[effect_index]);
	}
}

bool Mob::TryDeathSave()
{
	int aaClientTOTD = IsClient() ? CastToClient()->GetAA(aaTouchoftheDivine) : -1;

	if (aaClientTOTD > 0) 
	{
		int aaChance = (1.2 * CastToClient()->GetAA(aaTouchoftheDivine));	
		if (MakeRandomInt(0,100) < aaChance) 
		{
			/*
			int touchHealSpellID = 4544;
			switch (aaClientTOTD) {
				case 1:
					touchHealSpellID = 4544;
					break;
				case 2:
					touchHealSpellID = 4545;
					break;
				case 3:
					touchHealSpellID = 4546;
					break;
				case 4:
					touchHealSpellID = 4547;
					break;
				case 5:
					touchHealSpellID = 4548;
					break;
			} */

			// The above spell effect is not currently working. So, do a manual heal instead:

			float touchHealAmount = 0;
			switch (aaClientTOTD) 
			{
				case 1:
					touchHealAmount = 0.15;
					break;
				case 2:
					touchHealAmount = 0.3;
					break;
				case 3:
					touchHealAmount = 0.6;
					break;
				case 4:
					touchHealAmount = 0.8;
					break;
				case 5:
					touchHealAmount = 1.0;
					break;
			}

			this->Message(0, "Divine power heals your wounds.");
			SetHP(this->max_hp * touchHealAmount);

			// and "Touch of the Divine", an Invulnerability/HoT/Purify effect, only one for all 5 levels
			Spell *tod_spell = new Spell(4789, this, this);
			SpellOnTarget(tod_spell, this);
			safe_delete(tod_spell);

			// skip checking for DI fire if this goes off...
			return true;
		}
	}

	int buffSlot = GetBuffSlotFromType(SE_DeathSave);

	if(buffSlot >= 0 && buffs[buffSlot]) 
	{
		int8 SuccessChance = buffs[buffSlot]->GetDeathSaveChance();
		int8 CasterUnfailingDivinityAARank = buffs[buffSlot]->GetCasterAARank();
		int16 BuffSpellID = buffs[buffSlot]->GetSpell()->GetSpellID();
		int SaveRoll = MakeRandomInt(0, 100);

		LogFile->write(EQEMuLog::Debug, "%s chance for a death save was %i and the roll was %i", GetCleanName(), SuccessChance, SaveRoll);

		if(SuccessChance >= SaveRoll) 
		{
			if(buffs[buffSlot]->GetSpell()->IsFullDeathSaveSpell()) 
			{
				// Lazy man's full heal.
				SetHP(50000);
			}
			else {
				SetHP(300);
			}

			entity_list.MessageClose_StringID(this, false, 200, MT_CritMelee, DIVINE_INTERVENTION, GetCleanName());

			// Fade the buff
			BuffFadeBySlot(buffSlot);
			SetDeathSaveChance(false);
			return true;
		}
		else if (CasterUnfailingDivinityAARank >= 1) 
		{
			// Roll the virtual dice to see if the target atleast gets a heal out of this
			SuccessChance = 30;
			SaveRoll = MakeRandomInt(0, 100);
			LogFile->write(EQEMuLog::Debug, "%s chance for a Unfailing Divinity AA proc was %i and the roll was %i", GetCleanName(), SuccessChance, SaveRoll);

			if(SuccessChance >= SaveRoll) 
			{
				// Yep, target gets a modest heal
				SetHP(1500);
			}
		}
	}

	return false;
}

void Mob::TryDotCritical(const Spell *spell_to_cast, Mob *caster, int &damage)
{
	if(!caster)
		return;

	float critChance = 0.00f;

	switch(caster->GetAA(aaCriticalAffliction))
	{
		case 1:
			critChance += 0.03f;
			break;
		case 2:
			critChance += 0.06f;
			break;
		case 3:
			critChance += 0.10f;
			break;
		default:
			break;
	}

	switch (caster->GetAA(aaImprovedCriticalAffliction))
	{
		case 1:
			critChance += 0.03f;
			break;
		case 2:
			critChance += 0.06f;
			break;
		case 3:
			critChance += 0.10f;
			break;
		default:
			break;
	}

	// since DOTs are the Necromancer forte, give an innate bonus
	// however, no chance to crit unless they've trained atleast one level in the AA first
	if (caster->GetClass() == NECROMANCER && critChance > 0.0f)
	{
		critChance += 0.05f;
	}

	if (critChance > 0.0f)
	{
		if (MakeRandomFloat(0, 1) <= critChance)
		{
			damage *= 2;
		}
	}
}

bool Mob::AffectedExcludingSlot(int slot, int effect)
{
	for (int i = 0; i <= EFFECT_COUNT; i++)
	{
		if (i == slot)
			continue;

		if(buffs[i])
		{
			if(buffs[i]->GetSpell()->IsEffectInSpell(effect))
				return true;
		}
	}
	return false;
}

