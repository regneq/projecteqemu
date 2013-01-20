/*  EQEMu:  Everquest Server Emulator
    Copyright (C) 2001-2002  EQEMu Development Team (http://eqemu.org)

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
#ifndef MOB_H
#define MOB_H

#include "features.h"

/* solar: macros for IsAttackAllowed, IsBeneficialAllowed */
#define _CLIENT(x) (x && x->IsClient() && !x->CastToClient()->IsBecomeNPC())
#define _NPC(x) (x && x->IsNPC() && !x->CastToMob()->GetOwnerID())
#define _BECOMENPC(x) (x && x->IsClient() && x->CastToClient()->IsBecomeNPC())
#define _CLIENTCORPSE(x) (x && x->IsCorpse() && x->CastToCorpse()->IsPlayerCorpse() && !x->CastToCorpse()->IsBecomeNPCCorpse())
#define _NPCCORPSE(x) (x && x->IsCorpse() && (x->CastToCorpse()->IsNPCCorpse() || x->CastToCorpse()->IsBecomeNPCCorpse()))
#define _CLIENTPET(x) (x && x->CastToMob()->GetOwner() && x->CastToMob()->GetOwner()->IsClient())
#define _NPCPET(x) (x && x->IsNPC() && x->CastToMob()->GetOwner() && x->CastToMob()->GetOwner()->IsNPC())
#define _BECOMENPCPET(x) (x && x->CastToMob()->GetOwner() && x->CastToMob()->GetOwner()->IsClient() && x->CastToMob()->GetOwner()->CastToClient()->IsBecomeNPC())

//Spell specialization parameters, not sure of a better place for them
#define SPECIALIZE_FIZZLE 11		//% fizzle chance reduce at 200 specialized
#define SPECIALIZE_MANA_REDUCE 12	//% mana cost reduction at 200 specialized

#define MAX_SHIELDERS 2		//I dont know if this is based on a client limit

#define APPEAR_VISIBLE	0x0003
#define APPEAR_ANIM		0x000e
#define APPEAR_SNEAK	0x000f
#define	APPEAR_AFK		0x0018
#define	APPEAR_ANON		0x0015
#define	APPEAR_SPLIT	0x001c
#define APPEAR_HEIGHT	0x001d
#define	APPEAR_HP_TIC	0x0011

#define ARCHETYPE_HYBRID	1
#define ARCHETYPE_CASTER	2
#define ARCHETYPE_MELEE		3

#define CON_GREEN		2
#define CON_LIGHTBLUE	18
#define CON_BLUE		4
#define CON_WHITE		20
#define CON_YELLOW		15
#define CON_RED			13

#define	HIGHEST_RESIST 9 //Max resist type value

//LOS Parameters:
#define HEAD_POSITION 0.9f	//ratio of GetSize() where NPCs see from
#define SEE_POSITION 0.5f	//ratio of GetSize() where NPCs try to see for LOS
#define CHECK_LOS_STEP 1.0f

#include "../common/logsys.h"
#include "entity.h"
#include "spdat.h"
#include "event_codes.h"
#include "hate_list.h"
#include "../common/Kaiyodo-LList.h"
#include "../common/skills.h"
#include "../common/bodytypes.h"
#include "../common/MiscFunctions.h"
#include "pathing.h"
#include "map.h"
#include <set>
#include <vector>
#include <string>


typedef enum {	//focus types
	focusSpellHaste = 1,
	focusSpellDuration,
	focusRange,
	focusReagentCost,
	focusManaCost,
	focusImprovedHeal,
	focusImprovedDamage,
	focusImprovedDOT,		//i dont know about this...
	focusImprovedDamage2,
	focusImprovedUndeadDamage,
	focusPetPower,
	focusResistRate,
	focusSpellHateMod,
	focusTriggerOnCast,
	focusSpellVulnerability,
	focusTwincast,
	focusSympatheticProc,
	focusSpellDamage,
	focusFF_Damage_Amount, 
	focusSpellDurByTic,
	focusSwarmPetDuration,
	focusReduceRecastTime,
	focusBlockNextSpell,
	focusHealRate,
	focusAdditionalDamage,
	focusSpellEffectiveness,
	focusIncreaseNumHits,
	focusCriticalHealRate,
	focusAdditionalHeal2,
	focusAdditionalHeal,
} focusType; //Any new FocusType needs to be added to the Mob::IsFocus function
#define HIGHEST_FOCUS	focusAdditionalHeal //Should always be last focusType in enum


/*
Used:
b,d,f,g,j,m,n,o,p,r,t
A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U,W,Y

Unused:
a,c,e,h,k,l,q,s,u,v,w,x,y,z
P,V,X
*/

enum {
	SPECATK_NONE = 0,
	SPECATK_SUMMON,				// S
	SPECATK_ENRAGE,				// E
	SPECATK_RAMPAGE,			// R
	SPECATK_AREA_RAMPAGE,		// r
	SPECATK_FLURRY,				// F
	SPECATK_TRIPLE,				// T
	SPECATK_QUAD,				// Q
	SPECATK_INNATE_DW,			// L
	SPECATK_BANE,				// b
	SPECATK_MAGICAL,			// m
	SPECATK_RANGED_ATK,			// Y
	UNSLOWABLE,					// U
	UNMEZABLE,					// M
	UNCHARMABLE,				// C
	UNSTUNABLE,					// N
	UNSNAREABLE,				// I
	UNFEARABLE,					// D
	UNDISPELLABLE,				// K
	IMMUNE_MELEE,				// A
	IMMUNE_MAGIC,				// B
	IMMUNE_FLEEING,				// f
	IMMUNE_MELEE_EXCEPT_BANE,	// O
	IMMUNE_MELEE_NONMAGICAL,	// W
	IMMUNE_AGGRO,				// H - Won't aggro, ever.
	IMMUNE_AGGRO_ON,			// G - Immune to being aggroed
	IMMUNE_CASTING_FROM_RANGE,	// g
	IMMUNE_FEIGN_DEATH,			// d
	IMMUNE_TAUNT,				// i
	NPC_TUNNELVISION,			// t
	NPC_NO_BUFFHEAL_FRIENDS,	// n
	IMMUNE_PACIFY,				// p
	LEASH,						// J - Dispell, wipe agro && return to spawn
	TETHER,						// j - Return to spawn
	DESTRUCTIBLE_OBJECT,		// o - This is only for destructible objects
	NO_HARM_FROM_CLIENT,			// Z - This is to prevent attacking NPC's period for clients
	SPECATK_MAXNUM
};

typedef enum {	//fear states
	fearStateNotFeared = 0,
	fearStateRunning,		//I am running, hoping to find a grid at my WP
	fearStateRunningForever,	//can run straight until spell ends
	fearStateGrid,			//I am allready on a fear grid
	fearStateStuck			//I cannot move somehow...
} FearState;

enum { FlyMode0 = 0, FlyMode1 = 1, Flymode2 = 2, FlyMode3 = 3 };

struct TradeEntity;
class Trade;
enum TradeState {
	TradeNone,
	Trading,
	TradeAccepted,
	TradeCompleting
};

//this is our internal representation of the BUFF struct, can put whatever we want in it
struct Buffs_Struct {
	uint16	spellid;
	uint8	casterlevel;
	uint16	casterid;		// Maybe change this to a pointer sometime, but gotta make sure it's 0'd when it no longer points to anything
	char    caster_name[64];
    int32	ticsremaining;
	uint32  counters;
	uint32	numhits; //the number of physical hits this buff can take before it fades away, lots of druid armor spells take advantage of this mixed with powerful effects
	uint32	melee_rune;
	uint32	magic_rune;
	uint8	deathSaveSuccessChance;
	uint8	deathsaveCasterAARank;
    bool	persistant_buff;
    bool	client;  //True if the caster is a client
	bool	UpdateClient;
};

struct StatBonuses {
	int16	AC;
	int32	HP;
	int32	HPRegen;
	int32	MaxHP;
	int32	ManaRegen;
	int32	EnduranceRegen;
	int32	Mana;
	int32	Endurance;
	int16	ATK;
	//would it be worth it to create a Stat_Struct?
	int16	STR;
	int16	STRCapMod;
	int16	HeroicSTR;
	int16	STA;
	int16	STACapMod;
	int16	HeroicSTA;
	int16	DEX;
	int16	DEXCapMod;
	int16	HeroicDEX;
	int16	AGI;
	int16	AGICapMod;
	int16	HeroicAGI;
	int16	INT;
	int16	INTCapMod;
	int16	HeroicINT;
	int16	WIS;
	int16	WISCapMod;
	int16	HeroicWIS;
	int16	CHA;
	int16	CHACapMod;
	int16	HeroicCHA;
	int16	MR;
	int16	MRCapMod;
	int16	HeroicMR;
	int16	FR;
	int16	FRCapMod;
	int16	HeroicFR;
	int16	CR;
	int16	CRCapMod;
	int16	HeroicCR;
	int16	PR;
	int16	PRCapMod;
	int16	HeroicPR;
	int16	DR;
	int16	DRCapMod;
	int16	HeroicDR;
	int16	Corrup;
	int16	CorrupCapMod;
	int16	HeroicCorrup;
	uint16	DamageShieldSpellID;
	int		DamageShield;						// this is damage done to mobs that attack this
	DmgShieldType	DamageShieldType;
	int		SpellDamageShield;
	int		SpellShield;
	int		ReverseDamageShield; 				// this is damage done to the mob when it attacks
	uint16	ReverseDamageShieldSpellID;
	DmgShieldType	ReverseDamageShieldType;
	int		movementspeed;
	int16	haste;
	int16	hastetype2;
	int16	hastetype3;
	int16  inhibitmelee;
	float	AggroRange;							// when calculate just replace original value with this
	float	AssistRange;
	int16	skillmod[HIGHEST_SKILL+1];
	int		effective_casting_level;
	int		reflect_chance;						// chance to reflect incoming spell
	uint16	singingMod;
	uint16	brassMod;
	uint16	percussionMod;
	uint16	windMod;
	uint16	stringedMod;
	int8	hatemod;
	int32	EnduranceReduction;

