/*  EQEMu:  Everquest Server Emulator
Copyright (C) 2001-2002  EQEMu Development Team (http://eqemu.org)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
	 This program is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
	  You should have received a copy of the GNU General Public License
	  along with this program; if not, write to the Free Software
	  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "../common/debug.h"
#include "masterentity.h"
#include "NpcAI.h"
#include "../common/packet_functions.h"
#include "../common/packet_dump.h"
#include "worldserver.h"
extern EntityList entity_list;
extern WorldServer worldserver;

//
// Xorlac: This will need proper synchronization to make it work correctly.
//			Also, should investigate client ack for packet to ensure proper synch.
//

/*

note about how groups work:
A group contains 2 list, a list of pointers to members and a 
list of member names. All members of a group should have their
name in the membername array, wether they are in the zone or not.
Only members in this zone will have non-null pointers in the
members array. 

*/

//create a group which should allready exist in the database
Group::Group(int32 gid) 
: GroupIDConsumer(gid)
{
	leader = NULL;
	memset(members,0,sizeof(Mob*) * MAX_GROUP_MEMBERS);
	uint32 i;
	for(i=0;i<MAX_GROUP_MEMBERS;i++)
		memset(membername[i],0,64);	

	if(gid != 0) {
		if(!LearnMembers())
			SetID(0);
	}
}

//creating a new group
Group::Group(Mob* leader)
: GroupIDConsumer()
{
	memset(members, 0, sizeof(members));
	members[0] = leader;
	leader->SetGrouped(true);
	SetLeader(leader);
	uint32 i;
	for(i=0;i<MAX_GROUP_MEMBERS;i++)
		memset(membername[i],0,64);
	strcpy(membername[0],leader->GetName());

#ifdef EQBOTS

	if(!leader->IsBot())

#endif //EQBOTS

	strcpy(leader->CastToClient()->GetPP().groupMembers[0],leader->GetName());
}

//Cofruben:Split money used in OP_Split.
//Rewritten by Father Nitwit
void Group::SplitMoney(uint32 copper, uint32 silver, uint32 gold, uint32 platinum, Client *splitter) {
	//avoid unneeded work
	if(copper == 0 && silver == 0 && gold == 0 && platinum == 0)
		return;
	
  uint32 i;
  int8 membercount = 0;
  for (i = 0; i < MAX_GROUP_MEMBERS; i++) { 
	  if (members[i] != NULL) {

		  membercount++; 
	  } 
  } 

  if (membercount == 0) 
	  return;
  
  uint32 mod;
  //try to handle round off error a little better
  if(membercount > 1) {
	mod = platinum % membercount;
  	if((mod) > 0) {
  		platinum -= mod;
  		gold += 10 * mod;
  	}
	mod = gold % membercount;
  	if((mod) > 0) {
  		gold -= mod;
  		silver += 10 * mod;
  	}
	mod = silver % membercount;
  	if((mod) > 0) {
  		silver -= mod;
  		copper += 10 * mod;
  	}
  }
  
  //calculate the splits
  //We can still round off copper pieces, but I dont care
  uint32 sc;
  uint32 cpsplit = copper / membercount;
  sc = copper   % membercount;
  uint32 spsplit = silver / membercount;
  uint32 gpsplit = gold / membercount;
  uint32 ppsplit = platinum / membercount;

  char buf[128];
  buf[63] = '\0';
  string msg = "You receive";
  bool one = false;
  
  if(ppsplit > 0) {
	 snprintf(buf, 63, " %u platinum", ppsplit);
	 msg += buf;
	 one = true;
  }
  if(gpsplit > 0) {
	 if(one)
	 	msg += ",";
	 snprintf(buf, 63, " %u gold", gpsplit);
	 msg += buf;
	 one = true;
  }
  if(spsplit > 0) {
	 if(one)
	 	msg += ",";
	 snprintf(buf, 63, " %u silver", spsplit);
	 msg += buf;
	 one = true;
  }
  if(cpsplit > 0) {
	 if(one)
	 	msg += ",";
	 //this message is not 100% accurate for the splitter
	 //if they are receiving any roundoff
	 snprintf(buf, 63, " %u copper", cpsplit);
	 msg += buf;
	 one = true;
  }
  msg += " as your split";
  
  for (i = 0; i < MAX_GROUP_MEMBERS; i++) { 
	  if (members[i] != NULL && members[i]->IsClient()) { // If Group Member is Client
	  	Client *c = members[i]->CastToClient();
		//I could not get MoneyOnCorpse to work, so we use this
		c->AddMoneyToPP(cpsplit, spsplit, gpsplit, ppsplit, true);
		  
		c->Message(2, msg.c_str());
	  }
  }
}

