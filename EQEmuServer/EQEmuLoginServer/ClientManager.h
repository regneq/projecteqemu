#ifndef EQEMU_CLIENTMANAGER_H
#define EQEMU_CLIENTMANAGER_H

#include "../common/debug.h"
#include "../common/opcodemgr.h"
#include "../common/EQStreamType.h"
#include "../common/EQStreamFactory.h"
#include "Client.h"
#include <list>

using namespace std;

/**
 * Client manager class, holds all the client objects and does basic processing.
 */
class ClientManager
{
public:
	/**
	 * Constructor, sets up the stream factories and opcode managers.
	 */
	ClientManager();

	/**
	 * Destructor, shuts down the streams and opcode managers.
	 */
	~ClientManager();

	/**
	 * Processes every client in the internal list, removes them if necessary.
	 */
	void Process();

	/**
	 * Gets a client (if exists) by their account id.
	 */
	Client *GetClient(unsigned int account_id);
private:

	/**
	 * Processes disconnected clients, removes them if necessary.
	 */
	void ProcessDisconnect();

	list<Client*> clients;
	OpcodeManager *titanium_ops;
	EQStreamFactory *titanium_stream;
	OpcodeManager *sod_ops;
	EQStreamFactory *sod_stream;
};

#endif