	int16  StrikeThrough;						// PoP: Strike Through %
	int16 	MeleeMitigation;					//i = Shielding
	int16 	CriticalHitChance[HIGHEST_SKILL+2];	//i
	int16 	CriticalSpellChance;				//i
	int16 	SpellCritDmgIncrease;				//i
	int16 	DotCritDmgIncrease;					//i
	int16 	CriticalHealChance;					//i
	int16 	CriticalHealOverTime;				//i
	int16 	CriticalDoTChance;					//i
	int16 	CrippBlowChance;					//
	int16 	AvoidMeleeChance;					//AvoidMeleeChance/10 == % chance i = Avoidance
	int16 	RiposteChance;						//i
	int16 	DodgeChance;						//i
	int16 	ParryChance;						//i
	int16 	DualWieldChance;					//i
	int16 	DoubleAttackChance;					//i
	int16 	TripleAttackChance;					//i
	int16 	ResistSpellChance;					//i
	int16 	ResistFearChance;					//i
	bool   	Fearless;							//i
	bool   	IsFeared;							//i
	int16 	StunResist;							//i
	int16 	MeleeSkillCheck;					//i
	uint8  	MeleeSkillCheckSkill;
	int16 	HitChance;							//HitChance/15 == % increase i = Accuracy (Item: Accuracy)
	int16 	HitChanceEffect[HIGHEST_SKILL+2];	//Spell effect Chance to Hit, straight percent increase
	int16 	DamageModifier[HIGHEST_SKILL+2];	//i
	int16 	MinDamageModifier[HIGHEST_SKILL+2]; //i
	int16 	ProcChance;							// ProcChance/10 == % increase i = CombatEffects
	int16 	ExtraAttackChance;
	int16	DoTShielding;
	int16 	DivineSaveChance[2];				// Second Chance (base1 = chance, base2 = spell on trigger)
	uint16	DeathSave[4];						// Death Pact [0](value = 1 partial 2 = full) [1]=slot [2]=LvLimit [3]=HealAmt 
	int16 	FlurryChance;
	int16	Accuracy[HIGHEST_SKILL+2];			//Accuracy/15 == % increase	[Spell Effect: Accuracy)
	int16	HundredHands;						//extra haste, stacks with all other haste  i
	int8	MeleeLifetap;						//i
	int16 	HealRate;							// Spell effect that influences effectiveness of heals
	int16 	MaxHPChange;						// Spell Effect
	int16 	SkillDmgTaken[HIGHEST_SKILL+2];		// All Skills + -1
	int32 	HealAmt;							// Item Effect
	int32 	SpellDmg;							// Item Effect
	int32 	Clairvoyance;						// Item Effect
	int16 	DSMitigation;						// Item Effect
	int16  DSMitigationOffHand;				// Lowers damage shield from off hand attacks.
	uint32 	SpellTriggers[MAX_SPELL_TRIGGER];	// Innate/Spell/Item Spells that trigger when you cast
	uint32 	SpellOnKill[MAX_SPELL_TRIGGER*3];	// Chance to proc after killing a mob
	uint32 	SpellOnDeath[MAX_SPELL_TRIGGER*2];	// Chance to have effect cast when you die
	int16 	CritDmgMob[HIGHEST_SKILL+2];		// All Skills + -1
	int16 	SkillReuseTime[HIGHEST_SKILL+1];	// Reduces skill timers
	int16 	SkillDamageAmount[HIGHEST_SKILL+2];	// All Skills + -1
	int16 	TwoHandBluntBlock;					// chance to block when wielding two hand blunt weapon
	uint16 	ItemManaRegenCap;					// Increases the amount of mana you have can over the cap(aa effect)
	int16 	GravityEffect;						// Indictor of spell effect
	bool	AntiGate;							// spell effect that prevents gating
	bool	MagicWeapon;						// spell effect that makes weapon magical
	int16	IncreaseBlockChance;				// overall block chance modifier
	uint16	PersistantCasting;					// chance to continue casting through a stun
	int 	XPRateMod;							//i
	int		HPPercCap;							//Spell effect that limits you to being healed/regening beyond a % of your max
	int		ManaPercCap;						// ^^
	int		EndPercCap;							// ^^
	bool	BlockNextSpell;						// Indicates whether the client can block a spell or not
	//uint16	BlockSpellEffect[EFFECT_COUNT];		// Prevents spells with certain effects from landing on you *no longer used
	bool	ImmuneToFlee;						// Bypass the fleeing flag
	uint16	VoiceGraft;							// Stores the ID of the mob with which to talk through
	uint16   SpellProcChance;					// chance to proc from sympathetic spell effects
	uint16	CharmBreakChance;					// chance to break charm
	int16  SongRange;							// increases range of beneficial bard songs
	uint16  HPToManaConvert;					// Uses HP to cast spells at specific conversion
	uint16 	FocusEffects[HIGHEST_FOCUS+1];		// Stores the focus effectid for each focustype you have. 
	bool	NegateEffects;						// Check if you contain a buff with negate effect. (only spellbonuses)
	int16  SkillDamageAmount2[HIGHEST_SKILL+2];	// Adds skill specific damage
	uint16	NegateAttacks[2];					// 0 = bool HasEffect   1 = Buff Slot
	uint16	MitigateMeleeRune[2];				// 0 = Mitigation value 1 = Buff Slot
	uint16	MitigateSpellRune[2];				// 0 = Mitigation value 1 = Buff Slot
	uint16	ManaAbsorbPercentDamage[2];			// 0 = Mitigation value 1 = Buff Slot
	int16  ShieldBlock;						// Chance to Shield Block
	int16  BlockBehind;						// Chance to Block Behind (with our without shield)
	//bool	AbsorbMagicAtt;						// Magic Rune *Need to be implemented for NegateEffect
	//bool	MeleeRune;							// Melee Rune *Need to be implemented for NegateEffect
	
	// AAs
	int8	Packrat;							//weight reduction for items, 1 point = 10%
	uint8	BuffSlotIncrease;					// Increases number of available buff slots
	uint16	DelayDeath;							// how far below 0 hp you can go
	int8	BaseMovementSpeed;					// Adjust base run speed, does not stack with other movement bonuses.
	uint8	IncreaseRunSpeedCap;				// Increase max run speed above cap.
	int16	DoubleSpecialAttack;				// Chance to to perform a double special attack (ie flying kick 2x)
	int16	SpecialAttackKBProc[2];				// Chance to to do a knockback from special attacks. (0 = chance 1 = Skill)
	uint8	FrontalStunResist;					// Chance to resist a frontal stun
	int16  BindWound;							// Increase amount of HP by percent.
	int16  MaxBindWound;						// Increase max amount of HP you can bind wound.
	int16  ChannelChanceSpells;				// Modify chance to channel a spell.
	int16  ChannelChanceItems;					// Modify chance to channel a items.
	uint8	SeeInvis;							// See Invs. 
	uint8	TripleBackstab;						// Chance to triple backstab
	bool	FrontalBackstabMinDmg;				// Allow frontal backstabs for min damage
	uint8   FrontalBackstabChance;				// Chance to backstab from the front for full damage
	uint8	ConsumeProjectile;					// Chance to not consume arrow.
	uint16 	ArcheryDamageModifier;				// Increase Archery Damage by percent
	bool	SecondaryDmgInc;					// Allow off hand weapon to recieve damage bonus.
	uint16	GiveDoubleAttack;					// Allow classes to double attack with a specified chance.
	int16	SlayUndead[2];						// Allow classes to do extra damage verse undead.(base1 = rate, base2 = damage mod)
	int16  PetCriticalHit;						// Allow pets to critical hit with % value.
	int16	PetAvoidance;						// Pet avoidance chance.
	int16  CombatStability;					// Melee damage mitigation.
	int16  GiveDoubleRiposte[3];				// 0=Regular Chance, 1=Skill Attack Chance, 2=Skill
	uint16	RaiseSkillCap[2];					// Raise a specific skill cap (1 = value, 2=skill)
	int16  Ambidexterity;						// Increase chance to duel wield by adding bonus 'skill'.
	int16  PetMaxHP;							// Increase the max hp of your pet.
	int16  PetFlurry;							// Chance for pet to flurry.
	uint8	MasteryofPast;						// Can not fizzle spells below this level specified in value.
	bool	GivePetGroupTarget;					// All pets to recieve group buffs. (Pet Affinity)
	int16	RootBreakChance;					// Chance root will break;
	int16  UnfailingDivinity;					// Improves chance that DI will fire + increase partial heal.
	int16  ItemHPRegenCap;						// Increase item regen cap.	
	int16  SEResist[MAX_RESISTABLE_EFFECTS*2];	// Resist chance by specific spell effects.
	int16  OffhandRiposteFail;					// chance for opponent to fail riposte with offhand attack.
	int16	ItemATKCap;							// Raise item attack cap
	int32  FinishingBlow[2];					// Chance to do a finishing blow for specified damage amount.
	uint16	FinishingBlowLvl[2];				// Sets max level an NPC can be affected by FB. (base1 = lv, base2= ???)
};

typedef struct
{
    uint16 spellID;
    uint16 chance;
    uint16 base_spellID;
} tProc;

struct Shielders_Struct {
	uint32   shielder_id;
	uint16   shielder_bonus;
};

//eventually turn this into a typedef and
//make DoAnim take it instead of int, to enforce its use.
enum {	//type arguments to DoAnim
	animKick				= 1,
	animPiercing			= 2,	//might be piercing?
	anim2HSlashing			= 3,
	anim2HWeapon			= 4,
	anim1HWeapon			= 5,
	animDualWield			= 6,
	animTailRake			= 7,	//slam & Dpunch too
	animHand2Hand			= 8,
	animShootBow			= 9,
	animRoundKick			= 11,
	animSwarmAttack			= 20,	//dunno about this one..
	animFlyingKick			= 45,
	animTigerClaw			= 46,
	animEagleStrike			= 47,

};


typedef enum {
	petFamiliar,		//only listens to /pet get lost
	petAnimation,		//does not listen to any commands
	petOther,
	petCharmed,
	petNPCFollow,
	petHatelist			//remain active as long something is on the hatelist. Don't listen to any commands
} PetType;

class AA_SwarmPetInfo {
public:
	AA_SwarmPetInfo() { target = 0; owner_id = 0; duration = NULL;}
	~AA_SwarmPetInfo() { target = 0; owner_id = 0; safe_delete(duration); }
	Mob * GetOwner() { return entity_list.GetMobID(owner_id); }
	Timer *duration;
	uint32 target; //the target ID
	uint32 owner_id;
};

typedef enum {
	SingleTarget,	// causes effect to spell_target
	AETarget,			// causes effect in aerange of target + target
	AECaster,			// causes effect in aerange of 'this'
	GroupSpell,		// causes effect to caster + target's group
	CAHateList,		// causes effect to all people on caster's hate list within some range
	DirectionalAE,
	CastActUnknown
} CastAction_type;


