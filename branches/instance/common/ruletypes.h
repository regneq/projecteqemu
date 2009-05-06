


#ifndef RULE_CATEGORY
#define RULE_CATEGORY(name)
#endif
#ifndef RULE_INT
#define RULE_INT(cat, rule, default_value)
#endif
#ifndef RULE_REAL
#define RULE_REAL(cat, rule, default_value)
#endif
#ifndef RULE_BOOL
#define RULE_BOOL(cat, rule, default_value)
#endif
#ifndef RULE_CATEGORY_END
#define RULE_CATEGORY_END()
#endif




RULE_CATEGORY( Character )
RULE_INT ( Character, MaxLevel, 65 )
RULE_INT ( Character, MaxExpLevel, 0 ) //Sets the Max Level attainable via Experience
RULE_INT ( Character, DeathExpLossLevel, 10 )
RULE_INT ( Character, DeathItemLossLevel, 10 )
RULE_INT ( Character, DeathExpLossMultiplier, 3) //Adjust how much exp is lost
RULE_BOOL( Character, UseDeathExpLossMult, true ) //Adjust to use the above multiplier or to use code default.
RULE_INT ( Character, CorpseDecayTimeMS, 10800000 )
RULE_BOOL( Character, LeaveCorpses, false )
RULE_BOOL( Character, LeaveNakedCorpses, false )
RULE_REAL( Character, ExpMultiplier, 0.5 )
RULE_REAL( Character, AAExpMultiplier, 0.5 )
RULE_REAL( Character, GroupExpMultiplier, 0.5 )
RULE_REAL( Character, RaidExpMultiplier, 0.2 )
RULE_INT ( Character, AutosaveIntervalS, 300 )	//0=disabled
RULE_INT ( Character, HPRegenMultiplier, 100)
RULE_INT ( Character, ManaRegenMultiplier, 100)
RULE_INT ( Character, EnduranceRegenMultiplier, 100)
RULE_INT ( Character, ConsumptionMultiplier, 100) //item's hunger restored = this value * item's food level, 100 = normal, 50 = people eat 2x as fast, 200 = people eat 2x as slow
RULE_BOOL( Character, HealOnLevel, false)
RULE_BOOL( Character, FeignKillsPet, false)
RULE_INT ( Character, ItemManaRegenCap, 15)
RULE_INT ( Character, ItemHealthRegenCap, 35)
RULE_INT ( Character, ItemDamageShieldCap, 30)
RULE_INT ( Character, ItemAccuracyCap, 150)
RULE_INT ( Character, ItemAvoidanceCap, 100)
RULE_INT ( Character, ItemCombatEffectsCap, 100)
RULE_INT ( Character, ItemShieldingCap, 35)
RULE_INT ( Character, ItemSpellShieldingCap, 35)
RULE_INT ( Character, ItemDoTShieldingCap, 35)
RULE_INT ( Character, ItemStunResistCap, 35)
RULE_INT ( Character, ItemStrikethroughCap, 35)
RULE_INT ( Character, SkillUpModifier, 100) //skill ups are at 100%
RULE_BOOL ( Character, SharedBankPlat, false) //off by default to prevent duping for now
RULE_BOOL ( Character, BindAnywhere, false)
RULE_INT ( Character, RestRegenPercent, 0) // Set to >0 to enable rest state bonus HP and mana regen.
RULE_INT ( Character, RestRegenTimeToActivate, 30) // Time in seconds for rest state regen to kick in.
RULE_CATEGORY_END()

RULE_CATEGORY( Guild )
RULE_INT ( Guild, MaxMembers, 2048 )
RULE_CATEGORY_END()

RULE_CATEGORY( Skills )
RULE_INT ( Skills, MaxTrainTradeskills, 21 )
RULE_CATEGORY_END()

RULE_CATEGORY( Pets )
RULE_REAL( Pets, AttackCommandRange, 150 )
RULE_CATEGORY_END()

RULE_CATEGORY( GM )
RULE_INT ( GM, MinStatusToZoneAnywhere, 250 )
RULE_CATEGORY_END()

