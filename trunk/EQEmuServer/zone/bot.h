#ifndef BOT_H
#define BOT_H

#ifdef BOTS

#include "botStructs.h"
#include "mob.h"
#include "client.h"
#include "pets.h"
#include "groups.h"
#include "PlayerCorpse.h"
#include "zonedb.h"
#include "StringIDs.h"
#include "../common/MiscFunctions.h"
#include "../common/debug.h"
#include "guild_mgr.h"
#include "worldserver.h"

#include <sstream>

using namespace std;

extern bool spells_loaded;
extern WorldServer worldserver;

const int BotAISpellRange = 100; // TODO: Write a method that calcs what the bot's spell range is based on spell, equipment, AA, whatever and replace this
const int MaxSpellTimer = 15;
const int MaxDisciplineTimer = 10;
const int DisciplineReuseStart = MaxSpellTimer + 1;
const int MaxTimer = MaxSpellTimer + MaxDisciplineTimer;
const int MaxStances = 7;
const int MaxSpellTypes = 16;
const int MaxHealRotationMembers = 6;
const int MaxHealRotationTargets = 3;

typedef enum BotStanceType {
	BotStancePassive,
	BotStanceBalanced,
	BotStanceEfficient,
	BotStanceReactive,
	BotStanceAggressive,
	BotStanceBurn,
	BotStanceBurnAE
};

typedef enum SpellTypeIndex {
	SpellType_NukeIndex,
	SpellType_HealIndex,
	SpellType_RootIndex,
	SpellType_BuffIndex,
	SpellType_EscapeIndex,
	SpellType_PetIndex,
	SpellType_LifetapIndex,
	SpellType_SnareIndex,
	SpellType_DOTIndex,
	SpellType_DispelIndex,
	SpellType_InCombatBuffIndex,
	SpellType_MezIndex,
	SpellType_CharmIndex,
	SpellType_SlowIndex,
	SpellType_DebuffIndex,
	SpellType_CureIndex
};

class Bot : public NPC {
public:
	// Class enums
	typedef enum BotfocusType {	//focus types
		BotfocusSpellHaste = 1,
		BotfocusSpellDuration,
		BotfocusRange,
		BotfocusReagentCost,
		BotfocusManaCost,
		BotfocusImprovedHeal,
		BotfocusImprovedDamage,
		BotfocusImprovedDOT,		//i dont know about this...
		BotfocusImprovedDamage2,
		BotfocusImprovedUndeadDamage,
		BotfocusPetPower,
		BotfocusResistRate,
		BotfocusSpellHateMod,
		BotfocusTriggerOnCast,
		BotfocusSpellVulnerability,
		BotfocusTwincast,
		BotfocusSympatheticProc,
		BotfocusSpellDamage,
		BotfocusFF_Damage_Amount,
		BotfocusSpellDurByTic,
		BotfocusSwarmPetDuration,
		BotfocusReduceRecastTime,
		BotfocusBlockNextSpell,
		BotfocusHealRate,
		BotfocusAdditionalDamage,
		BotfocusSpellEffectiveness,
		BotfocusIncreaseNumHits,
		BotfocusCriticalHealRate,
		BotfocusAdditionalHeal2,
		BotfocusAdditionalHeal,
	};

	typedef enum BotTradeType {	// types of trades a bot can do
		BotTradeClientNormal,
		BotTradeClientNoDropNoTrade
	};

	typedef enum BotRoleType {
		BotRoleMainAssist,
		BotRoleGroupHealer,
		BotRoleRaidHealer
	};

	typedef enum EqExpansions {
		ExpansionNone,
		ExpansionEQ,
		ExpansionRoK,
		ExpansionSoV,
		ExpansionSoL,
		ExpansionPoP,
		ExpansionLoY,
		ExpansionLDoN,
		ExpansionGoD,
		ExpansionOoW,
		ExpansionDoN,
		ExpansionDoDH,
		ExpansionPoR,
		ExpansionTSS,
		ExpansionSoF,
		ExpansionSoD,
		ExpansionUF,
		ExpansionHoT,
		ExpansionVoA,
		ExpansionRoF
	};

	// Class Constructors
	Bot(NPCType npcTypeData, Client* botOwner);
	Bot(uint32 botID, uint32 botOwnerCharacterID, uint32 botSpellsID, double totalPlayTime, int32 lastZoneId, NPCType npcTypeData);

