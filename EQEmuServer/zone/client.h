/*  EQEMu:  Everquest Server Emulator
	Copyright (C) 2001-2003  EQEMu Development Team (http://eqemulator.net)

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
#ifndef CLIENT_H
#define CLIENT_H
class Client;

#include "../common/timer.h"
#include "../common/ptimer.h"
#include "../common/emu_opcodes.h"
#include "../common/eq_packet_structs.h"
#include "../common/eq_constants.h"
#include "../common/EQStreamIntf.h"
#include "../common/EQPacket.h"
#include "../common/linked_list.h"
#include "../common/extprofile.h"
#include "zonedb.h"
#include "errno.h"
#include "../common/classes.h"
#include "../common/races.h"
#include "../common/deity.h"
#include "mob.h"
#include "npc.h"
#include "zone.h"
#include "AA.h"
#include "../common/seperator.h"
#include "../common/Item.h"
#include "updatemgr.h"
#include "../common/guilds.h"
#include <float.h>
#include <set>

#define CLIENT_TIMEOUT		90000
#define CLIENT_LD_TIMEOUT	30000 // length of time client stays in zone after LDing
#define TARGETING_RANGE		200	// range for /assist and /target
extern Zone* zone;
extern TaskManager *taskmanager;

class CLIENTPACKET
{
public:
    CLIENTPACKET();
    ~CLIENTPACKET();
    EQApplicationPacket *app;
    bool ack_req;
};

enum {	//Type arguments to the Message* routines.
	//all not explicitly listed are the same grey color
	clientMessageWhite0 = 0,
	clientMessageLoot = 2,	//dark green
	clientMessageTradeskill = 4,	//light blue
	clientMessageTell = 5,		//magenta
	clientMessageWhite = 7,
	clientMessageWhite2 = 10,
	clientMessageLightGrey = 12,
	clientMessageError = 13,	//red
	clientMessageGreen = 14,
	clientMessageYellow = 15,
	clientMessageBlue = 16,
	clientMessageGroup = 18,	//cyan
	clientMessageWhite3 = 20,
};

typedef enum {	//focus types
	focusSpellHaste = 1,
	focusSpellDuration,
	focusRange,
	focusReagentCost,
	focusManaCost,
	focusImprovedHeal,
	focusImprovedDamage,
	focusImprovedDOT,		//i dont know about this...
	focusImprovedCritical,
	focusImprovedUndeadDamage
} focusType;

#define SPELLBAR_UNLOCK 0x2bc
enum {	//scribing argument to MemorizeSpell
	memSpellScribing = 0,
	memSpellMemorize = 1,
	memSpellForget = 2,
	memSpellSpellbar = 3
};

#define USE_ITEM_SPELL_SLOT 10
#define DISCIPLINE_SPELL_SLOT 10
#define ABILITY_SPELL_SLOT 9

// this enum and discipline system is obsolete,
//all this stuff should be handled by spells/bonuses now.
typedef enum {	//disciplines for disc_inuse
	discNone			= 0,
	//general
	discResistant		= 30,
	discFearless		= 31,
	discWhirlwind		= 6,	//Counterattack/Whirlwind/Furious
	discFellstrike		= 14,	//Duelist/Innerflame/Fellstrike
	discBlindingSpeed	= 15,	//Blindingspeed/Hundredfist		haste unknown
	discDeadeye			= 16,	//Deadeye/Charge
	//warrior
	discEvasive			= 4,
	discMightystrike	= 17,
	discDefensive		= 3,	//values unknown
	discPrecise			= 2,	//values unknown
	discAggressive		= 1,	//values unknown
	//monk
	discStonestance		= 11,
	discThunderkick		= 12,	//values unknown
	discVoidance		= 13,
	discSilentfist		= 20,	//values unknown
	discAshenhand		= 5,	//values unknown
	//rogue
	discNimble			= 19,
	discKinesthetics	= 21,
	//paladin
	discHolyforge		= 22,
	discSanctification	= 23,	//not sure of exact effect
	//ranger
	discTrueshot		= 24,
	discWeaponshield	= 25,
	//bard
	discDeftdance		= 28,
	discPuretone		= 29,	//exact value unknown
	//SK
	discUnholyAura		= 26,	//dosent make un-resistable yet
	discLeechCurse		= 27
};

//Modes for the zoning state of the client.
typedef enum {
	ZoneToSafeCoords,		// Always send ZonePlayerToBind_Struct to client: Succor/Evac
	GMSummon,				// Always send ZonePlayerToBind_Struct to client: Only a GM Summon
	ZoneToBindPoint,		// Always send ZonePlayerToBind_Struct to client: Death Only
	ZoneSolicited,			// Always send ZonePlayerToBind_Struct to client: Portal, Translocate, Evac spells that have a x y z coord in the spell data
	ZoneUnsolicited,
	GateToBindPoint,		// Always send RequestClientZoneChange_Struct to client: Gate spell or Translocate To Bind Point spell
	SummonPC,				// In-zone GMMove() always: Call of the Hero spell or some other type of in zone only summons
	Rewind,					// Lieka:  Summon to /rewind location.
	EvacToSafeCoords
} ZoneMode;

typedef enum {
	MQWarp,
	MQZone,
	MQGate,
	MQGhost
} CheatTypes;

class ClientFactory {
public:
	Client *MakeClient(EQStream* ieqs);
};

class Client : public Mob
{
public:
	//pull in opcode mappings:
	#include "client_packet.h"

	Client(EQStreamInterface * ieqs);
    ~Client();

//	void	Discipline(ClientDiscipline_Struct* disc_in, Mob* tar);
	void	AI_Init();
	void	AI_Start(int32 iMoveDelay = 0);
	void	AI_Stop();
	void	Trader_ShowItems();
	void	Trader_EndTrader();
	void	Trader_StartTrader();
	int8	WithCustomer();
	bool	CheckCheat();
	void	CheatDetected(CheatTypes Cheat);
	bool	WarpDetection(bool CTimer, float Distance);
	void KeyRingLoad();
	void KeyRingAdd(int32 item_id);
	bool KeyRingCheck(int32 item_id);
    void KeyRingList();
	virtual bool IsClient() const { return true; }
	virtual void DBAWComplete(int8 workpt_b1, DBAsyncWork* dbaw);
	bool	FinishConnState2(DBAsyncWork* dbaw);
	void	CompleteConnect();
	bool	TryStacking(ItemInst* item, int8 type = ItemPacketTrade, bool try_worn = true, bool try_cursor = true);
	void	SendTraderPacket(Client* trader);
	GetItems_Struct* GetTraderItems();
	void	SendBazaarWelcome();
	void	DyeArmor(DyeStruct* dye);
	int8	SlotConvert(int8 slot,bool bracer=false);
	void	Message_StringID(int32 type, int32 string_id, int32 distance = 0);
	void	Message_StringID(int32 type, int32 string_id, const char* message,const char* message2=0,const char* message3=0,const char* message4=0,const char* message5=0,const char* message6=0,const char* message7=0,const char* message8=0,const char* message9=0, int32 distance = 0);
	void	SendBazaarResults(int32 trader_id,int32 class_,int32 race,int32 stat,int32 slot,int32 type,char name[64],int32 minprice,int32 maxprice);
	void	SendTraderItem(int32 item_id,int16 quantity);
	int16	FindTraderItemCharges(int32 item_id);
	int16	FindTraderItem(int32 item_id,int16 quantity);
	void	FindAndNukeTraderItem(int32 item_id,int16 quantity,Client* customer,int16 traderslot);
	void	NukeTraderItem(int16 slot,int16 charges,int16 quantity,Client* customer,int16 traderslot);
	void	ReturnTraderReq(const EQApplicationPacket* app,int16 traderitemcharges);
	void	BuyTraderItem(TraderBuy_Struct* tbs,Client* trader,const EQApplicationPacket* app);
	void	TraderUpdate(int16 slot_id,int32 trader_id);
	void	FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho);
	virtual bool Process();
	void	LogMerchant(Client* player, Mob* merchant, Merchant_Sell_Struct* mp, const Item_Struct* item, bool buying);
	void	LogMerchant(Client* player, Mob* merchant, Merchant_Purchase_Struct* mp, const Item_Struct* item, bool buying);
	void	SendPacketQueue(bool Block = true);
	void	QueuePacket(const EQApplicationPacket* app, bool ack_req = true, CLIENT_CONN_STATUS = CLIENT_CONNECTINGALL, eqFilterType filter=FilterNone);
	void	FastQueuePacket(EQApplicationPacket** app, bool ack_req = true, CLIENT_CONN_STATUS = CLIENT_CONNECTINGALL);
	void	ChannelMessageReceived(int8 chan_num, int8 language, const char* message, const char* targetname=NULL);
	void	ChannelMessageSend(const char* from, const char* to, int8 chan_num, int8 language, const char* message, ...);
	void	Message(int32 type, const char* message, ...);
	void    QuestJournalledMessage(const char *npcname, const char* message);
	void	SendSound();

	int32   GetAdventureID() const {return 0/*m_pp.adventure_id*/; }
	void    SetAdventureID(int32 i){ /*m_pp.adventure_id=i;*/ }
	void	SendAdventureFinish(uint32 state=0,uint32 points=0,bool grouptoo=false);
	void	SendAdventureInfoRequest(const EQApplicationPacket* app);
	void	SendAdventureUpdate();
	void	SendAdventureRequestData(Group* group = NULL,bool EnteredDungeon=false,bool EnteredZone=false,bool Zoned=false);
	void	SendAdventureRequest();
	void	DeleteCharInAdventure(int32 id,int32 qid);

	EQApplicationPacket*	ReturnItemPacket(sint16 slot_id, const ItemInst* inst, ItemPacketType packet_type);

	bool	GetRevoked() const { return revoked; }
	void	SetRevoked(bool rev) { revoked = rev; }
	inline int32	GetIP()			const { return ip; }
	inline bool	GetHideMe()			const { return gmhideme; }
	void	SetHideMe(bool hm);
	inline int16	GetPort()		const { return port; }
	bool	IsDead() const { return(dead); }

	virtual bool	Save() { return Save(0); }
			bool	Save(int8 iCommitNow); // 0 = delayed, 1=async now, 2=sync now
			void	SaveBackup();

	inline bool ClientDataLoaded() const { return client_data_loaded; }
	inline bool	Connected()		const { return (client_state == CLIENT_CONNECTED); }
	inline bool	InZone()		const { return (client_state == CLIENT_CONNECTED || client_state == CLIENT_LINKDEAD); }
	inline void	Kick()			{ client_state = CLIENT_KICKED; }
	inline void	Disconnect()	{ eqs->Close(); client_state = DISCONNECTED; }
	inline bool IsLD()			const { return (bool) (client_state == CLIENT_LINKDEAD); }
	void	WorldKick();
	inline int8	GetAnon()		const {  return m_pp.anon; }
	inline PlayerProfile_Struct& GetPP()	{ return m_pp; }
	inline ExtendedProfile_Struct& GetEPP()	{ return m_epp; }
	inline Inventory& GetInv()				{ return m_inv; }
	inline const Inventory& GetInv() const	{ return m_inv; }
	bool	CheckAccess(sint16 iDBLevel, sint16 iDefaultLevel);

	void CheckQuests(const char* zonename, const char* message, uint32 npc_id, uint32 item_id, Mob* other);
	void LogLoot(Client* player,Corpse* corpse,const Item_Struct* item);
	bool	AutoAttackEnabled() const { return auto_attack; }
	bool	AutoFireEnabled() const { return auto_fire; }
	bool	Attack(Mob* other, int Hand = 13, bool bRiposte = false);	// 13 = Primary (default), 14 = secondary
	void	Damage(Mob* other, sint32 damage, int16 spell_id, SkillType attack_skill, bool avoidable = true, sint8 buffslot = -1, bool iBuffTic = false);
	void	Death(Mob* other, sint32 damage, int16 spell_id, SkillType attack_skill);
	void	MakeCorpse(int32 exploss);

	bool	ChangeFirstName(const char* in_firstname,const char* gmname);

	void	Duck();
	void	Stand();

	virtual void	SetMaxHP();
	sint32	LevelRegen();
	void	HPTick();
	void	SetGM(bool toggle);
	void	SetPVP(bool toggle);

	inline bool	GetPVP()	const { return zone->GetZoneID() == 77 ? true : m_pp.pvp; }
	inline bool	GetGM()		const { return (bool) m_pp.gm; }

	inline void	SetBaseClass(uint32 i) { m_pp.class_=i; }
	inline void	SetBaseRace(uint32 i) { m_pp.race=i; }
	inline void	SetBaseGender(uint32 i) { m_pp.gender=i; }
	inline  void SetDeity(uint32 i) {m_pp.deity=i;}

	inline int16	GetBaseRace()	const { return m_pp.race; }
	inline int16	GetBaseClass()	const { return m_pp.class_; }
	inline int8	GetBaseGender()	const { return m_pp.gender; }
	inline int8	GetBaseFace()	const { return m_pp.face; }
	sint32	CalcMaxMana();
	const sint32&	SetMana(sint32 amount);

	void	ServerFilter(SetServerFilter_Struct* filter);
	void	BulkSendTraderInventory(int32 char_id);
	void	BulkSendMerchantInventory(int merchant_id, int npcid);

	inline int8	GetLanguageSkill(int16 n)	const { return m_pp.languages[n]; }

	void	SendPickPocketResponce(Mob *from, uint32 amt, int type, const Item_Struct* item = NULL);

	inline const char*	GetLastName() const	{ return lastname; }
	inline int32	GetLDoNPoints() { return 0; }

	inline float ProximityX() const { return(proximity_x); }
	inline float ProximityY() const { return(proximity_y); }
	inline float ProximityZ() const { return(proximity_z); }
	inline void ClearAllProximities() { entity_list.ProcessMove(this, FLT_MAX, FLT_MAX, FLT_MAX); proximity_x = FLT_MAX; proximity_y = FLT_MAX; proximity_z = FLT_MAX; }

	/*
		Begin client modifiers
	*/

	virtual void CalcBonuses();
	//these are all precalculated now
	inline virtual sint16	GetAC()		const { return AC; }
	inline virtual sint16	GetATK()	const { return ATK; }
	inline virtual int	GetHaste() const { return Haste; }

	inline virtual sint16	GetSTR()	const { return STR; }
	inline virtual sint16	GetSTA()	const { return STA; }
	inline virtual sint16	GetDEX()	const { return DEX; }
	inline virtual sint16	GetAGI()	const { return AGI; }
	inline virtual sint16	GetINT()	const { return INT; }
	inline virtual sint16	GetWIS()	const { return WIS; }
	inline virtual sint16	GetCHA()	const { return CHA; }
	inline virtual sint16	GetMR() const { return MR; }
	inline virtual sint16	GetFR()	const { return FR; }
	inline virtual sint16	GetDR()	const { return DR; }
	inline virtual sint16	GetPR()	const { return PR; }
	inline virtual sint16	GetCR() const { return CR; }

	sint16	GetMaxStat() const;
	sint16  GetMaxSTR() const;
    sint16  GetMaxSTA() const;
    sint16  GetMaxDEX() const;
    sint16  GetMaxAGI() const;
    sint16  GetMaxINT() const;
    sint16  GetMaxWIS() const;
    sint16  GetMaxCHA() const;
	sint16  GetMaxMR() const;
	sint16  GetMaxPR() const;
	sint16  GetMaxDR() const;
	sint16  GetMaxCR() const;
	sint16  GetMaxFR() const;
	inline int8	GetBaseSTR()	const { return m_pp.STR; }
	inline int8	GetBaseSTA()	const { return m_pp.STA; }
	inline int8	GetBaseCHA()	const { return m_pp.CHA; }
	inline int8	GetBaseDEX()	const { return m_pp.DEX; }
	inline int8	GetBaseINT()	const { return m_pp.INT; }
	inline int8	GetBaseAGI()	const { return m_pp.AGI; }
	inline int8	GetBaseWIS()	const { return m_pp.WIS; }

	float  GetActSpellRange(int16 spell_id, float range);
	sint32  GetActSpellDamage(int16 spell_id, sint32 value);
	sint32  GetActSpellHealing(int16 spell_id, sint32 value);
	sint32  GetActSpellCost(int16 spell_id, sint32);
	sint32  GetActSpellDuration(int16 spell_id, sint32);
	sint32  GetActSpellCasttime(int16 spell_id, sint32);
	sint32  GetDotFocus(int16 spell_id, sint32 value);
	virtual bool CheckFizzle(int16 spell_id);

	inline const sint32	GetBaseHP() const { return base_hp; }

	int16 GetWeight() const { return(weight); }
	inline void RecalcWeight() { weight = CalcCurrentWeight(); }
	int16 CalcCurrentWeight();
	inline uint32	GetCopper()		const { return m_pp.copper; }
	inline uint32	GetSilver()		const { return m_pp.silver; }
	inline uint32	GetGold()		const { return m_pp.gold; }
	inline uint32	GetPlatinum()	const { return m_pp.platinum; }


	/*Endurance and such*/
	//This calculates the maximum endurance we can have
	void	CalcMaxEndurance();
	//This gets our current endurance
	sint32	GetEndurance()	const {return cur_end;}
	//This gets our endurance from the last CalcMaxEndurance() call
	sint32	GetMaxEndurance() const {return max_end;}
	//This sets the current endurance to the new value
	void SetEndurance(sint32 newEnd);
	//This Regenerates endurance
	void DoEnduranceRegen();
	//does the endurance upkeep
	void DoEnduranceUpkeep();


    bool Flurry();
    bool Rampage();

	inline uint32	GetEXP()		const { return m_pp.exp; }


	bool	UpdateLDoNPoints(sint32 points, int32 theme);

	void	AddEXP(uint32 add_exp, int8 conlevel = 0xFF, bool resexp = false);
	void	SetEXP(uint32 set_exp, uint32 set_aaxp, bool resexp=false);
	void	SetLeadershipEXP(uint32 group_exp, uint32 raid_exp);
	void	AddLeadershipEXP(uint32 group_exp, uint32 raid_exp);
	void	SendLeadershipEXPUpdate();
	uint32  GetRaidPoints() { return(m_pp.raid_leadership_points); }
	uint32  GetGroupPoints() { return(m_pp.group_leadership_points); }
	uint32  GetRaidEXP() { return(m_pp.raid_leadership_exp); }
	uint32  GetGroupEXP() { return(m_pp.group_leadership_exp); }
	virtual void SetLevel(uint8 set_level, bool command = false);
	void	SendLevelAppearance();
	void	GoToBind();
	void	GoToSafeCoords(uint16 zone_id);
	void	Gate();
	void	SetBindPoint(int to_zone = -1, float new_x = 0.0f, float new_y = 0.0f, float new_z = 0.0f);
	void	MovePC(const char* zonename, float x, float y, float z, float heading, int8 ignorerestrictions = 0, ZoneMode zm = ZoneSolicited);
	void	MovePC(int32 zoneID, float x, float y, float z, float heading, int8 ignorerestrictions = 0, ZoneMode zm = ZoneSolicited);
	void	MovePC(float x, float y, float z, float heading, int8 ignorerestrictions = 0, ZoneMode zm = ZoneSolicited);
	void	WhoAll();
	bool	CheckLoreConflict(const Item_Struct* item);
	void	ChangeLastName(const char* in_lastname);
	void	GetGroupAAs(GroupLeadershipAA_Struct *into) const;
      void	SacrificeConfirm(Client* caster);
	void	Sacrifice(Client* caster);
	void	GoToDeath();

	FACTION_VALUE	GetReverseFactionCon(Mob* iOther);
    FACTION_VALUE   GetFactionLevel(int32 char_id, int32 npc_id, int32 p_race, int32 p_class, int32 p_deity, sint32 pFaction, Mob* tnpc);
	sint32	GetCharacterFactionLevel(sint32 faction_id);

	void	SetFactionLevel(int32 char_id, int32 npc_id, int8 char_class, int8 char_race, int8 char_deity);
	void    SetFactionLevel2(int32 char_id, sint32 faction_id, int8 char_class, int8 char_race, int8 char_deity, sint32 value);
	sint16	GetRawItemAC();
	int16	GetCombinedAC_TEST();

	inline int32	LSAccountID()	const { return lsaccountid; }
	inline int32	GetWID()		const { return WID; }
	inline void		SetWID(int32 iWID) { WID = iWID; }
	inline int32	AccountID()		const { return account_id; }
	inline const char* AccountName()const { return account_name; }
	inline sint16	Admin()			const { return admin; }
	inline int32	CharacterID()	const { return character_id; }
	void	UpdateAdmin(bool iFromDB = true);
	void	UpdateWho(int8 remove = 0);
	bool	GMHideMe(Client* client = 0);

	inline bool IsInAGuild() const { return(guild_id != GUILD_NONE && guild_id != 0); }
	inline bool IsInGuild(uint32 in_gid) const { return(in_gid == guild_id && IsInAGuild()); }