struct MercType {
	uint32	Type;
	uint32	ClientVersion;	
};

struct MercData {
	uint32	MercTemplateID;
	uint32	MercType;				// From dbstr_us.txt - Apprentice (330000100), Journeyman (330000200), Master (330000300)
	uint32	MercSubType;			// From dbstr_us.txt - 330020105^23^Race: Guktan<br>Type: Healer<br>Confidence: High<br>Proficiency: Apprentice, Tier V...
	uint32	CostFormula;			// To determine cost to client
	uint32	ClientVersion;				// Only send valid mercs per expansion
	uint32	NPCID;
};

class EGNode;
class MobFearState;

class Mob : public Entity {
public:
bool logpos;
	enum CLIENT_CONN_STATUS { CLIENT_CONNECTING, CLIENT_CONNECTED, CLIENT_LINKDEAD,
                          CLIENT_KICKED, DISCONNECTED, CLIENT_ERROR, CLIENT_CONNECTINGALL };
	enum eStandingPetOrder { SPO_Follow, SPO_Sit, SPO_Guard };

	//all of this shit is public due to #pf:
	float	tarx;
	float	tary;
	float	tarz;
	uint8	tar_ndx;
	float	tar_vector;
	float	tar_vx;
	float	tar_vy;
	float	tar_vz;
	float	test_vector;


	uint32	GetPRange(float x, float y, float z);
	static	uint32	RandomTimer(int min, int max);
	static	uint8	GetDefaultGender(uint16 in_race, uint8 in_gender = 0xFF);
	static	void	CreateSpawnPacket(EQApplicationPacket* app, NewSpawn_Struct* ns);
//	static	int		CheckEffectIDMatch(uint8 effectindex, uint16 spellid1, uint8 caster_level1, uint16 spellid2, uint8 caster_level2);


	virtual void	RogueBackstab(Mob* other, bool min_damage = false, int ReuseTime = 10);
	virtual void	RogueAssassinate(Mob* other); // solar
	bool	BehindMob(Mob* other = 0, float playerx = 0.0f, float playery = 0.0f) const;

	void	TriggerDefensiveProcs(const ItemInst* weapon, Mob *on, uint16 hand = 13, int damage=0);

	Mob(const char*   in_name,
	    const char*   in_lastname,
	    int32  in_cur_hp,
	    int32  in_max_hp,
	    uint8    in_gender,
	    uint16	in_race,
	    uint8    in_class,
        bodyType    in_bodytype,
	    uint8    in_deity,
	    uint8    in_level,
		uint32   in_npctype_id,
		float	in_size,
		float	in_runspeed,
	    float   in_heading,
	    float	in_x_pos,
	    float	in_y_pos,
	    float	in_z_pos,
	    uint8    in_light,
		uint8	in_texture,
		uint8	in_helmtexture,
		uint16	in_ac,
		uint16	in_atk,
		uint16	in_str,
		uint16	in_sta,
		uint16	in_dex,
		uint16	in_agi,
		uint16	in_int,
		uint16	in_wis,
		uint16	in_cha,
		uint8	in_haircolor,
		uint8	in_beardcolor,
		uint8	in_eyecolor1, // the eyecolors always seem to be the same, maybe left and right eye?
		uint8	in_eyecolor2,
		uint8	in_hairstyle,
		uint8	in_luclinface,
		uint8	in_beard,
		uint32	in_drakkin_heritage,
		uint32	in_drakkin_tattoo,
		uint32	in_drakkin_details,
		uint32	in_armor_tint[MAX_MATERIALS],
		uint8	in_aa_title,
		uint8	in_see_invis,			// see through invis
		uint8	in_see_invis_undead,		// see through invis vs. undead
		uint8	in_see_hide,
		uint8	in_see_improved_hide,
		int32	in_hp_regen,
		int32	in_mana_regen,
		uint8	in_qglobal,
		uint8	in_maxlevel,
		uint32	in_scalerate
	);
	virtual ~Mob();

	inline virtual bool IsMob() const { return true; }
	inline virtual bool InZone() const { return true; }
	virtual void SetLevel(uint8 in_level, bool command = false) { level = in_level; }
	void SendLevelAppearance();
	void SendAppearanceEffect(uint32 parm1, uint32 parm2, uint32 parm3, uint32 parm4, uint32 parm5, Client *specific_target=NULL);
    void SendTargetable(bool on, Client *specific_target = NULL);
	void QuestReward(Client *c=NULL, uint32 silver = 0, uint32 gold = 0, uint32 platinum = 0);
	void CameraEffect(uint32 duration, uint32 intensity, Client *c=NULL, bool global = false);
	void SendSpellEffect(uint32 effectid, uint32 duration, uint32 finish_delay, bool zone_wide, uint32 unk020, bool perm_effect=false, Client *c=NULL);
	void TempName(const char *newname = NULL);
    void SetTargetable(bool on);
    bool IsTargetable() const { return m_targetable; }

	virtual inline int32 GetPrimaryFaction() const { return 0; }
	virtual uint16 GetSkill(SkillType skill_num) const { return 0; } //overloaded by things which actually have skill (NPC|client)
	virtual void SendWearChange(uint8 material_slot);
	virtual void SendTextureWC(uint8 slot, uint16 texture, uint32 hero_forge_model=0, uint32 elite_material=0, uint32 unknown06=0, uint32 unknown18=0);
	virtual void SetSlotTint(uint8 material_slot, uint8 red_tint, uint8 green_tint, uint8 blue_tint);
	virtual void WearChange(uint8 material_slot, uint16 texture, uint32 color);
	virtual uint32 GetEquipment(uint8 material_slot) const { return(0); }
	virtual int32 GetEquipmentMaterial(uint8 material_slot) const;
	virtual uint32 GetEquipmentColor(uint8 material_slot) const;
	virtual uint32 IsEliteMaterialItem(uint8 material_slot) const;

	void Warp( float x, float y, float z );
	inline bool IsMoving() const { return moving; }
	virtual void SetMoving(bool move) { moving = move; delta_x=0; delta_y=0; delta_z=0; delta_heading=0; }
	virtual void GoToBind(uint8 bindnum = 0) {}
	virtual void Gate();
	virtual void RangedAttack(Mob* other) { }
	virtual void ThrowingAttack(Mob* other) { }
	uint16 GetThrownDamage(int16 wDmg, int32& TotalDmg, int& minDmg);
	bool AffectedExcludingSlot(int slot, int effect);

	// abstract methods
	virtual void Death(Mob* killerMob, int32 damage, uint16 spell_id, SkillType attack_skill) = 0;
	virtual void Damage(Mob* from, int32 damage, uint16 spell_id, SkillType attack_skill, bool avoidable = true, int8 buffslot = -1, bool iBuffTic = false) = 0;
	virtual bool Attack(Mob* other, int Hand = 13, bool FromRiposte = false, bool IsStrikethrough = false, bool IsFromSpell = false) = 0;	// 13 = Primary (default), 14 = secondary
	virtual bool HasRaid() = 0;
	virtual bool HasGroup() = 0;
	virtual Raid* GetRaid() = 0;
	virtual Group* GetGroup() = 0;

	// methods with implementation
	virtual void Heal();
	virtual void HealDamage(uint32 ammount, Mob* caster = NULL);
	virtual void SetMaxHP() { cur_hp = max_hp; }
	static uint32 GetLevelCon(uint8 mylevel, uint8 iOtherLevel);
	inline uint32 GetLevelCon(uint8 iOtherLevel) const { return(this?GetLevelCon(GetLevel(), iOtherLevel):CON_GREEN); }
	virtual void AddToHateList(Mob* other, int32 hate = 0, int32 damage = 0, bool iYellForHelp = true, bool bFrenzy = false, bool iBuffTic = false);

	inline virtual void SetHP(int32 hp) { if (hp >= max_hp) cur_hp = max_hp; else cur_hp = hp;}
	bool ChangeHP(Mob* other, int32 amount, uint16 spell_id = 0, int8 buffslot = -1, bool iBuffTic = false);
	inline void SetOOCRegen(int32 newoocregen) {oocregen = newoocregen;}
	int MonkSpecialAttack(Mob* other, uint8 skill_used);
	virtual void TryBackstab(Mob *other,int ReuseTime = 10);
	void DoAnim(const int animnum, int type=0, bool ackreq = true, eqFilterType filter = FilterNone);
	void ProjectileAnimation(Mob* to, uint16 item_id, bool IsArrow = false, float speed = 0, float angle = 0, float tilt = 0, float arc = 0);

	void ChangeSize(float in_size, bool bNoRestriction = false);
	virtual void GMMove(float x, float y, float z, float heading = 0.01, bool SendUpdate = true);
	void SetDeltas(float delta_x, float delta_y, float delta_z, float delta_h);
	void SetTargetDestSteps(uint8 target_steps) { tar_ndx = target_steps; }
	void SendPosUpdate(uint8 iSendToSelf = 0);
	void MakeSpawnUpdateNoDelta(PlayerPositionUpdateServer_Struct* spu);
	void MakeSpawnUpdate(PlayerPositionUpdateServer_Struct* spu);
	void SendPosition();

