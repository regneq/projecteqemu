
#ifndef AA_H
#define AA_H

#include "../common/eq_packet_structs.h"

#define MANA_BURN 664


#include <map>
using namespace std;


#define MAX_SWARM_PETS 12	//this can change as long as you make more coords (swarm_pet_x/swarm_pet_y)

//this might be missing some, and some might not be used...
typedef enum {	//AA Targeting Constants
	aaTargetUser = 1,
	aaTargetCurrent = 2,		//use current target
	aaTargetGroup = 3,			//target group of user
	aaTargetCurrentGroup = 4,	//target group of current target
	aaTargetPet = 5				//target the user's pet
} aaTargetType;


typedef enum {
	aaActionNone 				= 0,
	aaActionAETaunt 			= 1,
	aaActionMassBuff 			= 2,
	aaActionFlamingArrows 		= 3,
	aaActionFrostArrows			= 4,
	aaActionRampage				= 5,
	aaActionSharedHealth		= 6,
	aaActionCelestialRegen		= 7,
	aaActionDireCharm			= 8,
	aaActionImprovedFamiliar	= 9,
	aaActionActOfValor			= 10,
	aaActionSuspendedMinion		= 11,
	aaActionEscape				= 12,
	aaActionBeastialAlignment	= 13,
	aaActionLeechTouch			= 14,
	aaActionProjectIllusion		= 15
} aaNonspellAction;

//use these for AAs which dont cast spells, yet need effects
//if this list grows beyond 32, more work is needed in *AAEffect
typedef enum {	//AA Effect IDs
	aaEffectMassGroupBuff = 1,
	aaEffectRampage,
	aaEffectSharedHealth,
	aaEffectFlamingArrows,
	aaEffectFrostArrows,
	aaEffectWarcry,
	aaEffectLeechTouch,
      aaEffectProjectIllusion // seveian 2008-09-23
} aaEffectType;


enum {	//leadership AA indexes
	groupAAMarkNPC = 0,
	groupAANPCHealth,
	groupAADelegateMainAssist,
	groupAADelegateMarkNPC,
	groupAA4,
	groupAA5,
	groupAAInspectBuffs,
	groupAA7,
	groupAASpellAwareness,
	groupAAOffenseEnhancement,
	groupAAManaEnhancement,
	groupAAHealthEnhancement,
	groupAAHealthRegeneration,
	groupAAFindPathToPC,
	groupAAHealthOfTargetsTarget,
	groupAA15,
	
	raidAAMarkNPC,	//0x10, things assume this is the first raid ability
	raidAANPCHealth,
	raidAADelegateMainAssist,
	raidAADelegateMarkNPC,
	raidAA4,
	raidAA5,
	raidAA6,
	raidAASpellAwareness,
	raidAAOffenseEnhancement,
	raidAAManaEnhancement,
	raidAAHealthEnhancement,
	raidAAHealthRegeneration,
	raidAAFindPathToPC,
	raidAAHealthOfTargetsTarget,
	raidAA14,
	raidAA15,
	
	_maxLeaderAA	//=32
};

