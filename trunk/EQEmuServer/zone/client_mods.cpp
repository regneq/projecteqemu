/*  EQEMu:  Everquest Server Emulator
	Copyright (C) 2001-2003  EQEMu Development Team (http://eqemulator.net)

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
#include "masterentity.h"
#include "worldserver.h"
#include "zonedb.h"
#include "spdat.h"
#include "../common/packet_dump.h"
#include "../common/packet_functions.h"
#include "petitions.h"
#include "../common/serverinfo.h"
#include "../common/ZoneNumbers.h"
#include "../common/moremath.h"
#include "../common/guilds.h"
#include "../common/logsys.h"
#include "StringIDs.h"
#include "NpcAI.h"


// Return max stat value for level
sint16 Client::GetMaxStat() const {
	int level = GetLevel();
	
	sint16 base = 0;
	
	if (level < 61) {
		base = 255;
	}
	else if (GetClientVersion() == EQClientSoF) {
		base = 255 + 5 * (level - 60);
	}
	else if (level < 71) {
		base = 255 + 5 * (level - 60);
	}
	else {
		base = 330;
	}
	
	base += GetAA(aaPlanarPower) * 5;
	base += GetAA(aaChaoticPotential) * 5;
	return(base);
}

sint16 Client::GetMaxResist() const 
{
	int level = GetLevel();

	sint16 base = 500;
	
	if(level > 65)
		base += ((level - 65) * 5);

	base +=  (GetAA(aaDiscordantDefiance) * 5);

	return base;
}

sint16 Client::GetMaxSTR() const {
	return GetMaxStat()
		+ itembonuses.STRCapMod
		+ spellbonuses.STRCapMod;
}
sint16 Client::GetMaxSTA() const {
	return GetMaxStat()
		+ itembonuses.STACapMod
		+ spellbonuses.STACapMod;
}
sint16 Client::GetMaxDEX() const {
	return GetMaxStat()
		+ itembonuses.DEXCapMod
		+ spellbonuses.DEXCapMod;
}
sint16 Client::GetMaxAGI() const {
	return GetMaxStat()
		+ itembonuses.AGICapMod
		+ spellbonuses.AGICapMod;
}
sint16 Client::GetMaxINT() const {
	return GetMaxStat()
		+ itembonuses.INTCapMod
		+ spellbonuses.INTCapMod
		+ (GetAA(aaInnateEnlightenment) * 10);
}
sint16 Client::GetMaxWIS() const {
	return GetMaxStat()
		+ itembonuses.WISCapMod
		+ spellbonuses.WISCapMod
		+ (GetAA(aaInnateEnlightenment) * 10);
}
sint16 Client::GetMaxCHA() const {
	return GetMaxStat()
		+ itembonuses.CHACapMod
		+ spellbonuses.CHACapMod;
}
sint16 Client::GetMaxMR() const {
	return GetMaxResist()
		+ itembonuses.MRCapMod
		+ spellbonuses.MRCapMod;
}
sint16 Client::GetMaxPR() const {
	return GetMaxResist()
		+ itembonuses.PRCapMod
		+ spellbonuses.PRCapMod;
}
sint16 Client::GetMaxDR() const {
	return GetMaxResist()
		+ itembonuses.DRCapMod
		+ spellbonuses.DRCapMod;
}
sint16 Client::GetMaxCR() const {
	return GetMaxResist()
		+ itembonuses.CRCapMod
		+ spellbonuses.CRCapMod;
}
sint16 Client::GetMaxFR() const {
	return GetMaxResist()
		+ itembonuses.FRCapMod
		+ spellbonuses.FRCapMod;
}
sint32 Client::LevelRegen()
{
	bool sitting = IsSitting();
	int level = GetLevel();
	
	sint32 hp = 0;
	if (level <= 19) {
		if(IsSitting())
			hp+=2;
		else
			hp+=1;
	}
	else if(level <= 49) {
		if(sitting)
			hp+=3;
		else
			hp+=1;
	}
	else if(level == 50) {
		if(sitting)
			hp+=4;

		else
			hp+=1;
	}
	else if(level <= 55) {
		if(sitting)
			hp+=5;
		else
			hp+=2;
	}
	else if(level <= 58) {
		if(sitting)
			hp+=6;
		else
			hp+=3;
	}
	else if(level <= 65) {
		if(sitting)
			hp+=7;
		else
			hp+=4;
	}
	else {
		if(sitting)
			hp+=8;
		else
			hp+=5;
	}
	if(GetRace() == IKSAR || GetRace() == TROLL) {
		if (level <= 19) {
			if(sitting)
				hp+=4;
			else
				hp += 2;
		}
		else if(level <= 49) {
			if(sitting)
				hp+=6;
			else
				hp+=2;
		}
		else if(level == 50) {
			if(sitting)
				hp+=8;
			else
				hp+=2;
		}
		else if(level == 51) {
			if(sitting)
				hp+=12;
			else
				hp+=6;
		}
		else if(level <= 56) {
			if(sitting)
				hp+=16;
			else
				hp+=10;
		}
		else if(level <= 65) {
			if(sitting)
				hp+=18;
			else
				hp+=12;
		}
		else {
			if(sitting)
				hp+=20;
			else
				hp+=10;
		}
	}
	// AA Regens
	hp += (GetAA(aaInnateRegeneration)
		//+ GetAA(aaInnateRegeneration2) //not currently in the AA table anymore, so why bother?
		+ GetAA(aaNaturalHealing)
		+ GetAA(aaBodyAndMindRejuvenation)
		+ GetAA(aaConvalescence)
		+ GetAA(aaHealthyAura)
		+ GroupLeadershipAAHealthRegeneration()
	);
	if (GetAppearance() == eaDead) {	//stunned/mezzed
		hp /= 4;
	}
	
	return hp;
}

sint32 Client::CalcMaxHP() {
	int32 nd = 10000;
	max_hp = (CalcBaseHP() + itembonuses.HP);

	//The AA desc clearly says it only applies to base hp..
	//but the actual effect sent on live causes the client
	//to apply it to (basehp + itemhp).. I will oblige to the client's whims over
	//the aa description
	switch(GetAA(aaNaturalDurability)) {
	case 1:
		nd += 200;
		break;
	case 2:
		nd += 500;
		break;
	case 3:
		nd += 1000;
		break;
	}
	
	if(GetAA(aaPhysicalEnhancement))
		nd += 200;

	nd += 150*GetAA(aaPlanarDurability);

	max_hp = max_hp * nd / 10000;
	max_hp += 100*GetAA(aaSturdiness);
	max_hp += spellbonuses.HP;

	max_hp += GroupLeadershipAAHealthEnhancement();
	
	if (cur_hp > max_hp)
		cur_hp = max_hp;
	return max_hp;
}

int16 Mob::GetClassLevelFactor(){
	int16 multiplier = 0;
	int8 mlevel=GetLevel();
	switch(GetClass())
	{
		case WARRIOR:{
			if (mlevel < 20)
				multiplier = 220;
			else if (mlevel < 30)
				multiplier = 230;
			else if (mlevel < 40)
				multiplier = 250;
			else if (mlevel < 53)
				multiplier = 270;
			else if (mlevel < 57)
				multiplier = 280;
			else if (mlevel < 60)
				multiplier = 290;
			else if (mlevel < 70)
				multiplier = 300;
			else 
				multiplier = 311;
			break;
		}
		case DRUID:
		case CLERIC:
		case SHAMAN:{
			if (mlevel < 70)
				multiplier = 150;
			else
				multiplier = 157;
			break;
		}
		case BERSERKER:
		case PALADIN:
		case SHADOWKNIGHT:{
			if (mlevel < 35)
				multiplier = 210;
			else if (mlevel < 45)
				multiplier = 220;
			else if (mlevel < 51)
				multiplier = 230;
			else if (mlevel < 56)
				multiplier = 240;
			else if (mlevel < 60)
				multiplier = 250;
			else if (mlevel < 68)
				multiplier = 260;
			else
				multiplier = 270;
			break;
		}
		case MONK:
		case BARD:
		case ROGUE:
		case BEASTLORD:{
			if (mlevel < 51)
				multiplier = 180;
			else if (mlevel < 58)
				multiplier = 190;
			else if (mlevel < 70)
				multiplier = 200;
			else
				multiplier = 210;
			break;
		}
		case RANGER:{
			if (mlevel < 58)
				multiplier = 200;
			else if (mlevel < 70)
				multiplier = 210;
			else
				multiplier = 220;
			break;
		}
		case MAGICIAN:
		case WIZARD:
		case NECROMANCER:
		case ENCHANTER:{
			if (mlevel < 70)
				multiplier = 120;
			else
				multiplier = 127;
			break;
		}
		default:{
			if (mlevel < 35)
				multiplier = 210;
			else if (mlevel < 45)
				multiplier = 220;
			else if (mlevel < 51)
				multiplier = 230;
			else if (mlevel < 56)
				multiplier = 240;
			else if (mlevel < 60)
				multiplier = 250;
			else
				multiplier = 260;
			break;
		}
	}
	return multiplier;
}

sint32 Client::CalcBaseHP()
{
	int16 lm=GetClassLevelFactor();
	int16 Post255;
	if((GetSTA()-255)/2 > 0)
		Post255 = (GetSTA()-255)/2;
	else
		Post255 = 0;
		
	base_hp = (5)+(GetLevel()*lm/10) + (((GetSTA()-Post255)*GetLevel()*lm/3000)) + ((Post255*GetLevel())*lm/6000);
	return base_hp;
}

// This should return the combined AC of all the items the player is wearing.
sint16 Client::GetRawItemAC() {
	sint16 Total = 0;
	
	for (sint16 slot_id=0; slot_id<21; slot_id++) {
		const ItemInst* inst = m_inv[slot_id];
		if (inst && inst->IsType(ItemClassCommon)) {
			Total += inst->GetItem()->AC;
		}
	}
	
	return Total;
}

sint16 Client::acmod() {
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
	LogFile->write(EQEMuLog::Error, "Error in Client::acmod(): Agility: %i, Level: %i",agility,level);
#endif
	return 0;
};

// This is a testing formula for AC, the value this returns should be the same value as the one the client shows...
// ac1 and ac2 are probably the damage migitation and damage avoidance numbers, not sure which is which.
// I forgot to include the iksar defense bonus and i cant find my notes now...
// AC from spells are not included (cant even cast spells yet..)
sint16 Client::CalcAC() {

	// new formula
	int avoidance = 0;
	avoidance = (acmod() + ((GetSkill(DEFENSE)*16)/9));
	if (avoidance < 0)
		avoidance = 0;

	int mitigation = 0;
	if (m_pp.class_ == WIZARD || m_pp.class_ == MAGICIAN || m_pp.class_ == NECROMANCER || m_pp.class_ == ENCHANTER) {
		//something is wrong with this, naked casters have the wrong natural AC
//		mitigation = (spellbonuses.AC/3) + (GetSkill(DEFENSE)/2) + (itembonuses.AC+1);
		mitigation = GetSkill(DEFENSE)/4 + (itembonuses.AC+1);
		//this might be off by 4..
		mitigation -= 4;
	} else {
//		mitigation = (spellbonuses.AC/4) + (GetSkill(DEFENSE)/3) + ((itembonuses.AC*4)/3);
		mitigation = GetSkill(DEFENSE)/3 + ((itembonuses.AC*4)/3);
		if(m_pp.class_ == MONK)
			mitigation += GetLevel() * 13/10;	//the 13/10 might be wrong, but it is close...
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
	
	//spell AC bonuses are added directly to natural total
	displayed += spellbonuses.AC;
	
	AC = displayed;
	return(AC);
}

sint32 Client::CalcMaxMana()
{
	int WisInt = 0;
	int MindLesserFactor, MindFactor;
	switch(GetCasterClass())
	{
		case 'I': 
			WisInt = GetINT();

			if((( WisInt - 199 ) / 2) > 0)
				MindLesserFactor = ( WisInt - 199 ) / 2;
			else
				MindLesserFactor = 0;

			MindFactor = WisInt - MindLesserFactor;
			if(WisInt > 100)
				max_mana = (((5 * (MindFactor + 20)) / 2) * 3 * GetLevel() / 40);
			else
				max_mana = (((5 * (MindFactor + 200)) / 2) * 3 * GetLevel() / 100);	
			
			max_mana += (itembonuses.Mana + spellbonuses.Mana + GroupLeadershipAAManaEnhancement());
			break;

		case 'W':
			WisInt = GetWIS();

			if((( WisInt - 199 ) / 2) > 0)
				MindLesserFactor = ( WisInt - 199 ) / 2;
			else
				MindLesserFactor = 0;

			MindFactor = WisInt - MindLesserFactor;
			if(WisInt > 100)
				max_mana = (((5 * (MindFactor + 20)) / 2) * 3 * GetLevel() / 40);
			else
				max_mana = (((5 * (MindFactor + 200)) / 2) * 3 * GetLevel() / 100);	
			
			max_mana += (itembonuses.Mana + spellbonuses.Mana + GroupLeadershipAAManaEnhancement());
			break;
				
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
	if (cur_mana > max_mana) {
		cur_mana = max_mana;
	}
#if EQDEBUG >= 11
	LogFile->write(EQEMuLog::Debug, "Client::CalcMaxMana() called for %s - returning %d", GetName(), max_mana);
#endif
	return max_mana;
}

int16 Client::CalcCurrentWeight() {
	const Item_Struct* TempItem = 0;
	ItemInst* ins;
	int16 Total = 0;
	int x;
	for(x = 0; x <= 30; x++)
	{
		TempItem = 0;
		ins = GetInv().GetItem(x);
		if (ins)
			TempItem = ins->GetItem();
		if (TempItem)
			Total += TempItem->Weight;
	}
	for (x = 251; x < 331; x++)
	{
		int TmpWeight = 0;
		TempItem = 0;
		ins = GetInv().GetItem(x);
		if (ins)
			TempItem = ins->GetItem();
		if (TempItem)
			TmpWeight = TempItem->Weight;
		if (TmpWeight > 0)
		{
			int bagslot = 22;
			int reduction = 0;
			for (int m = 261; m < 331; m += 10)
			{
				if (x >= m)
					bagslot += 1;
			}
			ItemInst* baginst = GetInv().GetItem(bagslot);
			if (baginst && baginst->GetItem() && baginst->IsType(ItemClassContainer))
				reduction = baginst->GetItem()->BagWR;
			if (reduction > 0)
				TmpWeight -= TmpWeight*reduction/100;
			Total += TmpWeight;
		}
	}

	if (GetAA(aaPackrat) > 0)
		Total *= (GetAA(aaPackrat) * 10) / 100; //AndMetal: guessing 10% per level, up to 50%. description just indicates it affects gear, doesn't mention coin
	
	Total += (m_pp.platinum + m_pp.gold + m_pp.silver + m_pp.copper) / 4;
	return Total;
}

sint16 Client::CalcSTR() {
	sint16 val = m_pp.STR + itembonuses.STR + spellbonuses.STR;
	
	sint16 mod = 2 * (GetAA(aaInnateStrength) + GetAA(aaAdvancedInnateStrength));
	
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

sint16 Client::CalcSTA() {
	sint16 val = m_pp.STA + itembonuses.STA + spellbonuses.STA;
	
	sint16 mod = 2 * (GetAA(aaInnateStamina) + GetAA(aaAdvancedInnateStamina));
	
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

sint16 Client::CalcAGI() {
	sint16 val = m_pp.AGI + itembonuses.AGI + spellbonuses.AGI;
	
	sint16 mod = 2 * (GetAA(aaInnateAgility) + GetAA(aaAdvancedInnateAgility));
	
	sint16 str = GetSTR()*10;
	if(weight > str) {
		//ratio is wrong (close), but better than nothing
		val -= (weight-str) * 100 / 1500;	//WR said /1875
	}
	
	if(val>255 && GetLevel() <= 60)
		val = 255;
	AGI = val + mod;
	
	if(AGI < 1)
		AGI = 1;

	int m = GetMaxAGI();
	if(AGI > m)
		AGI = m;
	
	return(AGI);
}

sint16 Client::CalcDEX() {
	sint16 val = m_pp.DEX + itembonuses.DEX + spellbonuses.DEX;
	
	sint16 mod = 2 * (GetAA(aaInnateDexterity) + GetAA(aaAdvancedInnateDexterity));
	
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

sint16 Client::CalcINT() {
	sint16 val = m_pp.INT + itembonuses.INT + spellbonuses.INT;
	
	sint16 mod = 2 * (GetAA(aaInnateIntelligence) + GetAA(aaAdvancedInnateIntelligence));
	
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

sint16 Client::CalcWIS() {
	sint16 val = m_pp.WIS + itembonuses.WIS + spellbonuses.WIS;
	
	sint16 mod = 2 * (GetAA(aaInnateWisdom) + GetAA(aaAdvancedInnateWisdom));
	
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

sint16 Client::CalcCHA() {
	sint16 val = m_pp.CHA + itembonuses.CHA + spellbonuses.CHA;
	
	sint16 mod = 2 * (GetAA(aaInnateCharisma) + GetAA(aaAdvancedInnateCharisma));
	
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

int Client::CalcHaste() {
	int h = spellbonuses.haste + spellbonuses.hastetype2 + itembonuses.haste;
	int cap = 0;
	int level = GetLevel();
	/*
	if(disc_inuse == discBlindingSpeed) {
		if(!disc_elapse.Check(false)) {
			h += 20;		//this ammount is completely unknown
		} else {
			disc_inuse = discNone;
		}
	} */

	if(level < 30) { // Rogean: Are these caps correct? Will use for now.
		cap = 50;
	} else if(level < 50) {
		cap = 74;
	} else if(level < 55) {
		cap = 84;
	} else if(level < 60) {
		cap = 94;
	} else {
		cap = 100;
	}
	

	if(h > cap) h = cap;

	h += spellbonuses.hastetype3;
	h += ExtraHaste;	//GM granted haste.
	
	Haste = h;
	return(Haste); 
}

