#include "../common/MiscFunctions.h"
#include "../common/servertalk.h"
#include "../common/rulesys.h"
#include "Adventure.h"
#include "AdventureManager.h"
#include "worlddb.h"
#include "zonelist.h"
#include "clientlist.h"
#include "cliententry.h"
#include <sstream>
#include <stdio.h>

extern ZSList zoneserver_list;
extern ClientList client_list;

AdventureManager::AdventureManager()
{
	process_timer = new Timer(500);
	save_timer = new Timer(60000);
	leaderboard_info_timer = new Timer(300000);
}

AdventureManager::~AdventureManager()
{
	safe_delete(process_timer);
	safe_delete(save_timer);
	safe_delete(leaderboard_info_timer);
}

void AdventureManager::Process()
{
	if(process_timer->Check())
	{
		list<Adventure*>::iterator iter = adventure_list.begin();
		while(iter != adventure_list.end())
		{
			if(!(*iter)->Process())
			{
				Adventure *adv = (*iter);
				iter = adventure_list.erase(iter);
				GetAdventureData(adv);
				delete adv;
				Save();
				continue;
			}
			iter++;
		}
	}

	if(leaderboard_info_timer->Check())
	{
		LoadLeaderboardInfo();
	}

	if(save_timer->Check())
	{
		Save();
	}
}

