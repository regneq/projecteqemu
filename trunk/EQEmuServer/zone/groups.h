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
#ifndef GROUPS_H
#define GROUPS_H

#include "../common/types.h"
#include "../common/linked_list.h"
#include "../common/emu_opcodes.h"
#include "../common/eq_packet_structs.h"
#include "entity.h"
#include "mob.h"
#include "features.h"
#include "../common/servertalk.h"

enum {	//Group  action fields
	groupActJoin = 0,
	groupActLeave = 1,
	groupActDisband = 6,
	groupActUpdate = 7,
	groupActMakeLeader = 8,
	groupActInviteInitial = 9
};

class GroupIDConsumer {
public:
	GroupIDConsumer() { id = 0; }
	GroupIDConsumer(uint32 gid) { id = gid; }
	inline const int32 GetID()	const { return id; }
	
protected:
	friend class EntityList;
	//use of this function is highly discouraged
	inline void SetID(int32 set_id) { id = set_id; }
private:
	int32 id;
};

class Group : public GroupIDConsumer {
public:
	Group(Mob* leader);
	Group(int32 gid);
	~Group() {}
	
	bool	AddMember(Mob* newmember);
	void	SendUpdate(int32 type,Mob* member);
	void	SendWorldGroup(int32 zone_id,Mob* zoningmember);
	bool	DelMember(Mob* oldmember,bool ignoresender = false);
	void	DisbandGroup();
	bool	IsGroupMember(Mob* client);
	bool	Process();
	bool	IsGroup()			{ return true; }
	void	CastGroupSpell(Mob* caster,uint16 spellid);
	void	GroupBardPulse(Mob* caster,uint16 spellid);
	void	SplitExp(uint32 exp, Mob* other);
	void	GroupMessage(Mob* sender,const char* message);
	void	GroupMessage_StringID(Mob* sender, int32 type, int32 string_id, const char* message,const char* message2=0,const char* message3=0,const char* message4=0,const char* message5=0,const char* message6=0,const char* message7=0,const char* message8=0,const char* message9=0, int32 distance = 0);
	int32	GetTotalGroupDamage(Mob* other);
	void	SplitMoney(uint32 copper, uint32 silver, uint32 gold, uint32 platinum, Client *splitter = NULL);
	void	SetLeader(Mob* newleader){ leader=newleader; };
	Mob*	GetLeader(){ return leader; };
	char*	GetLeaderName(){ return membername[0]; };
	void	SendHPPacketsTo(Mob* newmember);
	void	SendHPPacketsFrom(Mob* newmember);
	bool	UpdatePlayer(Mob* update);
	void	MemberZoned(Mob* removemob);
	bool	IsLeader(Mob* leadertest) { return leadertest==leader; };
	int8	GroupCount();

#ifdef EQBOTS

	// EQoffline
	int		BotGroupCount();

#endif //EQBOTS

	int32	GetHighestLevel();
	int32	GetLowestLevel();
	void	QueuePacket(const EQApplicationPacket *app, bool ack_req = true);
	void	TeleportGroup(Mob* sender, int32 zoneID, float x, float y, float z, float heading);
      uint16	GetAvgLevel();
	bool	LearnMembers();
	void	VerifyGroup();
	void	BalanceHP(sint32 penalty);
	
#ifdef ENABLE_GROUP_LINKING
	//linking methods
	void	ClearLink(int32 clear_id, bool all = false);
	bool	IsLinked(int32 link_id);
	void	EstablishLink(int32 link_id);
#endif
	
	Mob* members[MAX_GROUP_MEMBERS];
	char	membername[MAX_GROUP_MEMBERS][64];
	bool	disbandcheck;
	bool	castspell;

#ifdef GUILDWARS
	void	CauseEXPLoss();
	void	CauseFactionLoss(Mob* killed);
	void	GivePoints(Client* killed);
#endif

#ifdef RAIDADDICTS
	void	RASplitPointsAndEXP(uint32 exp, Mob* other);
#endif

private:
	Mob*	leader;
#ifdef ENABLE_GROUP_LINKING
	int32	link[MAX_GROUP_LINKS];
#endif
};

#endif
