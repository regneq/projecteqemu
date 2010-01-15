#ifdef BOTS

#include "bot.h"

bool Bot::AICastSpell(Mob* tar, int8 iChance, int16 iSpellTypes) {
	_ZP(Bot_AICastSpell);

	if (!tar) {
		return false;
	}

	if(!AI_HasSpells())
		return false;

	if(tar->GetAppearance() == eaDead) {
		if(tar->IsClient() && tar->CastToClient()->GetFeigned()) {
			// do nothing
		}
		else {
			return false;
		}
	}

	if (iChance < 100) {
		if (MakeRandomInt(0, 100) > iChance){
			return false;
		}
	}

	int8 botClass = GetClass();
	uint8 botLevel = GetLevel();

	bool checked_los = false;	//we do not check LOS until we are absolutely sure we need to, and we only do it once.

	bool castedSpell = false;

	BotSpell botSpell;
	botSpell.SpellId = 0;
	botSpell.SpellIndex = 0;
	botSpell.ManaCost = 0;

	switch (iSpellTypes) {
		case SpellType_Heal: {
			if (tar->DontHealMeBefore() < Timer::GetCurrentTime()) {
				int8 hpr = (int8)tar->GetHPRatio();

				if(hpr <= 95 || (tar->IsClient() && (hpr <= 95)) || (botClass == BARD)) {
					if(tar->GetClass() == NECROMANCER) {
						// Give necromancers a chance to go lifetap something or cleric can spend too much mana on a necro
						if(hpr > 40) {
							break;
						}
					}

					if(tar->FindType(SE_HealOverTime)) {
						// Let the heal over time buff do it's thing ...
						if(tar->IsEngaged() && hpr >= 85)
							break;
						else if(!tar->IsEngaged())
							break;
					}

					// Evaluate the situation
					if((tar->IsEngaged()) && ((botClass == CLERIC) || (botClass == DRUID) || (botClass == SHAMAN))) {
						if(hpr <= 35)
							botSpell = GetBestBotSpellForFastHeal(this);
						else if(hpr > 35 && hpr < 70)
							botSpell = GetBestBotSpellForPercentageHeal(this);
						else if(hpr >= 70 && hpr < 90)
							botSpell = GetBestBotSpellForRegularSingleTargetHeal(this);
						else
							botSpell = GetBestBotSpellForHealOverTime(this);
					}
					else if ((botClass == CLERIC) || (botClass == DRUID) || (botClass == SHAMAN)) {
						if(hpr <= 30)
							botSpell = GetBestBotSpellForPercentageHeal(this);
						else if(hpr > 30 && hpr < 75)
							botSpell = GetBestBotSpellForRegularSingleTargetHeal(this);
						else
							botSpell = GetBestBotSpellForHealOverTime(this);
					}	
			
					if(botSpell.SpellId == 0)
						botSpell = GetFirstBotSpellBySpellType(this, iSpellTypes);

					// If there is still no spell id, then there isn't going to be one so we are done
					if(botSpell.SpellId == 0)
						break;

					// Can we cast this spell on this target?
					if(!(spells[botSpell.SpellId].targettype==ST_GroupTeleport || spells[botSpell.SpellId].targettype == ST_Target || tar == this)
						&& !(tar->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0))
						break;

					int32 TempDontHealMeBeforeTime = tar->DontHealMeBefore();

					castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost, &TempDontHealMeBeforeTime);

					if(TempDontHealMeBeforeTime != tar->DontHealMeBefore())
						tar->SetDontHealMeBefore(TempDontHealMeBeforeTime);

					// For non-HoT heals, do a 4 second delay
					// TODO: Replace this code with logic that calculates the delay based on number of clerics in rotation
					//			and ignores heals for anyone except the main tank
					if(!IsHealOverTimeSpell(botSpell.SpellId)) {
						if(IsCompleteHealSpell(botSpell.SpellId)) { 
							// Complete Heal 4 second rotation
							tar->SetDontHealMeBefore(Timer::GetCurrentTime() + 4000);
						}
						else {
							tar->SetDontHealMeBefore(Timer::GetCurrentTime() + 1000);
						}
					}
				}
			}
			break;
							 }
		case SpellType_Root: {
			if (!tar->IsRooted() && tar->DontRootMeBefore() < Timer::GetCurrentTime()) {
					if(!checked_los) {
						if(!CheckLosFN(tar))
							break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
						
						checked_los = true;
					}

					// TODO: If there is a ranger in the group then don't allow root spells

					botSpell = GetFirstBotSpellBySpellType(this, iSpellTypes);

					if(botSpell.SpellId == 0)
						break;

					if(tar->CanBuffStack(botSpell.SpellId, botLevel, true) == 0)
						break;

					int32 TempDontRootMeBefore = tar->DontRootMeBefore();
					
					castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost, &TempDontRootMeBefore);
					
					if(TempDontRootMeBefore != tar->DontRootMeBefore())
						tar->SetDontRootMeBefore(TempDontRootMeBefore);
			}
			break;
							 }
		case SpellType_Buff: {
			if ((tar->DontBuffMeBefore() < Timer::GetCurrentTime())) {
				std::list<BotSpell> buffSpellList = GetBotSpellsBySpellType(this, SpellType_Buff);

				for(std::list<BotSpell>::iterator itr = buffSpellList.begin(); itr != buffSpellList.end(); itr++) {
					BotSpell selectedBotSpell = *itr;

					if(selectedBotSpell.SpellId == 0)
						continue;

					// no buffs with illusions.. use #bot command to cast illusions
					if(IsEffectInSpell(selectedBotSpell.SpellId, SE_Illusion))
						continue;

					// can not cast buffs for your own pet only on another pet that isn't yours
					if((spells[selectedBotSpell.SpellId].targettype == ST_Pet) && (tar != this->GetPet()))
						continue;

					// Validate target
					if(!((spells[selectedBotSpell.SpellId].targettype == ST_Target || spells[selectedBotSpell.SpellId].targettype == ST_Pet || tar == this)
						&& !tar->IsImmuneToSpell(selectedBotSpell.SpellId, this)
						&& (tar->CanBuffStack(selectedBotSpell.SpellId, botLevel, true) >= 0))) {
							continue;
					}

					// Put the zone levitate and movement check here since bots are able to bypass the client casting check
					if((IsEffectInSpell(selectedBotSpell.SpellId, SE_Levitate) && !zone->CanLevitate())
						|| (IsEffectInSpell(selectedBotSpell.SpellId, SE_MovementSpeed) && !zone->CanCastOutdoor())) {
							continue;
					}

					int32 TempDontBuffMeBefore = tar->DontBuffMeBefore();

					castedSpell = AIDoSpellCast(selectedBotSpell.SpellIndex, tar, selectedBotSpell.ManaCost, &TempDontBuffMeBefore);

					if(TempDontBuffMeBefore != tar->DontBuffMeBefore())
						tar->SetDontBuffMeBefore(TempDontBuffMeBefore);

					if(castedSpell)
						break;
				}
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
				botSpell = GetFirstBotSpellBySpellType(this, iSpellTypes);

				if(botSpell.SpellId == 0)
					break;

				castedSpell = AIDoSpellCast(botSpell.SpellIndex, this, botSpell.ManaCost);
			}
			break;
							   }
		case SpellType_Nuke: {
			if(((MakeRandomInt(1, 100) <= iChance) || ((botClass == BARD) || (botClass == SHAMAN) || (botClass == ENCHANTER)))
				&& ((tar->GetHPRatio() <= 95.0f) || ((botClass == BARD) || (botClass == SHAMAN) || (botClass == ENCHANTER))))
			{
				if(!checked_los) {
					if(!CheckLosFN(tar))
						break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
					
					checked_los = true;
				}

				if(botClass == MAGICIAN || botClass == SHADOWKNIGHT || botClass == NECROMANCER || botClass == PALADIN || botClass == RANGER || botClass == DRUID || botClass == CLERIC) {
					if(tar->GetBodyType() == BT_Undead || tar->GetBodyType() == BT_SummonedUndead || tar->GetBodyType() == BT_Vampire)
						botSpell = GetBestBotSpellForNukeByTargetType(this, ST_Undead);
					else if(tar->GetBodyType() == BT_Summoned || tar->GetBodyType() == BT_Summoned2 || tar->GetBodyType() == BT_Summoned3)
						botSpell = GetBestBotSpellForNukeByTargetType(this, ST_Summoned);
				}
				else if(botClass == WIZARD) {
					botSpell = GetBestBotWizardNukeSpellByTargetResists(this, tar);
				}
			
				if(botSpell.SpellId == 0)
					botSpell = GetBestBotSpellForNukeByTargetType(this, ST_Target);

				if(botSpell.SpellId == 0)
					break;

				if(!(!tar->IsImmuneToSpell(botSpell.SpellId, this) && (tar->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0)))
					break;
				
				if(IsFearSpell(botSpell.SpellId)) { 
					// don't let fear cast if the npc isn't snared or rooted
					if(tar->GetSnaredAmount() == -1) {
						if(!tar->IsRooted())
							break;
					}
				}

				castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);
			}
			break;
							 }
		case SpellType_Dispel: {
			if(tar->GetHPRatio() > 95.0f) {
				if(!checked_los) {
					if(!CheckLosFN(tar))
						break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
					
					checked_los = true;
				}
				
				botSpell = GetFirstBotSpellBySpellType(this, iSpellTypes);

				if(botSpell.SpellId == 0)
					break;

				// TODO: Check target to see if there is anything to dispel

				if(tar->CountDispellableBuffs() > 0) {
					castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);
				}
			}
			break;
							   }
		case SpellType_Pet: {
				//keep mobs from recasting pets when they have them.
				if (!IsPet() && !GetPetID() && !IsBotCharmer()) {
					if(botClass == MAGICIAN)
						botSpell = GetBestBotMagicianPetSpell(this);
					else
						botSpell = GetFirstBotSpellBySpellType(this, SpellType_Pet);

					if(botSpell.SpellId == 0)
						break;

					castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);
				}
				break;
							}
		case SpellType_Lifetap: {
			if (GetHPRatio() < 90.0f) {
				if(!checked_los) {
					if(!CheckLosFN(tar))
						break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
					
					checked_los = true;
				}

				botSpell = GetFirstBotSpellBySpellType(this, iSpellTypes);

				if(botSpell.SpellId == 0)
					break;

				if(!(!tar->IsImmuneToSpell(botSpell.SpellId, this) && (tar->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0)))
					break;

				castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);
			}
			break;
								}
		case SpellType_Snare: {
			if (tar->DontSnareMeBefore() < Timer::GetCurrentTime()) {
					if(!checked_los) {
						if(!CheckLosFN(tar))
							break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
						
						checked_los = true;
					}

					botSpell = GetFirstBotSpellBySpellType(this, iSpellTypes);

					if(botSpell.SpellId == 0)
						break;

					if(!(!tar->IsImmuneToSpell(botSpell.SpellId, this) && tar->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0))
						break;

					int32 TempDontSnareMeBefore = tar->DontSnareMeBefore();
					
					castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost, &TempDontSnareMeBefore);
					
					if(TempDontSnareMeBefore != tar->DontSnareMeBefore())
						tar->SetDontSnareMeBefore(TempDontSnareMeBefore);
			}
			break;
							  }
		case SpellType_DOT: {
			if ((tar->GetHPRatio() <= 98.0f) && (tar->DontDotMeBefore() < Timer::GetCurrentTime()) && (tar->GetHPRatio() > 15.0f)) {
				if(!checked_los) {
					if(!CheckLosFN(tar))
						break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call

					checked_los = true;
				}

				std::list<BotSpell> dotList = GetBotSpellsBySpellType(this, SpellType_DOT);

				const int maxDotSelect = 5;
				int dotSelectCounter = 0;

				for(list<BotSpell>::iterator itr = dotList.begin(); itr != dotList.end(); itr++) {
					BotSpell selectedBotSpell = *itr;

					if(selectedBotSpell.SpellId == 0)
						continue;

					if(!(!tar->IsImmuneToSpell(selectedBotSpell.SpellId, this) && tar->CanBuffStack(selectedBotSpell.SpellId, botLevel, true) >= 0))
						continue;

					int32 TempDontDotMeBefore = tar->DontDotMeBefore();

					castedSpell = AIDoSpellCast(selectedBotSpell.SpellIndex, tar, selectedBotSpell.ManaCost, &TempDontDotMeBefore);

					if(TempDontDotMeBefore != tar->DontDotMeBefore())
						tar->SetDontDotMeBefore(TempDontDotMeBefore);

					dotSelectCounter++;

					if((dotSelectCounter == maxDotSelect) || castedSpell)
						break;
				}
			}
			break;
							}
		case SpellType_Slow: {
			if (tar->GetHPRatio() <= 99.0f) {
					if(!checked_los) {
						if(!CheckLosFN(tar))
							break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
						
						checked_los = true;
					}
					
					if(botClass == SHAMAN && tar->GetDR() < tar->GetMR() && ((tar->GetMR() - tar->GetDR()) > 75)) {
						botSpell = GetBestBotSpellForDiseaseBasedSlow(this);
					}
					else {
						botSpell = GetBestBotSpellForMagicBasedSlow(this);
					}

					if(botSpell.SpellId == 0)
						break;

					if(!(!tar->IsImmuneToSpell(botSpell.SpellId, this) && tar->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0))
						break;
					
					castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);
			}
			break;
							}
		case SpellType_Mez: {
			if (tar->GetBodyType() != BT_Giant) {
					if(!checked_los) {
						if(!CheckLosFN(tar))
							break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
						
						checked_los = true;
					}
					
					botSpell = GetBestBotSpellForMez(this);

					if(botSpell.SpellId == 0)
						break;

					Mob* addMob = GetFirstIncomingMobToMez(this, botSpell);

					if(!addMob)
						break;

					if(!(!addMob->IsImmuneToSpell(botSpell.SpellId, this) && addMob->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0))
						break;
					
					castedSpell = AIDoSpellCast(botSpell.SpellIndex, addMob, botSpell.ManaCost);
			}
			break;
							}
		default: {
			break;
					  }
	}

	return castedSpell;
}

