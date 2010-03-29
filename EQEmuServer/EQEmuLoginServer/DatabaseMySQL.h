#ifndef EQEMU_DATABASEMYSQL_H
#define EQEMU_DATABASEMYSQL_H

#include "Database.h"
#ifdef EQEMU_MYSQL_ENABLED

#include <string>
#include <sstream>
#include <stdlib.h>
#include <mysql.h>

using namespace std;

/**
 * Mysql Database class
 */
class DatabaseMySQL : public Database
{
public:
	/**
	 * Constructor, sets our database to null.
	 */
	DatabaseMySQL() { db = NULL; }

	/**
	 * Constructor, tries to set our database to connect to the supplied options.
	 */
	DatabaseMySQL(string user, string pass, string host, string port, string name);

	/**
	 * Destructor, frees our database if needed.
	 */
	~DatabaseMySQL();

	/**
	 * @return Returns true if the database successfully connected.
	 */
	virtual bool IsConnected() { return (db != NULL); }

	/**
	 * Retrieves the login data (password hash and account id) from the account name provided
	 * Needed for client login procedure.
	 * Returns true if the record was found, false otherwise.
	 */
	virtual bool GetLoginDataFromAccountName(string name, string &password, unsigned int &id);

	/**
	 * Retrieves the world registration from the long and short names provided.
	 * Needed for world login procedure.
	 * Returns true if the record was found, false otherwise.
	 */
	virtual bool GetWorldRegistration(string long_name, string short_name, unsigned int &id, string &desc, unsigned int &list_id, 
		string &list_desc, string &account, string &password);

	/**
	 * Updates the ip address of the client with account id = id
	 */
	virtual void UpdateLSAccountData(unsigned int id, string ip_address);

	/**
	 * Updates the ip address of the world with account id = id
	 */
	virtual void UpdateWorldRegistration(unsigned int id, string ip_address);
protected:
	string user, pass, host, port, name;
	MYSQL *db;
};

#endif
#endif

