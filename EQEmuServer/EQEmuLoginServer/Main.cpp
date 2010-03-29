/*  EQEMu:  Everquest Server Emulator
    Copyright (C) 2001-2010  EQEMu Development Team (http://eqemulator.net)

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
#include "LoginServer.h"
#include "../common/types.h"
#include "../common/opcodemgr.h"
#include "../common/EQStreamFactory.h"
#include "../common/timer.h"
#include <time.h>
#include <stdlib.h>
#include <string>
#include <sstream>

TimeoutManager timeout_manager;
LoginServer server;
ErrorLog *log;
bool run_server = true;
using namespace std;

void CatchSignal(int sig_num)
{
}

int main()
{
	//Create our error log, is of format login_<number>.log
	time_t current_time = time(NULL);
	stringstream log_name(stringstream::in | stringstream::out);
	log_name << ".\\logs\\login_" << (unsigned int)current_time << ".log";
	log = new ErrorLog(log_name.str().c_str());
	log->Log(log_debug, "Logging System Init.");

	//Create our subsystem and parse the ini file.
	server.config = new Config();
	log->Log(log_debug, "Config System Init.");
	server.config->Parse("login.ini");
	
	//Parse unregistered allowed option.
	if(server.config->GetVariable("options", "unregistered_allowed").compare("FALSE") == 0)
	{
		server.options.AllowUnregistered(false);
	}

	//Parse trace option.
	if(server.config->GetVariable("options", "trace").compare("TRUE") == 0)
	{
		server.options.Trace(true);
	}

	//Parse packet inc dump option.
	if(server.config->GetVariable("options", "dump_packets_in").compare("TRUE") == 0)
	{
		server.options.DumpInPackets(true);
	}

	//Parse packet out dump option.
	if(server.config->GetVariable("options", "dump_packets_out").compare("TRUE") == 0)
	{
		server.options.DumpOutPackets(true);
	}

	//Parse encryption mode option.
	string mode = server.config->GetVariable("security", "mode");
	if(mode.size() > 0)
	{
		server.options.EncryptionMode(atoi(mode.c_str()));
	}

	//Parse local network option.
	string ln = server.config->GetVariable("options", "local_network");
	if(ln.size() > 0)
	{
		server.options.LocalNetwork(ln);
	}

	//Parse account table option.
	ln = server.config->GetVariable("schema", "account_table");
	if(ln.size() > 0)
	{
		server.options.AccountTable(ln);
	}

	//Parse world account table option.
	ln = server.config->GetVariable("schema", "world_registration_table");
	if(ln.size() > 0)
	{
		server.options.WorldRegistrationTable(ln);
	}

	//Parse admin world account table option.
	ln = server.config->GetVariable("schema", "world_admin_registration_table");
	if(ln.size() > 0)
	{
		server.options.WorldAdminRegistrationTable(ln);
	}

	//Parse world type table option.
	ln = server.config->GetVariable("schema", "world_server_type_table");
	if(ln.size() > 0)
	{
		server.options.WorldServerTypeTable(ln);
	}

	//Create our DB from options.
	if(server.config->GetVariable("database", "subsystem").compare("MySQL") == 0)
	{
#ifdef EQEMU_MYSQL_ENABLED
		log->Log(log_debug, "MySQL Database Init.");
		server.db = (Database*)new DatabaseMySQL(
			server.config->GetVariable("database", "user"), 
			server.config->GetVariable("database", "password"), 
			server.config->GetVariable("database", "host"), 
			server.config->GetVariable("database", "port"), 
			server.config->GetVariable("database", "db"));
#endif
	}
	else if(server.config->GetVariable("database", "subsystem").compare("PostgreSQL") == 0)
	{
#ifdef EQEMU_POSTGRESQL_ENABLED
		log->Log(log_debug, "PostgreSQL Database Init.");
		server.db = (Database*)new DatabasePostgreSQL(
			server.config->GetVariable("database", "user"), 
			server.config->GetVariable("database", "password"), 
			server.config->GetVariable("database", "host"), 
			server.config->GetVariable("database", "port"), 
			server.config->GetVariable("database", "db"));
#endif
	}

	//Make sure our database got created okay, otherwise cleanup and exit.
	if(!server.db)
	{
		log->Log(log_error, "Database Initialization Failure.");
		log->Log(log_debug, "Config System Shutdown.");
		delete server.config;
		log->Log(log_debug, "Log System Shutdown.");
		delete log;
		return 1;
	}

#if WIN32
	//initialize our encryption.
	log->Log(log_debug, "Encryption Initialize.");
	server.eq_crypto = new Encryption();
	if(server.eq_crypto->LoadCrypto(server.config->GetVariable("security", "plugin")))
	{
		log->Log(log_debug, "Encryption Loaded Successfully.");
	}
	else
	{
		//We can't run without encryption, cleanup and exit.
		log->Log(log_error, "Encryption Failed to Load.");
		log->Log(log_debug, "Database System Shutdown.");
		delete server.db;
		log->Log(log_debug, "Config System Shutdown.");
		delete server.config;
		log->Log(log_debug, "Log System Shutdown.");
		delete log;
		return 1;
	}
#endif

	//create our server manager.
	log->Log(log_debug, "Server Manager Initialize.");
	server.SM = new ServerManager();
	if(!server.SM)
	{
		//We can't run without a server manager, cleanup and exit.
		log->Log(log_error, "Server Manager Failed to Start.");
#ifdef WIN32
		log->Log(log_debug, "Encryption System Shutdown.");
		delete server.eq_crypto;
#endif
		log->Log(log_debug, "Database System Shutdown.");
		delete server.db;
		log->Log(log_debug, "Config System Shutdown.");
		delete server.config;
		log->Log(log_debug, "Log System Shutdown.");
		delete log;
		return 1;
	}


	//create our client manager.
	log->Log(log_debug, "Client Manager Initialize.");
	server.CM = new ClientManager();
	if(!server.CM)
	{
		//We can't run without a client manager, cleanup and exit.
		log->Log(log_error, "Client Manager Failed to Start.");
		log->Log(log_debug, "Server Manager Shutdown.");
		delete server.SM;
#ifdef WIN32
		log->Log(log_debug, "Encryption System Shutdown.");
		delete server.eq_crypto;
#endif
		log->Log(log_debug, "Database System Shutdown.");
		delete server.db;
		log->Log(log_debug, "Config System Shutdown.");
		delete server.config;
		log->Log(log_debug, "Log System Shutdown.");
		delete log;
		return 1;
	}

#ifdef WIN32
	SetConsoleTitle("EQEmu Login Server");
#endif

	log->Log(log_debug, "Server Started.");
	while(run_server)
	{
		Timer::SetCurrentTime();
		server.CM->Process();
		server.SM->Process();
		Sleep(100);
	}

	log->Log(log_debug, "Server Shutdown.");
	log->Log(log_debug, "Client Manager Shutdown.");
	delete server.CM;
	log->Log(log_debug, "Server Manager Shutdown.");
	delete server.SM;
#ifdef WIN32
	log->Log(log_debug, "Encryption System Shutdown.");
	delete server.eq_crypto;
#endif
	log->Log(log_debug, "Database System Shutdown.");
	delete server.db;
	log->Log(log_debug, "Config System Shutdown.");
	delete server.config;
	log->Log(log_debug, "Log System Shutdown.");
	delete log;
	return 0;
}