bool Bot::AIDoSpellCast(int8 i, Mob* tar, sint32 mana_cost, int32* oDontDoAgainBefore) {
	bool result = false;

	// manacost has special values, -1 is no mana cost, -2 is instant cast (no mana)
	sint32 manaCost = mana_cost;

	if (manaCost == -1)
		manaCost = spells[AIspells[i].spellid].mana;
	else if (manaCost == -2)
		manaCost = 0;

	sint32 extraMana = 0;
	sint32 hasMana = GetMana();

	// Allow bots to cast buff spells even if they are out of mana
	if(RuleB(Bots, BotFinishBuffing)) {
		if(manaCost > hasMana) {
			// Let's have the bots complete the buff time process
			if(AIspells[i].type & SpellType_Buff) {
				extraMana = manaCost - hasMana;
				SetMana(manaCost);
			}
		}
	}

	float dist2 = 0;

	if (AIspells[i].type & SpellType_Escape) {
		dist2 = 0; 
	} else 
		dist2 = DistNoRoot(*tar);

	if (((((spells[AIspells[i].spellid].targettype==ST_GroupTeleport && AIspells[i].type==2)
				|| spells[AIspells[i].spellid].targettype==ST_AECaster
				|| spells[AIspells[i].spellid].targettype==ST_Group
				|| spells[AIspells[i].spellid].targettype==ST_AEBard)
				&& dist2 <= spells[AIspells[i].spellid].aoerange*spells[AIspells[i].spellid].aoerange)
				|| dist2 <= spells[AIspells[i].spellid].range*spells[AIspells[i].spellid].range) && (mana_cost <= GetMana() || GetMana() == GetMaxMana()))
	{
		if(IsSitting())
			Stand();

		result = NPC::AIDoSpellCast(i, tar, mana_cost, oDontDoAgainBefore);
	}

	// if the spell wasn't casted, then take back any extra mana that was given to the bot to cast that spell
	if(!result) {
		SetMana(hasMana);
		extraMana = false;
	}

	return result;
}

