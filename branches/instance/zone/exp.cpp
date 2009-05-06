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
#include "features.h"
#include "masterentity.h"
#include "StringIDs.h"
#include "../common/MiscFunctions.h"
#include "../common/rulesys.h"

#ifdef EMBPERL
#include "embparser.h"
#endif

void Client::AddEXP(int32 add_exp, int8 conlevel, bool resexp) {
	if (m_epp.perAA<0 || m_epp.perAA>100)
		m_epp.perAA=0;	// stop exploit with sanity check
	
	int32 add_aaxp;
	if(resexp) {
		add_aaxp = 0;
	} else {
		//figure out how much of this goes to AAs
		add_aaxp = add_exp * m_epp.perAA / 100;
		//take that ammount away from regular exp
		add_exp -= add_aaxp;
	
		float totalmod = 1.0;
		float zemmod = 1.0;
		//get modifiers
		if(RuleR(Character, ExpMultiplier) >= 0){
			totalmod *= RuleR(Character, ExpMultiplier);
		}

		if(zone->newzone_data.zone_exp_multiplier >= 0){
			zemmod *= zone->newzone_data.zone_exp_multiplier;
		}

		if(GetBaseRace() == HALFLING){
			totalmod *= 1.05;
		}

		if(GetClass() == ROGUE || GetClass() == WARRIOR){
			totalmod *= 1.05;
		}

		add_exp = int32(float(add_exp) * totalmod * zemmod);
	
#ifdef CON_XP_SCALING
		if (conlevel != 0xFF) {
			switch (conlevel)
			{
			case CON_GREEN:
				//Message(15,"This creature is trivial to you and offers no experience.");
				return;
			case CON_LIGHTBLUE:
					add_exp = add_exp * 2/10;
				break;
			case CON_BLUE:
				//if (lvldiff >= 12)
				//	add_exp = add_exp * 6/10;
				//else if (lvldiff > 5)
					add_exp = add_exp * 8/10;
				//else if (lvldiff > 3)
				//	add_exp = add_exp * 9/10;
				break;
			case CON_WHITE:
					add_exp = add_exp * 125/100;
				break;
			case CON_YELLOW:
					add_exp = add_exp * 150/100;
				break;
			case CON_RED:
					add_exp = add_exp * 200/100;
				break;
			}
		}
#endif

	}	//end !resexp

		float aatotalmod = 1.0;
		if(zone->newzone_data.zone_exp_multiplier >= 0){
			aatotalmod *= zone->newzone_data.zone_exp_multiplier;
		}

		if(GetBaseRace() == HALFLING){
			aatotalmod *= 1.05;
		}

		if(GetClass() == ROGUE || GetClass() == WARRIOR){
			aatotalmod *= 1.05;
		}

	int32 exp = GetEXP() + add_exp;

	int32 aaexp = (int32)(RuleR(Character, AAExpMultiplier) * add_aaxp * aatotalmod);
	int32 had_aaexp = GetAAXP();
	aaexp += had_aaexp;
	if(aaexp < had_aaexp)
		aaexp = had_aaexp;	//watch for wrap
	
	SetEXP(exp, aaexp, resexp);
}