	void CreateDespawnPacket(EQApplicationPacket* app, bool Decay);
	void CreateHorseSpawnPacket(EQApplicationPacket* app, const char* ownername, uint16 ownerid, Mob* ForWho = 0);
	void CreateSpawnPacket(EQApplicationPacket* app, Mob* ForWho = 0);
	virtual void FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho);
	void CreateHPPacket(EQApplicationPacket* app);
	void SendHPUpdate();

	bool AddRangedProc(uint16 spell_id, uint16 iChance = 3, uint16 base_spell_id=SPELL_UNKNOWN);
	bool RemoveRangedProc(uint16 spell_id, bool bAll = false);
	bool HasRangedProcs() const;

	bool AddDefensiveProc(uint16 spell_id, uint16 iChance = 3, uint16 base_spell_id=SPELL_UNKNOWN);
	bool RemoveDefensiveProc(uint16 spell_id, bool bAll = false);
	bool HasDefensiveProcs() const;

	bool AddSkillProc(uint16 spell_id, uint16 iChance = 3, uint16 base_spell_id=SPELL_UNKNOWN);
	bool RemoveSkillProc(uint16 spell_id, bool bAll = false);
	bool HasSkillProcs() const;

	bool AddProcToWeapon(uint16 spell_id, bool bPerma = false, uint16 iChance = 3);
	bool RemoveProcFromWeapon(uint16 spell_id, bool bAll = false);
	bool HasProcs() const;

	inline uint8 SeeInvisible() const { return see_invis; }				
	inline bool SeeInvisibleUndead() const { return see_invis_undead; } 
	inline bool SeeHide() const { return see_hide; }
	inline bool SeeImprovedHide() const { return see_improved_hide; }

	bool CheckLos(Mob* other);
	bool CheckLosFN(Mob* other);
	bool CheckLosFN(float posX, float posY, float posZ, float mobSize);
	inline bool GetQglobal() const {return qglobal;}		// SCORPIOUS2K - return quest global flag

	bool IsInvisible(Mob* other = 0) const;
	void SetInvisible(uint8 state);

	void SetFlyMode(uint8 flymode);

	bool AttackAnimation(SkillType &skillinuse, int Hand, const ItemInst* weapon);
	uint16 GetSkillByItemType(int ItemType);
	virtual bool AvoidDamage(Mob* attacker, int32 &damage, bool CanRiposte=true);
	virtual bool CheckHitChance(Mob* attacker, SkillType skillinuse, int Hand, int16 chance_mod=0);
	virtual void TryCriticalHit(Mob *defender, uint16 skill, int32 &damage);
	void TryPetCriticalHit(Mob *defender, uint16 skill, int32 &damage);
	virtual bool TryFinishingBlow(Mob *defender, SkillType skillinuse);
	virtual bool TryHeadShot(Mob* defender, SkillType skillInUse);
	virtual void DoRiposte(Mob* defender);
	void ApplyMeleeDamageBonus(uint16 skill, int32 &damage);
	virtual void MeleeMitigation(Mob *attacker, int32 &damage, int32 minhit);

	void	DamageShield(Mob* other, bool spell_ds=false);
	int32  RuneAbsorb(int32 damage, uint16 type);
	bool	FindBuff(uint16 spellid);
	bool	FindType(uint16 type, bool bOffensive = false, uint16 threshold = 100);
	int16	GetBuffSlotFromType(uint16 type);
	uint16	GetSpellIDFromSlot(uint8 slot);
	int		CountDispellableBuffs();
	bool	HasBuffIcon(Mob* caster, Mob* target, uint16 spell_id);
	bool	CheckHitsRemaining(uint32 buff_slot, bool when_spell_done=false, bool negate=false,uint16 type=0, uint16 spell_id=0, bool use_skill=false,uint16 skill=0);
	void 	SpreadVirus(uint16 spell_id, uint16 casterID);
	
	virtual void MakePet(uint16 spell_id, const char* pettype, const char *petname = NULL);
	virtual void MakePoweredPet(uint16 spell_id, const char* pettype, int16 petpower, const char *petname = NULL);
//	inline void	MakePetType(uint16 spell_id, const char* pettype, const char *petname = NULL) { MakePet(spell_id, pettype, petname); }	//for perl
//	void	MakePet(uint16 spell_id, uint8 in_level, uint8 in_class, uint16 in_race, uint8 in_texture = 0, uint8 in_pettype = 0, float in_size = 0, uint8 type = 0, uint32 min_dmg = 0, uint32 max_dmg = 0, const char *petname = NULL);

	bool	CombatRange(Mob* other);