bool Bot::AI_PursueCastCheck() {
	bool result = false;
	
	if (AIautocastspell_timer->Check(false)) {
		_ZP(Bot_AI_Process_pursue_cast);
		
		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.
		
		mlog(AI__SPELLS, "Bot Engaged (pursuing) autocast check triggered. Trying to cast offensive spells.");
		
		if(!AICastSpell(GetTarget(), 100, SpellType_Snare)) {
			if(!AICastSpell(GetTarget(), 100, SpellType_Lifetap)) {
				if(!AICastSpell(GetTarget(), 100, SpellType_Nuke)) {
					/*AIautocastspell_timer->Start(RandomTimer(500, 2000), false);
					result = true;*/
					result = true;
				}

				result = true;
			}

			result = true;
		} 
		
		if(!AIautocastspell_timer->Enabled())
			AIautocastspell_timer->Start(RandomTimer(100, 250), false);
	}
	
	return result;
}

bool Bot::AI_IdleCastCheck() {
	bool result = false;

	if (AIautocastspell_timer->Check(false)) {
		_ZP(Bot_AI_IdleCastCheck);
#if MobAI_DEBUG_Spells >= 25
		cout << "Non-Engaged autocast check triggered: " << this->GetCleanName() << endl;
#endif
		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.

		//Ok, IdleCastCheck depends of class. 
		// Healers WITHOUT pets will check if a heal is needed before buffing.
		int8 botClass = GetClass();
		
		if(botClass == CLERIC || botClass == PALADIN || botClass == RANGER) {
			if (!AICastSpell(this, 100, SpellType_Buff)) {
				if (!AICastSpell(this, 100, SpellType_Heal)) {
					if(!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, MobAISpellRange, SpellType_Heal)) {
						if(!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, MobAISpellRange, SpellType_Buff)) {
							//
						}
					}
				}
			}

			result = true;
		}
		// Pets class will first cast their pet, then buffs
		else if(botClass == DRUID || botClass == MAGICIAN || botClass == SHADOWKNIGHT || botClass == SHAMAN || botClass == NECROMANCER || botClass == ENCHANTER || botClass == BEASTLORD  || botClass == WIZARD) {			
			if (!AICastSpell(this, 100, SpellType_Pet)) {
				if (!AICastSpell(this, 100, SpellType_Buff)) {
					if (!AICastSpell(this, 100, SpellType_Heal)) {
						if(!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, MobAISpellRange, SpellType_Heal)) {
							if(!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, MobAISpellRange, SpellType_Buff)) {
								//
							}
						}
					}
				}
			}

			result = true;
		}		
		else if(botClass == BARD) {
			// bard bots
			AICastSpell(this, 100, SpellType_Heal);
			result = true;
		}

		if(!AIautocastspell_timer->Enabled())
			AIautocastspell_timer->Start(RandomTimer(1000, 5000), false);
	}
	
	return result;
}