//The AA multipliers are set to be 5, but were 2 on WR
//The resistant discipline which I think should be here is implemented
//in Mob::ResistSpell
sint16	Client::CalcMR()
{
	//racial bases
	switch(GetBaseRace()) {
		case HUMAN:
			MR = 25;
			break;
		case BARBARIAN:
			MR = 25;
			break;
		case ERUDITE:
			MR = 30;
			break;
		case WOOD_ELF:
			MR = 25;
			break;
		case HIGH_ELF:
			MR = 25;
			break;
		case DARK_ELF:
			MR = 25;
			break;
		case HALF_ELF:
			MR = 25;
			break;
		case DWARF:
			MR = 30;
			break;
		case TROLL:
			MR = 25;
			break;
		case OGRE:
			MR = 25;
			break;
		case HALFLING:
			MR = 25;
			break;
		case GNOME:
			MR = 25;
			break;
		case IKSAR:
			MR = 25;
			break;
		case VAHSHIR:
			MR = 25;
			break;
		case FROGLOK:
			MR = 30;
			break;
		case DRAKKIN:
			MR = 35;
			break;
		default:
			MR = 20;
	}
	
	MR += itembonuses.MR + spellbonuses.MR;
	MR += (GetAA(aaInnateMagicProtection) + GetAA(aaMarrsProtection))*2;
	
	if(GetClass() == WARRIOR)
		MR += GetLevel() / 2;
	
	if(MR < 1)
		MR = 1;

	if(MR > GetMaxMR())
		MR = GetMaxMR();

	return(MR);
}

