#ifndef _EQE_QUESTINTERFACE_H
#define _EQE_QUESTINTERFACE_H

#include <stdint.h>
#include "event_codes.h"

class ItemInst;
class Client;
class NPC;
enum QuestStatus;

class QuestInterface {
public:
    virtual void EventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32_t extra_data);
    virtual void EventPlayer(QuestEventID evt, Client *client, std::string zone, std::string data, uint32_t extra_data);
    virtual void EventItem(QuestEventID evt, Client *client, ItemInst *item, uint32_t extra_data);
    virtual void EventSpell(QuestEventID evt, NPC* npc, Client *client, std::string data, uint32_t extra_data);

    virtual QuestStatus LoadScript(std::string zone, uint32_t npc_id) = 0;
    virtual QuestStatus LoadPlayerScript(std::string zone) = 0;
    virtual QuestStatus LoadItemScript(std::string item_script) = 0;
    virtual QuestStatus LoadSpellScript(uint32_t) = 0;
    virtual QuestStatus LoadDefaultScript(std::string zone) = 0;
    virtual QuestStatus LoadDefaultPlayerScript() = 0;
    virtual QuestStatus LoadDefaultItemScript() = 0;
    virtual QuestStatus LoadDefaultSpellScript() = 0;

    virtual bool HaveScriptFunction(const char *zone, uint32_t npc_id, const char* sub) = 0;
    virtual bool HavePlayerScriptFunction(const char *zone, const char* sub) = 0;
    virtual bool HaveItemScriptFunction(QuestEventID evt, ItemInst *inst, const char* sub) = 0;
    virtual bool HaveSpellScriptFunction(uint32_t, const char* sub) = 0;

    virtual void AddVar(std::string name, std::string val) = 0;
    virtual void Reload(bool reset_timers = true) = 0;
};

#endif