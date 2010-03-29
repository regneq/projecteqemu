#ifndef EQEMU_WORLDSERVER_H
#define EQEMU_WORLDSERVER_H

#include "../common/debug.h"
#include "../common/EQStreamFactory.h"
#include "../common/EmuTCPConnection.h"
#include "../common/EmuTCPServer.h"
#include "../common/servertalk.h"
#include "../common/packet_dump.h"
#include <string>

using namespace std;

/**
 * World server class, controls the connected server processing.
 */
class WorldServer
{
public:
	/**
	 * Constructor, sets our connection to c.
	 */
	WorldServer(EmuTCPConnection *c);

	/**
	 * Destructor, frees our connection if it exists.
	 */
	~WorldServer();

	/**
	 * Resets the basic stats of this server.
	 */
	void Reset();

	/**
	 * Does processing of all the connections for this world.
	 * Returns true except for a fatal error that requires disconnection.
	 */
	bool Process();

	/**
	 * Accesses connection, it is intentional that this is not const (trust me).
	 */
	EmuTCPConnection *GetConnection() { return connection; }
	
	/**
	 * Sets the connection to c.
	 */
	void SetConnection(EmuTCPConnection *c) { connection = c; }

	/**
	 * Gets the runtime id of this server.
	 */
	unsigned int GetRuntimeID() const { return runtime_id; }
	
	/**
	 * Sets the runtime id of this server.
	 */
	void SetRuntimeID(unsigned int id) { runtime_id = id; }

	/**
	 * Gets the long name of the server.
	 */
	string GetLongName() const { return long_name; }

	/**
	 * Gets whether the server is authorized to show up on the server list or not.
	 */
	bool IsAuthorized() const { return authorized; }

	/**
	 * Gets the local ip of the server.
	 */
	string GetLocalIP() const { return local_ip; }

	/**
	 * Gets the remote ip of the server.
	 */
	string GetRemoteIP() const { return remote_ip; }

	/**
	 * Gets what kind of server this server is (legends, preferred, normal)
	 */
	unsigned int GetServerListID() const { return server_list_id; }

	/**
	 * Gets the status of the server.
	 */
	int GetStatus() const { return status; }

	/**
	 * Gets the number of zones online on the server.
	 */
	unsigned int GetZonesBooted() const { return zones_booted; }

	/**
	 * Gets the number of players on the server.
	 */
	unsigned int GetPlayersOnline() const { return players_online; }

	/**
	 * Takes the info struct we recieved from world and processes it.
	 */
	void Handle_NewLSInfo(ServerNewLSInfo_Struct* i);

	/**
	 * Takes the status struct we recieved from world and processes it.
	 */
	void Handle_LSStatus(ServerLSStatus_Struct *s);

	/**
	 * Informs world that there is a client incoming with the following data.
	 */
	void SendClientAuth(unsigned int ip, string account, string key, unsigned int account_id);

private:

	EmuTCPConnection *connection;
	unsigned int zones_booted;
	unsigned int players_online;
	int status;
	unsigned int runtime_id;
	unsigned int id;
	unsigned int server_list_id;
	unsigned int server_type;
	string desc;
	string long_name;
	string short_name;
	string account_name;
	string account_password;
	string remote_ip;
	string local_ip;
	string protocol;
	string version;
	bool authorized;
	bool logged_in;
};

#endif

