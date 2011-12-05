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
#include <iostream>
using namespace std;
#include <string.h>
#include <stdio.h>
#include <iomanip>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>

#include "../common/servertalk.h"
#include "worldserver.h"
#include "queryservconfig.h"
#include "database.h"
#include "../common/packet_functions.h"
#include "../common/md5.h"
#include "../common/files.h"

extern WorldServer worldserver;
extern const queryservconfig *Config;
extern Database database;

WorldServer::WorldServer()
: WorldConnection(EmuTCPConnection::packetModeQueryServ, Config->SharedKey.c_str())
{
	pTryReconnect = true;
}

WorldServer::~WorldServer()
{
}

void WorldServer::OnConnected()
{
	_log(QUERYSERV__INIT, "Connected to World.");
	WorldConnection::OnConnected();
}

void WorldServer::Process()
{
	WorldConnection::Process();
	
	if (!Connected())
		return;

	ServerPacket *pack = 0;

	while((pack = tcpc.PopPacket()))
	{
		_log(UCS__TRACE, "Received Opcode: %4X", pack->opcode);

		switch(pack->opcode)
		{
			case 0: {
				break;
			}
			case ServerOP_KeepAlive:
			{
				break;
			}
			case ServerOP_Speech:
			{
			Server_Speech_Struct *SSS = (Server_Speech_Struct*)pack->pBuffer;

			string tmp1 = SSS->from;
			string tmp2 = SSS->to;

			database.AddSpeech(tmp1.c_str(), tmp2.c_str(), SSS->message, SSS->minstatus, SSS->guilddbid, SSS->type);
			break;
			}

		}
	}

	safe_delete(pack);
	return;
}

