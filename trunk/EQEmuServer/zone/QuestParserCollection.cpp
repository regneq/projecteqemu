#include "QuestParserCollection.h"
#include "QuestInterface.h"

#include <stdlib.h>

QuestParserCollection::QuestParserCollection() {
    _player_quest_status = Unloaded;
}

QuestParserCollection::~QuestParserCollection() {
}

void QuestParserCollection::RegisterQuestInterface(QuestInterface *qi, std::string name) {
    _interfaces[name] = qi;
    _load_precedence.push_back(qi);
}

void QuestParserCollection::UnregisterQuestInterface(std::string name) {
    std::map<std::string, QuestInterface*>::iterator iter = _interfaces.find(name);
    if(iter != _interfaces.end()) {
        QuestInterface *qi = (*iter).second;
        _interfaces.erase(iter);
        std::list<QuestInterface*>::iterator i = _load_precedence.begin();
        while(i != _load_precedence.end()) {
            if((*i) == qi) {
                _load_precedence.erase(i);
                return;
            }
            i++;
        }
    }
}

void QuestParserCollection::AddVar(std::string name, std::string val) {
    std::list<QuestInterface*>::iterator iter = _load_precedence.begin();
    while(iter != _load_precedence.end()) {
        (*iter)->AddVar(name, val);
        iter++;
    }
}

void QuestParserCollection::Reload(bool reset_timers) {
    _player_quest_status = Unloaded;
    _npc_quest_status.clear();
    _item_quest_status.clear();
    _spell_quest_status.clear();

    std::list<QuestInterface*>::iterator iter = _load_precedence.begin();
    while(iter != _load_precedence.end()) {
        (*iter)->Reload(reset_timers);
        iter++;
    }
}

void QuestParserCollection::_EventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string zone, std::string data, 
    uint32_t extra_data) {
        uint32_t npc_id = npc->GetNPCTypeID();
        std::map<uint32_t, QuestStatus>::iterator iter = _npc_quest_status.find(npc_id);
        if(iter != _npc_quest_status.end()) {
            switch((*iter).second) {
            case LoadedByPerl: {
                _interfaces["Perl"]->EventNPC(evt, npc, init, data, extra_data);
                break;
            }

            case LoadedByQst: {
                _interfaces["Qst"]->EventNPC(evt, npc, init, data, extra_data);
                break;
            }
            }
        } else {
            switch(LoadNPCQuest(zone, npc_id)) {
            case LoadedByPerl: {
                _interfaces["Perl"]->EventNPC(evt, npc, init, data, extra_data);
                break;
            }

            case LoadedByQst: {
                _interfaces["Qst"]->EventNPC(evt, npc, init, data, extra_data);
                break;
            }
            }
        }
}

