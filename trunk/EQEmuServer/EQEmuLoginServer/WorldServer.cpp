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
#include "WorldServer.h"
#include "ErrorLog.h"
#include "LoginServer.h"
#include "LoginStructures.h"

extern ErrorLog *log;
extern LoginServer server;

WorldServer::WorldServer(EmuTCPConnection *c)
{
	connection = c;
	zones_booted = 0;
	players_online = 0;
	status = 0;
	runtime_id;
	server_list_id = 0;
	server_type = 0;
	authorized = false;
	trusted = false;
	logged_in = false;
}

WorldServer::~WorldServer()
{
	if(connection)
	{
		connection->Free();
	}
}

void WorldServer::Reset()
{
	zones_booted = 0;
	players_online = 0;
	status = 0;
	runtime_id;
	server_list_id = 0;
	server_type = 0;
	authorized = false;
	logged_in = false;
}

bool WorldServer::Process()
{
	ServerPacket *app = NULL;
	while(app = connection->PopPacket())
	{
		if(server.options.IsTraceOn())
		{
			log->Log(log_network_trace, "Application packet recieved from server: 0x%.4X, (size %u)", app->opcode, app->size);
		}

		if(server.options.IsDumpInPacketsOn())
		{
			DumpPacket(app);
		}

		switch(app->opcode)
		{
		case ServerOP_NewLSInfo:
			{
				if(app->size < sizeof(ServerNewLSInfo_Struct))
				{
					log->Log(log_network_error, "Recieved application packet from server that had opcode ServerOP_NewLSInfo, "
						"but was too small. Discarded to avoid buffer overrun.");
					break;
				}

				if(server.options.IsTraceOn())
				{
					log->Log(log_network_trace, "New Login Info Recieved.");
				}

				ServerNewLSInfo_Struct *info = (ServerNewLSInfo_Struct*)app->pBuffer;
				Handle_NewLSInfo(info);
				break;
			}
		case ServerOP_LSStatus:
			{
				if(app->size < sizeof(ServerLSStatus_Struct))
				{
					log->Log(log_network_error, "Recieved application packet from server that had opcode ServerOP_LSStatus, "
						"but was too small. Discarded to avoid buffer overrun.");
					break;
				}

				if(server.options.IsTraceOn())
				{
					log->Log(log_network_trace, "World Server Status Recieved.");
				}

				ServerLSStatus_Struct *ls_status = (ServerLSStatus_Struct*)app->pBuffer;
				Handle_LSStatus(ls_status);
				break;
			}
		case ServerOP_LSZoneShutdown:
		case ServerOP_LSZoneStart:
		case ServerOP_LSZoneBoot:
		case ServerOP_LSZoneSleep:
			{
				if(server.options.IsTraceOn())
				{
					log->Log(log_network_trace, "Zone Status Packet Recieved.");
				}
				break;
			}

		case ServerOP_UsertoWorldResp:
			{
				if(app->size < sizeof(UsertoWorldResponse_Struct))
				{
					log->Log(log_network_error, "Recieved application packet from server that had opcode ServerOP_UsertoWorldResp, "
						"but was too small. Discarded to avoid buffer overrun.");
					break;
				}

				if(server.options.IsTraceOn())
				{
					log->Log(log_network_trace, "User-To-World Response recieved.");
				}

				UsertoWorldResponse_Struct *utwr = (UsertoWorldResponse_Struct*)app->pBuffer;
				Client *c = server.CM->GetClient(utwr->lsaccountid);
				if(c)
				{
					EQApplicationPacket *outapp = new EQApplicationPacket(OP_PlayEverquestResponse, sizeof(PlayEverquestResponse_Struct));
					PlayEverquestResponse_Struct *per = (PlayEverquestResponse_Struct*)outapp->pBuffer;

					if(utwr->response > 0)
					{
						per->Allowed = 1;
						SendClientAuth(c->GetConnection()->GetRemoteIP(), c->GetAccountName(), c->GetKey(), c->GetAccountID());
					}
					per->Sequence = c->GetPlaySequence();
					per->ServerNumber = c->GetPlayServerID();
					switch(utwr->response)
					{
					case 1:
						per->Message = 101;
						break;
					case 0:
						per->Message = 326;
						break;
					case -1:
						per->Message = 337;
						break;
					case -2:
						per->Message = 338;
						break;
					case -3:
						per->Message = 303;
						break;
					}

					if(server.options.IsDumpOutPacketsOn())
					{
						DumpPacket(outapp);
					}

					c->SendPlayResponse(outapp);
					delete outapp;
				}
				break;
			}
		case ServerOP_LSAccountUpdate:
			{
				log->Log(log_network_trace, "ServerOP_LSAccountUpdate packet recieved");
				ServerLSAccountUpdate_Struct *lsau = (ServerLSAccountUpdate_Struct*)app->pBuffer;
				if(trusted)
				{
					log->Log(log_network_trace, "ServerOP_LSAccountUpdate update processed");
					string name;
					string password;
					string email;
					name.assign(lsau->useraccount);
					password.assign(lsau->userpassword);
					email.assign(lsau->useremail);
					server.db->UpdateLSAccountInfo(lsau->useraccountid, name, password, email);
				}
				break;
			}
		default:
			{
				log->Log(log_network_error, "Recieved application packet from server that had an unknown operation code 0x%.4X.", app->opcode);
			}
		}

		delete app;
		app = NULL;
	}
	return true;
}