RULE_CATEGORY( World )
RULE_INT ( World, ZoneAutobootTimeoutMS, 60000 )
RULE_INT ( World, ClientKeepaliveTimeoutMS, 65000 )
RULE_BOOL ( World, UseBannedIPsTable, false ) //Lieka Edit: Toggle whether or not to check incoming client connections against the Banned_IPs table. Set this value to false to disable this feature.
RULE_BOOL ( World, EnableTutorialButton, true)
RULE_BOOL ( World, EnableReturnHomeButton, true)
RULE_INT  ( World, MaxLevelForTutorial, 10)
RULE_INT  ( World, TutorialZoneID, 189)
RULE_INT  ( World, MinOfflineTimeToReturnHome, 21600) // 21600 seconds is 6 Hours
RULE_INT ( World, MaxClientsPerIP, -1 ) //Lieka Edit: Maximum number of clients allowed to connect per IP address if account status is < AddMaxClientsStatus.  Default value: -1 (feature disabled)
RULE_INT ( World, ExemptMaxClientsStatus, -1 ) //Lieka Edit: Exempt accounts from the MaxClientsPerIP and AddMaxClientsStatus rules, if their status is >= this value.  Default value: -1 (feature disabled)
RULE_INT ( World, AddMaxClientsPerIP, -1 ) //Trevius Edit: Maximum number of clients allowed to connect per IP address if account status is < ExemptMaxClientsStatus.  Default value: -1 (feature disabled)
RULE_INT ( World, AddMaxClientsStatus, -1 ) //Trevius Edit: Accounts with status >= this rule will be allowed to use the amount of accounts defined in the AddMaxClientsPerIP.  Default value: -1 (feature disabled)
RULE_BOOL ( World, ClearTempMerchantlist, true) //cavedude: Clears temp merchant items when world boots.
RULE_INT ( World, AccountSessionLimit, -1 )  //Max number of characters allowed on at once from a single account (-1 is disabled)
RULE_INT ( World, ExemptAccountLimitStatus, -1 )  //Min status required to be exempt from multi-session per account limiting (-1 is disabled)
RULE_BOOL ( World, GMAccountIPList, false) // Voidd: Check ip list against GM Accounts, AntiHack GM Accounts.
RULE_INT ( World, MinGMAntiHackStatus, 1 ) //Minimum GM status to check against AntiHack list
RULE_INT ( World, SoFStartZoneID, -1 ) //Sets the Starting Zone for SoF Clients separate from Titanium Clients (-1 is disabled)
RULE_CATEGORY_END()

RULE_CATEGORY( Zone )
RULE_INT ( Zone,  NPCGlobalPositionUpdateInterval, 60000 ) //ms between intervals of sending a position update to the entire zone.
RULE_INT ( Zone,  ClientLinkdeadMS, 180000) //the time a client remains link dead on the server after a sudden disconnection
RULE_INT ( Zone,  GraveyardTimeMS, 1200000) //ms time until a player corpse is moved to a zone's graveyard, if one is specified for the zone
RULE_BOOL ( Zone, EnableShadowrest, 0 ) // enables or disables the shadowrest zone feature for player corpses. Default is turned off.
RULE_INT ( Zone, MQWarpExemptStatus, -1 ) //Lieka:  Required status level to exempt the MQWarpDetector.  Set to -1 to disable this feature.
RULE_INT ( Zone, MQZoneExemptStatus, -1 ) //Lieka:  Required status level to exempt the MQZoneDetector.  Set to -1 to disable this feature.
RULE_INT ( Zone, MQGateExemptStatus, -1 ) //Lieka:  Required status level to exempt the MQGateDetector.  Set to -1 to disable this feature.
RULE_INT ( Zone, MQGhostExemptStatus, -1 ) //Lieka:  Required status level to exempt the MGhostDetector.  Set to -1 to disable this feature.
RULE_BOOL ( Zone, EnableMQWarpDetector, true ) //Lieka:  Enable the MQWarp Detector.  Set to False to disable this feature.
RULE_BOOL ( Zone, EnableMQZoneDetector, true ) //Lieka:  Enable the MQZone Detector.  Set to False to disable this feature.
RULE_BOOL ( Zone, EnableMQGateDetector, true ) //Lieka:  Enable the MQGate Detector.  Set to False to disable this feature.
RULE_BOOL ( Zone, EnableMQGhostDetector, true ) //Lieka:  Enable the MQGhost Detector.  Set to False to disable this feature.
RULE_REAL ( Zone, MQWarpDetectorDistance, 4900 ) //Lieka:  Distance a player must travel between client to server location updates before a warp is registered.  30 allows for beyond GM speed without lag.
RULE_REAL ( Zone, MQWarpLagThreshold, 140 ) //Lieka:  Distance beyond the Zone:MQWarpDetectorDistance that a player must travel within the MQWarpThresholdTimer amount of time before tripping the MQWarp detector.  Set to 0 to disable this feature.
RULE_REAL ( Zone, MQWarpThresholdTimer, 90000 ) //Lieka:  Amount of time before the warp_threshold resets to the Zone:MQWarpLagThreshold value.  Default: 90000 (900 seconds/15 minutes).  Set to -1 to disable this feature.
RULE_INT ( Zone, AutoShutdownDelay, 5000 ) //How long a dynamic zone stays loaded while empty
RULE_CATEGORY_END()

