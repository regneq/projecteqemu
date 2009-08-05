#ifndef BOT_H
#define BOT_H

#ifdef BOTS

#include "botStructs.h"
#include "botRaids.h"
#include "mob.h"
#include "client.h"
#include "pets.h"
#include "groups.h"
#include "PlayerCorpse.h"
#include "zonedb.h"
#include "StringIDs.h"
#include "../common/MiscFunctions.h"

using namespace std;

extern bool spells_loaded;

class Bot : public NPC {
public:
	// Class Constructors
	Bot(NPCType npcTypeData, Client* botOwner);
	Bot(uint32 botID, uint32 botOwnerCharacterID, uint32 botSpellsID, NPCType npcTypeData);

	//abstract virtual function implementations requird by base abstract class
	virtual void Death(Mob* killerMob, sint32 damage, int16 spell_id, SkillType attack_skill);
	virtual void Damage(Mob* from, sint32 damage, int16 spell_id, SkillType attack_skill, bool avoidable = true, sint8 buffslot = -1, bool iBuffTic = false);
	virtual bool Attack(Mob* other, int Hand = 13, bool FromRiposte = false);

	// Class Methods
	bool IsValidRaceClassCombo();
	bool IsValidName();
	bool IsBotNameAvailable(std::string* errorMessage);
	bool DeleteBot(std::string* errorMessage);
	void Spawn(Client* botCharacterOwner, std::string* errorMessage);
	//void Depop(std::string* errorMessage);
	virtual void SetLevel(uint8 in_level, bool command = false);
	virtual void FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho);
	virtual bool Process();
	//virtual void AI_Process();
	virtual bool Save();
	virtual void Depop();
	void CalcBotStats(bool showtext = true);
	int16 BotGetSpells(int spellslot) { return AIspells[spellslot].spellid; }
	int16 BotGetSpellType(int spellslot) { return AIspells[spellslot].type; }
    int16 BotGetSpellPriority(int spellslot) { return AIspells[spellslot].priority; }
	virtual float CheckHitChance(Mob* attacker, SkillType skillinuse, int Hand);
	virtual float GetProcChances(float &ProcBonus, float &ProcChance, int16 weapon_speed);
	virtual bool AvoidDamage(Mob* other, sint32 &damage);
	virtual int GetMonkHandToHandDamage(void);
	virtual void TryCriticalHit(Mob *defender, int16 skill, sint32 &damage);
	virtual bool TryFinishingBlow(Mob *defender, SkillType skillinuse);
	virtual void DoRiposte(Mob* defender);
	inline virtual sint16 GetATK() const { return ATK + itembonuses.ATK + spellbonuses.ATK + ((GetSTR() + GetSkill(OFFENSE)) * 9 / 10); }
	virtual void MeleeMitigation(Mob *attacker, sint32 &damage, sint32 minhit);
	virtual void DoSpecialAttackDamage(Mob *who, SkillType skill, sint32 max_damage, sint32 min_damage = 1, sint32 hate_override = -1);
	virtual void TryBackstab(Mob *other);
	virtual void RogueBackstab(Mob* other, bool min_damage = false);
	virtual void RogueAssassinate(Mob* other);
	virtual void DoClassAttacks(Mob *target);
	virtual bool TryHeadShot(Mob* defender, SkillType skillInUse);
	virtual bool IsAttackAllowed(Mob *target, bool isSpellAttack = false);
	virtual sint32 CheckAggroAmount(int16 spellid);
	virtual void CalcBonuses();
	virtual void MakePet(int16 spell_id, const char* pettype, const char *petname = NULL);
	virtual void AI_Stop();
	virtual FACTION_VALUE GetReverseFactionCon(Mob* iOther);
	inline virtual bool IsPet() { return false; }
	virtual bool IsNPC() const { return false; }
	virtual Mob* GetOwner();
	virtual Mob* GetOwnerOrSelf();
	inline virtual bool HasOwner() { return (GetBotOwner() ? true : false); }
	virtual sint32 CheckHealAggroAmount(int16 spellid, int32 heal_possible = 0);
	virtual sint32 CalcMaxMana();
	virtual void SetAttackTimer();
	virtual sint32 CalcMaxHP();
	bool DoFinishedSpellAETarget(int16 spell_id, Mob* spellTarget, int16 slot, bool &stopLogic);
	bool DoFinishedSpellSingleTarget(int16 spell_id, Mob* spellTarget, int16 slot, bool &stopLogic);
	bool DoFinishedSpellGroupTarget(int16 spell_id, Mob* spellTarget, int16 slot, bool &stopLogic);
	void FinishTrade(Client* client);
	void SendBotArcheryWearChange(int8 material_slot, uint32 material, uint32 color);

	// Mob Spell Virtual Override Methods
	virtual sint32 GetActSpellDamage(int16 spell_id, sint32 value);
	virtual sint32 GetActSpellHealing(int16 spell_id, sint32 value);
	virtual sint32 GetActSpellCasttime(int16 spell_id, sint32 casttime);
	virtual sint32 GetActSpellCost(int16 spell_id, sint32 cost);
	virtual float GetActSpellRange(int16 spell_id, float range);
	virtual sint32 GetActSpellDuration(int16 spell_id, sint32 duration);
	virtual float GetAOERange(uint16 spell_id);
	virtual bool SpellEffect(Mob* caster, int16 spell_id, float partial = 100);
	virtual void DoBuffTic(int16 spell_id, int32 ticsremaining, int8 caster_level, Mob* caster = 0);
	virtual bool CastSpell(int16 spell_id, int16 target_id, int16 slot = 10, sint32 casttime = -1, sint32 mana_cost = -1, int32* oSpellWillFinish = 0, int32 item_slot = 0xFFFFFFFF);
	virtual bool SpellOnTarget(int16 spell_id, Mob* spelltar);
	virtual bool IsImmuneToSpell(int16 spell_id, Mob *caster);
	virtual bool DetermineSpellTargets(uint16 spell_id, Mob *&spell_target, Mob *&ae_center, CastAction_type &CastAction);
	virtual bool DoCastSpell(int16 spell_id, int16 target_id, int16 slot = 10, sint32 casttime = -1, sint32 mana_cost = -1, int32* oSpellWillFinish = 0, int32 item_slot = 0xFFFFFFFF);
	virtual int CheckStackConflict(int16 spellid1, int caster_level1, int16 spellid2, int caster_level2, Mob* caster1 = NULL, Mob* caster2 = NULL);

	// Bot Action Command Methods
	bool MesmerizeTarget(Mob* target);
	bool Bot_Command_Resist(int resisttype, int level);
	bool Bot_Command_DireTarget(int diretype, Mob *target);
	bool Bot_Command_CharmTarget(int charmtype, Mob *target);
	bool Bot_Command_CalmTarget(Mob *target);
	bool Bot_Command_RezzTarget(Mob *target);
	bool Bot_Command_Cure(int curetype, int level);

	// AI Methods
	virtual bool AI_EngagedCastCheck();
	virtual bool Bot_AI_PursueCastCheck();
	virtual bool Bot_AI_IdleCastCheck();
	virtual bool Bot_AICastSpell(Mob* tar, int8 iChance, int16 iSpellTypes);
	
	// Bot Orders
	//virtual bool IsBotOrderAttack() { return _botOrderAttack; }
	//virtual void SetBotOrderAttack(bool botAttack) { _botOrderAttack = botAttack; }

	// Bot Equipment & Inventory Class Methods
	void BotTradeSwapItem(Client* client, sint16 lootSlot, uint32 id, sint16 maxCharges, uint32 equipableSlots, std::string* errorMessage, bool swap = true);
	void BotTradeAddItem(uint32 id, sint16 maxCharges, uint32 equipableSlots, int16 lootSlot, std::string* errorMessage, bool addToDb = true);
	void EquipBot(std::string* errorMessage);

	// Static Class Methods
	static Bot* LoadBot(uint32 botID, std::string* errorMessage);
	static std::list<BotsAvailableList> GetBotList(uint32 botOwnerCharacterID, std::string* errorMessage);
	static void ProcessBotCommands(Client *c, const Seperator *sep);
	static std::list<SpawnedBotsList> ListSpawnedBots(uint32 characterID, std::string* errorMessage);
	static void SaveBotGroups(uint32 groupID, uint32 characterID, uint32 botID, uint16 slotID, std::string* errorMessage);
	static void DeleteBotGroups(uint32 characterID, std::string* errorMessage);
	static std::list<BotGroup> LoadBotGroups(uint32 characterID, std::string* errorMessage);
	static uint32 SpawnedBotCount(uint32 botOwnerCharacterID, std::string* errorMessage);
	static uint32 AllowedBotSpawns(uint32 botOwnerCharacterID, std::string* errorMessage);
	static uint32 GetBotOwnerCharacterID(uint32 botID, std::string* errorMessage);
	static bool SetBotOwnerCharacterID(uint32 botID, uint32 botOwnerCharacterID, std::string* errorMessage);
	static std::string ClassIdToString(uint16 classId);
	static std::string RaceIdToString(uint16 raceId);
	static uint32 GetCountBotsInGroup(Group* group);
	static bool AddBotToGroup(Bot* bot, Group* group);
	static void SendBotHPPacketsToGroup(Bot* bot, Group* group);
	static void DestroyBotObjects(Client* client);
	static bool IsBotAttackAllowed(Mob* attacker, Mob* target, bool& hasRuleDefined);

	// "GET" Class Methods
	uint32 GetBotID() { return _botID; }
	uint32 GetBotOwnerCharacterID() { return _botOwnerCharacterID; }
	uint32 GetBotSpellID() { return _botSpellID; }
	Mob* GetBotOwner() { return this->_botOwner; }
	uint32 GetBotArcheryRange() { return _botArcheryRange; }
	virtual bool GetSpawnStatus() { return _spawnStatus; }
	int8 GetPetChooserID() { return _petChooserID; }
	bool IsPetChooser() { return _petChooser; }
	bool IsBotArcher() { return _botArcher; }
	bool IsBotCharmer() { return _botCharmer; }
	virtual bool IsBot() const { return true; }

	// "SET" Class Methods
	void SetBotSpellID(uint32 newSpellID);
	virtual void SetSpawnStatus(bool spawnStatus) { _spawnStatus = spawnStatus; }
	void SetBotArcheryRange(uint32 archeryRange) { _botArcheryRange = archeryRange; }
	void SetPetChooserID(int8 id) { _petChooserID = id; }
	void SetBotArcher(bool a) { _botArcher = a; }
	void SetBotCharmer(bool c) { _botCharmer = c; }
	void SetPetChooser(bool p) { _petChooser = p; }
	void SetBotOwner(Mob* botOwner) { this->_botOwner = botOwner; }

	// Class Destructors
	virtual ~Bot();

	// Class enums
	enum botfocusType {	//focus types
		botfocusSpellHaste = 1,
		botfocusSpellDuration,
		botfocusRange,
		botfocusReagentCost,
		botfocusManaCost,
		botfocusImprovedHeal,
		botfocusImprovedDamage,
		botfocusImprovedDOT,		//i dont know about this...
		botfocusImprovedCritical,
		botfocusImprovedUndeadDamage,
		botfocusPetPower,
		botfocusResistRate,
		botfocusHateReduction,
	};

	typedef enum botfocusType botfocusType;

	typedef std::map<uint32, uint32> BotInventory;
	typedef std::pair<uint32, uint32> BotInventoryItem;

