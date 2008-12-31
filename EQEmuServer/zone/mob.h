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

#define CON_GREEN		2
#define CON_LIGHTBLUE	18
#define CON_BLUE		4
#define CON_WHITE		20
#define CON_YELLOW		15
#define CON_RED			13

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
#include "map.h"
#include <set>

/*enum FindSpellType {
	SPELLTYPE_SELF,
	SPELLTYPE_OFFENSIVE,
	SPELLTYPE_OTHER
};*/

enum {
	SPECATK_NONE = 0,
	SPECATK_SUMMON,		//S
	SPECATK_ENRAGE,		//E
	SPECATK_RAMPAGE,	//R
	SPECATK_FLURRY,		//F
	SPECATK_TRIPLE,		//T
	SPECATK_QUAD,		//Q
	UNSLOWABLE,			//U
	UNMEZABLE,			//M
	UNCHARMABLE,		//C
	UNSTUNABLE,			//N
	UNSNAREABLE,		//I
	UNFEARABLE,			//D
	IMMUNE_MELEE,		//A
	IMMUNE_MAGIC,		//B
	IMMUNE_FLEEING,		//f
	IMMUNE_MELEE_EXCEPT_BANE,	//O
	IMMUNE_MELEE_NONMAGICAL,	//W
	IMMUNE_AGGRO, //H, wont aggro, ever.
	IMMUNE_TARGET, //G, can't be aggroed, ever
	SPECATK_MAXNUM
				//X,Y,Z are old interactive NPC codes
};

typedef enum {	//fear states
	fearStateNotFeared = 0,
	fearStateRunning,		//I am running, hoping to find a grid at my WP
	fearStateRunningForever,	//can run straight until spell ends
	fearStateGrid,			//I am allready on a fear grid
	fearStateStuck			//I cannot move somehow...
} FearState;

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
	int32	spellid;
	int8		casterlevel;
	int16	casterid;		// Maybe change this to a pointer sometime, but gotta make sure it's 0'd when it no longer points to anything
	int8		durationformula;
	sint32		ticsremaining;
	sint32	poisoncounters;
	sint32	diseasecounters;
	sint32	cursecounters;
	bool	persistant_buff;
	uint32	numhits; //the number of physical hits this buff can take before it fades away, lots of druid armor spells take advantage of this mixed with powerful effects
	bool	client;  //True if the caster is a client
	bool	UpdateClient;
	int16	melee_rune;
	int16	magic_rune;
	int8	deathSaveSuccessChance;
	int8	casterAARank;				// The idea here is if you need to know what the caster AA rank was for a buff long after is has been casted.
	//todo: dot stacking?
};

struct StatBonuses {
	sint16	AC;
	sint32	HP;
	sint32	HPRegen;
	sint32	ManaRegen;
	sint32	EnduranceRegen;
	sint16	Mana;
	sint32	Endurance;
	sint16	ATK;
	//would it be worth it to create a Stat_Struct?
	sint16	STR;
	sint16	STRCapMod;
	sint16	STA;
	sint16	STACapMod;
	sint16	DEX;
	sint16	DEXCapMod;
	sint16	AGI;
	sint16	AGICapMod;
	sint16	INT;
	sint16	INTCapMod;
	sint16	WIS;
	sint16	WISCapMod;
	sint16	CHA;
	sint16	CHACapMod;
	sint16	MR;
	sint16	MRCapMod;
	sint16	FR;
	sint16	FRCapMod;
	sint16	CR;
	sint16	CRCapMod;
	sint16	PR;
	sint16	PRCapMod;
	sint16	DR;
	sint16	DRCapMod;
	uint16	DamageShieldSpellID;
	int		DamageShield; // this is damage done to mobs that attack this
	DmgShieldType	DamageShieldType;
	int		SpellDamageShield;
	int		ReverseDamageShield; // this is damage done to the mob when it attacks
	uint16	ReverseDamageShieldSpellID;
	DmgShieldType	ReverseDamageShieldType;
	int		movementspeed;
	sint8		haste;
	sint8	hastetype2;
	sint8	hastetype3;
	float	AggroRange; // when calculate just replace original value with this
	float	AssistRange;
	sint16	skillmod[HIGHEST_SKILL+1];
	int		effective_casting_level;
	int		reflect_chance;	// chance to reflect incoming spell
	int16	singingMod;
	int16	brassMod;
	int16	percussionMod;
	int16	windMod;
	int16	stringedMod;
	sint8	hatemod;
	sint32	EnduranceReduction;

	//PoP effects:
	sint16   StrikeThrough;          // PoP: Strike Through %
//	sint16   CombatEffects; //AFAIK: Combat Effects == ProcChance
//	sint16   Shielding;     //AFAIK, Shielding == MeleeMitigation
//	sint16   Avoidance;		//AFAIK: Avoidance == AvoidMeleeChance
//	sint16   Accuracy;      //AFAIK: Accuracy == HitChance


	//discipline and PoP effects
	//everything is a straight percent increase unless noted.
	sint16 MeleeMitigation;	//i
	sint16 CriticalHitChance;	//i
	sint16 CrippBlowChance;
	sint16 AvoidMeleeChance;	//AvoidMeleeChance/10 == % chance i
	sint16 RiposteChance;	//i
	sint16 DodgeChance;		//i
	sint16 ParryChance;		//i
	sint16 DualWeildChance;		//i
	sint16 DoubleAttackChance;	//i
	sint16 ResistSpellChance;	//i
	sint16 ResistFearChance;	//i
	sint16 StunResist;		//i
	sint16 MeleeSkillCheck;	//i
	uint8  MeleeSkillCheckSkill;
	sint16 HitChance;			//HitChance/15 == % increase i
	uint8  HitChanceSkill;
	sint16 DamageModifier;		//needs to be thought about more and implemented
	uint8  DamageModifierSkill;
	sint16 MinDamageModifier;   //i
	sint16 ProcChance;			// ProcChance/10 == % increase i
	sint16 ExtraAttackChance;
	sint16 DoTShielding;

	sint8 HundredHands;		//extra haste, stacks with all other haste  i
	bool MeleeLifetap;  //i
};

typedef struct
{
    int16 spellID;
    int8 chance;
    Timer *pTimer;
} tProc;

struct Shielders_Struct {
	int32   shielder_id;
	int16   shielder_bonus;
};