//	inline int32	GuildEQID()		{ return guildeqid; }
//	inline int32	GuildDBID()		{ return guilddbid; }
	inline int32	GuildID() const { return guild_id; }
	inline int8	GuildRank()		const { return guildrank; }
//	bool	SetGuild(int32 in_guilddbid, int8 in_rank);
//	void	GuildChangeRank(int32 guild_id,int32 oldrank,int32 newrank);
//	void	GuildChangeRank(const char* name,int32 guild_id,int32 oldrank,int32 newrank);
	void	SendGuildMOTD();
	void	SendGuildSpawnAppearance();
	void	SendGuildMembers();
    void	SendGuildList();
	void	SendGuildJoin(GuildJoin_Struct* gj);
	void	RefreshGuildInfo();


	void	SendManaUpdatePacket();
    // Disgrace: currently set from database.CreateCharacter.
	// Need to store in proper position in PlayerProfile...
	int8	GetFace()		const { return m_pp.face; }
	int32	PendingGuildInvite; // Used for /guildinvite
	void	WhoAll(Who_All_Struct* whom);
      void	FriendsWho(char *FriendsString);

	void	Stun(int duration);
	void	UnStun();
	void	ReadBook(BookRequest_Struct *book);
	void	SendClientMoneyUpdate(int8 type,int32 amount);
	void	SendMoneyUpdate();
	bool	TakeMoneyFromPP(uint32 copper);
	void	AddMoneyToPP(uint32 copper,bool updateclient);
	void	AddMoneyToPP(uint32 copper, uint32 silver, uint32 gold,uint32 platinum,bool updateclient);