bool Group::AddMember(Mob* newmember)
{

#ifdef EQBOTS

	//EQOffline
	if(newmember->IsBot()) {
		int i;
		//Let's see if the bot is already in the group
		for(i=0; i<MAX_GROUP_MEMBERS; i++) {
			if(members[i] && !strcasecmp(members[i]->GetName(),newmember->GetName()))
				return false;
		}

		// Put the bot in the group
		for(i=0; i<MAX_GROUP_MEMBERS; i++) {
			if(members[i] == NULL) {
				members[i] = newmember;
				break;
			}
		}
		
		// We copy the bot name in the group at the slot of the bot
		strcpy(membername[i],newmember->GetName());

		// Let's add the leader in its own list and if newmember is a bot, just add it to the leader group list
		newmember->SetGrouped(true);

		//build the template join packet
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate,sizeof(GroupJoin_Struct));
		GroupJoin_Struct* gj = (GroupJoin_Struct*) outapp->pBuffer;	
		strcpy(gj->membername, newmember->GetName());
		gj->action = 0;
	
		int z=1;
		for(i=0; i<MAX_GROUP_MEMBERS; i++) {
			if(members[i] && members[i]->IsClient()) {
				if(IsLeader(members[i])) {
					strcpy(gj->yourname,members[i]->GetName());
					strcpy(members[i]->CastToClient()->GetPP().groupMembers[0],members[i]->GetName());
					members[i]->CastToClient()->QueuePacket(outapp);
				}
				else {
					strcpy(members[i]->CastToClient()->GetPP().groupMembers[0+z],members[i]->GetName());
					members[i]->CastToClient()->QueuePacket(outapp);
				}
			}
			z++;
		}

		safe_delete(outapp);			
		return true;
	}
	// end of EQoffline

#endif //EQBOTS

	uint32 i=0;
	//see if they are allready in the group
	 for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if(members[i] != NULL && !strcasecmp(members[i]->GetName(),newmember->GetName()))
			return false;
	}
	//put them in the group
	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (members[i] == NULL) {
			members[i] = newmember;
			break;
		}
	}
	if ((i == MAX_GROUP_MEMBERS) || (!newmember->IsClient()))
		return false;
	strcpy(membername[i],newmember->GetName());
	int x=1;
	
	//build the template join packet	
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate,sizeof(GroupJoin_Struct));
	GroupJoin_Struct* gj = (GroupJoin_Struct*) outapp->pBuffer;	
	strcpy(gj->membername, newmember->GetName());
	gj->action = 0;
	
	
	for (i = 0;i < MAX_GROUP_MEMBERS; i++) {
		if (members[i] != NULL && members[i] != newmember) {
			//fill in group join & send it
			strcpy(gj->yourname,members[i]->GetName());

#ifdef EQBOTS

            if(!members[i]->IsBot())

#endif //EQBOTS

			members[i]->CastToClient()->QueuePacket(outapp);
			
#ifdef EQBOTS

            if(!members[i]->IsBot())

#endif //EQBOTS

			//put new member into existing person's list
			strcpy(members[i]->CastToClient()->GetPP().groupMembers[this->GroupCount()-1],newmember->GetName());
			
			//put this existing person into the new member's list
			if(IsLeader(members[i])){

#ifdef EQBOTS

              if(!members[i]->IsBot())

#endif //EQBOTS

				strcpy(newmember->CastToClient()->GetPP().groupMembers[0],members[i]->GetName());
			} else{

#ifdef EQBOTS

                if(!members[i]->IsBot())

#endif //EQBOTS

				strcpy(newmember->CastToClient()->GetPP().groupMembers[x],members[i]->GetName());
				x++;
			}
		}
	}
	

#ifdef EQBOTS

    if(!newmember->IsBot())