void AdventureManager::CalculateAdventureRequestReply(const char *data)
{
	ServerAdventureRequest_Struct *sar = (ServerAdventureRequest_Struct*)data;
	ClientListEntry *leader = client_list.FindCharacter(sar->leader);
	if(!leader)
	{
		return;
	}

	/** 
	 * This block checks to see if we actually have any adventures for the requested theme.
	 */
	map<uint32, list<AdventureTemplate*> >::iterator adv_list_iter = adventure_entries.find(sar->template_id);
	if(adv_list_iter == adventure_entries.end())
	{
		ServerPacket *pack = new ServerPacket(ServerOP_AdventureRequestDeny, sizeof(ServerAdventureRequestDeny_Struct));
		ServerAdventureRequestDeny_Struct *deny = (ServerAdventureRequestDeny_Struct*)pack->pBuffer;
		strcpy(deny->leader, sar->leader);
		strcpy(deny->reason, "There are currently no adventures set for this theme.");
		pack->Deflate();
		zoneserver_list.SendPacket(leader->zone(), leader->instance(), pack);
		delete pack;
		return;
	}

	/**
	 * This block checks to see if our requested group has anyone with an "Active" adventure
	 * Active being in progress, finished adventures that are still waiting to expire do not count
	 * Though they will count against you for which new adventure you can get.
	 */
	list<Adventure*>::iterator iter = adventure_list.begin();
	while(iter != adventure_list.end())
	{
		Adventure* current = (*iter);
		if(current->IsActive())
		{
			for(int i = 0; i < sar->member_count; ++i)
			{
				if(current->PlayerExists((data + sizeof(ServerAdventureRequest_Struct) + (64 * i))))
				{
					ServerPacket *pack = new ServerPacket(ServerOP_AdventureRequestDeny, sizeof(ServerAdventureRequestDeny_Struct));
					ServerAdventureRequestDeny_Struct *deny = (ServerAdventureRequestDeny_Struct*)pack->pBuffer;
					strcpy(deny->leader, sar->leader);

					stringstream ss(stringstream::out | stringstream::in);
					ss << (data + sizeof(ServerAdventureRequest_Struct) + (64 * i)) << "is already apart of an active adventure.";

					strcpy(deny->reason, ss.str().c_str());
					pack->Deflate();
					zoneserver_list.SendPacket(leader->zone(), leader->instance(), pack);
					delete pack;
					return;
				}
			}
		}
		iter++;
	}

	/**
	 * Now we need to get every available adventure for our selected theme and exclude ones we can't use.
	 * ie. the ones that would cause overlap issues for new adventures with the old unexpired adventures.
	 */
	list<AdventureZones> excluded_zones;
	list<AdventureZoneIn> excluded_zone_ins;
	for(int i = 0; i < sar->member_count; ++i)
	{
		int finished_adventures_count;
		Adventure **finished_adventures = GetFinishedAdventures((data + sizeof(ServerAdventureRequest_Struct) + (64 * i)), finished_adventures_count);
		for(int i = 0; i < finished_adventures_count; ++i)
		{
			if(!IsInExcludedZoneList(excluded_zones, finished_adventures[i]->GetTemplate()->zone, finished_adventures[i]->GetTemplate()->zone_version))
			{
				AdventureZones adv;
				adv.zone = finished_adventures[i]->GetTemplate()->zone;
				adv.version = finished_adventures[i]->GetTemplate()->zone_version;
				excluded_zones.push_back(adv);
			}
			if(!IsInExcludedZoneInList(excluded_zone_ins, finished_adventures[i]->GetTemplate()->zone_in_zone_id, 
				finished_adventures[i]->GetTemplate()->zone_in_object_id))
			{
				AdventureZoneIn adv;
				adv.door_id = finished_adventures[i]->GetTemplate()->zone_in_object_id;
				adv.zone_id = finished_adventures[i]->GetTemplate()->zone_in_zone_id;
				excluded_zone_ins.push_back(adv);
			}
		}
		safe_delete_array(finished_adventures);
	}

	list<AdventureTemplate*> eligible_adventures = adventure_entries[sar->template_id];
	/**
	 * Remove zones from eligible zones based on their difficulty and type.
	 * ie only use difficult zones for difficult, collect for collect, etc.
	 */
	list<AdventureTemplate*>::iterator ea_iter = eligible_adventures.begin();
	while(ea_iter != eligible_adventures.end())
	{
		if((*ea_iter)->is_hard != ((sar->risk == 1) ? true : false))
		{
			ea_iter = eligible_adventures.erase(ea_iter);
			continue;
		}

		if(sar->type != 0 && sar->type != (*ea_iter)->type)
		{
			ea_iter = eligible_adventures.erase(ea_iter);
			continue;
		}
		ea_iter++;
	}

	/**
	 * Get levels for this group.
	 */
	int valid_count = 0;
	int avg_level = 0;
	int min_level = 40000;
	int max_level = 0;

	for(int i = 0; i < sar->member_count; ++i)
	{
		ClientListEntry *current = client_list.FindCharacter((data + sizeof(ServerAdventureRequest_Struct) + (64 * i)));
		if(current)
		{
			int lvl = current->level();
			if(lvl != 0)
			{
				avg_level += lvl;
				valid_count++;
				if(lvl < min_level)
				{
					min_level = lvl;
				}
				if(lvl > max_level)
				{
					max_level = lvl;
				}
			}
			else
			{
				if(database.GetCharacterLevel((data + sizeof(ServerAdventureRequest_Struct) + (64 * i)), lvl))
				{
					avg_level += lvl;
					valid_count++;
					if(lvl < min_level)
					{
						min_level = lvl;
					}
					if(lvl > max_level)
					{
						max_level = lvl;
					}
				}
			}
		}
		else
		{
			int lvl = 0;
			if(database.GetCharacterLevel((data + sizeof(ServerAdventureRequest_Struct) + (64 * i)), lvl))
			{
				avg_level += lvl;
				valid_count++;
				if(lvl < min_level)
				{
					min_level = lvl;
				}
				if(lvl > max_level)
				{
					max_level = lvl;
				}
			}
		}
	}

	if(valid_count == 0)
	{
		ServerPacket *pack = new ServerPacket(ServerOP_AdventureRequestDeny, sizeof(ServerAdventureRequestDeny_Struct));
		ServerAdventureRequestDeny_Struct *deny = (ServerAdventureRequestDeny_Struct*)pack->pBuffer;
		strcpy(deny->leader, sar->leader);
		strcpy(deny->reason, "The number of found players for this adventure was zero.");
		pack->Deflate();
		zoneserver_list.SendPacket(leader->zone(), leader->instance(), pack);
		delete pack;
		return;
	}

	avg_level = avg_level / valid_count;

	if(max_level - min_level > RuleI(Adventure, MaxLevelRange))
	{
		ServerPacket *pack = new ServerPacket(ServerOP_AdventureRequestDeny, sizeof(ServerAdventureRequestDeny_Struct));
		ServerAdventureRequestDeny_Struct *deny = (ServerAdventureRequestDeny_Struct*)pack->pBuffer;
		strcpy(deny->leader, sar->leader);

		stringstream ss(stringstream::out | stringstream::in);
		ss << "The maximum level range for this adventure is " << RuleI(Adventure, MaxLevelRange); 
		ss << " but the level range calculated was " << (max_level - min_level) << ".";
		strcpy(deny->reason, ss.str().c_str());
		pack->Deflate();
		zoneserver_list.SendPacket(leader->zone(), leader->instance(), pack);
		delete pack;
		return;
	}

	/**
	 * Remove the zones from our eligible zones based on the exclusion above
	 */
	list<AdventureZones>::iterator ez_iter = excluded_zones.begin();
	while(ez_iter != excluded_zones.end())
	{
		list<AdventureTemplate*>::iterator ea_iter = eligible_adventures.begin();
		while(ea_iter != eligible_adventures.end())
		{
			if((*ez_iter).zone.compare((*ea_iter)->zone) == 0 && (*ez_iter).version == (*ea_iter)->zone_version)
			{
				ea_iter = eligible_adventures.erase(ea_iter);
				continue;
			}
			ea_iter++;
		}
		ez_iter++;
	}

	list<AdventureZoneIn>::iterator ezi_iter = excluded_zone_ins.begin();
	 while(ezi_iter != excluded_zone_ins.end())
	{
		list<AdventureTemplate*>::iterator ea_iter = eligible_adventures.begin();
		while(ea_iter != eligible_adventures.end())
		{
			if((*ezi_iter).zone_id == (*ea_iter)->zone_in_zone_id && (*ezi_iter).door_id == (*ea_iter)->zone_in_object_id)
			{
				ea_iter = eligible_adventures.erase(ea_iter);
				continue;
			}
			ea_iter++;
		}
		ezi_iter++;
	}

	 /**
	 * Remove Zones based on level
	 */
	ea_iter = eligible_adventures.begin();
	while(ea_iter != eligible_adventures.end())
	{
		if((*ea_iter)->min_level > avg_level)
		{
			ea_iter = eligible_adventures.erase(ea_iter);
			continue;
		}

		if((*ea_iter)->max_level < avg_level)
		{
			ea_iter = eligible_adventures.erase(ea_iter);
			continue;
		}
		ea_iter++;
	}

	if(eligible_adventures.size() > 0)
	{
		ea_iter = eligible_adventures.begin();
		int c_index = MakeRandomInt(0, (eligible_adventures.size()-1));
		for(int i = 0; i < c_index; ++i)
		{
			ea_iter++;
		}
		ServerPacket *pack = new ServerPacket(ServerOP_AdventureRequestAccept, sizeof(ServerAdventureRequestAccept_Struct) + (sar->member_count * 64));
		ServerAdventureRequestAccept_Struct *sra = (ServerAdventureRequestAccept_Struct*)pack->pBuffer;
		strcpy(sra->leader, sar->leader);
		strcpy(sra->text, (*ea_iter)->text);
		sra->theme = sar->template_id;
		sra->id = (*ea_iter)->id;
		sra->member_count = sar->member_count;
		memcpy((pack->pBuffer + sizeof(ServerAdventureRequestAccept_Struct)), (data + sizeof(ServerAdventureRequest_Struct)), (sar->member_count * 64));
		pack->Deflate();
		zoneserver_list.SendPacket(leader->zone(), leader->instance(), pack);
		delete pack;
		return;
	}
	else
	{
		ServerPacket *pack = new ServerPacket(ServerOP_AdventureRequestDeny, sizeof(ServerAdventureRequestDeny_Struct));
		ServerAdventureRequestDeny_Struct *deny = (ServerAdventureRequestDeny_Struct*)pack->pBuffer;
		strcpy(deny->leader, sar->leader);
		strcpy(deny->reason, "The number of adventures returned was zero.");
		pack->Deflate();
		zoneserver_list.SendPacket(leader->zone(), leader->instance(), pack);
		delete pack;
		return;
	}
}