void Client::SetEXP(int32 set_exp, int32 set_aaxp, bool isrezzexp) {
	max_AAXP = GetEXPForLevel(52) - GetEXPForLevel(51);
	if (max_AAXP == 0 || GetEXPForLevel(GetLevel()) == 0xFFFFFFFF) {
		Message(13, "Error in Client::SetEXP. EXP not set.");
		return; // Must be invalid class/race
	}
	
	
	if ((set_exp + set_aaxp) > (m_pp.exp+m_pp.expAA)) {
		if (isrezzexp)
			this->Message_StringID(15,REZ_REGAIN);
		else{
			if(this->IsGrouped())
				this->Message_StringID(15,GAIN_GROUPXP);
			else if(IsRaidGrouped())
				Message_StringID(15, GAIN_RAIDEXP);
			else
				this->Message_StringID(15,GAIN_XP);
		}
	}
	else if((set_exp + set_aaxp) < (m_pp.exp+m_pp.expAA)){ //only loss message if you lose exp, no message if you gained/lost nothing.
		Message(15, "You have lost experience.");
	}
	
	//check_level represents the level we should be when we have
	//this ammount of exp (once these loops complete)
	int16 check_level = GetLevel()+1;
	//see if we gained any levels
	while (set_exp >= GetEXPForLevel(check_level)) {
		check_level++;
		if (check_level > 127) {	//hard level cap
			check_level = 127;
			break;
		}
	}
	//see if we lost any levels
	while (set_exp < GetEXPForLevel(check_level-1)) {
		check_level--;
		if (check_level < 2) {	//hard level minimum
			check_level = 2;
			break;
		}
	}
	check_level--;

	
	//see if we gained any AAs
	if (set_aaxp >= max_AAXP) {
		/*
			Note: AA exp is stored differently than normal exp.
			Exp points are only stored in m_pp.expAA until you 
			gain a full AA point, once you gain it, a point is 
			added to m_pp.aapoints and the ammount needed to gain
			that point is subtracted from m_pp.expAA
			
			then, once they spend an AA point, it is subtracted from
			m_pp.aapoints. In theory it then goes into m_pp.aapoints_spent,
			but im not sure if we have that in the right spot.
		*/
		//record how many points we have
		uint32 last_unspentAA = m_pp.aapoints;
		
		//figure out how many AA points we get from the exp were setting
		m_pp.aapoints = set_aaxp / max_AAXP;
		
		//get remainder exp points, set in PP below
		set_aaxp = set_aaxp - (max_AAXP * m_pp.aapoints);
		
		//add in how many points we had
		m_pp.aapoints += last_unspentAA;
		//set_aaxp = m_pp.expAA % max_AAXP;
		
		//figure out how many points were actually gained
		uint32 gained = m_pp.aapoints - last_unspentAA;
		
		//Message(15, "You have gained %d skill points!!", m_pp.aapoints - last_unspentAA);
		char val1[20]={0};
		Message_StringID(15,GAIN_ABILITY_POINT,ConvertArray(m_pp.aapoints, val1),"(s)");
		//Message(15, "You now have %d skill points available to spend.", m_pp.aapoints);
	}

	int8 maxlevel = RuleI(Character, MaxExpLevel) + 1;

	if(maxlevel <= 1)
		maxlevel = RuleI(Character, MaxLevel) + 1;
	
	if(check_level > maxlevel) {
		check_level = maxlevel;
		set_exp = GetEXPForLevel(maxlevel);
	}
	
	if ((GetLevel() != check_level) && !(check_level >= maxlevel)) {
		char val1[20]={0};
		if (GetLevel() == check_level-1){
			Message_StringID(15,GAIN_LEVEL,ConvertArray(check_level,val1));
			SendLevelAppearance();
			//Message(15, "You have gained a level! Welcome to level %i!", check_level);
		}
		if (GetLevel() == check_level){
			Message_StringID(15,LOSE_LEVEL,ConvertArray(check_level,val1));
			//Message(15, "You lost a level! You are now level %i!", check_level);
		}
		else
			Message(15, "Welcome to level %i!", check_level);
		SetLevel(check_level);
	}
	
	//If were at max level then stop gaining experience if we make it to the cap
	if(GetLevel() == maxlevel - 1){
		int32 expneeded = GetEXPForLevel(maxlevel);
		if(set_exp > expneeded)
		{
			set_exp =  expneeded;
		}
	}	
	
	//set the client's EXP and AAEXP
	m_pp.exp = set_exp;
	m_pp.expAA = set_aaxp;
	
	if (GetLevel() < 51) {
		m_epp.perAA = 0;	// turn off aa exp if they drop below 51
	} else
		SendAAStats();	//otherwise, send them an AA update

	//send the expdata in any case so the xp bar isnt stuck after leveling
	int32 tmpxp1 = GetEXPForLevel(GetLevel()+1);
	int32 tmpxp2 = GetEXPForLevel(GetLevel());
	// Quag: crash bug fix... Divide by zero when tmpxp1 and 2 equalled each other, most likely the error case from GetEXPForLevel() (invalid class, etc)
	if (tmpxp1 != tmpxp2 && tmpxp1 != 0xFFFFFFFF && tmpxp2 != 0xFFFFFFFF) {
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_ExpUpdate, sizeof(ExpUpdate_Struct));
		ExpUpdate_Struct* eu = (ExpUpdate_Struct*)outapp->pBuffer;
		float tmpxp = (float) ( (float) set_exp-tmpxp2 ) / ( (float) tmpxp1-tmpxp2 );
		eu->exp = (uint32)(330.0f * tmpxp);
		FastQueuePacket(&outapp);
	}
	
	if (admin>=100 && GetGM()) {
		char val1[20]={0};
		char val2[20]={0};
		char val3[20]={0};
		Message_StringID(15,GM_GAINXP,ConvertArray(set_aaxp,val1),ConvertArray(set_exp,val2),ConvertArray(GetEXPForLevel(GetLevel()+1),val3));
		//Message(15, "[GM] You now have %d / %d EXP and %d / %d AA exp.", set_exp, GetEXPForLevel(GetLevel()+1), set_aaxp, max_AAXP);
	}
}