protected:
	virtual void BotAIProcess();
	virtual void PetAIProcess();
	static NPCType FillNPCTypeStruct(uint32 botSpellsID, std::string botName, std::string botLastName, uint8 botLevel, uint16 botRace, uint8 botClass, uint8 botBodyType, sint32 hitPoints, uint8 gender, float size, uint32 hitPointsRegenRate, uint32 manaRegenRate, uint32 face, uint32 hairStyle, uint32 hairColor, uint32 eyeColor, uint32 eyeColor2, uint32 beardColor, uint32 beard, uint32 drakkinHeritage, uint32 drakkinTattoo, uint32 drakkinDetails, float runSpeed, sint16 mr, sint16 cr, sint16 dr, sint16 fr, sint16 pr, sint16 ac, uint16 str, uint16 sta, uint16 dex, uint16 agi, uint16 _int, uint16 wis, uint16 cha, uint16 attack);
	static NPCType CreateDefaultNPCTypeStructForBot(std::string botName, std::string botLastName, uint8 botLevel, uint16 botRace, uint8 botClass, uint8 gender);
	virtual void BotMeditate(bool isSitting);
	virtual bool BotRangedAttack(Mob* other);
	virtual bool CheckBotDoubleAttack(bool Triple = false);
	virtual sint16 GetBotFocusEffect(botfocusType bottype, int16 spell_id);
	virtual sint16 CalcBotFocusEffect(botfocusType bottype, int16 focus_id, int16 spell_id);