//eventually turn this into a typedef and
//make DoAnim take it instead of int, to enforce its use.
enum {	//type arguments to DoAnim
	animKick				= 1,
	animPiercing			= 2,	//might be piercing?
	anim2HSlashing			= 3,
	anim2HWeapon			= 4,
	anim1HWeapon			= 5,
	animDualWeild			= 6,
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
	petCharmed
} PetType;

class AA_SwarmPetInfo {
public:
	AA_SwarmPetInfo() { target = 0; owner = NULL; duration = NULL;}
	~AA_SwarmPetInfo() { target = 0; owner = NULL; safe_delete(duration); }
	Timer *duration;
	int32 target; //the target ID
	Mob *owner;
};

typedef enum {
	SingleTarget,	// causes effect to spell_target
	AETarget,			// causes effect in aerange of target + target
	AECaster,			// causes effect in aerange of 'this'
	GroupSpell,		// causes effect to caster + target's group
	CastActUnknown
} CastAction_type;

class EGNode;
class MobFearState;

#define MAX_AISPELLS 16
class Mob : public Entity
{
	friend class NPC;	//why do I have to do this for a child to access protected members with ->?
public:
bool logpos;
	enum CLIENT_CONN_STATUS { CLIENT_CONNECTING, CLIENT_CONNECTED, CLIENT_LINKDEAD,
                          CLIENT_KICKED, DISCONNECTED, CLIENT_ERROR, CLIENT_CONNECTINGALL };
	enum eStandingPetOrder { SPO_Follow, SPO_Sit, SPO_Guard };

	//all of this shit is public due to #pf:
	float	tarx;
	float	tary;
	float	tarz;
	int8	tar_ndx;
	float	tar_vector;
	float	tar_vx;
	float	tar_vy;
	float	tar_vz;
	float	test_vector;


	int32	GetPRange(float x, float y, float z);
	static	int32	RandomTimer(int min, int max);
	static	int8	GetDefaultGender(int16 in_race, int8 in_gender = 0xFF);
	static	void	CreateSpawnPacket(EQApplicationPacket* app, NewSpawn_Struct* ns);
//	static	int		CheckEffectIDMatch(int8 effectindex, int16 spellid1, int8 caster_level1, int16 spellid2, int8 caster_level2);


	void	RogueBackstab(Mob* other, bool min_damage = false);
	void	RogueAssassinate(Mob* other); // solar
	bool	BehindMob(Mob* other = 0, float playerx = 0.0f, float playery = 0.0f) const;

	void	TriggerDefensiveProcs(Mob *on);

	Mob(const char*   in_name,
	    const char*   in_lastname,
	    sint32  in_cur_hp,
	    sint32  in_max_hp,
	    int8    in_gender,
	    uint16	in_race,
	    int8    in_class,
        bodyType    in_bodytype,
	    int8    in_deity,
	    int8    in_level,
		int32   in_npctype_id, // rembrant, Dec. 20, 2001
		float	in_size,
		float	in_runspeed,
	    float   in_heading,
	    float	in_x_pos,
	    float	in_y_pos,
	    float	in_z_pos,
	    int8    in_light,
		int8	in_texture,
		int8	in_helmtexture,
		int16	in_ac,
		int16	in_atk,
		int16	in_str,
		int16	in_sta,
		int16	in_dex,
		int16	in_agi,
		int16	in_int,
		int16	in_wis,
		int16	in_cha,
		int8	in_haircolor,
		int8	in_beardcolor,
		int8	in_eyecolor1, // the eyecolors always seem to be the same, maybe left and right eye?
		int8	in_eyecolor2,
		int8	in_hairstyle,
// vesuvias - appearence fix
		int8	in_luclinface,
		int8	in_beard,
		int8	in_aa_title,

		int8	in_see_invis,			// Mongrel: see through invis
		int8	in_see_invis_undead,		// Mongrel: see through invis vs. undead
		int8	in_see_hide,
		int8	in_see_improved_hide,
		sint16	in_hp_regen,
		sint16	in_mana_regen,
		int8	in_qglobal

	);
	virtual ~Mob();

	inline virtual bool IsMob() const { return true; }
	inline virtual bool InZone() const { return true; }
	void	BuffProcess();
	virtual void SetLevel(uint8 in_level, bool command = false) { level = in_level; }
	void	SendLevelAppearance();

	virtual inline sint32 GetPrimaryFaction() const { return 0; }
	virtual uint16 GetSkill(SkillType skill_num) const { return 0; } //overloaded by things which actually have skill (NPC|client)
	virtual void SendWearChange(int8 material_slot);
	virtual int32 GetEquipment(int8 material_slot) const { return(0); }
	virtual sint32 GetEquipmentMaterial(int8 material_slot) const;
	virtual uint32 GetEquipmentColor(int8 material_slot) const;

	void Warp( float x, float y, float z );
	inline bool IsMoving() const { return moving; }
	virtual void SetMoving(bool move) { moving = move; delta_x=0; delta_y=0; delta_z=0; delta_heading=0; }
	virtual void GoToBind() {}
	virtual void Gate();
	virtual bool Attack(Mob* other, int Hand = 13, bool FromRiposte = false) { return false; }		// 13 = Primary (default), 14 = secondary
      bool    AffectedExcludingSlot(int slot, int effect);

#ifdef EQBOTS

	//EQoffline-adds
	virtual bool BotAttackMelee(Mob* other, int Hand = 13, bool FromRiposte = false) { return false; }
	bool BotRaiding;
	bool OrderBotAttack;
	void SetBotRaiding(bool v) { BotRaiding = v; }
	void SetOrderBotAttack(bool v) { OrderBotAttack = v; }
	bool IsBotRaiding() const { return BotRaiding; }
	bool IsOrderBotAttack() const { return OrderBotAttack; }
	bool CheckBotDoubleAttack(bool Triple = false);

#endif //EQBOTS

	virtual void Damage(Mob* from, sint32 damage, int16 spell_id, SkillType attack_skill, bool avoidable = true, sint8 buffslot = -1, bool iBuffTic = false) {};
	virtual void Heal();
	virtual void HealDamage(uint32 ammount, Mob* caster = NULL);
	virtual void SetMaxHP() { cur_hp = max_hp; }
	virtual void Death(Mob* killer, sint32 damage, int16 spell_id, SkillType attack_skill) {}
	static int32 GetLevelCon(int8 mylevel, int8 iOtherLevel);
	inline int32 GetLevelCon(int8 iOtherLevel) const { return(this?GetLevelCon(GetLevel(), iOtherLevel):CON_GREEN); }