#endif //EQBOTS

	//put new member in his own list.
	strcpy(newmember->CastToClient()->GetPP().groupMembers[x],newmember->GetName());
	newmember->SetGrouped(true);
	
	if(newmember->IsClient()) {
		newmember->CastToClient()->Save();
		database.SetGroupID(newmember->GetName(), GetID(), newmember->CastToClient()->CharacterID());
	}
	
	safe_delete(outapp);
	return true;
}

void Group::QueuePacket(const EQApplicationPacket *app, bool ack_req)
{
	uint32 i;
	for(i = 0; i < MAX_GROUP_MEMBERS; i++)
		if(members[i] && members[i]->IsClient())
			members[i]->CastToClient()->QueuePacket(app, ack_req);
}

// solar: sends the rest of the group's hps to member.  this is useful when
// someone first joins a group, but otherwise there shouldn't be a need to
// call it
void Group::SendHPPacketsTo(Mob *member)
{
	EQApplicationPacket hpapp;
	uint32 i;

#ifdef EQBOTS

    // Franck-add
	if(member->IsBot()) {
		member->CreateHPPacket(&hpapp);
		if(GetLeader()->IsClient()) {
			GetLeader()->CastToClient()->QueuePacket(&hpapp, false);
        }
		return;
	}

#endif //EQBOTS

	if(!member || !member->IsClient())
		return;

	for (i = 0; i < MAX_GROUP_MEMBERS; i++)
	{
		if(members[i] && members[i] != member)
		{
			members[i]->CreateHPPacket(&hpapp);

#ifdef EQBOTS

			if(member->IsClient())					// jadams: putting these in, assuming Bot code? No comments...

#endif //EQBOTS

			member->CastToClient()->QueuePacket(&hpapp, false);
		}
	}
}

void Group::SendHPPacketsFrom(Mob *member)
{
	EQApplicationPacket hp_app;
	if(!member)
		return;

 	member->CreateHPPacket(&hp_app);

	uint32 i;
	for(i = 0; i < MAX_GROUP_MEMBERS; i++)

#ifdef EQBOTS

		if(members[i] && (members[i]->GetMaxHP() > 0))

#endif //EQBOTS

		if(members[i] && members[i] != member && members[i]->IsClient())
			members[i]->CastToClient()->QueuePacket(&hp_app);
}

//updates a group member's client pointer when they zone in
//if the group was in the zone allready
bool Group::UpdatePlayer(Mob* update){
	VerifyGroup();
	
	uint32 i=0;
	if(update->IsClient()) {
		//update their player profile
		PlayerProfile_Struct &pp = update->CastToClient()->GetPP();
		for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
			if(membername[0] == '\0')
				memset(pp.groupMembers[i], 0, 64);
			else
				strncpy(pp.groupMembers[i], membername[i], 64);
		}
	}
	
	for (i = 0; i < MAX_GROUP_MEMBERS; i++)
	{
		if (!strcasecmp(membername[i],update->GetName()))
		{
			members[i] = update;
			members[i]->SetGrouped(true);
			return true;
		}
	}
	return false;
}


void Group::MemberZoned(Mob* removemob) {
	uint32 i;

	if (removemob == NULL)
		return;

	if(removemob == GetLeader())
		SetLeader(NULL);

	 for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		  if (members[i] == removemob) {
				members[i] = NULL;
				//should NOT clear the name, it is used for world communication.
				break;
		  }
	 }
}

bool Group::DelMemberOOZ(const char *Name) {

	if(!Name) return false;

	// If a member out of zone has disbanded, clear out their name.
	//
	for(unsigned int i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if(!strcasecmp(Name, membername[i]))
			// This shouldn't be called if the member is in this zone.
			if(!members[i]) {
				memset(membername[i], 0, 64);
				return true;
			}
	}

	return false;
}

