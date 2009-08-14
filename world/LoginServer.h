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
#ifndef LOGINSERVER_H
#define LOGINSERVER_H

#include "../common/servertalk.h"
#include "../common/linked_list.h"
#include "../common/timer.h"
#include "../common/queue.h"
#include "../common/eq_packet_structs.h"
#include "../common/Mutex.h"
#include "../common/EmuTCPConnection.h"

#ifdef WIN32
	void AutoInitLoginServer(void *tmp);
#else
	void *AutoInitLoginServer(void *tmp);
#endif
bool InitLoginServer();

class LoginServer{
public:
	LoginServer(const char* iAddress = 0, int16 iPort = 5999);
    ~LoginServer();

	bool Process();
	bool Connect(const char* iAddress = 0, int16 iPort = 0);

	void SendInfo();
	void SendNewInfo();
	void SendStatus();

	void SendPacket(ServerPacket* pack) { tcpc->SendPacket(pack); }
	bool ConnectReady() { return tcpc->ConnectReady(); }
	bool Connected() { return tcpc->Connected(); }
	bool MiniLogin() { return minilogin; }

private:
	bool minilogin;
	EmuTCPConnection* tcpc;
	int32	LoginServerIP;
	int16	LoginServerPort;

	Timer statusupdate_timer;
};
#endif