//	bool	SimpleCheckIncreaseSkill(int16 skillid,sint16 chancemodi = 0);
	void	FinishTrade(Client* with);
	void	FinishTrade(NPC* with);
	bool	TGB() const { return tgb; }

	void	OnDisconnect(bool hard_disconnect);


	int16	GetSkillPoints() {return m_pp.points;}
	void	SetSkillPoints(int inp) {m_pp.points = inp;}

	void	IncreaseSkill(int skill_id, int value = 1) { if (skill_id <= HIGHEST_SKILL) { m_pp.skills[skill_id] += value; } }
	void	IncreaseLanguageSkill(int skill_id, int value = 1) { if (skill_id < 26) { m_pp.languages[skill_id] += value; } }
	virtual uint16 GetSkill(SkillType skill_id) const { if (skill_id <= HIGHEST_SKILL) { return((itembonuses.skillmod[skill_id] > 0)? m_pp.skills[skill_id]*(100 + itembonuses.skillmod[skill_id])/100 : m_pp.skills[skill_id]); } return 0; }
	uint32		GetRawSkill(SkillType skill_id) const { if (skill_id <= HIGHEST_SKILL) { return(m_pp.skills[skill_id]); } return 0; }
	bool HasSkill(SkillType skill_id) const;
	bool CanHaveSkill(SkillType skill_id) const;
	void SetSkill(SkillType skill_num, int16 value); // socket 12-29-01
	void	AddSkill(SkillType skillid, int16 value);
	void CheckSpecializeIncrease(int16 spell_id);
	void CheckSongSkillIncrease(int16 spell_id);
	bool	CheckIncreaseSkill(SkillType skillid, int chancemodi = 0);
	void    SetLanguageSkill(int langid, int value);
	void	SetHoTT(int32 mobid);

	int16	MaxSkill(SkillType skillid, int16 class_, int16 level) const;
    inline	int16	MaxSkill(SkillType skillid) const { return MaxSkill(skillid, GetClass(), GetLevel()); }
    int8	SkillTrainLevel(SkillType skillid, int16 class_);
	// Util functions for MaxSkill