void QuestParserCollection::_EventPlayer(QuestEventID evt, Client *client, std::string zone, std::string data, 
    uint32_t extra_data) {
    switch(_player_quest_status) {
    case LoadedByPerl: {
        _interfaces["Perl"]->EventPlayer(evt, client, zone, data, extra_data);
        break;
    }

    case LoadedByQst: {
        _interfaces["Qst"]->EventPlayer(evt, client, zone, data, extra_data);
        break;
    }
    case Unloaded: {
        switch(LoadPlayerQuest(zone.c_str())) {
            case LoadedByPerl: {
                _interfaces["Perl"]->EventPlayer(evt, client, zone, data, extra_data);
                break;
            }

            case LoadedByQst: {
                _interfaces["Qst"]->EventPlayer(evt, client, zone, data, extra_data);
                break;
            }
        }
    }
    }

}
void QuestParserCollection::_EventItem(QuestEventID evt, Client *client, ItemInst *item, 
    uint32_t extra_data) {

    std::string name = "item_";
    switch(evt) {
    case EVENT_SCALE_CALC:
        name += item->GetItem()->CharmFile;
        break;
    case EVENT_ITEM_CLICK:
    case EVENT_ITEM_CLICK_CAST:
        name += itoa(item->GetItem()->ScriptFileID);
        break;
    }

    std::map<std::string, QuestStatus>::iterator iter = _item_quest_status.find(name);
    if(iter != _item_quest_status.end()) {
        switch((*iter).second) {
        case LoadedByPerl: {
            _interfaces["Perl"]->EventItem(evt, client, item, extra_data);
            break;
        }

        case LoadedByQst: {
            _interfaces["Qst"]->EventItem(evt, client, item, extra_data);
            break;
        }
        }
    } else {
        switch(LoadItemQuest(name)) {
        case LoadedByPerl: {
            _interfaces["Perl"]->EventItem(evt, client, item, extra_data);
            break;
        }

        case LoadedByQst: {
            _interfaces["Qst"]->EventItem(evt, client, item, extra_data);
            break;
        }
        }
    }
}
void QuestParserCollection::_EventSpell(QuestEventID evt, NPC* npc, Client *client, std::string data, 
    uint32_t extra_data) 
{
    uint32_t spell_id = atoi(data.c_str());
    std::map<uint32_t, QuestStatus>::iterator iter = _spell_quest_status.find(spell_id);
    if(iter != _spell_quest_status.end()) {
        switch((*iter).second) {
        case LoadedByPerl: {
            _interfaces["Perl"]->EventSpell(evt, npc, client, data, extra_data);
            break;
        }

        case LoadedByQst: {
            _interfaces["Qst"]->EventSpell(evt, npc, client, data, extra_data);
            break;
        }
        }
    } else {
        switch(LoadSpellQuest(spell_id)) {
        case LoadedByPerl: {
            _interfaces["Perl"]->EventSpell(evt, npc, client, data, extra_data);
            break;
        }

        case LoadedByQst: {
            _interfaces["Qst"]->EventSpell(evt, npc, client, data, extra_data);
            break;
        }
        }
    }
}

//try to load reg quests in precedence order
//if that fails load default in precedence order
QuestStatus QuestParserCollection::LoadNPCQuest(std::string zone, uint32_t npc_id) {
    //FailedToLoad
    std::list<QuestInterface*>::iterator i = _load_precedence.begin();
    while(i != _load_precedence.end())
    {
        QuestStatus res = (*i)->LoadScript(zone, npc_id);
        if(res != FailedToLoad) {
            _npc_quest_status[npc_id] = res;
            return res;
        }
        ++i;
    }

    i = _load_precedence.begin();
    while(i != _load_precedence.end())
    {
        QuestStatus res = (*i)->LoadDefaultScript(zone);
        if(res != FailedToLoad) {
            _npc_quest_status[npc_id] = res;
            return res;
        }
        ++i;
    }
    _npc_quest_status[npc_id] = FailedToLoad;
    return FailedToLoad;
}

//try to load reg quests in precedence order
//if that fails load default in precedence order
QuestStatus QuestParserCollection::LoadPlayerQuest(std::string zone) {
    std::list<QuestInterface*>::iterator i = _load_precedence.begin();
    while(i != _load_precedence.end())
    {
        QuestStatus res = (*i)->LoadPlayerScript(zone);
        if(res != FailedToLoad) {
            _player_quest_status = res;
            return res;
        }
        ++i;
    }

    i = _load_precedence.begin();
    while(i != _load_precedence.end())
    {
        QuestStatus res = (*i)->LoadDefaultPlayerScript();
        if(res != FailedToLoad) {
            _player_quest_status = res;
            return res;
        }
        ++i;
    }
    _player_quest_status = FailedToLoad;
    return FailedToLoad;
}

//try to load reg quests in precedence order
//if that fails load default in precedence order
QuestStatus QuestParserCollection::LoadItemQuest(std::string item_script) {
    std::list<QuestInterface*>::iterator i = _load_precedence.begin();
    while(i != _load_precedence.end())
    {
        QuestStatus res = (*i)->LoadItemScript(item_script);
        if(res != FailedToLoad) {
            _item_quest_status[item_script] = res;
            return res;
        }
        ++i;
    }

    i = _load_precedence.begin();
    while(i != _load_precedence.end())
    {
        QuestStatus res = (*i)->LoadDefaultItemScript();
        if(res != FailedToLoad) {
            _item_quest_status[item_script] = res;
            return res;
        }
        ++i;
    }
    _item_quest_status[item_script] = FailedToLoad;
    return FailedToLoad;
}