void Client::SetLevel(int8 set_level, bool command)
{
        if (GetEXPForLevel(set_level) == 0xFFFFFFFF) {
                LogFile->write(EQEMuLog::Error,"Client::SetLevel() GetEXPForLevel(%i) = 0xFFFFFFFF", set_level);
                return;
        }

        EQApplicationPacket* outapp = new EQApplicationPacket(OP_LevelUpdate, sizeof(LevelUpdate_Struct));
        LevelUpdate_Struct* lu = (LevelUpdate_Struct*)outapp->pBuffer;
        lu->level = set_level;
        if(m_pp.level2 != 0)
                lu->level_old = m_pp.level2;
        else
                lu->level_old = level;

        level = set_level;

        if(IsRaidGrouped())
        {
                Raid *r = this->GetRaid();
                if(r){
                        r->UpdateLevel(GetName(), set_level);
                }
        }
        if(set_level > m_pp.level2)
        {
                if(m_pp.level2 == 0)
                        m_pp.points += 5;
                else
                        m_pp.points += (5 * (set_level - m_pp.level2));

                m_pp.level2 = set_level;
        }
        if(set_level > m_pp.level) {

#ifdef EMBPERL
                ((PerlembParser*)parse)->Event(EVENT_LEVEL_UP, 0, "", (NPC*)NULL, this);
#endif
        }

        m_pp.level = set_level;
        if (command){
                m_pp.exp = GetEXPForLevel(set_level);
                Message(15, "Welcome to level %i!", set_level);
                lu->exp = 0;
        }
        else {
                float tmpxp = (float) ( (float) m_pp.exp - GetEXPForLevel( GetLevel() )) /
                                                ( (float) GetEXPForLevel(GetLevel()+1) - GetEXPForLevel(GetLevel()));
                lu->exp =  (int32)(330.0f * tmpxp);
    }
        QueuePacket(outapp);
        safe_delete(outapp);
        this->SendAppearancePacket(AT_WhoLevel, set_level); // who level change

    LogFile->write(EQEMuLog::Normal,"Setting Level for %s to %i", GetName(), set_level);

        CalcBonuses();
        if(!RuleB(Character, HealOnLevel))
        {
                int mhp = CalcMaxHP();
                if(GetHP() > mhp)
                        SetHP(mhp);
        }
        else
        {
                SetHP(CalcMaxHP());             // Why not, lets give them a free heal
        }

        SendHPUpdate();
        SetMana(CalcMaxMana());
        UpdateWho();
        Save();
}

// Note: The client calculates exp separately, we cant change this function
// Add: You can set the values you want now, client will be always sync :) - Merkur
uint32 Client::GetEXPForLevel(int16 check_level)
{

	int16 check_levelm1 = check_level-1;
	float mod;
	if (check_level < 31)
		mod = 1.0;
	else if (check_level < 36)
		mod = 1.1;
	else if (check_level < 41)
		mod = 1.2;
	else if (check_level < 46)
		mod = 1.3;
	else if (check_level < 52)
		mod = 1.4;
	else if (check_level < 53)
		mod = 1.5;
	else if (check_level < 54)
		mod = 1.6;
	else if (check_level < 55)
		mod = 1.7;
	else if (check_level < 56)
		mod = 1.9;
	else if (check_level < 57)
		mod = 2.1;
	else if (check_level < 58)
		mod = 2.3;
	else if (check_level < 59)
		mod = 2.5;
	else if (check_level < 60)
		mod = 2.7;
	else if (check_level < 61)
		mod = 3.0;
	else
		mod = 3.1;
	
	float base = (check_levelm1)*(check_levelm1)*(check_levelm1);

	mod *= 1000;
	
	return(uint32(base * mod));
}