void AdventureManager::TryAdventureCreate(const char *data)
{
	ServerAdventureRequestCreate_Struct *src = (ServerAdventureRequestCreate_Struct*)data;
	ClientListEntry *leader = client_list.FindCharacter(src->leader);
	if(!leader)
	{
		return;
	}

	AdventureTemplate *adv_template = GetAdventureTemplate(src->theme, src->id);
	if(!adv_template)
	{
		ServerPacket *pack = new ServerPacket(ServerOP_AdventureCreateDeny, 64);
		strcpy((char*)pack->pBuffer, src->leader);
		pack->Deflate();
		zoneserver_list.SendPacket(leader->zone(), leader->instance(), pack);
		delete pack;
		return;
	}

	Adventure *adv = new Adventure(adv_template);
	if(!adv->CreateInstance())
	{
		ServerPacket *pack = new ServerPacket(ServerOP_AdventureCreateDeny, 64);
		strcpy((char*)pack->pBuffer, src->leader);
		pack->Deflate();
		zoneserver_list.SendPacket(leader->zone(), leader->instance(), pack);
		delete pack;
		delete adv;
		return;
	}

	for(int i = 0; i < src->member_count; ++i)
	{
		Adventure *a = GetActiveAdventure((data + sizeof(ServerAdventureRequestCreate_Struct) + (64 * i)));
		if(a)
		{
			ServerPacket *pack = new ServerPacket(ServerOP_AdventureCreateDeny, 64);
			strcpy((char*)pack->pBuffer, src->leader);
			pack->Deflate();
			zoneserver_list.SendPacket(leader->zone(), leader->instance(), pack);
			delete pack;
			delete adv;
			return;
		}
		
		adv->AddPlayer((data + sizeof(ServerAdventureRequestCreate_Struct) + (64 * i)));
	}

	//Need to send adventure data to zone server for each client.
	for(int i = 0; i < src->member_count; ++i)
	{

		ClientListEntry *player = client_list.FindCharacter((data + sizeof(ServerAdventureRequestCreate_Struct) + (64 * i)));
		if(player)
		{
			int f_count = 0;
			Adventure** finished_adventures = GetFinishedAdventures((data + sizeof(ServerAdventureRequestCreate_Struct) + (64 * i)), f_count);
			ServerPacket *pack = new ServerPacket(ServerOP_AdventureData, sizeof(ServerSendAdventureData_Struct) 
				+ (sizeof(ServerFinishedAdventures_Struct) * f_count));
			ServerSendAdventureData_Struct *sca = (ServerSendAdventureData_Struct*)pack->pBuffer;

			strcpy(sca->player, (data + sizeof(ServerAdventureRequestCreate_Struct) + (64 * i)));
			strcpy(sca->text, adv_template->text);
			sca->time_left = adv_template->zone_in_time;
			sca->time_to_enter = adv_template->zone_in_time;
			sca->risk = adv_template->is_hard ? 1 : 0;
			sca->x = adv_template->zone_in_x;
			sca->y = adv_template->zone_in_y;
			sca->zone_in_id = adv_template->zone_in_zone_id;
			sca->zone_in_object = adv_template->zone_in_object_id;
			sca->instance_id = adv->GetInstanceID();
			sca->count = 0;
			sca->total = adv_template->type_count;
			sca->finished_adventures = f_count;
			for(int f = 0; f < f_count; ++f)
			{
				ServerFinishedAdventures_Struct *sfa = (ServerFinishedAdventures_Struct*)(pack->pBuffer 
					+ sizeof(ServerSendAdventureData_Struct) 
					+ (sizeof(ServerFinishedAdventures_Struct) * f));
				sfa->zone_in_id = finished_adventures[f]->GetTemplate()->zone_in_zone_id;
				sfa->zone_in_object = finished_adventures[f]->GetTemplate()->zone_in_object_id;
			}

			pack->Deflate();
			zoneserver_list.SendPacket(player->zone(), player->instance(), pack);
			safe_delete_array(finished_adventures);
			delete pack;
		}
	}

	adventure_list.push_back(adv);
	Save();
}