//try to load reg quests in precedence order
//if that fails load default in precedence order
QuestStatus QuestParserCollection::LoadSpellQuest(uint32_t spell_id) {
    std::list<QuestInterface*>::iterator i = _load_precedence.begin();
    while(i != _load_precedence.end())
    {
        QuestStatus res = (*i)->LoadSpellScript(spell_id);
        if(res != FailedToLoad) {
            _spell_quest_status[spell_id] = res;
            return res;
        }
        ++i;
    }

    i = _load_precedence.begin();
    while(i != _load_precedence.end())
    {
        QuestStatus res = (*i)->LoadDefaultSpellScript();
        if(res != FailedToLoad) {
            _spell_quest_status[spell_id] = res;
            return res;
        }
        ++i;
    }
    _spell_quest_status[spell_id] = FailedToLoad;
    return FailedToLoad;
}

bool QuestParserCollection::HaveScriptFunction(const char *zone, uint32_t npc_id, const char* sub) {
    std::map<uint32_t, QuestStatus>::iterator iter = _npc_quest_status.find(npc_id);
    if(iter != _npc_quest_status.end()) {
        switch((*iter).second) {
        case LoadedByPerl:
            return _interfaces["Perl"]->HaveScriptFunction(zone, npc_id, sub);
            break;
        case LoadedByQst:
            return _interfaces["Qst"]->HaveScriptFunction(zone, npc_id, sub);
            break;
        }
    }
    return false;
}

bool QuestParserCollection::HavePlayerScriptFunction(const char *zone, const char* sub) {
    switch(_player_quest_status) {
        case LoadedByPerl:
            return _interfaces["Perl"]->HavePlayerScriptFunction(zone, sub);
            break;
        case LoadedByQst:
            return _interfaces["Qst"]->HavePlayerScriptFunction(zone, sub);
            break;
    }
    return false;
}

bool QuestParserCollection::HaveItemScriptFunction(QuestEventID evt, ItemInst *inst, const char* sub) {

    std::string name = "item_";
    if(evt == EVENT_SCALE_CALC) {
        name += inst->GetItem()->CharmFile;
    } else {
        name += itoa(inst->GetItem()->ScriptFileID);
    }

    std::map<std::string, QuestStatus>::iterator iter = _item_quest_status.find(name);
    if(iter != _item_quest_status.end()) {
        switch((*iter).second) {
        case LoadedByPerl:
            return _interfaces["Perl"]->HaveItemScriptFunction(evt, inst, sub);
            break;
        case LoadedByQst:
            return _interfaces["Qst"]->HaveItemScriptFunction(evt, inst, sub);
            break;
        }
    }
    return false;
}

bool QuestParserCollection::HaveSpellScriptFunction(uint32_t spell_id, const char* sub) {
    std::map<uint32_t, QuestStatus>::iterator iter = _spell_quest_status.find(spell_id);
    if(iter != _spell_quest_status.end()) {
        switch((*iter).second) {
        case LoadedByPerl:
            return _interfaces["Perl"]->HaveSpellScriptFunction(spell_id, sub);
            break;
        case LoadedByQst:
            return _interfaces["Qst"]->HaveSpellScriptFunction(spell_id, sub);
            break;
        }
    }
    return false;
}


void QuestParserCollection::EventSay(const char *message, uint32_t language, NPC * npc, Client * client, std::string zone) {
    _EventNPC(EVENT_SAY, npc, client, zone, message, 0);
}

void QuestParserCollection::EventItem(NPC *npc, Client *client, std::string zone) {
    _EventNPC(EVENT_ITEM, npc, client, zone, "", 0);
}

void QuestParserCollection::EventDeath(NPC *npc, Mob *killer, std::string zone) {
    _EventNPC(EVENT_DEATH, npc, killer, zone, "", 0);
}

void QuestParserCollection::EventSpawn(NPC *npc, std::string zone) {
    _EventNPC(EVENT_SPAWN, npc, NULL, zone, "", 0);
}

void QuestParserCollection::EventAttack(NPC *npc, Mob *attacker, std::string zone) {
    _EventNPC(EVENT_ATTACK, npc, attacker, zone, "", 0);
}