void Group::SplitExp(uint32 exp, Mob* other) {
	if( other->CastToNPC()->MerchantType != 0 ) // Ensure NPC isn't a merchant
	  return;

	if(other->GetOwner() && other->GetOwner()->IsClient()) // Ensure owner isn't pc
		return;
	
	int i; 
	uint32 groupexp = exp; 
	int8 membercount = 0; 
	int8 maxlevel = 1;
	
	for (i = 0; i < MAX_GROUP_MEMBERS; i++) { 
	  if (members[i] != NULL) { 
		  if(members[i]->GetLevel() > maxlevel) 
			  maxlevel = members[i]->GetLevel(); 

	float groupmod;
	if (i == 2)
		groupmod = 1.2;
	else if (i == 3)
		groupmod = 1.4;
	else if (i == 4)
		groupmod = 1.6;
	else if (i == 5)
		groupmod = 1.8;
	else if (i == 6)
		groupmod = 2.16;
	else
		groupmod = 1.0;

		  //groupexp += exp/10; 
		  groupexp += (uint32)(exp * groupmod * (RuleR(Character, GroupExpMultiplier)));

		  membercount++; 
	  } 
	}
	
	int conlevel = Mob::GetLevelCon(maxlevel, other->GetLevel());
	if(conlevel == CON_GREEN)
		return;	//no exp for greenies...
	
	if (membercount == 0) 
		return; 

	for (i = 0; i < MAX_GROUP_MEMBERS; i++)  {
		if (members[i] != NULL && members[i]->IsClient()) // If Group Member is Client
		{
			Client *cmember = members[i]->CastToClient();
//			if( cmember->GetLevelCon( other->GetLevel() ) != CON_GREEN ) // If Mob doesn't con green
//			{ 
				// add exp + exp cap 
				sint16 diff = cmember->GetLevel() - maxlevel;
				sint16 maxdiff = -(cmember->GetLevel()*15/10 - cmember->GetLevel());
					if(maxdiff > -5)
						maxdiff = -5;
				if (diff >= (maxdiff)) { /*Instead of person who killed the mob, the person who has the highest level in the group*/ 				
					uint32 tmp = (cmember->GetLevel()+3) * (cmember->GetLevel()+3) * 75 * 35 / 10;
					uint32 tmp2 = groupexp / membercount;
					cmember->AddEXP( tmp < tmp2 ? tmp : tmp2, conlevel ); 
				} 
//			} 
		} 
	}
}

void Raid::SplitExp(uint32 exp, Mob* other) {
	if( other->CastToNPC()->MerchantType != 0 ) // Ensure NPC isn't a merchant
		return;

	if(other->GetOwner() && other->GetOwner()->IsClient()) // Ensure owner isn't pc
		return;

	uint32 groupexp = exp; 
	int8 membercount = 0; 
	int8 maxlevel = 1;

	for (int i = 0; i < MAX_RAID_MEMBERS; i++) { 
		if (members[i].member != NULL) { 
			if(members[i].member->GetLevel() > maxlevel) 
				maxlevel = members[i].member->GetLevel(); 
			//groupexp += (uint32)(exp * zone->GetGroupEXPBonus());
			groupexp -= (groupexp * (RuleR(Character, RaidExpMultiplier)));

			membercount++; 
		} 
	}

	int conlevel = Mob::GetLevelCon(maxlevel, other->GetLevel());
	if(conlevel == CON_GREEN)
		return;	//no exp for greenies...

	if (membercount == 0) 
		return; 

	for (int x = 0; x < MAX_GROUP_MEMBERS; x++)  {
		if (members[x].member != NULL) // If Group Member is Client
		{
			Client *cmember = members[x].member;
			// add exp + exp cap 
			sint16 diff = cmember->GetLevel() - maxlevel;
			sint16 maxdiff = -(cmember->GetLevel()*15/10 - cmember->GetLevel());
			if(maxdiff > -5)
				maxdiff = -5;
			if (diff >= (maxdiff)) { /*Instead of person who killed the mob, the person who has the highest level in the group*/ 				
				uint32 tmp = (cmember->GetLevel()+3) * (cmember->GetLevel()+3) * 75 * 35 / 10;
				uint32 tmp2 = groupexp / membercount;
				cmember->AddEXP( tmp < tmp2 ? tmp : tmp2, conlevel ); 
			} 
		} 
	}
}

void Client::SetLeadershipEXP(uint32 group_exp, uint32 raid_exp) {
	while(group_exp >= GROUP_EXP_PER_POINT) {
		group_exp -= GROUP_EXP_PER_POINT;
		m_pp.group_leadership_points++;
	}
	while(raid_exp >= RAID_EXP_PER_POINT) {
		raid_exp -= RAID_EXP_PER_POINT;
		m_pp.raid_leadership_points++;
	}
	
	m_pp.group_leadership_exp = group_exp;
	m_pp.raid_leadership_exp = raid_exp;
	
	SendLeadershipEXPUpdate();
}

void Client::AddLeadershipEXP(uint32 group_exp, uint32 raid_exp) {
	SetLeadershipEXP(GetGroupEXP() + group_exp, GetRaidEXP() + raid_exp);
}

void Client::SendLeadershipEXPUpdate() {
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_LeadershipExpUpdate, sizeof(LeadershipExpUpdate_Struct));
	LeadershipExpUpdate_Struct* eu = (LeadershipExpUpdate_Struct *) outapp->pBuffer;
	
	eu->group_leadership_exp = m_pp.group_leadership_exp;
	eu->group_leadership_points = m_pp.group_leadership_points;
	eu->raid_leadership_exp = m_pp.raid_leadership_exp;
	eu->raid_leadership_points = m_pp.raid_leadership_points;
	
	FastQueuePacket(&outapp);
}