void AdventureManager::GetAdventureData(Adventure *adv)
{
	list<string> player_list = adv->GetPlayers();
	list<string>::iterator iter = player_list.begin();
	while(iter != player_list.end())
	{
		GetAdventureData((*iter).c_str());
		iter++;
	}
}

void AdventureManager::GetAdventureData(const char *name)
{
	ClientListEntry *player = client_list.FindCharacter(name);
	if(player)
	{
		int f_count = 0;
		Adventure** finished_adventures = GetFinishedAdventures(name, f_count);
		Adventure *current = GetActiveAdventure(name);
		ServerPacket *pack = new ServerPacket(ServerOP_AdventureData, sizeof(ServerSendAdventureData_Struct) 
			+ (sizeof(ServerFinishedAdventures_Struct) * f_count));
		ServerSendAdventureData_Struct *sca = (ServerSendAdventureData_Struct*)pack->pBuffer;

		if(current)
		{
			const AdventureTemplate *adv_template = current->GetTemplate();
			strcpy(sca->player, name);
			strcpy(sca->text, adv_template->text);
			sca->risk = adv_template->is_hard ? 1 : 0;
			sca->x = adv_template->zone_in_x;
			sca->y = adv_template->zone_in_y;
			sca->zone_in_id = adv_template->zone_in_zone_id;
			sca->zone_in_object = adv_template->zone_in_object_id;
			sca->count = current->GetCount();
			sca->total = adv_template->type_count;

			sca->time_left = current->GetRemainingTime();
			if(current->GetStatus() == AS_WaitingForZoneIn)
			{
				sca->time_to_enter = sca->time_left;
			}
		}
		else
		{
			//We have no mission and no finished missions
			//Delete our stuff and return instead of sending a blank packet.
			if(f_count == 0)
			{
				delete pack;
				ServerPacket *pack = new ServerPacket(ServerOP_AdventureDataClear, 64);
				strcpy((char*)pack->pBuffer, name);
				pack->Deflate();
				zoneserver_list.SendPacket(player->zone(), player->instance(), pack);

				delete pack;
				delete[] finished_adventures;
				return;
			}
		}

		sca->finished_adventures = f_count;
		for(int i = 0; i < f_count; ++i)
		{
			ServerFinishedAdventures_Struct *sfa = (ServerFinishedAdventures_Struct*)(pack->pBuffer 
				+ sizeof(ServerSendAdventureData_Struct) 
				+ (sizeof(ServerFinishedAdventures_Struct) * i));
			sfa->zone_in_id = finished_adventures[i]->GetTemplate()->zone_in_zone_id;
			sfa->zone_in_object = finished_adventures[i]->GetTemplate()->zone_in_object_id;
		}

		pack->Deflate();
		zoneserver_list.SendPacket(player->zone(), player->instance(), pack);
		safe_delete_array(finished_adventures);
		delete pack;
		delete[] finished_adventures;
	}
}

bool AdventureManager::IsInExcludedZoneList(list<AdventureZones> excluded_zones, string zone_name, int version)
{
	list<AdventureZones>::iterator iter = excluded_zones.begin();
	while(iter != excluded_zones.end())
	{
		if(((*iter).zone.compare(zone_name) == 0) && ((*iter).version == version))
		{
			return true;
		}
		iter++;
	}
	return false;
}

bool AdventureManager::IsInExcludedZoneInList(list<AdventureZoneIn> excluded_zone_ins, int zone_id, int door_object)
{
	list<AdventureZoneIn>::iterator iter = excluded_zone_ins.begin();
	while(iter != excluded_zone_ins.end())
	{
		if(((*iter).zone_id == zone_id) && ((*iter).door_id == door_object))
		{
			return true;
		}
		iter++;
	}
	return false;
}

Adventure **AdventureManager::GetFinishedAdventures(const char *player, int &count)
{
	Adventure **ret = NULL;
	count = 0;

	list<Adventure*>::iterator iter = adventure_list.begin();
	while(iter != adventure_list.end())
	{
		if(!(*iter)->IsActive())
		{
			if(ret)
			{
				Adventure **t = new Adventure*[count + 1];
				for(int i = 0; i < count; i++)
				{
					t[i] = ret[i];
				}
				t[count] = (*iter);
				delete[] ret;
				ret = t;
			}
			else
			{
				ret = new Adventure*[1];
				ret[0] = (*iter);
			}
			count++;
		}
		iter++;
	}
	return ret;
}

