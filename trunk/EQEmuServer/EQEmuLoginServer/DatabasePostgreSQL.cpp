#include "../common/debug.h"
#include "Database.h"

#ifdef EQEMU_POSTGRESQL_ENABLED
#include "DatabasePostgreSQL.h"
#include "ErrorLog.h"
#include "LoginServer.h"

extern ErrorLog *log;
extern LoginServer server;

#pragma comment(lib, "libpq.lib")

DatabasePostgreSQL::DatabasePostgreSQL(string user, string pass, string host, string port, string name)
{
	db = NULL;
	db = PQsetdbLogin(host.c_str(), port.c_str(), NULL, NULL, name.c_str(), user.c_str(), pass.c_str());
	if(!db)
	{
		log->Log(log_database, "Failed to connect to PostgreSQL Database.");
	}

	if(PQstatus(db) != CONNECTION_OK)
	{
		log->Log(log_database, "Failed to connect to PostgreSQL Database.");
		PQfinish(db);
		db = NULL;
	}
}

DatabasePostgreSQL::~DatabasePostgreSQL()
{
	if(db)
	{
		PQfinish(db);
	}
}

bool DatabasePostgreSQL::GetLoginDataFromAccountName(string name, string &password, unsigned int &id)
{
	if(!db)
	{
		return false;
	}

	/**
	 * PostgreSQL doesn't have automatic reconnection option like mysql 
	 * but it's easy to check and reconnect
	 */
	if(PQstatus(db) != CONNECTION_OK)
	{
		PQreset(db);
		if(PQstatus(db) != CONNECTION_OK)
		{
			return false;
		}
	}

	stringstream query(stringstream::in | stringstream::out);
	query << "SELECT LoginServerID, AccountPassword FROM " << server.options.GetAccountTable() << " WHERE AccountName = '";
	query << name;
	query << "'";

	PGresult *res = PQexec(db, query.str().c_str());

	char *error = PQresultErrorMessage(res);
	if(strlen(error) > 0)
	{
		log->Log(log_database, "Database error in DatabasePostgreSQL::GetLoginDataFromAccountName(): %s", error);
		PQclear(res);
		return false;
	}

	if(PQntuples(res) > 0)
	{
		id = atoi(PQgetvalue(res, 0, 0));
		password = PQgetvalue(res, 0, 1);
		PQclear(res);
		return true;
	}

	PQclear(res);
	return false;
}

bool DatabasePostgreSQL::GetWorldRegistration(string long_name, string short_name, unsigned int &id, string &desc, unsigned int &list_id, 
		string &list_desc, string &account, string &password)
{
	if(!db)
	{
		return false;
	}

	/**
	 * PostgreSQL doesn't have automatic reconnection option like mysql 
	 * but it's easy to check and reconnect
	 */
	if(PQstatus(db) != CONNECTION_OK)
	{
		PQreset(db);
		if(PQstatus(db) != CONNECTION_OK)
		{
			return false;
		}
	}

	stringstream query(stringstream::in | stringstream::out);
	query << "SELECT WSR.ServerID, WSR.ServerTagDescription, SLT.ServerListTypeID, ";
	query << "SLT.ServerListTypeDescription, SAR.AccountName, SAR.AccountPassword FROM " << server.options.GetWorldRegistrationTable();
	query << " AS WSR JOIN " << server.options.GetWorldServerTypeTable() << " AS SLT ON WSR.ServerListTypeID = SLT.ServerListTypeID JOIN ";
	query << server.options.GetWorldAdminRegistrationTable() << " AS SAR ON WSR.ServerAdminID = SAR.ServerAdminID WHERE WSR.ServerLongName";
	query << " = '";
	query << long_name;
	query << "' AND WSR.ServerShortName = '";
	query << short_name;
	query << "'";

	PGresult *res = PQexec(db, query.str().c_str());

	char *error = PQresultErrorMessage(res);
	if(strlen(error) > 0)
	{
		log->Log(log_database, "Database error in DatabasePostgreSQL::GetWorldRegistration(): %s", error);
		PQclear(res);
		return false;
	}

	if(PQntuples(res) > 0)
	{
		id = atoi(PQgetvalue(res, 0, 0));
		desc = PQgetvalue(res, 0, 1);
		list_id = atoi(PQgetvalue(res, 0, 2));
		list_desc = PQgetvalue(res, 0, 3);
		account = PQgetvalue(res, 0, 4); 
		password = PQgetvalue(res, 0, 5);

		PQclear(res);
		return true;
	}

	PQclear(res);
	return false;
}

void DatabasePostgreSQL::UpdateLSAccountData(unsigned int id, string ip_address)
{
	if(!db)
	{
		return;
	}

	/**
	 * PostgreSQL doesn't have automatic reconnection option like mysql 
	 * but it's easy to check and reconnect
	 */
	if(PQstatus(db) != CONNECTION_OK)
	{
		PQreset(db);
		if(PQstatus(db) != CONNECTION_OK)
		{
			return;
		}
	}

	stringstream query(stringstream::in | stringstream::out);
	query << "UPDATE " << server.options.GetAccountTable() << " SET LastIPAddress = '";
	query << ip_address;
	query << "', LastLoginDate = current_date where LoginServerID = ";
	query << id;
	PGresult *res = PQexec(db, query.str().c_str());

	char *error = PQresultErrorMessage(res);
	if(strlen(error) > 0)
	{
		log->Log(log_database, "Database error in DatabasePostgreSQL::GetLoginDataFromAccountName(): %s", error);
	}
	PQclear(res);
}

void DatabasePostgreSQL::UpdateWorldRegistration(unsigned int id, string ip_address)
{
	if(!db)
	{
		return;
	}

	/**
	 * PostgreSQL doesn't have automatic reconnection option like mysql 
	 * but it's easy to check and reconnect
	 */
	if(PQstatus(db) != CONNECTION_OK)
	{
		PQreset(db);
		if(PQstatus(db) != CONNECTION_OK)
		{
			return;
		}
	}

	stringstream query(stringstream::in | stringstream::out);
	query << "UPDATE " << server.options.GetWorldRegistrationTable() << " SET ServerLastLoginDate = current_date, ServerLastIPAddr = '";
	query << ip_address;
	query << "' where ServerID = ";
	query << id;
	PGresult *res = PQexec(db, query.str().c_str());

	char *error = PQresultErrorMessage(res);
	if(strlen(error) > 0)
	{
		log->Log(log_database, "Database error in DatabasePostgreSQL::GetLoginDataFromAccountName(): %s", error);
	}
	PQclear(res);
}

#endif

