#include "merc.h"
#include "masterentity.h"
#include "NpcAI.h"
#include "../common/packet_dump.h"
#include "../common/eq_packet_structs.h"
#include "../common/eq_constants.h"
#include "../common/skills.h"
#include "spdat.h"
#include "zone.h"
#include "StringIDs.h"
#include "../common/MiscFunctions.h"
#include "../common/rulesys.h"
#include "QuestParserCollection.h"
#include "watermap.h"

Merc::Merc(const NPCType* d, float x, float y, float z, float heading) 
	: NPC(d, 0, x, y, z, heading, 0, false) 
{
	_baseAC = d->AC;
	_baseSTR = d->STR;
	_baseSTA = d->STA;
	_baseDEX = d->DEX;
	_baseAGI = d->AGI;
	_baseINT = d->INT;
	_baseWIS = d->WIS;
	_baseCHA = d->CHA;
	_baseATK = d->ATK;
	_baseRace = d->race;
	_baseGender = d->gender;
	_baseMR = d->MR;
	_baseCR = d->CR;
	_baseDR = d->DR;
	_baseFR = d->FR;
	_basePR = d->PR;
	_baseCorrup = d->Corrup;

	_medding = false;
	_suspended = false;
	p_depop = false;

	int r;
	for(r = 0; r <= HIGHEST_SKILL; r++) {
		skills[r] = database.GetSkillCap(GetClass(),(SkillType)r,GetLevel());
	}

	CalcBonuses();

	SetHP(GetMaxHP());

	AI_Init();
	AI_Start();
}

Merc::~Merc() {
	entity_list.RemoveMerc(this->GetID());
	UninitializeBuffSlots();
}

void Merc::CalcBonuses()
{
	//_ZP(Merc_CalcBonuses);
	GenerateBaseStats();
	memset(&itembonuses, 0, sizeof(StatBonuses));
	memset(&aabonuses, 0, sizeof(StatBonuses));
	CalcItemBonuses(&itembonuses);
	
	CalcSpellBonuses(&spellbonuses);

	//_log(AA__BONUSES, "Calculating AA Bonuses for %s.", this->GetCleanName());
	//CalcAABonuses(&aabonuses);	//we're not quite ready for this
	//_log(AA__BONUSES, "Finished calculating AA Bonuses for %s.", this->GetCleanName());
	
	CalcAC();
	CalcATK();
	//CalcHaste();
	
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
	CalcCorrup();
	
	CalcMaxHP();
	CalcMaxMana();
	CalcMaxEndurance();
	
	rooted = FindType(SE_Root);
}

void Merc::GenerateBaseStats() {

	// base stats
	uint16 Strength = _baseSTR;
	uint16 Stamina = _baseSTA;
	uint16 Dexterity = _baseDEX;
	uint16 Agility = _baseAGI;
	uint16 Wisdom = _baseWIS;
	uint16 Intelligence = _baseINT;
	uint16 Charisma = _baseCHA;
	uint16 Attack = _baseATK;
	sint16 MagicResist = _baseMR;
	sint16 FireResist = _baseFR;
	sint16 DiseaseResist = _baseDR;
	sint16 PoisonResist = _basePR;
	sint16 ColdResist = _baseCR;
	sint16 CorruptionResist = _baseCorrup;

	switch(this->GetClass()) {
		case 1: // Warrior
			Strength += 10;
			Stamina += 20;
			Agility += 10;
			Dexterity += 10;
			Attack += 12;
			break;
		case 2: // Cleric
			Strength += 5;
			Stamina += 5;
			Agility += 10;
			Wisdom += 30;
			Attack += 8;
			break;
		case 4: // Ranger
			Strength += 15;
			Stamina += 10;
			Agility += 10;
			Wisdom += 15;
			Attack += 17;
			break;
		case 9: // Rogue
			Strength += 10;
			Stamina += 20;
			Agility += 10;
			Dexterity += 10;
			Attack += 12;
			break;
		case 12: // Wizard
			Stamina += 20;
			Intelligence += 30;
			Attack += 5;
			break;
	}

	this->_baseSTR = Strength;
	this->_baseSTA = Stamina;
	this->_baseDEX = Dexterity;
	this->_baseAGI = Agility;
	this->_baseWIS = Wisdom;
	this->_baseINT = Intelligence;
	this->_baseCHA = Charisma;
	this->_baseATK = Attack;
	this->_baseMR = MagicResist;
	this->_baseFR = FireResist;
	this->_baseDR = DiseaseResist;
	this->_basePR = PoisonResist;
	this->_baseCR = ColdResist;
	this->_baseCorrup = CorruptionResist;
}

int Merc::CalcRecommendedLevelBonus(int8 level, uint8 reclevel, int basestat)
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

void Merc::CalcItemBonuses(StatBonuses* newbon) {
	//memset assumed to be done by caller.
	

	unsigned int i;
	//should not include 21 (SLOT_AMMO)
	for (i=0; i<SLOT_AMMO; i++) {
		const ItemInst* inst = m_inv[i];
		if(inst == 0)
			continue;
		AddItemBonuses(inst, newbon);
	}
	
	//Power Source Slot
	if (GetClientVersion() >= EQClientSoF)
	{
		const ItemInst* inst = m_inv[9999];
		if(inst)
			AddItemBonuses(inst, newbon);
	}

	//tribute items
	for (i = 0; i < MAX_PLAYER_TRIBUTES; i++) {
		const ItemInst* inst = m_inv[TRIBUTE_SLOT_START + i];
		if(inst == 0)
			continue;
		AddItemBonuses(inst, newbon, false, true);
	}
	// Caps
	if(newbon->HPRegen > CalcHPRegenCap())
		newbon->HPRegen = CalcHPRegenCap();

	if(newbon->ManaRegen > CalcManaRegenCap())
		newbon->ManaRegen = CalcManaRegenCap();
	
	if(newbon->EnduranceRegen > CalcEnduranceRegenCap())
		newbon->EnduranceRegen = CalcEnduranceRegenCap();
			
	SetAttackTimer();
}
		
