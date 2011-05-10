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

    void RegisterQuestInterface(QuestInterface *qi);

    void EventSay(const char *message, uint32_t language, NPC * npc, Client * client, std::string zone);
    void EventItem(NPC *npc, Client *client, std::string zone);
    void EventDeath(NPC *npc, Mob *killer, std::string zone);
    void EventSpawn(NPC *npc, std::string zone);
    void EventAttack(NPC *npc, Mob *attacker, std::string zone);
    void EventCombat(uint32_t state, NPC *npc, Mob *attacker, std::string zone);
    void EventAggro(NPC *npc, Mob *other, std::string zone);
    void EventSlay(NPC *npc, Client *client, std::string zone);
    void EventNPCSlay(NPC *npc, NPC *attacker, std::string zone);
    void EventWaypointArrive(NPC *npc, std::string wp, std::string zone);
    void EventWaypointDepart(NPC *npc, std::string wp, std::string zone);
    void EventTimer(NPC *npc, std::string timer, std::string zone);
    void EventTimer(Client *client, std::string timer, std::string zone);
    void EventSignal(NPC *npc, std::string signal, std::string zone);
    void EventSignal(Client *client, std::string signal, std::string zone);
    void EventHP(NPC *npc, std::string hp, bool inc_hp_event, std::string zone);
    void EventEnter(NPC *npc, Client *client, std::string zone);
    void EventExit(NPC *npc, Client *client, std::string zone);
    void EventEnterZone(Client *client, std::string zone);
    void EventClickDoor(Client *client, std::string door_id, std::string zone);
    void EventLoot(Client *client, std::string looted, std::string zone);
    void EventZone(Client *client, std::string zone_id, std::string zone);
    void EventLevelUp(Client *client, std::string zone);
    void EventKilledMerit(NPC *npc, Client *client, std::string zone);
    void EventCastOn(NPC *npc, Client *client, std::string spell_id, std::string zone);
    void EventTaskAccepted(NPC *npc, Client *client, std::string task_id, std::string zone);
    void EventTaskStageComplete(Client *client, std::string task_info, std::string zone);
    void EventAggroSay(const char *message, uint32_t language, NPC * npc, Client * client, std::string zone);
    void EventPlayerPickup(Client *client, std::string item_id, std::string zone);
    void EventPopupResponse(Client *client, std::string popup_id, std::string zone);
    void EventProximitySay(const char *message, uint32_t language, NPC * npc, Client * client, std::string zone);
    void EventCast(Client *client, uint32_t spell_id);
    void EventScaleCalc(Client *client, ItemInst *item);
    void EventTargetChange(NPC *npc, Mob *other, std::string zone);
    void EventTargetChange(Client *client, std::string zone);
    void EventHateList(NPC *npc, Mob *other, bool join, std::string zone);
    void EventSpellEffectClient(Client *target, uint32_t spell_id, uint32_t caster_id);
    void EventSpellEffectNPC(NPC *target, uint32_t spell_id, uint32_t caster_id);
    void EventSpellEffectBuffTicClient(Client *target, uint32_t spell_id, uint32_t caster_id);
    void EventSpellEffectBuffTicNPC(NPC *target, uint32_t spell_id, uint32_t caster_id);
    void EventSpellEffectTranslocateComplete(Client *client, uint32_t spell_id);
    void EventCombineSuccess(Client *client, uint32_t recipe_id, std::string recipe_name, std::string zone);
    void EventCombineFailure(Client *client, uint32_t recipe_id, std::string recipe_name, std::string zone);
    void EventItemClick(Client *client, ItemInst *item, uint32_t slot_id);
    void EventItemClickCast(Client *client, ItemInst *item, uint32_t slot_id);
    void EventGroupChange(Client *client, std::string zone);

    void AddVar(std::string name, std::string val);
    void Reload(bool reset_timers = true);

    bool HaveScriptFunction(const char *zone, uint32_t npc_id, const char* sub);
    bool HavePlayerScriptFunction(const char *zone, const char* sub);
    bool HaveItemScriptFunction(QuestEventID evt, ItemInst *inst, const char* sub);
    bool HaveSpellScriptFunction(uint32_t spell_id, const char* sub);

private:
    //wrapper & helper functions
    void _EventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string zone, std::string data, uint32_t extra_data);
    void _EventPlayer(QuestEventID evt, Client *client, std::string zone, std::string data, uint32_t extra_data);
    void _EventItem(QuestEventID evt, Client *client, ItemInst *item, uint32_t extra_data);
    void _EventSpell(QuestEventID evt, NPC* npc, Client *client, uint32_t spell_id, uint32_t extra_data);

    QuestInterface *_LoadNPCQuest(std::string zone, uint32_t npc_id);
    QuestInterface *_LoadPlayerQuest(std::string zone);
    QuestInterface *_LoadItemQuest(std::string item_script);
    QuestInterface *_LoadSpellQuest(uint32_t spell_id);
                  
    std::map<uint32_t, QuestInterface*> _interfaces;
    std::list<QuestInterface*> _load_precedence;

    //0x00 = Unloaded
    //0xFFFFFFFF = Failed to Load
    std::map<uint32_t, uint32_t> _npc_quest_status;
    uint32_t _player_quest_status;
    std::map<std::string, uint32_t> _item_quest_status;
    std::map<uint32_t, uint32_t> _spell_quest_status;
};

#endif