Adventure *AdventureManager::GetActiveAdventure(const char *player)
{
	list<Adventure*>::iterator iter = adventure_list.begin();
	while(iter != adventure_list.end())
	{
		if((*iter)->PlayerExists(player) && (*iter)->IsActive())
		{
			return (*iter);
		}
		iter++;
	}
	return NULL;
}

AdventureTemplate *AdventureManager::GetAdventureTemplate(int theme, int id)
{
	map<uint32, list<AdventureTemplate*> >::iterator iter = adventure_entries.find(theme);
	if(iter == adventure_entries.end())
	{
		return NULL;
	}

	list<AdventureTemplate*>::iterator l_iter = (*iter).second.begin();
	while(l_iter != (*iter).second.end())
	{
		if((*l_iter)->id == id)
		{
			return (*l_iter);
		}
		l_iter++;
	}
	return NULL;
}

AdventureTemplate *AdventureManager::GetAdventureTemplate(int id)
{
	map<uint32, AdventureTemplate*>::iterator iter = adventure_templates.find(id);
	if(iter == adventure_templates.end())
	{
		return NULL;
	}

	return iter->second;
}

bool AdventureManager::LoadAdventureTemplates()
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if(database.RunQuery(query,MakeAnyLenString(&query,"SELECT id, zone, zone_version, "
		"is_hard, min_level, max_level, type, type_data, type_count, assa_x, "
		"assa_y, assa_z, assa_h, text, duration, zone_in_time, win_points, lose_points, "
		"theme, zone_in_zone_id, zone_in_x, zone_in_y, zone_in_object_id, dest_x, dest_y,"
		" dest_z, dest_h, graveyard_zone_id, graveyard_x, graveyard_y, graveyard_z, "
		"graveyard_radius FROM adventure_template"), errbuf, &result)) 
	{
		while((row = mysql_fetch_row(result))) 
		{
			int8 x = 0;
			AdventureTemplate *t = new AdventureTemplate;
			t->id = atoi(row[x++]);
			strcpy(t->zone, row[x++]);
			t->zone_version = atoi(row[x++]);
			t->is_hard = atoi(row[x++]);
			t->min_level = atoi(row[x++]);
			t->max_level = atoi(row[x++]);
			t->type = atoi(row[x++]);
			t->type_data = atoi(row[x++]);
			t->type_count = atoi(row[x++]);
			t->assa_x = atof(row[x++]);
			t->assa_y = atof(row[x++]);
			t->assa_z = atof(row[x++]);
			t->assa_h = atof(row[x++]);
			strcpy(t->text, row[x++]);
			t->duration = atoi(row[x++]);
			t->zone_in_time = atoi(row[x++]);
			t->win_points = atoi(row[x++]);
			t->lose_points = atoi(row[x++]);
			t->theme = atoi(row[x++]);
			t->zone_in_zone_id = atoi(row[x++]);
			t->zone_in_x = atof(row[x++]);
			t->zone_in_y = atof(row[x++]);
			t->zone_in_object_id = atoi(row[x++]);
			t->dest_x = atof(row[x++]);
			t->dest_y = atof(row[x++]);
			t->dest_z = atof(row[x++]);
			t->dest_h = atof(row[x++]);
			t->graveyard_zone_id = atoi(row[x++]);
			t->graveyard_x = atof(row[x++]);
			t->graveyard_y = atof(row[x++]);
			t->graveyard_z = atof(row[x++]);
			t->graveyard_radius = atof(row[x++]);
			adventure_templates[t->id] = t;
		}
		mysql_free_result(result);
		safe_delete_array(query);
		return true;
	}
	else
	{
		LogFile->write(EQEMuLog::Error, "Error in AdventureManager:::LoadAdventures: %s (%s)", query, errbuf);
		safe_delete_array(query);
		return false;
	}
	return false;
}

bool AdventureManager::LoadAdventureEntries()
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if(database.RunQuery(query,MakeAnyLenString(&query,"SELECT id, template_id FROM adventure_template_entry"), errbuf, &result)) 
	{
		while((row = mysql_fetch_row(result))) 
		{
			int id = atoi(row[0]);
			int template_id = atoi(row[1]);
			AdventureTemplate* tid = NULL;

			map<uint32, AdventureTemplate*>::iterator t_iter = adventure_templates.find(template_id);
			if(t_iter == adventure_templates.end())
			{
				continue;
			}
			else
			{
				tid = adventure_templates[template_id];
			}

			list<AdventureTemplate*> temp;
			map<uint32, list<AdventureTemplate*> >::iterator iter = adventure_entries.find(id);
			if(iter == adventure_entries.end())
			{
				temp.push_back(tid);
				adventure_entries[id] = temp;
			}
			else
			{
				temp = adventure_entries[id];
				temp.push_back(tid);
				adventure_entries[id] = temp;
			}
		}
		mysql_free_result(result);
		safe_delete_array(query);
		return true;
	}
	else
	{
		LogFile->write(EQEMuLog::Error, "Error in AdventureManager:::LoadAdventureEntries: %s (%s)", query, errbuf);
		safe_delete_array(query);
		return false;
	}
	return false;
}