void Merc::AddItemBonuses(const ItemInst *inst, StatBonuses* newbon, bool isAug, bool isTribute) {
	if(!inst || !inst->IsType(ItemClassCommon))
	{
		return;
	}

	if(inst->GetAugmentType()==0 && isAug == true)
	{
		return;
	}

	const Item_Struct *item = inst->GetItem();

	if(!isTribute && !inst->IsEquipable(GetBaseRace(),GetClass()))
	{
		if(item->ItemType != ItemTypeFood && item->ItemType != ItemTypeDrink)
			return;
	}

	if(GetLevel() < item->ReqLevel)
	{
		return;
	}

	if(GetLevel() >= item->RecLevel)
	{
		newbon->AC += item->AC;
		newbon->HP += item->HP;
		newbon->Mana += item->Mana;
		newbon->Endurance += item->Endur;
		newbon->STR += (item->AStr + item->HeroicStr);
		newbon->STA += (item->ASta + item->HeroicSta);
		newbon->DEX += (item->ADex + item->HeroicDex);
		newbon->AGI += (item->AAgi + item->HeroicAgi);
		newbon->INT += (item->AInt + item->HeroicInt);
		newbon->WIS += (item->AWis + item->HeroicWis);
		newbon->CHA += (item->ACha + item->HeroicCha);
		
		newbon->MR += (item->MR + item->HeroicMR);
		newbon->FR += (item->FR + item->HeroicFR);
		newbon->CR += (item->CR + item->HeroicCR);
		newbon->PR += (item->PR + item->HeroicPR);
		newbon->DR += (item->DR + item->HeroicDR);
		newbon->Corrup += (item->SVCorruption + item->HeroicSVCorrup);

		newbon->STRCapMod += item->HeroicStr;
		newbon->STACapMod += item->HeroicSta;
		newbon->DEXCapMod += item->HeroicDex;
		newbon->AGICapMod += item->HeroicAgi;
		newbon->INTCapMod += item->HeroicInt;
		newbon->WISCapMod += item->HeroicWis;
		newbon->CHACapMod += item->HeroicCha;
		newbon->MRCapMod += item->HeroicMR;
		newbon->CRCapMod += item->HeroicFR;
		newbon->FRCapMod += item->HeroicCR;
		newbon->PRCapMod += item->HeroicPR;
		newbon->DRCapMod += item->HeroicDR;
		newbon->CorrupCapMod += item->HeroicSVCorrup;

		newbon->HeroicSTR += item->HeroicStr;
		newbon->HeroicSTA += item->HeroicSta;
		newbon->HeroicDEX += item->HeroicDex;
		newbon->HeroicAGI += item->HeroicAgi;
		newbon->HeroicINT += item->HeroicInt;
		newbon->HeroicWIS += item->HeroicWis;
		newbon->HeroicCHA += item->HeroicCha;
		newbon->HeroicMR += item->HeroicMR;
		newbon->HeroicFR += item->HeroicFR;
		newbon->HeroicCR += item->HeroicCR;
		newbon->HeroicPR += item->HeroicPR;
		newbon->HeroicDR += item->HeroicDR;
		newbon->HeroicCorrup += item->HeroicSVCorrup;

	}
	else
	{
		int lvl = GetLevel();
		int reclvl = item->RecLevel;

		newbon->AC += CalcRecommendedLevelBonus( lvl, reclvl, item->AC );
		newbon->HP += CalcRecommendedLevelBonus( lvl, reclvl, item->HP );
		newbon->Mana += CalcRecommendedLevelBonus( lvl, reclvl, item->Mana );
		newbon->Endurance += CalcRecommendedLevelBonus( lvl, reclvl, item->Endur );
		newbon->STR += CalcRecommendedLevelBonus( lvl, reclvl, (item->AStr + item->HeroicStr) );
		newbon->STA += CalcRecommendedLevelBonus( lvl, reclvl, (item->ASta + item->HeroicSta) );
		newbon->DEX += CalcRecommendedLevelBonus( lvl, reclvl, (item->ADex + item->HeroicDex) );
		newbon->AGI += CalcRecommendedLevelBonus( lvl, reclvl, (item->AAgi + item->HeroicAgi) );
		newbon->INT += CalcRecommendedLevelBonus( lvl, reclvl, (item->AInt + item->HeroicInt) );
		newbon->WIS += CalcRecommendedLevelBonus( lvl, reclvl, (item->AWis + item->HeroicWis) );
		newbon->CHA += CalcRecommendedLevelBonus( lvl, reclvl, (item->ACha + item->HeroicCha) );

		newbon->MR += CalcRecommendedLevelBonus( lvl, reclvl, (item->MR + item->HeroicMR) );
		newbon->FR += CalcRecommendedLevelBonus( lvl, reclvl, (item->FR + item->HeroicFR) );
		newbon->CR += CalcRecommendedLevelBonus( lvl, reclvl, (item->CR + item->HeroicCR) );
		newbon->PR += CalcRecommendedLevelBonus( lvl, reclvl, (item->PR + item->HeroicPR) );
		newbon->DR += CalcRecommendedLevelBonus( lvl, reclvl, (item->DR + item->HeroicDR) );
		newbon->Corrup += CalcRecommendedLevelBonus( lvl, reclvl, (item->SVCorruption + item->HeroicSVCorrup) );

		newbon->STRCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicStr );
		newbon->STACapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicSta );
		newbon->DEXCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicDex );
		newbon->AGICapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicAgi );
		newbon->INTCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicInt );
		newbon->WISCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicWis );
		newbon->CHACapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicCha );
		newbon->MRCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicMR );
		newbon->CRCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicFR );
		newbon->FRCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicCR );
		newbon->PRCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicPR );
		newbon->DRCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicDR );
		newbon->CorrupCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicSVCorrup );

		newbon->HeroicSTR += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicStr );
		newbon->HeroicSTA += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicSta );
		newbon->HeroicDEX += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicDex );
		newbon->HeroicAGI += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicAgi );
		newbon->HeroicINT += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicInt );
		newbon->HeroicWIS += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicWis );
		newbon->HeroicCHA += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicCha );
		newbon->HeroicMR += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicMR );
		newbon->HeroicFR += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicFR );
		newbon->HeroicCR += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicCR );
		newbon->HeroicPR += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicPR );
		newbon->HeroicDR += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicDR );
		newbon->HeroicCorrup += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicSVCorrup );
	}
	
	//FatherNitwit: New style haste, shields, and regens
	if(newbon->haste < (sint16)item->Haste) {
		newbon->haste = item->Haste;
	}
	if(item->Regen > 0)
		newbon->HPRegen += item->Regen;

	if(item->ManaRegen > 0)
		newbon->ManaRegen += item->ManaRegen;
	
	if(item->EnduranceRegen > 0)
		newbon->EnduranceRegen += item->EnduranceRegen;

	if(item->Attack > 0) {
		
		int cap = RuleI(Character, ItemATKCap);
		cap += itembonuses.ItemATKCap + spellbonuses.ItemATKCap + aabonuses.ItemATKCap; 

		if((newbon->ATK + item->Attack) > cap)
			newbon->ATK = RuleI(Character, ItemATKCap);
		else
			newbon->ATK += item->Attack;
	}
	if(item->DamageShield > 0) {
		if((newbon->DamageShield + item->DamageShield) > RuleI(Character, ItemDamageShieldCap))
			newbon->DamageShield = RuleI(Character, ItemDamageShieldCap);
		else
			newbon->DamageShield += item->DamageShield;
	}
	if(item->SpellShield > 0) {
		if((newbon->SpellShield + item->SpellShield) > RuleI(Character, ItemSpellShieldingCap))
			newbon->SpellShield = RuleI(Character, ItemSpellShieldingCap);
		else
			newbon->SpellShield += item->SpellShield;
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
	
	if(item->HealAmt > 0) {
		if((newbon->HealAmt + item->HealAmt) > RuleI(Character, ItemHealAmtCap))
			newbon->HealAmt = RuleI(Character, ItemHealAmtCap);
		else
			newbon->HealAmt += item->HealAmt;
	}
	if(item->SpellDmg > 0) {
		if((newbon->SpellDmg + item->SpellDmg) > RuleI(Character, ItemSpellDmgCap))
			newbon->SpellDmg = RuleI(Character, ItemSpellDmgCap);
		else
			newbon->SpellDmg += item->SpellDmg;
	}
	if(item->Clairvoyance > 0) {
		if((newbon->Clairvoyance + item->Clairvoyance) > RuleI(Character, ItemClairvoyanceCap))
			newbon->Clairvoyance = RuleI(Character, ItemClairvoyanceCap);
		else
			newbon->Clairvoyance += item->Clairvoyance;
	}
	
	if(item->DSMitigation > 0) {
		if((newbon->DSMitigation + item->DSMitigation) > RuleI(Character, ItemDSMitigationCap))
			newbon->DSMitigation = RuleI(Character, ItemDSMitigationCap);
		else
			newbon->DSMitigation += item->DSMitigation;
	}
	if (item->Worn.Effect>0 && (item->Worn.Type == ET_WornEffect)) { // latent effects
		ApplySpellsBonuses(item->Worn.Effect, item->Worn.Level, newbon, 0, true);
	}

	if (item->Focus.Effect>0 && (item->Focus.Type == ET_Focus)) { // focus effects
		ApplySpellsBonuses(item->Focus.Effect, item->Focus.Level, newbon, 0, true);
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
	
	if (item->SkillModValue != 0 && item->SkillModType <= HIGHEST_SKILL){
		if ((item->SkillModValue > 0 && newbon->skillmod[item->SkillModType] < item->SkillModValue) ||
			(item->SkillModValue < 0 && newbon->skillmod[item->SkillModType] > item->SkillModValue))
		{
			newbon->skillmod[item->SkillModType] = item->SkillModValue;
		}
	}

	// Add Item Faction Mods
	if (item->FactionMod1)
	{
		if (item->FactionAmt1 > 0 && item->FactionAmt1 > GetItemFactionBonus(item->FactionMod1))
		{
			AddItemFactionBonus(item->FactionMod1, item->FactionAmt1);
		}
		else if (item->FactionAmt1 < 0 && item->FactionAmt1 < GetItemFactionBonus(item->FactionMod1))
		{
			AddItemFactionBonus(item->FactionMod1, item->FactionAmt1);
		}
	}
	if (item->FactionMod2)
	{
		if (item->FactionAmt2 > 0 && item->FactionAmt2 > GetItemFactionBonus(item->FactionMod2))
		{
			AddItemFactionBonus(item->FactionMod2, item->FactionAmt2);
		}
		else if (item->FactionAmt2 < 0 && item->FactionAmt2 < GetItemFactionBonus(item->FactionMod2))
		{
			AddItemFactionBonus(item->FactionMod2, item->FactionAmt2);
		}
	}
	if (item->FactionMod3)
	{
		if (item->FactionAmt3 > 0 && item->FactionAmt3 > GetItemFactionBonus(item->FactionMod3))
		{
			AddItemFactionBonus(item->FactionMod3, item->FactionAmt3);
		}
		else if (item->FactionAmt3 < 0 && item->FactionAmt3 < GetItemFactionBonus(item->FactionMod3))
		{
			AddItemFactionBonus(item->FactionMod3, item->FactionAmt3);
		}
	}
	if (item->FactionMod4)
	{
		if (item->FactionAmt4 > 0 && item->FactionAmt4 > GetItemFactionBonus(item->FactionMod4))
		{
			AddItemFactionBonus(item->FactionMod4, item->FactionAmt4);
		}
		else if (item->FactionAmt4 < 0 && item->FactionAmt4 < GetItemFactionBonus(item->FactionMod4))
		{
			AddItemFactionBonus(item->FactionMod4, item->FactionAmt4);
		}
	}
	
	if (item->ExtraDmgSkill != 0 && item->ExtraDmgSkill <= HIGHEST_SKILL) {
		if((newbon->SkillDamageAmount[item->ExtraDmgSkill] + item->ExtraDmgAmt) > RuleI(Character, ItemExtraDmgCap))
			newbon->SkillDamageAmount[item->ExtraDmgSkill] = RuleI(Character, ItemExtraDmgCap);
		else
			newbon->SkillDamageAmount[item->ExtraDmgSkill] += item->ExtraDmgAmt;
	}

	if (!isAug)
	{
		int i;
		for(i = 0; i < MAX_AUGMENT_SLOTS; i++) {
			AddItemBonuses(inst->GetAugment(i),newbon,true);
		}
	}

}

int Merc::GroupLeadershipAAHealthEnhancement()
{
	Group *g = GetGroup();

	if(!g || (g->GroupCount() < 3))
		return 0;

	switch(g->GetLeadershipAA(groupAAHealthEnhancement))
	{
		case 0:
			return 0;
		case 1:
			return 30;
		case 2:
			return 60;
		case 3:
			return 100;
	}

	return 0;
}

int Merc::GroupLeadershipAAManaEnhancement()
{
	Group *g = GetGroup();

	if(!g || (g->GroupCount() < 3))
		return 0;

	switch(g->GetLeadershipAA(groupAAManaEnhancement))
	{
		case 0:
			return 0;
		case 1:
			return 30;
		case 2:
			return 60;
		case 3:
			return 100;
	}

	return 0;
}

int Merc::GroupLeadershipAAHealthRegeneration()
{
	Group *g = GetGroup();

	if(!g || (g->GroupCount() < 3))
		return 0;

	switch(g->GetLeadershipAA(groupAAHealthRegeneration))
	{
		case 0:
			return 0;
		case 1:
			return 4;
		case 2:
			return 6;
		case 3:
			return 8;
	}

	return 0;
}

int Merc::GroupLeadershipAAOffenseEnhancement()
{
	Group *g = GetGroup();

	if(!g || (g->GroupCount() < 3))
		return 0;

	switch(g->GetLeadershipAA(groupAAOffenseEnhancement))
	{
		case 0:
			return 0;
		case 1:
			return 10;
		case 2:
			return 19;
		case 3:
			return 28;
		case 4:
			return 34;
		case 5:
			return 40;
	}
	return 0;
}

sint16 Merc::GetMaxStat() const {
	if((RuleI(Character, StatCap)) > 0)
		return (RuleI(Character, StatCap));

	int level = GetLevel();
	
	sint16 base = 0;
	
	if (level < 61) {
		base = 255;
	}
	else if (GetClientVersion() >= EQClientSoF) {
		base = 255 + 5 * (level - 60);
	}
	else if (level < 71) {
		base = 255 + 5 * (level - 60);
	}
	else {
		base = 330;
	}
	
	return(base);
}

sint16 Merc::GetMaxResist() const 
{
	int level = GetLevel();

	sint16 base = 500;
	
	if(level > 60)
		base += ((level - 60) * 5);

	return base;
}

sint16 Merc::GetMaxSTR() const {
	return GetMaxStat()
		+ itembonuses.STRCapMod
		+ spellbonuses.STRCapMod
		+ aabonuses.STRCapMod;
}
sint16 Merc::GetMaxSTA() const {
	return GetMaxStat()
		+ itembonuses.STACapMod
		+ spellbonuses.STACapMod
		+ aabonuses.STACapMod;
}
sint16 Merc::GetMaxDEX() const {
	return GetMaxStat()
		+ itembonuses.DEXCapMod
		+ spellbonuses.DEXCapMod
		+ aabonuses.DEXCapMod;
}
sint16 Merc::GetMaxAGI() const {
	return GetMaxStat()
		+ itembonuses.AGICapMod
		+ spellbonuses.AGICapMod
		+ aabonuses.AGICapMod;
}
sint16 Merc::GetMaxINT() const {
	return GetMaxStat()
		+ itembonuses.INTCapMod
		+ spellbonuses.INTCapMod
		+ aabonuses.INTCapMod;
}
sint16 Merc::GetMaxWIS() const {
	return GetMaxStat()
		+ itembonuses.WISCapMod
		+ spellbonuses.WISCapMod
		+ aabonuses.WISCapMod;
}
sint16 Merc::GetMaxCHA() const {
	return GetMaxStat()
		+ itembonuses.CHACapMod
		+ spellbonuses.CHACapMod
		+ aabonuses.CHACapMod;
}
sint16 Merc::GetMaxMR() const {
	return GetMaxResist()
		+ itembonuses.MRCapMod
		+ spellbonuses.MRCapMod
		+ aabonuses.MRCapMod;
}
sint16 Merc::GetMaxPR() const {
	return GetMaxResist()
		+ itembonuses.PRCapMod
		+ spellbonuses.PRCapMod
		+ aabonuses.PRCapMod;
}
sint16 Merc::GetMaxDR() const {
	return GetMaxResist()
		+ itembonuses.DRCapMod
		+ spellbonuses.DRCapMod
		+ aabonuses.DRCapMod;
}
sint16 Merc::GetMaxCR() const {
	return GetMaxResist()
		+ itembonuses.CRCapMod
		+ spellbonuses.CRCapMod
		+ aabonuses.CRCapMod;
}
sint16 Merc::GetMaxFR() const {
	return GetMaxResist()
		+ itembonuses.FRCapMod
		+ spellbonuses.FRCapMod
		+ aabonuses.FRCapMod;
}
sint16 Merc::GetMaxCorrup() const {
	return GetMaxResist()
		+ itembonuses.CorrupCapMod
		+ spellbonuses.CorrupCapMod
		+ aabonuses.CorrupCapMod;
}

sint32 Merc::LevelRegen()
{
	bool sitting = IsSitting();
	int level = GetLevel();
	//bool bonus = GetRaceBitmask(GetBaseRace()) & RuleI(Character, BaseHPRegenBonusRaces);
	//uint8 multiplier1 = bonus ? 2 : 1;
	sint32 hp = 0;

	//these calculations should match up with the info from Monkly Business, which was last updated ~05/2008: http://www.monkly-business.net/index.php?pageid=abilities
	if (level < 51) {
		if (sitting) {
			if (level < 20)
				hp += 2;
			else if (level < 50)
				hp += 3;
			else	//level == 50
				hp += 4;
		}
		else	//feigned or standing
			hp += 1;
	}
	//there may be an easier way to calculate this next part, but I don't know what it is
	else {	//level >= 51
		sint32 tmp = 0;

		if (level < 56) {
			tmp = 2;
		}
		else if (level < 60) {
			tmp = 3;
		}
		else if (level < 61) {
			tmp = 4;
		}
		else if (level < 63) {
			tmp = 5;
		}
		else if (level < 65) {
			tmp = 6;
		}
		else {	//level >= 65
			tmp = 7;
		}

		hp += sint32(float(tmp));

		if (sitting)
			hp += 3;
	}

	return hp;
}

sint16 Merc::CalcSTR() {
	sint16 val = _baseSTR + itembonuses.STR + spellbonuses.STR;
	
	sint16 mod = aabonuses.STR;
	
	if(val>255 && GetLevel() <= 60)
		val = 255;
	STR = val + mod;
	
	if(STR < 1)
		STR = 1;

	int m = GetMaxSTR();
	if(STR > m)
		STR = m;
	
	return(STR);
}

sint16 Merc::CalcSTA() {
	sint16 val = _baseSTA + itembonuses.STA + spellbonuses.STA;
	
	sint16 mod = aabonuses.STA;
	
	if(val>255 && GetLevel() <= 60)
		val = 255;
	STA = val + mod;
	
	if(STA < 1)
		STA = 1;

	int m = GetMaxSTA();
	if(STA > m)
		STA = m;
	
	return(STA);
}

sint16 Merc::CalcAGI() {
	sint16 val = _baseAGI + itembonuses.AGI + spellbonuses.AGI;
	sint16 mod = aabonuses.AGI;

	if(val>255 && GetLevel() <= 60)
		val = 255;

	sint16 str = GetSTR();
	
	AGI = val + mod;

	if(AGI < 1)
		AGI = 1;

	int m = GetMaxAGI();
	if(AGI > m)
		AGI = m;
	
	return(AGI);
}

sint16 Merc::CalcDEX() {
	sint16 val = _baseDEX + itembonuses.DEX + spellbonuses.DEX;
	
	sint16 mod = aabonuses.DEX;
	
	if(val>255 && GetLevel() <= 60)
		val = 255;
	DEX = val + mod;
	
	if(DEX < 1)
		DEX = 1;

	int m = GetMaxDEX();
	if(DEX > m)
		DEX = m;
	
	return(DEX);
}

sint16 Merc::CalcINT() {
	sint16 val = _baseINT + itembonuses.INT + spellbonuses.INT;

	sint16 mod = aabonuses.INT;
	
	if(val>255 && GetLevel() <= 60)
		val = 255;
	INT = val + mod;

	if(INT < 1)
		INT = 1;

	int m = GetMaxINT();
	if(INT > m)
		INT = m;
	
	return(INT);
}

sint16 Merc::CalcWIS() {
	sint16 val = _baseWIS + itembonuses.WIS + spellbonuses.WIS;
	
	sint16 mod = aabonuses.WIS;
	
	if(val>255 && GetLevel() <= 60)
		val = 255;
	WIS = val + mod;

	if(WIS < 1)
		WIS = 1;

	int m = GetMaxWIS();
	if(WIS > m)
		WIS = m;
	
	return(WIS);
}

sint16 Merc::CalcCHA() {
	sint16 val = _baseCHA + itembonuses.CHA + spellbonuses.CHA;
	
	sint16 mod = aabonuses.CHA;
	
	if(val>255 && GetLevel() <= 60)
		val = 255;
	CHA = val + mod;
	
	if(CHA < 1)
		CHA = 1;

	int m = GetMaxCHA();
	if(CHA > m)
		CHA = m;
	
	return(CHA);
}

//The AA multipliers are set to be 5, but were 2 on WR
//The resistant discipline which I think should be here is implemented
//in Mob::ResistSpell
sint16	Merc::CalcMR()
{
	MR = _baseMR + itembonuses.MR + spellbonuses.MR + aabonuses.MR;
	
	if(GetClass() == WARRIOR)
		MR += GetLevel() / 2;
	
	if(MR < 1)
		MR = 1;

	if(MR > GetMaxMR())
		MR = GetMaxMR();

	return(MR);
}

sint16	Merc::CalcFR()
{
	FR = _baseFR;
	
	int c = GetClass();
	if(c == RANGER) {
		FR += 4;
		
		int l = GetLevel();
		if(l > 49)
			FR += l - 49;
	}
	
	FR += itembonuses.FR + spellbonuses.FR + aabonuses.FR;
	
	if(FR < 1)
		FR = 1;
	
	if(FR > GetMaxFR())
		FR = GetMaxFR();

	return(FR);
}

sint16	Merc::CalcDR()
{
	DR = _baseDR + itembonuses.DR + spellbonuses.DR + aabonuses.DR;
	
	if(DR < 1)
		DR = 1;

	if(DR > GetMaxDR())
		DR = GetMaxDR();

	return(DR);
}

sint16	Merc::CalcPR()
{
	PR = _basePR;
	
	int c = GetClass();
	if(c == ROGUE) {
		PR += 8;
		
		int l = GetLevel();
		if(l > 49)
			PR += l - 49;

	}
	
	PR += itembonuses.PR + spellbonuses.PR + aabonuses.PR;
	
	if(PR < 1)
		PR = 1;

	if(PR > GetMaxPR())
		PR = GetMaxPR();

	return(PR);
}

sint16	Merc::CalcCR()
{
	CR = _baseCR;
	
	int c = GetClass();
	if(c == RANGER) {
		CR += 4;
		
		int l = GetLevel();
		if(l > 49)
			CR += l - 49;
	}
	
	CR += itembonuses.CR + spellbonuses.CR + aabonuses.CR;
	
	if(CR < 1)
		CR = 1;

	if(CR > GetMaxCR())
		CR = GetMaxCR();

	return(CR);
}

sint16	Merc::CalcCorrup()
{
	Corrup = _baseCorrup + itembonuses.Corrup + spellbonuses.Corrup + aabonuses.Corrup;
	
	if(Corrup > GetMaxCorrup())
		Corrup = GetMaxCorrup();

	return(Corrup);
}

sint16 Merc::CalcATK() {
	ATK = itembonuses.ATK + spellbonuses.ATK + aabonuses.ATK + GroupLeadershipAAOffenseEnhancement();
	return(ATK);
}

sint16 Merc::CalcAC() {

	// new formula
	int avoidance = (acmod() + ((GetSkill(DEFENSE) + itembonuses.HeroicAGI/10)*16)/9);
	if (avoidance < 0)
		avoidance = 0;

	int mitigation = 0;
	if (GetClass() == WIZARD) {
		mitigation = (GetSkill(DEFENSE) + itembonuses.HeroicAGI/10)/4 + (itembonuses.AC+1);
		//this might be off by 4..
		mitigation -= 4;
	} else {
		mitigation = (GetSkill(DEFENSE) + itembonuses.HeroicAGI/10)/3 + ((itembonuses.AC*4)/3);
	}
	int displayed = 0;
	displayed += ((avoidance+mitigation)*1000)/847;	//natural AC
	
	//Iksar AC, untested
	if (GetRace() == IKSAR) {
		displayed += 12;
		int iksarlevel = GetLevel();
		iksarlevel -= 10;
		if (iksarlevel > 25)
			iksarlevel = 25;
		if (iksarlevel > 0)
			displayed += iksarlevel * 12 / 10;
	}
	
	// Shield AC bonus for HeroicSTR
	if(itembonuses.HeroicSTR) {
		bool equiped = m_inv.GetItem(SLOT_SECONDARY);
		if(equiped) {
			uint8 shield = m_inv.GetItem(SLOT_SECONDARY)->GetItem()->ItemType;
			if(shield == ItemTypeShield) 
				displayed += itembonuses.HeroicSTR/2;
		}
	}
	
	//spell AC bonuses are added directly to natural total
	displayed += spellbonuses.AC;
	
	AC = displayed;
	return(AC);
}

sint16 Merc::acmod() {
	int agility = GetAGI();
	int level = GetLevel();
	if(agility < 1 || level < 1)
		return(0);
	
	if (agility <=74){
		if (agility == 1)
			return -24;
		else if (agility <=3)
			return -23;
		else if (agility == 4)
			return -22;
		else if (agility <=6)
			return -21;
		else if (agility <=8)
			return -20;
		else if (agility == 9)
			return -19;
		else if (agility <=11)
			return -18;
		else if (agility == 12)
			return -17;
		else if (agility <=14)
			return -16;
		else if (agility <=16)
			return -15;
		else if (agility == 17)
			return -14;
		else if (agility <=19)
			return -13;
		else if (agility == 20)
			return -12;
		else if (agility <=22)
			return -11;
		else if (agility <=24)
			return -10;
		else if (agility == 25)
			return -9;
		else if (agility <=27)
			return -8;
		else if (agility == 28)
			return -7;
		else if (agility <=30)
			return -6;
		else if (agility <=32)
			return -5;
		else if (agility == 33)
			return -4;
		else if (agility <=35)
			return -3;
		else if (agility == 36)
			return -2;
		else if (agility <=38)
			return -1;
		else if (agility <=65)
			return 0;
		else if (agility <=70)
			return 1;
		else if (agility <=74)
			return 5;
	}
	else if(agility <= 137) {
		if (agility == 75){
			if (level <= 6)
				return 9;
			else if (level <= 19)
				return 23;
			else if (level <= 39)
				return 33;
			else
				return 39;
		}
		else if (agility >= 76 && agility <= 79){
			if (level <= 6)
				return 10;
			else if (level <= 19)
				return 23;
			else if (level <= 39)
				return 33;
			else
				return 40;
		}
		else if (agility == 80){
			if (level <= 6)
				return 11;
			else if (level <= 19)
				return 24;
			else if (level <= 39)
				return 34;
			else
				return 41;
		}
		else if (agility >= 81 && agility <= 85){
			if (level <= 6)
				return 12;
			else if (level <= 19)
				return 25;
			else if (level <= 39)
				return 35;
			else
				return 42;
		}
		else if (agility >= 86 && agility <= 90){
			if (level <= 6)
				return 12;
			else if (level <= 19)
				return 26;
			else if (level <= 39)
				return 36;
			else
				return 42;
		}
		else if (agility >= 91 && agility <= 95){
			if (level <= 6)
				return 13;
			else if (level <= 19)
				return 26;
			else if (level <= 39)
				return 36;
			else
				return 43;
		}
		else if (agility >= 96 && agility <= 99){
			if (level <= 6)
				return 14;
			else if (level <= 19)
				return 27;
			else if (level <= 39)
				return 37;
			else 
				return 44;
		}
		else if (agility == 100 && level >= 7){
			if (level <= 19)
				return 28;
			else if (level <= 39)
				return 38;
			else
				return 45;
		}
		else if (level <= 6) {
			return 15;
		}
		//level is >6
		else if (agility >= 101 && agility <= 105){
			if (level <= 19)
				return 29;
			else if (level <= 39)
				return 39;// not verified
			else
				return 45;
		}
		else if (agility >= 106 && agility <= 110){
			if (level <= 19)
				return 29;
			else if (level <= 39)
				return 39;// not verified
			else
				return 46;
		}
		else if (agility >= 111 && agility <= 115){
			if (level <= 19)
				return 30;
			else if (level <= 39)
				return 40;// not verified
			else
				return 47;
		}
		else if (agility >= 116 && agility <= 119){
			if (level <= 19)
				return 31;
			else if (level <= 39)
				return 41;
			else
				return 47;
		}
		else if (level <= 19) {
				return 32;
		}
		//level is > 19
		else if (agility == 120){
			if (level <= 39)
				return 42;
			else
				return 48;
		}
		else if (agility <= 125){
			if (level <= 39)
				return 42;
			else
				return 49;
		}
		else if (agility <= 135){
			if (level <= 39)
				return 42;
			else
				return 50;
		}
		else {
			if (level <= 39)
				return 42;
			else
				return 51;
		}
	} else if(agility <= 300) {
		if(level <= 6) {
			if(agility <= 139)
				return(21);
			else if(agility == 140)
				return(22);
			else if(agility <= 145)
				return(23);
			else if(agility <= 150)
				return(23);
			else if(agility <= 155)
				return(24);
			else if(agility <= 159)
				return(25);
			else if(agility == 160)
				return(26);
			else if(agility <= 165)
				return(26);
			else if(agility <= 170)
				return(27);
			else if(agility <= 175)
				return(28);
			else if(agility <= 179)
				return(28);
			else if(agility == 180)
				return(29);
			else if(agility <= 185)
				return(30);
			else if(agility <= 190)
				return(31);
			else if(agility <= 195)
				return(31);
			else if(agility <= 199)
				return(32);
			else if(agility <= 219)
				return(33);
			else if(agility <= 239)
				return(34);
			else
				return(35);
		} else if(level <= 19) {
			if(agility <= 139)
				return(34);
			else if(agility == 140)
				return(35);
			else if(agility <= 145)
				return(36);
			else if(agility <= 150)
				return(37);
			else if(agility <= 155)
				return(37);
			else if(agility <= 159)
				return(38);
			else if(agility == 160)
				return(39);
			else if(agility <= 165)
				return(40);
			else if(agility <= 170)
				return(40);
			else if(agility <= 175)
				return(41);
			else if(agility <= 179)
				return(42);
			else if(agility == 180)
				return(43);
			else if(agility <= 185)
				return(43);
			else if(agility <= 190)
				return(44);
			else if(agility <= 195)
				return(45);
			else if(agility <= 199)
				return(45);
			else if(agility <= 219)
				return(46);
			else if(agility <= 239)
				return(47);
			else
				return(48);
		} else if(level <= 39) {
			if(agility <= 139)
				return(44);
			else if(agility == 140)
				return(45);
			else if(agility <= 145)
				return(46);
			else if(agility <= 150)
				return(47);
			else if(agility <= 155)
				return(47);
			else if(agility <= 159)
				return(48);
			else if(agility == 160)
				return(49);
			else if(agility <= 165)
				return(50);
			else if(agility <= 170)
				return(50);
			else if(agility <= 175)
				return(51);
			else if(agility <= 179)
				return(52);
			else if(agility == 180)
				return(53);
			else if(agility <= 185)
				return(53);
			else if(agility <= 190)
				return(54);
			else if(agility <= 195)
				return(55);
			else if(agility <= 199)
				return(55);
			else if(agility <= 219)
				return(56);
			else if(agility <= 239)
				return(57);
			else
				return(58);
		} else {	//lvl >= 40
			if(agility <= 139)
				return(51);
			else if(agility == 140)
				return(52);
			else if(agility <= 145)
				return(53);
			else if(agility <= 150)
				return(53);
			else if(agility <= 155)
				return(54);
			else if(agility <= 159)
				return(55);
			else if(agility == 160)
				return(56);
			else if(agility <= 165)
				return(56);
			else if(agility <= 170)
				return(57);
			else if(agility <= 175)
				return(58);
			else if(agility <= 179)
				return(58);
			else if(agility == 180)
				return(59);
			else if(agility <= 185)
				return(60);
			else if(agility <= 190)
				return(61);
			else if(agility <= 195)
				return(61);
			else if(agility <= 199)
				return(62);
			else if(agility <= 219)
				return(63);
			else if(agility <= 239)
				return(64);
			else
				return(65);
		}
	}
	else{
		//seems about 21 agil per extra AC pt over 300...
	return (65 + ((agility-300) / 21));
	}
#if EQDEBUG >= 11
	LogFile->write(EQEMuLog::Error, "Error in Merc::acmod(): Agility: %i, Level: %i",agility,level);
#endif
	return 0;
};

sint32 Merc::CalcHPRegen() {
	sint32 regen = LevelRegen() + itembonuses.HPRegen + spellbonuses.HPRegen;

	//regen += aabonuses.HPRegen + GroupLeadershipAAHealthRegeneration();

	return (regen * RuleI(Character, HPRegenMultiplier) / 100);
}

sint32 Merc::CalcHPRegenCap()
{
	int cap = RuleI(Character, ItemHealthRegenCap) + itembonuses.HeroicSTA/25;

	cap += aabonuses.ItemHPRegenCap + spellbonuses.ItemHPRegenCap + itembonuses.ItemHPRegenCap;
	
	return (cap * RuleI(Character, HPRegenMultiplier) / 100);
}

sint32 Merc::CalcMaxHP() {
	float nd = 10000;
	max_hp = (CalcBaseHP() + itembonuses.HP);

	//The AA desc clearly says it only applies to base hp..
	//but the actual effect sent on live causes the client
	//to apply it to (basehp + itemhp).. I will oblige to the client's whims over
	//the aa description
	nd += aabonuses.MaxHP;	//Natural Durability, Physical Enhancement, Planar Durability

	max_hp = (float)max_hp * (float)nd / (float)10000; //this is to fix the HP-above-495k issue
	max_hp += spellbonuses.HP + aabonuses.HP;

	//max_hp += GroupLeadershipAAHealthEnhancement();
	
	max_hp += max_hp * (spellbonuses.MaxHPChange + itembonuses.MaxHPChange) / 10000;
	
	if (cur_hp > max_hp)
		cur_hp = max_hp;
	
	int hp_perc_cap = spellbonuses.HPPercCap;
	if(hp_perc_cap) {
		int curHP_cap = (max_hp * hp_perc_cap) / 100;
		if (cur_hp > curHP_cap)
			cur_hp = curHP_cap;
	}
	
	return max_hp;
}

// This is for calculating Base HPs + STA bonus for SoD or later clients.
sint32 Merc::GetClassHPFactor() {

	int factor;

	// Note: Base HP factor under level 41 is equal to factor / 12, and from level 41 to 80 is factor / 6.
	// Base HP over level 80 is factor / 10
	// HP per STA point per level is factor / 30 for level 80+
	// HP per STA under level 40 is the level 80 HP Per STA / 120, and for over 40 it is / 60.
	
	switch(GetClass())
	{
		case WIZARD:
			factor = 240;
			break;
		case ROGUE:
			factor = 255;
			break;
		case CLERIC:
			factor = 264;
			break;
		case WARRIOR:
			factor = 300;
			break;
		default:
			factor = 240;
			break;
	}
	return factor;
}

sint32 Merc::CalcBaseHP()
{
	if(GetClientVersion() >= EQClientSoD && RuleB(Character, SoDClientUseSoDHPManaEnd)) {
		float SoDPost255;
		int16 NormalSTA = GetSTA();

		if(((NormalSTA - 255) / 2) > 0)
			SoDPost255 = ((NormalSTA - 255) / 2);
		else
			SoDPost255 = 0;

		int hp_factor = GetClassHPFactor();
		
		if (level < 41) {
			base_hp = (5 + (GetLevel() * hp_factor / 12) + 
				((NormalSTA - SoDPost255) * GetLevel() * hp_factor / 3600));
		}
		else if (level < 81) {
			base_hp = (5 + (40 * hp_factor / 12) + ((GetLevel() - 40) * hp_factor / 6) + 
				((NormalSTA - SoDPost255) * hp_factor / 90) + 
				((NormalSTA - SoDPost255) * (GetLevel() - 40) * hp_factor / 1800));
		}
		else { 
			base_hp = (5 + (80 * hp_factor / 8) + ((GetLevel() - 80) * hp_factor / 10) + 
				((NormalSTA - SoDPost255) * hp_factor / 90) + 
				((NormalSTA - SoDPost255) * hp_factor / 45));
		}

		base_hp += (GetHeroicSTA() * 10);

	}
	else {
		int16 Post255;
		int16 lm=GetClassLevelFactor();
		if((GetSTA()-255)/2 > 0)
			Post255 = (GetSTA()-255)/2;
		else
			Post255 = 0;
			
		base_hp = (5)+(GetLevel()*lm/10) + (((GetSTA()-Post255)*GetLevel()*lm/3000)) + ((Post255*GetLevel())*lm/6000);
	}
	return base_hp;
}

sint32 Merc::CalcMaxMana()
{
	switch(GetCasterClass())
	{
		case 'I': 
		case 'W': {
			max_mana = (CalcBaseMana() + itembonuses.Mana + spellbonuses.Mana + GroupLeadershipAAManaEnhancement());
			break;
		}
		case 'N': {
			max_mana = 0;
			break;
		}
		default: {
			LogFile->write(EQEMuLog::Debug, "Invalid Class '%c' in CalcMaxMana", GetCasterClass());
			max_mana = 0;
			break;
		}
	}
	if (max_mana < 0) {
		max_mana = 0;
	}
	
	if (cur_mana > max_mana) {
		cur_mana = max_mana;
	}
	
	int mana_perc_cap = spellbonuses.ManaPercCap;
	if(mana_perc_cap) {
		int curMana_cap = (max_mana * mana_perc_cap) / 100;
		if (cur_mana > curMana_cap)
			cur_mana = curMana_cap;
	}
	
#if EQDEBUG >= 11
	LogFile->write(EQEMuLog::Debug, "Merc::CalcMaxMana() called for %s - returning %d", GetName(), max_mana);
#endif
	return max_mana;
}

sint32 Merc::CalcBaseMana()
{
	int WisInt = 0;
	int MindLesserFactor, MindFactor;
	sint32 max_m = 0;
	int wisint_mana = 0;
	int base_mana = 0;
	int ConvertedWisInt = 0;
	switch(GetCasterClass())
	{
		case 'I': 
			WisInt = GetINT();

			if (GetClientVersion() >= EQClientSoD && RuleB(Character, SoDClientUseSoDHPManaEnd)) {
				
				if (WisInt > 100) {
					ConvertedWisInt = (((WisInt - 100) * 5 / 2) + 100);
					if (WisInt > 201) {
						ConvertedWisInt -= ((WisInt - 201) * 5 / 4);
					}
				}
				else {
					ConvertedWisInt = WisInt;
				}

				if (GetLevel() < 41) { 
					wisint_mana = (GetLevel() * 75 * ConvertedWisInt / 1000);
					base_mana = (GetLevel() * 15);
				}
				else if (GetLevel() < 81) {
					wisint_mana = ((3 * ConvertedWisInt) + ((GetLevel() - 40) * 15 * ConvertedWisInt / 100));
					base_mana = (600 + ((GetLevel() - 40) * 30));
				}
				else {
					wisint_mana = (9 * ConvertedWisInt);
					base_mana = (1800 + ((GetLevel() - 80) * 18));
				}
				max_m = base_mana + wisint_mana + (GetHeroicINT() * 10);
			}
			else
			{
				if((( WisInt - 199 ) / 2) > 0)
					MindLesserFactor = ( WisInt - 199 ) / 2;
				else
					MindLesserFactor = 0;

				MindFactor = WisInt - MindLesserFactor;
				if(WisInt > 100)
					max_m = (((5 * (MindFactor + 20)) / 2) * 3 * GetLevel() / 40);
				else
					max_m = (((5 * (MindFactor + 200)) / 2) * 3 * GetLevel() / 100);
			}
			break;

		case 'W':
			WisInt = GetWIS();

			if (GetClientVersion() >= EQClientSoD && RuleB(Character, SoDClientUseSoDHPManaEnd)) {

				if (WisInt > 100) {
					ConvertedWisInt = (((WisInt - 100) * 5 / 2) + 100);
					if (WisInt > 201) {
						ConvertedWisInt -= ((WisInt - 201) * 5 / 4);
					}
				}
				else {
					ConvertedWisInt = WisInt;
				}

				if (GetLevel() < 41) { 
					wisint_mana = (GetLevel() * 75 * ConvertedWisInt / 1000);
					base_mana = (GetLevel() * 15);
				}
				else if (GetLevel() < 81) {
					wisint_mana = ((3 * ConvertedWisInt) + ((GetLevel() - 40) * 15 * ConvertedWisInt / 100));
					base_mana = (600 + ((GetLevel() - 40) * 30));
				}
				else {
					wisint_mana = (9 * ConvertedWisInt);
					base_mana = (1800 + ((GetLevel() - 80) * 18));
				}
				max_m = base_mana + wisint_mana + (GetHeroicWIS() * 10);
			}
			else
			{
				if((( WisInt - 199 ) / 2) > 0)
					MindLesserFactor = ( WisInt - 199 ) / 2;
				else
					MindLesserFactor = 0;

				MindFactor = WisInt - MindLesserFactor;
				if(WisInt > 100)
					max_m = (((5 * (MindFactor + 20)) / 2) * 3 * GetLevel() / 40);
				else
					max_m = (((5 * (MindFactor + 200)) / 2) * 3 * GetLevel() / 100);
			}
			break;
				
		case 'N': {
			max_m = 0;
			break;
		}
		default: {
			LogFile->write(EQEMuLog::Debug, "Invalid Class '%c' in CalcMaxMana", GetCasterClass());
			max_m = 0;
			break;
		}
	}

#if EQDEBUG >= 11
	LogFile->write(EQEMuLog::Debug, "Merc::CalcBaseMana() called for %s - returning %d", GetName(), max_m);
#endif
	return max_m;
}

sint32 Merc::CalcBaseManaRegen() 
{
	uint8 clevel = GetLevel();
	sint32 regen = 0;
	if (IsSitting()) 
	{
		if(HasSkill(MEDITATE))
			regen = (((GetSkill(MEDITATE) / 10) + (clevel - (clevel / 4))) / 4) + 4;
		else
			regen = 2;
	}
	else {
		regen = 2;
	}
	return regen;
}

sint32 Merc::CalcManaRegen() 
{
	uint8 clevel = GetLevel();
	sint32 regen = 0;
	//this should be changed so we dont med while camping, etc...
	if (IsSitting()) 
	{
		BuffFadeBySitModifier();
		if(HasSkill(MEDITATE)) {
			this->_medding = true;
			regen = (((GetSkill(MEDITATE) / 10) + (clevel - (clevel / 4))) / 4) + 4;
			regen += spellbonuses.ManaRegen + itembonuses.ManaRegen;
		}
		else
			regen = 2 + spellbonuses.ManaRegen + itembonuses.ManaRegen;
	}
	else {
		this->_medding = false;
		regen = 2 + spellbonuses.ManaRegen + itembonuses.ManaRegen;
	}
	
	//AAs
	regen += aabonuses.ManaRegen;

	return (regen * RuleI(Character, ManaRegenMultiplier) / 100);
}

sint32 Merc::CalcManaRegenCap()
{
	sint32 cap = RuleI(Character, ItemManaRegenCap) + aabonuses.ItemManaRegenCap;
	switch(GetCasterClass())
	{
		case 'I': 
			cap += (itembonuses.HeroicINT / 25);
			break;
		case 'W': 
			cap += (itembonuses.HeroicWIS / 25);
			break;
	}

	return (cap * RuleI(Character, ManaRegenMultiplier) / 100);
}

void Merc::CalcMaxEndurance()
{
	max_end = CalcBaseEndurance() + spellbonuses.Endurance + itembonuses.Endurance;
	
	if (max_end < 0) {
		max_end = 0;
	}
		
	if (cur_end > max_end) {
		cur_end = max_end;
	}
	
	int end_perc_cap = spellbonuses.EndPercCap;
	if(end_perc_cap) {
		int curEnd_cap = (max_end * end_perc_cap) / 100;
		if (cur_end > curEnd_cap)
			cur_end = curEnd_cap;
	}
}

sint32 Merc::CalcBaseEndurance()
{
	sint32 base_end = 0;
	sint32 base_endurance = 0;
	sint32 ConvertedStats = 0;
	sint32 sta_end = 0;
	int Stats = 0;

	if(GetClientVersion() >= EQClientSoD && RuleB(Character, SoDClientUseSoDHPManaEnd)) {
		int HeroicStats = 0;

		Stats = ((GetSTR() + GetSTA() + GetDEX() + GetAGI()) / 4);
		HeroicStats = ((GetHeroicSTR() + GetHeroicSTA() + GetHeroicDEX() + GetHeroicAGI()) / 4);

		if (Stats > 100) {
			ConvertedStats = (((Stats - 100) * 5 / 2) + 100);
			if (Stats > 201) {
				ConvertedStats -= ((Stats - 201) * 5 / 4);
			}
		}
		else {
			ConvertedStats = Stats;
		}

		if (GetLevel() < 41) { 
			sta_end = (GetLevel() * 75 * ConvertedStats / 1000);
			base_endurance = (GetLevel() * 15);
		}
		else if (GetLevel() < 81) {
			sta_end = ((3 * ConvertedStats) + ((GetLevel() - 40) * 15 * ConvertedStats / 100));
			base_endurance = (600 + ((GetLevel() - 40) * 30));
		}
		else {
			sta_end = (9 * ConvertedStats);
			base_endurance = (1800 + ((GetLevel() - 80) * 18));
		}
		base_end = (base_endurance + sta_end + (HeroicStats * 10));
	}
	else
	{
		Stats = GetSTR()+GetSTA()+GetDEX()+GetAGI();
		int LevelBase = GetLevel() * 15;

		int at_most_800 = Stats;
		if(at_most_800 > 800)
			at_most_800 = 800;
		
		int Bonus400to800 = 0;
		int HalfBonus400to800 = 0;
		int Bonus800plus = 0;
		int HalfBonus800plus = 0;
		
		int BonusUpto800 = int( at_most_800 / 4 ) ;
		if(Stats > 400) {
			Bonus400to800 = int( (at_most_800 - 400) / 4 );
			HalfBonus400to800 = int( max( ( at_most_800 - 400 ), 0 ) / 8 );
			
			if(Stats > 800) {
				Bonus800plus = int( (Stats - 800) / 8 ) * 2;
				HalfBonus800plus = int( (Stats - 800) / 16 );
			}
		}
		int bonus_sum = BonusUpto800 + Bonus400to800 + HalfBonus400to800 + Bonus800plus + HalfBonus800plus;
		
		base_end = LevelBase;

		//take all of the sums from above, then multiply by level*0.075
		base_end += ( bonus_sum * 3 * GetLevel() ) / 40;
	}
	return base_end;
}

sint32 Merc::CalcEnduranceRegen() {
	sint32 regen = sint32(GetLevel() * 4 / 10) + 2;
	regen += aabonuses.EnduranceRegen + spellbonuses.EnduranceRegen + itembonuses.EnduranceRegen;

	return (regen * RuleI(Character, EnduranceRegenMultiplier) / 100);
}

sint32 Merc::CalcEnduranceRegenCap() {
	int cap = (RuleI(Character, ItemEnduranceRegenCap) + itembonuses.HeroicSTR/25 + itembonuses.HeroicDEX/25 + itembonuses.HeroicAGI/25 + itembonuses.HeroicSTA/25);
		
	return (cap * RuleI(Character, EnduranceRegenMultiplier) / 100);
}

bool Merc::HasSkill(SkillType skill_id) const {
	return((GetSkill(skill_id) > 0) && CanHaveSkill(skill_id));
}

bool Merc::CanHaveSkill(SkillType skill_id) const {
	return(database.GetSkillCap(GetClass(), skill_id, RuleI(Character, MaxLevel)) > 0);
	//if you don't have it by max level, then odds are you never will?
}

int16 Merc::MaxSkill(SkillType skillid, int16 class_, int16 level) const {
	return(database.GetSkillCap(class_, skillid, level));
}

void Merc::FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho) {
	if(ns) {
		Mob::FillSpawnStruct(ns, ForWho);

		ns->spawn.afk = 0;
		ns->spawn.lfg = 0;
		ns->spawn.anon = 0;
		ns->spawn.gm = 0;
		ns->spawn.guildID = 0xFFFFFFFF;		// 0xFFFFFFFF = NO GUILD, 0 = Unknown Guild
		ns->spawn.is_npc = 1;				// 0=no, 1=yes
		ns->spawn.is_pet = 0;
		ns->spawn.guildrank = 0;
		ns->spawn.showhelm = 1;
		ns->spawn.flymode = 0;
		ns->spawn.size = 0;
		ns->spawn.NPC = 1;					// 0=player,1=npc,2=pc corpse,3=npc corpse
		ns->spawn.IsMercenary = 1;
		/*const Item_Struct* item = 0;
		const ItemInst* inst = 0;

		uint32 spawnedmercid = 0;
		spawnedmercid = this->GetID();

		inst = GetBotItem(SLOT_HANDS);
		if(inst) {
			item = inst->GetItem();
			if(item) {
				ns->spawn.equipment[MATERIAL_HANDS]	= item->Material;
				ns->spawn.colors[MATERIAL_HANDS].color = GetEquipmentColor(MATERIAL_HANDS);
			}
		}

		inst = GetBotItem(SLOT_HEAD);
		if(inst) {
			item = inst->GetItem();
			if(item) {
				ns->spawn.equipment[MATERIAL_HEAD] = item->Material;
				ns->spawn.colors[MATERIAL_HEAD].color = GetEquipmentColor(MATERIAL_HEAD);
			}
		}

		inst = GetBotItem(SLOT_ARMS);
		if(inst) {
			item = inst->GetItem();
			if(item) {
				ns->spawn.equipment[MATERIAL_ARMS] = item->Material;
				ns->spawn.colors[MATERIAL_ARMS].color = GetEquipmentColor(MATERIAL_ARMS);
			}
		}

		inst = GetBotItem(SLOT_BRACER01);
		if(inst) {
			item = inst->GetItem();
			if(item) {
				ns->spawn.equipment[MATERIAL_BRACER] = item->Material;
				ns->spawn.colors[MATERIAL_BRACER].color	= GetEquipmentColor(MATERIAL_BRACER);
			}
		}

		inst = GetBotItem(SLOT_BRACER02);
		if(inst) {
			item = inst->GetItem();
			if(item) {
				ns->spawn.equipment[MATERIAL_BRACER] = item->Material;
				ns->spawn.colors[MATERIAL_BRACER].color	= GetEquipmentColor(MATERIAL_BRACER);
			}
		}

		inst = GetBotItem(SLOT_CHEST);
		if(inst) {
			item = inst->GetItem();
			if(item) {
				ns->spawn.equipment[MATERIAL_CHEST]	= item->Material;
				ns->spawn.colors[MATERIAL_CHEST].color = GetEquipmentColor(MATERIAL_CHEST);
			}
		}

		inst = GetBotItem(SLOT_LEGS);
		if(inst) {
			item = inst->GetItem();
			if(item) {
				ns->spawn.equipment[MATERIAL_LEGS] = item->Material;
				ns->spawn.colors[MATERIAL_LEGS].color = GetEquipmentColor(MATERIAL_LEGS);
			}
		}

		inst = GetBotItem(SLOT_FEET);
		if(inst) {
			item = inst->GetItem();
			if(item) {
				ns->spawn.equipment[MATERIAL_FEET] = item->Material;
				ns->spawn.colors[MATERIAL_FEET].color = GetEquipmentColor(MATERIAL_FEET);
			}
		}
		
		inst = GetBotItem(SLOT_PRIMARY);
		if(inst) {
			item = inst->GetItem();
			if(item) {
				if(strlen(item->IDFile) > 2)
					ns->spawn.equipment[MATERIAL_PRIMARY] = atoi(&item->IDFile[2]);
					ns->spawn.colors[MATERIAL_PRIMARY].color = GetEquipmentColor(MATERIAL_PRIMARY);
			}
		}

		inst = GetBotItem(SLOT_SECONDARY);
		if(inst) {
			item = inst->GetItem();
			if(item) {
				if(strlen(item->IDFile) > 2)
					ns->spawn.equipment[MATERIAL_SECONDARY] = atoi(&item->IDFile[2]);
					ns->spawn.colors[MATERIAL_SECONDARY].color = GetEquipmentColor(MATERIAL_SECONDARY);
			}
		}*/
	}
}