void QuestParserCollection::EventCombat(uint32_t state, NPC *npc, Mob *attacker, std::string zone) {
    _EventNPC(EVENT_ATTACK, npc, attacker, zone, state == 1 ? "1" : "0", 0);
}

void QuestParserCollection::EventAggro(NPC *npc, Mob *other, std::string zone) {
    _EventNPC(EVENT_AGGRO, npc, other, zone, "", 0);
}

void QuestParserCollection::EventSlay(NPC *npc, Client *client, std::string zone) {
    _EventNPC(EVENT_SLAY, npc, client, zone, "", 0);
}

void QuestParserCollection::EventNPCSlay(NPC *npc, NPC *attacker, std::string zone) {
    _EventNPC(EVENT_NPC_SLAY, npc, attacker, zone, "", 0);
}

void QuestParserCollection::EventWaypointArrive(NPC *npc, std::string wp, std::string zone) {
    _EventNPC(EVENT_WAYPOINT_ARRIVE, npc, NULL, zone, wp, 0);
}

void QuestParserCollection::EventWaypointDepart(NPC *npc, std::string wp, std::string zone) {
    _EventNPC(EVENT_WAYPOINT_DEPART, npc, NULL, zone, wp, 0);
}

void QuestParserCollection::EventTimer(NPC *npc, std::string timer, std::string zone) {
    _EventNPC(EVENT_TIMER, npc, NULL, zone, timer, 0);
}

void QuestParserCollection::EventTimer(Client *client, std::string timer, std::string zone) {
    _EventPlayer(EVENT_TIMER, client, zone, timer, 0);
}

void QuestParserCollection::EventSignal(NPC *npc, std::string signal, std::string zone) {
    _EventNPC(EVENT_SIGNAL, npc, NULL, zone, signal, 0);
}

void QuestParserCollection::EventSignal(Client *client, std::string signal, std::string zone) {
    _EventPlayer(EVENT_SIGNAL, client, zone, signal, 0);
}

void QuestParserCollection::EventHP(NPC *npc, std::string hp, bool inc_hp_event, std::string zone) {
    _EventNPC(EVENT_HP, npc, NULL, zone, hp, inc_hp_event ? 1 : 0);
}

void QuestParserCollection::EventEnter(NPC *npc, Client *client, std::string zone) {
    _EventNPC(EVENT_ENTER, npc, client, zone, "", 0);
}

void QuestParserCollection::EventExit(NPC *npc, Client *client, std::string zone) {
    _EventNPC(EVENT_EXIT, npc, client, zone, "", 0);
}

void QuestParserCollection::EventEnterZone(Client *client, std::string zone) {
     _EventPlayer(EVENT_ENTERZONE, client, zone, "", 0);
}

void QuestParserCollection::EventClickDoor(Client *client, std::string door_id, std::string zone) {
    _EventPlayer(EVENT_CLICKDOOR, client, zone, door_id, 0);
}

void QuestParserCollection::EventLoot(Client *client, std::string looted, std::string zone) {
    _EventPlayer(EVENT_LOOT, client, zone, looted, 0);
}

void QuestParserCollection::EventZone(Client *client, std::string zone_id, std::string zone) {
    _EventPlayer(EVENT_ZONE, client, zone, zone_id, 0);
}

void QuestParserCollection::EventLevelUp(Client *client, std::string zone) {
    _EventPlayer(EVENT_LEVEL_UP, client, zone, "", 0);
}

void QuestParserCollection::EventKilledMerit(NPC *npc, Client *client, std::string zone) {
    _EventNPC(EVENT_KILLED_MERIT, npc, client, zone, "", 0);
}

void QuestParserCollection::EventCastOn(NPC *npc, Client *client, std::string spell_id, std::string zone) {
    _EventNPC(EVENT_CAST_ON, npc, client, zone, spell_id, 0);
}

void QuestParserCollection::EventTaskAccepted(NPC *npc, Client *client, std::string task_id, std::string zone) {
    _EventNPC(EVENT_TASKACCEPTED, npc, client, zone, task_id, 0);
}