/*
	int16	MaxSkill_weapon(int16 skillid, int16 class_, int16 level) const;
    int16	MaxSkill_offensive(int16 skillid, int16 class_, int16 level) const;
    int16	MaxSkill_defensive(int16 skillid, int16 class_, int16 level) const;
    int16	MaxSkill_arcane(int16 skillid, int16 class_, int16 level) const;
    int16	MaxSkill_class(int16 skillid, int16 class_, int16 level) const;
*/



	void TradeskillSearchResults(const char *query, unsigned long qlen, unsigned long objtype, unsigned long someid);
	void SendTradeskillDetails(unsigned long  recipe_id);
	bool TradeskillExecute(DBTradeskillRecipe_Struct *spec);
	void CheckIncreaseTradeskill(sint16 bonusstat, sint16 stat_modifier, float skillup_modifier, uint16 success_modifier, SkillType tradeskill);

	int32	pendingrezzexp;
	void	GMKill();
	inline bool	IsMedding()	const {return medding;}
	inline int16	GetDuelTarget() const { return duel_target; }
	inline bool	IsDueling() const { return duelaccepted; }
	inline void	SetDuelTarget(int16 set_id) { duel_target=set_id; }
	inline void	SetDueling(bool duel) { duelaccepted = duel; }
	// use this one instead
	void MemSpell(int16 spell_id, int slot, bool update_client = true);
	void UnmemSpell(int slot, bool update_client = true);
	void UnmemSpellAll(bool update_client = true);
	void ScribeSpell(int16 spell_id, int slot, bool update_client = true);
	void UnscribeSpell(int slot, bool update_client = true);
	void UnscribeSpellAll(bool update_client = true);

	inline bool	IsSitting() const {return (playeraction == 1);}
	inline bool	IsBecomeNPC() const { return npcflag; }
	inline int8	GetBecomeNPCLevel() const { return npclevel; }
	inline void	SetBecomeNPC(bool flag) { npcflag = flag; }
	inline void	SetBecomeNPCLevel(int8 level) { npclevel = level; }
	bool	LootToStack(uint32 itemid);
	void	SetFeigned(bool in_feigned);
	// EverHood 6/16/06
	/// this cures timing issues cuz dead animation isn't done but server side feigning is?
	inline bool    GetFeigned()	const {return(feigned); }
	EQStreamInterface* Connection() { return eqs; }