bool Merc::Process()
{
	if(IsStunned() && stunned_timer.Check())
	{
		this->stunned = false;
		this->stunned_timer.Disable();
	}

	if (p_depop)
    {
		SetOwnerID(0);
		SetMercID(0);
		return false;
    }

	if(!GetMercOwner())
		return false;

	if(IsSuspended()) {
		//return false;
	}

	SpellProcess();

	if(tic_timer.Check())
	{
		//6 seconds, or whatever the rule is set to has passed, send this position to everyone to avoid ghosting
		if(!IsMoving() && !IsEngaged())
		{
			SendPosition();
		}

		BuffProcess();
	}

	if (IsStunned() || IsMezzed())
		return true;

	// Merc AI
	AI_Process();

	return true;
}

bool Merc::IsMercCasterCombatRange(Mob *target) {
	bool result = false;

	if(target) {
		float range = MercAISpellRange;

		range *= range;

		// half the max so the bot doesn't always stop at max range to allow combat movement
		range *= .5;

		float targetDistance = DistNoRootNoZ(*target);

		if(targetDistance > range)		
			result = false;
		else
			result = true;
	}

	return result;
}

void Merc::AI_Process() {
	if(!IsAIControlled())
		return;

	if(IsCasting())
		return;

	// A bot wont start its AI if not grouped
	if(!GetOwner() || !IsGrouped()) {
		return;
	}

	if(GetAppearance() == eaDead)
		return;

	Mob* MercOwner = GetOwner();

	// The bots need an owner
	if(!MercOwner)
		return;

	try {
		if(MercOwner->CastToClient()->IsDead()) {
			SetTarget(0);
			SetOwnerID(0);
			return;
		}
	}
	catch(...) {
		SetTarget(0);
		SetOwnerID(0);
		return;
	}

	if(!IsEngaged()) {
		if(GetFollowID()) {
			if(MercOwner && MercOwner->CastToClient()->AutoAttackEnabled() && MercOwner->GetTarget() &&
				MercOwner->GetTarget()->IsNPC() && MercOwner->GetTarget()->GetHateAmount(MercOwner)) {
					AddToHateList(MercOwner->GetTarget(), 1);
			}
			else {
				Group* g = GetGroup();

				if(g) {
					for(int counter = 0; counter < g->GroupCount(); counter++) {
						if(g->members[counter]) {
							if(g->members[counter]->IsEngaged() && g->members[counter]->GetTarget()) {
								AddToHateList(g->members[counter]->GetTarget(), 1);
								break;
							}
						}
					}
				}
			}
		}
	}

	if(IsEngaged())
	{
		_ZP(Mob_BOT_Process_IsEngaged);

		if(IsRooted())
			SetTarget(hate_list.GetClosest(this));
		else
			SetTarget(hate_list.GetTop(this));

		if(!GetTarget())
			return;

		if(HasPet())
			GetPet()->SetTarget(GetTarget());

		if(!IsSitting())
			FaceTarget(GetTarget());

		if(DivineAura())
			return;

		// Let's check if we have a los with our target.
		// If we don't, our hate_list is wiped.
		// Else, it was causing the bot to aggro behind wall etc... causing massive trains.
		if(!CheckLosFN(GetTarget()) || GetTarget()->IsMezzed() || !IsAttackAllowed(GetTarget())) {
			WipeHateList();

			if(IsMoving()) {
				SetHeading(0);
				SetRunAnimSpeed(0);

				if(moved) {
					moved = false;
					SendPosition();
					SetMoving(false);
				}
			}

			return;
		}

		bool atCombatRange = false;
		
		float meleeDistance = GetMaxMeleeRangeToTarget(GetTarget());

		if(GetClass() == SHADOWKNIGHT || GetClass() == PALADIN || GetClass() == WARRIOR) {
			meleeDistance = meleeDistance * .30;
		}
		else {
			meleeDistance *= (float)MakeRandomFloat(.50, .85);
		}
		if(IsMercCaster() && GetLevel() > 12) {
			if(IsMercCasterCombatRange(GetTarget()))
				atCombatRange = true;
		}
		else if(DistNoRoot(*GetTarget()) <= meleeDistance) {
			atCombatRange = true;
		}

		if(atCombatRange) {
			if(IsMoving()) {
				SetHeading(CalculateHeadingToTarget(GetTarget()->GetX(), GetTarget()->GetY()));
				SetRunAnimSpeed(0);
				
				if(moved) {
					moved = false;
					SendPosition();
					SetMoving(false);
				}
			}

			if(AImovement_timer->Check()) {
				if(!IsMoving() && GetClass() == ROGUE && !BehindMob(GetTarget(), GetX(), GetY())) {
					// Move the rogue to behind the mob
					float newX = 0;
					float newY = 0;
					float newZ = 0;

					if(PlotPositionAroundTarget(GetTarget(), newX, newY, newZ)) {
						CalculateNewPosition2(newX, newY, newZ, GetRunspeed());
						return;
					}
				}
				else if(!IsMoving() && GetClass() != ROGUE && (DistNoRootNoZ(*GetTarget()) < GetTarget()->GetSize())) {
					// If we are not a rogue trying to backstab, let's try to adjust our melee range so we don't appear to be bunched up
					float newX = 0;
					float newY = 0;
					float newZ = 0;

					if(PlotPositionAroundTarget(GetTarget(), newX, newY, newZ, false) && GetArchetype() != ARCHETYPE_CASTER) {
						CalculateNewPosition2(newX, newY, newZ, GetRunspeed());
						return;
					}
				}

				if(IsMoving())
					SendPosUpdate();
				else
					SendPosition();
			}

			if((!(IsMercCaster() && GetLevel() > 12)) && GetTarget() && !IsStunned() && !IsMezzed() && (GetAppearance() != eaDead)) {
				// we can't fight if we don't have a target, are stun/mezzed or dead..
				// Stop attacking if the target is enraged
				if(IsEngaged() && !BehindMob(GetTarget(), GetX(), GetY()) && GetTarget()->IsEnraged())
					return;
				//TODO: Implement Stances.
				/*if(GetBotStance() == BotStancePassive)
					return;*/

				// First, special attack per class (kick, backstab etc..)
				DoClassAttacks(GetTarget());

				//try main hand first
				if(attack_timer.Check()) {
					Attack(GetTarget(), SLOT_PRIMARY);

					bool tripleSuccess = false;

					if(GetOwner() && GetTarget() && CanThisClassDoubleAttack()) {

						if(GetOwner()) {
							Attack(GetTarget(), SLOT_PRIMARY, true);
						}

						if(GetOwner() && GetTarget() && SpecAttacks[SPECATK_TRIPLE]) {
							tripleSuccess = true;
							Attack(GetTarget(), SLOT_PRIMARY, true);
						}

						//quad attack, does this belong here??
						if(GetOwner() && GetTarget() && SpecAttacks[SPECATK_QUAD]) {
							Attack(GetTarget(), SLOT_PRIMARY, true);
						}
					}

					//Live AA - Flurry, Rapid Strikes ect (Flurry does not require Triple Attack).
					sint16 flurrychance = aabonuses.FlurryChance + spellbonuses.FlurryChance + itembonuses.FlurryChance;

					if (GetTarget() && flurrychance)
					{
						if(MakeRandomInt(0, 100) < flurrychance) 
						{
							Message_StringID(MT_NPCFlurry, 128);
							Attack(GetTarget(), SLOT_PRIMARY, false);
							Attack(GetTarget(), SLOT_PRIMARY, false);
						}
					}

					sint16 ExtraAttackChanceBonus = spellbonuses.ExtraAttackChance + itembonuses.ExtraAttackChance + aabonuses.ExtraAttackChance;

					if (GetTarget() && ExtraAttackChanceBonus) {
								if(MakeRandomInt(0, 100) < ExtraAttackChanceBonus)
								{
									Attack(GetTarget(), SLOT_PRIMARY, false);
								}
							}
				}

				// TODO: Do mercs berserk? Find this out on live...
				//if (GetClass() == WARRIOR || GetClass() == BERSERKER) {
				//	if(GetHP() > 0 && !berserk && this->GetHPRatio() < 30) {
				//		entity_list.MessageClose_StringID(this, false, 200, 0, BERSERK_START, GetName());
				//		this->berserk = true;
				//	}
				//	if (berserk && this->GetHPRatio() > 30) {
				//		entity_list.MessageClose_StringID(this, false, 200, 0, BERSERK_END, GetName());
				//		this->berserk = false;
				//	}
				//}

				//now off hand
				if(GetTarget() && attack_dw_timer.Check() && CanThisClassDualWield()) {

						int weapontype = NULL;
						bool bIsFist = true;

						if(bIsFist || ((weapontype != ItemType2HS) && (weapontype != ItemType2HPierce) && (weapontype != ItemType2HB))) {
							float DualWieldProbability = 0.0f;
				
							sint16 Ambidexterity = aabonuses.Ambidexterity + spellbonuses.Ambidexterity + itembonuses.Ambidexterity;
							DualWieldProbability = (GetSkill(DUAL_WIELD) + GetLevel() + Ambidexterity) / 400.0f; // 78.0 max
							sint16 DWBonus = spellbonuses.DualWieldChance + itembonuses.DualWieldChance;
							DualWieldProbability += DualWieldProbability*float(DWBonus)/ 100.0f;

							float random = MakeRandomFloat(0, 1);

							if (random < DualWieldProbability){ // Max 78% of DW

								Attack(GetTarget(), SLOT_SECONDARY);	// Single attack with offhand
					
								if( CanThisClassDoubleAttack()) {
									if(GetTarget() && GetTarget()->GetHP() > -10)
										Attack(GetTarget(), SLOT_SECONDARY);	// Single attack with offhand
								}
							}
						}
					}
				}
			}// end in combat range
			else {
				if(GetTarget()->IsFeared() && !spellend_timer.Enabled()){
					// This is a mob that is fleeing either because it has been feared or is low on hitpoints
					//TODO: Implement Stances. 
					//if(GetBotStance() != BotStancePassive)
						AI_PursueCastCheck();
				}

				if (AImovement_timer->Check()) {
					if(!IsRooted()) {
						mlog(AI__WAYPOINTS, "Pursuing %s while engaged.", GetTarget()->GetCleanName());
						CalculateNewPosition2(GetTarget()->GetX(), GetTarget()->GetY(), GetTarget()->GetZ(), GetRunspeed());
						return;
					}

					if(IsMoving())
						SendPosUpdate();
					else
						SendPosition();
				}
			} // end not in combat range

			if(!IsMoving() && !spellend_timer.Enabled()) {
			
				//TODO: Implement Stances.
				//if(GetBotStance() == BotStancePassive)
				//	return;

				if(AI_EngagedCastCheck()) {
					MercMeditate(false);
				}
				else if(GetArchetype() == ARCHETYPE_CASTER)
					MercMeditate(true);
			}
		} // end IsEngaged()
	else {
		// Not engaged in combat
		SetTarget(0);

		if(!IsMoving() && AIthink_timer->Check() && !spellend_timer.Enabled()) {
			
			//TODO: Implement passive stances.
			//if(GetBotStance() != BotStancePassive) {
			if(!AI_IdleCastCheck() && !IsCasting())
				MercMeditate(true);
		}

		if(AImovement_timer->Check()) {
			if(GetFollowID()) {
				Mob* follow = entity_list.GetMob(GetFollowID());

				if(follow) {
					float dist = DistNoRoot(*follow);
					float speed = GetRunspeed();

					if(dist < GetFollowDistance() + 1000) 
						speed = GetWalkspeed();

					SetRunAnimSpeed(0);

					if(dist > GetFollowDistance()) {
						CalculateNewPosition2(follow->GetX(), follow->GetY(), follow->GetZ(), speed);
						return;
					} 
					else
					{						
						if(moved)
						{
							moved=false;
							SendPosition();
							SetMoving(false);
						}
					}
				}
			}
		}
	}
}

