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

/* Franck-: EQoffline
/* This file was added to the EQEMu project to custom the Bot AI process  
/* It's based on MobAI.cpp for the most part but there are a lot of changes/tweaks
/* for the bots..
*/

#ifdef EQBOTS

#include "../common/debug.h"
#include <iostream>
using namespace std;
#include <iomanip>
using namespace std;
#include <stdlib.h>
#include <math.h>
#include "npc.h"
#include "masterentity.h"
#include "NpcAI.h"
#include "map.h"
#include "../common/moremath.h"
#include "parser.h"
#include "StringIDs.h"
#include "../common/MiscFunctions.h"


#ifdef WIN32
#define strncasecmp	_strnicmp
#define strcasecmp  _stricmp
#endif

#ifndef NEW_LoadSPDat
	extern SPDat_Spell_Struct spells[SPDAT_RECORDS];
#endif

extern EntityList entity_list;

extern Zone *zone;
extern Parser * parse;

const int Z_AGGRO=10;

const int MobAISpellRange=100; // max range of buffs
const int SpellType_Nuke=1;
const int SpellType_Heal=2;
const int SpellType_Root=4;
const int SpellType_Buff=8;
const int SpellType_Escape=16;
const int SpellType_Pet=32;
const int SpellType_Lifetap=64;
const int SpellType_Snare=128;
const int SpellType_DOT=256;
const int SpellType_Dispel=512;

const int SpellTypes_Detrimental = SpellType_Nuke|SpellType_Root|SpellType_Lifetap|SpellType_Snare|SpellType_DOT|SpellType_Dispel;
const int SpellTypes_Beneficial = SpellType_Heal|SpellType_Buff|SpellType_Escape|SpellType_Pet;

#define SpellType_Any		0xFFFF
#ifdef _EQDEBUG
	#define MobAI_DEBUG_Spells	-1
#else
	#define MobAI_DEBUG_Spells	-1
#endif