#ifdef PACKET_PROFILER
	void DumpPacketProfile() { if(eqs) eqs->DumpPacketProfile(); }
#endif
	int32 GetEquipment(int8 material_slot) const;	// returns item id
	//sint32 GetEquipmentMaterial(int8 material_slot);
	uint32 GetEquipmentColor(int8 material_slot) const;

	inline bool AutoSplitEnabled() { return(m_pp.autosplit); }

/*    bool GetReduceManaCostItem(int16 &spell_id, char *itemname);
    bool GetExtendedRangeItem(int16 &spell_id, char *itemname);
    bool GetIncreaseSpellDurationItem(int16 &spell_id, char *itemname);
    bool GetReduceCastTimeItem(int16 &spell_id, char *itemname);
    bool GetImprovedHealingItem(int16 &spell_id, char *itemname);
    bool GetImprovedDamageItem(int16 &spell_id, char *itemname);
    sint32 GenericFocus(int16 spell_id, int16 modspellid);
*/
	void SummonHorse(int16 spell_id);
	void SetHorseId(int16 horseid_in);
	int16 GetHorseId() const { return horseId; }

	bool BindWound(Mob* bindmob, bool start, bool fail = false);
	void SetTradeskillObject(Object* object) { m_tradeskill_object = object; }
	Object* GetTradeskillObject() { return m_tradeskill_object; }
	void	SendTributes();
	void	SendGuildTributes();
	void	DoTributeUpdate();
	void	SendTributeDetails(int32 client_id, uint32 tribute_id);
	sint32	TributeItem(int32 slot, int32 quantity);
	sint32	TributeMoney(int32 platinum);
	void	AddTributePoints(sint32 ammount);
	void	ChangeTributeSettings(TributeInfo_Struct *t);
	void	SendTributeTimer();
	void	ToggleTribute(bool enabled);
	void	SendPathPacket(vector<FindPerson_Point> &path);

	inline PTimerList &GetPTimers() { return(p_timers); }

	//AA Methods
	void  SendAAList();
	void  ResetAA();
	void  SendAA(int32 id, int seq=1);
	void  SendPreviousAA(int32 id, int seq=1);
	void  BuyAA(AA_Action* action);
	// solar: this function is used by some AA stuff
	void MemorizeSpell(int32 slot,int32 spellid,int32 scribing);
	void	SetAATitle(const char *txt) { strncpy(m_pp.title, txt, 48); }
	inline int32	GetMaxAAXP(void) const { return max_AAXP; }
	inline uint32  GetAAXP()   const { return m_pp.expAA; }
	void SendAAStats();
	void SendAATable();
	void SendAATimers();
	void ActivateAA(aaID activate);
	void SendAATimer(int32 ability, int32 begin, int32 end);
	void EnableAAEffect(aaEffectType type, int32 duration = 0);
	void DisableAAEffect(aaEffectType type);
	bool CheckAAEffect(aaEffectType type);
	void HandleAAAction(aaID activate);
	int32 GetAA(int32 aa_id) const;
	bool SetAA(int32 aa_id, int32 new_value);
	//void TemporaryPets(int16 spell_id, Mob *target, const char *name_override = NULL, uint32 duration_override = 0);


	sint16 acmod();

	// Item methods
	uint32	NukeItem(uint32 itemnum);
	void	SetTint(sint16 slot_id, uint32 color);
	void	SetTint(sint16 slot_id, Color_Struct& color);
	void	SetMaterial(sint16 slot_id, uint32 item_id);
	void	Undye();
	uint32	GetItemIDAt(sint16 slot_id);
	bool	PutItemInInventory(sint16 slot_id, const ItemInst& inst, bool client_update = false);
	bool	PushItemOnCursor(const ItemInst& inst, bool client_update = false);
	void	DeleteItemInInventory(sint16 slot_id, sint8 quantity = 0, bool client_update = false);
	bool	SwapItem(MoveItem_Struct* move_in);
	void	PutLootInInventory(sint16 slot_id, const ItemInst &inst, ServerLootItem_Struct** bag_item_data = 0);
	bool	AutoPutLootInInventory(ItemInst& inst, bool try_worn = false, bool try_cursor = true, ServerLootItem_Struct** bag_item_data = 0);
	void	SummonItem(uint32 item_id, sint8 charges = 0, uint32 aug1=0, uint32 aug2=0, uint32 aug3=0, uint32 aug4=0, uint32 aug5=0);
	void	SetStats(int8 type,sint16 set_val);
	void	IncStats(int8 type,sint16 increase_val);
	void	DropItem(sint16 slot_id);
	void	SendItemLink(const ItemInst* inst, bool sendtoall=false);
	void	SendLootItemInPacket(const ItemInst* inst, sint16 slot_id);
	void	SendItemPacket(sint16 slot_id, const ItemInst* inst, ItemPacketType packet_type);

	bool	IsTrader() const { return(Trader); }
	eqFilterMode	GetFilter(eqFilterType filter_id) const { return ClientFilters[filter_id]; }
	void	SetFilter(eqFilterType filter_id, eqFilterMode value) { ClientFilters[filter_id]=value; }

	void	BreakInvis();
	Group*	GetGroup() { return entity_list.GetGroupByClient(this); }
	void	LeaveGroup();
	Raid*	GetRaid() { return entity_list.GetRaidByClient(this); }

	bool	Hungry() const {if (GetGM()) return false; return m_pp.hunger_level <= 3000;}
	bool	Thirsty() const {if (GetGM()) return false; return m_pp.thirst_level <= 3000;}

	bool	CheckTradeLoreConflict(Client* other);
	void	LinkDead();
	void	Insight(int32 t_id);
    bool	CheckDoubleAttack(bool tripleAttack = false);
	
	//remove charges/multiple objects from inventory:
	//bool	DecreaseByType(int32 type, int8 amt);
	bool	DecreaseByID(int32 type, int8 amt);
	int8	SlotConvert2(int8 slot);	//Maybe not needed.
	void	Escape(); //AA Escape
	void    RemoveNoRent();
	void	RangedAttack(Mob* other);
	void	ThrowingAttack(Mob* other);

	void	SetZoneFlag(uint32 zone_id);
	void	ClearZoneFlag(uint32 zone_id);
	bool	HasZoneFlag(uint32 zone_id) const;
	void	SendZoneFlagInfo(Client *to) const;
	void	LoadZoneFlags();

	void	ChangeSQLLog(const char *file);
	void	LogSQL(const char *fmt, ...);
	void	GoFish();
	void	ForageItem();
	//Calculate vendor price modifier based on CHA: (reverse==selling)
	float	CalcPriceMod(Mob* other = 0, bool reverse = false);
	void	ResetTrade();
	void	DropInst(const ItemInst* inst);
	bool	TrainDiscipline(int32 itemid);
	void	SendDisciplineUpdate();
	bool	UseDiscipline(int32 spell_id, int32 target);