//	uint8	flag[60];		//this is for quests or something...

	virtual inline uint16	GetBaseRace()	const { return base_race; }
	virtual inline uint8	GetBaseGender()		const { return base_gender; }
	virtual inline uint8	GetDeity()			const { return deity; }
	inline uint16	GetRace()				const { return race; }
	inline uint8	GetGender()					const { return gender; }
	inline uint8	GetTexture()				const { return texture; }
	inline uint8	GetHelmTexture()			const { return helmtexture; }
	inline uint8	GetHairColor()				const { return haircolor; }
	inline uint8	GetBeardColor()				const { return beardcolor; }
	inline uint8	GetEyeColor1()				const { return eyecolor1; }
	inline uint8	GetEyeColor2()				const { return eyecolor2; }
	inline uint8	GetHairStyle()				const { return hairstyle; }
	inline uint8	GetLuclinFace()				const { return luclinface; }
	inline uint8	GetBeard()					const { return beard; }
	inline uint8	GetDrakkinHeritage()		const { return drakkin_heritage; }
	inline uint8	GetDrakkinTattoo()			const { return drakkin_tattoo; }
	inline uint8	GetDrakkinDetails()			const { return drakkin_details; }
	inline uint32  GetArmorTint(uint8 i)		const { return armor_tint[(i < MAX_MATERIALS) ? i : 0]; }
	inline uint8	GetClass()					const { return class_; }
	inline uint8	GetLevel()				const { return level; }
	inline const char*	GetName()			const { return name; }
	inline const char*	GetOrigName()			const { return orig_name; }
	inline const char*	GetLastName()			const { return lastname; }
	const char *GetCleanName();
	virtual void SetName(const char *new_name = NULL) { new_name ? strn0cpy(name, new_name, 64) : strn0cpy(name, GetName(), 64); return;};
	inline Mob*			GetTarget()			const { return target; }
	virtual void SetTarget(Mob* mob);
	virtual inline float		GetHPRatio() const { return max_hp == 0 ? 0 : ((float)cur_hp/max_hp*100); }

	bool IsLoggingEnabled() const { return(logging_enabled); }
	void EnableLogging() { logging_enabled = true; }
	void DisableLogging() { logging_enabled = false; }
	bool IsWarriorClass() const;
	virtual bool IsAttackAllowed(Mob *target, bool isSpellAttack = false);
	bool IsBeneficialAllowed(Mob *target);

	inline int32	GetHP()			const { return cur_hp; }
	inline int32	GetMaxHP()		const { return max_hp; }
	virtual int32	CalcMaxHP();
	float GetWalkspeed() const { return(_GetMovementSpeed(-47)); }
	float GetRunspeed() const { return(_GetMovementSpeed(0)); }
	float GetBaseRunspeed() const { return runspeed; }
	float GetMovespeed() const {
		if(IsRunning())
		{
			return GetRunspeed();
		}
		else
		{
			return GetWalkspeed();
		}
	}
	bool IsRunning() const { return m_is_running; }
	void SetRunning(bool val) { m_is_running = val; }

	virtual int GetCasterLevel(uint16 spell_id);
	void ApplySpellsBonuses(uint16 spell_id, uint8 casterlevel, StatBonuses* newbon, uint16 casterID = 0, bool item_bonus = false, uint32 ticsremaining = 0, int buffslot=-1);
	void NegateSpellsBonuses(uint16 spell_id);

	inline int32	GetMaxMana()	const { return max_mana; }
	inline int32	GetMana()		const { return cur_mana; }
	int32	GetItemHPBonuses();
	int32	GetSpellHPBonuses();
	virtual const int32& SetMana(int32 amount);
	inline float	GetManaRatio()	const { return max_mana == 0 ? 100 : (((float)cur_mana/max_mana)*100); }
	void			SetZone(uint32 zone_id, uint32 instance_id);

	// moved from client to use in NPC too
 	char GetCasterClass() const;
	uint8 GetArchetype() const;
 	virtual int32 CalcMaxMana();
 
	inline virtual int16	GetAC()		const { return AC + itembonuses.AC + spellbonuses.AC; } // Quagmire - this is NOT the right math
	inline virtual int16	GetATK()	const { return ATK + itembonuses.ATK + spellbonuses.ATK; }
	inline virtual int16	GetATKBonus()	const { return itembonuses.ATK + spellbonuses.ATK; }
	inline virtual int16	GetSTR()	const { return STR + itembonuses.STR + spellbonuses.STR; }
	inline virtual int16	GetSTA()	const { return STA + itembonuses.STA + spellbonuses.STA; }
	inline virtual int16	GetDEX()	const { return DEX + itembonuses.DEX + spellbonuses.DEX; }
	inline virtual int16	GetAGI()	const { return AGI + itembonuses.AGI + spellbonuses.AGI; }
	inline virtual int16	GetINT()	const { return INT + itembonuses.INT + spellbonuses.INT; }
	inline virtual int16	GetWIS()	const { return WIS + itembonuses.WIS + spellbonuses.WIS; }
	inline virtual int16	GetCHA()	const { return CHA + itembonuses.CHA + spellbonuses.CHA; }
	inline virtual int16	GetMR() const { return MR + itembonuses.MR + spellbonuses.MR; }
	inline virtual int16	GetFR()	const { return FR + itembonuses.FR + spellbonuses.FR; }
	inline virtual int16	GetDR()	const { return DR + itembonuses.DR + spellbonuses.DR; }
	inline virtual int16	GetPR()	const { return PR + itembonuses.PR + spellbonuses.PR; }
	inline virtual int16	GetCR() const { return CR + itembonuses.CR + spellbonuses.CR; }
	inline virtual int16	GetCorrup() const { return Corrup + itembonuses.Corrup + spellbonuses.Corrup; }
	inline StatBonuses GetItemBonuses() const { return itembonuses; }
	inline StatBonuses GetSpellBonuses() const { return spellbonuses; }
	inline StatBonuses GetAABonuses() const { return aabonuses; }

	inline virtual int16  GetMaxSTR() const { return GetSTR(); }
	inline virtual int16  GetMaxSTA() const { return GetSTA(); }
	inline virtual int16  GetMaxDEX() const { return GetDEX(); }
	inline virtual int16  GetMaxAGI() const { return GetAGI(); }
	inline virtual int16  GetMaxINT() const { return GetINT(); }
	inline virtual int16  GetMaxWIS() const { return GetWIS(); }
	inline virtual int16  GetMaxCHA() const { return GetCHA(); }
	inline virtual int16  GetMaxMR() const { return 255; }
	inline virtual int16  GetMaxPR() const { return 255; }
	inline virtual int16  GetMaxDR() const { return 255; }
	inline virtual int16  GetMaxCR() const { return 255; }
	inline virtual int16  GetMaxFR() const { return 255; }

	inline virtual int16	GetDelayDeath()		const { return 0; }

	bool IsNimbusEffectActive(uint32 nimbus_effect);
	void SetNimbusEffect(uint32 nimbus_effect);
	inline virtual uint32  GetNimbusEffect1() const { return nimbus_effect1; }
	inline virtual uint32  GetNimbusEffect2() const { return nimbus_effect2; }
	inline virtual uint32  GetNimbusEffect3() const { return nimbus_effect3; }
	void RemoveNimbusEffect(int effectid);

	virtual float GetActSpellRange(uint16 spell_id, float range, bool IsBard = false){ return range;}
	virtual int32  GetActSpellDamage(uint16 spell_id, int32 value) { return value; }
	virtual int32  GetActSpellHealing(uint16 spell_id, int32 value) { return value; }
	virtual int32 GetActSpellCost(uint16 spell_id, int32 cost){ return cost;}
	virtual int32 GetActSpellDuration(uint16 spell_id, int32 duration){ return duration;}
	virtual int32 GetActSpellCasttime(uint16 spell_id, int32 casttime);
	float ResistSpell(uint8 resist_type, uint16 spell_id, Mob *caster, bool use_resist_override = false, int resist_override = 0, bool CharismaCheck = false);
	uint16 GetSpecializeSkillValue(uint16 spell_id) const;

	void ShowStats(Client* client);
	void ShowBuffs(Client* client);
	void ShowBuffList(Client* client);
	uint32 GetNPCTypeID()			const { return npctype_id; } // rembrant, Dec. 20, 2001

	float Dist(const Mob &) const;
	float DistNoZ(const Mob &) const;
	float DistNoRoot(const Mob &) const;
	float DistNoRoot(float x, float y, float z) const;
	float DistNoRootNoZ(float x, float y) const;
	float DistNoRootNoZ(const Mob &) const;

	static float GetReciprocalHeading(Mob* target);
	bool PlotPositionAroundTarget(Mob* target, float &x_dest, float &y_dest, float &z_dest, bool lookForAftArc = true);

	bool IsTargeted() const { return (targeted > 0); }
	inline void IsTargeted(int in_tar) { targeted += in_tar; if(targeted < 0) targeted = 0;}

	inline void		Teleport(VERTEX NewPosition) { x_pos = NewPosition.x; y_pos = NewPosition.y; z_pos = NewPosition.z; };
	inline const float	GetX() const		{ return x_pos; }
	inline const float	GetY() const		{ return y_pos; }
	inline const float	GetZ() const		{ return z_pos; }
	inline const float	GetHeading() const	{ return heading; }
	inline const float	GetSize() const		{ return size; }
	inline const float	GetBaseSize() const		{ return base_size; }
	inline void			SetChanged()		{ pLastChange = Timer::GetCurrentTime(); }
	inline const uint32	LastChange() const	{ return pLastChange; }
    bool IsBoat();

	void	SetFollowID(uint32 id) { follow = id; }
	void	SetFollowDistance(uint32 dist) { follow_dist = dist; }
	uint32	GetFollowID()		  const { return follow; }
	uint32	GetFollowDistance()		  const { return follow_dist; }

	virtual void	Message(uint32 type, const char* message, ...) {} // fake so dont have to worry about typecasting
	virtual void	Message_StringID(uint32 type, uint32 string_id, uint32 distance = 0) {}
	virtual void	Message_StringID(uint32 type, uint32 string_id, const char* message,const char* message2=0,const char* message3=0,const char* message4=0,const char* message5=0,const char* message6=0,const char* message7=0,const char* message8=0, const char* message9=0, uint32 distance = 0) {}
	void Say(const char *format, ...);
	void Say_StringID(uint32 string_id, const char *message3 = 0, const char *message4 = 0, const char *message5 = 0, const char *message6 = 0, const char *message7 = 0, const char *message8 = 0, const char *message9 = 0);
	void Shout(const char *format, ...);
	void Emote(const char *format, ...);
	void QuestJournalledSay(Client *QuestInitiator, const char *str);
	uint32 GetItemStat(uint32 itemid, const char *identifier);

	//Casting related
 	void SendSpellBarDisable();
 	void SendSpellBarEnable(uint16 spellid);
	void ZeroCastingVars();
	virtual void SpellProcess();


	
	//TODO: put these ridiculous options in a damned struct or something

	virtual bool CastSpell(uint16 spell_id, uint16 target_id, uint16 slot = 10, int32 casttime = -1, int32 mana_cost = -1, uint32* oSpellWillFinish = 0, uint32 item_slot = 0xFFFFFFFF, uint32 timer = 0xFFFFFFFF, uint32 timer_duration = 0, uint32 type = 0, int16 *resist_adjust = NULL);
	virtual bool DoCastSpell(uint16 spell_id, uint16 target_id, uint16 slot = 10, int32 casttime = -1, int32 mana_cost = -1, uint32* oSpellWillFinish = 0, uint32 item_slot = 0xFFFFFFFF, uint32 timer = 0xFFFFFFFF, uint32 timer_duration = 0, uint32 type = 0, int16 resist_adjust = 0);
	void CastedSpellFinished(uint16 spell_id, uint32 target_id, uint16 slot, uint16 mana_used, uint32 inventory_slot = 0xFFFFFFFF, int16 resist_adjust = 0);
	bool SpellFinished(uint16 spell_id, Mob *target, uint16 slot = 10, uint16 mana_used = 0, uint32 inventory_slot = 0xFFFFFFFF, int16 resist_adjust = 0);
	virtual bool SpellOnTarget(uint16 spell_id, Mob* spelltar, bool reflect = false, bool use_resist_adjust = false, int16 resist_adjust = 0);
	virtual bool SpellEffect(Mob* caster, uint16 spell_id, float partial = 100);
	virtual bool DetermineSpellTargets(uint16 spell_id, Mob *&spell_target, Mob *&ae_center, CastAction_type &CastAction);
	virtual bool CheckFizzle(uint16 spell_id);
	virtual bool IsImmuneToSpell(uint16 spell_id, Mob *caster);
	virtual float GetAOERange(uint16 spell_id);
	void InterruptSpell(uint16 spellid = SPELL_UNKNOWN);
	void InterruptSpell(uint16, uint16, uint16 spellid = SPELL_UNKNOWN);
	inline bool IsCasting() const { return((casting_spell_id != 0)); }
	uint16 CastingSpellID() const { return casting_spell_id; }
	
	//Song related
	bool UseBardSpellLogic(uint16 spell_id = 0xffff, int slot = -1);
	bool ApplyNextBardPulse(uint16 spell_id, Mob *spell_target, uint16 slot);
	void BardPulse(uint16 spell_id, Mob *caster);

	//Buff related
	void BuffProcess();
	virtual void DoBuffTic(uint16 spell_id, uint32 ticsremaining, uint8 caster_level, Mob* caster = 0);
	void BuffFadeBySpellID(uint16 spell_id);
	void BuffFadeByEffect(int effectid, int skipslot = -1);
	void BuffFadeAll();
	void BuffFadeDetrimental();
	void BuffFadeBySlot(int slot, bool iRecalcBonuses = true);
	void BuffFadeDetrimentalByCaster(Mob *caster);
	void BuffFadeBySitModifier();
	void BuffModifyDurationBySpellID(uint16 spell_id, int32 newDuration);
	int	AddBuff(Mob *caster, const uint16 spell_id, int duration = 0, int32 level_override = -1);
	int	CanBuffStack(uint16 spellid, uint8 caster_level, bool iFailIfOverwrite = false);
	int	CalcBuffDuration(Mob *caster, Mob *target, uint16 spell_id, int32 caster_level_override = -1);
	void SendPetBuffsToClient();
	virtual int GetCurrentBuffSlots() const { return 0; }
	virtual int GetCurrentSongSlots() const { return 0; }
	virtual int GetCurrentDiscSlots() const { return 0; }
	virtual int GetMaxBuffSlots() const { return 0; }
	virtual int GetMaxSongSlots() const { return 0; }
	virtual int GetMaxDiscSlots() const { return 0; }
	virtual int GetMaxTotalSlots() const { return 0; }
	virtual void InitializeBuffSlots() { buffs = NULL; current_buff_count = 0; }
	virtual void UninitializeBuffSlots() { }
	inline bool HasRune() const { return m_hasRune; }
	inline bool HasSpellRune() const { return m_hasSpellRune; }
	inline bool HasPartialMeleeRune() const { return m_hasPartialMeleeRune; }
	inline bool HasPartialSpellRune() const { return m_hasPartialSpellRune; }
	inline void SetHasRune(bool hasRune) { m_hasRune = hasRune; }
	inline void SetHasSpellRune(bool hasSpellRune) { m_hasSpellRune = hasSpellRune; }
	inline void SetHasPartialMeleeRune(bool hasPartialMeleeRune) { m_hasPartialMeleeRune = hasPartialMeleeRune; }
	inline void SetHasPartialSpellRune(bool hasPartialSpellRune) { m_hasPartialSpellRune = hasPartialSpellRune; }
	//inline bool HasDeathSaveChance() const { return m_hasDeathSaveChance; }
	//inline void SetDeathSaveChance(bool hasDeathSaveChance) { m_hasDeathSaveChance = hasDeathSaveChance; }
	EQApplicationPacket *MakeBuffsPacket(bool for_target = true);
	void SendBuffsToClient(Client *c);
	inline Buffs_Struct* GetBuffs() { return buffs; }
	void DoGravityEffect();
	Timer GravityTimer;

	//effect related
	int16 CalcFocusEffect(focusType type, uint16 focus_id, uint16 spell_id, bool best_focus=false);
	uint8 IsFocusEffect(uint16 spellid, int effect_index, bool AA=false,uint32 aa_effect=0); 
	void SendIllusionPacket(uint16 in_race, uint8 in_gender = 0xFF, uint8 in_texture = 0xFF, uint8 in_helmtexture = 0xFF, uint8 in_haircolor = 0xFF, uint8 in_beardcolor = 0xFF, uint8 in_eyecolor1 = 0xFF, uint8 in_eyecolor2 = 0xFF, uint8 in_hairstyle = 0xFF, uint8 in_luclinface = 0xFF, uint8 in_beard = 0xFF, uint8 in_aa_title = 0xFF, uint32 in_drakkin_heritage = 0xFFFFFFFF, uint32 in_drakkin_tattoo = 0xFFFFFFFF, uint32 in_drakkin_details = 0xFFFFFFFF, float in_size = 0xFFFFFFFF);
	virtual void Stun(int duration);
	virtual void UnStun();
	inline void Silence(bool newval) { silenced = newval; }
	inline void Amnesia(bool newval) { amnesiad = newval; }
	void TemporaryPets(uint16 spell_id, Mob *target, const char *name_override = NULL, uint32 duration_override = 0);
	void TypesTemporaryPets(uint32 typesid, Mob *target, const char *name_override = NULL, uint32 duration_override = 0, bool followme = false);
	void WakeTheDead(uint16 spell_id, Mob *target, uint32 duration);
	void Spin();
	void Kill();
	bool PassCharismaCheck(Mob* caster, Mob* spellTarget, uint16 spell_id);
	bool TryDeathSave();
	bool TryDivineSave();	
	void DoBuffWearOffEffect(uint32 index);
	void TryTriggerOnCast(uint32 spell_id, bool aa_trigger);
	void TriggerOnCast(uint32 focus_spell, uint32 spell_id, bool aa_trigger);
	void TrySpellTrigger(Mob *target, uint32 spell_id);
	void TryApplyEffect(Mob *target, uint32 spell_id);
	void TryTwincast(Mob *caster, Mob *target, uint32 spell_id);
	void TrySympatheticProc(Mob *target, uint32 spell_id);
	bool TryFadeEffect(int slot);
	uint16 GetSpellEffectResistChance(uint16 spell_id);
	int16 GetHealRate(uint16 spell_id);
	int16 GetCriticalHealRate(uint16 spell_id);
	int32 GetVulnerability(int32 damage, Mob *caster, uint32 spell_id, uint32 ticsremaining);
	int32 GetAdditionalDamage(Mob *caster, uint32 spell_id, bool use_skill = false, uint16 skill=0);
	int16 GetSkillDmgTaken(const SkillType skill_used);
	void DoKnockback(Mob *caster, uint32 pushback, uint32 pushup);
	int16 CalcResistChanceBonus();
	int16 CalcFearResistChance();
	void TrySpellOnKill(uint8 level, uint16 spell_id);
	bool TrySpellOnDeath();
	void CastOnCurer(uint32 spell_id);
	void CastOnCure(uint32 spell_id);
	void CastOnNumHitFade(uint32 spell_id);
	int SlowMitigation(bool slow_msg=false, Mob *caster = NULL,int slow_value = 0); 
	int16 GetCritDmgMob(uint16 skill);
	int16 GetMeleeDamageMod_SE(uint16 skill);
	int16 GetMeleeMinDamageMod_SE(uint16 skill);
	int16 GetCrippBlowChance();
	int16 GetSkillReuseTime(uint16 skill);
	int16 GetCriticalChanceBonus(uint16 skill);
	int16 GetSkillDmgAmt(uint16 skill);
	bool TryReflectSpell(uint32 spell_id);
	bool CanBlockSpell() const { return(spellbonuses.BlockNextSpell); }
	bool DoHPToManaCovert(uint16 mana_cost = 0);
	int32 ApplySpellEffectiveness(Mob* caster, int16 spell_id, int32 value, bool IsBard = false);

	void  ModSkillDmgTaken(SkillType skill_num, int value); 
	int16 GetModSkillDmgTaken(const SkillType skill_num);
	void ModVulnerability(uint8 resist, int16 value); 
	int16 GetModVulnerability(const uint8 resist);	

	void SetAllowBeneficial(bool value) { m_AllowBeneficial = value; }
	bool GetAllowBeneficial() { return m_AllowBeneficial; }
	void SetDisableMelee(bool value) { m_DisableMelee = value; }
	bool IsMeleeDisabled() { return m_DisableMelee; }
	
	bool IsOffHandAtk() const { return offhand; }
	inline void OffHandAtk(bool val) { offhand = val; }

	inline void SetFlurryChance(uint8 value) { NPC_FlurryChance = value;}
	uint8 GetFlurryChance() { return NPC_FlurryChance; }
	
	static uint32 GetAppearanceValue(EmuAppearance iAppearance);
	void SendAppearancePacket(uint32 type, uint32 value, bool WholeZone = true, bool iIgnoreSelf = false, Client *specific_target=NULL);
	void SetAppearance(EmuAppearance app, bool iIgnoreSelf = true);
	inline EmuAppearance GetAppearance() const { return _appearance; }
	inline const uint8 GetRunAnimSpeed()	const { return pRunAnimSpeed; }
	inline void	SetRunAnimSpeed(int8 in) { if (pRunAnimSpeed != in) { pRunAnimSpeed = in; pLastChange = Timer::GetCurrentTime(); } }
	bool IsDestructibleObject() { return destructibleobject; }
	void SetDestructibleObject(bool in) { destructibleobject = in; }

	Mob* GetPet();
	void SetPet(Mob* newpet);
	virtual Mob* GetOwner();
	virtual Mob* GetOwnerOrSelf();
	Mob* GetUltimateOwner();
	void SetPetID(uint16 NewPetID);
	inline uint16 GetPetID()	const { return petid; }
	inline PetType GetPetType() const { return typeofpet; }
	void SetPetType(PetType p) { typeofpet = p; } 
	inline int16 GetPetPower() const { return (petpower < 0) ? 0 : petpower; }
	void SetPetPower(int16 p) { if (p < 0) petpower = 0; else petpower = p; }
	bool IsFamiliar() const { return(typeofpet == petFamiliar); }
	bool IsAnimation() const { return(typeofpet == petAnimation); }
	bool IsCharmed() const { return(typeofpet == petCharmed); }
	void SetOwnerID(uint16 NewOwnerID);
	inline uint16 GetOwnerID() const { return ownerid; }
	inline virtual bool HasOwner() { if(GetOwnerID()==0){return false;} return( entity_list.GetMob(GetOwnerID()) != 0); }
	inline virtual bool IsPet() { return(HasOwner()); }
	inline bool HasPet() const { if(GetPetID()==0){return false;} return (entity_list.GetMob(GetPetID()) != 0);}
	bool HadTempPets() const { return(hasTempPet); }
	void TempPets(bool i) { hasTempPet = i; }
	bool HasPetAffinity() { if (aabonuses.GivePetGroupTarget ||	itembonuses.GivePetGroupTarget || spellbonuses.GivePetGroupTarget) return true; return false; }

	inline const bodyType GetBodyType() const { return bodytype; }
	inline const bodyType GetOrigBodyType() const { return orig_bodytype; }
	void SetBodyType(bodyType new_body, bool overwrite_orig);

	uint8	invisible, see_invis;
	bool	invulnerable, invisible_undead, invisible_animals, sneaking, hidden, improved_hidden;
	bool	see_invis_undead, see_hide, see_improved_hide;   
	bool	qglobal;		

	virtual void SetAttackTimer();
	inline void	SetInvul(bool invul) { invulnerable=invul; }
	inline bool	GetInvul(void) { return invulnerable; }
	inline void	SetExtraHaste(int Haste) { ExtraHaste = Haste; }
	virtual int GetHaste();

	uint8	GetWeaponDamageBonus(const Item_Struct* Weapon);
	uint16	GetDamageTable(SkillType skillinuse);
	virtual int GetMonkHandToHandDamage(void);

	bool	CanThisClassDoubleAttack(void) const;
	bool	CanThisClassDualWield(void) const;
	bool	CanThisClassRiposte(void) const;
	bool	CanThisClassDodge(void) const;
	bool	CanThisClassParry(void) const;
	bool	CanThisClassBlock(void) const;

	int	GetMonkHandToHandDelay(void);
	uint16	GetClassLevelFactor();
	void	Mesmerize();
	inline bool	IsMezzed() const { return mezzed; }
	inline bool	IsStunned() const { return stunned; }
	inline bool	IsSilenced() const { return silenced; }
	inline bool	IsAmnesiad() const { return amnesiad; }
	inline uint16	GetErrorNumber() const {return adverrorinfo;}

	int32	ReduceDamage(int32 damage);
	int32  AffectMagicalDamage(int32 damage, uint16 spell_id, const bool iBuffTic, Mob* attacker);

	virtual void DoSpecialAttackDamage(Mob *who, SkillType skill, int32 max_damage, int32 min_damage = 1, int32 hate_override = -1, int ReuseTime = 10, bool HitChance=false);
	virtual void DoThrowingAttackDmg(Mob* other, const ItemInst* RangeWeapon=NULL, const Item_Struct* item=NULL, uint16 weapon_damage=0, int16 chance_mod=0,int16 focus=0);
	virtual void DoMeleeSkillAttackDmg(Mob* other, uint16 weapon_damage, SkillType skillinuse, int16 chance_mod=0, int16 focus=0, bool CanRiposte=false);
	virtual void DoArcheryAttackDmg(Mob* other, const ItemInst* RangeWeapon=NULL, const ItemInst* Ammo=NULL, uint16 weapon_damage=0, int16 chance_mod=0, int16 focus=0);
	bool CanDoSpecialAttack(Mob *other);
	bool Flurry();
    bool Rampage();
    bool AddRampage(Mob*);
	void ClearRampage();
	void AreaRampage();

    void StartEnrage();
	void ProcessEnrage();
    bool IsEnraged();
	void Taunt(NPC* who, bool always_succeed,  float chance_bonus = 0);

	virtual void		AI_Init();
	virtual void		AI_Start(uint32 iMoveDelay = 0);
	virtual void		AI_Stop();
	virtual void		AI_Process();

	const char* GetEntityVariable(const char *id);
	void SetEntityVariable(const char *id, const char *m_var);
	bool EntityVariableExists(const char *id);

	void				AI_Event_Engaged(Mob* attacker, bool iYellForHelp = true);
	void				AI_Event_NoLongerEngaged();

	FACTION_VALUE		GetSpecialFactionCon(Mob* iOther);
	inline const bool	IsAIControlled() const { return pAIControlled; }
	inline const float GetAggroRange() const { return (spellbonuses.AggroRange == -1) ? pAggroRange : spellbonuses.AggroRange; }
	inline const float GetAssistRange() const { return (spellbonuses.AssistRange == -1) ? pAssistRange : spellbonuses.AssistRange; }


	inline void			SetPetOrder(eStandingPetOrder i) { pStandingPetOrder = i; }
	inline const eStandingPetOrder GetPetOrder() const { return pStandingPetOrder; }
	inline void			SetHeld(bool nState) { held = nState; }
	inline const bool	IsHeld() const { return held; }
	inline void			SetNoCast(bool nState) { nocast = nState; }
	inline const bool	IsNoCast() const { return nocast; }
	inline void			SetFocused(bool nState) { focused = nState; }
	inline const bool	IsFocused() const { return focused; }
	inline const bool	IsRoamer() const { return roamer; }
	inline const bool   IsRooted() const { return rooted || permarooted; }
	inline const bool   HasVirus() const { return has_virus; }
	int					GetSnaredAmount();

	bool				RemoveFromHateList(Mob* mob);
	void				SetHate(Mob* other, int32 hate = 0, int32 damage = 0) {hate_list.Set(other,hate,damage);}
	uint32				GetHateAmount(Mob* tmob, bool is_dam = false)  {return hate_list.GetEntHate(tmob,is_dam);}
	uint32				GetDamageAmount(Mob* tmob)  {return hate_list.GetEntHate(tmob, true);}
	Mob*				GetHateTop()  {return hate_list.GetTop(this);}
	Mob*				GetHateDamageTop(Mob* other)  {return hate_list.GetDamageTop(other);}
	Mob*				GetHateRandom()  {return hate_list.GetRandom();}
	//this is diff from GetHateTop as it does not account for things like frenzy, just the person with the most hate
	Mob*				GetHateMost()	{return hate_list.GetMostHate();}
	bool				IsEngaged()   {return(!hate_list.IsEmpty()); }
	bool				HateSummon();
	void				FaceTarget(Mob* MobToFace = 0);
	void				SetHeading(float iHeading) { if (heading != iHeading) { pLastChange = Timer::GetCurrentTime(); heading = iHeading; } }
	void				WipeHateList();
	void				AddFeignMemory(Client* attacker);
	void				RemoveFromFeignMemory(Client* attacker);
	void				ClearFeignMemory();
	void				PrintHateListToClient(Client *who) { hate_list.PrintToClient(who); }
	void GetHateList(std::list<tHateEntry*> &h_list) { return hate_list.GetHateList(h_list); }


	int					GetCurWp(){ return cur_wp; }

	//old fear function
	//void SetFeared(Mob *caster, uint32 duration, bool flee = false);
	float GetFearSpeed();
	bool IsFeared() { return curfp; } // This returns true if the mob is feared or fleeing due to low HP
	//old fear: inline void StartFleeing() { SetFeared(GetHateTop(), FLEE_RUN_DURATION, true); }
	inline void StartFleeing() { flee_mode = true; CalculateNewFearpoint(); }
	void ProcessFlee();
	void CheckFlee();

	inline bool			CheckAggro(Mob* other) {return hate_list.IsOnHateList(other);}
    float				CalculateHeadingToTarget(float in_x, float in_y);
    bool				CalculateNewPosition(float x, float y, float z, float speed, bool checkZ = false);
	virtual bool		CalculateNewPosition2(float x, float y, float z, float speed, bool checkZ = true);
    float				CalculateDistance(float x, float y, float z);
	float				GetGroundZ(float new_x, float new_y, float z_offset=0.0);
	void				SendTo(float new_x, float new_y, float new_z);
	void				SendToFixZ(float new_x, float new_y, float new_z);
	void				NPCSpecialAttacks(const char* parse, int permtag, bool reset = true, bool remove = false);
	inline uint32		DontHealMeBefore() const { return pDontHealMeBefore; }
	inline uint32		DontBuffMeBefore() const { return pDontBuffMeBefore; }
	inline uint32		DontDotMeBefore() const { return pDontDotMeBefore; }
	inline uint32		DontRootMeBefore() const { return pDontRootMeBefore; }
	inline uint32		DontSnareMeBefore() const { return pDontSnareMeBefore; }
	inline uint32		DontCureMeBefore() const { return pDontCureMeBefore; }
	void				SetDontRootMeBefore(uint32 time) { pDontRootMeBefore = time; }
	void				SetDontHealMeBefore(uint32 time) { pDontHealMeBefore = time; }
	void				SetDontBuffMeBefore(uint32 time) { pDontBuffMeBefore = time; }
	void				SetDontDotMeBefore(uint32 time) { pDontDotMeBefore = time; }
	void				SetDontSnareMeBefore(uint32 time) { pDontSnareMeBefore = time; }
	void				SetDontCureMeBefore(uint32 time) { pDontCureMeBefore = time; }

	// calculate interruption of spell via movement of mob
	void SaveSpellLoc() {spell_x = x_pos; spell_y = y_pos; spell_z = z_pos; }
	inline float GetSpellX() const {return spell_x;}
	inline float GetSpellY() const {return spell_y;}
	inline float GetSpellZ() const {return spell_z;}
	inline bool	IsGrouped()	 const { return isgrouped; }
	void SetGrouped(bool v);
	inline bool IsRaidGrouped() const { return israidgrouped; }
	void SetRaidGrouped(bool v);
	inline bool	IsLooting()	 const { return islooting; }
	void SetLooting(bool val) { islooting = val; }

	bool CheckWillAggro(Mob *mob);

	void	InstillDoubt(Mob *who);
	int16	GetResist(uint8 type) const;
	Mob*	GetShieldTarget()			const { return shield_target; }
	void	SetShieldTarget(Mob* mob)	{ shield_target = mob; }
	bool	HasActiveSong() const { return(bardsong != 0); }
	bool	Charmed() const { return charmed; }
	static uint32	GetLevelHP(uint8 tlevel);
	uint32	GetZoneID() const;	//for perl
	virtual int32 CheckAggroAmount(uint16 spellid);
	virtual int32 CheckHealAggroAmount(uint16 spellid, uint32 heal_possible = 0);
	virtual uint32 GetAA(uint32 aa_id) const { return(0); }

	uint16	GetInstrumentMod(uint16 spell_id) const;
	int CalcSpellEffectValue(uint16 spell_id, int effect_id, int caster_level = 1, Mob *caster = NULL, int ticsremaining = 0);
	int CalcSpellEffectValue_formula(int formula, int base, int max, int caster_level, uint16 spell_id, int ticsremaining = 0);
	virtual int CheckStackConflict(uint16 spellid1, int caster_level1, uint16 spellid2, int caster_level2, Mob* caster1 = NULL, Mob* caster2 = NULL);
	uint32 GetCastedSpellInvSlot() const { return casting_spell_inventory_slot; }

	// HP Event
	inline int GetNextHPEvent() const { return nexthpevent; }
	void SetNextHPEvent( int hpevent );
	void SendItemAnimation(Mob *to, const Item_Struct *item, SkillType skillInUse);
	inline int& GetNextIncHPEvent() { return nextinchpevent; }
	void SetNextIncHPEvent( int inchpevent );

	bool DivineAura() const;
    bool SpecAttacks[SPECATK_MAXNUM];
	bool HasNPCSpecialAtk(const char* parse);
	Shielders_Struct shielder[MAX_SHIELDERS];
	Trade* trade;

	//temporary:
	bool fix_pathing;
	float rewind_x; //Lieka:  Used for storing /rewind values
	float rewind_y; //Lieka:  Used for storing /rewind values
	float rewind_z; //Lieka:  Used for storing /rewind values
	Timer rewind_timer; //Lieka:  Used for measuring how long a player stays in one spot
	inline float GetCWPX() const { return(cur_wp_x); }
	inline float GetCWPY() const { return(cur_wp_y); }
	inline float GetCWPZ() const { return(cur_wp_z); }
	inline float GetCWPH() const { return(cur_wp_heading); }
	inline float GetCWPP() const { return(cur_wp_pause); }
	inline int GetCWP() const { return(cur_wp); }
	void SetCurrentWP(uint16 waypoint) { cur_wp = waypoint; }
	virtual FACTION_VALUE GetReverseFactionCon(Mob* iOther) { return FACTION_INDIFFERENT; }
	
	inline bool IsTrackable() const { return(trackable); }
	Timer* GetAIThinkTimer() { return AIthink_timer; }
	Timer* GetAIMovementTimer() { return AImovement_timer; }
	Timer GetAttackTimer() { return attack_timer; }
	Timer GetAttackDWTimer() { return attack_dw_timer; }
	inline bool IsFindable() { return findable; }
	inline uint8 GetManaPercent() { return (uint8)((float)cur_mana / (float)max_mana * 100.0f); }
	virtual uint8 GetEndurancePercent() { return 0; }

	inline virtual bool IsBlockedBuff(int16 SpellID) { return false; }
	inline virtual bool IsBlockedPetBuff(int16 SpellID) { return false; }

	void SetGlobal(const char *varname, const char *newvalue, int options, const char *duration, Mob *other = NULL);
	void TarGlobal(const char *varname, const char *value, const char *duration, int npcid, int charid, int zoneid);
	void DelGlobal(const char *varname);