void Merc::AI_Stop() {
	NPC::AI_Stop();
	Mob::AI_Stop();
}


void Merc::MercMeditate(bool isSitting) {
	if(isSitting) {
		// If the bot is a caster has less than 99% mana while its not engaged, he needs to sit to meditate
		if(GetManaRatio() < 99.0f)
		{
			if(!IsSitting())
				Sit();
		}
		else
		{
			if(IsSitting())
				Stand();
		}
	}
	else
	{
		if(IsSitting())
			Stand();
	}
}


void Merc::Sit() {
	if(IsMoving()) {
		moved = false;
		// SetHeading(CalculateHeadingToTarget(GetTarget()->GetX(), GetTarget()->GetY()));
		SendPosition();
		SetMoving(false);
		tar_ndx = 0;
	}

	SetAppearance(eaSitting);
}

void Merc::Stand() {
	SetAppearance(eaStanding);
}

bool Merc::IsSitting() {
	bool result = false;

	if(GetAppearance() == eaSitting && !IsMoving())
		result = true;

	return result;
}

bool Merc::IsStanding() {
	bool result = false;

	if(GetAppearance() == eaStanding)
		result = true;

	return result;
}

float Merc::GetMaxMeleeRangeToTarget(Mob* target) {
	float result = 0;
	
	if(target) {
		float size_mod = GetSize();
		float other_size_mod = target->GetSize();

		if(GetRace() == 49 || GetRace() == 158 || GetRace() == 196) //For races with a fixed size
			size_mod = 60.0f;
		else if (size_mod < 6.0)
			size_mod = 8.0f;

		if(target->GetRace() == 49 || target->GetRace() == 158 || target->GetRace() == 196) //For races with a fixed size
			other_size_mod = 60.0f;
		else if (other_size_mod < 6.0)
			other_size_mod = 8.0f;

		if (other_size_mod > size_mod) {
			size_mod = other_size_mod;
		}

		// this could still use some work, but for now it's an improvement....

		if (size_mod > 29)
			size_mod *= size_mod;
		else if (size_mod > 19)
			size_mod *= size_mod * 2;
		else
			size_mod *= size_mod * 4;

		// prevention of ridiculously sized hit boxes
		if (size_mod > 10000)
			size_mod = size_mod / 7;

		result = size_mod;
	}

	return result;
}