private:
	// Class Members
	uint32 _botID;
	uint32 _botOwnerCharacterID;
	uint32 _botSpellID;
	bool _spawnStatus;
	Mob* _botOwner;
	bool _botOrderAttack;
	bool _botArcher;
	bool _botCharmer;
	bool _petChooser;
	int8 _petChooserID;
	uint32 _botArcheryRange;
	bool cast_last_time;
	BotInventory _botInventory;

	// Private "base stats" Members
	sint16 _baseMR;
	sint16 _baseCR;
	sint16 _baseDR;
	sint16 _baseFR;
	sint16 _basePR;
	int _baseAC;
	sint16 _baseSTR;
	sint16 _baseSTA;
	sint16 _baseDEX;
	sint16 _baseAGI;
	sint16 _baseINT;
	sint16 _baseWIS;
	sint16 _baseCHA;
	sint16 _baseATK;

	// Class Methods
	void GenerateBaseStats();
	void GenerateAppearance();
	void GenerateArmorClass();
	void GenerateBaseHitPoints();
	void GenerateAABonuses();
	void GenerateBaseManaPoints();
	void GenerateSpecialAttacks();
	void DoAIProcessing();
	void SetBotID(uint32 botID);
	uint32 GetItemID(int slot_id);
	bool CalcBotHitChance(Mob* target, SkillType skillinuse, int Hand);

	// Private "Inventory" Methods
	BotInventory GetBotItems(std::string* errorMessage);
	uint32 GetBotItem(uint32 slotID);
	void BotRemoveEquipItem(int slot);
	void BotAddEquipItem(int slot, uint32 id);
	uint32 GetBotItemBySlot(uint32 slotID, std::string* errorMessage);
	void RemoveBotItemBySlot(uint32 slotID, std::string* errorMessage);
	void SetBotItemInSlot(uint32 slotID, uint32 itemID, std::string* errorMessage);
	uint32 GetBotItemsCount(std::string* errorMessage);
};

#endif // BOTS

#endif // BOT_H