	inline virtual void SetHP(sint32 hp) { if (hp >= max_hp) cur_hp = max_hp; else cur_hp = hp;}
	bool ChangeHP(Mob* other, sint32 amount, int16 spell_id = 0, sint8 buffslot = -1, bool iBuffTic = false);
	inline void SetOOCRegen(sint32 newoocregen) {oocregen = newoocregen;}
	int MonkSpecialAttack(Mob* other, int8 skill_used);
	void TryBackstab(Mob *other);
	void DoAnim(const int animnum, int type=0, bool ackreq = true, eqFilterType filter = FilterNone);

	void ChangeSize(float in_size, bool bNoRestriction = false);
	virtual void GMMove(float x, float y, float z, float heading = 0.01);
	void SendPosUpdate(int8 iSendToSelf = 0);
	void MakeSpawnUpdateNoDelta(PlayerPositionUpdateServer_Struct* spu);
	void MakeSpawnUpdate(PlayerPositionUpdateServer_Struct* spu);
	void SendPosition();
	void SendAllPosition();

	void CreateDespawnPacket(EQApplicationPacket* app);
	void CreateHorseSpawnPacket(EQApplicationPacket* app, const char* ownername, uint16 ownerid, Mob* ForWho = 0);
	void CreateSpawnPacket(EQApplicationPacket* app, Mob* ForWho = 0);
	virtual void FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho);
	void CreateHPPacket(EQApplicationPacket* app);
	void SendHPUpdate();

	bool AddRangedProc(int16 spell_id, int8 iChance = 3);
	bool RemoveRangedProc(int16 spell_id, bool bAll = false);
	bool HasRangedProcs() const;

	bool AddDefensiveProc(int16 spell_id, int8 iChance = 3);
	bool RemoveDefensiveProc(int16 spell_id, bool bAll = false);
	bool HasDefensiveProcs() const;

	bool AddProcToWeapon(int16 spell_id, bool bPerma = false, int8 iChance = 3);
	bool RemoveProcFromWeapon(int16 spell_id, bool bAll = false);
	bool HasProcs() const;

	inline bool SeeInvisible() const { return see_invis; }				// Mongrel: Now using the flags
	inline bool SeeInvisibleUndead() const { return see_invis_undead; }   // Mongrel: Now using the flags
	inline bool SeeHide() const { return see_hide; }
	inline bool SeeImprovedHide() const { return see_improved_hide; }

	bool CheckLos(Mob* other);
	bool CheckLosFN(Mob* other);
	inline bool GetQglobal() const {return qglobal;}		// SCORPIOUS2K - return quest global flag

	bool IsInvisible(Mob* other = 0) const;
	void SetInvisible(bool state);

	bool AttackAnimation(SkillType &skillinuse, int Hand, const ItemInst* weapon);
	bool AvoidDamage(Mob* attacker, sint32 &damage);
	bool CheckHitChance(Mob* attacker, SkillType skillinuse, int Hand);
	void TryCriticalHit(Mob *defender, int16 skill, sint32 &damage);
	void TryPetCriticalHit(Mob *defender, int16 skill, sint32 &damage);
	bool TryFinishingBlow(Mob *defender, SkillType skillinuse);
	bool TryHeadShot(Mob* defender, SkillType skillInUse);
	void DoRiposte(Mob* defender);
	void ApplyMeleeDamageBonus(int16 skill, sint32 &damage);
	void MeleeMitigation(Mob *attacker, sint32 &damage, sint32 minhit);

	void	DamageShield(Mob* other);
	bool	FindBuff(int16 spellid);
	bool	FindType(int8 type, bool bOffensive = false, int16 threshold = 100);
	sint8	GetBuffSlotFromType(int8 type);
	int		CountDispellableBuffs();

	void	MakePet(int16 spell_id, const char* pettype, const char *petname = NULL);
//	inline void	MakePetType(int16 spell_id, const char* pettype, const char *petname = NULL) { MakePet(spell_id, pettype, petname); }	//for perl
//	void	MakePet(int16 spell_id, int8 in_level, int8 in_class, int16 in_race, int8 in_texture = 0, int8 in_pettype = 0, float in_size = 0, int8 type = 0, int32 min_dmg = 0, int32 max_dmg = 0, const char *petname = NULL);

	bool	CombatRange(Mob* other);