bool Merc::Attack(Mob* other, int Hand, bool bRiposte, bool IsStrikethrough, bool IsFromSpell)
{

	_ZP(Client_Attack);

	if (!other) {
		SetTarget(NULL);
		LogFile->write(EQEMuLog::Error, "A null Mob object was passed to Merc::Attack() for evaluation!");
		return false;
	}

	return NPC::Attack(other, Hand, bRiposte, IsStrikethrough, IsFromSpell);
}

void Merc::Damage(Mob* other, sint32 damage, int16 spell_id, SkillType attack_skill, bool avoidable, sint8 buffslot, bool iBuffTic)
{
	if(IsDead() || IsCorpse())
		return;
	
	if(spell_id==0)
		spell_id = SPELL_UNKNOWN;

	NPC::Damage(other, damage, spell_id, attack_skill, avoidable, buffslot, iBuffTic);

	//Not needed since we're using NPC damage.
	//CommonDamage(other, damage, spell_id, attack_skill, avoidable, buffslot, iBuffTic);
}


Mob* Merc::GetOwnerOrSelf() {
	Mob* Result = 0;

	if(this->GetMercOwner())
		Result = GetMercOwner();
	else
		Result = this;

	return Result;
}

void Merc::Death(Mob* killerMob, sint32 damage, int16 spell, SkillType attack_skill)
{
	NPC::Death(killerMob, damage, spell, attack_skill);
	Save();

	Mob *give_exp = hate_list.GetDamageTop(this);
	Client *give_exp_client = NULL;

	if(give_exp && give_exp->IsClient())
		give_exp_client = give_exp->CastToClient();

	bool IsLdonTreasure = (this->GetClass() == LDON_TREASURE);

	//if (give_exp_client && !IsCorpse() && MerchantType == 0)
	//{
	//	Group *kg = entity_list.GetGroupByClient(give_exp_client);
	//	Raid *kr = entity_list.GetRaidByClient(give_exp_client);

	//	if(!kr && give_exp_client->IsClient() && give_exp_client->GetBotRaidID() > 0) {
	//		BotRaids *br = entity_list.GetBotRaidByMob(give_exp_client->CastToMob());
	//		if(br) {
	//			if(!IsLdonTreasure)
	//				br->SplitExp((EXP_FORMULA), this);

	//			if(br->GetBotMainTarget() == this)
	//				br->SetBotMainTarget(NULL);

	//			/* Send the EVENT_KILLED_MERIT event for all raid members */
	//			if(br->BotRaidGroups[0]) {
	//				for(int j=0; j<MAX_GROUP_MEMBERS; j++) {
	//					if(br->BotRaidGroups[0]->members[j] && br->BotRaidGroups[0]->members[j]->IsClient()) {
	//						parse->Event(EVENT_KILLED_MERIT, GetNPCTypeID(), "killed", this, br->BotRaidGroups[0]->members[j]);
	//						if(RuleB(TaskSystem, EnableTaskSystem)) {
	//							br->BotRaidGroups[0]->members[j]->CastToClient()->UpdateTasksOnKill(GetNPCTypeID());
	//						}
	//					}
	//				}
	//			}
	//		}
	//	}
	//}

	//corpse->Depop();

	//no corpse, no exp if we're a merc. We'll suspend instead, since that's what live does. I'm not actually sure live supports 'depopping' merc corpses.
	//if(entity_list.GetCorpseByID(GetID()))
	//	entity_list.GetCorpseByID(GetID())->Depop();

	if(Suspend())
	{
		//If the merc owner is valid, and the merc is 'pseudo-suspended', do this:
		if(GetMercOwner() != NULL)
		{
							Client* owner = GetMercOwner();
							owner->GetEPP().mercIsSuspended = true;
							owner->GetEPP().mercSuspendedTime = time(NULL) + RuleI(Mercs, SuspendIntervalS);
							owner->GetEPP().mercTimerRemaining = owner->GetMercTimer().GetRemainingTime();
							owner->GetMercTimer().Disable();
							owner->UpdateMercTimer();
							owner->GetPTimers().Start(pTimerMercSuspend, RuleI(Mercs, SuspendIntervalS));
							owner->SendMercSuspendResponsePacket(owner->GetEPP().mercSuspendedTime);
							owner->SendMercMerchantResponsePacket(0);
		}
	}
}