RULE_CATEGORY( Map )
//enable these to help prevent mob hopping when they are pathing
RULE_BOOL ( Map, FixPathingZWhenLoading, true )		//increases zone boot times a bit to reduce hopping.
RULE_BOOL ( Map, FixPathingZAtWaypoints, false )	//alternative to `WhenLoading`, accomplishes the same thing but does it at each waypoint instead of once at boot time.
RULE_BOOL ( Map, FixPathingZWhenMoving, false )		//very CPU intensive, but helps hopping with widely spaced waypoints.
RULE_BOOL ( Map, FixPathingZOnSendTo, false )		//try to repair Z coords in the SendTo routine as well.
RULE_REAL ( Map, FixPathingZMaxDeltaMoving, 20 )	//at runtime while pathing: max change in Z to allow the BestZ code to apply.
RULE_REAL ( Map, FixPathingZMaxDeltaWaypoint, 20 )	//at runtime at each waypoint: max change in Z to allow the BestZ code to apply.
RULE_REAL ( Map, FixPathingZMaxDeltaSendTo, 20 )	//at runtime in SendTo: max change in Z to allow the BestZ code to apply.
RULE_REAL ( Map, FixPathingZMaxDeltaLoading, 45 )	//while loading each waypoint: max change in Z to allow the BestZ code to apply.
RULE_CATEGORY_END()

RULE_CATEGORY( Watermap )
// enable these to use the water detection code. Requires Water Maps generated by awater utility
RULE_BOOL ( Watermap, CheckWaypointsInWaterWhenLoading, false ) // Does not apply BestZ as waypoints are loaded if they are in water
RULE_BOOL ( Watermap, CheckForWaterAtWaypoints, false) 		// Check if a mob has moved into/out of water when at waypoints and sets flymode
RULE_BOOL ( Watermap, CheckForWaterWhenMoving, false)		// Checks if a mob has moved into/out of water each time it's loc is recalculated
RULE_BOOL ( Watermap, CheckForWaterOnSendTo, false)		// Checks if a mob has moved into/out of water on SendTo
RULE_BOOL ( Watermap, CheckForWaterWhenFishing, false)		// Only lets a player fish near water (if a water map exists for the zone)
RULE_REAL ( Watermap, FishingRodLength, 30)			// How far in front of player water must be for fishing to work
RULE_REAL ( Watermap, FishingLineLength, 40)			// If water is more than this far below the player, it is considered too far to fish
RULE_CATEGORY_END()

RULE_CATEGORY( Spells )
RULE_INT ( Spells, AutoResistDiff, 15)
RULE_REAL ( Spells, ResistChance, 2.0) //chance to resist given no resists and same level
RULE_REAL ( Spells, ResistMod, 0.40) //multiplier, chance to resist = this * ResistAmount
RULE_REAL ( Spells, PartialHitChance, 0.7) //The chance when a spell is resisted that it will partial hit.
RULE_REAL ( Spells, PartialHitChanceFear, 0.25) //The chance when a fear spell is resisted that it will partial hit.
RULE_INT ( Spells, BaseCritChance, 0) //base % chance that everyone has to crit a spell
RULE_INT ( Spells, BaseCritRatio, 100) //base % bonus to damage on a successful spell crit. 100 = 2x damage
RULE_INT ( Spells, WizCritLevel, 12) //level wizards first get spell crits
RULE_INT ( Spells, WizCritChance, 7) //wiz's crit chance, on top of BaseCritChance
RULE_INT ( Spells, WizCritRatio, 0) //wiz's crit bonus, on top of BaseCritRatio (should be 0 for Live-like)
RULE_INT (Spells, ResistPerLevelDiff, 85) //8.5 resist per level difference.
RULE_INT (Spells, TranslocateTimeLimit, 0) // If not zero, time in seconds to accept a Translocate.
RULE_CATEGORY_END()

