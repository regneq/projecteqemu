#ifndef EQEMU_DATABASE_H
#define EQEMU_DATABASE_H

#include <string>

using namespace std;

#define EQEMU_MYSQL_ENABLED
//#define EQEMU_POSTGRESQL_ENABLED

/**
 * Base database class, intended to be extended.
 */
class Database
{
public:
	Database() : user(""), pass(""), host(""), port(""), name("") { }
	~Database() { }

	/**
	 * Returns true if the database successfully connected.
	 */
	virtual bool IsConnected() { return false; }

	/**
	 * Retrieves the login data (password hash and account id) from the account name provided
	 * Needed for client login procedure.
	 * Returns true if the record was found, false otherwise.
	 */
	virtual bool GetLoginDataFromAccountName(string name, string &password, unsigned int &id) { return false; }

	/**
	 * Retrieves the world registration from the long and short names provided.
	 * Needed for world login procedure.
	 * Returns true if the record was found, false otherwise.
	 */
	virtual bool GetWorldRegistration(string long_name, string short_name, unsigned int &id, string &desc, unsigned int &list_id, 
		string &list_desc, string &account, string &password) { return false; }

	/**
	 * Updates the ip address of the client with account id = id
	 */
	virtual void UpdateLSAccountData(unsigned int id, string ip_address) { }

	/**
	 * Updates the ip address of the world with account id = id
	 */
	virtual void UpdateWorldRegistration(unsigned int id, string ip_address) { }
protected:
	string user, pass, host, port, name;
};

#endif