bool Bot::AI_EngagedCastCheck() {
	bool result = false;

	if (GetTarget() && AIautocastspell_timer->Check(false)) {
		_ZP(Bot_AI_Process_engaged_cast);

		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.

		int8 botClass = GetClass();
		uint8 botLevel = GetLevel();

		mlog(AI__SPELLS, "Engaged autocast check triggered (BOTS). Trying to cast healing spells then maybe offensive spells.");

		if(botClass == CLERIC) {
			if(!AICastSpell(this, 100, SpellType_Heal)) {
				if(!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, MobAISpellRange, SpellType_Heal)) {
					if(!AICastSpell(GetTarget(), 50, SpellType_Nuke)) {
						//AIautocastspell_timer->Start(RandomTimer(100, 250), false);		// Do not give healer classes a lot of time off or your tank's die
					}
				}
			}

			result = true;
		}
		else if(botClass == DRUID) {
			if (!AICastSpell(GetTarget(), 100, SpellType_DOT)) {
				if(!AICastSpell(this, 100, SpellType_Heal)) {
					if(!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, MobAISpellRange, SpellType_Heal)) {
						if(!AICastSpell(GetTarget(), 100, SpellType_Nuke)) {
							//AIautocastspell_timer->Start(RandomTimer(100, 250), false);		// Do not give healer classes a lot of time off or your tank's die
						}
					}
				}
			}

			result = true;
		}
		else if(botClass == SHAMAN) {
			if (!AICastSpell(GetTarget(), 100, SpellType_Slow)) {
				if (!AICastSpell(this, 100, SpellType_Pet)) {
					if (!AICastSpell(GetTarget(), 100, SpellType_DOT)) {
						if(!AICastSpell(this, 100, SpellType_Heal)) {
							if (!AICastSpell(GetPet(), 100, SpellType_Heal)) {
								if(!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, MobAISpellRange, SpellType_Heal)) {
									if(!AICastSpell(GetTarget(), 100, SpellType_Nuke)) {
										//AIautocastspell_timer->Start(RandomTimer(100, 250), false);		// Do not give healer classes a lot of time off or your tank's die
									}
								}
							}
						}
					}
				}
			}

			result = true;
		}
		else if(botClass == RANGER) {
			if (!AICastSpell(GetTarget(), 50, SpellType_DOT)) {
				if (!AICastSpell(this, 100, SpellType_Heal)) {
					if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 30, MobAISpellRange, SpellType_Heal)) {
						if (!AICastSpell(GetTarget(), 50, SpellType_Nuke)) {
							//
						}
					}
				}
			}

			result = true;
		}
		else if(botClass == BEASTLORD) {
			if (!AICastSpell(GetTarget(), 100, SpellType_Slow)) {
				if (!AICastSpell(this, 100, SpellType_Pet)) {
					if (!AICastSpell(GetTarget(), 50, SpellType_DOT)) {
						if (!AICastSpell(this, 100, SpellType_Heal)) {
							if (!AICastSpell(GetPet(), 100, SpellType_Heal)) {
								if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 30, MobAISpellRange, SpellType_Heal)) {
									if(!AICastSpell(GetTarget(), 50, SpellType_Nuke)) {
										//
									}
								}
							}
						}
					}
				}
			}

			result = true;
		}
		else if(botClass == WIZARD) {
			if (!AICastSpell(GetTarget(), 100, SpellType_Nuke)) {
				if(!AICastSpell(GetTarget(), 60, SpellType_Escape)) {
					//
				}
			}
		}
		else if(botClass == PALADIN) {
			if (!AICastSpell(this, 100, SpellType_Heal)) {
				if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 30, MobAISpellRange, SpellType_Heal)) {
					if (!AICastSpell(GetTarget(), 50, SpellType_Nuke)) {
						//
					}
				}
			}

			result = true;
		}
		else if(botClass == SHADOWKNIGHT) {
			if (!AICastSpell(this, 100, SpellType_Pet)) {
				if (!AICastSpell(GetTarget(), 100, SpellType_Lifetap)) {
					if (!AICastSpell(GetPet(), 100, SpellType_Heal)) {
						if (!AICastSpell(GetTarget(), 50, SpellType_DOT)) {
							if (!AICastSpell(GetTarget(), 50, SpellType_Nuke)) {
								//
							}
						}
					}
				}
			}

			result = true;
		}
		else if(botClass == MAGICIAN) {
			if (!AICastSpell(this, 100, SpellType_Pet)) {
				if (!AICastSpell(GetPet(), 100, SpellType_Heal)) {
					if (!AICastSpell(GetTarget(), 100, SpellType_Nuke)) {
						//
					}
				}
			}

			result = true;
		}
		else if(botClass == NECROMANCER) {
			if (!AICastSpell(this, 100, SpellType_Pet)) {
				if (!AICastSpell(GetTarget(), 100, SpellType_Lifetap)) {
					if (!AICastSpell(GetTarget(), 100, SpellType_DOT)) {
						if (!AICastSpell(GetPet(), 100, SpellType_Heal)) {
							if (!AICastSpell(GetTarget(), 100, SpellType_Nuke)) {
								//
							}
						}
					}
				}
			}

			result = true;
		}
		else if(botClass == ENCHANTER) {
			if (!AICastSpell(GetTarget(), 100, SpellType_Mez)) {
				if (!AICastSpell(GetTarget(), 100, SpellType_Slow)) {
					if (!AICastSpell(this, 100, SpellType_Pet)) {
						if (!AICastSpell(GetTarget(), 100, SpellType_DOT)) {
							if (!AICastSpell(GetTarget(), 50, SpellType_Nuke)) {
								if(!AICastSpell(GetTarget(), 50, SpellType_Escape)) {
									//
								}
							}
						}
					}
				}
			}

			result = true;
		}
		else if(botClass == BARD) {
			if(!AICastSpell(this, 100, SpellType_Buff)) {
				if(!AICastSpell(GetTarget(), 100, SpellType_Nuke | SpellType_Dispel | SpellType_Escape)) {// Bards will use their debuff songs
					//
				}					
			}

			result = true;
		}

		if(!AIautocastspell_timer->Enabled()) {
			AIautocastspell_timer->Start(RandomTimer(100, 250), false);
		}
	}

	return result;
}