#ifdef PACKET_UPDATE_MANAGER
	inline UpdateManager *GetUpdateManager() { return(&update_manager); }
#endif
	void	EnteringMessages(Client* client);
	void	SendRules(Client* client);
	std::list<std::string> consent_list;

	//This is used to later set the buff duration of the spell, in slot to duration.
	//Doesn't appear to work directly after the client recieves an action packet.
	void SendBuffDurationPacket(int16 spell_id, int duration, int inlevel);

	bool ClientFinishedLoading() { return (conn_state == ClientConnectFinished); }
	int FindSpellBookSlotBySpellID(int16 spellid);
	int GetNextAvailableSpellBookSlot();
	int16	GetMaxSkillAfterSpecializationRules(SkillType skillid, int16 maxSkill);
       void SendPopupToClient(char *Title, char *Text);
       bool	PendingTranslocate;
 	bool	PendingSacrifice;
 	string	SacrificeCaster;
 	struct	Translocate_Struct PendingTranslocateData;
 	void SendOPTranslocateConfirm(Mob *Caster, int16 SpellID);

	//      Task System Methods
	void	LoadClientTaskState();
	void	RemoveClientTaskState();
	void 	SendTaskActivityComplete(int TaskID, int ActivityID, int TaskIndex, int TaskIncomplete=1);
	void 	SendTaskFailed(int TaskID, int TaskIndex);
	void 	SendTaskComplete(int TaskIndex);

	inline void CancelTask(int TaskIndex) { if(taskstate) taskstate->CancelTask(this, TaskIndex); }

	inline bool SaveTaskState()
		    { return (taskmanager ? taskmanager->SaveClientState(this, taskstate) : false); }

	inline bool IsTaskStateLoaded() { return taskstate != NULL; }

	inline bool IsTaskActive(int TaskID)
		    { return (taskstate ? taskstate->IsTaskActive(TaskID) : false); }

	inline bool IsTaskActivityActive(int TaskID, int ActivityID)
	            { return (taskstate ? taskstate->IsTaskActivityActive(TaskID, ActivityID) : false); }

	inline ActivityState GetTaskActivityState(int index, int ActivityID)
	      		     { return (taskstate ? taskstate->GetTaskActivityState(index, ActivityID) : ActivityHidden); }

	inline void UpdateTaskActivity(int TaskID, int ActivityID, int Count)
		    { if(taskstate) taskstate->UpdateTaskActivity(this, TaskID, ActivityID, Count); }

	inline void UpdateTasksOnKill(int NPCTypeID)
	            { if(taskstate) taskstate->UpdateTasksOnKill(this, NPCTypeID); }

	inline void UpdateTasksForItem(ActivityType Type, int ItemID, int Count=1)
	            { if(taskstate) taskstate->UpdateTasksForItem(this, Type, ItemID, Count); }

	inline void UpdateTasksOnExplore(int ExploreID)
	            { if(taskstate) taskstate->UpdateTasksOnExplore(this, ExploreID); }

	inline bool UpdateTasksOnSpeakWith(int NPCTypeID)
	            { if(taskstate) return taskstate->UpdateTasksOnSpeakWith(this, NPCTypeID); else return false; }

	inline bool UpdateTasksOnDeliver(int32 *Items, int Cash, int NPCTypeID)
	            { if(taskstate) return taskstate->UpdateTasksOnDeliver(this, Items, Cash, NPCTypeID); else return false; }

	inline void TaskSetSelector(Mob *mob, int TaskSetID)
	            { if(taskmanager) taskmanager->TaskSetSelector(this, taskstate, mob, TaskSetID); }

	inline void EnableTask(int TaskCount, int *TaskList)
	            { if(taskstate) taskstate->EnableTask(CharacterID(), TaskCount, TaskList); }

	inline void DisableTask(int TaskCount, int *TaskList)
	            { if(taskstate) taskstate->DisableTask(CharacterID(), TaskCount, TaskList); }

	inline bool IsTaskEnabled(int TaskID)
	            { return (taskstate ? taskstate->IsTaskEnabled(TaskID) : false); }

	inline void ProcessTaskProximities(float X, float Y, float Z)
	            { if(taskstate) taskstate->ProcessTaskProximities(this, X, Y, Z); }

	inline void AssignTask(int TaskID, int NPCID)
	            { if(taskstate) taskstate->AcceptNewTask(this, TaskID, NPCID); }

	inline int ActiveSpeakTask(int NPCID)
	            { if(taskstate) return taskstate->ActiveSpeakTask(NPCID); else return 0; }

	inline int ActiveSpeakActivity(int NPCID, int TaskID)
	            { if(taskstate) return taskstate->ActiveSpeakActivity(NPCID, TaskID); else return 0; }

	inline void FailTask(int TaskID)
	            { if(taskstate) taskstate->FailTask(this, TaskID); }

	inline int TaskTimeLeft(int TaskID)
	            { return (taskstate ? taskstate->TaskTimeLeft(TaskID) : 0); }

	inline int EnabledTaskCount(int TaskSetID)
	            { return (taskstate ? taskstate->EnabledTaskCount(TaskSetID) : -1); }

	inline int IsTaskCompleted(int TaskID)
	            { return (taskstate ? taskstate->IsTaskCompleted(TaskID) : -1); }

	inline void ShowClientTasks()
	            { if(taskstate) taskstate->ShowClientTasks(this); }

	inline void CancelAllTasks()
	            { if(taskstate) taskstate->CancelAllTasks(this); }

	inline int GetActiveTaskCount()
		   { return (taskstate ? taskstate->GetActiveTaskCount() : 0); }

	inline int GetActiveTaskID(int index)
		   { return (taskstate ? taskstate->GetActiveTaskID(index) : -1); }

	inline int GetTaskStartTime(int index)
		   { return (taskstate ? taskstate->GetTaskStartTime(index) : -1); }

	inline bool IsTaskActivityCompleted(int index, int ActivityID)
		    { return (taskstate ? taskstate->IsTaskActivityCompleted(index, ActivityID) : false); }

	inline int GetTaskActivityDoneCount(int ClientTaskIndex, int ActivityID)
	 	   { return (taskstate ? taskstate->GetTaskActivityDoneCount(ClientTaskIndex, ActivityID) :0); }

	inline int ActiveTasksInSet(int TaskSet)
	 	   { return (taskstate ? taskstate->ActiveTasksInSet(TaskSet) :0); }

	inline int CompletedTasksInSet(int TaskSet)
	 	   { return (taskstate ? taskstate->CompletedTasksInSet(TaskSet) :0); }


