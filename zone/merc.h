#ifndef MERC_H
#define MERC_H
#include "mob.h"
#include "zonedb.h"
#include "npc.h"
using namespace std;


const int MercAISpellRange = 100; // TODO: Write a method that calcs what the merc's spell range is based on spell, equipment, AA, whatever and replace this

class Merc : public NPC {
public:
	Merc(const NPCType* d, float x, float y, float z, float heading);
	virtual ~Merc();

	//abstract virtual function implementations requird by base abstract class
	virtual void Death(Mob* killerMob, sint32 damage, int16 spell_id, SkillType attack_skill);
	virtual void Damage(Mob* from, sint32 damage, int16 spell_id, SkillType attack_skill, bool avoidable = true, sint8 buffslot = -1, bool iBuffTic = false);
	virtual bool Attack(Mob* other, int Hand = SLOT_PRIMARY, bool FromRiposte = false, bool IsStrikethrough = false, bool IsFromSpell = false);
	virtual bool HasRaid() { return false; }
	virtual bool HasGroup() { return (GetGroup() ? true : false); }
	virtual Raid* GetRaid() { return 0; }
	virtual Group* GetGroup() { return entity_list.GetGroupByMob(this); }

	// Mob AI Virtual Override Methods
	virtual void AI_Stop();
	virtual void AI_Process();

	virtual bool Process();

	// Static Merc Group Methods
	static bool AddMercToGroup(Merc* merc, Group* group);
	static bool RemoveMercFromGroup(Merc* merc, Group* group);
	void ProcessClientZoneChange(Client* mercOwner);

	virtual bool IsMerc() const { return true; }