	//abstract virtual function implementations requird by base abstract class
	virtual void Death(Mob* killerMob, sint32 damage, int16 spell_id, SkillType attack_skill);
	virtual void Damage(Mob* from, sint32 damage, int16 spell_id, SkillType attack_skill, bool avoidable = true, sint8 buffslot = -1, bool iBuffTic = false);
	virtual bool Attack(Mob* other, int Hand = 13, bool FromRiposte = false, bool IsStrikethrough = false, bool IsFromSpell = false);
	virtual bool HasRaid() { return (GetRaid() ? true : false);  }
	virtual bool HasGroup() { return (GetGroup() ? true : false); }
	virtual Raid* GetRaid() { return entity_list.GetRaidByMob(this); }
	virtual Group* GetGroup() { return entity_list.GetGroupByMob(this); }
	
	// Common, but informal "interfaces" with Client object
	int32 CharacterID() { return GetBotID(); } // Just returns the Bot Id
	inline bool IsInAGuild() const { return (_guildId != GUILD_NONE && _guildId != 0); }
	inline bool IsInGuild(uint32 in_gid) const { return (in_gid == _guildId && IsInAGuild()); }
	inline int32 GuildID() const { return _guildId; }
	inline int8	GuildRank()	const { return _guildRank; }

	// Class Methods
	bool IsValidRaceClassCombo();
	bool IsValidName();
	bool IsBotNameAvailable(std::string* errorMessage);
	bool DeleteBot(std::string* errorMessage);
	void Spawn(Client* botCharacterOwner, std::string* errorMessage);
	virtual void SetLevel(uint8 in_level, bool command = false);
	virtual void FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho);
	virtual bool Process();
	void FinishTrade(Client* client, BotTradeType tradeType);
	virtual bool Save();
	virtual void Depop();
	void CalcBotStats(bool showtext = true);
	int16 BotGetSpells(int spellslot) { return AIspells[spellslot].spellid; }
	int16 BotGetSpellType(int spellslot) { return AIspells[spellslot].type; }
    int16 BotGetSpellPriority(int spellslot) { return AIspells[spellslot].priority; }
	virtual float GetProcChances(float &ProcBonus, float &ProcChance, int16 weapon_speed, int16 hand);
	virtual bool AvoidDamage(Mob* other, sint32 &damage, bool CanRiposte);
	virtual int GetMonkHandToHandDamage(void);
	virtual void TryCriticalHit(Mob *defender, int16 skill, sint32 &damage);
	virtual bool TryFinishingBlow(Mob *defender, SkillType skillinuse);
	virtual void DoRiposte(Mob* defender);
	inline virtual sint16 GetATK() const { return ATK + itembonuses.ATK + spellbonuses.ATK + ((GetSTR() + GetSkill(OFFENSE)) * 9 / 10); }
	inline virtual sint16 GetATKBonus() const { return itembonuses.ATK + spellbonuses.ATK; }
	uint16 GetTotalATK();
	uint16 GetATKRating();
	uint16 GetPrimarySkillValue();
	int16	MaxSkill(SkillType skillid, int16 class_, int16 level) const;
	inline	int16	MaxSkill(SkillType skillid) const { return MaxSkill(skillid, GetClass(), GetLevel()); }
	virtual void MeleeMitigation(Mob *attacker, sint32 &damage, sint32 minhit);
	virtual void DoSpecialAttackDamage(Mob *who, SkillType skill, sint32 max_damage, sint32 min_damage = 1, sint32 hate_override = -1, int ReuseTime = 10, bool HitChance=false);
	virtual void TryBackstab(Mob *other,int ReuseTime = 10);
	virtual void RogueBackstab(Mob* other, bool min_damage = false, int ReuseTime = 10);
	virtual void RogueAssassinate(Mob* other);
	virtual void DoClassAttacks(Mob *target, bool IsRiposte=false);
	virtual bool TryHeadShot(Mob* defender, SkillType skillInUse);
	virtual void DoMeleeSkillAttackDmg(Mob* other, int16 weapon_damage, SkillType skillinuse, sint16 chance_mod=0, sint16 focus=0, bool CanRiposte=false);
	virtual void ApplySpecialAttackMod(SkillType skill, sint32 &dmg, sint32 &mindmg);
	bool CanDoSpecialAttack(Mob *other);
	virtual sint32 CheckAggroAmount(int16 spellid);
	virtual void CalcBonuses();
	void CalcItemBonuses();
	virtual void MakePet(int16 spell_id, const char* pettype, const char *petname = NULL);
	virtual FACTION_VALUE GetReverseFactionCon(Mob* iOther);
	inline virtual bool IsPet() { return false; }
	virtual bool IsNPC() const { return false; }
	virtual Mob* GetOwner();
	virtual Mob* GetOwnerOrSelf();
	inline virtual bool HasOwner() { return (GetBotOwner() ? true : false); }
	virtual sint32 CheckHealAggroAmount(int16 spellid, int32 heal_possible = 0);
	virtual sint32 CalcMaxMana();
	virtual void SetAttackTimer();
	int32 GetClassHPFactor();
	virtual sint32 CalcMaxHP();
	bool DoFinishedSpellAETarget(int16 spell_id, Mob* spellTarget, int16 slot, bool &stopLogic);
	bool DoFinishedSpellSingleTarget(int16 spell_id, Mob* spellTarget, int16 slot, bool &stopLogic);
	bool DoFinishedSpellGroupTarget(int16 spell_id, Mob* spellTarget, int16 slot, bool &stopLogic);
	void SendBotArcheryWearChange(int8 material_slot, uint32 material, uint32 color);
	void Camp(bool databaseSave = true);
	virtual void AddToHateList(Mob* other, sint32 hate = 0, sint32 damage = 0, bool iYellForHelp = true, bool bFrenzy = false, bool iBuffTic = false);
	virtual void SetTarget(Mob* mob);
	virtual void Zone();
	std::vector<AISpells_Struct> GetBotSpells() { return AIspells; }
	bool IsArcheryRange(Mob* target);
	void ChangeBotArcherWeapons(bool isArcher);
	void Sit();
	void Stand();
	bool IsSitting();
	bool IsStanding();
	bool IsBotCasterCombatRange(Mob *target);
	bool CalculateNewPosition2(float x, float y, float z, float speed, bool checkZ = true) ;
	bool UseDiscipline(int32 spell_id, int32 target);
	int8 GetNumberNeedingHealedInGroup(int8 hpr, bool includePets);
	bool GetNeedsCured(Mob *tar);
	bool HasOrMayGetAggro();
	void SetDefaultBotStance();
	void CalcChanceToCast();
	void CreateHealRotation( Mob* target, int32 timer = 10000 );
	bool AddHealRotationMember( Bot* healer );
	bool RemoveHealRotationMember( Bot* healer );
	bool AddHealRotationTarget( Mob* target );
	//bool AddHealRotationTarget( const char *targetName, int index);
	bool AddHealRotationTarget( Mob* target, int index);
	bool RemoveHealRotationTarget( Mob* target );
	bool RemoveHealRotationTarget( int index);
	void NotifyNextHealRotationMember( bool notifyNow = false );
	void ClearHealRotationLeader() { _healRotationLeader = NULL; }
	void ClearHealRotationMembers();
	void ClearHealRotationTargets();
	inline virtual sint16  GetMaxStat();
	inline virtual sint16  GetMaxResist();
	inline virtual sint16  GetMaxSTR();
	inline virtual sint16  GetMaxSTA();
	inline virtual sint16  GetMaxDEX();
	inline virtual sint16  GetMaxAGI();
	inline virtual sint16  GetMaxINT();
	inline virtual sint16  GetMaxWIS();
	inline virtual sint16  GetMaxCHA();
	inline virtual sint16  GetMaxMR();
	inline virtual sint16  GetMaxPR();
	inline virtual sint16  GetMaxDR();
	inline virtual sint16  GetMaxCR();
	inline virtual sint16  GetMaxFR();
	inline virtual sint16  GetMaxCorrup();
	sint16  CalcATK();
	sint16  CalcSTR();
	sint16  CalcSTA();
	sint16  CalcDEX();
	sint16  CalcAGI();
	sint16  CalcINT();
	sint16  CalcWIS();
	sint16  CalcCHA();
	sint16	CalcMR();
	sint16	CalcFR();
	sint16	CalcDR();
	sint16	CalcPR();
	sint16	CalcCR();
	sint16	CalcCorrup();
	sint32  CalcHPRegenCap();
	sint32 	CalcManaRegenCap();
	sint32	LevelRegen();
	sint32	CalcHPRegen();
	sint32	CalcManaRegen();
	uint32	CalcCurrentWeight();
	int 	GroupLeadershipAAHealthEnhancement();
	int 	GroupLeadershipAAManaEnhancement();
	int	GroupLeadershipAAHealthRegeneration();
	int 	GroupLeadershipAAOffenseEnhancement();
	void CalcRestState();
	sint32	CalcMaxEndurance();	//This calculates the maximum endurance we can have
	sint32	CalcBaseEndurance();	//Calculates Base End
	sint32	CalcEnduranceRegen();	//Calculates endurance regen used in DoEnduranceRegen()
	sint32	GetEndurance()	const {return cur_end;}	//This gets our current endurance
	sint32	GetMaxEndurance() const {return max_end;}	//This gets our endurance from the last CalcMaxEndurance() call
	sint32	CalcEnduranceRegenCap();	
	inline uint8 GetEndurancePercent() { return (uint8)((float)cur_end / (float)max_end * 100.0f); }
	void SetEndurance(sint32 newEnd);	//This sets the current endurance to the new value
	void DoEnduranceRegen();	//This Regenerates endurance
	void DoEnduranceUpkeep();	//does the endurance upkeep

	// AI Methods
	virtual bool AICastSpell(Mob* tar, int8 iChance, int16 iSpellTypes);
	virtual bool AI_EngagedCastCheck();
	virtual bool AI_PursueCastCheck();
	virtual bool AI_IdleCastCheck();
	bool AIHealRotation(Mob* tar, bool useFastHeals);

	// Mob AI Virtual Override Methods
	virtual void AI_Process();
	virtual void AI_Stop();

	// Mob Spell Virtual Override Methods
	virtual void SpellProcess();
	sint32 Additional_SpellDmg(int16 spell_id, bool bufftick = false);
	sint32 Additional_Heal(int16 spell_id);
	virtual sint32 GetActSpellDamage(int16 spell_id, sint32 value);
	virtual sint32 GetActSpellHealing(int16 spell_id, sint32 value);
	virtual sint32 GetActSpellCasttime(int16 spell_id, sint32 casttime);
	virtual sint32 GetActSpellCost(int16 spell_id, sint32 cost);
	virtual float GetActSpellRange(int16 spell_id, float range);
	virtual sint32 GetActSpellDuration(int16 spell_id, sint32 duration);
	virtual float GetAOERange(uint16 spell_id);
	virtual bool SpellEffect(Mob* caster, int16 spell_id, float partial = 100);
	virtual void DoBuffTic(int16 spell_id, int32 ticsremaining, int8 caster_level, Mob* caster = 0);
	virtual bool CastSpell(int16 spell_id, int16 target_id, int16 slot = 10, sint32 casttime = -1, sint32 mana_cost = -1, int32* oSpellWillFinish = 0, int32 item_slot = 0xFFFFFFFF, sint16 *resist_adjust = NULL);
	virtual bool SpellOnTarget(int16 spell_id, Mob* spelltar);
	virtual bool IsImmuneToSpell(int16 spell_id, Mob *caster);
	virtual bool DetermineSpellTargets(uint16 spell_id, Mob *&spell_target, Mob *&ae_center, CastAction_type &CastAction);
	virtual bool DoCastSpell(int16 spell_id, int16 target_id, int16 slot = 10, sint32 casttime = -1, sint32 mana_cost = -1, int32* oSpellWillFinish = 0, int32 item_slot = 0xFFFFFFFF);

	// Bot Action Command Methods
	bool MesmerizeTarget(Mob* target);
	bool Bot_Command_Resist(int resisttype, int level);
	bool Bot_Command_DireTarget(int diretype, Mob *target);
	bool Bot_Command_CharmTarget(int charmtype, Mob *target);
	bool Bot_Command_CalmTarget(Mob *target);
	bool Bot_Command_RezzTarget(Mob *target);
	bool Bot_Command_Cure(int curetype, int level);

	// Bot Equipment & Inventory Class Methods
	void BotTradeSwapItem(Client* client, sint16 lootSlot, const ItemInst* inst, const ItemInst* inst_swap, uint32 equipableSlots, std::string* errorMessage, bool swap = true);
	void BotTradeAddItem(uint32 id, const ItemInst* inst, sint16 charges, uint32 equipableSlots, int16 lootSlot, std::string* errorMessage, bool addToDb = true);
	void EquipBot(std::string* errorMessage);
	bool CheckLoreConflict(const Item_Struct* item);
	uint32 GetEquipmentColor(int8 material_slot) const;

	// Static Class Methods
	static void SaveBotGroup(Group* botGroup, std::string botGroupName, std::string* errorMessage);
	static void DeleteBotGroup(std::string botGroupName, std::string* errorMessage);
	static std::list<BotGroup> LoadBotGroup(std::string botGroupName, std::string* errorMessage);
	static uint32 CanLoadBotGroup(uint32 botOwnerCharacterId, std::string botGroupName, std::string* errorMessage);
	static uint32 GetBotGroupIdByBotGroupName(std::string botGroupName, std::string* errorMessage);
	static uint32 GetBotGroupLeaderIdByBotGroupName(std::string botGroupName);
	static std::list<BotGroupList> GetBotGroupListByBotOwnerCharacterId(uint32 botOwnerCharacterId, std::string* errorMessage);
	static bool DoesBotGroupNameExist(std::string botGroupName);
	//static void DestroyBotRaidObjects(Client* client);	// Can be removed after bot raids are dumped
	static uint32 GetBotIDByBotName(std::string botName);
	static Bot* LoadBot(uint32 botID, std::string* errorMessage);
	static std::list<BotsAvailableList> GetBotList(uint32 botOwnerCharacterID, std::string* errorMessage);
	static void ProcessBotCommands(Client *c, const Seperator *sep);
	static std::list<SpawnedBotsList> ListSpawnedBots(uint32 characterID, std::string* errorMessage);
	static uint32 SpawnedBotCount(uint32 botOwnerCharacterID, std::string* errorMessage);
	static uint32 CreatedBotCount(uint32 botOwnerCharacterID, std::string* errorMessage);
	static uint32 AllowedBotSpawns(uint32 botOwnerCharacterID, std::string* errorMessage);
	static uint32 GetBotOwnerCharacterID(uint32 botID, std::string* errorMessage);
	//static bool SetBotOwnerCharacterID(uint32 botID, uint32 botOwnerCharacterID, std::string* errorMessage);
	static std::string ClassIdToString(uint16 classId);
	static std::string RaceIdToString(uint16 raceId);
	static bool IsBotAttackAllowed(Mob* attacker, Mob* target, bool& hasRuleDefined);
	static void BotGroupOrderFollow(Group* group, Client* client);
	static void BotGroupOrderGuard(Group* group, Client* client);
	static void BotGroupOrderAttack(Group* group, Mob* target, Client* client);
	static void BotGroupSummon(Group* group, Client* client);
	static Bot* GetBotByBotClientOwnerAndBotName(Client* c, std::string botName);
	static void ProcessBotGroupInvite(Client* c, std::string botName);
	static void ProcessBotGroupDisband(Client* c, std::string botName);
	static void BotOrderCampAll(Client* c);
	static void BotHealRotationsClear( Client* c );
	static void ProcessBotInspectionRequest(Bot* inspectedBot, Client* client);
	static std::list<uint32> GetGroupedBotsByGroupId(uint32 groupId, std::string* errorMessage);
	static void LoadAndSpawnAllZonedBots(Client* botOwner);
	static bool GroupHasBot(Group* group);
	static Bot* GetFirstBotInGroup(Group* group);
	static void ProcessClientZoneChange(Client* botOwner);
	static void ProcessBotOwnerRefDelete(Mob* botOwner);	// Removes a Client* reference when the Client object is destroyed
	static void ProcessGuildInvite(Client* guildOfficer, Bot* botToGuild);	// Processes a client's request to guild a bot
	static bool ProcessGuildRemoval(Client* guildOfficer, std::string botName);	// Processes a client's request to deguild a bot
	static sint32 GetSpellRecastTimer(Bot *caster, int timer_index);
	static bool CheckSpellRecastTimers(Bot *caster, int SpellIndex);
	static sint32 GetDisciplineRecastTimer(Bot *caster, int timer_index);
	static bool CheckDisciplineRecastTimers(Bot *caster, int timer_index);
	static int32 GetDisciplineRemainingTime(Bot *caster, int timer_index);
	static std::list<BotSpell> GetBotSpellsForSpellEffect(Bot* botCaster, int spellEffect);
	static std::list<BotSpell> GetBotSpellsForSpellEffectAndTargetType(Bot* botCaster, int spellEffect, SpellTargetType targetType);
	static std::list<BotSpell> GetBotSpellsBySpellType(Bot* botCaster, int16 spellType);
	static BotSpell GetFirstBotSpellBySpellType(Bot* botCaster, int16 spellType);
	static BotSpell GetBestBotSpellForFastHeal(Bot* botCaster);
	static BotSpell GetBestBotSpellForHealOverTime(Bot* botCaster);
	static BotSpell GetBestBotSpellForPercentageHeal(Bot* botCaster);
	static BotSpell GetBestBotSpellForRegularSingleTargetHeal(Bot* botCaster);
	static BotSpell GetFirstBotSpellForSingleTargetHeal(Bot* botCaster);
	static BotSpell GetBestBotSpellForGroupHealOverTime(Bot* botCaster);
	static BotSpell GetBestBotSpellForGroupCompleteHeal(Bot* botCaster);
	static BotSpell GetBestBotSpellForGroupHeal(Bot* botCaster);
	static BotSpell GetBestBotSpellForMagicBasedSlow(Bot* botCaster);
	static BotSpell GetBestBotSpellForDiseaseBasedSlow(Bot* botCaster);
	static Mob* GetFirstIncomingMobToMez(Bot* botCaster, BotSpell botSpell);
	static BotSpell GetBestBotSpellForMez(Bot* botCaster);
	static BotSpell GetBestBotMagicianPetSpell(Bot* botCaster);
	static std::string GetBotMagicianPetType(Bot* botCaster);
	static BotSpell GetBestBotSpellForNukeByTargetType(Bot* botCaster, SpellTargetType targetType);
	static BotSpell GetBestBotSpellForStunByTargetType(Bot* botCaster, SpellTargetType targetType);
	static BotSpell GetBestBotWizardNukeSpellByTargetResists(Bot* botCaster, Mob* target);
	static BotSpell GetDebuffBotSpell(Bot* botCaster, Mob* target);
	static BotSpell GetBestBotSpellForCure(Bot* botCaster, Mob* target);
	static BotSpell GetBestBotSpellForResistDebuff(Bot* botCaster, Mob* target);
	static NPCType CreateDefaultNPCTypeStructForBot(std::string botName, std::string botLastName, uint8 botLevel, uint16 botRace, uint8 botClass, uint8 gender);
	static std::list<Bot*> GetBotsInHealRotation( Bot* leader );

	// Static Bot Group Methods
	static bool AddBotToGroup(Bot* bot, Group* group);
	static bool RemoveBotFromGroup(Bot* bot, Group* group);
	static bool BotGroupCreate(std::string botGroupLeaderName);
	static bool BotGroupCreate(Bot* botGroupLeader);
	static bool	GroupHasClass(Group* group, uint8 classId);
	static bool GroupHasClericClass(Group* group) { return GroupHasClass(group, CLERIC); }
	static bool GroupHasDruidClass(Group* group) { return GroupHasClass(group, DRUID); }
	static bool GroupHasShamanClass(Group* group) { return GroupHasClass(group, SHAMAN); }
	static bool GroupHasEnchanterClass(Group* group) { return GroupHasClass(group, ENCHANTER); }
	static bool GroupHasPriestClass(Group* group) { return GroupHasClass(group, CLERIC | DRUID | SHAMAN); }
	static void BotGroupSay(Mob *speaker, const char *msg, ...);

	// "GET" Class Methods
	uint32 GetBotID() const { return _botID; }
	uint32 GetBotOwnerCharacterID() { return _botOwnerCharacterID; }
	uint32 GetBotSpellID() { return npc_spells_id; }
	Mob* GetBotOwner() { return this->_botOwner; }
	uint32 GetBotArcheryRange();
	ItemInst* GetBotItem(uint32 slotID);
	virtual bool GetSpawnStatus() { return _spawnStatus; }
	int8 GetPetChooserID() { return _petChooserID; }
	bool IsPetChooser() { return _petChooser; }
	bool IsBotArcher() { return _botArcher; }
	bool IsBotCharmer() { return _botCharmer; }
	virtual bool IsBot() const { return true; }
	bool GetRangerAutoWeaponSelect() { return _rangerAutoWeaponSelect; }
	BotRoleType GetBotRole() { return _botRole; }
	BotStanceType GetBotStance() { return _botStance; }
	int8 GetChanceToCastBySpellType(int16 spellType);
	bool IsGroupPrimaryHealer();
	bool IsGroupPrimarySlower();
	bool IsBotCaster() { return (GetClass() == CLERIC || GetClass() == DRUID || GetClass() == SHAMAN || GetClass() == NECROMANCER || GetClass() == WIZARD || GetClass() == MAGICIAN || GetClass() == ENCHANTER); }
	bool IsBotINTCaster() { return (GetClass() == NECROMANCER || GetClass() == WIZARD || GetClass() == MAGICIAN || GetClass() == ENCHANTER); }
	bool IsBotWISCaster() { return (GetClass() == CLERIC || GetClass() == DRUID || GetClass() == SHAMAN); }
	bool CanHeal();
	int GetRawACNoShield(int &shield_ac);
	void LoadAAs();
	int32 GetAA(int32 aa_id);
	void ApplyAABonuses(uint32 aaid, uint32 slots, StatBonuses* newbon);
	bool GetHasBeenSummoned() { return _hasBeenSummoned; }
	float GetPreSummonX() { return _preSummonX; }
	float GetPreSummonY() { return _preSummonY; }
	float GetPreSummonZ() { return _preSummonZ; }
	bool GetGroupMessagesOn() { return _groupMessagesOn; }
	bool GetInHealRotation() { return _isInHealRotation; }
	bool GetHealRotationActive() { return (GetInHealRotation() && _isHealRotationActive); }
	bool GetHealRotationUseFastHeals() { return _healRotationUseFastHeals; }
	bool GetHasHealedThisCycle() { return _hasHealedThisCycle; }
	Mob* GetHealRotationTarget();
	Mob* GetHealRotationTarget(int8 index);
	Bot* GetHealRotationLeader();
	Bot* GetNextHealRotationMember();
	Bot* GetPrevHealRotationMember();
	int8 GetNumHealRotationMembers () { return _numHealRotationMembers; }
	int32 GetHealRotationNextHealTime() { return _healRotationNextHeal; }
	int32 GetHealRotationTimer () { return _healRotationTimer; }
	inline virtual sint16	GetAC()	const { return AC; }
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
	//Heroic
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
	
	inline virtual sint16	GetDelayDeath()		const { return aabonuses.DelayDeath + spellbonuses.DelayDeath + itembonuses.DelayDeath; }

	inline InspectMessage_Struct& GetInspectMessage()			  { return _botInspectMessage; }
	inline const InspectMessage_Struct& GetInspectMessage() const { return _botInspectMessage; }

	// "SET" Class Methods
	void SetBotSpellID(uint32 newSpellID);
	virtual void SetSpawnStatus(bool spawnStatus) { _spawnStatus = spawnStatus; }
	void SetPetChooserID(int8 id) { _petChooserID = id; }
	void SetBotArcher(bool a) { _botArcher = a; }
	void SetBotCharmer(bool c) { _botCharmer = c; }
	void SetPetChooser(bool p) { _petChooser = p; }
	void SetBotOwner(Mob* botOwner) { this->_botOwner = botOwner; }
	// void SetBotOwnerCharacterID(uint32 botOwnerCharacterID) { _botOwnerCharacterID = botOwnerCharacterID; }
	void SetRangerAutoWeaponSelect(bool enable) { GetClass() == RANGER ? _rangerAutoWeaponSelect = enable : _rangerAutoWeaponSelect = false; }
	void SetBotRole(BotRoleType botRole) { _botRole = botRole; }
	void SetBotStance(BotStanceType botStance) { _botStance = botStance; }
	void SetSpellRecastTimer(int timer_index, sint32 recast_delay);
	void SetDisciplineRecastTimer(int timer_index, sint32 recast_delay);
	void SetHasBeenSummoned(bool s);
	void SetPreSummonX(float x) { _preSummonX = x; }
	void SetPreSummonY(float y) { _preSummonY = y; }
	void SetPreSummonZ(float z) { _preSummonZ = z; }
	void SetGroupMessagesOn(bool groupMessagesOn) { _groupMessagesOn = groupMessagesOn; }
	void SetInHealRotation( bool inRotation ) { _isInHealRotation = inRotation; }
	void SetHealRotationActive( bool isActive ) { _isHealRotationActive = isActive; }
	void SetHealRotationUseFastHeals( bool useFastHeals ) { _healRotationUseFastHeals = useFastHeals; }
	void SetHasHealedThisCycle( bool hasHealed ) { _hasHealedThisCycle = hasHealed; }
	void SetHealRotationLeader( Bot* leader );
	void SetNextHealRotationMember( Bot* healer );
	void SetPrevHealRotationMember( Bot* healer );
	void SetHealRotationNextHealTime( int32 nextHealTime ) { _healRotationNextHeal = nextHealTime; }
	void SetHealRotationTimer( int32 timer ) { _healRotationTimer = timer; }
	void SetNumHealRotationMembers( int8 numMembers ) { _numHealRotationMembers = numMembers; }

	// Class Destructors
	virtual ~Bot();