void AdventureManager::PlayerClickedDoor(const char *player, int zone_id, int door_id)
{
	list<Adventure*>::iterator iter = adventure_list.begin();
	while(iter != adventure_list.end())
	{
		const AdventureTemplate *t = (*iter)->GetTemplate();
		if(t->zone_in_zone_id == zone_id && t->zone_in_object_id == door_id)
		{
			if((*iter)->PlayerExists(player))
			{
				ClientListEntry *pc = client_list.FindCharacter(player);
				if(pc)
				{
					ServerPacket *pack = new ServerPacket(ServerOP_AdventureClickDoorReply, sizeof(ServerPlayerClickedAdventureDoorReply_Struct));
					ServerPlayerClickedAdventureDoorReply_Struct *sr = (ServerPlayerClickedAdventureDoorReply_Struct*)pack->pBuffer;
					strcpy(sr->player, player);
					sr->zone_id = database.GetZoneID(t->zone);
					sr->instance_id = (*iter)->GetInstanceID();
					sr->x = t->dest_x;
					sr->y = t->dest_y;
					sr->z = t->dest_z;
					sr->h = t->dest_h;
					if((*iter)->GetStatus() == AS_WaitingForZoneIn)
					{
						(*iter)->SetStatus(AS_WaitingForPrimaryEndTime);
						Save();
					}

					pack->Deflate();
					zoneserver_list.SendPacket(pc->zone(), pc->instance(), pack);
					safe_delete(pack);
				}
				return;
			}
		}
		iter++;
	}

	ClientListEntry *pc = client_list.FindCharacter(player);
	if(pc)
	{
		ServerPacket *pack = new ServerPacket(ServerOP_AdventureClickDoorError, 64);
		strcpy((char*)pack->pBuffer, player);
		pack->Deflate();
		zoneserver_list.SendPacket(pc->zone(), pc->instance(), pack);
		safe_delete(pack);
	}
}

void AdventureManager::LeaveAdventure(const char *name)
{
	ClientListEntry *pc = client_list.FindCharacter(name);
	if(pc)
	{
		Adventure *current = GetActiveAdventure(name);
		if(current)
		{
			if(pc->instance() != 0 && pc->instance() == current->GetInstanceID())
			{
				ServerPacket *pack = new ServerPacket(ServerOP_AdventureLeaveDeny, 64);
				strcpy((char*)pack->pBuffer, name);
				pack->Deflate();
				zoneserver_list.SendPacket(pc->zone(), pc->instance(), pack);
				safe_delete(pack);
			}
			else
			{
				if(current->GetStatus() != AS_WaitingForZoneIn)
				{
					database.UpdateAdventureStatsEntry(database.GetCharacterID(name), current->GetTemplate()->theme, false);
				}

				current->RemovePlayer(name);
				Save();
				ServerPacket *pack = new ServerPacket(ServerOP_AdventureLeaveReply, 64);
				strcpy((char*)pack->pBuffer, name);
				pack->Deflate();
				zoneserver_list.SendPacket(pc->zone(), pc->instance(), pack);
				safe_delete(pack);
			}
		}
		else
		{
			ServerPacket *pack = new ServerPacket(ServerOP_AdventureLeaveReply, 64);
			strcpy((char*)pack->pBuffer, name);
			pack->Deflate();
			zoneserver_list.SendPacket(pc->zone(), pc->instance(), pack);
			safe_delete(pack);
		}
	}
}

void AdventureManager::IncrementCount(uint16 instance_id)
{
	list<Adventure*>::iterator iter = adventure_list.begin();
	Adventure *current = NULL;
	while(iter != adventure_list.end())
	{
		if((*iter)->GetInstanceID() == instance_id)
		{
			current = (*iter);
			break;
		}
		iter++;
	}

	if(current)
	{
		current->IncrementCount();
		list<string> slist = current->GetPlayers();
		list<string>::iterator siter = slist.begin();
		ServerPacket *pack = new ServerPacket(ServerOP_AdventureCountUpdate, sizeof(ServerAdventureCountUpdate_Struct));
		ServerAdventureCountUpdate_Struct *ac = (ServerAdventureCountUpdate_Struct*)pack->pBuffer;
		ac->count = current->GetCount();
		ac->total = current->GetTemplate()->type_count;
		
		while(siter != slist.end())
		{
			ClientListEntry *pc = client_list.FindCharacter((*siter).c_str());
			if(pc)
			{
				memset(ac->player, 0, 64);
				strcpy(ac->player, (*siter).c_str());
				zoneserver_list.SendPacket(pc->zone(), pc->instance(), pack);
			}
			siter++;
		}

		delete pack;
	}
}

void AdventureManager::IncrementAssassinationCount(uint16 instance_id)
{
	list<Adventure*>::iterator iter = adventure_list.begin();
	Adventure *current = NULL;
	while(iter != adventure_list.end())
	{
		if((*iter)->GetInstanceID() == instance_id)
		{
			current = (*iter);
			break;
		}
		iter++;
	}

	if(current)
	{
		current->IncrementAssassinationCount();
	}
}


