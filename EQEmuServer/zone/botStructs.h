#ifndef BOT_STRUCTS
#define BOT_STRUCTS

#ifdef BOTS

#include "../common/types.h"

struct BotsAvailableList {
	uint32 BotID;
	char BotName[64];
	uint16 BotClass;
	uint8 BotLevel;
	uint16 BotRace;
};

struct BotGroup {
	uint32 GroupID;
	uint32 BotID;
	uint32 CharacterID;
};

struct SpawnedBotsList {
	char BotName[64];
	char ZoneName[64];
	uint32 BotLeaderCharID;
};

#endif // BOTS

#endif // BOT_STRUCTS
