#include "types.h"

class Cache {
public:
	Cache();
	~Cache();

	const Item_Struct*	GetItem(uint32 iID);
	const NPCType*		GetNPCType(uint32 iID);
	uint32				GetZoneID(const char* zonename);
	const char*			GetZoneName(uint32 zoneID, bool ErrorUnknown = false);
private:
	uint32				max_zonename;
	char**				zonename_array;
	uint32				max_item;
	Item_Struct**		item_array;
	uint32				max_npc_type;
	NPCType**			npc_type_array;
};