	virtual void FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho);
	static Merc* LoadMerc(Client *c, MercTemplate* merc_template, uint32 merchant_id);
	static const char *GetRandomName();
	bool Spawn(Client *owner);
	bool Dismiss();
	bool Suspend();
	bool Unsuspend();
	void Zone();
	virtual void Depop();
	bool GetDepop() { return p_depop; }

	bool IsDead() { return GetHP() < 0;};
	bool IsMedding() {return _medding; };
	bool IsSuspended() {return _suspended; };

	static int32 CalcPurchaseCost( uint32 templateID , uint8 level, uint8 currency_type = 0);
	static int32 CalcUpkeepCost( uint32 templateID , uint8 level, uint8 currency_type = 0);

	// "GET" Class Methods
	virtual Mob* GetOwner();
	Client* GetMercOwner();
	virtual Mob* GetOwnerOrSelf();
	uint32 GetMercID() { return _MercID; }
	uint32 GetMercCharacterID( ) { return owner_char_id; }
	uint32 GetMercTemplateID() { return _MercTemplateID; }
	uint32 GetMercType() { return _MercType; }
	uint32 GetMercSubType() { return _MercSubType; }
	uint32 GetProficiencyID() { return _ProficiencyID; }
	uint32 GetCostFormula() { return _CostFormula; }
	uint32 GetMercNameType() { return _NameType; }
	inline const uint8 GetClientVersion() const { return _OwnerClientVersion; }

	bool	HasSkill(SkillType skill_id) const;
	bool	CanHaveSkill(SkillType skill_id) const;
	int16	MaxSkill(SkillType skillid, int16 class_, int16 level) const;
	inline	int16	MaxSkill(SkillType skillid) const { return MaxSkill(skillid, GetClass(), GetLevel()); }

	// stat functions
	virtual void CalcBonuses();
	inline virtual sint16	GetAC()		const { return AC; }
	inline virtual sint16 GetATK() const { return ATK + itembonuses.ATK + spellbonuses.ATK + ((GetSTR() + GetSkill(OFFENSE)) * 9 / 10); }
	inline virtual sint16 GetATKBonus() const { return itembonuses.ATK + spellbonuses.ATK; }
	int GetRawACNoShield(int &shield_ac) const;

	inline virtual sint16	GetSTR()	const { return STR; }
	inline virtual sint16	GetSTA()	const { return STA; }
	inline virtual sint16	GetDEX()	const { return DEX; }
	inline virtual sint16	GetAGI()	const { return AGI; }
	inline virtual sint16	GetINT()	const { return INT; }
	inline virtual sint16	GetWIS()	const { return WIS; }
	inline virtual sint16	GetCHA()	const { return CHA; }
	inline virtual sint16	GetMR()	const { return MR; }
	inline virtual sint16	GetFR()	const { return FR; }
	inline virtual sint16	GetDR()	const { return DR; }
	inline virtual sint16	GetPR()	const { return PR; }
	inline virtual sint16	GetCR()	const { return CR; }
	inline virtual sint16	GetCorrup()	const { return Corrup; }
	
	sint16	GetMaxStat() const;
	sint16	GetMaxResist() const;
	sint16  GetMaxSTR() const;
    sint16  GetMaxSTA() const;
    sint16  GetMaxDEX() const;
    sint16  GetMaxAGI() const;
    sint16  GetMaxINT() const;
    sint16  GetMaxWIS() const;
    sint16  GetMaxCHA() const;
	sint16  GetMaxMR() const;
	sint16  GetMaxPR() const;
	sint16  GetMaxDR() const;
	sint16  GetMaxCR() const;
	sint16  GetMaxFR() const;
	sint16  GetMaxCorrup() const;

	inline virtual sint16	GetHeroicSTR()	const { return itembonuses.HeroicSTR; }
	inline virtual sint16	GetHeroicSTA()	const { return itembonuses.HeroicSTA; }
	inline virtual sint16	GetHeroicDEX()	const { return itembonuses.HeroicDEX; }
	inline virtual sint16	GetHeroicAGI()	const { return itembonuses.HeroicAGI; }
	inline virtual sint16	GetHeroicINT()	const { return itembonuses.HeroicINT; }
	inline virtual sint16	GetHeroicWIS()	const { return itembonuses.HeroicWIS; }
	inline virtual sint16	GetHeroicCHA()	const { return itembonuses.HeroicCHA; }
	inline virtual sint16	GetHeroicMR()	const { return itembonuses.HeroicMR; }
	inline virtual sint16	GetHeroicFR()	const { return itembonuses.HeroicFR; }
	inline virtual sint16	GetHeroicDR()	const { return itembonuses.HeroicDR; }
	inline virtual sint16	GetHeroicPR()	const { return itembonuses.HeroicPR; }
	inline virtual sint16	GetHeroicCR()	const { return itembonuses.HeroicCR; }
	inline virtual sint16	GetHeroicCorrup()	const { return itembonuses.HeroicCorrup; }
	// Mod2
	inline virtual sint16	GetShielding()		const { return itembonuses.MeleeMitigation; }
	inline virtual sint16	GetSpellShield()	const { return itembonuses.SpellShield; }
	inline virtual sint16	GetDoTShield()		const { return itembonuses.DoTShielding; }
	inline virtual sint16	GetStunResist()		const { return itembonuses.StunResist; }
	inline virtual sint16	GetStrikeThrough()	const { return itembonuses.StrikeThrough; }
	inline virtual sint16	GetAvoidance()		const { return itembonuses.AvoidMeleeChance; }
	inline virtual sint16	GetAccuracy()		const { return itembonuses.HitChance; }
	inline virtual sint16	GetCombatEffects()	const { return itembonuses.ProcChance; }
	inline virtual sint16	GetDS()				const { return itembonuses.DamageShield; }
	// Mod3
	inline virtual sint16	GetHealAmt()		const { return itembonuses.HealAmt; }
	inline virtual sint16	GetSpellDmg()		const { return itembonuses.SpellDmg; }
	inline virtual sint16	GetClair()			const { return itembonuses.Clairvoyance; }
	inline virtual sint16	GetDSMit()			const { return itembonuses.DSMitigation; }
	
	inline virtual sint16	GetSingMod()		const { return itembonuses.singingMod; }
	inline virtual sint16	GetBrassMod()		const { return itembonuses.brassMod; }
	inline virtual sint16	GetPercMod()		const { return itembonuses.percussionMod; }
	inline virtual sint16	GetStringMod()		const { return itembonuses.stringedMod; }
	inline virtual sint16	GetWindMod()		const { return itembonuses.windMod; }
	
	inline virtual sint16	GetDelayDeath()		const { return aabonuses.DelayDeath + spellbonuses.DelayDeath + itembonuses.DelayDeath + 11; }

	// "SET" Class Methods
	void SetMercData (uint32 templateID );
	void SetMercID( uint32 mercID ) { _MercID = mercID; }
	void SetMercCharacterID( uint32 mercID ) { owner_char_id = mercID; }
	void SetMercTemplateID( uint32 templateID ) { _MercTemplateID = templateID; }
	void SetMercType( uint32 type ) { _MercType = type; }
	void SetMercSubType( uint32 subtype ) { _MercSubType = subtype; }
	void SetProficiencyID( uint8 proficiency_id ) { _ProficiencyID = proficiency_id; }
	void SetCostFormula( uint8 costformula ) { _CostFormula = costformula; }
	void SetMercNameType( uint8 nametype ) { _NameType = nametype; }
	void SetClientVersion(uint8 clientVersion) { _OwnerClientVersion = clientVersion; }
	void SetSuspended(bool suspended) { _suspended = suspended; }

	void Sit();
	void Stand();
	bool IsSitting();
	bool IsStanding();

	// Merc-specific functions
	bool IsMercCaster() { return (GetClass() == CLERIC || GetClass() == DRUID || GetClass() == SHAMAN || GetClass() == NECROMANCER || GetClass() == WIZARD || GetClass() == MAGICIAN || GetClass() == ENCHANTER); }
	bool IsMercCasterCombatRange(Mob *target);
	virtual float GetMaxMeleeRangeToTarget(Mob* target);
	virtual void MercMeditate(bool isSitting);