bool Group::DelMember(Mob* oldmember,bool ignoresender){
	if (oldmember == NULL){
		return false;
	}

#ifdef EQBOTS

	if(oldmember->IsClient() && (oldmember == GetLeader())) {
		database.CleanBotLeader(oldmember->CastToClient()->CharacterID());
		if(oldmember->IsBotRaiding()) {
			BotRaids* br = entity_list.GetBotRaidByMob(oldmember);
			if(br) {
				br->RemoveRaidBots();
				br = NULL;
			}
		}
		Group *g = entity_list.GetGroupByMob(oldmember);
		if(g) {
			bool hasBots = false;
			for(int i=5; i>=0; i--) {
				if(g->members[i] && g->members[i]->IsBot()) {
					hasBots = true;
					g->members[i]->BotOwner = NULL;
					g->members[i]->Kill();
				}
			}
			if(hasBots) {
				hasBots = false;
				if(g->BotGroupCount() <= 1) {
					g->DisbandGroup();
				}
			}
		}
	}

#endif //EQBOTS

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (members[i] == oldmember) {
			//handle leader quitting group gracefully
			if (oldmember == GetLeader() && GroupCount() > 2) {
				EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate,sizeof(GroupJoin_Struct));
				GroupJoin_Struct* gu = (GroupJoin_Struct*) outapp->pBuffer;
				gu->action = 8;
				for (uint32 nl = 0; nl < MAX_GROUP_MEMBERS; nl++) {
					if (members[nl] && members[nl] != oldmember) {
						strcpy(gu->membername, members[nl]->GetName());
						strcpy(gu->yourname, oldmember->GetName());
						SetLeader(members[nl]);
						database.SetGroupLeaderName(GetID(), members[nl]->GetName());
						for (uint32 ld = 0; ld < MAX_GROUP_MEMBERS; ld++) {
							if (members[ld] && members[ld] != oldmember) {
								members[ld]->CastToClient()->QueuePacket(outapp);
							}
						}
						break;
					}
				}
				
				safe_delete(outapp);
			}
			members[i] = NULL;
			membername[i][0] = '\0';
			memset(membername[i],0,64);
			break;
		  }
	}

	ServerPacket* pack = new ServerPacket(ServerOP_GroupLeave, sizeof(ServerGroupLeave_Struct));
	ServerGroupLeave_Struct* gl = (ServerGroupLeave_Struct*)pack->pBuffer;
	gl->gid = GetID();
	gl->zoneid = zone->GetZoneID();
	strcpy(gl->member_name, oldmember->GetName());
	worldserver.SendPacket(pack);
	safe_delete(pack);

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate,sizeof(GroupJoin_Struct));
	GroupJoin_Struct* gu = (GroupJoin_Struct*) outapp->pBuffer;
	gu->action = groupActLeave;
	strcpy(gu->membername, oldmember->GetName());
	strcpy(gu->yourname, oldmember->GetName());

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (members[i] == NULL) {
			//if (DEBUG>=5) LogFile->write(EQEMuLog::Debug, "Group::DelMember() null member at slot %i", i);
			continue;
		}
		if (members[i] != oldmember && members[i]->IsClient()) {
			strcpy(gu->yourname, members[i]->GetName());
			members[i]->CastToClient()->QueuePacket(outapp);
		}
		#ifdef IPC
		if(members[i] == oldmember && members[i]->IsNPC() && members[i]->CastToNPC()->IsGrouped() && members[i]->CastToNPC()->IsInteractive()) {
			 members[i]->CastToNPC()->TakenAction(23,0);
		}
		#endif	
	}

	if (!ignoresender && oldmember->IsClient()) {
		strcpy(gu->yourname,oldmember->GetName());
		strcpy(gu->membername,oldmember->GetName());
		gu->action = groupActLeave;

		oldmember->CastToClient()->QueuePacket(outapp);
	 }

	database.SetGroupID(oldmember->GetName(), 0, oldmember->CastToClient()->CharacterID());
	
	oldmember->SetGrouped(false);
	disbandcheck = true;

	safe_delete(outapp);

	return true;	
}

// does the caster + group
void Group::CastGroupSpell(Mob* caster, uint16 spell_id) {
	uint32 z;
	float range, distance;

	if(!caster)
		return;

	castspell = true;
	range = caster->GetAOERange(spell_id);
	
	float range2 = range*range;

//	caster->SpellOnTarget(spell_id, caster);

	for(z=0; z < MAX_GROUP_MEMBERS; z++)
	{
		if(members[z] == caster) {
			caster->SpellOnTarget(spell_id, caster);
#ifdef GROUP_BUFF_PETS
			if(caster->GetPet() && caster->GetAA(aaPetAffinity) && !caster->GetPet()->IsCharmed())
				caster->SpellOnTarget(spell_id, caster->GetPet());
#endif
		}
		else if(members[z] != NULL)
		{
			distance = caster->DistNoRoot(*members[z]);
			if(distance <= range2) {
				caster->SpellOnTarget(spell_id, members[z]);
#ifdef GROUP_BUFF_PETS
				if(members[z]->GetPet() && members[z]->GetAA(aaPetAffinity) && !members[z]->GetPet()->IsCharmed())
					caster->SpellOnTarget(spell_id, members[z]->GetPet());
#endif
			} else
				_log(SPELLS__CASTING, "Group spell: %s is out of range %f at distance %f from %s", members[z]->GetName(), range, distance, caster->GetName());
		}
	}

	castspell = false;
	disbandcheck = true;
}