sint16	Client::CalcFR()
{
	//racial bases
	switch(GetBaseRace()) {
		case HUMAN:
			FR = 25;
			break;
		case BARBARIAN:
			FR = 25;
			break;
		case ERUDITE:
			FR = 25;
			break;
		case WOOD_ELF:
			FR = 25;
			break;
		case HIGH_ELF:
			FR = 25;
			break;
		case DARK_ELF:
			FR = 25;
			break;
		case HALF_ELF:
			FR = 25;
			break;
		case DWARF:
			FR = 25;
			break;
		case TROLL:
			FR = 5;
			break;
		case OGRE:
			FR = 25;
			break;
		case HALFLING:
			FR = 25;
			break;
		case GNOME:
			FR = 25;
			break;
		case IKSAR:
			FR = 30;
			break;
		case VAHSHIR:
			FR = 25;
			break;
		case FROGLOK:
			FR = 25;
			break;
		case DRAKKIN:
			FR = 25;
			break;
		default:
			FR = 20;
	}
	
	int c = GetClass();
	if(c == RANGER) {
		FR += 4;
		
		int l = GetLevel();
		if(l > 49)
			FR += l - 49;
	}
	
	FR += itembonuses.FR + spellbonuses.FR;
	FR += (GetAA(aaInnateFireProtection) + GetAA(aaWardingofSolusek))*2;
	
	if(FR < 1)
		FR = 1;
	
	if(FR > GetMaxFR())
		FR = GetMaxFR();

	return(FR);
}