//	int8	flag[60];		//this is for quests or something...

	virtual inline int16	GetBaseRace()	const { return base_race; }
	virtual inline int8	GetBaseGender()		const { return base_gender; }
	virtual inline int8	GetDeity()			const { return deity; }
	inline int16	GetRace()				const { return race; }
	inline int8	GetGender()					const { return gender; }
	inline int8	GetTexture()				const { return texture; }
	inline int8	GetHelmTexture()			const { return helmtexture; }
	inline int8	GetClass()					const { return class_; }
	inline uint8	GetLevel()				const { return level; }
	inline const char*	GetName()			const { return name; }
	const char *GetCleanName();
	inline Mob*			GetTarget()			const { return target; }
	virtual void SetTarget(Mob* mob);
	virtual inline float		GetHPRatio() const { return max_hp == 0 ? 0 : ((float)cur_hp/max_hp*100); }

	float GetLWDistance()					{ return last_warp_distance; }
	float GetWarpThreshold()				{ return warp_threshold; }

	bool IsLoggingEnabled() const { return(logging_enabled); }
	void EnableLogging() { logging_enabled = true; }
	void DisableLogging() { logging_enabled = false; }
	bool IsWarriorClass() const;
	bool IsAttackAllowed(Mob *target);
	bool IsBeneficialAllowed(Mob *target);

	inline sint32	GetHP()			const { return cur_hp; }
	inline sint32	GetMaxHP()		const { return max_hp; }
	virtual inline sint32	CalcMaxHP()		{ return max_hp = (base_hp  + itembonuses.HP + spellbonuses.HP); }
	float GetWalkspeed() const { return(_GetMovementSpeed(-47)); }
	float GetRunspeed() const { return(_GetMovementSpeed(0)); }
	virtual int GetCasterLevel(int16 spell_id);
	void ApplySpellsBonuses(int16 spell_id, int8 casterlevel, StatBonuses* newbon, int16 casterID = 0);

	inline sint32	GetMaxMana()	const { return max_mana; }
	inline sint32	GetMana()		const { return cur_mana; }
	virtual const sint32& SetMana(sint32 amount);
	inline float	GetManaRatio()	const { return max_mana == 0 ? 100 : (((float)cur_mana/max_mana)*100); }
	void			SetZone(int32 zone_id);

	// neotokyo: moved from client to use in NPC too
	char GetCasterClass() const;
	virtual sint32 CalcMaxMana();

	inline virtual sint16	GetAC()		const { return AC + itembonuses.AC + spellbonuses.AC; } // Quagmire - this is NOT the right math
	inline virtual sint16	GetATK()	const { return ATK + itembonuses.ATK + spellbonuses.ATK; }
	inline virtual sint16	GetSTR()	const { return STR + itembonuses.STR + spellbonuses.STR; }
	inline virtual sint16	GetSTA()	const { return STA + itembonuses.STA + spellbonuses.STA; }
	inline virtual sint16	GetDEX()	const { return DEX + itembonuses.DEX + spellbonuses.DEX; }
	inline virtual sint16	GetAGI()	const { return AGI + itembonuses.AGI + spellbonuses.AGI; }
	inline virtual sint16	GetINT()	const { return INT + itembonuses.INT + spellbonuses.INT; }
	inline virtual sint16	GetWIS()	const { return WIS + itembonuses.WIS + spellbonuses.WIS; }
	inline virtual sint16	GetCHA()	const { return CHA + itembonuses.CHA + spellbonuses.CHA; }
	inline virtual sint16	GetMR() const { return MR + itembonuses.MR + spellbonuses.MR; }
	inline virtual sint16	GetFR()	const { return FR + itembonuses.FR + spellbonuses.FR; }
	inline virtual sint16	GetDR()	const { return DR + itembonuses.DR + spellbonuses.DR; }
	inline virtual sint16	GetPR()	const { return PR + itembonuses.PR + spellbonuses.PR; }
	inline virtual sint16	GetCR() const { return CR + itembonuses.CR + spellbonuses.CR; }

	inline StatBonuses GetItemBonuses() const { return itembonuses; }
	inline StatBonuses GetSpellBonuses() const { return spellbonuses; 
}

	
#ifdef EQBOTS

	void CalcBotStats();

#endif //EQBOTS	
	
	inline virtual sint16  GetMaxSTR() const { return GetSTR(); }
	inline virtual sint16  GetMaxSTA() const { return GetSTA(); }
	inline virtual sint16  GetMaxDEX() const { return GetDEX(); }
	inline virtual sint16  GetMaxAGI() const { return GetAGI(); }
	inline virtual sint16  GetMaxINT() const { return GetINT(); }
	inline virtual sint16  GetMaxWIS() const { return GetWIS(); }
	inline virtual sint16  GetMaxCHA() const { return GetCHA(); }
	inline virtual sint16  GetMaxMR() const { return 255; }
	inline virtual sint16  GetMaxPR() const { return 255; }
	inline virtual sint16  GetMaxDR() const { return 255; }
	inline virtual sint16  GetMaxCR() const { return 255; }
	inline virtual sint16  GetMaxFR() const { return 255; }

	virtual float GetActSpellRange(int16 spell_id, float range){ return range;}
	virtual sint32  GetActSpellDamage(int16 spell_id, sint32 value) { return value; }
	virtual sint32  GetActSpellHealing(int16 spell_id, sint32 value) { return value; }
	virtual sint32 GetActSpellCost(int16 spell_id, sint32 cost){ return cost;}
	virtual sint32 GetActSpellDuration(int16 spell_id, sint32 duration){ return duration;}
	virtual sint32 GetActSpellCasttime(int16 spell_id, sint32 casttime);
	float ResistSpell(int8 resist_type, int16 spell_id, Mob *caster);
	uint16 GetSpecializeSkillValue(int16 spell_id) const;

	void ShowStats(Client* client);
	void ShowBuffs(Client* client);
	void ShowBuffList(Client* client);
	int32 GetNPCTypeID()			const { return npctype_id; } // rembrant, Dec. 20, 2001

	float Dist(const Mob &) const;
	float DistNoZ(const Mob &) const;
	float DistNoRoot(const Mob &) const;
	float DistNoRootNoZ(const Mob &) const;

	bool IsTargeted() const { return targeted; }
	void IsTargeted(bool in_tar) { targeted = in_tar; }

	inline const float	GetX() const		{ return x_pos; }
	inline const float	GetY() const		{ return y_pos; }
	inline const float	GetZ() const		{ return z_pos; }
	inline const float	GetHeading() const	{ return heading; }
	inline const float	GetSize() const		{ return size; }
	inline void			SetChanged()		{ pLastChange = Timer::GetCurrentTime(); }
	inline const int32	LastChange() const	{ return pLastChange; }

	void	SetFollowID(int32 id) { follow = id; }
	int32	GetFollowID()		  const { return follow; }

	virtual void	Message(int32 type, const char* message, ...) {} // fake so dont have to worry about typecasting
	virtual void	Message_StringID(int32 type, int32 string_id, int32 distance = 0) {}
	virtual void	Message_StringID(int32 type, int32 string_id, const char* message,const char* message2=0,const char* message3=0,const char* message4=0,const char* message5=0,const char* message6=0,const char* message7=0,const char* message8=0, const char* message9=0, int32 distance = 0) {}
	void Say(const char *format, ...);
	void Say_StringID(int32 string_id, const char *message3 = 0, const char *message4 = 0, const char *message5 = 0, const char *message6 = 0, const char *message7 = 0, const char *message8 = 0, const char *message9 = 0);
	void Shout(const char *format, ...);
	void Emote(const char *format, ...);
	void QuestJournalledSay(Client *QuestInitiator, const char *str);


	virtual void SpellProcess();
	virtual bool CheckFizzle(int16 spell_id);
	void ZeroCastingVars();
	bool UseBardSpellLogic(int16 spell_id = 0xffff, int slot = -1);
	void InterruptSpell(int16 spellid = SPELL_UNKNOWN);
	void InterruptSpell(int16, int16, int16 spellid = SPELL_UNKNOWN);
	bool	CastSpell(int16 spell_id, int16 target_id, int16 slot = 10, sint32 casttime = -1, sint32 mana_cost = -1, int32* oSpellWillFinish = 0, int32 item_slot = 0xFFFFFFFF);
	bool	DoCastSpell(int16 spell_id, int16 target_id, int16 slot = 10, sint32 casttime = -1, sint32 mana_cost = -1, int32* oSpellWillFinish = 0, int32 item_slot = 0xFFFFFFFF);
	void	CastedSpellFinished(int16 spell_id, int32 target_id, int16 slot, int16 mana_used, int32 inventory_slot = 0xFFFFFFFF);
	bool	SpellFinished(int16 spell_id, Mob *target, int16 slot = 10, int16 mana_used = 0, int32 inventory_slot = 0xFFFFFFFF);
	bool	SpellOnTarget(int16 spell_id, Mob* spelltar);
	bool	ApplyNextBardPulse(int16 spell_id, Mob *spell_target, int16 slot);
	void	BardPulse(uint16 spell_id, Mob *caster);
	bool	DetermineSpellTargets(uint16 spell_id, Mob *&spell_target, Mob *&ae_center, CastAction_type &CastAction);
	int		CalcBuffDuration(Mob *caster, Mob *target, int16 spell_id);
	void	SendPetBuffsToClient();