std::list<BotSpell> Bot::GetBotSpellsForSpellEffect(Bot* botCaster, int spellEffect) {
	std::list<BotSpell> result;

	if(botCaster && botCaster->AI_HasSpells()) {
		std::vector<AISpells_Struct> botSpellList = botCaster->GetBotSpells();

		for (int i = botSpellList.size() - 1; i >= 0; i--) {
			if (botSpellList[i].spellid <= 0 || botSpellList[i].spellid >= SPDAT_RECORDS) {
				// this is both to quit early to save cpu and to avoid casting bad spells
				// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
				continue;
			}

			if(IsEffectInSpell(botSpellList[i].spellid, spellEffect)) {
				BotSpell botSpell;
				botSpell.SpellId = botSpellList[i].spellid;
				botSpell.SpellIndex = i;
				botSpell.ManaCost = botSpellList[i].manacost;

				result.push_back(botSpell);
			}
		}
	}

	return result;
}

std::list<BotSpell> Bot::GetBotSpellsForSpellEffectAndTargetType(Bot* botCaster, int spellEffect, SpellTargetType targetType) {
	std::list<BotSpell> result;

	if(botCaster && botCaster->AI_HasSpells()) {
		std::vector<AISpells_Struct> botSpellList = botCaster->GetBotSpells();

		for (int i = botSpellList.size() - 1; i >= 0; i--) {
			if (botSpellList[i].spellid <= 0 || botSpellList[i].spellid >= SPDAT_RECORDS) {
				// this is both to quit early to save cpu and to avoid casting bad spells
				// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
				continue;
			}

			if(IsEffectInSpell(botSpellList[i].spellid, spellEffect)) {
				if(spells[botSpellList[i].spellid].targettype == targetType) {
					BotSpell botSpell;
					botSpell.SpellId = botSpellList[i].spellid;
					botSpell.SpellIndex = i;
					botSpell.ManaCost = botSpellList[i].manacost;

					result.push_back(botSpell);
				}
			}
		}
	}

	return result;
}