RULE_CATEGORY( Combat )
RULE_REAL ( Combat, BaseCritChance, 0.0 ) //The base crit chance for non warriors, NOTE: This will apply to NPCs as well
RULE_REAL ( Combat, WarBerBaseCritChance, 0.03 ) //The base crit chance for warriors and berserkers, only applies to clients
RULE_REAL ( Combat, BerserkBaseCritChance, 0.06 ) //The bonus base crit chance you get when you're berserk
RULE_INT ( Combat, NPCBashKickLevel, 6 ) //The level that npcs can KICK/BASH
RULE_REAL ( Combat, ClientBaseCritChance, 0.0 ) //The base crit chance for all clients, this will stack with warrior's/zerker's crit chance.
RULE_BOOL ( Combat, UseIntervalAC, true)
RULE_INT ( Combat, PetAttackMagicLevel, 30)
RULE_BOOL ( Combat, EnableFearPathing, true)
RULE_INT ( Combat, FleeHPRatio, 25)
RULE_BOOL ( Combat, FleeIfNotAlone, false) // If false, mobs won't flee if other mobs are in combat with it.
RULE_BOOL ( Combat, AdjustProcPerMinute, true)
RULE_REAL ( Combat, AvgProcsPerMinute, 18.0)
RULE_REAL ( Combat, ProcPerMinDexContrib, 0.075)
RULE_REAL ( Combat, BaseProcChance, 0.035)
RULE_REAL ( Combat, ProcDexDivideBy, 11000)
RULE_REAL ( Combat, BaseHitChance, 69.0)
RULE_REAL ( Combat, HitFalloffMinor, 5.0) //hit will fall off up to 5% over the initial level range
RULE_REAL ( Combat, HitFalloffModerate, 7.0) //hit will fall off up to 7% over the three levels after the initial level range
RULE_REAL ( Combat, HitFalloffMajor, 50.0) //hit will fall off sharply if we're outside the minor and moderate range
RULE_REAL ( Combat, HitBonusPerLevel, 0.4) //You gain this % of hit for every level you are above your target
RULE_REAL ( Combat, WeaponSkillFalloff, 0.33) //For every weapon skill point that's not maxed you lose this % of hit
RULE_REAL ( Combat, ArcheryHitPenalty, 0.25) //Archery has a hit penalty to try to help balance it with the plethora of long term +hit modifiers for it
RULE_REAL ( Combat, AgiHitFactor, 0.01) 
RULE_INT ( Combat, MinRangedAttackDist, 25) //Minimum Distance to use Ranged Attacks 
RULE_CATEGORY_END()

RULE_CATEGORY( NPC )
RULE_INT ( NPC, MinorNPCCorpseDecayTimeMS, 450000 ) //level<55
RULE_INT ( NPC, MajorNPCCorpseDecayTimeMS, 1500000 ) //level>=55
RULE_INT ( NPC, CorpseUnlockTimer, 150000 )
RULE_INT ( NPC, EmptyNPCCorpseDecayTimeMS, 0 )
RULE_BOOL (NPC, UseItemBonusesForNonPets, true)
RULE_INT ( NPC, SayPauseTimeInSec, 5)
RULE_INT ( NPC, OOCRegen, 0)
RULE_BOOL ( NPC, BuffFriends, false )
RULE_BOOL ( NPC, EnableNPCQuestJournal, false)
RULE_CATEGORY_END()

RULE_CATEGORY ( Aggro )
RULE_BOOL ( Aggro, SmartAggroList, true )
RULE_INT ( Aggro, SittingAggroMod, 35 ) //35%
RULE_INT ( Aggro, MeleeRangeAggroMod, 10 ) //10%
RULE_INT ( Aggro, CurrentTargetAggroMod, 0 ) //0% --will prefer our current target to any other; makes it harder for our npcs to switch targets.
RULE_INT ( Aggro, CriticallyWoundedAggroMod, 100 ) //100%
RULE_INT ( Aggro, SlowAggroMod, 450 )
RULE_INT ( Aggro, IncapacitateAggroMod, 500 ) //mez, blind, charm etc etc
RULE_INT ( Aggro, MovementImpairAggroMod, 175 )
RULE_INT ( Aggro, SpellAggroMod, 100 )
RULE_INT ( Aggro, SongAggroMod, 33 )
RULE_INT ( Aggro, PetSpellAggroMod, 10 )
RULE_INT ( Aggro, StunAggroMod, 750 )
RULE_CATEGORY_END()

RULE_CATEGORY ( TaskSystem)
RULE_BOOL ( TaskSystem, EnableTaskSystem, true) // Globally enable or disable the Task system
RULE_INT ( TaskSystem, PeriodicCheckTimer, 5) // Seconds between checks for failed tasks. Also used by the 'Touch' activity
RULE_BOOL ( TaskSystem, RecordCompletedTasks, true)
RULE_BOOL ( TaskSystem, RecordCompletedOptionalActivities, false)
RULE_BOOL ( TaskSystem, KeepOneRecordPerCompletedTask, true)
RULE_BOOL ( TaskSystem, EnableTaskProximity, true)
RULE_CATEGORY_END()

#ifdef EQBOTS