sint16	Client::CalcDR()
{
	//racial bases
	switch(GetBaseRace()) {
		case HUMAN:
			DR = 15;
			break;
		case BARBARIAN:
			DR = 15;
			break;
		case ERUDITE:
			DR = 10;
			break;
		case WOOD_ELF:
			DR = 15;
			break;
		case HIGH_ELF:
			DR = 15;
			break;
		case DARK_ELF:
			DR = 15;
			break;
		case HALF_ELF:
			DR = 15;
			break;
		case DWARF:
			DR = 15;
			break;
		case TROLL:
			DR = 15;
			break;
		case OGRE:
			DR = 15;
			break;
		case HALFLING:
			DR = 20;
			break;
		case GNOME:
			DR = 15;
			break;
		case IKSAR:
			DR = 15;
			break;
		case VAHSHIR:
			DR = 15;
			break;
		case FROGLOK:
			DR = 15;
			break;
		case DRAKKIN:
			DR = 15;
			break;
		default:
			DR = 15;
	}
	
	int c = GetClass();
	if(c == PALADIN) {
		DR += 8;
		
		int l = GetLevel();
		if(l > 49)
			DR += l - 49;

	} else if(c == SHADOWKNIGHT) {
		DR += 4;
		
		int l = GetLevel();
		if(l > 49)
			DR += l - 49;
	}
	
	DR += itembonuses.DR + spellbonuses.DR;
	DR += (GetAA(aaInnateDiseaseProtection) + GetAA(aaBertoxxulousGift))*2;
	
	if(DR < 1)
		DR = 1;

	if(DR > GetMaxDR())
		DR = GetMaxDR();

	return(DR);
}