std::list<BotSpell> Bot::GetBotSpellsBySpellType(Bot* botCaster, int16 spellType) {
	std::list<BotSpell> result;

	if(botCaster && botCaster->AI_HasSpells()) {
		std::vector<AISpells_Struct> botSpellList = botCaster->GetBotSpells();

		for (int i = botSpellList.size() - 1; i >= 0; i--) {
			if (botSpellList[i].spellid <= 0 || botSpellList[i].spellid >= SPDAT_RECORDS) {
				// this is both to quit early to save cpu and to avoid casting bad spells
				// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
				continue;
			}

			if(botSpellList[i].type & spellType) {
				BotSpell botSpell;
				botSpell.SpellId = botSpellList[i].spellid;
				botSpell.SpellIndex = i;
				botSpell.ManaCost = botSpellList[i].manacost;

				result.push_back(botSpell);
			}
		}
	}

	return result;
}

BotSpell Bot::GetFirstBotSpellBySpellType(Bot* botCaster, int16 spellType) {
	BotSpell result;
	
	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if(botCaster && botCaster->AI_HasSpells()) {
		std::vector<AISpells_Struct> botSpellList = botCaster->GetBotSpells();

		for (int i = botSpellList.size() - 1; i >= 0; i--) {
			if (botSpellList[i].spellid <= 0 || botSpellList[i].spellid >= SPDAT_RECORDS) {
				// this is both to quit early to save cpu and to avoid casting bad spells
				// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
				continue;
			}

			if(botSpellList[i].type & spellType) {
				result.SpellId = botSpellList[i].spellid;
				result.SpellIndex = i;
				result.ManaCost = botSpellList[i].manacost;
				
				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForFastHeal(Bot *botCaster) {
	BotSpell result;
	
	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if(botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, SE_CurrentHP);

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); botSpellListItr++) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsFastHealSpell(botSpellListItr->SpellId)) {
				result.SpellId = botSpellListItr->SpellId;
				result.SpellIndex = botSpellListItr->SpellIndex;
				result.ManaCost = botSpellListItr->ManaCost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForHealOverTime(Bot* botCaster) {
	BotSpell result;
	
	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if(botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, SE_HealOverTime);

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); botSpellListItr++) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsHealOverTimeSpell(botSpellListItr->SpellId)) {
				result.SpellId = botSpellListItr->SpellId;
				result.SpellIndex = botSpellListItr->SpellIndex;
				result.ManaCost = botSpellListItr->ManaCost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForPercentageHeal(Bot *botCaster) {
	BotSpell result;
	
	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if(botCaster && botCaster->AI_HasSpells()) {
		std::vector<AISpells_Struct> botSpellList = botCaster->GetBotSpells();

		for (int i = botSpellList.size() - 1; i >= 0; i--) {
			if (botSpellList[i].spellid <= 0 || botSpellList[i].spellid >= SPDAT_RECORDS) {
				// this is both to quit early to save cpu and to avoid casting bad spells
				// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
				continue;
			}

			if(IsCompleteHealSpell(botSpellList[i].spellid)) {
				result.SpellId = botSpellList[i].spellid;
				result.SpellIndex = i;
				result.ManaCost = botSpellList[i].manacost;
				
				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForRegularSingleTargetHeal(Bot* botCaster) {
	BotSpell result;
	
	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if(botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, SE_CurrentHP);

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); botSpellListItr++) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsRegularSingleTargetHealSpell(botSpellListItr->SpellId)) {
				result.SpellId = botSpellListItr->SpellId;
				result.SpellIndex = botSpellListItr->SpellIndex;
				result.ManaCost = botSpellListItr->ManaCost;
				
				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForMez(Bot* botCaster) {
	BotSpell result;
	
	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if(botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, SE_Mez);

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); botSpellListItr++) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsMezSpell(botSpellListItr->SpellId)) {
				result.SpellId = botSpellListItr->SpellId;
				result.SpellIndex = botSpellListItr->SpellIndex;
				result.ManaCost = botSpellListItr->ManaCost;
				
				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForMagicBasedSlow(Bot* botCaster) {
	BotSpell result;
	
	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if(botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, SE_AttackSpeed);

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); botSpellListItr++) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsSlowSpell(botSpellListItr->SpellId) && spells[botSpellListItr->SpellId].resisttype == RESIST_MAGIC) {
				result.SpellId = botSpellListItr->SpellId;
				result.SpellIndex = botSpellListItr->SpellIndex;
				result.ManaCost = botSpellListItr->ManaCost;
				
				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForDiseaseBasedSlow(Bot* botCaster) {
	BotSpell result;
	
	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if(botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, SE_AttackSpeed);

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); botSpellListItr++) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsSlowSpell(botSpellListItr->SpellId) && spells[botSpellListItr->SpellId].resisttype == RESIST_DISEASE) {
				result.SpellId = botSpellListItr->SpellId;
				result.SpellIndex = botSpellListItr->SpellIndex;
				result.ManaCost = botSpellListItr->ManaCost;
				
				break;
			}
		}
	}

	return result;
}