protected:
	void CalcItemBonuses(StatBonuses* newbon);
	void AddItemBonuses(const ItemInst *inst, StatBonuses* newbon, bool isAug = false, bool isTribute = false);
	int  CalcRecommendedLevelBonus(int8 level, uint8 reclevel, int basestat);

	int16   skills[HIGHEST_SKILL+1];
	int32   equipment[MAX_WORN_INVENTORY];	//this is an array of item IDs
	int16	d_meele_texture1;			//this is an item Material value
	int16	d_meele_texture2;			//this is an item Material value (offhand)
	int8	prim_melee_type;			//Sets the Primary Weapon attack message and animation
	int8	sec_melee_type;				//Sets the Secondary Weapon attack message and animation

private:

	sint16    CalcAC();
	sint16    GetACMit();
	sint16    GetACAvoid();
	sint16	  acmod();
	sint16    CalcATK();
	//int      CalcHaste();

	sint16   CalcSTR();
	sint16   CalcSTA();
	sint16   CalcDEX();
	sint16   CalcAGI();
	sint16   CalcINT();
	sint16   CalcWIS();
	sint16   CalcCHA();

    sint16	CalcMR();
	sint16	CalcFR();
	sint16	CalcDR();
	sint16	CalcPR();
	sint16	CalcCR();
	sint16	CalcCorrup();
	sint32	CalcMaxHP();
	sint32	CalcBaseHP();
	sint32	GetClassHPFactor();
	sint32	LevelRegen();
	sint32	CalcHPRegen();
	sint32	CalcHPRegenCap();
	sint32	CalcMaxMana();
	sint32	CalcBaseMana();
	sint32	CalcManaRegen();
	sint32	CalcBaseManaRegen();
	sint32	CalcManaRegenCap();
	void	CalcMaxEndurance();	//This calculates the maximum endurance we can have
	sint32	CalcBaseEndurance();	//Calculates Base End
	sint32	CalcEnduranceRegen();	//Calculates endurance regen used in DoEnduranceRegen()
	sint32	CalcEnduranceRegenCap();	
	int 	GroupLeadershipAAHealthEnhancement();
	int 	GroupLeadershipAAManaEnhancement();
	int		GroupLeadershipAAHealthRegeneration();
	int 	GroupLeadershipAAOffenseEnhancement();

	void GenerateBaseStats();
	void GenerateAppearance();

	// Private "base stats" Members
	int _baseAC;
	sint16 _baseSTR;
	sint16 _baseSTA;
	sint16 _baseDEX;
	sint16 _baseAGI;
	sint16 _baseINT;
	sint16 _baseWIS;
	sint16 _baseCHA;
	sint16 _baseATK;
	int16 _baseRace;	// Necessary to preserve the race otherwise mercs get their race updated in the db when they get an illusion.
	int8 _baseGender;	// Merc gender. Necessary to preserve the original value otherwise it can be changed by illusions.
	sint16 _baseMR;
	sint16 _baseCR;
	sint16 _baseDR;
	sint16 _baseFR;
	sint16 _basePR;
	sint16 _baseCorrup;

	uint32 _MercID;
	uint32 _MercTemplateID;
	uint32 _MercType;
	uint32 _MercSubType;
	uint8  _ProficiencyID;
	uint8 _CostFormula;
	uint8 _NameType;
	uint8 _OwnerClientVersion;

	Inventory m_inv;
	sint32	max_end;
	sint32	cur_end;
	bool	_medding;
	bool	_suspended;
	bool	p_depop;
	int32	owner_char_id;
	const NPCType*	ourNPCData;
};

#endif // MERC_H