#define MAX_LEADERSHIP_TIERS 6
//each progression should be 0 terminated to mark it as the end.
static const int8 LeadershipAACosts[_maxLeaderAA][MAX_LEADERSHIP_TIERS] = { 
{ 1, 2, 3, 0, 0, 0 },	//groupAAMarkNPC
{ 2, 0, 0, 0, 0, 0 },	//groupAANPCHealth
{ 4, 0, 0, 0, 0, 0 },	//groupAADelegateMainAssist
{ 4, 0, 0, 0, 0, 0 },	//groupAADelegateMarkNPC
{ 0, 0, 0, 0, 0, 0 },	//groupAA4
{ 0, 0, 0, 0, 0, 0 },	//groupAA5
{ 4, 6, 0, 0, 0, 0 },	//groupAAInspectBuffs
{ 0, 0, 0, 0, 0, 0 },	//groupAA7
{ 6, 0, 0, 0, 0, 0 },	//groupAASpellAwareness
{ 4, 5, 6, 7, 8, 0 },	//groupAAOffenseEnhancement
{ 4, 6, 8, 0, 0, 0 },	//groupAAManaEnhancement
{ 4, 6, 8, 0, 0, 0 },	//groupAAHealthEnhancement
{ 4, 6, 8, 0, 0, 0 },	//groupAAHealthRegeneration
{ 4, 0, 0, 0, 0, 0 },	//groupAAFindPathToPC
{ 7, 0, 0, 0, 0, 0 },	//groupAAHealthOfTargetsTarget
{ 0, 0, 0, 0, 0, 0 },	//groupAA15
	
{ 5, 99, 99, 0, 0, 0 },	//raidAAMarkNPC	//0x10
{ 4, 0, 0, 0, 0, 0 },	//raidAANPCHealth
{ 6, 99, 99, 0, 0, 0 },	//raidAADelegateMainAssist
{ 6, 99, 99, 0, 0, 0 },	//raidAADelegateMarkNPC
{ 0, 0, 0, 0, 0, 0 },	//raidAA4
{ 0, 0, 0, 0, 0, 0 },	//raidAA5
{ 0, 0, 0, 0, 0, 0 },	//raidAA6
{ 8, 0, 0, 0, 0, 0 },	//raidAASpellAwareness
{ 6, 99, 99, 99, 99, 0 },	//raidAAOffenseEnhancement
{ 6, 99, 99, 0, 0, 0 },	//raidAAManaEnhancement
{ 6, 99, 99, 0, 0, 0 },	//raidAAHealthEnhancement
{ 6, 99, 99, 0, 0, 0 },	//raidAAHealthRegeneration
{ 5, 0, 0, 0, 0, 0 },	//raidAAFindPathToPC
{ 9, 0, 0, 0, 0, 0 },	//raidAAHealthOfTargetsTarget
{ 0, 0, 0, 0, 0, 0 },	//raidAA14
{ 0, 0, 0, 0, 0, 0 },	//raidAA15
};