Client* Merc::GetMercOwner() {
	Client* mercOwner = 0;

	if(GetOwner())
	{
		if(GetOwner()->IsClient())
		{
		mercOwner = GetOwner()->CastToClient();
		}
	}

	return mercOwner; 
}

Mob* Merc::GetOwner() {
	Mob* Result = 0;

	Result = entity_list.GetMob(GetOwnerID());

	if(!Result) {
		this->SetOwnerID(0);
	}

	return Result->CastToMob();
}

const char* Merc::GetRandomName(){
	// creates up to a 10 char name
	static char name[17];
	char vowels[18]="aeiouyaeiouaeioe";
	char cons[48]="bcdfghjklmnpqrstvwxzybcdgklmnprstvwbcdgkpstrkd";
	char rndname[17]="\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
	char paircons[33]="ngrkndstshthphsktrdrbrgrfrclcr";
	bool valid = false;

	while(!valid) {
		int rndnum=MakeRandomInt(0, 75),n=1;
		bool dlc=false;
		bool vwl=false;
		bool dbl=false;
		if (rndnum>63)
		{	// rndnum is 0 - 75 where 64-75 is cons pair, 17-63 is cons, 0-16 is vowel
			rndnum=(rndnum-61)*2;	// name can't start with "ng" "nd" or "rk"
			rndname[0]=paircons[rndnum];
			rndname[1]=paircons[rndnum+1];
			n=2;
		}
		else if (rndnum>16)
		{
			rndnum-=17;
			rndname[0]=cons[rndnum];
		}
		else
		{
			rndname[0]=vowels[rndnum];
			vwl=true;
		}
		int namlen=MakeRandomInt(5, 10);
		for (int i=n;i<namlen;i++)
		{
			dlc=false;
			if (vwl)	//last char was a vowel
			{			// so pick a cons or cons pair
				rndnum=MakeRandomInt(0, 62);
				if (rndnum>46)
				{	// pick a cons pair
					if (i>namlen-3)	// last 2 chars in name?
					{	// name can only end in cons pair "rk" "st" "sh" "th" "ph" "sk" "nd" or "ng"
						rndnum=MakeRandomInt(0, 7)*2;
					}
					else
					{	// pick any from the set
						rndnum=(rndnum-47)*2;
					}
					rndname[i]=paircons[rndnum];
					rndname[i+1]=paircons[rndnum+1];
					dlc=true;	// flag keeps second letter from being doubled below
					i+=1;
				}
				else
				{	// select a single cons
					rndname[i]=cons[rndnum];
				}
			}
			else
			{		// select a vowel
				rndname[i]=vowels[MakeRandomInt(0, 16)];
			}
			vwl=!vwl;
			if (!dbl && !dlc)
			{	// one chance at double letters in name
				if (!MakeRandomInt(0, i+9))	// chances decrease towards end of name
				{
					rndname[i+1]=rndname[i];
					dbl=true;
					i+=1;
				}
			}
		}

		rndname[0]=toupper(rndname[0]);

		if(!database.CheckNameFilter(rndname)) {
			valid = false;
		}
		else if(rndname[0] < 'A' && rndname[0] > 'Z') {
			//name must begin with an upper-case letter.
			valid = false;
		}
		else if (database.CheckUsedName(rndname)) {
			valid = true;
		}
		else {
			valid = false;
		}
	}

	memset(name, 0, 17);
	strcpy(name, rndname);
	return name;
}