void Mob::BOT_Process() {
    _ZP(Mob_BOT_Process);

    if(!IsAIControlled())
        return;

    if(!(AIthink_timer->Check() || attack_timer.Check(false)))
        return;

	int8 botClass = GetClass();
	uint8 botLevel = GetLevel();

    if(IsCasting() && (botClass != BARD))
        return;

    // A bot wont start its AI if not grouped
    if((IsPet() && !GetOwner()->IsBot()) || (IsBot() && !IsGrouped())) {
        return;
    }

	// The bots need an owner
	if(!BotOwner)
		return;

	if(!IsEngaged()) {
		if(GetFollowID()) {
			if(BotOwner && BotOwner->CastToClient()->AutoAttackEnabled() && BotOwner->GetTarget() &&
				BotOwner->GetTarget()->IsNPC() && BotOwner->GetTarget()->GetHateAmount(BotOwner)) {
					BotOwner->CastToClient()->SetOrderBotAttack(true);
					AddToHateList(BotOwner->GetTarget(), 1);
					BotOwner->CastToClient()->SetOrderBotAttack(false);
			}
		}
	}

    if(IsEngaged()) {
        _ZP(Mob_BOT_Process_IsEngaged);
        if(IsRooted())
			CastToNPC()->SetTarget(hate_list.GetClosest(this));
        else
            CastToNPC()->SetTarget(hate_list.GetTop(this));

        if(!target)
            return;

        if(DivineAura())
            return;

        if(GetHPRatio() < 15)
            StartEnrage();

        // Let's check if we have a los with our target.
        // If we don't, our hate_list is wiped.
        // Else, it was causing the bot to aggro behind wall etc... causing massive trains.
        if(!CheckLosFN(target) || target->IsMezzed() || !IsAttackAllowed(target))
        {
			WipeHateList();
            CastToNPC()->SetTarget(BotOwner);
            return;
        }

        bool is_combat_range = CombatRange(target);

        // We're engaged, each class type has a special AI
        // Only melee class will go to melee. Casters and healers will stop and stay behind.
        // We 're a melee or any other class lvl<12. Yes, because after it becomes hard to go into melee for casters.. even for bots..
		if((botLevel <= 12) || (botClass == WARRIOR) || (botClass == PALADIN) || (botClass == RANGER) || (botClass == SHADOWKNIGHT) || (botClass == MONK) || (botClass == ROGUE) || (botClass == BEASTLORD) || (botClass == BERSERKER) || (botClass == BARD))
		{
			cast_last_time = true;
		}
        if(is_combat_range && cast_last_time)
        {
			cast_last_time = false;
            AImovement_timer->Check();
            if(IsMoving())
            {
                SetRunAnimSpeed(0);
				SetHeading(target->GetHeading());
				if(moved) {
					moved=false;
					SetMoving(false);
					SendPosUpdate();
				}
                tar_ndx = 0;
            }


            // we can't fight if we don't have a target, are stun/mezzed or dead..
            if(target && !IsStunned() && !IsMezzed() && (GetAppearance() != eaDead))
            {
                //we should check to see if they die mid-attacks, previous
                //crap of checking target for null was not gunna cut it

                //try main hand first
                if(attack_timer.Check())
                {
                    BotAttackMelee(target, SLOT_PRIMARY);
					bool tripleSuccess = false;
                    if(BotOwner && target && CanThisClassDoubleAttack()) {

						if(BotOwner && CheckBotDoubleAttack()) {
							BotAttackMelee(target, SLOT_PRIMARY, true);
						}
						if(BotOwner && target && SpecAttacks[SPECATK_TRIPLE] && CheckBotDoubleAttack(true)) {
							tripleSuccess = true;
							BotAttackMelee(target, SLOT_PRIMARY, true);
						}
						//quad attack, does this belong here??
						if(BotOwner && target && SpecAttacks[SPECATK_QUAD] && CheckBotDoubleAttack(true)) {
							BotAttackMelee(target, SLOT_PRIMARY, true);
						}
                    }

					// Handle Flurrys
					if((botClass == WARRIOR) && (botLevel >= 59)) {
						int flurrychance = 0;
						if(botLevel >= 59) { // Flurry AA's
							flurrychance += 10;
						}
						if(botLevel >= 60) {
							flurrychance += 10;
						}
						if(botLevel >= 61) {
							flurrychance += 10;
						}
						if(tripleSuccess) {
							tripleSuccess = false;
							if(botLevel >= 63) { // Raging Flurry AA's
								flurrychance += 10;
							}
							if(botLevel >= 64) {
								flurrychance += 10;
							}
							if(botLevel >= 65) {
								flurrychance += 10;
							}
							tripleSuccess = false;
						}
						if(rand()%1000 < flurrychance) {
							Message_StringID(MT_CritMelee, 128);
							BotAttackMelee(target, SLOT_PRIMARY, true);

							//50% chance for yet another attack?
							if(MakeRandomFloat(0, 1) < 0.5)
								BotAttackMelee(target, SLOT_PRIMARY, true);
						}
					}

					// Handle Punishing Blade and Speed of the Knight and Wicked Blade
                    if((botClass == MONK)||(botClass == RANGER)||(botClass == WARRIOR)||(botClass == PALADIN)||(botClass == SHADOWKNIGHT)) {
						if(botLevel >= 61) {
							ItemInst* weapon = NULL;
							const Item_Struct* botweapon = NULL;
							botweapon = database.GetItem(CastToNPC()->GetEquipment(MATERIAL_PRIMARY));
							if(botweapon != NULL) {
								weapon = new ItemInst(botweapon);
							}
							if(weapon) {
								if( weapon->GetItem()->ItemType == ItemType2HS ||
									weapon->GetItem()->ItemType == ItemType2HB ||
									weapon->GetItem()->ItemType == ItemType2HPierce )
								{
									int extatk = 0;
									if(botLevel >= 61) {
										extatk += 5;
									}
									if(botLevel >= 63) {
										extatk += 5;
									}
									if(botLevel >= 65) {
										extatk += 5;
									}
									if(botLevel >= 70) {
										extatk += 15;
									}
									if(MakeRandomInt(0, 100) < extatk) {
										BotAttackMelee(target, SLOT_PRIMARY, true);
									}
								}
							}
						}
					}
				}
				
                //now off hand
                if(target && attack_dw_timer.Check() && CanThisClassDualWield())
                {
                    //can only dual weild without a weapon if you're a monk
                    if(((GetEquipment(MATERIAL_SECONDARY) != 0) && (botLevel > 39)) || (botClass == MONK))
                    {
						const Item_Struct* weapon = NULL;
						weapon = database.GetItem(CastToNPC()->GetEquipment(MATERIAL_PRIMARY));
						int weapontype = NULL;
						bool bIsFist = true;
						if(weapon != NULL) {
							weapontype = weapon->ItemType;
							bIsFist = false;
						}
						if(bIsFist || ((weapontype != ItemType2HS) && (weapontype != ItemType2HPierce) && (weapontype != ItemType2HB))) {
							float DualWieldProbability = (GetSkill(DUAL_WIELD) + botLevel) / 400.0f;
							if(botLevel >= 59) { // AA Ambidexterity
								DualWieldProbability += 0.1f;
							}
							//discipline effects:
							DualWieldProbability += (spellbonuses.DualWeildChance + itembonuses.DualWeildChance) / 100.0f;

							float random = MakeRandomFloat(0, 1);
							if (random < DualWieldProbability) { // Max 78% of DW
								BotAttackMelee(target, SLOT_SECONDARY);
								if(target && CanThisClassDoubleAttack() && CheckBotDoubleAttack()) {
									BotAttackMelee(target, SLOT_SECONDARY);
								}
							}
						}
                    }
                }

                // Now, special attack per class (kick, backstab etc..)
                CastToNPC()->DoClassAttacks(target);

                //Bard, rangers, SKs, Paladin can cast also
				if(botClass == BARD || botClass == RANGER || botClass == SHADOWKNIGHT || botClass == PALADIN || botClass == BEASTLORD) {
                    Bot_AI_EngagedCastCheck();
					BotMeditate(false);
				}
            }
        } //end is within combat range
        // Now, if we re casters, we have a particular AI.
        if((botClass == CLERIC) || (botClass == DRUID) || (botClass == SHAMAN) || (botClass == NECROMANCER) || (botClass == WIZARD) || (botClass == MAGICIAN) || (botClass == ENCHANTER))
        {
			cast_last_time = true;
            // First, let's make them stop
            AImovement_timer->Check();
            if(IsMoving())
            {
                SetRunAnimSpeed(0);
				SetHeading(target->GetHeading());
				if(moved) {
					moved=false;
					SetMoving(false);
					SendPosUpdate();
				}
                tar_ndx = 0;
            }

			BotMeditate(false);

            // Then, use their special engaged AI.
			Bot_AI_EngagedCastCheck();
        } //end is within combat range
        else {
            //we cannot reach our target...
            // See if we can summon the mob to us
            if(!HateSummon())
            {
                //could not summon them, start pursuing...
                // TODO: Check here for another person on hate list with close hate value
                if(target && Bot_AI_PursueCastCheck())
                {}
                else if(target && AImovement_timer->Check())
                {
                    if(!IsRooted()) {
                        mlog(AI__WAYPOINTS, "Pursuing %s while engaged.", target->GetName());
                        CalculateNewPosition2(target->GetX(), target->GetY(), target->GetZ(), GetRunspeed(), false);
                    } else {
						SetHeading(target->GetHeading());
						if(moved) {
							moved=false;
							SetMoving(false);
							SendPosUpdate();
						}
                    }
                }
            }
        }
    }
    else {
        // Franck: EQoffline
        // Ok if we're not engaged, what's happening..
		CastToNPC()->SetTarget(entity_list.GetMob(GetFollowID()));
		if(!IsMoving()) {
			BotMeditate(true);
			Bot_AI_IdleCastCheck(); // let's rebuff, heal, etc..
		}

        // now the followID: that's what happening as the bots follow their leader.
        if(GetFollowID())
        {
			if(!target) {
				SetFollowID(0);
			}
			else if(AImovement_timer->Check()){
				float dist2 = DistNoRoot(*target);
				SetRunAnimSpeed(0);
				if(dist2>184) {
					CalculateNewPosition2(target->GetX(), target->GetY(), target->GetZ(), GetRunspeed(), false);
				}
				else {
					SetHeading(target->GetHeading());
					if(moved) {
						moved=false;
						SetMoving(false);
						SendPosUpdate();
					}
				}
			}
		}
	}
}