void AdventureManager::GetZoneData(uint16 instance_id)
{
	list<Adventure*>::iterator iter = adventure_list.begin();
	Adventure *current = NULL;
	while(iter != adventure_list.end())
	{
		if((*iter)->GetInstanceID() == instance_id)
		{
			current = (*iter);
			break;
		}
		iter++;
	}

	if(current)
	{
		ServerPacket *pack = new ServerPacket(ServerOP_AdventureZoneData, sizeof(ServerZoneAdventureDataReply_Struct));
		ServerZoneAdventureDataReply_Struct *zd = (ServerZoneAdventureDataReply_Struct*)pack->pBuffer;

		const AdventureTemplate* temp = current->GetTemplate();
		zd->instance_id = instance_id;
		zd->count = current->GetCount();
		zd->total = temp->type_count;
		zd->type = temp->type;
		zd->data_id = temp->type_data;
		zd->assa_count = current->GetAssassinationCount();
		zd->assa_x = temp->assa_x;
		zd->assa_y = temp->assa_y;
		zd->assa_z = temp->assa_z;
		zd->assa_h = temp->assa_h;
		zd->dest_x = temp->dest_x;
		zd->dest_y = temp->dest_y;
		zd->dest_z = temp->dest_z;
		zd->dest_h = temp->dest_h;
		zoneserver_list.SendPacket(0, instance_id, pack);
		delete pack;
	}
}

// Sort the leaderboard by wins to losses
bool pred_s_lb1(const LeaderboardInfo &lbi1, const LeaderboardInfo &lbi2)
{
	return (lbi1.wins - lbi1.losses) > (lbi2.wins - lbi2.losses);
}

// Sort the leaderboard by wins.
bool pred_s_lb2(const LeaderboardInfo &lbi1, const LeaderboardInfo &lbi2)
{
	return lbi1.wins > lbi2.wins;
}

void AdventureManager::LoadLeaderboardInfo()
{
	leaderboard_info.clear();
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if(database.RunQuery(query,MakeAnyLenString(&query,"select ch.name, ch.id, adv_stats.* from adventure_stats "
		"AS adv_stats ""left join character_ AS ch on adv_stats.player_id = ch.id;"), errbuf, &result)) 
	{
		while((row = mysql_fetch_row(result))) 
		{
			if(row[0])
			{
				LeaderboardInfo lbi;
				lbi.name = row[0];
				lbi.wins = atoi(row[3]);
				lbi.wins += atoi(row[4]);
				lbi.wins += atoi(row[5]);
				lbi.wins += atoi(row[6]);
				lbi.wins += atoi(row[7]);
				lbi.losses = atoi(row[8]);
				lbi.losses += atoi(row[9]);
				lbi.losses += atoi(row[10]);
				lbi.losses += atoi(row[11]);
				lbi.losses += atoi(row[12]);
				leaderboard_info.push_back(lbi);
			}
		}
		mysql_free_result(result);
		safe_delete_array(query);
		leaderboard_info.sort(pred_s_lb2);
		leaderboard_info.sort(pred_s_lb1);
		return;
	}
	else
	{
		LogFile->write(EQEMuLog::Error, "Error in AdventureManager:::GetLeaderboardInfo: %s (%s)", query, errbuf);
		safe_delete_array(query);
		return;
	}
	return;
};

void AdventureManager::DoLeaderboardRequest(const char* player)
{
	ClientListEntry *pc = client_list.FindCharacter(player);
	if(pc)
	{
		ServerPacket *pack = new ServerPacket(ServerOP_AdventureLeaderboard, 64 + sizeof(AdventureLeaderboard_Struct));
		AdventureLeaderboard_Struct *al = (AdventureLeaderboard_Struct*)(pack->pBuffer + 64);
		strcpy((char*)pack->pBuffer, player);

		int place = -1;
		int our_successes;
		int our_failures;
		int i = 0;
		list<LeaderboardInfo>::iterator iter = leaderboard_info.begin();
		while(i < 100 && iter != leaderboard_info.end())
		{
			LeaderboardInfo li = (*iter);
			if(li.name.compare(player) == 0)
			{
				place = i;
				our_successes = li.wins;
				our_failures = li.losses;
			}

			al->entries[i].success = li.wins;
			al->entries[i].failure = li.losses;
			strcpy(al->entries[i].name, li.name.c_str());
			i++;
			iter++;
		}

		if(place == -1 && iter != leaderboard_info.end())
		{
			while(iter != leaderboard_info.end())
			{
				LeaderboardInfo li = (*iter);
				if(li.name.compare(player) == 0)
				{
					place = i;
					our_successes = li.wins;
					our_failures = li.losses;
					break;
				}
				i++;
				iter++;
			}
		}

		if(place == -1)
		{
			al->our_rank = leaderboard_info.size() + 1;
			al->success = 0;
			al->failure = 0;
		}
		else
		{
			al->our_rank = place;
			al->success = our_successes;
			al->failure = our_failures;
		}
		
		pack->Deflate();
		zoneserver_list.SendPacket(pc->zone(), pc->instance(), pack);
		delete pack;
	}
}

bool AdventureManager::PopFinishedEvent(const char *name, AdventureFinishEvent &fe)
{
	list<AdventureFinishEvent>::iterator iter = finished_list.begin();
	while(iter != finished_list.end())
	{
		if((*iter).name.compare(name) == 0)
		{
			fe.name = (*iter).name;
			fe.points = (*iter).points;
			fe.theme = (*iter).theme;
			fe.win = (*iter).win;
			finished_list.erase(iter);
			Save();
			return true;
		}
		iter++;
	}
	return false;
}