void WorldServer::Handle_NewLSInfo(ServerNewLSInfo_Struct* i)
{
	if(logged_in)
	{
		log->Log(log_network_error, "WorldServer::Handle_NewLSInfo called but the login server was already marked as logged in, aborting.");
		return;
	}

	if(strlen(i->account) <= 30)
	{
		account_name = i->account;
	}
	else
	{
		log->Log(log_network_error, "Handle_NewLSInfo error, account name was too long.");
		return;
	}

	if(strlen(i->password) <= 30)
	{
		account_password = i->password;
	}
	else
	{
		log->Log(log_network_error, "Handle_NewLSInfo error, account password was too long.");
		return;
	}

	if(strlen(i->name) <= 200)
	{
		long_name = i->name;
	}
	else
	{
		log->Log(log_network_error, "Handle_NewLSInfo error, long name was too long.");
		return;
	}

	if(strlen(i->shortname) <= 50)
	{
		short_name = i->shortname;
	}
	else
	{
		log->Log(log_network_error, "Handle_NewLSInfo error, short name was too long.");
		return;
	}

	if(strlen(i->local_address) <= 125)
	{
		if(strlen(i->local_address) == 0)
		{
			log->Log(log_network_error, "Handle_NewLSInfo error, local address was null, defaulting to localhost");
			local_ip = "127.0.0.1";
		}
		else
		{
			local_ip = i->local_address;
		}
	}
	else
	{
		log->Log(log_network_error, "Handle_NewLSInfo error, local address was too long.");
		return;
	}

	if(strlen(i->remote_address) <= 125)
	{
		if(strlen(i->remote_address) == 0)
		{
			log->Log(log_network_error, "Handle_NewLSInfo error, remote address was null, defaulting to stream address %s.", remote_ip.c_str());
			in_addr in;
			in.s_addr = GetConnection()->GetrIP();
			remote_ip = inet_ntoa(in);
		}
		else
		{
			remote_ip = i->remote_address;
		}
	}
	else
	{
		log->Log(log_network_error, "Handle_NewLSInfo error, remote address was too long, defaulting to stream address.");
		in_addr in;
		in.s_addr = GetConnection()->GetrIP();
		remote_ip = inet_ntoa(in);
	}

	if(strlen(i->serverversion) <= 64)
	{
		version = i->serverversion;
	}
	else
	{
		log->Log(log_network_error, "Handle_NewLSInfo error, server version was too long.");
		return;
	}

	if(strlen(i->protocolversion) <= 25)
	{
		protocol = i->protocolversion;
	}
	else
	{
		log->Log(log_network_error, "Handle_NewLSInfo error, protocol version was too long.");
		return;
	}

	server_type = i->servertype;
	logged_in = true;

	if(!server.options.IsUnregisteredAllowed())
	{
		if(account_name.size() > 0 && account_password.size() > 0)
		{
			unsigned int s_id = 0;
			unsigned int s_list_type = 0;
			unsigned int s_trusted = 0;
			string s_desc;
			string s_list_desc;
			string s_acct_name;
			string s_acct_pass;
			if(server.db->GetWorldRegistration(long_name, short_name, s_id, s_desc, s_list_type, s_trusted, s_list_desc, s_acct_name, s_acct_pass))
			{
				if(s_acct_name.compare(account_name) == 0 && s_acct_pass.compare(account_password) == 0)
				{
					log->Log(log_world, "Server %s(%s) successfully logged in.", 
						long_name.c_str(), short_name.c_str());
					authorized = true;
					id = s_id;
					server_list_id = s_list_type;
					desc = s_desc;
					if(s_trusted)
					{
						log->Log(log_network_trace, "ServerOP_LSAccountUpdate sent to world");
						trusted = true;
						ServerPacket *outapp = new ServerPacket(ServerOP_LSAccountUpdate, 0);
						connection->SendPacket(outapp);
					}
				}
				else
				{
					log->Log(log_world, "Server %s(%s) attempted to log in but account and password did not match the entry in the database, and only"
						" registered servers are allowed.", long_name.c_str(), short_name.c_str());
					return;
				}
			}
			else
			{
				log->Log(log_world, "Server %s(%s) attempted to log in but database couldn't find an entry and only registered servers are allowed.", 
					long_name.c_str(), short_name.c_str());
				return;
			}
		}
		else
		{
			log->Log(log_world, "Server %s(%s) did not attempt to log in but only registered servers are allowed.", 
				long_name.c_str(), short_name.c_str());
			return;
		}
	}
	else
	{
		if(account_name.size() > 0 && account_password.size() > 0)
		{
			unsigned int s_id = 0;
			unsigned int s_list_type = 0;
			unsigned int s_trusted = 0;
			string s_desc;
			string s_list_desc;
			string s_acct_name;
			string s_acct_pass;
			if(server.db->GetWorldRegistration(long_name, short_name, s_id, s_desc, s_list_type, s_trusted, s_list_desc, s_acct_name, s_acct_pass))
			{
				if(s_acct_name.compare(account_name) == 0 && s_acct_pass.compare(account_password) == 0)
				{
					log->Log(log_world, "Server %s(%s) successfully logged in.", 
						long_name.c_str(), short_name.c_str());
					authorized = true;
					id = s_id;
					server_list_id = s_list_type;
					desc = s_desc;
					if(s_trusted)
					{
						log->Log(log_network_trace, "ServerOP_LSAccountUpdate sent to world");
						trusted = true;
						ServerPacket *outapp = new ServerPacket(ServerOP_LSAccountUpdate, 0);
						connection->SendPacket(outapp);
					}
				}
				else
				{
					log->Log(log_world, "Server %s(%s) attempted to log in but account and password did not match the entry in the database.", 
						long_name.c_str(), short_name.c_str());
					authorized = true;
					server_list_id = 3;
				}
			}
			else
			{
				log->Log(log_world, "Server %s(%s) attempted to log in but database couldn't find an entry.", 
					long_name.c_str(), short_name.c_str());
				authorized = true;
				server_list_id = 3;
			}
		}
		else
		{
			log->Log(log_world, "Server %s(%s) did not attempt to log in but unregistered servers are allowed.", 
					long_name.c_str(), short_name.c_str());
			authorized = true;
			server_list_id = 3;
		}
	}

	in_addr in;
	in.s_addr = connection->GetrIP();
	server.db->UpdateWorldRegistration(id, string(inet_ntoa(in)));
}

