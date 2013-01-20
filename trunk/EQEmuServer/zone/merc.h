#ifndef MERC_H
#define MERC_H
#include "mob.h"
#include "zonedb.h"
#include "npc.h"
using namespace std;

#define MERC_DEBUG 0
const int MercAISpellRange = 100; // TODO: Write a method that calcs what the merc's spell range is based on spell, equipment, AA, whatever and replace this

class Merc : public NPC {
public:
	Merc(const NPCType* d, float x, float y, float z, float heading);
	virtual ~Merc();

	//abstract virtual function implementations requird by base abstract class
	virtual void Death(Mob* killerMob, int32 damage, uint16 spell_id, SkillType attack_skill);
	virtual void Damage(Mob* from, int32 damage, uint16 spell_id, SkillType attack_skill, bool avoidable = true, int8 buffslot = -1, bool iBuffTic = false);
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
	void UpdateMercStats(Client *c);
	void UpdateMercAppearance(Client *c);
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

	static uint32 CalcPurchaseCost( uint32 templateID , uint8 level, uint8 currency_type = 0);
	static uint32 CalcUpkeepCost( uint32 templateID , uint8 level, uint8 currency_type = 0);

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

	virtual void SetTarget(Mob* mob);
	bool	HasSkill(SkillType skill_id) const;
	bool	CanHaveSkill(SkillType skill_id) const;
	uint16	MaxSkill(SkillType skillid, uint16 class_, uint16 level) const;
	inline	uint16	MaxSkill(SkillType skillid) const { return MaxSkill(skillid, GetClass(), GetLevel()); }

	// stat functions
	virtual void CalcBonuses();
	inline virtual int16	GetAC()		const { return AC; }
	inline virtual int16 GetATK() const { return ATK + itembonuses.ATK + spellbonuses.ATK + ((GetSTR() + GetSkill(OFFENSE)) * 9 / 10); }
	inline virtual int16 GetATKBonus() const { return itembonuses.ATK + spellbonuses.ATK; }
	int GetRawACNoShield(int &shield_ac) const;

	inline virtual int16	GetSTR()	const { return STR; }
	inline virtual int16	GetSTA()	const { return STA; }
	inline virtual int16	GetDEX()	const { return DEX; }
	inline virtual int16	GetAGI()	const { return AGI; }
	inline virtual int16	GetINT()	const { return INT; }
	inline virtual int16	GetWIS()	const { return WIS; }
	inline virtual int16	GetCHA()	const { return CHA; }
	inline virtual int16	GetMR()	const { return MR; }
	inline virtual int16	GetFR()	const { return FR; }
	inline virtual int16	GetDR()	const { return DR; }
	inline virtual int16	GetPR()	const { return PR; }
	inline virtual int16	GetCR()	const { return CR; }
	inline virtual int16	GetCorrup()	const { return Corrup; }

	inline virtual int16	GetHeroicSTR()	const { return itembonuses.HeroicSTR; }
	inline virtual int16	GetHeroicSTA()	const { return itembonuses.HeroicSTA; }
	inline virtual int16	GetHeroicDEX()	const { return itembonuses.HeroicDEX; }
	inline virtual int16	GetHeroicAGI()	const { return itembonuses.HeroicAGI; }
	inline virtual int16	GetHeroicINT()	const { return itembonuses.HeroicINT; }
	inline virtual int16	GetHeroicWIS()	const { return itembonuses.HeroicWIS; }
	inline virtual int16	GetHeroicCHA()	const { return itembonuses.HeroicCHA; }
	inline virtual int16	GetHeroicMR()	const { return itembonuses.HeroicMR; }
	inline virtual int16	GetHeroicFR()	const { return itembonuses.HeroicFR; }
	inline virtual int16	GetHeroicDR()	const { return itembonuses.HeroicDR; }
	inline virtual int16	GetHeroicPR()	const { return itembonuses.HeroicPR; }
	inline virtual int16	GetHeroicCR()	const { return itembonuses.HeroicCR; }
	inline virtual int16	GetHeroicCorrup()	const { return itembonuses.HeroicCorrup; }
	// Mod2
	inline virtual int16	GetShielding()		const { return itembonuses.MeleeMitigation; }
	inline virtual int16	GetSpellShield()	const { return itembonuses.SpellShield; }
	inline virtual int16	GetDoTShield()		const { return itembonuses.DoTShielding; }
	inline virtual int16	GetStunResist()		const { return itembonuses.StunResist; }
	inline virtual int16	GetStrikeThrough()	const { return itembonuses.StrikeThrough; }
	inline virtual int16	GetAvoidance()		const { return itembonuses.AvoidMeleeChance; }
	inline virtual int16	GetAccuracy()		const { return itembonuses.HitChance; }
	inline virtual int16	GetCombatEffects()	const { return itembonuses.ProcChance; }
	inline virtual int16	GetDS()				const { return itembonuses.DamageShield; }
	// Mod3
	inline virtual int16	GetHealAmt()		const { return itembonuses.HealAmt; }
	inline virtual int16	GetSpellDmg()		const { return itembonuses.SpellDmg; }
	inline virtual int16	GetClair()			const { return itembonuses.Clairvoyance; }
	inline virtual int16	GetDSMit()			const { return itembonuses.DSMitigation; }
	