Merc* Merc::LoadMerc(Client *c, MercTemplate* merc_template, uint32 merchant_id) {
	Merc* merc;

	if(c)
	{
		if(c->GetMercID()) {
			merc_template = zone->GetMercTemplate(c->GetEPP().mercTemplateID);
		}
	}

	//get mercenary data
	if(merc_template) {
		NPCType* npc_type = new NPCType;
		memset(npc_type, 0, sizeof(NPCType));

		sprintf(npc_type->name, "%s", GetRandomName());

		int8 gender;
		if(merchant_id > 0) {
			NPC* tar = entity_list.GetNPCByID(merchant_id);
			if(tar) {
				gender = Mob::GetDefaultGender(merc_template->RaceID, tar->GetGender());
			}
		}
		else {
			gender = c->GetEPP().mercGender;
		}

		sprintf(npc_type->lastname, "%s's %s", c->GetName(), "Mercenary");
		npc_type->cur_hp = 4000000;
		npc_type->max_hp = 4000000;
		npc_type->race = 1;
		npc_type->gender = gender;
		npc_type->class_ = merc_template->ClassID;
		npc_type->deity= 1;
		npc_type->level = c->GetLevel();
		npc_type->npc_id = 0;
		npc_type->loottable_id = 0;
		npc_type->texture = 1;
		npc_type->light = 0;
		npc_type->runspeed = 0;
		npc_type->d_meele_texture1 = 1;
		npc_type->d_meele_texture2 = 1;
		npc_type->merchanttype = 1;
		npc_type->bodytype = 1;

		npc_type->maxlevel = c->GetLevel();
		npc_type->size = 6.0;
		npc_type->npc_id = 0;
		npc_type->cur_hp = 0;
		npc_type->drakkin_details = 0;
		npc_type->drakkin_heritage = 0;
		npc_type->drakkin_tattoo = 0;
		npc_type->runspeed = 1.88;
		npc_type->findable = 0;
		npc_type->hp_regen = 1;
		npc_type->mana_regen = 1;
		npc_type->qglobal = false;
		npc_type->npc_spells_id = 0;
		npc_type->attack_speed = 0;
		npc_type->STR = 75;
		npc_type->STA = 75;
		npc_type->DEX = 75;
		npc_type->AGI = 75;
		npc_type->WIS = 75;
		npc_type->INT = 75;
		npc_type->CHA = 75;
		npc_type->ATK = 75;
		npc_type->MR = 25;
		npc_type->FR = 25;
		npc_type->DR = 15;
		npc_type->PR = 15;
		npc_type->CR = 25;
		npc_type->Corrup = 15;
		npc_type->AC = 12;

		Merc* merc = new Merc(npc_type, c->GetX(), c->GetY(), c->GetZ(), 0);
	
		merc->SetMercData( merc_template->MercTemplateID );

		return merc;
	}

	return 0;
}

bool Merc::Spawn(Client *owner) {
	if(!owner)
		return false;

	MercTemplate* merc_template = zone->GetMercTemplate(GetMercTemplateID());

	if(!merc_template)
		return false;

	entity_list.AddMerc(this, true, true);

	SendPosition();

	//printf("Spawned Merc with ID %i\n", npc->GetID()); fflush(stdout);

	/*
	uint32 itemID = 0;
	int8 materialFromSlot = 0xFF;
	for(int i=0; i<22; ++i) {
		itemID = GetMercItemBySlot(i);
		if(itemID != 0) {
			materialFromSlot = Inventory::CalcMaterialFromSlot(i);
			if(materialFromSlot != 0xFF) {
				this->SendWearChange(materialFromSlot);
			}
		}
	}
	*/

		
	SendIllusionPacket(merc_template->RaceID, GetGender(), GetTexture(), GetHelmTexture(), GetHairColor(), GetBeardColor(),
		GetEyeColor1(), GetEyeColor2(), GetHairStyle(), GetLuclinFace(), GetBeard(), 0xFF,
		GetDrakkinHeritage(), GetDrakkinTattoo(), GetDrakkinDetails(), GetSize());

	return true;
}

void Client::UpdateMercTimer()
{
	Merc *merc =  GetMerc();

	if(merc && !merc->IsSuspended()) {		
			if(merc_timer.Check())
			{
			int32 upkeep = Merc::CalcUpkeepCost(merc->GetMercTemplateID(), GetLevel());
			//TakeMoneyFromPP(upkeep, true);
			// Send Mercenary Status/Timer packet
			SendMercTimerPacket(GetID(), 5, GetEPP().mercSuspendedTime, RuleI(Mercs, UpkeepIntervalMS), RuleI(Mercs, SuspendIntervalMS));
			SendMercMerchantResponsePacket(10);
			GetEPP().mercTimerRemaining = RuleI(Mercs, UpkeepIntervalMS);
			merc_timer.Start(GetEPP().mercTimerRemaining);
			}
	}
}

void Client::CheckMercSuspendTimer()
{
	if(GetEPP().mercSuspendedTime != 0) {
			if(time(NULL) >= GetEPP().mercSuspendedTime){
			GetEPP().mercSuspendedTime = 0;
			SendMercSuspendResponsePacket(GetEPP().mercSuspendedTime);
			p_timers.Start(pTimerMercSuspend, RuleI(Mercs, SuspendIntervalS));
		}
	}
}

void Client::SuspendMercCommand()
{
	bool ExistsMerc = GetEPP().mercTemplateID != 0;
		if(ExistsMerc == true)
		{
			if(GetEPP().mercIsSuspended) {
					GetEPP().mercIsSuspended = false;
					//p_timers.Enable(pTimerMercReuse);
					GetEPP().mercSuspendedTime = 0;
			
					// Get merc, assign it to client & spawn
					Merc* merc = Merc::LoadMerc(this, &zone->merc_templates[GetEPP().mercTemplateID], 0);
					merc->Spawn(this);
					merc->SetSuspended(false);
					SetMerc(merc);
					if(merc->Unsuspend())
					{
						merc_timer.Start(GetEPP().mercTimerRemaining);
						if(!p_timers.Expired(&database, pTimerMercSuspend, false)) 
							p_timers.Clear(&database, pTimerMercSuspend);
							SendMercPersonalInfo();
							SendMercMerchantResponsePacket(0);

						if(!IsGrouped()) {
								Group *g = new Group(this);
								if(merc->AddMercToGroup(merc, g)) {
								entity_list.AddGroup(g);
								database.SetGroupLeaderName(g->GetID(), this->GetName());
								g->SaveGroupLeaderAA();
								database.SetGroupID(this->GetName(), g->GetID(), this->CharacterID());
								database.SetGroupID(merc->GetCleanName(), g->GetID(), merc->GetMercID());
								}
							}
						else {
							merc->AddMercToGroup(merc, this->GetGroup());
							database.SetGroupID(GetCleanName(), this->GetGroup()->GetID(), merc->GetMercID());
						}
					}
			}
		
			else {
				Merc* CurrentMerc = GetMerc();

				if(CurrentMerc && GetMercID()) {
					if(CurrentMerc->Suspend()) {
						// Set merc suspended time for client & merc
						GetEPP().mercIsSuspended = true;
						GetEPP().mercSuspendedTime = time(NULL) + RuleI(Mercs, SuspendIntervalS);
						GetEPP().mercTimerRemaining = merc_timer.GetRemainingTime();
						merc_timer.Disable();
						UpdateMercTimer();
						p_timers.Start(pTimerMercSuspend, RuleI(Mercs, SuspendIntervalS));
						SendMercSuspendResponsePacket(GetEPP().mercSuspendedTime);
						SendMercMerchantResponsePacket(0);
					}
				}
			}
		}
}


// Handles all client zone change event
void Merc::ProcessClientZoneChange(Client* mercOwner) {
	if(mercOwner) 
	{
		Zone();
	}
}

void Client::SpawnMercOnZone()
{
	bool ExistsMerc = GetEPP().mercTemplateID != 0;
		if(ExistsMerc == true)
		{
				if(!GetEPP().mercIsSuspended) {
					GetEPP().mercSuspendedTime = 0;			
					// Get merc, assign it to client & spawn
					Merc* merc = Merc::LoadMerc(this, &zone->merc_templates[GetEPP().mercTemplateID], 0);
					merc->Spawn(this);
					merc->SetSuspended(false);
					SetMerc(merc);
					merc_timer.Start(GetEPP().mercTimerRemaining);

					// Send Mercenary Status/Timer packet
					SendMercTimerPacket(GetID(), 5, GetEPP().mercSuspendedTime, GetEPP().mercTimerRemaining, RuleI(Mercs, SuspendIntervalMS));

					// Send Mercenary Assign packet twice - This is actually just WeaponEquip
					SendMercAssignPacket(GetID(), 1, 2);
					SendMercAssignPacket(GetID(), 0, 13);
					SendMercPersonalInfo();
					SendMercMerchantResponsePacket(0);
					if(!IsGrouped()) {
					Group *g = new Group(this);
					if(merc->AddMercToGroup(merc, g)) {
					entity_list.AddGroup(g);
					database.SetGroupLeaderName(g->GetID(), this->GetName());
					g->SaveGroupLeaderAA();
					database.SetGroupID(this->GetName(), g->GetID(), this->CharacterID());
					database.SetGroupID(merc->GetCleanName(), g->GetID(), merc->GetMercID());
					}
				}
				else {
					merc->AddMercToGroup(merc, this->GetGroup());
					database.SetGroupID(GetCleanName(), this->GetGroup()->GetID(), merc->GetMercID());
				}
			}
			else
			{
					// Send Mercenary Status/Timer packet
				SendMercTimerPacket(GetID(), 1, GetEPP().mercSuspendedTime, GetEPP().mercTimerRemaining, p_timers.GetRemainingTime(pTimerMercSuspend) * 1000);

				// Send Mercenary Assign packet twice - This is actually just WeaponEquip
				SendMercAssignPacket(GetID(), 1, 2);
				SendMercAssignPacket(GetID(), 0, 13);

				SendMercPersonalInfo();
				if(GetEPP().mercSuspendedTime != 0) {
					if(time(NULL) >= GetEPP().mercSuspendedTime){
					GetEPP().mercSuspendedTime = 0;
					}
				}
				SendMercSuspendResponsePacket(GetEPP().mercSuspendedTime);
				SendMercMerchantResponsePacket(0);
			}
		}
}

bool Merc::Suspend() {
	Client* mercOwner;

	if(GetMercOwner()) {
		mercOwner = GetMercOwner();
	}

	if(!mercOwner)
		return false;

	SetSuspended(true);

	if(IsGrouped()) {
		RemoveMercFromGroup(this, GetGroup());
	}

	//Save();
	
	Depop();

	return true;
}

bool Merc::Unsuspend() {
	Client* mercOwner;

	if(GetMercOwner()) {
		mercOwner = GetMercOwner();
	}

	if(!mercOwner)
		return false;

	if(GetMercID()) {
		int32 entityID = 0;
		int32 mercState = 5;
		int32 suspendedTime = 0;

		if(mercOwner->GetEPP().mercIsSuspended) {
			mercState = 1;
			suspendedTime = mercOwner->GetEPP().mercSuspendedTime;
		}

		// TODO: Populate these packets properly instead of hard coding the data fields.

		// Send Mercenary Status/Timer packet
		mercOwner->SendMercTimerPacket(GetID(), mercState, suspendedTime, mercOwner->GetEPP().mercTimerRemaining, RuleI(Mercs, SuspendIntervalMS));

		// Send Mercenary Assign packet twice - This is actually just WeaponEquip
		mercOwner->SendMercAssignPacket(GetID(), 1, 2);
		mercOwner->SendMercAssignPacket(GetID(), 0, 13);

	}

	return true;
}

bool Merc::Dismiss() {
	Client* mercOwner;

	if(GetMercOwner()) {
		mercOwner = GetMercOwner();
	}

	if(!mercOwner)
		return false;

	if(IsGrouped()) {
		RemoveMercFromGroup(this, GetGroup());
	}
	SetMercID(0);

	mercOwner->SetMerc(0);

	Depop();

	return true;
}

void Merc::Zone() {
	if(HasGroup()) {
		GetGroup()->MemberZoned(this);
	}

	if(IsGrouped()) { //Names and IDs will change as a result of mercs being in a group.
		RemoveMercFromGroup(this, GetGroup());
	}

	SetMercID(0);

	//Save();
	Depop();
}

void Merc::Depop() {
	WipeHateList();
	
	entity_list.RemoveFromHateLists(this);
	
	if(HasGroup())
		RemoveMercFromGroup(this, GetGroup());
	
	if(HasPet()) {
		GetPet()->Depop();
	}
	
	SetOwnerID(0);

	p_depop = true;
	
	NPC::Depop(false);
}

