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
#include "../common/rulesys.h"
#include "features.h"

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
#define ABS(x) ((x)<0?-(x):(x))

//NOTE: do NOT pass in beneficial and detrimental spell types into the same call here!
bool NPC::AICastSpell(Mob* tar, int8 iChance, int16 iSpellTypes) {
	_ZP(Mob_AICastSpell);
// Faction isnt checked here, it's assumed you wouldnt pass a spell type you wouldnt want casted on the mob
	if (!tar)
		return false;
	if (iChance < 100) {
		if (MakeRandomInt(0, 100) >= iChance)
			return false;
	}
		
	float dist2;

	if (iSpellTypes & SpellType_Escape) {
	    dist2 = 0; //DistNoRoot(*this);	//WTF was up with this...
    } else 
	    dist2 = DistNoRoot(*tar);

	bool checked_los = false;	//we do not check LOS until we are absolutely sure we need to, and we only do it once.
	
	float manaR = GetManaRatio();
//	for (int i=0; i<MAX_AISPELLS; i++) {
	for (int i=MAX_AISPELLS-1; i >= 0; i--) {
		if (AIspells[i].spellid <= 0 || AIspells[i].spellid >= SPDAT_RECORDS) {
			// this is both to quit early to save cpu and to avoid casting bad spells
			// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
			//return false;
			continue;
		}
		if (iSpellTypes & AIspells[i].type) {
			// manacost has special values, -1 is no mana cost, -2 is instant cast (no mana)
			sint32 mana_cost = AIspells[i].manacost;
			if (mana_cost == -1)
				mana_cost = spells[AIspells[i].spellid].mana;
			else if (mana_cost == -2)
				mana_cost = 0;
			if (
				((
					(spells[AIspells[i].spellid].targettype==ST_AECaster || spells[AIspells[i].spellid].targettype==ST_AEBard)
					&& dist2 <= spells[AIspells[i].spellid].aoerange*spells[AIspells[i].spellid].aoerange
				 ) ||
				 dist2 <= spells[AIspells[i].spellid].range*spells[AIspells[i].spellid].range
				)
				&& (mana_cost <= GetMana() || GetMana() == GetMaxMana())
				&& (AIspells[i].time_cancast+(rand()%5)) <= Timer::GetCurrentTime() //break up the spelling casting over a period of time.
				) {

#if MobAI_DEBUG_Spells >= 21
				cout << "Mob::AICastSpell: Casting: spellid=" << AIspells[i].spellid
                    << ", tar=" << tar->GetName() 
                    << ", dist2[" << dist2 << "]<=" << spells[AIspells[i].spellid].range *spells[AIspells[i].spellid].range 
                    << ", mana_cost[" << mana_cost << "]<=" << GetMana() 
                    << ", cancast[" << AIspells[i].time_cancast << "]<=" << Timer::GetCurrentTime()
                    << ", type=" << AIspells[i].type << endl;
#endif

				switch (AIspells[i].type) {
					case SpellType_Heal: {
						if (
							(spells[AIspells[i].spellid].targettype == ST_Target || tar == this)
							&& tar->DontHealMeBefore() < Timer::GetCurrentTime()
							&& !(tar->IsPet() && tar->GetOwner()->IsClient())	//no buffing PC's pets
							) {
							int8 hpr = (int8)tar->GetHPRatio();
							if (
								hpr <= 35 
								|| (!IsEngaged() && hpr <= 50)
								|| (tar->IsClient() && hpr <= 99)
								) {
								AIDoSpellCast(i, tar, mana_cost, &tar->pDontHealMeBefore);
								return true;
							}
						}
						break;
					}
					case SpellType_Root: {
						if (
							!tar->IsRooted() 
							&& dist2 >= 900 
							&& MakeRandomInt(0, 99) < 50
							&& tar->DontRootMeBefore() < Timer::GetCurrentTime()
							&& tar->CanBuffStack(AIspells[i].spellid, GetLevel(), true) >= 0
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
							&& tar->CanBuffStack(AIspells[i].spellid, GetLevel(), true) >= 0
							&& !(tar->IsPet() && tar->GetOwner()->IsClient() && this != tar)	//no buffing PC's pets, but they can buff themself
							) {
							AIDoSpellCast(i, tar, mana_cost, &tar->pDontBuffMeBefore);
							return true;
						}
						break;
					}
					case SpellType_Escape: {
	                #ifdef IPC          
                        if (GetHPRatio() <= 5 || (IsNPC() && CastToNPC()->IsInteractive() && tar != this) )
					#else
                        if (GetHPRatio() <= 5 )	
                    #endif
                    	{
                            AIDoSpellCast(i, tar, mana_cost);
							return true;
						}
						break;
					}
					case SpellType_Nuke: {
						if (
							manaR >= 10 && (rand()%100) < 70
							&& tar->CanBuffStack(AIspells[i].spellid, GetLevel(), true) >= 0
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
						if(MakeRandomInt(0, 100) < 15)
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
						if (!IsPet() && !GetPetID() && MakeRandomInt(0, 99) < 25) {
							AIDoSpellCast(i, tar, mana_cost);
							return true;
						}
						break;
					}
					case SpellType_Lifetap: {
						if (   GetHPRatio() <= 95
							&& MakeRandomInt(0, 99) < 50
							&& tar->CanBuffStack(AIspells[i].spellid, GetLevel(), true) >= 0
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
							&& MakeRandomInt(0, 99) < 50
							&& tar->DontSnareMeBefore() < Timer::GetCurrentTime()
							&& tar->CanBuffStack(AIspells[i].spellid, GetLevel(), true) >= 0
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
							MakeRandomInt(0, 99) < 60
							&& tar->DontDotMeBefore() < Timer::GetCurrentTime()
							&& tar->CanBuffStack(AIspells[i].spellid, GetLevel(), true) >= 0
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
#if MobAI_DEBUG_Spells >= 21
			else {
				cout << "Mob::AICastSpell: NotCasting: spellid=" << AIspells[i].spellid << ", tar=" << tar->GetName() << ", dist2[" << dist2 << "]<=" << spells[AIspells[i].spellid].range*spells[AIspells[i].spellid].range << ", mana_cost[" << mana_cost << "]<=" << GetMana() << ", cancast[" << AIspells[i].time_cancast << "]<=" << Timer::GetCurrentTime() << endl;
			}
#endif
		}
	}
	return false;
}

void NPC::AIDoSpellCast(int8 i, Mob* tar, sint32 mana_cost, int32* oDontDoAgainBefore) {
#if MobAI_DEBUG_Spells >= 1
	cout << "Mob::AIDoSpellCast: spellid=" << AIspells[i].spellid << ", tar=" << tar->GetName() << ", mana=" << mana_cost << ", Name: " << spells[AIspells[i].spellid].name << endl;
#endif
	casting_spell_AIindex = i;
	
	//stop moving if were casting a spell and were not a bard...
	if(!IsBardSong(AIspells[i].spellid)) {
		SetRunAnimSpeed(0);
		SendPosition();
		SetMoving(false);
	}
	
	if(		AIspells[i].type == SpellType_Nuke 
	   ||	AIspells[i].type == SpellType_Root
	   ||	AIspells[i].type == SpellType_Lifetap
	   ||	AIspells[i].type == SpellType_Snare
	   ||	AIspells[i].type == SpellType_DOT
	   ||	AIspells[i].type == SpellType_Dispel
	  ) {
		//we are attacking somebody, handle event_combat
		if(!combat_event) {
			mlog(COMBAT__HITS, "Triggering EVENT_COMBAT due to spell index %d (id %d) of type %d on %s", i, AIspells[i].spellid, AIspells[i].type, tar->GetName());
			parse->Event(EVENT_COMBAT, this->GetNPCTypeID(), "1", this, tar);
			combat_event = true;
		}
		combat_event_timer.Start(CombatEventTimer_expire);
	}
	
	CastSpell(AIspells[i].spellid, tar->GetID(), 1, AIspells[i].manacost == -2 ? 0 : -1, mana_cost, oDontDoAgainBefore);
}

bool EntityList::AICheckCloseBeneficialSpells(NPC* caster, int8 iChance, float iRange, int16 iSpellTypes) {
	_ZP(EntityList_AICheckCloseBeneficialSpells);

	if((iSpellTypes&SpellTypes_Detrimental) != 0) {
		//according to live, you can buff and heal through walls...
		//now with PCs, this only applies if you can TARGET the target, but
		// according to Rogean, Live NPCs will just cast through walls/floors, no problem..
		//
		// This check was put in to address an idle-mob CPU issue
		_log(AI__ERROR, "Error: detrimental spells requested from AICheckCloseBeneficialSpells!!");
		return(false);
	}
	
	if (iChance < 100) {
		int8 tmp = MakeRandomInt(0, 99);
		if (tmp >= iChance)
			return false;
	}
	if (caster->GetPrimaryFaction() == 0 )
		return(false); // well, if we dont have a faction set, we're gonna be indiff to everybody

	float iRange2 = iRange*iRange;
	
	float t1, t2, t3;
	

	//Only iterate through NPCs
    LinkedListIterator<NPC*> iterator(npc_list);
    for(iterator.Reset(); iterator.MoreElements(); iterator.Advance()) {
		NPC* mob = iterator.GetData();
		
		//Since >90% of mobs will always be out of range, try to
		//catch them with simple bounding box checks first. These
		//checks are about 6X faster than DistNoRoot on my athlon 1Ghz
		t1 = mob->GetX() - caster->GetX();
		t2 = mob->GetY() - caster->GetY();
		t3 = mob->GetZ() - caster->GetZ();
		//cheap ABS()
		if(t1 < 0)
			t1 = 0 - t1;
		if(t2 < 0)
			t2 = 0 - t2;
		if(t3 < 0)
			t3 = 0 - t3;
		if (   t1 > iRange
			|| t2 > iRange
			|| t3 > iRange
			|| mob->DistNoRoot(*caster) > iRange2
				//this call should seem backwards:
			|| mob->GetReverseFactionCon(caster) >= FACTION_KINDLY
		) {
			continue;
		}

		//since we assume these are beneficial spells, which do not
		//require LOS, we just go for it.
		// we have a winner!
		if((iSpellTypes & SpellType_Buff) && !RuleB(NPC, BuffFriends)){
			if (mob != caster)
				iSpellTypes = SpellType_Heal;
		}

		if (caster->AICastSpell(mob, 100, iSpellTypes))
			return true;
	}
	return false;
}

void Mob::AI_Init() {
	pAIControlled = false;
	AIthink_timer = 0;
	AIwalking_timer = 0;
	AImovement_timer = 0;
	AIfeignremember_timer = NULL;
	AIscanarea_timer = 0;
	pLastFightingDelayMoving = 0;
	minLastFightingDelayMoving = 10000;
	maxLastFightingDelayMoving = 20000;

	pDontHealMeBefore = 0;
	pDontBuffMeBefore = 0;
	pDontDotMeBefore = 0;
	pDontRootMeBefore = 0;
	pDontSnareMeBefore = 0;
}

void NPC::AI_Init() {
	Mob::AI_Init();
	
	AIautocastspell_timer = 0;
	casting_spell_AIindex = MAX_AISPELLS;

	roambox_max_x = 0;
	roambox_max_y = 0;
	roambox_min_x = 0;
	roambox_min_y = 0;
	roambox_distance = 0;
	roambox_movingto_x = 0;
	roambox_movingto_y = 0;
	roambox_delay = 2500;
}

void Client::AI_Init() {
	Mob::AI_Init();
	minLastFightingDelayMoving = CLIENT_LD_TIMEOUT;
	maxLastFightingDelayMoving = CLIENT_LD_TIMEOUT;
}

void Mob::AI_Start(int32 iMoveDelay) {
	if (iMoveDelay)
		pLastFightingDelayMoving = Timer::GetCurrentTime() + iMoveDelay;
	else
		pLastFightingDelayMoving = 0;
	if (pAIControlled)
		return;
	pAIControlled = true;
	AIthink_timer = new Timer(AIthink_duration);
	AIthink_timer->Trigger();
	AIwalking_timer = new Timer(0);
	AImovement_timer = new Timer(AImovement_duration);
	AIfeignremember_timer = new Timer(AIfeignremember_delay);
	AIscanarea_timer = new Timer(AIscanarea_delay);
#ifdef REVERSE_AGGRO
	if(IsNPC() && !CastToNPC()->WillAggroNPCs())
		AIscanarea_timer->Disable();
#endif

	if (GetAggroRange() == 0)
		pAggroRange = 70;
	if (GetAssistRange() == 0)
		pAssistRange = 70;
	hate_list.Wipe();

	delta_heading = 0;
	delta_x = 0;
	delta_y = 0;
	delta_z = 0;
	pRunAnimSpeed = 0;
	pLastChange = Timer::GetCurrentTime();
}

void Client::AI_Start(int32 iMoveDelay) {
	Mob::AI_Start(iMoveDelay);
	if (!pAIControlled)
		return;
	// copy memed spells to the spells struct here
	this->Message_StringID(13,PLAYER_CHARMED);
/*	EQApplicationPacket *app = new EQApplicationPacket(OP_Charm, sizeof(Charm_Struct));
	Charm_Struct *ps = (Charm_Struct*)app->pBuffer;
	ps->owner_id = GetOwnerOrSelf()->GetID();
	ps->pet_id = this->GetID();
	ps->command = 1;
	FastQueuePacket(&app);*/
	Group* group = GetGroup();
	if (this->isgrouped && group != NULL)
    {
		group->DelMember(CastToMob(),true);
    }
	
//	SaveSpawnSpot();
	pClientSideTarget = target ? target->GetID() : 0;
	SendAppearancePacket(AT_Anim, ANIM_FREEZE);	// this freezes the client
	SendAppearancePacket(AT_Linkdead, 1); // Sending LD packet so *LD* appears by the player name when charmed/feared -Kasai
	SetAttackTimer();
}

void NPC::AI_Start(int32 iMoveDelay) {
	Mob::AI_Start(iMoveDelay);
	if (!pAIControlled)
		return;
	
	if (AIspells[0].spellid == 0 || AIspells[0].spellid == SPELL_UNKNOWN) {
		AIautocastspell_timer = new Timer(1000);
		AIautocastspell_timer->Disable();
	} else {
		AIautocastspell_timer = new Timer(750);
		AIautocastspell_timer->Start(RandomTimer(0, 15000), false);
	}
	
	if (NPCTypedata) {
		AI_AddNPCSpells(NPCTypedata->npc_spells_id);
		NPCSpecialAttacks(NPCTypedata->npc_attacks,0);
	}
	
	SendTo(GetX(), GetY(), GetZ());
	SetChanged();
//	SaveSpawnSpot();
	SaveGuardSpot();
}

void Mob::AI_Stop() {
	if (!IsAIControlled())
		return;
	pAIControlled = false;
	safe_delete(AIthink_timer);
	safe_delete(AIwalking_timer);
	safe_delete(AImovement_timer);
	safe_delete(AIscanarea_timer);
	safe_delete(AIfeignremember_timer);
	hate_list.Wipe();
}

void NPC::AI_Stop() {

#ifdef EQBOTS

	if(IsBot()) {
		Mob::AI_Stop();					// jadams: Unsure if this is bot code? No comment
	}

#endif //EQBOTS

	Waypoints.clear();
	safe_delete(AIautocastspell_timer);
}

void Client::AI_Stop() {
	Mob::AI_Stop();
	this->Message_StringID(13,PLAYER_REGAIN);
	EQApplicationPacket *app = new EQApplicationPacket(OP_Charm, sizeof(Charm_Struct));
	Charm_Struct *ps = (Charm_Struct*)app->pBuffer;
	ps->owner_id = 0;
	ps->pet_id = this->GetID();
	ps->command = 0;
	FastQueuePacket(&app);
	SetTarget(entity_list.GetMob(pClientSideTarget));
	SendAppearancePacket(AT_Anim, GetAppearanceValue(GetAppearance()));
	SendAppearancePacket(AT_Linkdead, 0); // Removing LD packet so *LD* no longer appears by the player name when charmed/feared -Kasai
	if (!auto_attack) {
		attack_timer.Disable();
		attack_dw_timer.Disable();
	}
	if (IsLD())
	{
		Save();
		Disconnect();
	}
}

void Mob::AI_Process() {
	_ZP(Mob_AI_Process);
	

	if (!IsAIControlled())
		return;

	if (!(AIthink_timer->Check() || attack_timer.Check(false)))
		return;

	if (IsCasting())
		return;
	
	bool engaged = IsEngaged();

	// Begin: Additions for Wiz Fear Code
	//
	if(RuleB(Combat, EnableFearPathing)){
		if(curfp) {
			if(IsRooted()) {
				//make sure everybody knows were not moving, for appearance sake
				if(IsMoving())
				{
					if(target)
						SetHeading(CalculateHeadingToTarget(target->GetX(), target->GetY()));
					SetRunAnimSpeed(0);
					SendPosition();
					SetMoving(false);
					moved=false;
				}
				//continue on to attack code, ensuring that we execute the engaged code
				engaged = true;
			} else {
				if(AImovement_timer->Check()) {
					// Check if we have reached the last fear point
					if((ABS(GetX()-fear_walkto_x) < 0.1) && (ABS(GetY()-fear_walkto_y) <0.1)) {
						// Calculate a new point to run to
						CalculateNewFearpoint();
					}
					CalculateNewPosition2(fear_walkto_x, fear_walkto_y, fear_walkto_z, GetFearSpeed(), true);
				}
				return;
			}
		}
	}
	
	// trigger EVENT_SIGNAL if required
	if(IsNPC()) {
		CastToNPC()->CheckSignal();
	}
	
	if (engaged) 
	{
		_ZP(Mob_AI_Process_engaged);
		if (IsRooted())
			SetTarget(hate_list.GetClosest(this));
		else
			SetTarget(hate_list.GetTop(this));

		if (!target)
			return;
		
		if(DivineAura())
			return;

		if (GetHPRatio() < 15)
			StartEnrage();
		
		bool is_combat_range = CombatRange(target);
		
	
        if (is_combat_range) 
        {
			if (AImovement_timer->Check()) 
			{
				SetRunAnimSpeed(0);
			}
			if(IsMoving())
			{
				SetMoving(false);
				moved=false;
				SetHeading(CalculateHeadingToTarget(target->GetX(), target->GetY()));
				/*while(DistNoZ(*target)<10){ //dont want them too close
					x_pos -= tar_vx*.2;
					y_pos -= tar_vy*.2;
					z_pos -= tar_vz*.2;
				}*/
				SendPosition();
				tar_ndx =0;
			}
			
			//casting checked above...
			if(target && !IsStunned() && !IsMezzed() && GetAppearance() != eaDead) {
				
				//we should check to see if they die mid-attacks, previous
				//crap of checking target for null was not gunna cut it
				
				//try main hand first
				if(attack_timer.Check()) {
					Attack(target, 13);
					if (target) 
					{
						//we use this random value in three comparisons with different
						//thresholds, and if its truely random, then this should work
						//out reasonably and will save us compute resources.
						sint32 RandRoll = MakeRandomInt(0, 99);
						if (CanThisClassDoubleAttack()
							//check double attack, this is NOT the same rules that clients use...
							&& RandRoll < (GetLevel() + NPCDualAttackModifier))  
						{
							if (Attack(target, 13)) 
							{
								// lets see if we can do a triple attack with the main hand
								//pets are excluded from triple and quads...
								if (SpecAttacks[SPECATK_TRIPLE]
									&& !IsPet() && RandRoll < (GetLevel()+NPCTripleAttackModifier))
								{
									if (Attack(target, 13)) 
									{	// now lets check the quad attack
										if (SpecAttacks[SPECATK_QUAD]
											&& RandRoll < (GetLevel() + NPCQuadAttackModifier))  
										{
											Attack(target, 13);
										} // if (SpecAttacks[SPECATK_QUAD])
									}
								} // if (SpecAttacks[SPECATK_TRIPLE])
							}
						} // if (CanThisClassDoubleAttack())
					}
	
					if (SpecAttacks[SPECATK_FLURRY]) {
					    // perhaps get the values from the db?
					    if (MakeRandomInt(0, 99) < 20)
							Flurry();
					}
	
					if (SpecAttacks[SPECATK_RAMPAGE]) {
					    // perhaps get the values from the db?
					    if (MakeRandomInt(0, 99) < 20)
							Rampage();
					}
				}
				
				//now off hand
				if (attack_dw_timer.Check() && CanThisClassDualWield()) 
				{
					int myclass = GetClass();
					//can only dual weild without a weapon if your a monk
					if((GetEquipment(MATERIAL_SECONDARY) != 0 && GetLevel() > 39) || myclass == MONK || myclass == MONKGM) {
						float DualWieldProbability = (GetSkill(DUAL_WIELD) + GetLevel()) / 400.0f;
						DualWieldProbability -= MakeRandomFloat(0, 1);
						if(DualWieldProbability < 0){
							Attack(target, 14);
							if (CanThisClassDoubleAttack()) 
							{
								sint32 RandRoll = rand()%100;
								if (RandRoll < (GetLevel() + 20))  
								{
									if (Attack(target, 14));
								}
							} // if (CanThisClassDoubleAttack())
						}
					}
				}
				
				//now special attacks (kick, etc)
				if(IsNPC())
					CastToNPC()->DoClassAttacks(target);
			}
			AI_EngagedCastCheck();
		}	//end is within combat range
		else {
			//we cannot reach our target...
			// See if we can summon the mob to us
			if (!HateSummon()) 
			{
				//could not summon them, start pursuing...
// TODO: Check here for another person on hate list with close hate value
				if(AI_PursueCastCheck()){
					//we did something, so do not process movement.
				}
				else if (AImovement_timer->Check()) 
				{
					if(!IsRooted()) {
						mlog(AI__WAYPOINTS, "Pursuing %s while engaged.", target->GetName());
						CalculateNewPosition2(target->GetX(), target->GetY(), target->GetZ(), GetRunspeed());
					} else if(IsMoving()) {
						SetHeading(CalculateHeadingToTarget(target->GetX(), target->GetY()));
						SetRunAnimSpeed(0);
						SendPosition();
						SetMoving(false);
						moved=false;
					}
				}
			}
		}
	}
	else { // not engaged
		//if (pStandingPetOrder == SPO_Follow && IsPet() && !IsStunned())
			//SetHeading(CalculateHeadingToTarget(target->GetX(), target->GetY())*8);
			//FaceTarget(GetOwner(), true);
		
		if(AIfeignremember_timer->Check()) {
			// EverHood - 6/14/06
			// Improved Feign Death Memory
			// check to see if any of our previous feigned targets have gotten up.
			std::set<int32>::iterator RememberedCharID, tmp;
			RememberedCharID=feign_memory_list.begin();
			bool got_one = false;
			while(RememberedCharID != feign_memory_list.end()) {
				Client* remember_client = entity_list.GetClientByCharID(*RememberedCharID);
				if(remember_client == NULL) {
					//they are gone now...
					tmp = RememberedCharID;
					RememberedCharID++;
					feign_memory_list.erase(tmp);
				} else if (!remember_client->GetFeigned()) {
					AddToHateList(remember_client->CastToMob(),1);
					tmp = RememberedCharID;
					RememberedCharID++;
					feign_memory_list.erase(tmp);
					got_one = true;
					break;
				} else {
					//they are still feigned, carry on...
					RememberedCharID++;
				}
			}
		}
		if (AI_IdleCastCheck()) 
		{
			//we processed a spell action, so do nothing else.
		}
		else if (AIscanarea_timer->Check()) 
		{
			/*                                                                              
            * This is where NPCs look around to see if they want to attack anybody.
            *
            * if REVERSE_AGGRO is enabled, then this timer is disabled unless they
            * have the npc_aggro flag on them, and aggro against clients is checked
            * by the clients.
            *
            */
			_ZP(Mob_AI_Process_scanarea);
			
			Mob* tmptar = entity_list.AICheckCloseAggro(this, GetAggroRange(), GetAssistRange());
			if (tmptar) 
				AddToHateList(tmptar);
		}
		else if (AImovement_timer->Check() && !IsRooted()) 
		{
			_ZP(Mob_AI_Process_move);
			SetRunAnimSpeed(0);
			if (IsPet()) 
			{
				_ZP(Mob_AI_Process_pet);
				// we're a pet, do as we're told
				switch (pStandingPetOrder) 
				{
					case SPO_Follow: 
					{
						
						Mob* owner = GetOwner();
						if(owner == NULL)
							break;
						
						//if(owner->IsClient())
						//	printf("Pet start pos: (%f, %f, %f)\n", GetX(), GetY(), GetZ());
						
						float dist = DistNoRoot(*owner);
						if (dist >= 100) 
						{
							float speed = GetWalkspeed();
							if (dist >= 25)
								speed = GetRunspeed();
							CalculateNewPosition2(owner->GetX(), owner->GetY(), owner->GetZ(), speed);
						}
						else
						{
							SetHeading(owner->GetHeading());
							if(moved)
							{
								moved=false;
								SetMoving(false);
								SendPosition();
							}
						}
					
						/*
						//fix up Z
						float zdiff = GetZ() - owner->GetZ();
						if(zdiff < 0)
							zdiff = 0 - zdiff;
						if(zdiff > 2.0f) {
							SendTo(GetX(), GetY(), owner->GetZ());
							SendPosition();
						}
						
						if(owner->IsClient())
							printf("Pet pos: (%f, %f, %f)\n", GetX(), GetY(), GetZ());
						*/
						
						break;
					}
					case SPO_Sit: 
					{
						SetAppearance(eaSitting, false);
						break;
					}
					case SPO_Guard: 
					{
						//only NPCs can guard stuff. (forced by where the guard movement code is in the AI)
						if(IsNPC()) {
							CastToNPC()->NextGuardPosition();
						}
						break;
					}
				}
			}
			else if (GetFollowID()) 
			{
				Mob* follow = entity_list.GetMob(GetFollowID());
				if (!follow) SetFollowID(0);
				else 
				{
					float dist2 = DistNoRoot(*follow);
					if (dist2 >= 100) 
					{
						float speed = GetWalkspeed();
						if (dist2 >= 225)
							speed = GetRunspeed();
						CalculateNewPosition2(follow->GetX(), follow->GetY(), follow->GetZ(), speed);
					}
					else
					{
						if(moved)
						{
							SendPosition();
							moved=false;
							SetMoving(false);
						}
					}
					
					/*
					//fix up Z proble mssince CalculateNewPosition2 ignores pure-Z-movement now...
					float zdiff = GetZ() - follow->GetZ();
					if(zdiff < 0)
						zdiff = 0 - zdiff;
					if(zdiff > 2.0f) {
						SendTo(GetX(), GetY(), follow->GetZ());
						SendPosition();
					}
					
					if(follow->IsClient())
						printf("Follow pos: (%f, %f, %f)\n", GetX(), GetY(), GetZ());
					*/
					
				}
			}
			else //not a pet, and not following somebody...
			{
				// dont move till a bit after you last fought
				if (pLastFightingDelayMoving < Timer::GetCurrentTime()) 
				{
					if (this->IsClient()) 
					{
						// LD timer expired, drop out of world
						if (this->CastToClient()->IsLD())
							this->CastToClient()->Disconnect();
						return;
					}
					
					if(IsNPC())
						CastToNPC()->AI_DoMovement();
				}
				
         } 
      } // else if (AImovement_timer->Check()) 
   }
}

void NPC::AI_DoMovement() {
	float walksp = GetWalkspeed();
	if(walksp <= 0.0f)
		return;	//this is idle movement at walk speed, and we are unable to walk right now.
	
	if (roambox_distance > 0) {
		_ZP(Mob_AI_Process_roambox);
		if (
			roambox_movingto_x > roambox_max_x
			|| roambox_movingto_x < roambox_min_x
			|| roambox_movingto_y > roambox_max_y
			|| roambox_movingto_y < roambox_min_y
			) 
		{
			float movedist = roambox_distance*roambox_distance;
			float movex = MakeRandomFloat(0, movedist);
			float movey = movedist - movex;
			movex = sqrtf(movex);
			movey = sqrtf(movey);
//cout << "1: MoveDist: " << roambox_distance << " MoveX: " << movex << " MoveY: " << movey << " MaxX: " << roambox_max_x << " MinX: " << roambox_min_x << " MaxY: " << roambox_max_y << " MinY: " << roambox_min_y << endl;
			movex *= rand()%2 ? 1 : -1;
			movey *= rand()%2 ? 1 : -1;
			roambox_movingto_x = GetX() + movex;
			roambox_movingto_y = GetY() + movey;
//printf("Roambox: Moving to: %1.2f, %1.2f  Move: %1.2f, %1.2f\n", roambox_movingto_x, roambox_movingto_y, movex, movey);
//cout << "2: RoamBox: Moving to: " << roambox_movingto_x << ", " << roambox_movingto_y << "  Move: " << movex << ", " << movey << endl;
			if (roambox_movingto_x > roambox_max_x || roambox_movingto_x < roambox_min_x)
				roambox_movingto_x -= movex * 2;
			if (roambox_movingto_y > roambox_max_y || roambox_movingto_y < roambox_min_y)
				roambox_movingto_y -= movey * 2;
//cout << "3: RoamBox: Moving to: " << roambox_movingto_x << ", " << roambox_movingto_y << "  Move: " << movex << ", " << movey << endl;
			if (roambox_movingto_x > roambox_max_x || roambox_movingto_x < roambox_min_x)
				roambox_movingto_x = roambox_max_x;
			if (roambox_movingto_y > roambox_max_y || roambox_movingto_y < roambox_min_y)
				roambox_movingto_y = roambox_max_y;
//cout << "4: RoamBox: Moving to: " << roambox_movingto_x << ", " << roambox_movingto_y << "  Move: " << movex << ", " << movey << endl;
		}
		
		mlog(AI__WAYPOINTS, "Roam Box: d=%.3f (%.3f->%.3f,%.3f->%.3f): Go To (%.3f,%.3f)", 
			roambox_distance, roambox_min_x, roambox_max_x, roambox_min_y, roambox_max_y, roambox_movingto_x, roambox_movingto_y);
		if (!CalculateNewPosition2(roambox_movingto_x, roambox_movingto_y, GetZ(), walksp, true)) 
		{
			roambox_movingto_x = roambox_max_x + 1; // force update
			pLastFightingDelayMoving = Timer::GetCurrentTime() + RandomTimer(roambox_delay, roambox_delay + 5000);
			SetMoving(false);
			SendPosition();	// makes mobs stop clientside
		}
	}
	else if (roamer) 
	{
		_ZP(Mob_AI_Process_roamer);
		if (AIwalking_timer->Check())
		{
			movetimercompleted=true;
			AIwalking_timer->Disable();
		}

		
		sint16 gridno = CastToNPC()->GetGrid(); 

// handle quest command roamers with no grids too
		if (gridno > 0 || cur_wp==-2)  {
			if (movetimercompleted==true) {  // time to pause at wp is over
// MYRA - Added code to depop at end of grid for wander type 4
				if (wandertype == 4 && cur_wp == CastToNPC()->GetMaxWp()) {
		           CastToNPC()->Depop(); 
				} else {
					movetimercompleted=false; 
					
					mlog(QUESTS__PATHING, "We have reached waypoint %d.", cur_wp);
					
					//if we were under quest control (with no grid), we are done now..
					if(cur_wp == -2) {
						mlog(QUESTS__PATHING, "Non-grid quest mob has reached its quest ordered waypoint. Leaving pathing mode.");
						roamer = false;
						cur_wp = 0;
					}
					
					//not sure why we do this...
					SetAppearance(eaStanding, false);
					
					//kick off event_waypoint
					char temp[16]; 
					sprintf(temp, "%d", cur_wp);
					parse->Event(EVENT_WAYPOINT,this->GetNPCTypeID(), temp, CastToNPC(), NULL); 
					
					entity_list.OpenDoorsNear(CastToNPC());
					//setup our next waypoint, if we are still on our normal grid
					//remember that the quest event above could have done anything it wanted with our grid
					if(gridno > 0)
						CastToNPC()->CalculateNewWaypoint();
                } 
            }	// endif (movetimercompleted==true)     
			else if (!(AIwalking_timer->Enabled()))
			{	// currently moving
				if (cur_wp_x == GetX() && cur_wp_y == GetY()) 
				{	// are we there yet? then stop
					mlog(AI__WAYPOINTS, "We have reached waypoint %d (%.3f,%.3f,%.3f) on grid %d", cur_wp, GetX(), GetY(), GetZ(), GetGrid());
					SetWaypointPause();
					SetAppearance(eaStanding, false);
					SetMoving(false);
					SendPosition();
					
					// EverHood - wipe feign memory since we reached our first waypoint
					if(cur_wp == 1)
						ClearFeignMemory();
				} 
				else
				{	// not at waypoint yet, so keep moving
					CalculateNewPosition2(cur_wp_x, cur_wp_y, cur_wp_z, walksp, true); 
				}
			} 
		}		// endif (gridno > 0) 
// handle new quest grid command processing
		else if (gridno < 0) 
		{	// this mob is under quest control
			if (movetimercompleted==true)    
			{ // time to pause has ended
				SetGrid( 0 - GetGrid()); // revert to AI control
				mlog(QUESTS__PATHING, "Quest pathing is finished. Resuming on grid %d", GetGrid());
				SetAppearance(eaStanding, false); 
				CalculateNewWaypoint();
			}
		}

  } 
  else if (IsGuarding()) 
  {
	_ZP(Mob_AI_Process_guard);
     if (!CalculateNewPosition2(guard_x, guard_y, guard_z, walksp)) 
     {
		if(moved) {
			mlog(AI__WAYPOINTS, "Reached guard point (%.3f,%.3f,%.3f)", guard_x, guard_y, guard_z);
			ClearFeignMemory();
			moved=false;
			SetMoving(false);
			if (GetTarget() == NULL || DistNoRoot(*GetTarget()) >= 5*5 )
			{
				SetHeading(guard_heading); 
			} else { 
				FaceTarget(GetTarget(), true); 
			}
			SendPosition();			
		}
	 } 
  } 
}

// Note: Mob that caused this may not get added to the hate list until after this function call completes
void Mob::AI_Event_Engaged(Mob* attacker, bool iYellForHelp) {
	if (!IsAIControlled())
		return;
	if (iYellForHelp) {
		if(IsPet()) {
			GetOwner()->AI_Event_Engaged(attacker, iYellForHelp);
		} else {
			entity_list.AIYellForHelp(this, attacker);
		}
	}
}

// Note: Hate list may not be actually clear until after this function call completes
void Mob::AI_Event_NoLongerEngaged() {
	if (!IsAIControlled())
		return;
	this->AIwalking_timer->Start(RandomTimer(3000,20000));
	pLastFightingDelayMoving = Timer::GetCurrentTime();
	if (minLastFightingDelayMoving == maxLastFightingDelayMoving)
		pLastFightingDelayMoving += minLastFightingDelayMoving;
	else
		pLastFightingDelayMoving += (rand() % (maxLastFightingDelayMoving-minLastFightingDelayMoving)) + minLastFightingDelayMoving;
	// EverHood - So mobs don't keep running as a ghost until AIwalking_timer fires
	// if they were moving prior to losing all hate
	if(IsMoving()){
		SetRunAnimSpeed(0);
		SetMoving(false);
		SendPosition();
	}
	ClearRampage();
}

//this gets called from InterruptSpell() for failure or SpellFinished() for success
void NPC::AI_Event_SpellCastFinished(bool iCastSucceeded, int8 slot) {
	if (slot == 1) {
		int32 recovery_time = 0;
		if (iCastSucceeded) {
			if (casting_spell_AIindex < MAX_AISPELLS) {
					recovery_time += spells[AIspells[casting_spell_AIindex].spellid].recovery_time;
					if (AIspells[casting_spell_AIindex].recast_delay >= 0){
						if (AIspells[casting_spell_AIindex].recast_delay <10000)
							AIspells[casting_spell_AIindex].time_cancast = Timer::GetCurrentTime() + (AIspells[casting_spell_AIindex].recast_delay*1000);
}
					else
						AIspells[casting_spell_AIindex].time_cancast = Timer::GetCurrentTime() + spells[AIspells[casting_spell_AIindex].spellid].recast_time;
			}
			if (recovery_time < AIautocastspell_timer->GetSetAtTrigger())
				recovery_time = AIautocastspell_timer->GetSetAtTrigger();
			AIautocastspell_timer->Start(recovery_time, false);
		}
		else
			AIautocastspell_timer->Start(800, false);
		casting_spell_AIindex = MAX_AISPELLS;
	}
}


bool NPC::AI_EngagedCastCheck() {
	if (AIautocastspell_timer->Check(false)) {
		_ZP(Mob_AI_Process_engaged_cast);
		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.
		
		mlog(AI__SPELLS, "Engaged autocast check triggered. Trying to cast healing spells then maybe offensive spells.");
		
		// try casting a heal or gate
		if (!AICastSpell(this, 100, SpellType_Heal | SpellType_Escape)) {
			// try casting a heal on nearby
			if (!entity_list.AICheckCloseBeneficialSpells(this, 25, MobAISpellRange, SpellType_Heal)) {
				//nobody to heal, try some detrimental spells.
				if(!AICastSpell(target, 20, SpellType_Nuke | SpellType_Lifetap | SpellType_DOT | SpellType_Dispel)) {
					//no spell to cast, try again soon.
					AIautocastspell_timer->Start(RandomTimer(500, 1000), false);
				}
			} //else, spell casting finishing will reset the timer.
		}
		return(true);
	}
	
	return(false);
}

bool NPC::AI_PursueCastCheck() {
	if (AIautocastspell_timer->Check(false)) {
		_ZP(Mob_AI_Process_pursue_cast);
		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.
		
		mlog(AI__SPELLS, "Engaged (pursuing) autocast check triggered. Trying to cast offensive spells.");
		if(!AICastSpell(target, 90, SpellType_Root | SpellType_Nuke | SpellType_Lifetap | SpellType_Snare | SpellType_DOT | SpellType_Dispel)) {
			//no spell cast, try again soon.
			AIautocastspell_timer->Start(RandomTimer(500, 2000), false);
		} //else, spell casting finishing will reset the timer.
		return(true);
	}
	return(false);
}

bool NPC::AI_IdleCastCheck() {
	if (AIautocastspell_timer->Check(false)) {
		_ZP(Mob_AI_Process_autocast);
#if MobAI_DEBUG_Spells >= 25
		cout << "Non-Engaged autocast check triggered: " << this->GetName() << endl;
#endif
		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.
		if (!AICastSpell(this, 100, SpellType_Heal | SpellType_Buff | SpellType_Pet)) {
			if(!entity_list.AICheckCloseBeneficialSpells(this, 33, MobAISpellRange, SpellType_Heal | SpellType_Buff)) {
				//if we didnt cast any spells, our autocast timer just resets to the 
				//last duration it was set to... try to put up a more reasonable timer...
				AIautocastspell_timer->Start(RandomTimer(1000, 5000), false);
			}	//else, spell casting finishing will reset the timer.
		}	//else, spell casting finishing will reset the timer.
		return(true);
	}
	return(false);
}

void Mob::StartEnrage()
{
    // dont continue if already enraged
    if (bEnraged)
        return;
    if (SpecAttackTimers[SPECATK_ENRAGE] && !SpecAttackTimers[SPECATK_ENRAGE]->Check())
        return;
    // see if NPC has possibility to enrage
    if (!SpecAttacks[SPECATK_ENRAGE])
        return;
    // check if timer exists (should be true at all times)
    if (SpecAttackTimers[SPECATK_ENRAGE])
    {
		safe_delete(SpecAttackTimers[SPECATK_ENRAGE]);
        SpecAttackTimers[SPECATK_ENRAGE] = NULL;
    }

    if (!SpecAttackTimers[SPECATK_ENRAGE])
    {
        SpecAttackTimers[SPECATK_ENRAGE] = new Timer(EnragedDurationTimer);
    }
    // start the timer. need to call IsEnraged frequently since we dont have callback timers :-/
    SpecAttackTimers[SPECATK_ENRAGE]->Start();
    bEnraged = true;
	entity_list.MessageClose(this, true, 200, 13, "%s has become ENRAGED.", GetCleanName());
}

void Mob::ProcessEnrage(){
	if(IsEnraged()){
		if(SpecAttackTimers[SPECATK_ENRAGE] && SpecAttackTimers[SPECATK_ENRAGE]->Check()){
			entity_list.MessageClose(this, true, 200, 13, "%s is no longer enraged.", GetCleanName());
			SpecAttackTimers[SPECATK_ENRAGE]->Start(EnragedTimer);
			bEnraged = false;
		}
	}
}

bool Mob::IsEnraged() 
{
    return bEnraged;
}

bool Mob::Flurry()
{
    // attack the most hated target, regardless of range or whatever
    Mob *target = GetHateTop();
	if (target) {
		entity_list.MessageClose(this, true, 200, 13, "%s executes a FLURRY of attacks on %s!", GetCleanName(), target->GetCleanName());
		for (int i = 0; i < MAX_FLURRY_HITS; i++)
			Attack(target);
	}
    return true;
}

bool Mob::AddRampage(Mob *mob)
{
	if(!mob)
		return false;

    if (!SpecAttacks[SPECATK_RAMPAGE])
        return false;
    for (int i = 0; i < MAX_RAMPAGE_LIST; i++)
    {
        // if name is already on the list dont add it again
        if (strcasecmp(mob->GetName(), RampageArray[i]) == 0)
            return false;

		if (RampageArray[i][0] == 0 ){ //we assume the slot is free if the first character is empty.
			strcpy(RampageArray[i], mob->GetName());
			LogFile->write(EQEMuLog::Normal, "Adding %s to Rampage List in slot %d", RampageArray[i], i);
			return true;
		}
	}
    return false;
}

void Mob::ClearRampage(){
	memset(RampageArray, 0, sizeof(RampageArray));
}

bool Mob::Rampage()
{
	int index_hit = 0;
	entity_list.MessageClose(this, true, 200, 13, "%s goes on a RAMPAGE!", GetCleanName());
    for (int i = 0; i < MAX_RAMPAGE_LIST; i++)
    {
		if(index_hit >= MAX_RAMPAGE_TARGETS)
			break;	
        // range is important
        if (strlen(RampageArray[i]) == 0)
        	continue;
        Mob *target = entity_list.GetMob(RampageArray[i]);
        if(target)
        {
            if (CombatRange(target)){
                Attack(target);
				index_hit++;
			}
        }
    }
    return true;
}

int32 Mob::GetLevelCon(int8 mylevel, int8 iOtherLevel) {
    sint16 diff = iOtherLevel - mylevel;
	int32 conlevel=0;
	
    if (diff == 0)
        return CON_WHITE;
    else if (diff >= 1 && diff <= 2)
        return CON_YELLOW;
    else if (diff >= 3)
        return CON_RED;

    if (mylevel <= 8)
    {
        if (diff <= -4)
            conlevel = CON_GREEN;
        else
            conlevel = CON_BLUE;
    }
    else if (mylevel <= 9)
	{
        if (diff <= -6)
            conlevel = CON_GREEN;
        else if (diff <= -4)
            conlevel = CON_LIGHTBLUE;
        else
            conlevel = CON_BLUE;
	}
    else if (mylevel <= 13)
	{
        if (diff <= -7)
            conlevel = CON_GREEN;
        else if (diff <= -5)
            conlevel = CON_LIGHTBLUE;
        else
            conlevel = CON_BLUE;
	}
	else if (mylevel <= 15)
	{
        if (diff <= -7)
            conlevel = CON_GREEN;
        else if (diff <= -5)
            conlevel = CON_LIGHTBLUE;
        else
            conlevel = CON_BLUE;
	}
	else if (mylevel <= 17)
	{
        if (diff <= -8)
            conlevel = CON_GREEN;
        else if (diff <= -6)
            conlevel = CON_LIGHTBLUE;
        else
            conlevel = CON_BLUE;
	}
	else if (mylevel <= 21)
	{
        if (diff <= -9)
            conlevel = CON_GREEN;
        else if (diff <= -7)
            conlevel = CON_LIGHTBLUE;
        else
            conlevel = CON_BLUE;
	}
	else if (mylevel <= 25)
	{
        if (diff <= -10)
            conlevel = CON_GREEN;
        else if (diff <= -8)
            conlevel = CON_LIGHTBLUE;
        else
            conlevel = CON_BLUE;
	}
	else if (mylevel <= 29)
	{
        if (diff <= -11)
            conlevel = CON_GREEN;
        else if (diff <= -9)
            conlevel = CON_LIGHTBLUE;
        else
            conlevel = CON_BLUE;
	}
	else if (mylevel <= 31)
	{
        if (diff <= -12)
            conlevel = CON_GREEN;
        else if (diff <= -9)
            conlevel = CON_LIGHTBLUE;
        else
            conlevel = CON_BLUE;
	}
	else if (mylevel <= 33)
	{
        if (diff <= -13)
            conlevel = CON_GREEN;
        else if (diff <= -10)
            conlevel = CON_LIGHTBLUE;
        else
            conlevel = CON_BLUE;
	}
	else if (mylevel <= 37)
	{
        if (diff <= -14)
            conlevel = CON_GREEN;
        else if (diff <= -11)
            conlevel = CON_LIGHTBLUE;
        else
            conlevel = CON_BLUE;
	}
	else if (mylevel <= 41)
	{
        if (diff <= -16)
            conlevel = CON_GREEN;
        else if (diff <= -12)
            conlevel = CON_LIGHTBLUE;
        else
            conlevel = CON_BLUE;
	}
	else if (mylevel <= 45)
	{
        if (diff <= -17)
            conlevel = CON_GREEN;
        else if (diff <= -13)
            conlevel = CON_LIGHTBLUE;
        else
            conlevel = CON_BLUE;
	}
	else if (mylevel <= 49)
	{
        if (diff <= -18)
            conlevel = CON_GREEN;
        else if (diff <= -14)
            conlevel = CON_LIGHTBLUE;
        else
            conlevel = CON_BLUE;
	}
	else if (mylevel <= 53)
	{
        if (diff <= -19)
            conlevel = CON_GREEN;
        else if (diff <= -15)
            conlevel = CON_LIGHTBLUE;
        else
            conlevel = CON_BLUE;
	}
	else if (mylevel <= 55)
	{
        if (diff <= -20)
            conlevel = CON_GREEN;
        else if (diff <= -15)
            conlevel = CON_LIGHTBLUE;
        else
            conlevel = CON_BLUE;
	}
	else
	{
        if (diff <= -21)
            conlevel = CON_GREEN;
        else if (diff <= -16)
            conlevel = CON_LIGHTBLUE;
        else
            conlevel = CON_BLUE;
	}
	return conlevel;
}

void NPC::CheckSignal() {
	if (signaled) {
		char buf[32];
		snprintf(buf, 31, "%d", signal_id);
		buf[31] = '\0';
		parse->Event(EVENT_SIGNAL, GetNPCTypeID(), buf, this, NULL);
		signaled=false;
	}
}



/*
alter table npc_types drop column usedspells;
alter table npc_types add column npc_spells_id int(11) unsigned not null default 0 after merchant_id;
Create Table npc_spells (
	id int(11) unsigned not null auto_increment primary key,
	name tinytext,
	parent_list int(11) unsigned not null default 0,
	attack_proc smallint(5) not null default -1,
	proc_chance tinyint(3) not null default 3
	);
create table npc_spells_entries (
	id int(11) unsigned not null auto_increment primary key,
	npc_spells_id int(11) not null,
	spellid smallint(5) not null default 0,
	type smallint(5) unsigned not null default 0,
	minlevel tinyint(3) unsigned not null default 0,
	maxlevel tinyint(3) unsigned not null default 255,
	manacost smallint(5) not null default '-1',
	recast_delay int(11) not null default '-1',
	priority smallint(5) not null default 0,
	index npc_spells_id (npc_spells_id)
	);
*/ 

bool IsSpellInList(DBnpcspells_Struct* spell_list, sint16 iSpellID);

bool NPC::AI_AddNPCSpells(int32 iDBSpellsID) {
	// ok, this function should load the list, and the parent list then shove them into the struct and sort
	npc_spells_id = iDBSpellsID;
	memset(AIspells, 0, sizeof(AIspells));
	if (iDBSpellsID == 0) {
		AIautocastspell_timer->Disable();
		return false;
	}
	DBnpcspells_Struct* spell_list = database.GetNPCSpells(iDBSpellsID);
	if (!spell_list) {
		AIautocastspell_timer->Disable();
		return false;
	}
	DBnpcspells_Struct* parentlist = database.GetNPCSpells(spell_list->parent_list);
	uint32 i;
#if MobAI_DEBUG_Spells >= 10
	cout << "Loading NPCSpells onto " << this->GetName() << ": dbspellsid=" << iDBSpellsID;
	if (spell_list) {
		cout << " (found, " << spell_list->numentries << "), parentlist=" << spell_list->parent_list;
		if (spell_list->parent_list) {
			if (parentlist) {
				cout << " (found, " << parentlist->numentries << ")";
			}
			else
				cout << " (not found)";
		}
	}
	else
		cout << " (not found)";
	cout << endl;
#endif
	sint16 attack_proc_spell = -1;
	sint8 proc_chance = 3;
	if (parentlist) {
		attack_proc_spell = parentlist->attack_proc;
		proc_chance = parentlist->proc_chance;
		for (i=0; i<parentlist->numentries; i++) {
			if (GetLevel() >= parentlist->entries[i].minlevel && GetLevel() <= parentlist->entries[i].maxlevel && parentlist->entries[i].spellid > 0) {
				if (!IsSpellInList(spell_list, parentlist->entries[i].spellid))
					AddSpellToNPCList(AIspells, parentlist->entries[i].priority, parentlist->entries[i].spellid, parentlist->entries[i].type, parentlist->entries[i].manacost, parentlist->entries[i].recast_delay);
			}
		}
	}
	if (spell_list->attack_proc >= 0) {
		attack_proc_spell = spell_list->attack_proc;
		proc_chance = spell_list->proc_chance;
	}
	for (i=0; i<spell_list->numentries; i++) {
		if (GetLevel() >= spell_list->entries[i].minlevel && GetLevel() <= spell_list->entries[i].maxlevel && spell_list->entries[i].spellid > 0) {
			AddSpellToNPCList(AIspells, spell_list->entries[i].priority, spell_list->entries[i].spellid, spell_list->entries[i].type, spell_list->entries[i].manacost, spell_list->entries[i].recast_delay);
		}
	}
	if (attack_proc_spell > 0)
		AddProcToWeapon(attack_proc_spell, true, proc_chance);

#if MobAI_DEBUG_Spells >= 11
	i=0;
	for (int j=0; j<MAX_AISPELLS; j++) {
		if (AIspells[j].spellid > 0) {
			cout << "NPCSpells on " << this->GetName() << ": AIspells[" << j << "].spellid=" << setw(5) << AIspells[j].spellid << ": " << spells[AIspells[j].spellid].name << endl;
			i++;
		}
	}
	cout << i << " NPCSpells on " << this->GetName() << endl;
#endif

	if (AIspells[0].spellid == 0)
		AIautocastspell_timer->Disable();
	else
		AIautocastspell_timer->Trigger();
	return true;
}

bool IsSpellInList(DBnpcspells_Struct* spell_list, sint16 iSpellID) {
	for (uint32 i=0; i < spell_list->numentries; i++) {
		if (spell_list->entries[i].spellid == iSpellID)
			return true;
	}
	return false;
}

// adds a spell to the list, taking into account priority and resorting list as needed.
void NPC::AddSpellToNPCList(AISpells_Struct* AIspells, sint16 iPriority, sint16 iSpellID, uint16 iType, sint16 iManaCost, sint32 iRecastDelay) {
	if (iSpellID <= 0 || iSpellID > SPDAT_RECORDS) {

#if MobAI_DEBUG_Spells >= 1
		cout << "AddSpellToNPCList: Spell #" << iSpellID << " not added, out of bounds" << endl;
#endif

		return;
	}

#if MobAI_DEBUG_Spells >= 12
	cout << "Adding spell #" << iSpellID;
#endif

	for (int i=0; i<MAX_AISPELLS; i++) {
		if (AIspells[i].spellid <= 0) {
			AIspells[i].spellid = iSpellID;
			AIspells[i].priority = iPriority;
			AIspells[i].type = iType;
			AIspells[i].manacost = iManaCost;
			AIspells[i].recast_delay = iRecastDelay;

#if MobAI_DEBUG_Spells >= 12
			cout << " to slot " << i;
#endif

			break;
		}
		else if (AIspells[i].priority < iPriority) {
			for (int j=MAX_AISPELLS-1; j>i; j--) {
				AIspells[j].spellid = AIspells[j-1].spellid;
				AIspells[j].priority = AIspells[j-1].priority;
				AIspells[j].type = AIspells[j-1].type;
				AIspells[j].manacost = AIspells[j-1].manacost;
				AIspells[j].recast_delay = AIspells[j-1].recast_delay;
			}
			AIspells[i].spellid = iSpellID;
			AIspells[i].priority = iPriority;
			AIspells[i].type = iType;
			AIspells[i].manacost = iManaCost;
			AIspells[i].recast_delay = iRecastDelay;

#if MobAI_DEBUG_Spells >= 12
			cout << " to slot " << i;
#endif

			break;
		}
	}

#if MobAI_DEBUG_Spells >= 12
	cout << endl;
#endif

}


DBnpcspells_Struct* ZoneDatabase::GetNPCSpells(int32 iDBSpellsID) {
	if (iDBSpellsID == 0)
		return 0;
	if (!npc_spells_cache) {
		npc_spells_maxid = GetMaxNPCSpellsID();
		npc_spells_cache = new DBnpcspells_Struct*[npc_spells_maxid+1];
		npc_spells_loadtried = new bool[npc_spells_maxid+1];
		for (uint32 i=0; i<=npc_spells_maxid; i++) {
			npc_spells_cache[i] = 0;
			npc_spells_loadtried[i] = false;
		}
	}
	if (iDBSpellsID > npc_spells_maxid)
		return 0;
	if (npc_spells_cache[iDBSpellsID]) { // it's in the cache, easy =)
		return npc_spells_cache[iDBSpellsID];
	}
	else if (!npc_spells_loadtried[iDBSpellsID]) { // no reason to ask the DB again if we have failed once already
		npc_spells_loadtried[iDBSpellsID] = true;
		char errbuf[MYSQL_ERRMSG_SIZE];
		char *query = 0;
		MYSQL_RES *result;
		MYSQL_ROW row;
		
		if (RunQuery(query, MakeAnyLenString(&query, "SELECT id, parent_list, attack_proc, proc_chance from npc_spells where id=%d", iDBSpellsID), errbuf, &result)) {
			safe_delete_array(query);
			if (mysql_num_rows(result) == 1) {
				row = mysql_fetch_row(result);
				int32 tmpparent_list = atoi(row[1]);
				sint16 tmpattack_proc = atoi(row[2]);
				int8 tmpproc_chance = atoi(row[3]);
				mysql_free_result(result);
				if (RunQuery(query, MakeAnyLenString(&query, "SELECT spellid, type, minlevel, maxlevel, manacost, recast_delay, priority from npc_spells_entries where npc_spells_id=%d ORDER BY minlevel", iDBSpellsID), errbuf, &result)) {
					safe_delete_array(query);
					int32 tmpSize = sizeof(DBnpcspells_Struct) + (sizeof(DBnpcspells_entries_Struct) * mysql_num_rows(result));
					npc_spells_cache[iDBSpellsID] = (DBnpcspells_Struct*) new uchar[tmpSize];
					memset(npc_spells_cache[iDBSpellsID], 0, tmpSize);
					npc_spells_cache[iDBSpellsID]->parent_list = tmpparent_list;
					npc_spells_cache[iDBSpellsID]->attack_proc = tmpattack_proc;
					npc_spells_cache[iDBSpellsID]->proc_chance = tmpproc_chance;
					npc_spells_cache[iDBSpellsID]->numentries = mysql_num_rows(result);
					int j = 0;
					while ((row = mysql_fetch_row(result))) {
						npc_spells_cache[iDBSpellsID]->entries[j].spellid = atoi(row[0]);
						npc_spells_cache[iDBSpellsID]->entries[j].type = atoi(row[1]);
						npc_spells_cache[iDBSpellsID]->entries[j].minlevel = atoi(row[2]);
						npc_spells_cache[iDBSpellsID]->entries[j].maxlevel = atoi(row[3]);
						npc_spells_cache[iDBSpellsID]->entries[j].manacost = atoi(row[4]);
						npc_spells_cache[iDBSpellsID]->entries[j].recast_delay = atoi(row[5]);
						npc_spells_cache[iDBSpellsID]->entries[j].priority = atoi(row[6]);
						j++;
					}
					mysql_free_result(result);
					return npc_spells_cache[iDBSpellsID];
				}
				else {
					cerr << "Error in AddNPCSpells query1 '" << query << "' " << errbuf << endl;
					safe_delete_array(query);
					return 0;
				}
			}
			else {
				mysql_free_result(result);
			}
		}
		else {
			cerr << "Error in AddNPCSpells query1 '" << query << "' " << errbuf << endl;
			safe_delete_array(query);
			return 0;
		}
		
		return 0;	
	}
	return 0;
}

int32 ZoneDatabase::GetMaxNPCSpellsID() {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;
	
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT max(id) from npc_spells"), errbuf, &result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			int32 ret = 0;
			if (row[0])
				ret = atoi(row[0]);
			mysql_free_result(result);
			return ret;
		}
		mysql_free_result(result);
	}
	else {
		cerr << "Error in GetMaxNPCSpellsID query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
		return 0;
	}
	
	return 0;	
}