void AdventureManager::SendAdventureFinish(AdventureFinishEvent fe)
{
	ClientListEntry *pc = client_list.FindCharacter(fe.name.c_str());
	if(pc)
	{
		ServerPacket *pack = new ServerPacket(ServerOP_AdventureFinish, sizeof(ServerAdventureFinish_Struct));
		ServerAdventureFinish_Struct *af = (ServerAdventureFinish_Struct*)pack->pBuffer;
		strcpy(af->player, fe.name.c_str());
		af->theme = fe.theme;
		af->win = fe.win;
		af->points = fe.points;

		zoneserver_list.SendPacket(pc->zone(), pc->instance(), pack);
		delete pack;
	}
}

void AdventureManager::Save()
{
	stringstream ss(stringstream::in | stringstream::out);

	int number_of_elements = adventure_list.size();
	ss.write((const char*)&number_of_elements, sizeof(int));
	
	char null_term = 0;
	list<Adventure*>::iterator a_iter = adventure_list.begin();
	while(a_iter != adventure_list.end())
	{
		int cur = (*a_iter)->GetCount();
		ss.write((const char*)&cur, sizeof(int));

		cur = (*a_iter)->GetAssassinationCount();
		ss.write((const char*)&cur, sizeof(int));

		cur = (*a_iter)->GetTemplate()->id;
		ss.write((const char*)&cur, sizeof(int));

		cur = (int)(*a_iter)->GetStatus();
		ss.write((const char*)&cur, sizeof(int));

		cur = (*a_iter)->GetInstanceID();
		ss.write((const char*)&cur, sizeof(int));

		cur = (*a_iter)->GetRemainingTime();
		ss.write((const char*)&cur, sizeof(int));

		list<string> players = (*a_iter)->GetPlayers();
		cur = players.size();
		ss.write((const char*)&cur, sizeof(int));

		list<string>::iterator s_iter = players.begin();
		while(s_iter != players.end())
		{
			ss.write((const char*)(*s_iter).c_str(), (*s_iter).size());
			ss.write((const char*)&null_term, sizeof(char));
			s_iter++;
		}

		a_iter++;
	}

	number_of_elements = finished_list.size();
	ss.write((const char*)&number_of_elements, sizeof(int));
	list<AdventureFinishEvent>::iterator f_iter = finished_list.begin();
	while(f_iter != finished_list.end())
	{
		ss.write((const char*)&(*f_iter).win, sizeof(bool));
		ss.write((const char*)&(*f_iter).points, sizeof(int));
		ss.write((const char*)&(*f_iter).theme, sizeof(int));
		ss.write((const char*)(*f_iter).name.c_str(), (*f_iter).name.size());
		ss.write((const char*)&null_term, sizeof(char));
		f_iter++;
	}

	FILE *f = fopen("adventure_state.dat", "w");
	if(f)
	{
		fwrite(ss.str().c_str(), ss.str().size(), 1, f);
		fclose(f);
	}
}

void AdventureManager::Load()
{
	char *data = NULL;
	FILE *f = fopen("adventure_state.dat", "r");
	if(f)
	{
		fseek(f, 0, SEEK_END);
		long length = ftell(f);
		if(length > 0)
		{
			data = new char[length];
			fseek(f, 0, SEEK_SET);
			fread(data, length, 1, f);
		}
		fclose(f);
	}

	if(data)
	{
		char *ptr = data;

		int number_of_adventures = *((int*)ptr);
		ptr += sizeof(int);

		for(int i = 0; i < number_of_adventures; ++i)
		{
			int count = *((int*)ptr);
			ptr += sizeof(int);

			int a_count = *((int*)ptr);
			ptr += sizeof(int);

			int template_id = *((int*)ptr);
			ptr += sizeof(int);

			int status = *((int*)ptr);
			ptr += sizeof(int);

			int instance_id = *((int*)ptr);
			ptr += sizeof(int);

			int rem_time = *((int*)ptr);
			ptr += sizeof(int);

			int num_players = *((int*)ptr);
			ptr += sizeof(int);

			AdventureTemplate *t = GetAdventureTemplate(template_id);
			if(t)
			{
				Adventure *adv = new Adventure(t, count, a_count, (AdventureStatus)status, instance_id, rem_time);
				for(int j = 0; j < num_players; ++j)
				{
					adv->AddPlayer((const char*)ptr, false);
					ptr += strlen((const char*)ptr);
					ptr += 1;
				}
				adventure_list.push_back(adv);
			}
			else
			{
				for(int j = 0; j < num_players; ++j)
				{
					ptr += strlen((const char*)ptr);
					ptr += 1;
				}
			}
		}

		int number_of_finished = *((int*)ptr);
		ptr += sizeof(int);

		for(int k = 0; k < number_of_finished; ++k)
		{
			AdventureFinishEvent afe;
			afe.win = *((bool*)ptr);
			ptr += sizeof(bool);

			afe.points = *((int*)ptr);
			ptr += sizeof(int);

			afe.theme = *((int*)ptr);
			ptr += sizeof(int);

			afe.name = (const char*)ptr;
			ptr += strlen((const char*)ptr);
			ptr += 1;

			finished_list.push_back(afe);
		}

		safe_delete_array(data);
	}
}

