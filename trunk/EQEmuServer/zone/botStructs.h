#ifndef BOT_STRUCTS
#define BOT_STRUCTS

#ifdef BOTS

#include "../common/types.h"

struct BotsAvailableList {
	uint32 BotID;
	char BotName[64];
	uint8 BotLevel;
	uint16 BotRace;
};

#endif // BOTS

#endif // BOT_STRUCTS