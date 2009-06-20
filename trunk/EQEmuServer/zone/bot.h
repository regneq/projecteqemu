#ifndef BOT_H
#define BOT_H

#ifdef BOTS

#include "botStructs.h"
#include "mob.h"
#include "client.h"
#include "zonedb.h"
#include "../common/MiscFunctions.h"

class Bot : public Mob {
public:
	// Class Constructors
	Bot(uint32 botID, uint32 botOwnerCharacterID, uint32 botInventoryID, uint32 botSpellsID, std::string botName, std::string botLastName, uint8 botLevel, uint16 botRace, uint8 botClass, uint8 botBodyType, sint32 hitPoints, uint8 gender, float size, uint32 hitPointsRegenRate, uint32 manaRegenRate, uint32 face, uint32 hairStyle, uint32 hairColor, uint32 eyeColor, uint32 eyeColor2, uint32 beardColor, uint32 beard, uint32 drakkinHeritage, uint32 drakkinTattoo, uint32 drakkinDetails, float runSpeed, sint16 mr, sint16 cr, sint16 dr, sint16 fr, sint16 pr, sint16 ac, uint16 str, uint16 sta, uint16 dex, uint16 agi, uint16 _int, uint16 wis, uint16 cha, uint16 attack);
	Bot(std::string botName, uint8 botClass, uint16 botRace, uint8 botGender, Client* botOwner);

	// Class Methods
	bool IsValidRaceClassCombo();
	bool IsValidName();
	bool IsBotNameAvailable(std::string* errorMessage);
	uint32 CreateNewBotRecord(std::string* errorMessage);
	std::string SaveBot();
	std::string DeleteBot();
	void Spawn(float xPos, float yPos, float zPos, float heading);
	void SetBotOwnerCharacterID(uint32 botOwnerCharacterID, std::string* errorMessage);
	void Depop(std::string* errorMessage);

	// Bot Inventory Class Methods
	uint32 GetBotItemBySlot(uint32 slotID, std::string* errorMessage);
	std::list<BotInventory> GetBotItems(std::string* errorMessage);
	void RemoveBotItemBySlot(uint32 slotID, std::string* errorMessage);
	void SetBotItemInSlot(uint32 slotID, uint32 itemID, std::string* errorMessage);
	uint32 GetBotItemsCount(std::string* errorMessage);

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
	static void CleanBotLeader(uint32 botOwnerCharacterID, std::string* errorMessage);
	static uint32 GetBotOwnerCharacterID(uint32 botID, std::string* errorMessage);

	// Inline "GET" Class Methods
	uint32 GetBotID() { return _botID; };
	uint32 GetBotOwnerCharacterID() { return _botOwnerCharacterID; };
	uint32 GetBotSpellID() { return _botSpellID; };
	uint32 GetInventoryID() { return _botInventoryID; };
	bool IsSpawned() { return _isSpawned; };

	// Inline "SET" Class Methods
	void SetBotSpellID(uint32 newSpellID) { _botSpellID = newSpellID; };

	// Class Deconstructors
	virtual ~Bot();

private:
	// Class Members
	uint32 _botOwnerCharacterID;
	uint32 _botID;
	uint32 _botSpellID;
	uint32 _botInventoryID;
	bool _isSpawned;
	// uint32 _expPoints;
	// uint32 _aaPoints;

	// Class Methods
	void GenerateBaseStats();
	void GenerateAppearance();
	void GenerateArmorClass();
	void GenerateBaseHitPoints();
	void SetBotLeader(uint32 botID, uint32 botOwnerCharacterID, std::string botName, std::string zoneShortName, std::string* errorMessage);
	uint32 GetBotLeader(uint32 botID, std::string* errorMessage);
	void CleanBotLeaderEntries(std::string* errorMessage);
};

#endif // BOTS

#endif // BOT_H