typedef enum {	//AA IDs
	aaNone = 0,
	aaInnateStrength = 2,			//works
	aaInnateStamina = 7,			//works
	aaInnateAgility = 12,			//works
	aaInnateDexterity = 17,			//works
	aaInnateIntelligence = 22,		//works
	aaInnateWisdom = 27,			//works
	aaInnateCharisma = 32,			//works
	aaInnateFireProtection = 37,		//works
	aaInnateColdProtection = 42,		//works
	aaInnateMagicProtection = 47,		//works
	aaInnatePoisonProtection = 52,		//works
	aaInnateDiseaseProtection = 57,		//works
	aaInnateRunSpeed = 62,			//works
	aaInnateRegeneration = 65,		//works
	aaInnateMetabolism = 68,		
	aaInnateLungCapacity = 71,		
	aaFirstAid = 74,			//untested
	aaHealingAdept = 77,			//untested
	aaHealingGift = 80,			//untested
	aaSpellCastingMastery = 83,		//untested
	aaSpellCastingReinforcement = 86,	//untested
	aaMentalClarity = 89,			
	aaSpellCastingFury = 92,		//untested
	aaChanellingFocus = 95,			
	aaSpellCastingSubtlety = 98,		//untested
	aaSpellCastingExpertise = 101,		//untested
	aaSpellCastingDeftness = 104,		//untested
	aaNaturalDurability = 107,		//works
	aaNaturalHealing = 110,			//untested
	aaCombatFury = 113,			//untested
	aaFearResistance = 116,			//untested
	aaFinishingBlow = 119,			//untested
	aaCombatStability = 122,
	aaCombatAgility = 125,
	aaMassGroupBuff = 128,			//untested
	aaDivineResurrection = 129,		//DB
	aaInnateInvisToUndead = 130,		//DB
	aaCelestialRegeneration = 131,		//untested
	aaBestowDivineAura = 132,		//DB
	aaTurnUndead = 133,			//DB
	aaPurifySoul = 136,			//DB
	aaQuickEvacuation = 137,			//untested
	aaExodus = 140,				//untested
	aaQuickDamage = 141,			//untested
	aaEnhancedRoot = 144,			
	aaDireCharm = 145,			//untested
	aaCannibalization = 146,			//DB
	aaQuickBuff = 147,			//untested
	aaAlchemyMastery = 150,			
	aaRabidBear = 153,			//DB
	aaManaBurn = 154,			//DB
	aaImprovedFamiliar = 155,		//untested, implemented?
	aaNexusGate = 156,			//DB
	aaUnknown54 = 157,			
	aaPermanentIllusion = 158,		
	aaJewelCraftMastery = 159,		
	aaGatherMana = 162,			//DB
	aaMendCompanion = 163,			//DB
	aaQuickSummoning = 164,			//untested
	aaFrenziedBurnout = 167,			//DB
	aaElementalFormFire = 168,		//DB
	aaElementalFormWater = 171,		//DB
	aaElementalFormEarth = 174,		//DB
	aaElementalFormAir = 177,		//DB
	aaImprovedReclaimEnergy = 180,		//untested
	aaTurnSummoned = 181,			//DB
	aaElementalPact = 182,			//DB
	aaLifeBurn = 183,			//DB
	aaDeadMesmerization = 184,		//DB
	aaFearstorm = 185,			//DB
	aaFleshToBone = 186,			//DB
	aaCallToCorpse = 187,			//DB
	aaDivineStun = 188,			//DB
	aaImprovedLayOnHands = 189,		
	aaSlayUndead = 190,			
	aaActOfValor = 193,			//DB
	aaHolySteed = 194,			//DB
	aaFearless = 195,			
	aa2HandBash = 196,			//works. handled by client?
	aaInnateCamouflage = 197,		//DB
	aaAmbidexterity = 198,			//untested
	aaArcheryMastery = 199,			//untested
	aaFletchingMastery = 202,		//removed from db?
	aaEndlessQuiver = 205,			//untested
	aaUnholySteed = 206,			//DB
	aaImprovedHarmTouch = 207,		//untested
	aaLeechTouch = 208,			//DB
	aaDeathPeace = 209,			
	aaSoulAbrasion = 210,			//untested
	aaInstrumentMastery = 213,		//untested
	aaUnknown91 = 216,			
	aaUnknown92 = 219,			
	aaUnknown93 = 222,			
	aaJamFest = 225,				
	aaUnknown95 = 228,
	aaSonicCall = 229,			
	aaCriticalMend = 230,			//untested
	aaPurifyBody = 233,			//DB
	aaChainCombo = 234,			
	aaRapidFeign = 237,			//works
	aaReturnKick = 240,			
	aaEscape = 243,				//DB
	aaPoisonMastery = 244,			
	aaDoubleRiposte = 247,			//untested
	aaQuickHide = 250,			
	aaQuickThrow = 251,			
	aaPurgePoison = 254,			//DB
	aaFlurry = 255,				//untested
	aaRampage = 258,			//untested
	aaAreaTaunt = 259,			//untested
	aaWarcry = 260,				//DB
	aaBandageWound = 263,			//untested
	aaSpellCastingReinforcementMastery = 266,	//untested
	aaSpellCastingFuryMastery = 267,	//untested
	aaExtendedNotes = 270,			//untested
	aaDragonPunch = 273,			
	aaStrongRoot = 274,			//DB
	aaSingingMastery = 275,			//untested
	aaBodyAndMindRejuvenation = 278,	//added
	aaPhysicalEnhancement = 279,		//untested
	aaAdvTrapNegotiation = 280,		//untested
	aaAcrobatics = 283,				//untested
	aaScribbleNotes = 286,			
	aaChaoticStab = 287,			//untested
	aaPetDiscipline = 288,			//added
	aaHobbleofSpirits = 289,		//DB
	aaFrenzyofSpirit = 290,			//DB
	aaParagonofSpirit = 291,		//DB
	aaAdvancedInnateStrength = 292,		//works
	aaAdvancedInnateStamina = 302,		//works
	aaAdvancedInnateAgility = 312,		//works
	aaAdvancedInnateDexterity = 322,	//works
	aaAdvancedInnateIntelligence = 332, //works
	aaAdvancedInnateWisdom = 342,		//works
	aaAdvancedInnateCharisma = 352,		//works
	aaWardingofSolusek = 362,			//works
	aaBlessingofEci = 372,				//works
	aaMarrsProtection = 382,			//works
	aaShroudofTheFaceless = 392,		//works
	aaBertoxxulousGift = 402,			//works
	aaNewTanaanCraftingMastery = 412,
	aaPlanarPower = 418,				//untested
	aaPlanarDurability = 423, //added
	aaInnateEnlightenment = 426, //added
	aaAdvancedSpellCastingMastery = 431,	//untested
	aaAdvancedHealingAdept = 434,		//untested
	aaAdvancedHealingGift = 437,		//untested
	aaCoupdeGrace = 440, //added
	aaFuryoftheAges = 443, //added
	aaMasteryofthePast = 446,			//untested
	aaLightningReflexes = 449, //added
	aaInnateDefense = 454, //added
	aaRadiantCure = 459,			//DB
	aaHastenedDivinity = 462,			//DB
	aaHastenedTurning = 465,			//DB
	aaHastenedPurificationofSoul = 468,	//DB
	aaHastenedGathering = 471,			//DB
	aaHastenedRabidity = 474,			//DB
	aaHastenedExodus = 477,				//DB
	aaHastenedRoot = 480,				//DB
	aaHastenedMending = 483,			//DB
	aaHastenedBanishment = 486,			//DB
	aaHastenedInstigation = 489,		//DB, maybe
	aaFuriousRampage = 492,				//DB
	aaHastenedPurificationoftheBody = 495,	//DB
	aaHastyExit = 498,					//DB
	aaHastenedPurification = 501,		//DB
	aaFlashofSteel = 504,
	aaDivineArbitration = 507,		//DB
	aaWrathoftheWild = 510,			//DB
	aaVirulentParalysis = 513,		//DB
	aaHarvestofDruzzil = 516,		//DB
	aaEldritchRune = 517,			//DB
	aaServantofRo = 520,			//DB
	aaWaketheDead = 523,			//DB
	aaSuspendedMinion = 526,		//untested
	aaSpiritCall = 528,			//DB
	aaCelestialRenewal = 531,		//DB
	aaAllegiantFamiliar = 533,		
	aaHandofPiety = 534,			//DB
	aaMithanielsBinding = 537,		//untested
	aaMendingoftheTranquil = 539,		
	aaRagingFlurry = 542,			
	aaGuardianoftheForest = 545,	//DB	
	aaSpiritoftheWood = 548,		//DB
	aaBestialFrenzy = 551,			//untested
	aaHarmoniousAttack = 556,		//untested
	aaKnightsAdvantage = 561,		
	aaFerocity = 564,			
	aaViscidRoots = 567,			
	aaSionachiesCrescendo = 568,		//untested
	aaAyonaesTutelage = 571,		
	aaFeignedMinion = 574,			
	aaUnfailingDivinity = 577,		
	aaAnimationEmpathy = 580,			// Implemented	
	aaRushtoJudgement = 583,		
	aaLivingShield = 586,			
	aaConsumptionoftheSoul = 589,		//untested
	aaBoastfulBellow = 592,			//DB
	aaFervrentBlessing = 593,		//untested
	aaTouchoftheWicked = 596,		//untested
	aaPunishingBlade = 599,			
	aaSpeedoftheKnight = 602,		
	aaShroudofStealth = 605,		
	aaNimbleEvasion = 606,			
	aaTechniqueofMasterWu = 611,		
	aaHostoftheElements = 616,		//DB
	aaCallofXuzl = 619,			//DB
	aaHastenedStealth = 622,		
	aaIngenuity = 625,			
	aaFleetofFoot = 628,			
	aaFadingMemories = 630,			
	aaTacticalMastery = 631,		
	aaTheftofLife = 634,			
	aaFuryofMagic = 637,
	aaFuryofMagicMastery2 = 640,		//whats the difference?
	aaProjectIllusion = 643,		
	aaHeadshot = 644,			//added
	aaEntrap = 645,				//DB
	aaUnholyTouch = 646,			//untested
	aaTotalDomination = 649,		// Implemented
	aaStalwartEndurance = 652,		
	aaQuickSummoning2 = 655,		//whats the difference?
	aaMentalClarity2 = 658,			//whats the difference?
	aaInnateRegeneration2 = 661,	//whats the difference?
	aaManaBurn2 = 664,				//whats the difference?
	aaExtendedNotes2 = 665,			//whats the difference? untested
	aaSionachiesCrescendo2 = 668,	//whats the difference? untested
	aaImprovedReclaimEnergy2 = 671,	//whats the difference? untetsed
	aaSwiftJourney = 672,			
	aaConvalescence = 674, //added 9/26/08
	aaLastingBreath = 676,
	aaPackrat = 678, //added 9/29/08
	aaHeightenedEndurance = 683,
	aaWeaponAffinity = 686,				//implemented
	aaSecondaryForte = 691,
	aaPersistantCasting = 692,
	aaTuneofPursuance = 695,
	aaImprovedInstrumentMastery = 700,
	aaImprovedSingingMastery =701,
	aaExultantBellowing = 702,
	aaEchoofTaelosia = 707,
	aaInternalMetronome = 710,
	aaPiousSupplication = 715,
	aaBeastialAlignment = 718,		//untested
	aaWrathofXuzl = 721,
	aaFeralSwipe = 723,			//DB?
	aaWardersFury = 724,
	aaWardersAlacrity = 729,
	aaPetAffinity = 734,			// Implemented
	aaMasteryofthePast2 = 735,		//whats the difference?
	aaSpellCastingSubtlety2 = 738,		//whats the difference?
	aaTouchoftheDivine = 741,
	aaDivineAvatar = 746,			//DB
	aaExquisiteBenediction = 749,		//DB
	aaQuickenedCuring = 754,
	aaNaturesBoon = 757,			//DB
	aaAdvancedTracking = 762,
	aaCriticalAffliction = 767,
	aaFuryofMagicMastery = 770,	//whats the difference?
	aaDoppelganger = 773,
	aaEnchancedForgetfulness = 776,
	aaMesmerizationMastery = 781,
	aaQuickMassGroupBuff = 782,
	aaSharedHealth = 785,
	aaElementalFury = 790,
	aaElementalAlacrity = 795,
	aaElementalAgility = 800,
	aaElementalDurability = 803,
	aaSinisterStrikes = 806,
	aaStrikethrough = 807,
	aaStonewall = 810,
	aaRapidStrikes = 815,
	aaKickMastery = 820,
	aaHightenedAwareness = 823,
	aaDestructiveForce = 828,		//DB
	aaSwarmofDecay = 831,			//DB
	aaDeathsFury = 834,
	aaQuickeningofDeath = 839,
	aaAdvancedTheftofLife = 844,
	aaTripleBackstab = 846,
	aaHastenedPiety = 849,
	aaImmobilizingBash = 852,
	aaViciousSmash = 855,
	aaRadiantCure2 = 860,			//whats the difference?
	aaPurification = 863,			
	aaPrecisionofthePathfinder = 864,
	aaCoatofThistles = 867,
	aaFlamingArrows = 872,			//untested
	aaFrostArrows = 875,			//untested
	aaSeizedOpportunity = 878,
	aaTrapCircumvention = 881,
	aaImprovedHastyExit = 886,
	aaVirulentVenom = 888,
	aaImprovedConsumptionofSoul = 893,
	aaIntenseHatred = 895,
	aaAdvancedSpiritCall = 900,
	aaCalloftheAncients = 902,		//DB
	aaSturdiness = 907,
	aaWarlordsTenacity = 912,		//DB
	aaStrengthenedStrike = 915,
	aaExtendedShielding = 918,
	aaRosFlamingFamiliar = 921,		//DB
	aaEcisIcyFamiliar = 922,		//DB
	aaDruzzilsMysticalFamiliar = 923,	//DB
	aaAdvancedFuryofMagicMastery = 924,	
	aaWardofDestruction = 926,		//DB
	aaFrenziedDevastation = 931,		//DB
	aaCombatFury2 = 934,	//whats the difference?
	aaCombatFury3 = 937,	//whats the difference?
	aaCombatFury4 = 940,	//whats the difference?
	aaFuryoftheAges2 = 943,	//whats the difference?
	aaFuryoftheAges3 = 946,	//whats the difference?
	aaFuryoftheAges4 = 949,	//whats the difference?
	aaPlanarDurability2 = 952,	//whats the difference?
	aaInnateEnlightenment2 = 955,	//whats the difference?
	aaDireCharm2 = 960,	//whats the difference?
	aaDireCharm3 = 961,	//whats the difference?
	aaTouchoftheDivine2 = 962,	//whats the difference?
	aaTouchofDecay = 967,
	aaCalloftheAncients2 = 970,	//whats the difference?
	aaImprovedVision = 975,
	aaEternalBreath = 978,
	aaBlacksmithingMastery = 979, //added 9/29/08
	aaBakingMastery = 982, //added 9/29/08
	aaBrewingMastery = 985, //added 9/29/08
	aaFletchingMastery2 = 988,	 //added 9/29/08
	aaPotteryMastery = 991, //added 9/29/08
	aaTailoringMastery = 994, //added 9/29/08
	aaSalvage = 997,
	aaOrigin = 1000, //spell
	aaChaoticPotential = 1001, //added
	aaDiscordantDefiance = 1006, //added 9/29/08
	aaTrialsofMataMuram = 1011,
	aaMysticalAttuning = 1021,
	aaDelayDeath = 1026,
	aaHealthyAura = 1031,
	aaFitness = 1036,
	aaVeteransWrath = 1041, //added 9/29/08
	aaVeteransWrath2 = 1044,	//whats the difference?
	aaVeteransWrath3 = 1047,	//whats the difference?
	aaVeteransWrath4 = 1050,	//whats the difference?
	aaDeathblow = 1053,
	aaReflexiveMastery = 1061,
	aaDefensiveInstincts = 1066,
	aaMnemonicRetention = 1071,		//Implemented
	aaExpansiveMind = 1072, //added 9/29/08
	aaSleightofHand = 1077,
	aaSleightofHand2 = 1080,	//whats the difference?
	aaHealingAdeptMastery = 1083,
	aaHealingGiftMastery = 1086,
	aaArcaneTongues = 1089,
	aaMasterofDisguise = 1092,
	aaSlipperyAttacks = 1093,
	aaImprovedCriticalAffliction = 1099,
	aaFortifiedBellowing = 1102,
	aaFuryofMagic2 = 1107,	//whats the difference?
	aaDanceofBlades = 1110,
	aaShieldofNotes = 1116,
	aaRoarofThunder = 1119,
	aaPersisentMinion = 1122,
	aaPerfectionofSpirit = 1123,
	aaReplentishCompanion = 1126,
	aaAdvancedPetDiscipline = 1129,
	aaThrowingMastery = 1131,
	aaBlurofAxes = 1134,
	aaHastenedWarCry = 1137,
	aaDeadAim = 1140,
	aaFrenziedDefense = 1143,
	aaTirelessSprint = 1146,
	aaDesperation = 1149,
	aaUntamedRage = 1150,
	aaEchoingCries = 1155,
	aaViciousFrenzy = 1158,
	aaCrazedOnslaught = 1163,
	aaOverwhelmingAttack = 1172,
	aaFuriousRage = 1175,
	aaBloodPact = 1178,
	aaShieldingResistance = 1181,
	aaHealingBoon = 1186,
	aaResplendentCure = 1189,
	aaCelestialHammer = 1192,
	aaDivineRetribution = 1195,
	aaCelestialRejuvination = 1203,
	aaFerventBenediction = 1206,
	aaSanctuary = 1209,
	aaDestructiveFury = 1210,
	aaDestructiveFury2 = 1213,	//whats the difference?
	aaBoonoftheForest = 1222,
	aaSpiritoftheGrove = 1225,
	aaCalloftheWild = 1228,
	aaSecondaryRecall = 1229,
	aaNaturesBounty = 1230,
	aaStasis = 1233,
	aaColorShock = 1239,
	aaMindOverMatter = 1242,
	aaSoothingWords = 1245,
	aaElementalSwarm = 1248,
	aaHeartofFlames = 1251,
	aaHeartofVapor = 1252,
	aaHeartofIce = 1253,
	aaHeartofStone = 1254,
	aaImitateDeath = 1255,
	aaCripplingStrike = 1256,
	aaStunningKick = 1259,
	aaEyeGouge = 1262,
	aaIronKicks = 1265,
	aaStyleoftheMimic = 1268,
	aaDeathPeace2 = 1272,	//whats the difference?
	aaArmyoftheDead = 1274,
	aaCelestialStun = 1277,
	aaHandofDevotion = 1278,
	aaSteadfastWill = 1284,
	aaShieldBlock = 1287,
	aaScoutsEfficiency = 1290,
	aaGuardianoftheGlade = 1293,
	aaTrackingMastery = 1296,
	aaFlurryofKnives = 1301,
	aaPrecision = 1304,
	aaNervesofSteel = 1307,
	aaTouchoftheCursed = 1313,
	aaSpiritualCorrosion = 1316,
	aaSoulThief = 1319,
	aaSpiritualChanneling = 1323,
	aaBoonoftheAncients = 1324,
	aaAncestralAid = 1327,
	aaResoluteDefiance = 1330,
	aaPresstheAttack = 1333,
	aaMindCrash = 1334,
	aaProlongedDestruction = 1337,
	aaRosGreaterFamiliar = 1340,
	aaEcisGreaterFamiliar = 1341,
	aaDruzzilsGreaterFamiliar = 1342,
	aaTeleportBind = 1343,
	aaDevotedFamiliar = 1344,
	aaAuspiceoftheHunter = 1345,
	aaSavageSpirit = 1348,
	aaPresstheAttack2 = 1351,	//whats the difference?
	aaCripplingStrike2 = 1352,	//whats the difference?
	aaStunningKick2 = 1353,	//whats the difference?
	aaEyeGouge2 = 1358,	//whats the difference?
	aaHighestID		//this should always be last, and should always
					//follow the highest AA ID
} aaID;