// does the caster + group
void Group::GroupBardPulse(Mob* caster, uint16 spell_id) {
	uint32 z;
	float range, distance;

	if(!caster)
		return;

	castspell = true;
	range = caster->GetAOERange(spell_id);
	
	float range2 = range*range;

	for(z=0; z < MAX_GROUP_MEMBERS; z++) {
		if(members[z] == caster) {
			caster->BardPulse(spell_id, caster);
#ifdef GROUP_BUFF_PETS
			if(caster->GetPet() && caster->GetAA(aaPetAffinity) && !caster->GetPet()->IsCharmed())
				caster->BardPulse(spell_id, caster->GetPet());
#endif
		}
		else if(members[z] != NULL)
		{
			distance = caster->DistNoRoot(*members[z]);
			if(distance <= range2) {
				members[z]->BardPulse(spell_id, caster);
#ifdef GROUP_BUFF_PETS
				if(members[z]->GetPet() && members[z]->GetAA(aaPetAffinity) && !members[z]->GetPet()->IsCharmed())
					members[z]->GetPet()->BardPulse(spell_id, caster);
#endif
			} else
				_log(SPELLS__BARDS, "Group bard pulse: %s is out of range %f at distance %f from %s", members[z]->GetName(), range, distance, caster->GetName());
		}
	}
}

bool Group::IsGroupMember(Mob* client)
{
	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++)
	 {
		if (members[i] == client)
		  {
			return true;
		  }
	}

	return false;
}

void Group::GroupMessage(Mob* sender, int8 language, int8 lang_skill, const char* message) {
	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if(!members[i])
			continue;

		if (members[i]->IsClient() && members[i]->CastToClient()->GetFilter(FILTER_GROUP)!=0)
			members[i]->CastToClient()->ChannelMessageSend(sender->GetName(),members[i]->GetName(),2,language,lang_skill,message);
		#ifdef IPC
		if (members[i]->CastToNPC()->IsInteractive() && members[i] != sender)
			members[i]->CastToNPC()->InteractiveChat(2,1,message,(sender->GetTarget() != NULL) ? sender->GetTarget()->GetName():sender->GetName(),sender);
				//InteractiveChat(int8 chan_num, int8 language, const char * message, const char* targetname,Mob* sender);
  		 #endif
	}

	ServerPacket* pack = new ServerPacket(ServerOP_OOZGroupMessage, sizeof(ServerGroupChannelMessage_Struct) + strlen(message) + 1);
	ServerGroupChannelMessage_Struct* gcm = (ServerGroupChannelMessage_Struct*)pack->pBuffer;
	gcm->zoneid = zone->GetZoneID();
	gcm->groupid = GetID();
	strcpy(gcm->from, sender->GetName());
	strcpy(gcm->message, message);
	worldserver.SendPacket(pack);
	safe_delete(pack);	
}

int32 Group::GetTotalGroupDamage(Mob* other) {
	 int32 total = 0;

	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if(!members[i])
			continue;
		if (other->CheckAggro(members[i]))
			total += other->GetHateAmount(members[i],true);
	}
	return total;
}

void Group::DisbandGroup() {
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate,sizeof(GroupUpdate_Struct));

	GroupUpdate_Struct* gu = (GroupUpdate_Struct*) outapp->pBuffer;
	gu->action = groupActDisband;

	Client *Leader = NULL;

	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (members[i] == NULL) {
			continue;
		}

		if (members[i]->IsClient()) {
			if(IsLeader(members[i]))
				Leader = members[i]->CastToClient();

			strcpy(gu->yourname, members[i]->GetName());
			database.SetGroupID(members[i]->GetName(), 0, members[i]->CastToClient()->CharacterID());
			members[i]->CastToClient()->QueuePacket(outapp);
		}
		members[i]->SetGrouped(false);
		members[i] = NULL;
		membername[i][0] = '\0';
	}
	
	ServerPacket* pack = new ServerPacket(ServerOP_DisbandGroup, sizeof(ServerDisbandGroup_Struct));
	ServerDisbandGroup_Struct* dg = (ServerDisbandGroup_Struct*)pack->pBuffer;
	dg->zoneid = zone->GetZoneID();
	dg->groupid = GetID();
	worldserver.SendPacket(pack);
	safe_delete(pack);	

	entity_list.RemoveGroup(GetID());
	if(GetID() != 0)
		 database.ClearGroup(GetID());

	if(Leader && (Leader->IsLFP())) {
		Leader->UpdateLFP();
	}

	safe_delete(outapp);
}