void WorldServer::Handle_LSStatus(ServerLSStatus_Struct *s)
{
	players_online = s->num_players;
	zones_booted = s->num_zones;
	status = s->status;
}

void WorldServer::SendClientAuth(unsigned int ip, string account, string key, unsigned int account_id)
{
	ServerPacket *outapp = new ServerPacket(ServerOP_LSClientAuth, sizeof(ServerLSClientAuth));
	ServerLSClientAuth* slsca = (ServerLSClientAuth*)outapp->pBuffer;
			
	slsca->lsaccount_id = account_id;
	strncpy(slsca->name, account.c_str(), account.size() > 30 ? 30 : account.size());
	strncpy(slsca->key, key.c_str(), 10);
	slsca->lsadmin = 0;
	slsca->worldadmin = 0;
	slsca->ip = ip;

	in_addr in;
	in.s_addr = ip;connection->GetrIP();
	string client_address(inet_ntoa(in));
	in.s_addr = connection->GetrIP();
	string world_address(inet_ntoa(in));

	if(client_address.compare(world_address) == 0)
	{
		slsca->local = 1;
	}
	else if(client_address.find(server.options.GetLocalNetwork()) != string::npos)
	{
		slsca->local = 1;
	}
	else
	{
		slsca->local = 0;
	}

	connection->SendPacket(outapp);

	if(server.options.IsDumpInPacketsOn())
	{
		DumpPacket(outapp);
	}
	delete outapp;
}