sint16	Client::CalcPR()
{
	//racial bases
	switch(GetBaseRace()) {
		case HUMAN:
			PR = 15;
			break;
		case BARBARIAN:
			PR = 15;
			break;
		case ERUDITE:
			PR = 15;
			break;
		case WOOD_ELF:
			PR = 15;
			break;
		case HIGH_ELF:
			PR = 15;
			break;
		case DARK_ELF:
			PR = 15;
			break;
		case HALF_ELF:
			PR = 15;
			break;
		case DWARF:
			PR = 20;
			break;
		case TROLL:
			PR = 15;
			break;
		case OGRE:
			PR = 15;
			break;
		case HALFLING:
			PR = 20;
			break;
		case GNOME:
			PR = 15;
			break;
		case IKSAR:
			PR = 15;
			break;
		case VAHSHIR:
			PR = 15;
			break;
		case FROGLOK:
			PR = 30;
			break;
		case DRAKKIN:
			PR = 15;
			break;
		default:
			PR = 15;
	}
	
	int c = GetClass();
	if(c == ROGUE) {
		PR += 8;
		
		int l = GetLevel();
		if(l > 49)
			PR += l - 49;

	} else if(c == SHADOWKNIGHT) {
		PR += 4;
		
		int l = GetLevel();
		if(l > 49)
			PR += l - 49;
	}
	
	PR += itembonuses.PR + spellbonuses.PR;
	PR += (GetAA(aaInnatePoisonProtection) + GetAA(aaShroudofTheFaceless))*2;
	
	if(PR < 1)
		PR = 1;

	if(PR > GetMaxPR())
		PR = GetMaxPR();

	return(PR);
}