bool Group::Process() {
	if(disbandcheck && !GroupCount())
		return false;
	else if(disbandcheck && GroupCount())
		disbandcheck = false;
	return true;
}

void Group::SendUpdate(int32 type, Mob* member){
	if(!member->IsClient())
		return;
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate,sizeof(GroupUpdate2_Struct));
	GroupUpdate2_Struct* gu = (GroupUpdate2_Struct*)outapp->pBuffer;	
	gu->action = type;
	strcpy(gu->yourname,member->GetName());
	
	int x=0;
	uint32 i=0;
	for (i = 0;i < MAX_GROUP_MEMBERS; i++) {
		if (members[i] != NULL && members[i] != member) {
			if(IsLeader(members[i])){
				strcpy(gu->leadersname,members[i]->GetName());
				strcpy(gu->membername[x],members[i]->GetName());
				((Client *)members[i])->GetGroupAAs(&gu->leader_aas);
				x++;
			}
			else{
				strcpy(gu->membername[x],members[i]->GetName());
				x++;
			}
		}
	}
	member->CastToClient()->QueuePacket(outapp);
	safe_delete(outapp);
}

int8 Group::GroupCount() {
	return (database.GroupCount(GetID()));
}

#ifdef EQBOTS

int Group::BotGroupCount() {
	int count = 0;
	for(int i=count; i<MAX_GROUP_MEMBERS; i++) {
		if(members[i] && (members[i]->GetMaxHP()>0))
			count++;
	}
	return count;
}

#endif //EQBOTS

int32 Group::GetHighestLevel()
{
int32 level = 1;
uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++)
	 {
		if (members[i])
		  {
			if(members[i]->GetLevel() > level)
				level = members[i]->GetLevel();
		  }
	}
	return level;
}
int32 Group::GetLowestLevel()
{
int32 level = 255;
uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++)
	 {
		if (members[i])
		  {
			if(members[i]->GetLevel() < level)
				level = members[i]->GetLevel();
		  }
	}
	return level;
}

void Group::TeleportGroup(Mob* sender, int32 zoneID, float x, float y, float z, float heading)
{
	uint32 i;
	 for (i = 0; i < MAX_GROUP_MEMBERS; i++)
	 {
	 #ifdef IPC
		if (members[i] != NULL && (members[i]->IsClient() || (members[i]->IsNPC() && members[i]->CastToNPC()->IsInteractive())) && members[i] != sender)
	 #else
		  if (members[i] != NULL && members[i]->IsClient() && members[i] != sender)
	 #endif
	 	{
			members[i]->CastToClient()->MovePC(int(zoneID), x, y, z, heading, 0, ZoneSolicited);
		}
	}	
}

bool Group::LearnMembers() {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	if (database.RunQuery(query,MakeAnyLenString(&query, "SELECT name FROM group_id WHERE groupid=%lu", GetID()),errbuf,&result)){
		safe_delete_array(query);
		if(mysql_num_rows(result) < 1) {	//could prolly be 2
			mysql_free_result(result);
			LogFile->write(EQEMuLog::Error, "Error getting group members for group %lu: %s", GetID(), errbuf);
			return(false);
		}
		int i = 0;
		while((row = mysql_fetch_row(result))) {
			if(!row[0])
				continue;
			members[i] = NULL;
			strncpy(membername[i], row[0], 64);
			i++;
		}
		mysql_free_result(result);
	}
	return(true);
}

