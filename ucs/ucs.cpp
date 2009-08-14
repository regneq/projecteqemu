/*
	EQEMu:  Everquest Server Emulator

	Copyright (C) 2001-2008 EQEMu Development Team (http://eqemulator.net)

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
#include "clientlist.h"
#include "../common/opcodemgr.h"
#include "../common/EQStreamFactory.h"
#include "../common/rulesys.h"
#include "database.h"
#include "ucsconfig.h"
#include "IRC.h"
#include "chatchannel.h"
#include <list>
#include <signal.h>
#ifdef WIN32
#include <process.h>
#else
#include <pthread.h>
#endif


volatile bool RunLoops = true;

uint32 MailMessagesSent = 0;
uint32 ChatMessagesSent = 0;

TimeoutManager          timeout_manager;

Clientlist *CL;

ChatChannelList *ChannelList;

Database database;

string WorldShortName;

const ucsconfig *Config;
//const bool UseIRC = ucsconfig::get()->UseIRC;
IRC conn;

RuleManager *rules = new RuleManager();

void CatchSignal(int sig_num) {

	RunLoops = false;
}

string GetMailPrefix() {

	return "SOE.EQ." + WorldShortName + ".";

}
/* maintains the loop for processing the other two IRC commands */
/*
#ifdef WIN32
void IRCConnect(void* irc_conn) {
#else	//not Windows
void *IRCConnect(void* irc_conn) {
#endif
		conn.message_loop();
}

int end_of_motd(char* params, irc_reply_data* hostd, void* conn)	//hooks END OF MOTD message from IRC
	{
		IRC* irc_conn=(IRC*)conn;
	
		const ucsconfig *Config=ucsconfig::get();

		char name[128];

		strcpy(name,Config->ChannelToOutput.c_str());
			

		irc_conn->join(name);

		return 0;
	}

std::string GetCleanMessageFromIRC(std::string in) {
	std::string out = in;	//our return string

	if (in.empty())	//if there's nothing there, we don't really need to do the rest of this
		return in;

	if (in.at(0) == ':')	//get rid of that annoying colon at the beginning
		out = in.substr(1);

	//TODO: convert ACTION to an actual emote, or something similar
	//

	return out;
}

int triggers(char*params,irc_reply_data*hostd,void*conn)	//hooks privmsg to your specified channel
{

	IRC* irc_conn=(IRC*)conn;
	

	const ucsconfig *Config=ucsconfig::get();

	string ChannelName = Config->EQChannelToOutput;

	ChatChannel *RequiredChannel = ChannelList->FindChannel(ChannelName);

	char name[128];
	strcpy(name,Config->ChannelToOutput.c_str());

	if(!strcmp(params,":!rejoin"))
	{
		irc_conn->join(name);
	}	

	string parame = params;
	string IRCName = hostd->nick;

	if(!strcmp(hostd->target,name))
	{
		RequiredChannel->SendMessageToChannelFromIRC(GetCleanMessageFromIRC(parame), IRCName); // I got an IRC command, now let's send it
	}
	return 0;
	
}
*/

int main() {

	// Check every minute for unused channels we can delete
	//
	Timer ChannelListProcessTimer(60000);

	_log(UCS__INIT, "Starting EQEmu Universal Chat Server.");

	if (!ucsconfig::LoadConfig()) {

		_log(UCS__INIT, "Loading server configuration failed.");

		return(1);
	}

	Config = ucsconfig::get();
/*
	if(UseIRC) {
		char array1[64];
		char array2[64];
		strcpy(array1, Config->ChatIRCHost.c_str());
		strcpy(array2, Config->ChatIRCNick.c_str());
		conn.hook_irc_command("PRIVMSG",&triggers);
		conn.hook_irc_command("376", &end_of_motd); //hook the end of MOTD message
		conn.hook_irc_command("422", &end_of_motd);	//MOTD File is missing
		conn.start(array1,Config->ChatIRCPort,array2,"EQEMu IRC Bot","EQEMu IRC Bot",0);

#ifdef WIN32
		_beginthread(IRCConnect,0,(void*)&conn);
#else
		pthread_t th1;
		pthread_create(&th1,NULL,IRCConnect,(void*)&conn);
#endif
	}
*/
	if(!load_log_settings(Config->LogSettingsFile.c_str()))
		_log(UCS__INIT, "Warning: Unable to read %s", Config->LogSettingsFile.c_str());
	else
		_log(UCS__INIT, "Log settings loaded from %s", Config->LogSettingsFile.c_str());

	WorldShortName = Config->ShortName;

	_log(UCS__INIT, "Connecting to MySQL...");

	if (!database.Connect(
		Config->DatabaseHost.c_str(),
		Config->DatabaseUsername.c_str(),
		Config->DatabasePassword.c_str(),
		Config->DatabaseDB.c_str(),
		Config->DatabasePort)) {
		_log(WORLD__INIT_ERR, "Cannot continue without a database connection.");
		return(1);
	}

	char tmp[64];

	if (database.GetVariable("RuleSet", tmp, sizeof(tmp)-1)) {
		_log(WORLD__INIT, "Loading rule set '%s'", tmp);
		if(!rules->LoadRules(&database, tmp)) {
			_log(UCS__ERROR, "Failed to load ruleset '%s', falling back to defaults.", tmp);
		}
	} else {
		if(!rules->LoadRules(&database, "default")) {
			_log(UCS__INIT, "No rule set configured, using default rules");
		} else {
			_log(UCS__INIT, "Loaded default rule set 'default'", tmp);
		}
	}

	database.ExpireMail();

	if(Config->ChatPort != Config->MailPort)
	{
		_log(UCS__ERROR, "MailPort and CharPort must be the same in eqemu_config.xml for UCS.");
		exit(1);
	}

	CL = new Clientlist(Config->ChatPort);

	ChannelList = new ChatChannelList();
	
	database.LoadChatChannels();

	if (signal(SIGINT, CatchSignal) == SIG_ERR)	{
		_log(UCS__ERROR, "Could not set signal handler");
		return 0;
	}
	if (signal(SIGTERM, CatchSignal) == SIG_ERR)	{
		_log(UCS__ERROR, "Could not set signal handler");
		return 0;
	}

	while(RunLoops) {

		Timer::SetCurrentTime();

		CL->Process();

		if(ChannelListProcessTimer.Check())
			ChannelList->Process();

		timeout_manager.CheckTimeouts();

		Sleep(100);
	}

	ChannelList->RemoveAllChannels();

	CL->CloseAllConnections();

}

void UpdateWindowTitle(char* iNewTitle) {
#ifdef WIN32
        char tmp[500];
        if (iNewTitle) {
                snprintf(tmp, sizeof(tmp), "UCS: %s", iNewTitle);
        }
        else {
                snprintf(tmp, sizeof(tmp), "UCS");
        }
        SetConsoleTitle(tmp);
#endif
}