// franck: EQoffline
// This function is reworked for the caster bots, when engaged.
// Depending of the class:
// -- Cleric, Druid, Shaman, Paladin will first check if there is someone to heal.
// -- Wizard, Mage, Necro will start the nuke.  
// -- TODO : Enchanter will nuke untill it it sees if there is an add. 
bool NPC::Bot_AI_EngagedCastCheck() {
	if (target && AIautocastspell_timer->Check(false)) {
		_ZP(NPC_Bot_AI_EngagedCastCheck);
		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.
		
		int8 botClass = GetClass();
		uint8 botLevel = GetLevel();

		mlog(AI__SPELLS, "Engaged autocast check triggered. Trying to cast healing spells then maybe offensive spells.");

        BotRaids *br = entity_list.GetBotRaidByMob(this);
		if(botClass == CLERIC)
        {
			if(br && IsBotRaiding())
            {
				// try to heal the raid main tank
				if(br->GetBotMainTank() && (br->GetBotMainTank()->GetHPRatio() < 80))
				{
					if(!Bot_AICastSpell(br->GetBotMainTank(), 100, SpellType_Heal)) {
						AIautocastspell_timer->Start(RandomTimer(500, 2000), false);
						return true;
					}
				}
				// try to heal the raid secondar tank
				else if(br->GetBotSecondTank() && (br->GetBotSecondTank()->GetHPRatio() < 80))
				{
					if(!Bot_AICastSpell(br->GetBotSecondTank(), 100, SpellType_Heal))
					{
						AIautocastspell_timer->Start(RandomTimer(500, 2000), false);
						return true;
					}
				}
			}
            if(!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, MobAISpellRange, SpellType_Heal)) {
				if(!Bot_AICastSpell(this, 100, SpellType_Escape)) {
					if(!Bot_AICastSpell(this, 100, SpellType_Heal)) {
						if(!Bot_AICastSpell(target, 4, SpellType_DOT | SpellType_Nuke | SpellType_Lifetap | SpellType_Dispel)) {
							AIautocastspell_timer->Start(RandomTimer(500, 2000), false);
							return true;
						}
					}
				}
			}
		}
        else if((botClass == DRUID) || (botClass == SHAMAN) || (botClass == PALADIN) || (botClass == SHADOWKNIGHT) || (botClass == BEASTLORD) || (botClass == RANGER))
        {
            if (!Bot_AICastSpell(this, 100, SpellType_Escape | SpellType_Pet)) {
				if (!Bot_AICastSpell(this, 100, SpellType_Heal)) {
					if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 80, MobAISpellRange, SpellType_Heal)) {
						if(!Bot_AICastSpell(target, 100, SpellType_Root | SpellType_Snare | SpellType_DOT | SpellType_Nuke | SpellType_Lifetap | SpellType_Dispel)) {
							AIautocastspell_timer->Start(RandomTimer(1000, 5000), false);
							return true;
						}
					}
				}
			}
        }
		else if((botClass == WIZARD) || (botClass == MAGICIAN) || (botClass == NECROMANCER)) {
			if (!Bot_AICastSpell(this, 100, SpellType_Escape | SpellType_Pet)) {
				if(!Bot_AICastSpell(target, 100, SpellType_Root | SpellType_Snare | SpellType_DOT | SpellType_Nuke | SpellType_Lifetap | SpellType_Dispel)) {
					//no spell to cast, try again soon.
					AIautocastspell_timer->Start(RandomTimer(500, 2000), false);
					return true;
				}
			}
		}

		// TODO: Make enchanter to be able to mez
		else if(botClass == ENCHANTER) {
			if (!Bot_AICastSpell(this, 100, SpellType_Escape | SpellType_Pet)) {
				if(!Bot_AICastSpell(target, 100, SpellType_DOT | SpellType_Nuke | SpellType_Dispel)) {
					AIautocastspell_timer->Start(RandomTimer(500, 2000), false);
					return true;
				}
			}
		}
		else if(botClass == BARD)
        {
			if(!Bot_AICastSpell(target, 100, SpellType_Nuke | SpellType_Dispel | SpellType_Escape)) // Bards will use their debuff songs
            {
				if(!Bot_AICastSpell(this, 100, SpellType_Buff | SpellType_Heal))
                {
					if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, MobAISpellRange, SpellType_Heal | SpellType_Buff))
                    {
						AIautocastspell_timer->Start(RandomTimer(500, 2000), false);
						return true;
					}					
				}
			}
		}
		// And for all the others classes..
		else {
            if(!Bot_AICastSpell(this, 100, SpellType_Heal | SpellType_Escape)) {                                 // heal itself
				if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 50, MobAISpellRange, SpellType_Heal)) {	// heal others
					if(!Bot_AICastSpell(target, 100, SpellTypes_Detrimental)) {		// nuke..
						AIautocastspell_timer->Start(RandomTimer(500, 2000), false);							// timer 5 t 20 seconds
						return true;
					}
				}
			}
		}
		if(botClass != BARD) {
			AIautocastspell_timer->Start(RandomTimer(500, 2000), false);
		}
		return true;
	}
	return false;
}

