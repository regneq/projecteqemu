#ifndef _EQE_QUESTINTERFACE_H
#define _EQE_QUESTINTERFACE_H

#include <stdint.h>
#include "event_codes.h"

class ItemInst;
class Client;
class NPC;

class QuestInterface {
public:
    virtual void EventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32_t extra_data) { }
    virtual void EventPlayer(QuestEventID evt, Client *client, std::string data, uint32_t extra_data) { }
    virtual void EventItem(QuestEventID evt, Client *client, ItemInst *item, uint32_t objid, uint32_t extra_data) { }
    virtual void EventSpell(QuestEventID evt, NPC* npc, Client *client, uint32_t spell_id, uint32_t extra_data) { }

	virtual bool HasQuestSub(int32 npcid, const char *subname) { return false; }
	virtual bool PlayerHasQuestSub(const char *subname) { return false; }
	virtual bool SpellHasQuestSub(uint32 spell_id, const char *subname) { return false; }
    virtual bool ItemHasQuestSub(ItemInst *itm, const char *subname) { return false; }

    virtual void AddVar(std::string name, std::string val) { }
    virtual void ReloadQuests(bool reset_timers = true) { }
    virtual uint32_t GetIdentifier() { return 0; }
};

#endif