void Group::VerifyGroup() {
	/*
		The purpose of this method is to make sure that a group
		is in a valid state, to prevent dangling pointers.
		Only called every once in a while (on member re-join for now).
	*/

	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (membername[i][0] == '\0') {
#if EQDEBUG >= 7
LogFile->write(EQEMuLog::Debug, "Group %lu: Verify %d: Empty.\n", id, i);
#endif
			members[i] = NULL;
			continue;
		}
		
		//it should be safe to use GetClientByName, but Group is trying
		//to be generic, so we'll go for general Mob
		Mob *them = entity_list.GetMob(membername[i]);
		if(them == NULL && members[i] != NULL) {	//they arnt here anymore....
#if EQDEBUG >= 6
		LogFile->write(EQEMuLog::Debug, "Member of group %lu named '%s' has disappeared!!", GetID(), membername[i]);
#endif
			membername[i][0] = '\0';
			members[i] = NULL;
			continue;
		}
		
		if(them != NULL && members[i] != them) {	//our pointer is out of date... not so good.
#if EQDEBUG >= 5
		LogFile->write(EQEMuLog::Debug, "Member of group %lu named '%s' had an out of date pointer!!", GetID(), membername[i]);
#endif
			members[i] = them;
			continue;
		}
#if EQDEBUG >= 8
		LogFile->write(EQEMuLog::Debug, "Member of group %lu named '%s' is valid.", GetID(), membername[i]);
#endif
	}
}


void Group::GroupMessage_StringID(Mob* sender, int32 type, int32 string_id, const char* message,const char* message2,const char* message3,const char* message4,const char* message5,const char* message6,const char* message7,const char* message8,const char* message9, int32 distance) {
	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if(members[i] == NULL)
			continue;
		
		if(members[i] == sender)
			continue;
		
		members[i]->Message_StringID(type, string_id, message, message2, message3, message4, message5, message6, message7, message8, message9, 0);
	}
}



void Client::LeaveGroup() {

#ifdef EQBOTS

	Mob *clientmob = CastToMob();
	database.CleanBotLeader(CharacterID());
	if(clientmob) {
		if(clientmob->IsBotRaiding()) {
			BotRaids* br = entity_list.GetBotRaidByMob(clientmob);
			if(br) {
				br->RemoveRaidBots();
				br = NULL;
			}
		}
		Group *g = entity_list.GetGroupByMob(clientmob);
		if(g) {
			bool hasBots = false;
			for(int i=5; i>=0; i--) {
				if(g->members[i] && g->members[i]->IsBot()) {
					hasBots = true;
					g->members[i]->BotOwner = NULL;
					g->members[i]->Kill();
				}
			}
			if(hasBots) {
				hasBots = false;
				if(g->BotGroupCount() <= 1) {
					g->DisbandGroup();
				}
			}
		}
	}

#endif //EQBOTS

	Group *g = GetGroup();
	
	if(g) {
		if(g->GroupCount() < 3)
			g->DisbandGroup();
		else
			g->DelMember(this);
	} else {
		//force things a little
		database.SetGroupID(GetName(), 0, CharacterID());
	}
	
	isgrouped = false;
}

void Group::BalanceHP(sint32 penalty)
{
	int dmgtaken = 0, numMem = 0;
	unsigned int gi = 0;
	for(; gi < MAX_GROUP_MEMBERS; gi++)
	{
		if(members[gi]){
			dmgtaken += (members[gi]->GetMaxHP() - members[gi]->GetHP());
			numMem += 1;
		}
	}

	dmgtaken += dmgtaken * penalty / 100;
	dmgtaken /= numMem;
	for(gi = 0; gi < MAX_GROUP_MEMBERS; gi++)
	{
		if(members[gi]){
			if((members[gi]->GetMaxHP() - dmgtaken) < 1){ //this way the ability will never kill someone
				members[gi]->SetHP(1);					 //but it will come darn close
				members[gi]->SendHPUpdate();
			}
			else{
				members[gi]->SetHP(members[gi]->GetMaxHP() - dmgtaken);
				members[gi]->SendHPUpdate();
			}
		}
	}
}

uint16 Group::GetAvgLevel()
{
	double levelHolder = 0;
	uint8 i = 0;
	uint8 numMem = 0;
	while(i < MAX_GROUP_MEMBERS)
	{
		if (members[i])
		{
			numMem++;
			levelHolder = levelHolder + (members[i]->GetLevel());
		}
		i++;
	}
	levelHolder = ((levelHolder/numMem)+.5); // total levels divided by num of characters
	return (uint16(levelHolder));
}

