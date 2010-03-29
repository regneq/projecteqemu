#ifndef EQEMU_SERVERMANAGER_H
#define EQEMU_SERVERMANAGER_H

#include "../common/debug.h"
#include "../common/EQStreamFactory.h"
#include "../common/EmuTCPConnection.h"
#include "../common/EmuTCPServer.h"
#include "../common/servertalk.h"
#include "../common/packet_dump.h"
#include "WorldServer.h"
#include "Client.h"
#include <list>

using namespace std;

/**
 * Server manager class, deals with management of the world servers.
 */
class ServerManager
{
public:
	/**
	 * Constructor, sets up the TCP server and starts listening.
	 */
	ServerManager();

	/**
	 * Destructor, shuts down the TCP server.
	 */
	~ServerManager();

	/**
	 * Does basic processing for all the servers.
	 */
	void Process();

	/**
	 * Sends a request to world to see if the client is banned or suspended.
	 */
	void SendUserToWorldRequest(unsigned int server_id, unsigned int client_account_id);

	/**
	 * Creates a server list packet for the client.
	 */
	EQApplicationPacket *CreateServerListPacket(Client *c);

private:
	/**
	 * Processes all the disconnected connections in Process(), not used outside.
	 */
	void ProcessDisconnect();

	/**
	 * Retrieves a server(if exists) by ip address
	 * Useful utility for the reconnect process.
	 */
	WorldServer* GetServerByAddress(unsigned int address);

	EmuTCPServer* tcps;
	list<WorldServer*> world_servers;
	unsigned int unique_id_counter;
};

#endif

