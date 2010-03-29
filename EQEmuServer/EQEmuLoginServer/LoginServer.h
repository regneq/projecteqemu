#ifndef EQEMU_LOGINSERVER_H
#define EQEMU_LOGINSERVER_H

#include "ErrorLog.h"
#include "Config.h"
#include "Database.h"
#include "DatabaseMySQL.h"
#include "DatabasePostgreSQL.h"
#include "Encryption.h"
#include "Options.h"
#include "ServerManager.h"
#include "ClientManager.h"

/**
 * Login server struct, contains every variable for the server that needs to exist 
 * outside the scope of main().
 */
struct LoginServer
{
public:
	/**
	 * I don't really like how this looks with all the ifdefs...
	 * but it's the most trivial way to do this.
	 */
#ifdef WIN32
	LoginServer() : config(NULL), db(NULL), eq_crypto(NULL), SM(NULL) { }
#else
	LoginServer() : config(NULL), db(NULL) { }
#endif

	Config *config;
	Database *db;
	Options options;
	ServerManager *SM;
	ClientManager *CM;

#ifdef WIN32
	Encryption *eq_crypto;
#endif
};

#endif