RULE_CATEGORY ( EQOffline )
RULE_REAL ( EQOffline, BotManaRegen, 1.0 ) // Adjust mana regen for bots, 1 is fast and higher numbers slow it down 3 is about the same as players.
RULE_BOOL ( EQOffline, BotFinishBuffing, true ) // Allow for buffs to complete even if the bot caster is out of mana.  Only affects buffing out of combat.
RULE_INT ( EQOffline, CreateBotCount, 150 ) // Number of bots that each account can create
RULE_INT ( EQOffline, SpawnBotCount, 71 ) // Number of bots a character can have spawned at one time, You + 71 bots is a 12 group raid
RULE_BOOL ( EQOffline, BotQuest, false ) // Optional quest method to manage bot spawn limits using the quest_globals name bot_spawn_limit, see: /bazaar/Aediles_Thrall.pl
RULE_CATEGORY_END()

#endif //EQBOTS

RULE_CATEGORY ( Chat )
RULE_BOOL ( Chat, ServerWideOOC, true)
RULE_BOOL ( Chat, ServerWideAuction, true)
RULE_BOOL ( Chat, EnableVoiceMacros, true)
RULE_BOOL ( Chat, EnableMailKeyIPVerification, true)
RULE_BOOL ( Chat, EnableAntiSpam, true)
RULE_INT ( Chat, MinStatusToBypassAntiSpam, 100)
RULE_INT ( Chat, MinimumMessagesPerInterval, 4)
RULE_INT ( Chat, MaximumMessagesPerInterval, 12)
RULE_INT ( Chat, MaxMessagesBeforeKick, 20)
RULE_INT ( Chat, IntervalDurationMS, 60000)
RULE_INT ( Chat, KarmaUpdateIntervalMS, 1200000)
RULE_CATEGORY_END()

RULE_CATEGORY ( Merchant )
RULE_BOOL ( Merchant, UsePriceMod, true) // Use faction/charisma price modifiers.
RULE_REAL ( Merchant, SellCostMod, 1.05) // Modifier for NPC sell price.
RULE_REAL ( Merchant, BuyCostMod, 0.95) // Modifier for NPC buy price.
RULE_INT ( Merchant, PriceBonusPct, 4) // Determines maximum price bonus from having good faction/CHA. Value is a percent.
RULE_INT ( Merchant, PricePenaltyPct, 4) // Determines maximum price penalty from having bad faction/CHA. Value is a percent.
RULE_REAL( Merchant, ChaBonusMod, 3.45) // Determines CHA cap, from 104 CHA. 3.45 is 132 CHA at apprehensive. 0.34 is 400 CHA at apprehensive.
RULE_REAL ( Merchant, ChaPenaltyMod, 1.52) // Determines CHA bottom, up to 102 CHA. 1.52 is 37 CHA at apprehensive. 0.98 is 0 CHA at apprehensive.
RULE_CATEGORY_END()

RULE_CATEGORY ( Bazaar )
RULE_BOOL ( Bazaar, AuditTrail, false)
RULE_INT ( Bazaar, MaxSearchResults, 50)
RULE_BOOL ( Bazaar, EnableWarpToTrader, true)
RULE_INT ( Bazaar, MaxBarterSearchResults, 200) // The max results returned in the /barter search
RULE_CATEGORY_END()

RULE_CATEGORY ( Mail )
RULE_BOOL ( Mail, EnableMailSystem, true) // If false, client won't bring up the Mail window.
RULE_INT ( Mail, ExpireTrash, 0) // Time in seconds. 0 will delete all messages in the trash when the mailserver starts
RULE_INT ( Mail, ExpireRead, 31536000 ) // 1 Year. Set to -1 for never
RULE_INT ( Mail, ExpireUnread, 31536000 ) // 1 Year. Set to -1 for never
RULE_CATEGORY_END()

RULE_CATEGORY ( Channels )
RULE_INT ( Channels, RequiredStatusAdmin, 251) // Required status to administer chat channels
RULE_INT ( Channels, RequiredStatusListAll, 251) // Required status to list all chat channels
RULE_INT ( Channels, DeleteTimer, 1440) // Empty password protected channels will be deleted after this many minutes
RULE_CATEGORY_END()

RULE_CATEGORY ( EventLog )
RULE_BOOL ( EventLog, RecordSellToMerchant, false ) // Record sales from a player to an NPC merchant in eventlog table
RULE_BOOL ( EventLog, RecordBuyFromMerchant, false ) // Record purchases by a player from an NPC merchant in eventlog table
RULE_CATEGORY_END()

#undef RULE_CATEGORY
#undef RULE_INT
#undef RULE_REAL
#undef RULE_BOOL
#undef RULE_CATEGORY_END