protected:
	friend class Mob;
	void CalcItemBonuses(StatBonuses* newbon);
	void AddItemBonuses(const ItemInst *inst, StatBonuses* newbon, bool isAug = false);
	int  CalcRecommendedLevelBonus(int8 level, uint8 reclevel, int basestat);
	void CalcEdibleBonuses(StatBonuses* newbon);
	void MakeBuffFadePacket(int16 spell_id, int slot_id, bool send_message = true);
	bool client_data_loaded;

	sint16	GetFocusEffect(focusType type, int16 spell_id);
	sint16	CalcFocusEffect(focusType type, int16 focus_id, int16 spell_id);
private:
	eqFilterMode ClientFilters[_FilterCount];
	sint32	HandlePacket(const EQApplicationPacket *app);
	void	OPTGB(const EQApplicationPacket *app);
	void	OPRezzAnswer(const EQApplicationPacket *app);
	void	OPMemorizeSpell(const EQApplicationPacket *app);
	void	OPMoveCoin(const EQApplicationPacket* app);
	void	MoveItemCharges(ItemInst &from, sint16 to_slot, int8 type);
	void	OPGMTraining(const EQApplicationPacket *app);
	void	OPGMEndTraining(const EQApplicationPacket *app);
	void	OPGMTrainSkill(const EQApplicationPacket *app);
	void	OPGMSummon(const EQApplicationPacket *app);
	void	OPCombatAbility(const EQApplicationPacket *app);

	sint16    CalcAC();
	sint16    CalcATK();
	int      CalcHaste();

	sint16   CalcSTR();
	sint16   CalcSTA();
	sint16   CalcDEX();
	sint16   CalcAGI();
	sint16   CalcINT();
	sint16   CalcWIS();
	sint16   CalcCHA();

    sint16	CalcMR();
	sint16	CalcFR();
	sint16	CalcDR();
	sint16	CalcPR();
	sint16	CalcCR();
	sint32	CalcMaxHP();
	sint32	CalcBaseHP();
	void DoHPRegen(/*SpawnAppearance_Struct* sa*/);
	void DoManaRegen();
	void DoStaminaUpdate();

	int32 pLastUpdate;
	int32 pLastUpdateWZ;
	int8  playeraction;

	EQStreamInterface* eqs;

	int32				ip;
	int16				port;
    CLIENT_CONN_STATUS  client_state;
	int32				character_id;
	int32				WID;
	int32				account_id;
	char				account_name[30];
	int32				lsaccountid;
	char				lskey[30];
	sint16				admin;
