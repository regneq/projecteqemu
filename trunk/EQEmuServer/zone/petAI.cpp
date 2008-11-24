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
#include "../common/rulesys.h"

#if !defined(NEW_LoadSPDat) && !defined(DB_LoadSPDat)
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

// Franck-add: EQoffline
// AI, for the pets

void Mob::PET_Process() {
	_ZP(Mob_PET_Process);
 
	// of course, if we're not a pet
	if( !IsPet() || !GetOwner() )
		return;

	if( !GetOwner() )
		Kill();
 
	
	if (!IsAIControlled())
		return;
 
	if (!(AIthink_timer->Check() || attack_timer.Check(false)))
		return;
 
	if (IsCasting())
		return;
 
	// if our owner isn't a pet or if he is not a client...
	if (!GetOwner()->IsBot() || ( !GetOwner()->IsBot() && !GetOwner()->IsClient() ) )
		return;
    
    if (IsEngaged())
	{
		_ZP(Mob_PET_Process_IsEngaged);
        if (IsRooted())
            CastToNPC()->SetTarget(hate_list.GetClosest(this));
        else
            CastToNPC()->SetTarget(hate_list.GetTop(this));

        if (!target)
            return;
 
		// Let's check if we have a los with our target.
		// If we don't, our hate_list is wiped.
		// It causes some cpu stress but without it, it was causing the bot/pet to aggro behind wall, floor etc... 
        if(!CheckLosFN(target) || target->IsMezzed() || !IsAttackAllowed(target)) {
			WipeHateList();
            CastToNPC()->SetTarget(GetOwner());
            return;
        }

        bool is_combat_range = CombatRange(target);
 
		// Ok, we're engaged, each class type has a special AI
		// Only melee class will go to melee. Casters and healers will stay behind, following the leader by default.
		// I should probably make the casters staying in place so they can cast..
 
		// Ok, we 're a melee or any other class lvl<12. Yes, because after it becomes hard to go in melee for casters.. even for bots..
		if( is_combat_range )
		{
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
			}
			// we can't fight if we don't have a target, are stun/mezzed or dead..
			if(target && !IsStunned() && !IsMezzed() && GetAppearance() != eaDead ) 
			{
				if(attack_timer.Check())  // check the delay on the attack
				{		
					if(Attack(target, 13))			// try the main hand
					if (target)					// Do we still have a target?
					{
						// We're a pet so we re able to dual attack
						sint32 RandRoll = MakeRandomInt(0, 99);	
						if (CanThisClassDoubleAttack() && (RandRoll < (GetLevel() + NPCDualAttackModifier)))	
						{
							if(Attack(target, 13)) 
							{}
						}
					}
 
					// Ok now, let's check pet's offhand. 
					if (attack_dw_timer.Check() && GetOwnerID() && ( GetOwner()->GetClass() == MAGICIAN || GetOwner()->GetClass() == NECROMANCER || GetOwner()->GetClass() == SHADOWKNIGHT || GetOwner()->GetClass() == BEASTLORD ) ) 
					{
						if(GetOwner()->GetLevel() >= 24)
						{
							float DualWieldProbability = (GetSkill(DUAL_WIELD) + GetLevel()) / 400.0f;
                            DualWieldProbability -= MakeRandomFloat(0, 1);
							if(DualWieldProbability < 0){
                                Attack(target, 14);
                                if (CanThisClassDoubleAttack())
                                {
                                    sint32 RandRoll = rand()%100;
                                    if (RandRoll < (GetLevel() + 20))
                                    {
                                        Attack(target, 14);
                                    }
                                }
							}
						}
					}
					// Special attack
					CastToNPC()->DoClassAttacks(target); 
				}
                // See if the pet can cast any spell
                Bot_AI_EngagedCastCheck();
            }	
		}// end of the combat in range
		else{
			// Now, if we cannot reach our target
			if (!HateSummon()) 
			{
				if(target && Bot_AI_PursueCastCheck()) 
				{}
				else if (target && AImovement_timer->Check()) 
				{
					SetRunAnimSpeed(0);
					if(!IsRooted()) {
						mlog(AI__WAYPOINTS, "Pursuing %s while engaged.", target->GetName());
						CalculateNewPosition2(target->GetX(), target->GetY(), target->GetZ(), GetOwner()->GetRunspeed(), false);
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
	else{
		// Franck: EQoffline
		// Ok if we're not engaged, what's happening..
		if(target != GetOwner()) {
			CastToNPC()->SetTarget(GetOwner());
		}
		if(!IsMoving()) {
			Bot_AI_IdleCastCheck();
		}
		if(AImovement_timer->Check()) {
			switch(pStandingPetOrder) {
				case SPO_Follow:
					{
						float dist = DistNoRoot(*target);
						SetRunAnimSpeed(0);
						if(dist > 184) {
							CalculateNewPosition2(target->GetX(), target->GetY(), target->GetZ(), target->GetRunspeed(), false);
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
					break;
				case SPO_Sit:
					SetAppearance(eaSitting);
					break;
				case SPO_Guard:
					CastToNPC()->NextGuardPosition();
					break;
			}
		}
	}
}

#endif //EQBOTS