//	int		CheckAddBuff(Mob* caster, const int16& spell_id, const int& caster_level, int* buffdur, int ticsremaining = -1);
	int		AddBuff(Mob *caster, const int16 spell_id, int duration = 0);
	bool	SpellEffect(Mob* caster, int16 spell_id, float partial = 100);
	bool	IsImmuneToSpell(int16 spell_id, Mob *caster);
	void	DoBuffTic(int16 spell_id, int32 ticsremaining, int8 caster_level, Mob* caster = 0);
	void	BuffFadeBySpellID(int16 spell_id);
	void	BuffFadeByEffect(int effectid, int skipslot = -1);
	void	BuffFadeAll();
	void	BuffFadeDetrimental();
	void	BuffFadeBySlot(int slot, bool iRecalcBonuses = true);
	int		CanBuffStack(int16 spellid, int8 caster_level, bool iFailIfOverwrite = false);
	inline	bool	IsCasting() const { return((casting_spell_id != 0)); }
	int16	CastingSpellID() const { return casting_spell_id; }
	float	GetAOERange(uint16 spell_id);
	void	TemporaryPets(int16 spell_id, Mob *target, const char *name_override = NULL, uint32 duration_override = 0);
	void	WakeTheDead(int16 spell_id, Mob *target, uint32 duration);
	void	TryDotCritical(int16 spell_id, Mob *caster, int &damage);

// vesuvias - appearence fix
	void	SendIllusionPacket(int16 in_race, int8 in_gender = 0xFF, int16 in_texture = 0xFFFF, int16 in_helmtexture = 0xFFFF, int8 in_haircolor = 0xFF, int8 in_beardcolor = 0xFF, int8 in_eyecolor1 = 0xFF, int8 in_eyecolor2 = 0xFF, int8 in_hairstyle = 0xFF, int8 in_luclinface = 0xFF, int8 in_beard = 0xFF, int8 in_aa_title = 0xFF);

	static	int32	GetAppearanceValue(EmuAppearance iAppearance);
	void	SendAppearancePacket(int32 type, int32 value, bool WholeZone = true, bool iIgnoreSelf = false, Client *specific_target=NULL);
	void	SetAppearance(EmuAppearance app, bool iIgnoreSelf = true);
	inline EmuAppearance	GetAppearance()	const { return _appearance; }
	inline const int8	GetRunAnimSpeed()	const { return pRunAnimSpeed; }
	inline void			SetRunAnimSpeed(sint8 in)	{ if (pRunAnimSpeed != in) { pRunAnimSpeed = in; pLastChange = Timer::GetCurrentTime(); } }

	Mob*	GetPet();
	void	SetPet(Mob* newpet);
	Mob*	GetOwner();
	Mob*	GetOwnerOrSelf();
	void	SetPetID(int16 NewPetID);
	inline int16	GetPetID()		const			{ return petid;  }
	inline PetType GetPetType() const { return typeofpet; }
	bool IsFamiliar() const { return(typeofpet == petFamiliar); }
	bool IsAnimation() const { return(typeofpet == petAnimation); }
	void SetOwnerID(int16 NewOwnerID);
	inline int16 GetOwnerID()	const			{ return ownerid; }
	inline bool HasOwner() const { if(GetOwnerID()==0){return false;} return( entity_list.GetMob(GetOwnerID()) != 0); }
	inline bool IsPet() const { return(HasOwner()); }
	inline bool HasPet() const { if(GetPetID()==0){return false;} return (entity_list.GetMob(GetPetID()) != 0);}
	bool HadTempPets() const { return(hasTempPet); }
	void TempPets(bool i) { hasTempPet = i; }

    inline const	bodyType	GetBodyType() const	{ return bodytype; }
//    int16   FindSpell(int16 classp, int16 level, int type, FindSpellType spelltype, float distance, sint32 mana_avail);
//	void	CheckBuffs();
//	bool	CheckSelfBuffs();
//	void	CheckPet();

 	void    SendSpellBarDisable();
 	void    SendSpellBarEnable(int16 spellid);
 	virtual void    Stun(int duration);
	virtual void	UnStun();
	inline void Silence(bool newval) { silenced = newval; }

	bool	invulnerable;
	bool	invisible, invisible_undead, invisible_animals, sneaking, hidden, improved_hidden;
	bool	see_invis, see_invis_undead, see_hide, see_improved_hide;   // Mongrel: See Invis and See Invis vs. Undead
	bool	qglobal;		// SCORPIOUS2K - qglobal flag

	void	Spin();
	void	Kill();

	void	SetAttackTimer();
	inline void	SetInvul(bool invul) { invulnerable=invul; }
	inline bool	GetInvul(void) { return invulnerable; }
	inline void	SetExtraHaste(int Haste) { ExtraHaste = Haste; }
	virtual int GetHaste();

	int8		GetWeaponDamageBonus(const Item_Struct* Weapon);
	int		GetMonkHandToHandDamage(void);

	bool	CanThisClassDoubleAttack(void) const;
	bool	CanThisClassDualWield(void) const;
	bool	CanThisClassRiposte(void) const;
	bool	CanThisClassDodge(void) const;
	bool	CanThisClassParry(void) const;
	bool	CanThisClassBlock(void) const;

	int	GetMonkHandToHandDelay(void);
	int16	GetClassLevelFactor();
	void	Mesmerize();
	inline bool	IsMezzed() const { return mezzed; }
	inline bool	IsStunned() const { return stunned; }
	inline bool	IsSilenced() const { return silenced; }
	inline int16	GetErrorNumber() const {return adverrorinfo;}

	sint32	ReduceDamage(sint32 damage);
	sint32  ReduceMagicalDamage(sint32 damage);