//	int32				guilddbid; // guild's ID in the database
	int32				guild_id;
	int8				guildrank; // player's rank in the guild, 0-GUILD_MAX_RANK
	int16				duel_target;
	bool				duelaccepted;
	std::list<int32> keyring;
	bool				tellsoff;	// GM /toggle
	bool				gmhideme;
	bool				LFG;
	bool				AFK;
	bool				auto_attack;
	bool				auto_fire;
	int8				gmspeed;
	bool				medding;
	int16				horseId;
	bool				revoked;
	int32				pQueuedSaveWorkID;
	int16				pClientSideTarget;
//	bool				auto_split;
	int16				weight;
	bool				berserk;
	bool				dead;
	bool				IsOnBoat;
	bool				IsTracking;
	bool				withcustomer;
	bool	Trader;
	bool	cheater;
	float	cheat_x;
	float	cheat_y;
	bool	AbilityTimer;
	int8	cheatcount;
	int Haste;  //precalced value

	sint32				max_end;
	sint32				cur_end;

	PlayerProfile_Struct		m_pp;
	ExtendedProfile_Struct		m_epp;
	Inventory					m_inv;
//	ServerSideFilters_Struct	ssfs;
	Object*						m_tradeskill_object;

	void NPCSpawn(const Seperator* sep);
	uint32 GetEXPForLevel(uint16 level);

	bool	CanBeInZone();
	void	SendLogoutPackets();
    bool    AddPacket(const EQApplicationPacket *, bool);
    bool    AddPacket(EQApplicationPacket**, bool);
    bool    SendAllPackets();
	LinkedList<CLIENTPACKET *> clientpackets;

	//Zoning related stuff
	void SendZoneCancel(ZoneChange_Struct *zc);
	void SendZoneError(ZoneChange_Struct *zc, sint8 err);
	void DoZoneSuccess(ZoneChange_Struct *zc, uint16 zone_id, float dest_x, float dest_y, float dest_z, float dest_h, sint8 ignore_r);
	void ZonePC(int32 zoneID, float x, float y, float z, float heading, int8 ignorerestrictions, ZoneMode zm);
	void ProcessMovePC(int32 zoneID, float x, float y, float z, float heading, int8 ignorerestrictions = 0, ZoneMode zm = ZoneSolicited);
	//	char	zonesummon_name[32];
	float	zonesummon_x;
	float	zonesummon_y;
	float	zonesummon_z;
	uint16	zonesummon_id;
	int8	zonesummon_ignorerestrictions;
	ZoneMode zone_mode;


	Timer	position_timer;
	int8	position_timer_counter;

	PTimerList p_timers;		//persistent timers
	Timer	hpupdate_timer;
	Timer	camp_timer;
	Timer	process_timer;
	Timer	stamina_timer;
//	Timer	LDTimer;
	Timer	zoneinpacket_timer;
	Timer	linkdead_timer;
	Timer	dead_timer;
	Timer	ooc_timer;
	Timer	shield_timer;
	Timer	fishing_timer;
	Timer	endupkeep_timer;
	Timer	forget_timer;	// our 2 min everybody forgets you timer
	Timer	autosave_timer;
#ifdef REVERSE_AGGRO
	Timer	scanarea_timer;
#endif
	Timer	tribute_timer;

#ifdef PACKET_UPDATE_MANAGER
	UpdateManager update_manager;
#endif

	Timer	proximity_timer;
	float	proximity_x;
	float	proximity_y;
	float	proximity_z;

	Timer	TaskPeriodic_Timer;

	void	BulkSendInventoryItems();

	faction_map factionvalues;

	int32 tribute_master_id;

	FILE *SQL_log;
	int32       max_AAXP;
	int32		staminacount;
	AA_Array* aa[MAX_PP_AA_ARRAY];		//this list contains pointers into our player profile
	map<int32,int8> aa_points;
	bool npcflag;
	int8 npclevel;
	bool feigned;
	bool zoning;
	bool tgb;
	bool instalog;
	sint32	last_reported_mana;
	sint32	last_reported_endur;

	set<uint32> zone_flags;

	ClientTaskState *taskstate;


	//Connecting debug code.
	enum { //connecting states, used for debugging only
		NoPacketsReceived,		//havent gotten anything
		//this is the point where the client changes to the loading screen
		ReceivedZoneEntry,		//got the first packet, loading up PP
		PlayerProfileLoaded,	//our DB work is done, sending it
		ZoneInfoSent,		//includes PP, tributes, tasks, spawns, time and weather
		//this is the point where the client shows a status bar zoning in
		NewZoneRequested,	//received and sent new zone request
		ClientSpawnRequested,	//client sent ReqClientSpawn
		ZoneContentsSent,		//objects, doors, zone points
		ClientReadyReceived,	//client told us its ready, send them a bunch of crap like guild MOTD, etc
		//this is the point where the client releases the mouse
		ClientConnectFinished	//client finally moved to finished state, were done here
	} conn_state;
	void ReportConnectingState();
};

#include "parser.h"
#endif