sint16	Client::CalcCR()
{
	//racial bases
	switch(GetBaseRace()) {
		case HUMAN:
			CR = 25;
			break;
		case BARBARIAN:
			CR = 35;
			break;
		case ERUDITE:
			CR = 25;
			break;
		case WOOD_ELF:
			CR = 25;
			break;
		case HIGH_ELF:
			CR = 25;
			break;
		case DARK_ELF:
			CR = 25;
			break;
		case HALF_ELF:
			CR = 25;
			break;
		case DWARF:
			CR = 25;
			break;
		case TROLL:
			CR = 25;
			break;
		case OGRE:
			CR = 25;
			break;
		case HALFLING:
			CR = 25;
			break;
		case GNOME:
			CR = 25;
			break;
		case IKSAR:
			CR = 15;
			break;
		case VAHSHIR:
			CR = 25;
			break;
		case FROGLOK:
			CR = 25;
			break;
		case DRAKKIN:
			CR = 25;
			break;
		default:
			CR = 25;
	}
	
	int c = GetClass();
	if(c == RANGER) {
		CR += 4;
		
		int l = GetLevel();
		if(l > 49)
			CR += l - 49;
	}
	
	CR += itembonuses.CR + spellbonuses.CR;
	CR += (GetAA(aaInnateColdProtection) + GetAA(aaBlessingofEci))*2;
	
	if(CR < 1)
		CR = 1;

	if(CR > GetMaxCR())
		CR = GetMaxCR();

	return(CR);
}