#define MAX_RAMPAGE_TARGETS 3
#define MAX_RAMPAGE_LIST 20
#define MAX_FLURRY_HITS 2
    bool Flurry();
    bool Rampage();
    bool AddRampage(Mob*);
	void ClearRampage();

    void StartEnrage();
	void ProcessEnrage();
    bool IsEnraged();
	void Taunt(NPC* who, bool always_succeed);

	virtual void		AI_Init();
	virtual void		AI_Start(int32 iMoveDelay = 0);
	virtual void		AI_Stop();
	void				AI_Process();

#ifdef EQBOTS

	/*Franck-add: EQoffline  */
	void				BOT_Process();
	void				PET_Process();
	/*-----------------------*/
	
#endif //EQBOTS

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
	inline const bool	IsRoamer() const { return roamer; }
	inline const bool   IsRooted() const { return rooted || permarooted; }
	int					GetSnaredAmount();

	bool				RemoveFromHateList(Mob* mob);
    void				AddToHateList(Mob* other, sint32 hate = 0, sint32 damage = 0, bool iYellForHelp = true, bool bFrenzy = false, bool iBuffTic = false);
	void				SetHate(Mob* other, sint32 hate = 0, sint32 damage = 0) {hate_list.Set(other,hate,damage);}
	int32				GetHateAmount(Mob* tmob, bool is_dam = false)  {return hate_list.GetEntHate(tmob,is_dam);}
	int32				GetDamageAmount(Mob* tmob)  {return hate_list.GetEntHate(tmob, true);}
	Mob*				GetHateTop()  {return hate_list.GetTop(this);}
	Mob*				GetHateDamageTop(Mob* other)  {return hate_list.GetDamageTop(other);}
	Mob*				GetHateRandom()  {return hate_list.GetRandom();}
	//this is diff from GetHateTop as it does not account for things like frenzy, just the person with the most hate
	Mob*				GetHateMost()	{return hate_list.GetMostHate();}
	bool				IsEngaged()   {return(!hate_list.IsEmpty()); }
	bool				HateSummon();
	void				FaceTarget(Mob* MobToFace = 0);
	void				SetHeading(float iHeading) { if (heading != iHeading) { pLastChange = Timer::GetCurrentTime(); heading = iHeading; } }
	void				WipeHateList(); //Wipe?
	// EverHood 6/14/06
	// Mobs need to be able to remember more than one feigned attacker
	void				AddFeignMemory(Client* attacker);
	void				RemoveFromFeignMemory(Client* attacker);
	void				ClearFeignMemory();
	void				PrintHateListToClient(Client *who) { hate_list.PrintToClient(who); }


	int					GetCurWp(){ return cur_wp; }

	//old fear function
	//void SetFeared(Mob *caster, int32 duration, bool flee = false);
	float GetFearSpeed();
	bool IsFeared() { return curfp; } // This returns true if the mob is feared or fleeing due to low HP
	//old fear: inline void StartFleeing() { SetFeared(GetHateTop(), FLEE_RUN_DURATION, true); }
	inline void StartFleeing() { flee_mode = true; CalculateNewFearpoint(); }
	void ProcessFlee();
	void CheckFlee();

	inline bool			CheckAggro(Mob* other) {return hate_list.IsOnHateList(other);}
    float				CalculateHeadingToTarget(float in_x, float in_y);
    bool				CalculateNewPosition(float x, float y, float z, float speed, bool checkZ = false);
	bool				CalculateNewPosition2(float x, float y, float z, float speed, bool checkZ = true);
    float				CalculateDistance(float x, float y, float z);
	void				SendTo(float new_x, float new_y, float new_z);
	void				SendToFixZ(float new_x, float new_y, float new_z);
	void				NPCSpecialAttacks(const char* parse, int permtag);
	inline int32		DontHealMeBefore() const { return pDontHealMeBefore; }
	inline int32		DontBuffMeBefore() const { return pDontBuffMeBefore; }
	inline int32		DontDotMeBefore() const { return pDontDotMeBefore; }
	inline int32		DontRootMeBefore() const { return pDontRootMeBefore; }
	inline int32		DontSnareMeBefore() const { return pDontSnareMeBefore; }

	// calculate interruption of spell via movement of mob
	void SaveSpellLoc() {spell_x = x_pos; spell_y = y_pos; spell_z = z_pos; }
	inline float GetSpellX() const {return spell_x;}
	inline float GetSpellY() const {return spell_y;}
	inline float GetSpellZ() const {return spell_z;}
	inline bool	IsGrouped()	 const { return isgrouped; }
	void SetGrouped(bool v) { isgrouped = v; }
	inline bool IsRaidGrouped() const { return israidgrouped; }
	void SetRaidGrouped(bool v) { israidgrouped = v; }

	bool CheckWillAggro(Mob *mob);

	void	InstillDoubt(Mob *who);
	sint16	GetResist(int8 type) const;
	Mob*	GetShieldTarget()			const { return shield_target; }
	void	SetShieldTarget(Mob* mob)	{ shield_target = mob; }
