#ifndef GUILD_MGR_H_
#define GUILD_MGR_H_

#include "../common/types.h"
#include "../common/guild_base.h"
#include <map>
#include "../zone/petitions.h"

extern PetitionList petition_list;
//extern GuildRanks_Struct guilds[512];
//extern ZoneDatabase database;

#define PBUFFER 50
#define MBUFFER 50

class Client;
class ServerPacket;

class GuildApproval
{
public:
	GuildApproval(const char* guildname,Client* owner,int32 id);
	~GuildApproval();
	bool	ProcessApproval();
	bool	AddMemberApproval(Client* addition);
	int32	GetID() { return refid; }
	Client*	GetOwner() { return owner; }
	void	GuildApproved();
	void	ApprovedMembers(Client* requestee);
private:
	Timer* deletion_timer;
	char guild[16];
	char founders[3];
	Client* owner;
	Client* members[6];
	int32 refid;
};

class ZoneGuildManager : public BaseGuildManager {
public:
	~ZoneGuildManager(void);

	void	AddGuildApproval(const char* guildname, Client* owner);
	void	AddMemberApproval(int32 refid,Client* name);
	void	ClearGuildsApproval();
	GuildApproval* FindGuildByIDApproval(int32 refid);
	GuildApproval* FindGuildByOwnerApproval(Client* owner);
	void	ProcessApproval();
	int32	GetFreeID() { return id+1; }
	//called by worldserver when it receives a message from world.
	void ProcessWorldPacket(ServerPacket *pack);
	
	void ListGuilds(Client *c) const;
	void DescribeGuild(Client *c, uint32 guild_id) const;
	
	
//	bool	DonateTribute(int32 charid, int32 guild_id, int32 tribute_amount);
	
	uint8 *MakeGuildMembers(int32 guild_id, const char *prefix_name, uint32 &length);	//make a guild member list packet, returns ownership of the buffer.
	
	void RecordInvite(int32 char_id, int32 guild_id, int8 rank);
	bool VerifyAndClearInvite(int32 char_id, int32 guild_id, int8 rank);
	
protected:
	virtual void SendGuildRefresh(int32 guild_id, bool name, bool motd, bool rank, bool relation);
	virtual void SendCharRefresh(int32 old_guild_id, int32 guild_id, int32 charid);
	virtual void SendGuildDelete(int32 guild_id);

	std::map<int32, std::pair<int32, int8> > m_inviteQueue;	//map from char ID to guild,rank

private:
	LinkedList<GuildApproval*> list;
	int32 id;	
};

extern ZoneGuildManager guild_mgr;


#endif /*GUILD_MGR_H_*/

