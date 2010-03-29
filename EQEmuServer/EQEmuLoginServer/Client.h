#ifndef EQEMU_CLIENT_H
#define EQEMU_CLIENT_H

#include "../common/debug.h"
#include "../common/opcodemgr.h"
#include "../common/EQStreamType.h"
#include "../common/EQStreamFactory.h"
#ifndef WIN32
#include "EQCryptoAPI.h"
#endif
#include <string>

using namespace std;

enum ClientVersion
{
	cv_titanium,
	cv_sod
};

enum ClientStatus
{
	cs_not_sent_session_ready,
	cs_waiting_for_login,
	cs_logged_in,
	cs_play_request
};

/**
 * Client class, controls a single client and it's 
 * connection to the login server.
 */
class Client
{
public:
	/**
	 * Constructor, sets our connection to c and version to v
	 */
	Client(EQStream *c, ClientVersion v);

	/**
	 * Destructor.
	 */
	~Client() { }

	/**
	 * Processes the client's connection and does various actions.
	 */
	bool Process();

	/**
	 * Sends our reply to session ready packet.
	 */
	void Handle_SessionReady();

	/**
	 * Verifies login and send a reply.
	 */
	void Handle_Login(const char* data, unsigned int size);

	/**
	 * Sends a packet to the requested server to see if the client is allowed or not.
	 */
	void Handle_Play(const char* data);

	/**
	 * Sends a server list packet to the client.
	 */
	void SendServerListPacket();

	/**
	 * Sends the input packet to the client and clears our play response states.
	 */
	void SendPlayResponse(EQApplicationPacket *outapp);

	/**
	 * Generates a random login key for the client during login.
	 */
	void GenerateKey();

	/**
	 * Gets the account id of this client.
	 */
	unsigned int GetAccountID() const { return account_id; }

	/**
	 * Gets the account name of this client.
	 */
	string GetAccountName() const { return account_name; }

	/**
	 * Gets the key generated at login for this client.
	 */
	string GetKey() const { return key; }

	/**
	 * Gets the server selected to be played on for this client.
	 */
	unsigned int GetPlayServerID() const { return play_server_id; }

	/**
	 * Gets the play sequence state for this client.
	 */
	unsigned int GetPlaySequence() const { return play_sequence_id; }

	/**
	 * Gets the connection for this client.
	 */
	EQStream *GetConnection() { return connection; }
private:
	EQStream *connection;
	ClientVersion version;
	ClientStatus status;

	string account_name;
	unsigned int account_id;
	unsigned int play_server_id;
	unsigned int play_sequence_id;
	string key;
};

#endif