protected:
	void CommonDamage(Mob* other, int32 &damage, const uint16 spell_id, const SkillType attack_skill, bool &avoidable, const int8 buffslot, const bool iBuffTic);
	static uint16 GetProcID(uint16 spell_id, uint8 effect_index);
	float _GetMovementSpeed(int mod) const;
	virtual bool MakeNewPositionAndSendUpdate(float x, float y, float z, float speed, bool checkZ);

	virtual bool AI_EngagedCastCheck() { return(false); }
	virtual bool AI_PursueCastCheck() { return(false); }
	virtual bool AI_IdleCastCheck() { return(false); }
	
	//used by mlog() for VC6
	#ifdef NO_VARIADIC_MACROS
	void mob_log(LogType type, const char *fmt, ...);
	void mob_hex(LogType type, const char *data, unsigned long length, unsigned char padding=4);
	#endif

	bool	IsFullHP;
	bool	moved;

	std::vector<std::string> RampageArray;
	std::map<std::string, std::string> m_EntityVariables;

	int16 SkillDmgTaken_Mod[HIGHEST_SKILL+2];
	int16 Vulnerability_Mod[HIGHEST_RESIST+2]; 
	bool m_AllowBeneficial;
	bool m_DisableMelee;
	uint8 NPC_FlurryChance;

	bool	isgrouped; //These meant to be private?
	bool	israidgrouped;
	bool	pendinggroup;
	bool	islooting;
	uint8	texture;
	uint8	helmtexture;

	int	AC;
	int16 ATK;
	int16 STR;
	int16 STA;
	int16 DEX;
	int16 AGI;
	int16 INT;
	int16 WIS;
	int16 CHA;
	int16 MR;
	int16 CR;
	int16 FR;
	int16 DR;
	int16 PR;
	int16 Corrup;
	bool moving;
	int targeted;
	bool findable;
	bool trackable;
	int32  cur_hp;
	int32  max_hp;
	int32	base_hp;
	int32	cur_mana;
	int32	max_mana;
	int32	hp_regen;
	int32	mana_regen;
	int32	oocregen; // Out of Combat Regen, % per tick
	uint8 maxlevel;
	uint32 scalerate;
	Buffs_Struct *buffs;
	uint32 current_buff_count;
	Timer *buff_tic_timer;
	StatBonuses	itembonuses;
	StatBonuses	spellbonuses;
	StatBonuses	aabonuses;
	uint16 petid;
	uint16 ownerid;
	PetType typeofpet;
	int16 petpower;		// Should be part of class Pet, like PetType
	uint32 follow;
	uint32 follow_dist;

	uint8    gender;
	uint16	race;
	uint8	base_gender;
	uint16	base_race;
	uint8    class_;
	bodyType    bodytype;
	bodyType    orig_bodytype;
	uint16	deity;
	uint8    level;
	uint32   npctype_id;
	float	x_pos;
	float	y_pos;
	float	z_pos;
	float	heading;
	uint16	animation;
	float	base_size;
	float	size;
	float	runspeed;
	uint32 pLastChange;
	bool held;
	bool nocast;
	bool focused;
	void CalcSpellBonuses(StatBonuses* newbon);
	virtual void CalcBonuses();
	void TrySkillProc(Mob *on, uint16 skill, float chance);
	bool PassLimitToSkill(uint16 spell_id, uint16 skill);
	bool PassLimitClass(uint32 Classes_, uint16 Class_);
	void TryDefensiveProc(const ItemInst* weapon, Mob *on, uint16 hand = 13, int damage=0);
	void TryWeaponProc(const Item_Struct* weapon, Mob *on, uint16 hand = 13);
	void TryWeaponProc(const ItemInst* weapon, Mob *on, uint16 hand = 13);
	void ExecWeaponProc(uint16 spell_id, Mob *on);
	virtual float GetProcChances(float &ProcBonus, float &ProcChance, uint16 weapon_speed = 30, uint16 hand = 13);
	virtual float GetDefensiveProcChances(float &ProcBonus, float &ProcChance, uint16 weapon_speed = 30, uint16 hand = 13);
	int GetWeaponDamage(Mob *against, const Item_Struct *weapon_item);
	int GetWeaponDamage(Mob *against, const ItemInst *weapon_item, uint32 *hate = NULL);
	int GetKickDamage();
	int GetBashDamage();
	virtual void ApplySpecialAttackMod(SkillType skill, int32 &dmg, int32 &mindmg);
	bool HasDied();
	void CalculateNewFearpoint();
	float FindGroundZ(float new_x, float new_y, float z_offset=0.0);
	VERTEX UpdatePath(float ToX, float ToY, float ToZ, float Speed, bool &WaypointChange, bool &NodeReached);
	void PrintRoute();
	void UpdateRuneFlags();

	virtual float GetSympatheticProcChances(float &ProcBonus, float &ProcChance, int32 cast_time, int16 ProcRateMod);

	enum {MAX_PROCS = 4};
	tProc PermaProcs[MAX_PROCS];
	tProc SpellProcs[MAX_PROCS];
	tProc DefensiveProcs[MAX_PROCS];
	tProc RangedProcs[MAX_PROCS];
	tProc SkillProcs[MAX_PROCS];

	char    name[64];
	char    orig_name[64];
	char	clean_name[64];
	char    lastname[64];

    bool bEnraged;
    Timer *SpecAttackTimers[SPECATK_MAXNUM];
	bool destructibleobject;

	int32	delta_heading;
    float	delta_x;
	float	delta_y;
	float	delta_z;

	uint8    light;

	float	fixedZ;
	EmuAppearance    _appearance;
	uint8	pRunAnimSpeed;
	bool	m_is_running;

	
	Timer	attack_timer;
	Timer	attack_dw_timer;
	Timer	ranged_timer;
	float	attack_speed;		//% increase/decrease in attack speed (not haste)
	float	slow_mitigation;   // Allows for a slow mitigation based on a % in decimal form.  IE, 1 = 100% mitigation, .5 is 50%
	Timer	tic_timer;
	Timer	mana_timer;

	//spell casting vars
	Timer spellend_timer;
	uint16 casting_spell_id;
    float spell_x, spell_y, spell_z;
	int	attacked_count;
	bool delaytimer;
	uint16 casting_spell_targetid;
	uint16 casting_spell_slot;
	uint16 casting_spell_mana;
	uint32 casting_spell_inventory_slot;
	uint32 casting_spell_timer;
	uint32 casting_spell_timer_duration;
	uint32 casting_spell_type;
	int16 casting_spell_resist_adjust;
	uint16 bardsong;
	uint8 bardsong_slot;
	uint32 bardsong_target_id;

	// Currently 3 max nimbus particle effects at a time
	uint32 nimbus_effect1;
	uint32 nimbus_effect2;
	uint32 nimbus_effect3;

	uint8 haircolor;
	uint8 beardcolor;
	uint8 eyecolor1; // the eyecolors always seem to be the same, maybe left and right eye?
	uint8 eyecolor2;
	uint8 hairstyle;
	uint8 luclinface; // 
	uint8 beard;
	uint32 drakkin_heritage;
	uint32 drakkin_tattoo;
	uint32 drakkin_details;
	uint32 armor_tint[MAX_MATERIALS];

	uint8 aa_title;

	Mob* shield_target;

	int		ExtraHaste;	// for the #haste command
	bool	mezzed;
	bool	stunned;
	bool	charmed;	//this isnt fully implemented yet
	bool	rooted;
	bool	silenced;
	bool	amnesiad;
	bool	inWater;	// Set to true or false by Water Detection code if enabled by rules
	bool	has_virus;	// whether this mob has a viral spell on them
	uint16	viral_spells[MAX_SPELL_TRIGGER*2]; // Stores the spell ids of the viruses on target and caster ids
	int16	rooted_mod; //Modifier to root break chance, defined when root is cast on a target.
	bool	offhand;

	Timer	stunned_timer;
	Timer	spun_timer;
	Timer	bardsong_timer;
	Timer 	ViralTimer;
	uint8	viral_timer_counter;
	uint16	adverrorinfo;

	// MobAI stuff
	eStandingPetOrder pStandingPetOrder;
	uint32	minLastFightingDelayMoving;
	uint32	maxLastFightingDelayMoving;
	float	pAggroRange;
	float	pAssistRange;
	Timer*	AIthink_timer;
	Timer*	AImovement_timer;
	Timer*	AItarget_check_timer;
	bool	movetimercompleted;
	bool	permarooted;
	Timer*	AIscanarea_timer;
	Timer*	AIwalking_timer;
	Timer*	AIfeignremember_timer;
	uint32	pLastFightingDelayMoving;
	HateList hate_list;
	std::set<uint32> feign_memory_list;
	// This is to keep track of mobs we cast faction mod spells on
	std::map<uint32,int32> faction_bonuses; // Primary FactionID, Bonus
	void	AddFactionBonus(uint32 pFactionID,int32 bonus);
	int32	GetFactionBonus(uint32 pFactionID);
	// This is to keep track of item faction modifiers
	std::map<uint32,int32> item_faction_bonuses; // Primary FactionID, Bonus
	void	AddItemFactionBonus(uint32 pFactionID,int32 bonus);
	int32	GetItemFactionBonus(uint32 pFactionID);
	void	ClearItemFactionBonuses();

	void CalculateFearPosition();
	uint32 move_tic_count;
	//bool FearTryStraight(Mob *caster, uint32 duration, bool flee, VERTEX &hit, VERTEX &fv);