Mob* Bot::GetFirstIncomingMobToMez(Bot* botCaster, BotSpell botSpell) {
	Mob* result = 0;

	if(botCaster && IsMezSpell(botSpell.SpellId)) {
		for(std::list<NPC*>::iterator itr = entity_list.GetNPCList().begin(); itr != entity_list.GetNPCList().end(); itr++) {
			NPC* npc = *itr;

			if(npc->DistNoRootNoZ(*botCaster) <= spells[botSpell.SpellId].range) {
				if(!npc->IsMezzed()) {
					if(botCaster->HasGroup()) {
						Group* g = botCaster->GetGroup();

						if(g) {
							for(int counter = 0; counter < g->GroupCount(); counter++) {
								if(npc->IsOnHatelist(g->members[counter]) && g->members[counter]->GetTarget() != npc && g->members[counter]->IsEngaged()) {
									result = npc;
									break;
								}
							}
						}
					}
				}
			}

			if(result)
				break;
		}
	}

	return result;
}

BotSpell Bot::GetBestBotMagicianPetSpell(Bot *botCaster) {
	BotSpell result;
	
	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if(botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, SE_SummonPet);

		std::string petType = GetBotMagicianPetType(botCaster);

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); botSpellListItr++) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsSummonPetSpell(botSpellListItr->SpellId)) {
				if(!strncmp(spells[botSpellListItr->SpellId].teleport_zone, petType.c_str(), petType.length())) {
					result.SpellId = botSpellListItr->SpellId;
					result.SpellIndex = botSpellListItr->SpellIndex;
					result.ManaCost = botSpellListItr->ManaCost;

					break;
				}
			}
		}
	}

	return result;
}