//	Mob*	GetSongTarget()			{ return bardsong_target; }
//	void	SetSongTarget(Mob* mob)	{ bardsong_target = mob; }
	bool	HasActiveSong() const { return(bardsong != 0); }
	bool	Charmed() const { return charmed; }
	static int32	GetLevelHP(int8 tlevel);
	int32	GetZoneID() const;	//for perl
	sint32	CheckAggroAmount(int16 spellid);
	sint32	CheckHealAggroAmount(int16 spellid);
	virtual int32 GetAA(int32 aa_id) const { return(0); }

	int16	GetInstrumentMod(int16 spell_id) const;
	int CalcSpellEffectValue(int16 spell_id, int effect_id, int caster_level = 1, Mob *caster = NULL, int ticsremaining = 0);
	int CalcSpellEffectValue_formula(int formula, int base, int max, int caster_level, int16 spell_id, int ticsremaining = 0);
	int CheckStackConflict(int16 spellid1, int caster_level1, int16 spellid2, int caster_level2, Mob* caster1 = NULL, Mob* caster2 = NULL);

//	inline EGNode *GetEGNode() { return(_egnode); }
//	inline void SetEGNode(EGNode *s) { _egnode = s; }


  // HP Event
   inline int GetNextHPEvent() const { return nexthpevent; }
   void SetNextHPEvent( int hpevent );
	void SendItemAnimation(Mob *to, const Item_Struct *item);
	inline int& GetNextIncHPEvent() { return nextinchpevent; }
	void SetNextIncHPEvent( int inchpevent );

	bool DivineAura() const;
    bool SpecAttacks[SPECATK_MAXNUM];
	Shielders_Struct shielder[MAX_SHIELDERS];
	Trade* trade;

	Timer cheat_timer; //Lieka:  Timer used to check for movement exemptions/client-based, unsolicited zone exemptions
	Timer threshold_timer;  //Null:  threshold timer
	float warp_threshold;   //Null:  threshold for warp detector
	float last_warp_distance;  //Null:  last distance logged as a warp, used for logs and #showstats

	//temporary:
	bool fix_pathing;
	float rewind_x; //Lieka:  Used for storing /rewind values
	float rewind_y; //Lieka:  Used for storing /rewind values
	float rewind_z; //Lieka:  Used for storing /rewind values
	Timer rewind_timer; //Lieka:  Used for measuring how long a player stays in one spot
	inline float GetCWPX() const { return(cur_wp_x); }
	inline float GetCWPY() const { return(cur_wp_y); }
	inline float GetCWPZ() const { return(cur_wp_z); }
	inline float GetCWPP() const { return(cur_wp_pause); }
	inline int GetCWP() const { return(cur_wp); }
	virtual FACTION_VALUE GetReverseFactionCon(Mob* iOther) { return FACTION_INDIFFERENT; }
	
#ifdef EQBOTS

    //franck-add: EQoffline
	void MakeBot(Mob *m);
	bool AmIaBot;
	bool cast_last_time;
	bool IsBot() { return AmIaBot; }
	int GetBotLeader();
	sint32 GetBotActSpellDamage(int16 spell_id, sint32 value);
	void BotMeditate(bool isSitting);
	Mob *BotOwner;
	int BotRaidID;
	void SetBotRaidID( int rId ) { BotRaidID = rId; }
	int GetBotRaidID() { return BotRaidID; }
	
#endif //EQBOTS

	inline bool IsTrackable() const { return(trackable); }
	inline bool HasRune() const { return m_hasRune; }
	inline bool HasSpellRune() const { return m_hasSpellRune; }
	inline void SetHasRune(bool hasRune) { m_hasRune = hasRune; }
	inline void SetHasSpellRune(bool hasSpellRune) { m_hasSpellRune = hasSpellRune; }
	inline bool HasDeathSaveChance() const { return m_hasDeathSaveChance; }
	inline void SetDeathSaveChance(bool hasDeathSaveChance) { m_hasDeathSaveChance = hasDeathSaveChance; }
	bool PassCharismaCheck(Mob* caster, Mob* spellTarget, int16 spell_id);
	bool TryDeathSave();

protected:
	void CommonDamage(Mob* other, sint32 &damage, const uint16 spell_id, const SkillType attack_skill, bool &avoidable, const sint8 buffslot, const bool iBuffTic);
	static uint16 GetProcID(uint16 spell_id, uint8 effect_index);
	float _GetMovementSpeed(int mod) const;

	virtual bool AI_EngagedCastCheck() { return(false); }
	virtual bool AI_PursueCastCheck() { return(false); }
	virtual bool AI_IdleCastCheck() { return(false); }
	
#ifdef EQBOTS

	//franck-add:EQoffline
	virtual bool Bot_AI_IdleCastCheck() { return(false); }
	virtual bool Bot_AI_EngagedCastCheck() { return(false); }
	virtual bool Bot_AI_PursueCastCheck() { return(false); }
	virtual bool Bot_Command_MezzTarget(Mob *target) { return(false); }
	virtual bool Bot_Command_RezzTarget(Mob *target) { return(false); }
	virtual bool Bot_Command_Cure(int curetype, int level) { return(false); }
	virtual bool Bot_Command_CalmTarget(Mob *target) { return(false); }