// franck: EQoffline
// This function has been reworked for the caster bots, when engaged.
// Healers bots must heal thoses who loose HP.
bool EntityList::Bot_AICheckCloseBeneficialSpells(NPC* caster, int8 iChance, float iRange, int16 iSpellTypes) {
	_ZP(EntityList_Bot_AICheckCloseBeneficialSpells);

	if((iSpellTypes&SpellTypes_Detrimental) != 0) {
		//according to live, you can buff and heal through walls...
		//now with PCs, this only applies if you can TARGET the target, but
		// according to Rogean, Live NPCs will just cast through walls/floors, no problem..
		//
		// This check was put in to address an idle-mob CPU issue
		_log(AI__ERROR, "Error: detrimental spells requested from AICheckCloseBeneficialSpells!!");
		return(false);
	}
	
	if(!caster)
		return false;

	if(!caster->AI_HasSpells())
		return false;

	if (iChance < 100) {
		int8 tmp = MakeRandomInt(0, 99);
		if (tmp >= iChance)
			return false;
	}

	// Franck: EQoffline.
	// Ok, Beneficial spells depend of the class of the caster also..
	int8 botCasterClass = caster->GetClass();;

	// Heal and buffs spells might have a different chance, that's why I separe them .
	if( botCasterClass == CLERIC || botCasterClass == DRUID || botCasterClass == SHAMAN || botCasterClass == PALADIN || botCasterClass == BEASTLORD || botCasterClass == RANGER)
	{
		//If AI_EngagedCastCheck() said to the healer that he had to heal
		if( iSpellTypes == SpellType_Heal )	// 
		{
			// check raids
			if( caster->CastToMob()->IsGrouped() && caster->CastToMob()->IsBotRaiding() && (entity_list.GetBotRaidByMob(caster) != NULL))
			{
				BotRaids *br = entity_list.GetBotRaidByMob(caster);
				// boolean trying to ai the heal rotation, prolly not working well.
				if(br) {
					if(br->GetBotMainTank() && (br->GetBotMainTank()->GetHPRatio() < 80))
					{
						if(caster->Bot_AICastSpell(br->GetBotMainTank(), 100, SpellType_Heal)) {
							return true;
						}
					}
					else if(br->GetBotSecondTank() && (br->GetBotSecondTank()->GetHPRatio() < 80))
					{
						if(caster->Bot_AICastSpell(br->GetBotSecondTank(), 100, SpellType_Heal)) {
							return true;
						}
					}
				}
			}

			// check in group
			if( caster->IsGrouped() )
			{
				Group *g = entity_list.GetGroupByMob(caster);
				if(g) {
					for( int i = 0; i<MAX_GROUP_MEMBERS; i++)
					{
						if(g->members[i] && (g->members[i]->GetHPRatio() < 80))
						{
							if(caster->Bot_AICastSpell(g->members[i], 100, SpellType_Heal))
								return true;
						}
						if(g->members[i] && g->members[i]->HasPet() && (g->members[i]->GetPet()->GetHPRatio() < 60)) {
							if(caster->Bot_AICastSpell(g->members[i]->GetPet(), 100, SpellType_Heal))
								return true;
						}
					}
				}
			}
		}
	}
	//Ok for the buffs..
	if( iSpellTypes == SpellType_Buff)
	{
		// Let's try to make Bard working...
		if(botCasterClass == BARD)
		{
			if(caster->Bot_AICastSpell(caster, 100, SpellType_Buff))
				return true;
			else
				return false;
		}

		if(caster->IsGrouped() )
		{
			Group *g = entity_list.GetGroupByMob(caster);
			if(g) {
				for( int i = 0; i<MAX_GROUP_MEMBERS; i++)
				{
					if(g->members[i]) {
						if(caster->Bot_AICastSpell(g->members[i], 100, SpellType_Buff)) {
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}


// Franck-add: EQoffline
// This function was reworked a bit for bots.
bool NPC::Bot_AI_IdleCastCheck() {
	if (AIautocastspell_timer->Check(false)) {
		_ZP(NPC_Bot_AI_IdleCastCheck);
#if MobAI_DEBUG_Spells >= 25
		cout << "Non-Engaged autocast check triggered: " << this->GetName() << endl;
#endif
		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.
		
		//Ok, IdleCastCheck depends of class. 
		// Healers will check if a heal is needed before buffing.
		int8 botClass = GetClass();
		if(botClass == CLERIC || botClass == PALADIN || botClass == RANGER)
		{
			if (!Bot_AICastSpell(this, 100, SpellType_Heal | SpellType_Buff))
			{
				if(!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, MobAISpellRange, SpellType_Heal))
				{
					if(!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, MobAISpellRange, SpellType_Buff))
					{
						AIautocastspell_timer->Start(RandomTimer(1000, 5000), false);
						return(true);
					}
				}
			}
		}
		// Pets class will first cast their pet, then buffs
		else if(botClass == DRUID || botClass == MAGICIAN || botClass == SHADOWKNIGHT || botClass == SHAMAN || botClass == NECROMANCER || botClass == ENCHANTER || botClass == BEASTLORD  || botClass == WIZARD)
		{			
			if (!Bot_AICastSpell(this, 100, SpellType_Pet))
			{
				if (!Bot_AICastSpell(this, 100, SpellType_Heal | SpellType_Buff))
				{
					if(!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, MobAISpellRange, SpellType_Heal))
					{
						if(!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, MobAISpellRange, SpellType_Buff)) // then buff the group
						{
							AIautocastspell_timer->Start(RandomTimer(1000, 5000), false);
							return(true);
						}
					}
				}
			}
		}		
		// bard bots
		else if(botClass == BARD)
		{
			Bot_AICastSpell(this, 100, SpellType_Heal);
			AIautocastspell_timer->Start(3000, false);
			return true;
		}

		// and standard buffing for others..
		else {
			if (!Bot_AICastSpell(this, 100, SpellType_Heal | SpellType_Buff | SpellType_Pet))
			{
				if(!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, MobAISpellRange, SpellType_Heal | SpellType_Buff)) {
					AIautocastspell_timer->Start(RandomTimer(1000, 5000), false);
					return true;
				}
			}
		}
		AIautocastspell_timer->Start(RandomTimer(1000, 5000), false);
		return true;
	}
	return false;
}

// Franck-add: EQoffline
// This function was reworked a bit for bots.
bool NPC::Bot_AICastSpell(Mob* tar, int8 iChance, int16 iSpellTypes) {
	_ZP(NPC_Bot_AICastSpell);

    if (!tar){
		return false;
	}
	
	if(!AI_HasSpells())
		return false;

	if (iChance < 100) {
		if (MakeRandomInt(0, 100) >= iChance){
			return false;
		}
	}
	
	int8 botClass = GetClass();
	uint8 botLevel = GetLevel();

	float dist2;

	if (iSpellTypes & SpellType_Escape) {
	    dist2 = 0; 
    } else 
	    dist2 = DistNoRoot(*tar);

	bool checked_los = false;	//we do not check LOS until we are absolutely sure we need to, and we only do it once.
	
	float manaR = GetManaRatio();
	for (int i=MAX_AISPELLS-1; i >= 0; i--) {
		if (AIspells[i].spellid <= 0 || AIspells[i].spellid >= SPDAT_RECORDS) {
			// this is both to quit early to save cpu and to avoid casting bad spells
			// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
			continue;
		}
		if (iSpellTypes & AIspells[i].type) {
			// manacost has special values, -1 is no mana cost, -2 is instant cast (no mana)
			sint32 mana_cost = AIspells[i].manacost;
			if (mana_cost == -1)
				mana_cost = spells[AIspells[i].spellid].mana;
			else if (mana_cost == -2)
				mana_cost = 0;
			bool extraMana = false;
			sint32 hasMana = GetMana();
			if(mana_cost > hasMana) {
				// Let's have the bots complete the buff time process
				if(iSpellTypes & SpellType_Buff) {
					SetMana(mana_cost + 10);
					extraMana = true;
				}
			}
			if (((((spells[AIspells[i].spellid].targettype==ST_GroupTeleport && AIspells[i].type==2)
				|| spells[AIspells[i].spellid].targettype==ST_AECaster
				|| spells[AIspells[i].spellid].targettype==ST_Group
				|| spells[AIspells[i].spellid].targettype==ST_AEBard)
				&& dist2 <= spells[AIspells[i].spellid].aoerange*spells[AIspells[i].spellid].aoerange)
				
				|| dist2 <= spells[AIspells[i].spellid].range*spells[AIspells[i].spellid].range) && (mana_cost <= GetMana() || GetMana() == GetMaxMana())) {


				switch (AIspells[i].type) {
					case SpellType_Heal: {
						if (
							( (spells[AIspells[i].spellid].targettype==ST_GroupTeleport || spells[AIspells[i].spellid].targettype == ST_Target || tar == this)
							&& tar->DontHealMeBefore() < Timer::GetCurrentTime()
							&& tar->CanBuffStack(AIspells[i].spellid, botLevel, true) >= 0))
						{
							if(botClass == BARD) {
								if(IsEffectInSpell(AIspells[i].spellid, SE_MovementSpeed) && !zone->CanCastOutdoor()) {
									break;
								}
							}
							int8 hpr = (int8)tar->GetHPRatio();
							if(hpr<= 80 || (tar->IsClient() && (hpr <= 99)) || (botClass == BARD))
							{
								if(tar->GetClass() == NECROMANCER) {
									// Necro bots use too much cleric mana with thier
									// mana for life spells... give them a chance
									// to lifetap something
									if(hpr > 60) {
										break;
									}
								}
								AIDoSpellCast(i, tar, mana_cost, &tar->pDontHealMeBefore);
								// If the healer is casting a HoT don't immediately cast the regular heal afterwards
								// The first HoT is at level 19 and is priority 1
								// The regular heal is priority 2
								// Let the HoT heal for at least 3 tics before checking for the regular heal
								// For non-HoT heals, do a 5 second delay
								if((botClass == CLERIC || botClass == PALADIN) && (botLevel >= 19) && (BotGetSpellPriority(i) == 1)) {
									tar->pDontHealMeBefore = (Timer::GetCurrentTime() + 18000);
								}
								else if((botClass == CLERIC || botClass == PALADIN) && (botLevel >= 19) && (BotGetSpellPriority(i) == 2)) {
									tar->pDontHealMeBefore = (Timer::GetCurrentTime() + 5000);
								}
								return true;
							}
						}
						break;
					}
					case SpellType_Root: {
						if (
							!tar->IsRooted() 
							&& tar->DontRootMeBefore() < Timer::GetCurrentTime()
							&& tar->CanBuffStack(AIspells[i].spellid, botLevel, true) >= 0
							) {
							if(!checked_los) {
								if(!CheckLosFN(tar))
									return(false);	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
								checked_los = true;
							}
							AIDoSpellCast(i, tar, mana_cost, &tar->pDontRootMeBefore);
							return true;
						}
						break;
					}
					case SpellType_Buff: {
						if (
							(spells[AIspells[i].spellid].targettype == ST_Target || tar == this)
							&& tar->DontBuffMeBefore() < Timer::GetCurrentTime()
							&& !tar->IsImmuneToSpell(AIspells[i].spellid, this)
							&& (tar->CanBuffStack(AIspells[i].spellid, botLevel, true) >= 0)
							&&  !(tar->IsPet() && tar->GetOwner()->IsClient() && this != tar)	//no buffing PC's pets, but they can buff themself

							) {
								// Put the zone levitate and movement check here since bots are able to bypass the client casting check
								if(	(IsEffectInSpell(AIspells[i].spellid, SE_Levitate) && !zone->CanLevitate()) ||
									(IsEffectInSpell(AIspells[i].spellid, SE_MovementSpeed) && !zone->CanCastOutdoor())) {
									break;
								}
								// when a pet class buffs its pet, it only needs to do it once
								if(spells[AIspells[i].spellid].targettype == ST_Pet) {
									Mob* newtar = GetPet();
									if(newtar) {
										if(!(newtar->CanBuffStack(AIspells[i].spellid, botLevel, true) >= 0)) {
											break;
										}
									}
									else {
										break;
									}
								}
								AIDoSpellCast(i, tar, mana_cost, &tar->pDontBuffMeBefore);
								return true;
						}
						break;
					}
					case SpellType_Escape: {
						int8 hpr = (int8)GetHPRatio();
	                #ifdef IPC          
                        if (hpr <= 5 || (IsNPC() && CastToNPC()->IsInteractive() && tar != this) )
					#else
                        if ((hpr <= 15) && (tar == this))
                    #endif
                    	{
                            AIDoSpellCast(i, this, mana_cost);
							return true;
						}
						break;
					}
					case SpellType_Nuke: {
						if (
							((MakeRandomInt(1, 100) < 50) || (botClass == BARD))
							&& ((tar->GetHPRatio()<=90.0f)||(!IsBotRaiding()))
							&& !tar->IsImmuneToSpell(AIspells[i].spellid, this)
							&& tar->CanBuffStack(AIspells[i].spellid, botLevel, true) >= 0
							) {
							if(!checked_los) {
								if(!CheckLosFN(tar))
									return(false);	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
								checked_los = true;
							}
							AIDoSpellCast(i, tar, mana_cost);
							return true;
						}
						break;
					}
					case SpellType_Dispel: {
						if(MakeRandomInt(1, 100) < 15)
						{
							if(!checked_los) {
								if(!CheckLosFN(tar))
									return(false);	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
								checked_los = true;
							}
							if(tar->CountDispellableBuffs() > 0)
							{
								AIDoSpellCast(i, tar, mana_cost);
								return true;
							}
						}
						break;
					}
					case SpellType_Pet: {
						 //keep mobs from recasting pets when they have them.
						if (!IsPet() && !GetPetID()) {
							if(botClass == MAGICIAN) {
								// have the magician bot randomly summon
								// the air, earth, fire or water pet
                                // include monster summoning after they
                                // become level 30 magicians
								int randpets;
								if(botLevel >= 30) {
									randpets = 4;
								}
								else {
									randpets = 3;
								}
								i = MakeRandomInt(i-randpets, i);
							}
							AIDoSpellCast(i, tar, mana_cost);
							return true;
						}
						break;
					}
					case SpellType_Lifetap: {
						if (
							((tar->GetHPRatio()<=80.0f)||(!IsBotRaiding()))
							&& !tar->IsImmuneToSpell(AIspells[i].spellid, this)
							&& tar->CanBuffStack(AIspells[i].spellid, botLevel, true) >= 0
							) {
							if(!checked_los) {
								if(!CheckLosFN(tar))
									return(false);	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
								checked_los = true;
							}
							AIDoSpellCast(i, tar, mana_cost);
							return true;
						}
						break;
					}
					case SpellType_Snare: {
						if (
							!tar->IsRooted()
							&& !tar->IsImmuneToSpell(AIspells[i].spellid, this)
							&& tar->DontSnareMeBefore() < Timer::GetCurrentTime()
							&& tar->CanBuffStack(AIspells[i].spellid, botLevel, true) >= 0
							) {
							if(!checked_los) {
								if(!CheckLosFN(tar))
									return(false);	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
								checked_los = true;
							}
							AIDoSpellCast(i, tar, mana_cost, &tar->pDontSnareMeBefore);
							return true;
						}
						break;
					}
					case SpellType_DOT: {
						if (
							((tar->GetHPRatio()<=80.0f)||(!IsBotRaiding()))
							&& !tar->IsImmuneToSpell(AIspells[i].spellid, this)
							&& tar->DontDotMeBefore() < Timer::GetCurrentTime()
							&& tar->CanBuffStack(AIspells[i].spellid, botLevel, true) >= 0
							) {
							if(!checked_los) {
								if(!CheckLosFN(tar))
									return(false);	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
								checked_los = true;
							}
							AIDoSpellCast(i, tar, mana_cost, &tar->pDontDotMeBefore);
							return true;
						}
						break;
					}
					default: {
						cout<<"Error: Unknown spell type in AICastSpell. caster:"<<this->GetName()<<" type:"<<AIspells[i].type<<" slot:"<<i<<endl;
						break;
					}
				}
			}
			if(extraMana) {
				// If the bot is just looping through spells and not casting
				// then don't let them keep the extra mana we gave them during
                // buff time
				SetMana(hasMana);
				extraMana = false;
			}
		}
	}
	return false;
}

bool NPC::Bot_AI_PursueCastCheck() {
	if (AIautocastspell_timer->Check(false)) {
		_ZP(Bot_AI_Process_pursue_cast);
		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.
		
		mlog(AI__SPELLS, "Bot Engaged (pursuing) autocast check triggered. Trying to cast offensive spells.");
		if(!Bot_AICastSpell(target, 90, SpellType_Root | SpellType_Nuke | SpellType_Lifetap | SpellType_Snare | SpellType_DOT | SpellType_Dispel)) {
			//no spell cast, try again soon.
			AIautocastspell_timer->Start(RandomTimer(500, 2000), false);
		} //else, spell casting finishing will reset the timer.
		return(true);
	}
	return(false);
}

bool NPC::Bot_Command_Cure(int curetype, int level) {
	int cureid = 0;
	switch(curetype) {
		case 1: // Poison
			if(level >= 58) {
				cureid = 1525;
			}
			else if(level >= 48) {
				cureid = 97;
			}
			else if(level >= 22) {
				cureid = 95;
			}
			else if(level >= 1) {
				cureid = 203;
			}
			break;
		case 2: // Disease
			if(level >= 51) {
				cureid = 3693;
			}
			else if(level >= 28) {
				cureid = 96;
			}
			else if(level >= 4) {
				cureid = 213;
			}
			break;
		case 3: // Curse
			if(level >= 54) {
				cureid = 2880;
			}
			else if(level >= 38) {
				cureid = 2946;
			}
			else if(level >= 23) {
				cureid = 4057;
			}
			else if(level >= 8) {
				cureid = 4056;
			}
			break;
		case 4: // Blindness
			if(level >= 3) {
				cureid = 212;
			}
			break;
	}
	if(cureid > 0) {
		if(IsBotRaiding()) {
			BotRaids* br = entity_list.GetBotRaidByMob(this);
			if(br) {
				for(int i=0; i<MAX_BOT_RAID_GROUPS; i++) {
					Group* gr = br->BotRaidGroups[i];
					if(gr) {
						for(int j=0; j<MAX_GROUP_MEMBERS; j++) {
							if(gr->members[j]) {
								CastSpell(cureid, gr->members[j]->GetID(), 1, -1, -1, &gr->members[j]->pDontHealMeBefore);
							}
						}
					}
				}
			}
		}
		else {
			Group* g = entity_list.GetGroupByMob(this);
			if(g) {
				for(int k=0; k<MAX_GROUP_MEMBERS; k++) {
					if(g->members[k]) {
						CastSpell(cureid, g->members[k]->GetID(), 1, -1, -1, &g->members[k]->pDontHealMeBefore);
					}
				}
				return true;
			}
		}
	}
	return false;
}

bool NPC::Bot_Command_MezzTarget(Mob *target) {
	if(target) {
		int mezid = 0;
		int mezlevel = GetLevel();
		if(mezlevel >= 69) {
			mezid = 5520;
		}
		else if(mezlevel == 68) {
			mezid = 8035;
		}
		else if(mezlevel == 67) {
			mezid = 5503;
		}
		else if(mezlevel >= 64) {
			mezid = 3358;
		}
		else if(mezlevel == 63) {
			mezid = 3354;
		}
		else if(mezlevel >= 61) {
			mezid = 3341;
		}
		else if(mezlevel == 60) {
			mezid = 2120;
		}
		else if(mezlevel == 59) {
			mezid = 1692;
		}
		else if(mezlevel >= 54) {
			mezid = 1691;
		}
		else if(mezlevel >= 47) {
			mezid = 190;
		}
		else if(mezlevel >= 30) {
			mezid = 188;
		}
		else if(mezlevel >= 13) {
			mezid = 187;
		}
		else if(mezlevel >= 2) {
			mezid = 292;
		}
		if(mezid > 0) {
			CastSpell(mezid, target->GetID(), 1, -1, -1, &target->pDontRootMeBefore);
			return true;
		}
	}
	return false;
}

bool NPC::Bot_Command_RezzTarget(Mob *target) {
	if(target) {
		int rezid = 0;
		int rezlevel = GetLevel();
		if(rezlevel >= 56) {
			rezid = 1524;
		}
		else if(rezlevel == 47) {
			rezid = 392;
		}
		else if(rezlevel == 42) {
			rezid = 2172;
		}
		else if(rezlevel >= 37) {
			rezid = 388;
		}
		else if(rezlevel >= 32) {
			rezid = 2171;
		}
		else if(rezlevel >= 27) {
			rezid = 391;
		}
		else if(rezlevel >= 22) {
			rezid = 2170;
		}
		else if(rezlevel >= 18) {
			rezid = 2169;
		}
		if(rezid > 0) {
			CastSpell(rezid, target->GetID(), 1, -1, -1, &target->pDontRootMeBefore);
			return true;
		}
	}
	return false;
}
void EntityList::ShowSpawnWindow(Client* client, int Distance, bool NamedOnly) {

	const char *WindowTitle = "Bot Tracking Window";

	string WindowText;
	int LastCon = -1;
	int CurrentCon = 0;
	
	int32 array_counter = 0;
	
	LinkedListIterator<Mob*> iterator(mob_list);
	iterator.Reset();

	while(iterator.MoreElements())
	{
		if (iterator.GetData() && (iterator.GetData()->DistNoZ(*client)<=Distance))
		{
			if(iterator.GetData()->IsTrackable()) {  //angelox
				Mob* cur_entity = iterator.GetData();
				const char *const MyArray[] = {
					"a_","an_","Innkeep_","Barkeep_",
					"Guard_","Merchant_","Lieutenant_",
					"Banker_","Centaur_","Aviak_","Baker_",
					"Sir_","Armorer_","Deathfist_","Deputy_",
					"Sentry_","Sentinel_","Leatherfoot_",
					"Corporal_","goblin_","Bouncer_","Captain_",
					"orc_","fire_","inferno_","young_","cinder_",
					"flame_","gnomish_","CWG_","sonic_","greater_",
					"ice_","dry_","Priest_","dark-boned_",
					"Tentacle_","Basher_","Dar_","Greenblood_",
					"clockwork_","guide_","rogue_","minotaur_",
					"brownie_","Teir'","dark_","tormented_",
					"mortuary_","lesser_","giant_","infected_",
					"wharf_","Apprentice_","Scout_","Recruit_",
					"Spiritist_","Pit_","Royal_","scalebone_",
					"carrion_","Crusader_","Trooper_","hunter_",
					"decaying_","iksar_","klok_","templar_","lord_",
					"froglok_","war_","large_","charbone_","icebone_",
					"Vicar_","Cavalier_","Heretic_","Reaver_","venomous_",
					"Sheildbearer_","pond_","mountain_","plaguebone_","Brother_",
					"great_","strathbone_","briarweb_","strathbone_","skeletal_",
					"minion_","spectral_","myconid_","spurbone_","sabretooth_",
					"Tin_","Iron_","Erollisi_","Petrifier_","Burynai_",
					"undead_","decayed_","You_","smoldering_","gyrating_",
					"lumpy_","Marshal_","Sheriff_","Chief_","Risen_",
					"lascar_","tribal_","fungi_","Xi_","Legionnaire_",
					"Centurion_","Zun_","Diabo_","Scribe_","Defender_","Capt_",
					"_",""
				};
				unsigned int MyArraySize;
				 for ( MyArraySize = 0; true; MyArraySize++) {   //Find empty string & get size
				   if (!(*(MyArray[MyArraySize]))) break;   //Checks for null char in 1st pos
				};
				if (NamedOnly) {
				   bool ContinueFlag = false;
				   const char *CurEntityName = cur_entity->GetName();  //Call function once
				   for (int Index = 0; Index < MyArraySize; Index++) {
				      if (!strncasecmp(CurEntityName, MyArray[Index], strlen(MyArray[Index]))) {
				         iterator.Advance();
				         ContinueFlag = true;
				         break;   //From Index for
				       };
				   };
				  if (ContinueFlag) continue; //Moved here or would apply to Index for
				};

				CurrentCon = client->GetLevelCon(cur_entity->GetLevel());
				if(CurrentCon != LastCon) {

					if(LastCon != -1)
						WindowText += "</c>";

					LastCon = CurrentCon;

					switch(CurrentCon) {

						case CON_GREEN: {
							WindowText += "<c \"#00FF00\">";
							break;
						}

						case CON_LIGHTBLUE: {
							WindowText += "<c \"#8080FF\">";
							break;
						}
						case CON_BLUE: {
							WindowText += "<c \"#2020FF\">";
							break;
						}

						case CON_YELLOW: {
							WindowText += "<c \"#FFFF00\">";
							break;
						}
						case CON_RED: {
							WindowText += "<c \"#FF0000\">";
							break;
						}
						default: {
							WindowText += "<c \"#FFFFFF\">";
							break;
						}
					}
				}

				WindowText += cur_entity->GetCleanName();
				WindowText += "<br>";

				if(strlen(WindowText.c_str()) > 4000) {
					// Popup window is limited to 4096 characters.
					WindowText += "</c><br><br>List truncated ... too many mobs to display";
					break;
				}
			}
		}

		iterator.Advance();
	}
	WindowText += "</c>";

	client->SendPopupToClient(WindowTitle, WindowText.c_str());

	return; 
}

bool NPC::Bot_Command_CalmTarget(Mob *target) {
	if(target) {
		int calmid = 0;
		int calmlevel = GetLevel();
		if((calmlevel >= 67) && (calmlevel <= 75)) {
			calmid = 5274;
		}
		else if((calmlevel >= 62) && (calmlevel <= 66)) {
			calmid = 3197;
		}
		else if((calmlevel >= 35) && (calmlevel <= 61)) {
			calmid = 45;
		}
		else if((calmlevel >= 18) && (calmlevel <= 34)) {
			calmid = 47;
		}
		else if((calmlevel >= 6) && (calmlevel <= 17)) {
			calmid = 501;
		}
		else if((calmlevel >= 1) && (calmlevel <= 5)) {
			calmid = 208;
		}
		if(calmid > 0) {
			CastSpell(calmid, target->GetID(), 1, -1, -1, &target->pDontRootMeBefore);
			return true;
		}
	}
	return false;
}
#endif //EQBOTS