std::string Bot::GetBotMagicianPetType(Bot* botCaster) {
	std::string result;

	if(botCaster) {
		if(botCaster->IsPetChooser()) {
			switch(botCaster->GetPetChooserID()) {
				case 0:
					result = std::string("SumWater");
					break;
				case 1:
					result = std::string("SumFire");
					break;
				case 2:
					result = std::string("SumAir");
					break;
				case 3:
					result = std::string("SumEarth");
					break;
				default:
					result = std::string("MonsterSum");
					break;
			}
		}
		else {
			if(botCaster->GetLevel() == 2)
				result = std::string("SumWater");
			else if(botCaster->GetLevel() == 3)
				result = std::string("SumFire");
			else if(botCaster->GetLevel() == 4)
				result = std::string("SumAir");
			else if(botCaster->GetLevel() == 5)
				result = std::string("SumEarth");
			else if(botCaster->GetLevel() < 30) {
				// Under level 30
				int counter = MakeRandomInt(0, 3);
				
				switch(counter) {
					case 0:
					result = std::string("SumWater");
					break;
				case 1:
					result = std::string("SumFire");
					break;
				case 2:
					result = std::string("SumAir");
					break;
				case 3:
					result = std::string("SumEarth");
					break;
				default:
					result = std::string("MonsterSum");
					break;
				}
			}
			else {
				// Over level 30
				int counter = MakeRandomInt(0, 4);
				
				switch(counter) {
					case 0:
					result = std::string("SumWater");
					break;
				case 1:
					result = std::string("SumFire");
					break;
				case 2:
					result = std::string("SumAir");
					break;
				case 3:
					result = std::string("SumEarth");
					break;
				default:
					result = std::string("MonsterSum");
					break;
				}
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForNukeByTargetType(Bot* botCaster, SpellTargetType targetType) {
	BotSpell result;
	
	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if(botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffectAndTargetType(botCaster, SE_CurrentHP, targetType);

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); botSpellListItr++) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsPureNukeSpell(botSpellListItr->SpellId)) {
				result.SpellId = botSpellListItr->SpellId;
				result.SpellIndex = botSpellListItr->SpellIndex;
				result.ManaCost = botSpellListItr->ManaCost;
				
				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotWizardNukeSpellByTargetResists(Bot* botCaster, Mob* target) {
	BotSpell result;
	
	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if(botCaster && target) {
		const int lureResisValue = -100;
		const int maxTargetResistValue = 300;
		bool selectLureNuke = false;

		if((target->GetMR() > maxTargetResistValue) && (target->GetCR() > maxTargetResistValue) && (target->GetFR() > maxTargetResistValue))
			selectLureNuke = true;
				

		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffectAndTargetType(botCaster, SE_CurrentHP, ST_Target);

		BotSpell firstWizardMagicNukeSpellFound;
		firstWizardMagicNukeSpellFound.SpellId = 0;
		firstWizardMagicNukeSpellFound.SpellIndex = 0;
		firstWizardMagicNukeSpellFound.ManaCost = 0;

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); botSpellListItr++) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			bool spellSelected = false;

			if(selectLureNuke && (spells[botSpellListItr->SpellId].ResistDiff < lureResisValue)) {
				spellSelected = true;
			}
			else if(IsStunSpell(botSpellListItr->SpellId) && !target->SpecAttacks[UNSTUNABLE] && (MakeRandomInt(0, 5) == 5)) {
				spellSelected = true;
			}
			else if(IsPureNukeSpell(botSpellListItr->SpellId)) {
				if(((target->GetMR() < target->GetCR()) || (target->GetMR() < target->GetFR())) && (GetSpellResistType(botSpellListItr->SpellId) == RESIST_MAGIC) 
					&& (spells[botSpellListItr->SpellId].ResistDiff > lureResisValue))
				{
					spellSelected = true;
				}
				else if(((target->GetCR() < target->GetMR()) || (target->GetCR() < target->GetFR())) && (GetSpellResistType(botSpellListItr->SpellId) == RESIST_COLD) 
					&& (spells[botSpellListItr->SpellId].ResistDiff > lureResisValue)) 
				{
					spellSelected = true;
				}
				else if(((target->GetFR() < target->GetCR()) || (target->GetFR() < target->GetMR())) && (GetSpellResistType(botSpellListItr->SpellId) == RESIST_FIRE)
					&& (spells[botSpellListItr->SpellId].ResistDiff > lureResisValue)) 
				{
					spellSelected = true;
				}
				else if((GetSpellResistType(botSpellListItr->SpellId) == RESIST_MAGIC) && (spells[botSpellListItr->SpellId].ResistDiff > lureResisValue)) {
					firstWizardMagicNukeSpellFound.SpellId = botSpellListItr->SpellId;
					firstWizardMagicNukeSpellFound.SpellIndex = botSpellListItr->SpellIndex;
					firstWizardMagicNukeSpellFound.ManaCost = botSpellListItr->ManaCost;
				}
			}

			if(spellSelected) {
				result.SpellId = botSpellListItr->SpellId;
				result.SpellIndex = botSpellListItr->SpellIndex;
				result.ManaCost = botSpellListItr->ManaCost;

				break;
			}
		}

		if(result.SpellId == 0) {
			result = firstWizardMagicNukeSpellFound;
		}
	}

	return result;
}

#endif