	inline virtual int16	GetSingMod()		const { return itembonuses.singingMod; }
	inline virtual int16	GetBrassMod()		const { return itembonuses.brassMod; }
	inline virtual int16	GetPercMod()		const { return itembonuses.percussionMod; }
	inline virtual int16	GetStringMod()		const { return itembonuses.stringedMod; }
	inline virtual int16	GetWindMod()		const { return itembonuses.windMod; }
	
	inline virtual int16	GetDelayDeath()		const { return aabonuses.DelayDeath + spellbonuses.DelayDeath + itembonuses.DelayDeath + 11; }

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
	bool FindTarget();

protected:
	void CalcItemBonuses(StatBonuses* newbon);
	void AddItemBonuses(const ItemInst *inst, StatBonuses* newbon, bool isAug = false, bool isTribute = false);
	int  CalcRecommendedLevelBonus(uint8 level, uint8 reclevel, int basestat);

	uint16   skills[HIGHEST_SKILL+1];
	uint32   equipment[MAX_WORN_INVENTORY];	//this is an array of item IDs
	uint16	d_meele_texture1;			//this is an item Material value
	uint16	d_meele_texture2;			//this is an item Material value (offhand)
	uint8	prim_melee_type;			//Sets the Primary Weapon attack message and animation
	uint8	sec_melee_type;				//Sets the Secondary Weapon attack message and animation

private:

	int16    CalcAC();
	int16    GetACMit();
	int16    GetACAvoid();
	int16	  acmod();
	int16    CalcATK();
	//int      CalcHaste();

	int16   CalcSTR();
	int16   CalcSTA();
	int16   CalcDEX();
	int16   CalcAGI();
	int16   CalcINT();
	int16   CalcWIS();
	int16   CalcCHA();

    int16	CalcMR();
	int16	CalcFR();
	int16	CalcDR();
	int16	CalcPR();
	int16	CalcCR();
	int16	CalcCorrup();
	int32	CalcMaxHP();
	int32	CalcBaseHP();
	int32	GetClassHPFactor();
	int32	LevelRegen();
	int32	CalcHPRegen();
	int32	CalcHPRegenCap();
	int32	CalcMaxMana();
	int32	CalcBaseMana();
	int32	CalcManaRegen();
	int32	CalcBaseManaRegen();
	int32	CalcManaRegenCap();
	void	CalcMaxEndurance();	//This calculates the maximum endurance we can have
	int32	CalcBaseEndurance();	//Calculates Base End
	int32	GetEndurance()	const {return cur_end;}	//This gets our current endurance
	int32	GetMaxEndurance() const {return max_end;}	//This gets our endurance from the last CalcMaxEndurance() call
	int32	CalcEnduranceRegen();	//Calculates endurance regen used in DoEnduranceRegen()
	int32	CalcEnduranceRegenCap();
	void	SetEndurance(int32 newEnd);	//This sets the current endurance to the new value
	void	DoEnduranceUpkeep();	//does the endurance upkeep

	int 	GroupLeadershipAAHealthEnhancement();
	int 	GroupLeadershipAAManaEnhancement();
	int		GroupLeadershipAAHealthRegeneration();
	int 	GroupLeadershipAAOffenseEnhancement();

	void GenerateBaseStats();
	void GenerateAppearance();

	// Private "base stats" Members
	int32 base_mana;
	int _baseAC;
	int16 _baseSTR;
	int16 _baseSTA;
	int16 _baseDEX;
	int16 _baseAGI;
	int16 _baseINT;
	int16 _baseWIS;
	int16 _baseCHA;
	int16 _baseATK;
	uint16 _baseRace;	// Necessary to preserve the race otherwise mercs get their race updated in the db when they get an illusion.
	uint8 _baseGender;	// Merc gender. Necessary to preserve the original value otherwise it can be changed by illusions.
	int16 _baseMR;
	int16 _baseCR;
	int16 _baseDR;
	int16 _baseFR;
	int16 _basePR;
	int16 _baseCorrup;

	uint32 _MercID;
	uint32 _MercTemplateID;
	uint32 _MercType;
	uint32 _MercSubType;
	uint8  _ProficiencyID;
	uint8 _CostFormula;
	uint8 _NameType;
	uint8 _OwnerClientVersion;

	Inventory m_inv;
	int32	max_end;
	int32	cur_end;
	bool	_medding;
	bool	_suspended;
	bool	p_depop;
	uint32	owner_char_id;
	const NPCType*	ourNPCData;

	Timer	endupkeep_timer;
};

#endif // MERC_H