//Structure representing the database's AA actions
struct AA_DBAction {
	uint16 reuse_time;			//in seconds
	int16 spell_id;				//spell to cast, SPELL_UNKNOWN=no spell
	aaTargetType target;		//from aaTargetType
	aaNonspellAction action;	//non-spell action to take
	uint16 mana_cost;			//mana the NON-SPELL action costs
	uint16 duration;			//duration of NON-SPELL effect, 0=N/A
	aaID redux_aa;				//AA which reduces reuse time
	sint32 redux_rate;			//%/point in redux_aa reduction in reuse time
};

//Structure representing the database's swarm pet configs
struct AA_SwarmPet {
	uint8 count;		//number to summon
	uint32 npc_id;		//id from npc_types to represent it.
	int16 duration;		//how long they last, in seconds
};

//assumes that no activatable AA has more than 5 ranks
#define MAX_AA_ACTION_RANKS 5
extern AA_DBAction AA_Actions[aaHighestID][MAX_AA_ACTION_RANKS];	//[aaid][rank]
extern map<int16, AA_SwarmPet> AA_SwarmPets;	//key=spell_id

#define AA_Choose3(val, v1, v2, v3) (val==1?v1:(val==2?v2:v3))

extern map<int32,SendAA_Struct*>aas_send;


enum {	//values of AA_Action.action
	aaActionActivate = 0,
	aaActionSetEXP = 1,
	aaActionDisableEXP = 2,
	aaActionBuy = 3
};

#endif