#endif //EQBOTS
	
	//used by mlog() for VC6
	#ifdef NO_VARIADIC_MACROS
	void mob_log(LogType type, const char *fmt, ...);
	void mob_hex(LogType type, const char *data, unsigned long length, unsigned char padding=4);
	#endif

	bool	IsFullHP;
	bool	moved;

    char RampageArray[MAX_RAMPAGE_LIST][64];

	bool	isgrouped; //These meant to be private?
	bool	israidgrouped;
	bool	pendinggroup;
	int8	texture;
	int8	helmtexture;

	int	AC;
	sint16	ATK;
	sint16	STR;
	sint16	STA;
	sint16	DEX;
	sint16	AGI;
	sint16	INT;
	sint16	WIS;
	sint16	CHA;
	sint16 MR;
	sint16 CR;
	sint16 FR;
	sint16 DR;
	sint16 PR;
	bool moving;
	bool targeted;
	bool findable;
	bool trackable;
	sint32  cur_hp;
	sint32  max_hp;
	sint32	base_hp;
	sint32	cur_mana;
	sint32	max_mana;
	sint16	hp_regen;
	sint16	mana_regen;
	sint32	oocregen; //Out of Combat Regen, % per tick
	Buffs_Struct	buffs[BUFF_COUNT];
	StatBonuses		itembonuses;
	StatBonuses		spellbonuses;
	StatBonuses		aabonuses;	//AndMetal
	int16			petid;
	int16			ownerid;
	PetType			typeofpet;

	int32			follow;

	int8    gender;
	int16	race;
	int8	base_gender;
	int16	base_race;
	int8    class_;
	bodyType    bodytype;
	int16	deity;
	uint8    level;
	int32   npctype_id; // rembrant, Dec. 20, 2001
	float	x_pos;
	float	y_pos;
	float	z_pos;
	float	heading;
	uint16	animation;
	float	size;
	float	runspeed;
	int32 pLastChange;
	bool held;
	void CalcSpellBonuses(StatBonuses* newbon);
	virtual void CalcBonuses();
	void TryDefensiveProc(Mob *on);
	void TryWeaponProc(const Item_Struct* weapon, Mob *on);
	void TryWeaponProc(const ItemInst* weapon, Mob *on);
	void ExecWeaponProc(uint16 spell_id, Mob *on);
	float GetProcChances(float &ProcBonus, float &ProcChance, uint16 weapon_speed = 30);
	int GetWeaponDamage(Mob *against, const Item_Struct *weapon_item);
	int GetWeaponDamage(Mob *against, const ItemInst *weapon_item);
	int GetKickDamage() const;
	int GetBashDamage() const;
	void DoSpecialAttackDamage(Mob *who, SkillType skill, sint32 max_damage, sint32 min_damage = 1);
	bool HasDied();
	void CalculateNewFearpoint();
	float FindGroundZ(float new_x, float new_y, float z_offset=0.0);

	enum {MAX_PROCS = 4};
	tProc PermaProcs[MAX_PROCS];
	tProc SpellProcs[MAX_PROCS];
	tProc DefensiveProcs[MAX_PROCS];
	tProc RangedProcs[MAX_PROCS];

	char    name[64];
	char		clean_name[64];
	char    lastname[32];

    bool bEnraged;
    Timer *SpecAttackTimers[SPECATK_MAXNUM];

	sint32	delta_heading;
    float	delta_x;
	float	delta_y;
	float	delta_z;
//	uint32	guildeqid; // guild's EQ ID, 0-511, 0xFFFFFFFF = none

	int8    light;

	float	fixedZ;
	EmuAppearance    _appearance;
	int8	pRunAnimSpeed;

	Mob*	target;
	Timer	attack_timer;
	Timer	attack_dw_timer;
	Timer	ranged_timer;
	float	attack_speed;		//% increase/decrease in attack speed (not haste)
	Timer	tic_timer;
	Timer	mana_timer;

	//spell casting vars
	Timer spellend_timer;
	int16	casting_spell_id;
    float spell_x, spell_y, spell_z;
	int	attacked_count;
	bool	delaytimer;
	int16 casting_spell_targetid;
	int16 casting_spell_slot;
	int16 casting_spell_mana;
	int32 casting_spell_inventory_slot;
	int16	bardsong;
	int8	bardsong_slot;
//	Mob*	bardsong_target;
	int32	bardsong_target_id;


	int8	haircolor;
	int8	beardcolor;
	int8	eyecolor1; // the eyecolors always seem to be the same, maybe left and right eye?
	int8	eyecolor2;
	int8	hairstyle;
	int8	luclinface; // and beard
// vesuvias - appearence fix
	int8	beard;

	int8	aa_title;

	Mob*	shield_target;

	int ExtraHaste;	// for the #haste command
	bool	mezzed;
	bool	stunned;
	bool	charmed;	//this isnt fully implemented yet
	bool	rooted;
	bool	silenced;
	bool	inWater;	// Set to true or false by Water Detection code if enabled by rules
//	Timer mezzed_timer;
	Timer  stunned_timer;
	Timer	bardsong_timer;
	int16	adverrorinfo;

	// MobAI stuff
	eStandingPetOrder pStandingPetOrder;
	int32	minLastFightingDelayMoving;
	int32	maxLastFightingDelayMoving;
	float	pAggroRange;
	float	pAssistRange;
	Timer*	AIthink_timer;
	Timer*	AImovement_timer;
	bool	movetimercompleted;
	bool   permarooted;
	Timer*	AIscanarea_timer;
	Timer*	AIwalking_timer;
	Timer*	AIfeignremember_timer;
	int32	pLastFightingDelayMoving;
	HateList hate_list;
	std::set<int32> feign_memory_list;
	// EverHood - This is to keep track of mobs we cast faction mod spells on
	std::map<uint32,sint32> faction_bonuses; // Primary FactionID, Bonus
	void	AddFactionBonus(uint32 pFactionID,sint32 bonus);
	sint32	GetFactionBonus(uint32 pFactionID);

	void CalculateFearPosition();
	//bool FearTryStraight(Mob *caster, int32 duration, bool flee, VERTEX &hit, VERTEX &fv);
//	VERTEX fear_vector;
	//FearState fear_state;
	//MobFearState *fear_path_state;
	bool flee_mode;
	Timer flee_timer;

	bool	pAIControlled;
	bool	roamer;
	bool	logging_enabled;

	int		wandertype;
	int		pausetype;

	int		cur_wp;
	float		cur_wp_x;
	float		cur_wp_y;
	float		cur_wp_z;
	int		cur_wp_pause;

	int		patrol;
	float fear_walkto_x;
    float fear_walkto_y;
    float fear_walkto_z;
	bool curfp;


	int32	pDontHealMeBefore;
	int32	pDontBuffMeBefore;
	int32	pDontDotMeBefore;
	int32	pDontRootMeBefore;
	int32	pDontSnareMeBefore;

	// Bind wound
	Timer  bindwound_timer;
	Mob*    bindwound_target;
	// hp event
	int nexthpevent;
	int nextinchpevent;

	//temppet
	bool hasTempPet;

	EGNode *_egnode;	//the EG node we are in

	bool	m_hasRune;
	bool	m_hasSpellRune;
	bool	m_hasDeathSaveChance;

private:
	void	_StopSong();		//this is not what you think it is
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
	void AddEntity(int16 from_slot_id, int16 trade_slot_id);

	// Audit trade
	void LogTrade();

	// Debug only method
	#if (EQDEBUG >= 9)
		void DumpTrade();
	#endif

public:
	// Object state
	TradeState state;
	sint32 pp;
	sint32 gp;
	sint32 sp;
	sint32 cp;

private:
	// Send item data for trade item to other person involved in trade
	void SendItemData(const ItemInst* inst, sint16 dest_slot_id);

	uint32 with_id;
	Mob* owner;
};

#endif