protected:
	virtual void PetAIProcess();
	static NPCType FillNPCTypeStruct(uint32 botSpellsID, std::string botName, std::string botLastName, uint8 botLevel, uint16 botRace, uint8 botClass, uint8 gender, float size, uint32 face, uint32 hairStyle, uint32 hairColor, uint32 eyeColor, uint32 eyeColor2, uint32 beardColor, uint32 beard, uint32 drakkinHeritage, uint32 drakkinTattoo, uint32 drakkinDetails, sint32 hp, sint32 mana, sint16 mr, sint16 cr, sint16 dr, sint16 fr, sint16 pr, sint16 corrup, sint16 ac, uint16 str, uint16 sta, uint16 dex, uint16 agi, uint16 _int, uint16 wis, uint16 cha, uint16 attack);
	virtual void BotMeditate(bool isSitting);
	virtual void BotRangedAttack(Mob* other);
	virtual bool CheckBotDoubleAttack(bool Triple = false);
	virtual sint16 GetBotFocusEffect(BotfocusType bottype, int16 spell_id);
	virtual sint16 CalcBotFocusEffect(BotfocusType bottype, int16 focus_id, int16 spell_id, bool best_focus=false);
	virtual sint16 CalcBotAAFocus(BotfocusType type, uint32 aa_ID, int16 spell_id);
	virtual void PerformTradeWithClient(sint16 beginSlotID, sint16 endSlotID, Client* client);
	virtual bool AIDoSpellCast(int8 i, Mob* tar, sint32 mana_cost, int32* oDontDoAgainBefore = 0);
	virtual float GetMaxMeleeRangeToTarget(Mob* target);

	static void SetBotGuildMembership(int32 botId, int32 guildid, int8 rank);