void QuestParserCollection::EventTaskStageComplete(Client *client, std::string task_info, std::string zone) {
    _EventPlayer(EVENT_TASK_STAGE_COMPLETE, client, zone, task_info, 0);
}

void QuestParserCollection::EventAggroSay(const char *message, uint32_t language, NPC * npc, Client * client, std::string zone) {
     _EventNPC(EVENT_AGGRO_SAY, npc, client, zone, message, language);
}

void QuestParserCollection::EventPlayerPickup(Client *client, std::string item_id, std::string zone) {
    _EventPlayer(EVENT_PLAYER_PICKUP, client, zone, item_id, 0);
}

void QuestParserCollection::EventPopupResponse(Client *client, std::string popup_id, std::string zone) {
    _EventPlayer(EVENT_POPUPRESPONSE, client, zone, popup_id, 0);
}

void QuestParserCollection::EventProximitySay(const char *message, uint32_t language, NPC * npc, Client * client, std::string zone) {
    _EventNPC(EVENT_PROXIMITY_SAY, npc, client, zone, message, language);
}

void QuestParserCollection::EventCast(Client *client, std::string spell_id) {
    _EventSpell(EVENT_CAST, NULL, client, spell_id, 0);
}

void QuestParserCollection::EventScaleCalc(Client *client, ItemInst *item) {
    _EventItem(EVENT_SCALE_CALC, client, item, 0);
}

void QuestParserCollection::EventTargetChange(NPC *npc, Mob *other, std::string zone) {
    _EventNPC(EVENT_TARGET_CHANGE, npc, other, zone, "", 0);
}

void QuestParserCollection::EventTargetChange(Client *client, std::string zone) {
    _EventPlayer(EVENT_TARGET_CHANGE, client, zone, "", 0);
}

void QuestParserCollection::EventHateList(NPC *npc, Mob *other, bool join, std::string zone) {
    _EventNPC(EVENT_HATE_LIST, npc, other, zone, join ? "1" : "0", 0);
}

void QuestParserCollection::EventSpellEffectClient(Client *target, std::string spell_id, uint32_t caster_id) {
    _EventSpell(EVENT_SPELL_EFFECT_CLIENT, NULL, target, spell_id, caster_id);
}

void QuestParserCollection::EventSpellEffectNPC(NPC *target, std::string spell_id, uint32_t caster_id) {
    _EventSpell(EVENT_SPELL_EFFECT_NPC, target, NULL, spell_id, caster_id);
}

void QuestParserCollection::EventSpellEffectBuffTicClient(Client *target, std::string spell_id, uint32_t caster_id) {
    _EventSpell(EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT, NULL, target, spell_id, caster_id);
}

void QuestParserCollection::EventSpellEffectBuffTicNPC(NPC *target, std::string spell_id, uint32_t caster_id) {
    _EventSpell(EVENT_SPELL_EFFECT_BUFF_TIC_NPC, target, NULL, spell_id, caster_id);
}

void QuestParserCollection::EventSpellEffectTranslocateComplete(Client *client, std::string spell_id) {
    _EventSpell(EVENT_SPELL_EFFECT_TRANSLOCATE_COMPLETE, NULL, client, spell_id, 0);
}

void QuestParserCollection::EventCombineSuccess(Client *client, uint32_t recipe_id, std::string recipe_name, std::string zone) {
    _EventPlayer(EVENT_COMBINE_SUCCESS, client, zone, recipe_name, recipe_id);
}

void QuestParserCollection::EventCombineFailure(Client *client, uint32_t recipe_id, std::string recipe_name, std::string zone) {
    _EventPlayer(EVENT_COMBINE_FAILURE, client, zone, recipe_name, recipe_id);
}

void QuestParserCollection::EventItemClick(Client *client, ItemInst *item, uint32_t slot_id) {
    _EventItem(EVENT_ITEM_CLICK, client, item, slot_id);
}

void QuestParserCollection::EventItemClickCast(Client *client, ItemInst *item, uint32_t slot_id) {
    _EventItem(EVENT_ITEM_CLICK_CAST, client, item, slot_id);
}

void QuestParserCollection::EventGroupChange(Client *client, std::string zone)
{
    _EventPlayer(EVENT_GROUP_CHANGE, client, zone, "", 0);
}