//	VERTEX fear_vector;
	//FearState fear_state;
	//MobFearState *fear_path_state;
	bool flee_mode;
	Timer flee_timer;

	bool pAIControlled;
	bool roamer;
	bool logging_enabled;

	int wandertype;
	int pausetype;

	int cur_wp;
	float cur_wp_x;
	float cur_wp_y;
	float cur_wp_z;
	int cur_wp_pause;
	float cur_wp_heading;

	int patrol;
	float fear_walkto_x;
	float fear_walkto_y;
	float fear_walkto_z;
	bool curfp;

	// Pathing
	//
	VERTEX PathingDestination;
	VERTEX PathingLastPosition;
	int PathingLoopCount;
	int PathingLastNodeVisited;
	list<int> Route;
	LOSType PathingLOSState;
	Timer *PathingLOSCheckTimer;
	Timer *PathingRouteUpdateTimerShort;
	Timer *PathingRouteUpdateTimerLong;
	bool DistractedFromGrid;
	int PathingTraversedNodes;

	uint32 pDontHealMeBefore;
	uint32 pDontBuffMeBefore;
	uint32 pDontDotMeBefore;
	uint32 pDontRootMeBefore;
	uint32 pDontSnareMeBefore;
	uint32 pDontCureMeBefore;

	// Bind wound
	Timer bindwound_timer;
	Mob* bindwound_target;
	// hp event
	int nexthpevent;
	int nextinchpevent;

	//temppet
	bool hasTempPet;

	EGNode *_egnode;	//the EG node we are in

	bool m_hasRune;
	bool m_hasSpellRune;
	bool m_hasPartialMeleeRune;
	bool m_hasPartialSpellRune;
	bool m_hasDeathSaveChance;
	uint32 m_spellHitsLeft[38]; // Used to track which spells will have their numhits incremented when spell finishes casting, 38 Buffslots
	int	flymode;
    bool m_targetable;
	int QGVarDuration(const char *fmt);
	void InsertQuestGlobal(int charid, int npcid, int zoneid, const char *name, const char *value, int expdate);

private:
	void _StopSong();		//this is not what you think it is
	Mob* target;
};

// All data associated with a single trade
class Trade
{
public:
	Trade(Mob* in_owner);
	virtual ~Trade();

	void Reset();
	void SetTradeCash(uint32 in_pp, uint32 in_gp, uint32 in_sp, uint32 in_cp);

	// Initiate a trade with another mob
	// Also puts other mob into trader mode with this mob
	void Start(uint32 mob_id, bool initiate_with=true);

	// Mob the owner is trading with
	Mob* With();

	// Add item from cursor slot to trade bucket (automatically does bag data too)
	void AddEntity(uint16 from_slot_id, uint16 trade_slot_id);

	// Audit trade
	void LogTrade();

	// Debug only method
	#if (EQDEBUG >= 9)
		void DumpTrade();
	#endif

public:
	// Object state
	TradeState state;
	int32 pp;
	int32 gp;
	int32 sp;
	int32 cp;

private:
	// Send item data for trade item to other person involved in trade
	void SendItemData(const ItemInst* inst, int16 dest_slot_id);

	uint32 with_id;
	Mob* owner;
};

#endif