bool Merc::RemoveMercFromGroup(Merc* merc, Group* group) {
	bool Result = false;

	if(merc && group) {
		if(merc->HasGroup()) {
			if(!group->IsLeader(merc)) {
				merc->SetFollowID(0);

				if(group->DelMember(merc))
					database.SetGroupID(merc->GetCleanName(), 0, merc->GetMercID());

				if(group->GroupCount() <= 1)
					group->DisbandGroup();
			}
			else {
				for(int i = 0; i < MAX_GROUP_MEMBERS; i++) {
					if(!group->members[i])
						continue;
				}

				group->DisbandGroup();
				database.SetGroupID(merc->GetCleanName(), 0, merc->GetMercID());
			}

			Result = true;
		}
	}

	return Result;
}

bool Merc::AddMercToGroup(Merc* merc, Group* group) {
	bool Result = false;

	if(merc && group) {
		if(!merc->HasGroup()) {
			// Add bot to this group
			if(group->AddMember(merc)) {
				if(group->GetLeader()) {
					merc->SetFollowID(group->GetLeader()->GetID());

					// Need to send this only once when a group is formed with a bot so the client knows it is also the group leader
					if(group->GroupCount() == 2 && group->GetLeader()->IsClient()) {
						group->UpdateGroupAAs();
						Mob *TempLeader = group->GetLeader();
						group->SendUpdate(groupActUpdate, TempLeader);
					}
				}

				Result = true;
			}
		}
	}

	return Result;
}

Merc* Client::GetMerc() {
	if(GetMercID() == 0)
		return(NULL);
	
	Merc* tmp = entity_list.GetMercByID(GetMercID());
	if(tmp == NULL) {
		SetMercID(0);
		return(NULL);
	}
	
	if(tmp->GetOwnerID() != GetID()) {
		SetMercID(0);
		return(NULL);
	}
	
	return(tmp);
}

void Merc::SetMercData( uint32 template_id ) {
	MercTemplate* merc_template = zone->GetMercTemplate(template_id);

	SetMercID(1);   //ToDO: Get ID From Merc table after saving
	SetMercTemplateID( merc_template->MercTemplateID );
	SetMercType( merc_template->MercType );
	SetMercSubType( merc_template->MercSubType );
	SetProficiencyID( merc_template->ProficiencyID );
	SetCostFormula( merc_template->CostFormula );
	SetMercNameType( merc_template->MercNameType );
}

MercTemplate* Zone::GetMercTemplate( uint32 template_id ) {
	return &merc_templates[template_id];
}

void Client::SetMerc(Merc* newmerc) {
	Merc* oldmerc = GetMerc();
	if (oldmerc) {
		oldmerc->SetOwnerID(0);
	}
	if (newmerc == NULL) {
		SetMercID(0);
		GetEPP().mercTemplateID = 0;
		GetEPP().mercIsSuspended = false;
		GetEPP().mercTimerRemaining = 0;
		GetEPP().mercSuspendedTime = 0;
		GetEPP().mercGender = 0;
		GetEPP().mercState = 0;
	} else {
		SetMercID(newmerc->GetID());
		Client* oldowner = entity_list.GetClientByID(newmerc->GetOwnerID());
		if (oldowner)
			oldowner->SetMercID(0);
		newmerc->SetOwnerID(this->GetID());
		newmerc->SetClientVersion((uint8)this->GetClientVersion());
		GetEPP().mercTemplateID = newmerc->GetMercTemplateID();
		GetEPP().mercIsSuspended = newmerc->IsSuspended();
		GetEPP().mercSuspendedTime = 0;
		GetEPP().mercGender = newmerc->GetGender();

		if(!IsGrouped()) {
			Group *g = new Group(this);

			if(newmerc->AddMercToGroup(newmerc, g)) {
			entity_list.AddGroup(g);
			database.SetGroupLeaderName(g->GetID(), this->GetName());
			g->SaveGroupLeaderAA();
			database.SetGroupID(this->GetName(), g->GetID(), this->CharacterID());
			database.SetGroupID(newmerc->GetCleanName(), g->GetID(), newmerc->GetMercID());
			}
		}
		else {
			newmerc->AddMercToGroup(newmerc, this->GetGroup());
			database.SetGroupID(GetCleanName(), this->GetGroup()->GetID(), newmerc->GetMercID());
		}
	}
}

void Client::SendMercMerchantResponsePacket(sint32 response_type) {
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_MercenaryHire, sizeof(MercenaryMerchantResponse_Struct));
	MercenaryMerchantResponse_Struct* mmr = (MercenaryMerchantResponse_Struct*)outapp->pBuffer;
	mmr->ResponseType = response_type;		// send specified response type
	
	DumpPacket(outapp);
	FastQueuePacket(&outapp);
}

void Client::SendMercSuspendResponsePacket(int32 suspended_time) {
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_MercenarySuspendResponse, sizeof(SuspendMercenaryResponse_Struct));
	SuspendMercenaryResponse_Struct* smr = (SuspendMercenaryResponse_Struct*)outapp->pBuffer;
	smr->SuspendTime = suspended_time;		// Seen 0 (not suspended) or c9 c2 64 4f (suspended on Sat Mar 17 11:58:49 2012) - Unix Timestamp
	
	DumpPacket(outapp);
	FastQueuePacket(&outapp);
}

void Client::SendMercTimerPacket(sint32 entity_id, sint32 merc_state, sint32 suspended_time, sint32 update_interval, sint32 unk01) {
	// Send Mercenary Status/Timer packet
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_MercenaryTimer, sizeof(MercenaryStatus_Struct));
	MercenaryStatus_Struct* mss = (MercenaryStatus_Struct*)outapp->pBuffer;
	mss->MercEntityID = entity_id; // Seen 0 (no merc spawned) or 615843841 and 22779137
	mss->UpdateInterval = update_interval; // Seen 900000 - Matches from 0x6537 packet (15 minutes in ms?)
	mss->MercUnk01 = unk01; // Seen 180000 - 3 minutes in milleseconds? Maybe next update interval?
	mss->MercState = merc_state; // Seen 5 (normal) or 1 (suspended)
	mss->SuspendedTime = suspended_time; // Seen 0 (not suspended) or c9 c2 64 4f (suspended on Sat Mar 17 11:58:49 2012) - Unix Timestamp
	
	DumpPacket(outapp);
	FastQueuePacket(&outapp);
}

void Client::SendMercAssignPacket(int32 entityID, int32 unk01, int32 unk02) {
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_MercenaryAssign, sizeof(MercenaryAssign_Struct));
	MercenaryAssign_Struct* mas = (MercenaryAssign_Struct*)outapp->pBuffer;
	mas->MercEntityID = entityID;
	mas->MercUnk01 = unk01;
	mas->MercUnk02 = unk02;
	FastQueuePacket(&outapp);
}

void NPC::LoadMercTypes(){
	std::string errorMessage;
	char* Query = 0;
	char TempErrorMessageBuffer[MYSQL_ERRMSG_SIZE];
	MYSQL_RES* DatasetResult;
	MYSQL_ROW DataRow;

	if(!database.RunQuery(Query, MakeAnyLenString(&Query, "SELECT DISTINCT MTyp.dbstring, MTyp.clientversion FROM merc_merchant_entries MME, merc_merchant_template_entries MMTE, merc_types MTyp, merc_templates MTem WHERE MME.merchant_id = %i AND MME.merc_merchant_template_id = MMTE.merc_merchant_template_id AND MMTE.merc_template_id = MTem.merc_template_id AND MTem.merc_type_id = MTyp.merc_type_id;", GetNPCTypeID()), TempErrorMessageBuffer, &DatasetResult)) {
		errorMessage = std::string(TempErrorMessageBuffer);
	}
	else {
		while(DataRow = mysql_fetch_row(DatasetResult)) {
			MercType tempMercType;

			tempMercType.Type = atoi(DataRow[0]);
			tempMercType.ClientVersion = atoi(DataRow[1]);

			mercTypeList.push_back(tempMercType);
		}

		mysql_free_result(DatasetResult);
	}

	safe_delete(Query);
	Query = 0;

	if(!errorMessage.empty()) {
		LogFile->write(EQEMuLog::Error, "Error in NPC::LoadMercTypes()");
	}
}

void NPC::LoadMercs(){

	std::string errorMessage;
	char* Query = 0;
	char TempErrorMessageBuffer[MYSQL_ERRMSG_SIZE];
	MYSQL_RES* DatasetResult;
	MYSQL_ROW DataRow;

	if(!database.RunQuery(Query, MakeAnyLenString(&Query, "SELECT DISTINCT MTem.merc_template_id, MTyp.dbstring AS merc_type_id, MTem.dbstring AS merc_subtype_id, 0 AS CostFormula, MTem.clientversion FROM merc_merchant_entries MME, merc_merchant_template_entries MMTE, merc_types MTyp, merc_templates MTem WHERE MME.merchant_id = %i AND MME.merc_merchant_template_id = MMTE.merc_merchant_template_id AND MMTE.merc_template_id = MTem.merc_template_id AND MTem.merc_type_id = MTyp.merc_type_id;", GetNPCTypeID()), TempErrorMessageBuffer, &DatasetResult)) {
		errorMessage = std::string(TempErrorMessageBuffer);
	}
	else {
		while(DataRow = mysql_fetch_row(DatasetResult)) {
			MercData tempMerc;

			tempMerc.MercTemplateID = atoi(DataRow[0]);
			tempMerc.MercType = atoi(DataRow[1]);
			tempMerc.MercSubType = atoi(DataRow[2]);
			tempMerc.CostFormula = atoi(DataRow[3]);
			tempMerc.ClientVersion = atoi(DataRow[4]);

			mercDataList.push_back(tempMerc);
		}

		mysql_free_result(DatasetResult);
	}

	safe_delete(Query);
	Query = 0;

	if(!errorMessage.empty()) {
		LogFile->write(EQEMuLog::Error, "Error in NPC::LoadMercTypes()");
	}
}

int NPC::GetNumMercTypes(int32 clientVersion)
{
	int count = 0;
	std::list<MercType> mercTypeList = GetMercTypesList();

	for(std::list<MercType>::iterator mercTypeListItr = mercTypeList.begin(); mercTypeListItr != mercTypeList.end(); mercTypeListItr++) {
		if(mercTypeListItr->ClientVersion <= clientVersion)
			count++;
	}

	return count;
}

int NPC::GetNumMercs(int32 clientVersion)
{
	int count = 0;
	std::list<MercData> mercDataList = GetMercsList();

	for(std::list<MercData>::iterator mercListItr = mercDataList.begin(); mercListItr != mercDataList.end(); mercListItr++) {
		if(mercListItr->ClientVersion <= clientVersion)
			count++;
	}

	return count;
}

std::list<MercType> NPC::GetMercTypesList(int32 clientVersion) {
	std::list<MercType> result;

	if(GetNumMercTypes() > 0) {
		for(std::list<MercType>::iterator mercTypeListItr = mercTypeList.begin(); mercTypeListItr != mercTypeList.end(); mercTypeListItr++) {
			if(mercTypeListItr->ClientVersion <= clientVersion) {
				MercType mercType;
				mercType.Type = mercTypeListItr->Type;
				mercType.ClientVersion = mercTypeListItr->ClientVersion;
				result.push_back(mercType);
			}
		}		
	}

	return result;
}

std::list<MercData> NPC::GetMercsList(int32 clientVersion) {
	std::list<MercData> result;

	if(GetNumMercs() > 0) {
		for(std::list<MercData>::iterator mercListItr = mercDataList.begin(); mercListItr != mercDataList.end(); mercListItr++) {
			if(mercListItr->ClientVersion <= clientVersion) {
				MercTemplate *merc_template = zone->GetMercTemplate(mercListItr->MercTemplateID);

				if(merc_template) {
					MercData mercData;
					mercData.MercTemplateID = mercListItr->MercTemplateID;
					mercData.MercType = merc_template->MercType;
					mercData.MercSubType = merc_template->MercSubType;			
					mercData.CostFormula = merc_template->CostFormula;		
					mercData.ClientVersion = merc_template->ClientVersion;
					result.push_back(mercData);
				}
			}
		}		
	}

	return result;
}

int32 Merc::CalcPurchaseCost( uint32 templateID , uint8 level, uint8 currency_type) {
	int32 cost = 0;

	MercTemplate *mercData = zone->GetMercTemplate(templateID);

	if(mercData) {
		if(currency_type == 0) { //calculate cost in coin - cost in gold
			int levels_above_cutoff;
			switch (mercData->CostFormula) {
			case 0:
				levels_above_cutoff = level > 10 ? (level - 10) : 0;
				cost = levels_above_cutoff * 300;
				cost += level >= 10 ? 100 : 0;
				break;
			default:
				break;
			}
		}
		else if(currency_type == 19) {
			cost = 0;
		}
	}

	return cost/100;
}

int32 Merc::CalcUpkeepCost( uint32 templateID , uint8 level, uint8 currency_type) {
	int32 cost = 0;

	MercTemplate *mercData = zone->GetMercTemplate(templateID);

	if(mercData) {
		if(currency_type == 0) { //calculate cost in coin - cost in gold
			int levels_above_cutoff;
			switch (mercData->CostFormula) {
			case 0:
				levels_above_cutoff = level > 10 ? (level - 10) : 0;
				cost = levels_above_cutoff * 300;
				cost += level >= 10 ? 100 : 0;
				break;
			default:
				break;
			}
		}
		else if(currency_type == 19) { // cost in Bayle Marks
			cost = 1;
		}
	}

	return cost/100;
}