private:
	// Class Members
	uint32 _botID;
	uint32 _botOwnerCharacterID;
	//uint32 _botSpellID;
	bool _spawnStatus;
	Mob* _botOwner;
	bool _botOrderAttack;
	bool _botArcher;
	bool _botCharmer;
	bool _petChooser;
	int8 _petChooserID;
	bool berserk;
	Inventory m_inv;
	double _lastTotalPlayTime;
	time_t _startTotalPlayTime;
	Mob* _previousTarget;
	uint32 _guildId;
	int8 _guildRank;
	std::string _guildName;
	int32 _lastZoneId;
	bool _rangerAutoWeaponSelect;
	BotRoleType _botRole;
	BotStanceType _botStance;
	BotStanceType _baseBotStance;
	unsigned int RestRegenHP;
	unsigned int RestRegenMana;
	unsigned int RestRegenEndurance;
	Timer rest_timer;
	sint32	base_end;
	sint32	cur_end;
	sint32	max_end;
	sint16	end_regen;
	int32 timers[MaxTimer];
	bool _hasBeenSummoned;
	float _preSummonX;
	float _preSummonY;
	float _preSummonZ;
	int8 _spellCastingChances[MaxStances][MaxSpellTypes];
	bool _groupMessagesOn;
	bool _isInHealRotation;
	bool _isHealRotationActive;
	bool _healRotationUseFastHeals;
	bool _hasHealedThisCycle;
	int32 _healRotationTimer;
	int32 _healRotationNextHeal;
	//char _healRotationTargets[MaxHealRotationTargets][64];
	int16 _healRotationTargets[MaxHealRotationTargets];
	uint32 _healRotationLeader;
	uint32 _healRotationMemberNext;
	uint32 _healRotationMemberPrev;
	int8 _numHealRotationMembers;
	std::map<uint32, BotAA> botAAs;
	InspectMessage_Struct _botInspectMessage;

	// Private "base stats" Members
	sint16 _baseMR;
	sint16 _baseCR;
	sint16 _baseDR;
	sint16 _baseFR;
	sint16 _basePR;
	sint16 _baseCorrup;
	int _baseAC;
	sint16 _baseSTR;
	sint16 _baseSTA;
	sint16 _baseDEX;
	sint16 _baseAGI;
	sint16 _baseINT;
	sint16 _baseWIS;
	sint16 _baseCHA;
	sint16 _baseATK;
	int16 _baseRace;	// Necessary to preserve the race otherwise bots get their race updated in the db when they get an illusion.
	int8 _baseGender;	// Bots gender. Necessary to preserve the original value otherwise it can be changed by illusions.

	// Class Methods
	sint16 acmod();
	void GenerateBaseStats();
	void GenerateAppearance();
	void GenerateArmorClass();
	sint32 GenerateBaseHitPoints();
	void GenerateAABonuses(StatBonuses* newbon);
	sint32 GenerateBaseManaPoints();
	void GenerateSpecialAttacks();
	void SetBotID(uint32 botID);

	// Private "Inventory" Methods
	void GetBotItems(std::string* errorMessage, Inventory &inv);
	void BotRemoveEquipItem(int slot);
	void BotAddEquipItem(int slot, uint32 id);
	uint32 GetBotItemBySlot(uint32 slotID);
	void RemoveBotItemBySlot(uint32 slotID, std::string* errorMessage);
	void SetBotItemInSlot(uint32 slotID, uint32 itemID, const ItemInst* inst, std::string* errorMessage);
	uint32 GetBotItemsCount(std::string* errorMessage);
	uint32 GetTotalPlayTime();
	void SaveBuffs();	// Saves existing buffs to the database to persist zoning and camping
	void LoadBuffs();	// Retrieves saved buffs from the database on spawning
	void LoadPetBuffs(SpellBuff_Struct* petBuffs, uint32 botPetSaveId);
	void SavePetBuffs(SpellBuff_Struct* petBuffs, uint32 botPetSaveId);
	void LoadPetItems(int32* petItems, uint32 botPetSaveId);
	void SavePetItems(int32* petItems, uint32 botPetSaveId);
	void LoadPetStats(std::string* petName, int16* petMana, int16* petHitPoints, uint32* botPetId, uint32 botPetSaveId);
	uint32 SavePetStats(std::string petName, int16 petMana, int16 petHitPoints, uint32 botPetId);
	void LoadPet();	// Load and spawn bot pet if there is one
	void SavePet();	// Save and depop bot pet if there is one
	uint32 GetPetSaveId();
	void DeletePetBuffs(uint32 botPetSaveId);
	void DeletePetItems(uint32 botPetSaveId);
	void DeletePetStats(uint32 botPetSaveId);
	void LoadGuildMembership(int32* guildId, int8* guildRank, std::string* guildName);
	void LoadStance();
	void SaveStance();
	void LoadTimers();
	void SaveTimers();
};

#endif // BOTS

#endif // BOT_H
