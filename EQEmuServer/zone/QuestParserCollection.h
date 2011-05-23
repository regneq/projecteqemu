#ifndef _EQE_QUESTPARSERCOLLECTION_H
#define _EQE_QUESTPARSERCOLLECTION_H

#include <stdint.h>
#include <string.h>
#include <string>
#include <list>
#include <map>

#include "masterentity.h"
#include "../common/Item.h"
#include "QuestInterface.h"

#define QuestFailedToLoad 0xFFFFFFFF
#define QuestUnloaded 0x00

class QuestParserCollection {
public:
    QuestParserCollection();
    ~QuestParserCollection();

    void RegisterQuestInterface(QuestInterface *qi, std::string ext);

    void AddVar(std::string name, std::string val);
    void ReloadQuests(bool reset_timers = true);

    bool HasQuestSub(uint32 npcid, const char *subname);
	bool PlayerHasQuestSub(const char *subname);
	bool SpellHasQuestSub(uint32 spell_id, const char *subname);
    bool ItemHasQuestSub(ItemInst *itm, const char *subname);

    void EventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32_t extra_data);
    void EventPlayer(QuestEventID evt, Client *client, std::string data, uint32_t extra_data);
    void EventItem(QuestEventID evt, Client *client, ItemInst *item, uint32_t objid, uint32_t extra_data);
    void EventSpell(QuestEventID evt, NPC* npc, Client *client, uint32_t spell_id, uint32_t extra_data);

private:
    QuestInterface *GetQIByNPCQuest(uint32_t npcid);
    QuestInterface *GetQIByPlayerQuest();
    QuestInterface *GetQIBySpellQuest(uint32_t spell_id);
    QuestInterface *GetQIByItemQuest(std::string item_script);

    std::map<uint32_t, QuestInterface*> _interfaces;
    std::map<uint32_t, std::string> _extensions;
    std::list<QuestInterface*> _load_precedence;

    //0x00 = Unloaded
    //0xFFFFFFFF = Failed to Load
    std::map<uint32_t, uint32_t> _npc_quest_status;
    uint32_t _player_quest_status;
    std::map<uint32_t, uint32_t> _spell_quest_status;
    std::map<std::string, uint32_t> _item_quest_status;
};

extern QuestParserCollection *parse;

#endif

