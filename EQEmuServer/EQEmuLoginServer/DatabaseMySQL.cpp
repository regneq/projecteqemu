#include "../common/debug.h"
#include "Database.h"

#ifdef EQEMU_MYSQL_ENABLED
#include "DatabaseMySQL.h"
#include "ErrorLog.h"
#include "LoginServer.h"

extern ErrorLog *log;
extern LoginServer server;

#pragma comment(lib, "mysqlclient.lib")

DatabaseMySQL::DatabaseMySQL(string user, string pass, string host, string port, string name)
{
	this->user = user;
	this->pass = pass;
	this->host = host;
	this->name = name;

	db = mysql_init(NULL);
	if(db)
	{
		my_bool r = 1;
		mysql_options(db, MYSQL_OPT_RECONNECT, &r);
		if(!mysql_real_connect(db, host.c_str(), user.c_str(), pass.c_str(), name.c_str(), atoi(port.c_str()), NULL, 0)) 
		{
			mysql_close(db);
			log->Log(log_database, "Failed to connect to MySQL database.");
		}
	}
	else
	{
		log->Log(log_database, "Failed to create db object in MySQL database.");
	}
}

DatabaseMySQL::~DatabaseMySQL()
{
	if(db)
	{
		mysql_close(db);
	}
}

bool DatabaseMySQL::GetLoginDataFromAccountName(string name, string &password, unsigned int &id)
{
	if(!db)
	{
		return false;
	}

	MYSQL_RES *res;
	MYSQL_ROW row;
	stringstream query(stringstream::in | stringstream::out);
	query << "SELECT LoginServerID, AccountPassword FROM " << server.options.GetAccountTable() << " WHERE AccountName = '";
	query << name;
	query << "'";
	
	if(mysql_query(db, query.str().c_str()) != 0)
	{
		log->Log(log_database, "Mysql query failed: %s", query.str().c_str());
		return false;
	}

	res = mysql_use_result(db);

	if(res)
	{
		while((row = mysql_fetch_row(res)) != NULL)
		{
			id = atoi(row[0]);
			password = row[1];
			mysql_free_result(res);
			return true;
		}
	}

	log->Log(log_database, "Mysql query returned no result: %s", query.str().c_str());
	return false;
}

bool DatabaseMySQL::GetWorldRegistration(string long_name, string short_name, unsigned int &id, string &desc, unsigned int &list_id, 
		string &list_desc, string &account, string &password)
{
	if(!db)
	{
		return false;
	}

	MYSQL_RES *res;
	MYSQL_ROW row;
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
	
	if(mysql_query(db, query.str().c_str()) != 0)
	{
		log->Log(log_database, "Mysql query failed: %s", query.str().c_str());
		return false;
	}

	res = mysql_use_result(db);
	if(res)
	{
		while((row = mysql_fetch_row(res)) != NULL)
		{
			id = atoi(row[0]);
			desc = row[1];
			list_id = atoi(row[2]);
			list_desc = row[3];
			account = row[4]; 
			password = row[5];
			mysql_free_result(res);
			return true;
		}
	}

	log->Log(log_database, "Mysql query returned no result: %s", query.str().c_str());
	return false;
}

void DatabaseMySQL::UpdateLSAccountData(unsigned int id, string ip_address)
{
	if(!db)
	{
		return;
	}

	stringstream query(stringstream::in | stringstream::out);
	query << "UPDATE " << server.options.GetAccountTable() << " SET LastIPAddress = '";
	query << ip_address;
	query << "', LastLoginDate = now() where LoginServerID = ";
	query << id;

	if(mysql_query(db, query.str().c_str()) != 0)
	{
		log->Log(log_database, "Mysql query failed: %s", query.str().c_str());
	}
}

void DatabaseMySQL::UpdateWorldRegistration(unsigned int id, string ip_address)
{
	if(!db)
	{
		return;
	}

	stringstream query(stringstream::in | stringstream::out);
	query << "UPDATE " << server.options.GetWorldRegistrationTable() << " SET ServerLastLoginDate = now(), ServerLastIPAddr = '";
	query << ip_address;
	query << "' where ServerID = ";
	query << id;

	if(mysql_query(db, query.str().c_str()) != 0)
	{
		log->Log(log_database, "Mysql query failed: %s", query.str().c_str());
	}
}

#endif