sint16  Client::CalcATK() {
	ATK = itembonuses.ATK + spellbonuses.ATK + GroupLeadershipAAOffenseEnhancement();
	return(ATK);
}

int16 Mob::GetInstrumentMod(int16 spell_id) const {
	if(GetClass() != BARD)
		return(10);
	
	int16 effectmod = 10;
	
	//this should never use spell modifiers...
	//if a spell grants better modifers, they are copied into the item mods
	//because the spells are supposed to act just like having the intrument.
	
	//item mods are in 10ths of percent increases
	switch(spells[spell_id].skill) {
		case PERCUSSION_INSTRUMENTS:
			if(itembonuses.percussionMod == 0 && spellbonuses.percussionMod == 0)
				effectmod = 10;
			else if(GetSkill(PERCUSSION_INSTRUMENTS) == 0)
				effectmod = 10;
			else if(itembonuses.percussionMod > spellbonuses.percussionMod)
				effectmod = itembonuses.percussionMod;
			else
				effectmod = spellbonuses.percussionMod;
			break;
		case STRINGED_INSTRUMENTS:
			if(itembonuses.stringedMod == 0 && spellbonuses.stringedMod == 0)
				effectmod = 10;
			else if(GetSkill(STRINGED_INSTRUMENTS) == 0)
				effectmod = 10;
			else if(itembonuses.stringedMod > spellbonuses.stringedMod)
				effectmod = itembonuses.stringedMod;
			else
				effectmod = spellbonuses.stringedMod;
			break;
		case WIND_INSTRUMENTS:
			if(itembonuses.windMod == 0 && spellbonuses.windMod == 0)
				effectmod = 10;
			else if(GetSkill(WIND_INSTRUMENTS) == 0)
				effectmod = 10;
			else if(itembonuses.windMod > spellbonuses.windMod)
				effectmod = itembonuses.windMod;
			else
				effectmod = spellbonuses.windMod;
			break;
		case BRASS_INSTRUMENTS:
			if(itembonuses.brassMod == 0 && spellbonuses.brassMod == 0)
				effectmod = 10;
			else if(GetSkill(BRASS_INSTRUMENTS) == 0)
				effectmod = 10;
			else if(itembonuses.brassMod > spellbonuses.brassMod)
				effectmod = itembonuses.brassMod;
			else
				effectmod = spellbonuses.brassMod;
			break;
		case SINGING:
			if(itembonuses.singingMod == 0 && spellbonuses.singingMod == 0)
				effectmod = 10;
			else if(itembonuses.singingMod > spellbonuses.singingMod)
				effectmod = itembonuses.singingMod;
			else
				effectmod = spellbonuses.singingMod;
			break;
		default:
			effectmod = 10;
			break;
	}
	
	if(spells[spell_id].skill == SINGING)
	{
		effectmod += 2*GetAA(aaSingingMastery);
		effectmod += 2*GetAA(aaImprovedSingingMastery);
	}
	else
	{
		effectmod += 2*GetAA(aaInstrumentMastery);
		effectmod += 2*GetAA(aaImprovedInstrumentMastery);
	}
	effectmod += 2*GetAA(aaAyonaesTutelage); //singing & instruments
	effectmod += 2*GetAA(aaEchoofTaelosia); //singing & instruments


	if(effectmod < 10)
		effectmod = 10;
	
	_log(SPELLS__BARDS, "%s::GetInstrumentMod() spell=%d mod=%d\n", GetName(), spell_id, effectmod);
	
	return(effectmod);
}

//Info taken from magelo, it's a *little* off but accurate enough.
void Client::CalcMaxEndurance()
{
	int Stats = GetSTR()+GetSTA()+GetDEX()+GetAGI();

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
	
	max_end = LevelBase;

	//take all of the sums from above, then multiply by level*0.075
	max_end += ( bonus_sum * 3 * GetLevel() ) / 40;
	
	max_end += spellbonuses.Endurance + itembonuses.Endurance;
}



