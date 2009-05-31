/*  EQEMu:  Everquest Server Emulator
	Copyright (C) 2001-2003  EQEMu Development Team (http://eqemulator.net)

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
#include "../common/debug.h"
#include <iostream>
using namespace std;
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errmsg.h>
#include <mysqld_error.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>
#include <map>

// Disgrace: for windows compile
#ifdef WIN32
#include <windows.h>
#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#else
#include "unix.h"
#include <netinet/in.h>
#include <sys/time.h>
#endif

#include "database.h"
#include "eq_packet_structs.h"
#include "guilds.h"
#include "MiscFunctions.h"
#include "extprofile.h"
/*#include "../common/classes.h"
#include "../common/races.h"
#include "../common/files.h"
#include "../common/EQEMuError.h"
#include "../common/packet_dump.h"
*/
extern Client client;

/*
This is the amount of time in seconds the client has to enter the zone
server after the world server, or inbetween zones when that is finished
*/

/*
Establish a connection to a mysql database with the supplied parameters

  Added a very simple .ini file parser - Bounce

	Modify to use for win32 & linux - misanthropicfiend
*/
Database::Database ()
{
	DBInitVars();
}

/*
Establish a connection to a mysql database with the supplied parameters
*/

Database::Database(const char* host, const char* user, const char* passwd, const char* database, int32 port)
{
	DBInitVars();
	Connect(host, user, passwd, database, port);
}

bool Database::Connect(const char* host, const char* user, const char* passwd, const char* database, int32 port)
{
	int32 errnum= 0;
	char errbuf[MYSQL_ERRMSG_SIZE];
	if (!Open(host, user, passwd, database, port, &errnum, errbuf))
	{
		LogFile->write(EQEMuLog::Error, "Failed to connect to database: Error: %s", errbuf);
		HandleMysqlError(errnum);

		return false;
	}
	else
	{
		LogFile->write(EQEMuLog::Status, "Using database '%s' at %s:%d",database,host,port);
		return true;
	}
}

void Database::DBInitVars() {

	max_zonename = 0;
	zonename_array = 0;
	varcache_array = 0;
	varcache_max = 0;
	varcache_lastupdate = 0;
}



void Database::HandleMysqlError(int32 errnum) {
/*	switch(errnum) {
		case 0:
			break;
		case 1045: // Access Denied
		case 2001: {
			AddEQEMuError(EQEMuError_Mysql_1405, true);
			break;
		}
		case 2003: { // Unable to connect
			AddEQEMuError(EQEMuError_Mysql_2003, true);
			break;
		}
		case 2005: { // Unable to connect
			AddEQEMuError(EQEMuError_Mysql_2005, true);
			break;
		}
		case 2007: { // Unable to connect
			AddEQEMuError(EQEMuError_Mysql_2007, true);
			break;
		}
	}*/
}

/*

Close the connection to the database
*/
Database::~Database()
{
	unsigned int x;
	if (zonename_array) {
		for (x=0; x<=max_zonename; x++) {
			if (zonename_array[x])
				safe_delete_array(zonename_array[x]);
		}
		safe_delete_array(zonename_array);
	}
	if (varcache_array) {
		for (x=0; x<varcache_max; x++) {
			safe_delete_array(varcache_array[x]);
		}
		safe_delete_array(varcache_array);
	}
}


/*
Check if there is an account with name "name" and password "password"
Return the account id or zero if no account matches.
Zero will also be returned if there is a database error.
*/
int32 Database::CheckLogin(const char* name, const char* password, sint16* oStatus) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;

	if(strlen(name) >= 50 || strlen(password) >= 50)
		return(0);

	char tmpUN[100];
	char tmpPW[100];
	DoEscapeString(tmpUN, name, strlen(name));
	DoEscapeString(tmpPW, password, strlen(password));

	if (RunQuery(query, MakeAnyLenString(&query,
		"SELECT id, status FROM account WHERE name='%s' AND password is not null "
		"and length(password) > 0 and (password='%s' or password=MD5('%s'))",
		tmpUN, tmpPW, tmpPW), errbuf, &result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1)
		{
			row = mysql_fetch_row(result);
			int32 id = atoi(row[0]);
			if (oStatus)
				*oStatus = atoi(row[1]);
			mysql_free_result(result);
			return id;
		}
		else
		{
			mysql_free_result(result);
			return 0;
		}
		mysql_free_result(result);
	}
	else
	{
		cerr << "Error in CheckLogin query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
		return false;
	}

	return 0;
}


//Lieka:  Get Banned IP Address List - Only return false if the incoming connection's IP address is not present in the banned_ips table.
bool Database::CheckBannedIPs(const char* loginIP)
{
 	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
 	//cout << "Checking against Banned IPs table."<< endl; //Lieka:  Debugging
 	if (RunQuery(query, MakeAnyLenString(&query, "SELECT ip_address FROM Banned_IPs WHERE ip_address='%s'", loginIP), errbuf, &result)) {
 		safe_delete_array(query);
 		if (mysql_num_rows(result) != 0)
 		{
 			//cout << loginIP << " was present in the banned IPs table" << endl; //Lieka:  Debugging
 			mysql_free_result(result);
 			return true;
 		}
 		else
 		{
 			//cout << loginIP << " was not present in the banned IPs table." << endl; //Lieka:  Debugging
 			mysql_free_result(result);
 			return false;
 		}
 		mysql_free_result(result);
 	}
 	else
 	{
 		cerr << "Error in CheckBannedIPs query '" << query << "' " << errbuf << endl;
 		safe_delete_array(query);
 		return true;
 	}
 	return true;
}

bool Database::AddBannedIP(char* bannedIP, const char* notes)
{
 	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;

 	if (!RunQuery(query, MakeAnyLenString(&query, "INSERT into Banned_IPs SET ip_address='%s', notes='%s'", bannedIP, notes), errbuf)) {
 		cerr << "Error in ReserveName query '" << query << "' " << errbuf << endl;
 		safe_delete_array(query);
 		return false;
 	}
 	safe_delete_array(query);
 	return true;
}
 //End Lieka Edit
 
 bool Database::CheckGMIPs(const char* ip_address, int32 account_id) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT * FROM `gm_ips` WHERE `ip_address` = '%s' AND `account_id` = %i", ip_address, account_id), errbuf, &result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1) {
			mysql_free_result(result);
			return true;
		} else {
			mysql_free_result(result);
			return false;
		}
		mysql_free_result(result);

	} else {
		safe_delete_array(query);
		return false;
	}
	
	return false;
}

bool Database::AddGMIP(char* ip_address, char* name) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if (!RunQuery(query, MakeAnyLenString(&query, "INSERT into `gm_ips` SET `ip_address` = '%s', `name` = '%s'", ip_address, name), errbuf)) {
		safe_delete_array(query);
		return false;
	}
	safe_delete_array(query);
	return true;
}

sint16 Database::CheckStatus(int32 account_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT status FROM account WHERE id='%i'", account_id), errbuf, &result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1)
		{
			row = mysql_fetch_row(result);
			sint16 status = atoi(row[0]);

			mysql_free_result(result);
			return status;
		}
		else
		{
			mysql_free_result(result);
			return 0;
		}
		mysql_free_result(result);
	}
	else
	{
		cerr << "Error in CheckStatus query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
		return false;
	}

	return 0;
}

int32 Database::CreateAccount(const char* name, const char* password, sint16 status, int32 lsaccount_id) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
	int32 querylen;
	int32 last_insert_id;

	if (password)
		querylen = MakeAnyLenString(&query, "INSERT INTO account SET name='%s', password='%s', status=%i, lsaccount_id=%i;",name,password,status, lsaccount_id);
	else
		querylen = MakeAnyLenString(&query, "INSERT INTO account SET name='%s', status=%i, lsaccount_id=%i;",name, status, lsaccount_id);

	cerr << "Account Attempting to be created:" << name << " " << (sint16) status << endl;
	if (!RunQuery(query, querylen, errbuf, 0, 0, &last_insert_id)) {
		cerr << "Error in CreateAccount query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
		return 0;
	}
	safe_delete_array(query);

	if (last_insert_id == 0) {
		cerr << "Error in CreateAccount query '" << query << "' " << errbuf << endl;
		return 0;
	}

	return last_insert_id;
}

bool Database::DeleteAccount(const char* name) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
	int32 affected_rows = 0;

	cerr << "Account Attempting to be deleted:" << name << endl;
	if (RunQuery(query, MakeAnyLenString(&query, "DELETE FROM account WHERE name='%s';",name), errbuf, 0, &affected_rows)) {
		safe_delete_array(query);
		if (affected_rows == 1) {
			return true;
		}
	}
	else {

		cerr << "Error in DeleteAccount query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
	}

	return false;
}

bool Database::SetLocalPassword(int32 accid, const char* password) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;

	if (!RunQuery(query, MakeAnyLenString(&query, "UPDATE account SET password=MD5('%s') where id=%i;", password, accid), errbuf)) {
		cerr << "Error in SetLocalPassword query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
		return false;
	}

	safe_delete_array(query);
	return true;
}

bool Database::SetAccountStatus(const char* name, sint16 status) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	int32	affected_rows = 0;

	cout << "Account being GM Flagged:" << name << ", Level: " << (sint16) status << endl;
	if (!RunQuery(query, MakeAnyLenString(&query, "UPDATE account SET status=%i WHERE name='%s';", status, name), errbuf, 0, &affected_rows)) {
		safe_delete_array(query);
		return false;
	}
	safe_delete_array(query);

	if (affected_rows == 0) {
		cout << "Account: " << name << " does not exist, therefore it cannot be flagged\n";
		return false;
	}

	return true;
}


//---------------------------------
//End of adventure database code.--
//---------------------------------


bool Database::ReserveName(int32 account_id, char* name)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;

	if (!RunQuery(query, MakeAnyLenString(&query, "INSERT into character_ SET account_id=%i, name='%s', profile=NULL", account_id, name), errbuf)) {
		cerr << "Error in ReserveName query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
		return false;
	}
	safe_delete_array(query);
	return true;
}

/*
Delete the character with the name "name"
returns false on failure, true otherwise
*/
bool Database::DeleteCharacter(char *name)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query=0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int charid, matches;
	int32 affected_rows;

	if(!name ||	!strlen(name))
	{
		printf("DeleteCharacter: request to delete without a name (empty char slot)\n");
		return false;
	}

// SCORPIOUS2K - get id from character_ before deleting record so we can clean up inventory and qglobal

#if DEBUG >= 5
	printf("DeleteCharacter: Attempting to delete '%s'\n", name);
#endif
	RunQuery(query, MakeAnyLenString(&query, "SELECT id from character_ WHERE name='%s'", name), errbuf, &result);
	if (query)
	{
		safe_delete_array(query);
		query = NULL;
	}
	matches = mysql_num_rows(result);
	if(matches == 1)
	{
		row = mysql_fetch_row(result);
		charid = atoi(row[0]);
#if DEBUG >= 5
		printf("DeleteCharacter: found '%s' with char id: %d\n", name, charid);
#endif
	}
	else
	{
		printf("DeleteCharacter: error: got %d rows matching '%s'\n", matches, name);
		if(result)
		{
			mysql_free_result(result);
			result = NULL;
		}
		return false;
	}

	if(result)
	{
		mysql_free_result(result);
		result = NULL;
	}



#if DEBUG >= 5
	printf("DeleteCharacter: deleting '%s' (id %d): ", name, charid);
	printf(" quest_globals");
#endif
	RunQuery(query, MakeAnyLenString(&query, "DELETE from quest_globals WHERE charid='%d'", charid), errbuf, NULL, &affected_rows);
	if(query)
	{
		safe_delete_array(query);
		query = NULL;
	}

#if DEBUG >= 5
	printf(" character_tasks");
#endif
	RunQuery(query, MakeAnyLenString(&query, "DELETE from character_tasks WHERE charid='%d'", charid), errbuf, NULL, &affected_rows);
	if(query)
	{
		safe_delete_array(query);
		query = NULL;
	}

#if DEBUG >= 5
	printf(" character_activities");
#endif
	RunQuery(query, MakeAnyLenString(&query, "DELETE from character_activities WHERE charid='%d'", charid), errbuf, NULL, &affected_rows);
	if(query)
	{
		safe_delete_array(query);
		query = NULL;
	}

#if DEBUG >= 5
	printf(" character_enabledtasks");
#endif
	RunQuery(query, MakeAnyLenString(&query, "DELETE from character_enabledtasks WHERE charid='%d'", charid), errbuf, NULL, &affected_rows);
	if(query)
	{
		safe_delete_array(query);
		query = NULL;
	}

#if DEBUG >= 5
	printf(" completed_tasks");
#endif
	RunQuery(query, MakeAnyLenString(&query, "DELETE from completed_tasks WHERE charid='%d'", charid), errbuf, NULL, &affected_rows);
	if(query)
	{
		safe_delete_array(query);
		query = NULL;
	}

#if DEBUG >= 5
	printf(" ptimers");
#endif
	RunQuery(query, MakeAnyLenString(&query, "DELETE from timers WHERE char_id='%d'", charid), errbuf, NULL, &affected_rows);
	if(query)
	{
		safe_delete_array(query);
		query = NULL;
	}

#if DEBUG >= 5
	printf(" inventory");
#endif
	RunQuery(query, MakeAnyLenString(&query, "DELETE from inventory WHERE charid='%d'", charid), errbuf, NULL, &affected_rows);
	if(query)
	{
		safe_delete_array(query);
		query = NULL;
	}

#if DEBUG >= 5
	printf(" guild_members");
#endif
	RunQuery(query, MakeAnyLenString(&query, "DELETE FROM guild_members WHERE char_id='%d'", charid), errbuf, NULL, &affected_rows);
	if(query)
	{
		safe_delete_array(query);
		query = NULL;
	}

#if DEBUG >= 5
	printf(" _character");
#endif
	RunQuery(query, MakeAnyLenString(&query, "DELETE from character_ WHERE id='%d'", charid), errbuf, NULL, &affected_rows);
	if(query)
	{
		safe_delete_array(query);
		query = NULL;
	}
	if(affected_rows != 1)	// here we have to have a match or it's an error
	{
		LogFile->write(EQEMuLog::Error, "DeleteCharacter: error: delete operation affected %d rows\n", affected_rows);
		return false;
	}
#if DEBUG >= 5
    printf(" keyring");
#endif
    RunQuery(query, MakeAnyLenString(&query, "DELETE FROM keyring WHERE char_id='%d'", charid), errbuf, NULL, &affected_rows);
    if(query)
    {
        safe_delete_array(query);
        query = NULL;
    }
#if DEBUG >= 5
	printf("\n");
#endif
	printf("DeleteCharacter: successfully deleted '%s' (id %d)\n", name, charid);

	return true;
}
// Store new character information into the character_ and inventory tables
bool Database::StoreCharacter(uint32 account_id, PlayerProfile_Struct* pp, Inventory* inv, ExtendedProfile_Struct *ext)
{
	_CP(Database_StoreCharacter);
	char errbuf[MYSQL_ERRMSG_SIZE];
	char query[256+sizeof(PlayerProfile_Struct)*2+sizeof(ExtendedProfile_Struct)*2+5];
	char* end = query;
	int32 affected_rows = 0;
	int i;
	int32 charid = 0;
	char* charidquery = 0;
	char* invquery = 0;
	MYSQL_RES *result;
	MYSQL_ROW row = 0;
	char zone[50];
	float x, y, z;

//	memset(&playeraa, 0, sizeof(playeraa));

	// get the char id (used in inventory inserts below)
	if(!RunQuery
	(
		charidquery,
		MakeAnyLenString
		(
			&charidquery,
			"SELECT id FROM character_ where name='%s'",
			pp->name
		),
		errbuf,
		&result
	)) {
		LogFile->write(EQEMuLog::Error, "Error in char store id query: %s: %s", charidquery, errbuf);
		return(false);
	}
	safe_delete_array(charidquery);

	if(mysql_num_rows(result) == 1)
	{
		row = mysql_fetch_row(result);
		if(row[0])
			charid = atoi(row[0]);
	}

	if(!charid)
	{
		LogFile->write(EQEMuLog::Error, "StoreCharacter: no character id");
		return false;
	}

	const char *zname = GetZoneName(pp->zone_id);
	if(zname == NULL) {
		//zone not in the DB, something to prevent crash...
		strncpy(zone, "qeynos", 49);
		pp->zone_id = 1;
	} else
		strncpy(zone, zname, 49);
	x=pp->x;
	y=pp->y;
	z=pp->z;

	// construct the character_ query
	end += sprintf(end,
		"UPDATE character_ SET timelaston=0, "
		"zonename=\'%s\', x=%f, y=%f, z=%f, profile=\'",
		zone, x, y, z
	);
	end += DoEscapeString(end, (char*)pp, sizeof(PlayerProfile_Struct));
	end += sprintf(end, "\', extprofile=\'");
	end += DoEscapeString(end, (char*)ext, sizeof(ExtendedProfile_Struct));
	end += sprintf(end, "\' WHERE account_id=%d AND name='%s'",account_id, pp->name);

	RunQuery(query, (int32) (end - query), errbuf, 0, &affected_rows);

	if(!affected_rows)
	{
		LogFile->write(EQEMuLog::Error, "StoreCharacter query '%s' %s", query, errbuf);
		return false;
	}

	affected_rows = 0;


	// Doodman: Is this even used?
	// now the inventory

	for (i=0; i<=2270;)
	{
		const ItemInst* newinv = inv->GetItem((sint16)i);
		if (newinv)
		{
			MakeAnyLenString
			(
				&invquery,
				"INSERT INTO inventory SET "
				"charid=%0u, slotid=%0d, itemid=%0u, charges=%0d, color=%0u",
				charid, i, newinv->GetItem()->ID,
				newinv->GetCharges(), newinv->GetColor()
			);

			RunQuery(invquery, strlen(invquery), errbuf, 0, &affected_rows);
			if(!affected_rows)
			{
				LogFile->write(EQEMuLog::Error, "StoreCharacter inventory failed.  Query '%s' %s", invquery, errbuf);
			}
			safe_delete_array(invquery);
#if EQDEBUG >= 9
			else
			{
				LogFile->write(EQEMuLog::Debug, "StoreCharacter inventory succeeded.  Query '%s' %s", invquery, errbuf);
			}
#endif
		}

		if(i==30){ //end of standard inventory/cursor, jump to internals of bags/cursor
			i = 251;
			continue;
		} else if(i==340){ //end of internals of bags/cursor, jump to bank slots
			i = 2000;
			continue;
		} else if(i==2023){ //end of bank slots, jump to internals of bank bags
			i = 2031;
			continue;
		}

		i++;
	}

	return true;
}

//0=failure, otherwise returns the char ID for the given char name.
int32 Database::GetCharacterID(const char *name) {
	int32 cid = 0;
	if(GetAccountIDByChar(name, &cid) == 0)
		return(0);
	return(cid);
}

/*
This function returns the account_id that owns the character with
the name "name" or zero if no character with that name was found
Zero will also be returned if there is a database error.
*/
int32 Database::GetAccountIDByChar(const char* charname, int32* oCharID) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT account_id, id FROM character_ WHERE name='%s'", charname), errbuf, &result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1)
		{
			row = mysql_fetch_row(result);
			int32 tmp = atoi(row[0]); // copy to temp var because gotta free the result before exitting this function
			if (oCharID)
				*oCharID = atoi(row[1]);
			mysql_free_result(result);
			return tmp;
		}
		mysql_free_result(result);
	}
	else {
		cerr << "Error in GetAccountIDByChar query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
	}

	return 0;
}

// Retrieve account_id for a given char_id
uint32 Database::GetAccountIDByChar(uint32 char_id) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char* query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;
	uint32 ret = 0;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT account_id FROM character_ WHERE id=%i", char_id), errbuf, &result)) {
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			ret = atoi(row[0]); // copy to temp var because gotta free the result before exitting this function
		}
		mysql_free_result(result);
	}
	else {
		LogFile->write(EQEMuLog::Error, "Error in GetAccountIDByChar query '%s': %s", query, errbuf);
	}

	safe_delete_array(query);
	return ret;
}

int32 Database::GetAccountIDByName(const char* accname, sint16* status, int32* lsid) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;


	for (unsigned int i=0; i<strlen(accname); i++) {
		if ((accname[i] < 'a' || accname[i] > 'z') &&
			(accname[i] < 'A' || accname[i] > 'Z') &&
			(accname[i] < '0' || accname[i] > '9'))
			return 0;
	}

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT id, status, lsaccount_id FROM account WHERE name='%s'", accname), errbuf, &result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			int32 tmp = atoi(row[0]); // copy to temp var because gotta free the result before exitting this function
			if (status)
				*status = atoi(row[1]);
			if (lsid) {
				if (row[2])
					*lsid = atoi(row[2]);
				else
					*lsid = 0;
			}
			mysql_free_result(result);
			return tmp;
		}
		mysql_free_result(result);
	}
	else {
		cerr << "Error in GetAccountIDByAcc query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
	}

	return 0;
}

void Database::GetAccountName(int32 accountid, char* name, int32* oLSAccountID) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT name, lsaccount_id FROM account WHERE id='%i'", accountid), errbuf, &result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);

			strcpy(name, row[0]);
			if (row[1] && oLSAccountID) {
				*oLSAccountID = atoi(row[1]);
			}
		}

		mysql_free_result(result);
	}
	else {
		safe_delete_array(query);
		cerr << "Error in GetAccountName query '" << query << "' " << errbuf << endl;
	}
}

void Database::GetCharName(int32 char_id, char* name) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT name FROM character_ WHERE id='%i'", char_id), errbuf, &result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);

			strcpy(name, row[0]);
		}

		mysql_free_result(result);
	}
	else {
		safe_delete_array(query);
		cerr << "Error in GetCharName query '" << query << "' " << errbuf << endl;
	}

}

bool Database::LoadVariables() {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;

	if (RunQuery(query, LoadVariables_MQ(&query), errbuf, &result)) {
		safe_delete_array(query);
		bool ret = LoadVariables_result(result);
		mysql_free_result(result);
		return ret;
	}
	else {
		cerr << "Error in LoadVariables query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
	}
	return false;
}

int32 Database::LoadVariables_MQ(char** query) {
// the read of this single variable should be atomic... this was causing strange problems
//	LockMutex lock(&Mvarcache);
	return MakeAnyLenString(query, "SELECT varname, value, unix_timestamp() FROM variables where unix_timestamp(ts) >= %d", varcache_lastupdate);
}

bool Database::LoadVariables_result(MYSQL_RES* result) {
	int32 i;
    MYSQL_ROW row;
	LockMutex lock(&Mvarcache);
	if (mysql_num_rows(result) > 0) {
		if (!varcache_array) {
			varcache_max = mysql_num_rows(result);
			varcache_array = new VarCache_Struct*[varcache_max];
			for (i=0; i<varcache_max; i++)
				varcache_array[i] = 0;
		}
		else {
			int32 tmpnewmax = varcache_max + mysql_num_rows(result);
			VarCache_Struct** tmp = new VarCache_Struct*[tmpnewmax];
			for (i=0; i<tmpnewmax; i++)
				tmp[i] = 0;
			for (i=0; i<varcache_max; i++)
				tmp[i] = varcache_array[i];
			VarCache_Struct** tmpdel = varcache_array;
			varcache_array = tmp;
			varcache_max = tmpnewmax;
			delete tmpdel;
		}
		while ((row = mysql_fetch_row(result))) {
			varcache_lastupdate = atoi(row[2]);
			for (i=0; i<varcache_max; i++) {
				if (varcache_array[i]) {
					if (strcasecmp(varcache_array[i]->varname, row[0]) == 0) {
						delete varcache_array[i];
						varcache_array[i] = (VarCache_Struct*) new int8[sizeof(VarCache_Struct) + strlen(row[1]) + 1];
						strn0cpy(varcache_array[i]->varname, row[0], sizeof(varcache_array[i]->varname));
						strcpy(varcache_array[i]->value, row[1]);
						break;
					}
				}
				else {
					varcache_array[i] = (VarCache_Struct*) new int8[sizeof(VarCache_Struct) + strlen(row[1]) + 1];
					strcpy(varcache_array[i]->varname, row[0]);
					strcpy(varcache_array[i]->value, row[1]);
					break;
				}
			}
		}
		int32 max_used = 0;
		for (i=0; i<varcache_max; i++) {
			if (varcache_array[i]) {
				if (i > max_used)
					max_used = i;
			}
		}
		max_used++;
		varcache_max = max_used;
	}
	return true;
}

// Gets variable from 'variables' table
bool Database::GetVariable(const char* varname, char* varvalue, int16 varvalue_len) {
	varvalue[0] = '\0';

	LockMutex lock(&Mvarcache);
	if (strlen(varname) <= 1)
		return false;
	for (int32 i=0; i<varcache_max; i++) {

		if (varcache_array[i]) {
			if (strcasecmp(varcache_array[i]->varname, varname) == 0) {
				snprintf(varvalue, varvalue_len, "%s", varcache_array[i]->value);
				varvalue[varvalue_len-1] = 0;
				return true;
			}
		}
		else
			return false;
	}
	return false;
}

bool Database::SetVariable(const char* varname_in, const char* varvalue_in) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
	int32 affected_rows = 0;

	char *varname,*varvalue;

	varname=(char *)malloc(strlen(varname_in)*2+1);
	varvalue=(char *)malloc(strlen(varvalue_in)*2+1);
	DoEscapeString(varname, varname_in, strlen(varname_in));
	DoEscapeString(varvalue, varvalue_in, strlen(varvalue_in));

	if (RunQuery(query, MakeAnyLenString(&query, "Update variables set value='%s' WHERE varname like '%s'", varvalue, varname), errbuf, 0, &affected_rows)) {
		safe_delete_array(query);
		if (affected_rows == 1) {
			LoadVariables(); // refresh cache
			free(varname);
			free(varvalue);
			return true;
		}
		else {
			if (RunQuery(query, MakeAnyLenString(&query, "Insert Into variables (varname, value) values ('%s', '%s')", varname, varvalue), errbuf, 0, &affected_rows)) {
				safe_delete_array(query);
				if (affected_rows == 1) {
					LoadVariables(); // refresh cache
					free(varname);
					free(varvalue);
					return true;
				}
			}
		}
	}
	else {
		cerr << "Error in SetVariable query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
	}
	free(varname);
	free(varvalue);
	return false;
}

int32 Database::GetMiniLoginAccount(char* ip){
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
	MYSQL_ROW row;
	int32 retid = 0;
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT id FROM account WHERE minilogin_ip='%s'", ip), errbuf, &result)) {
		safe_delete_array(query);
		if ((row = mysql_fetch_row(result)))
			retid = atoi(row[0]);
		mysql_free_result(result);
	}
	else
	{
		cerr << "Error in GetMiniLoginAccount query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
	}
	return retid;
}

// Pyro: Get zone starting points from DB
bool Database::GetSafePoints(const char* short_name, float* safe_x, float* safe_y, float* safe_z, sint16* minstatus, int8* minlevel, char *flag_needed) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
	//	int buf_len = 256;
	//    int chars = -1;
    MYSQL_RES *result;
    MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query,
		"SELECT safe_x, safe_y, safe_z, min_status, min_level, "
		" flag_needed FROM zone "
		" WHERE short_name='%s'", short_name), errbuf, &result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			if (safe_x != 0)
				*safe_x = atof(row[0]);
			if (safe_y != 0)
				*safe_y = atof(row[1]);
			if (safe_z != 0)
				*safe_z = atof(row[2]);
			if (minstatus != 0)
				*minstatus = atoi(row[3]);
			if (minlevel != 0)
				*minlevel = atoi(row[4]);
			if (flag_needed != NULL)
				strcpy(flag_needed, row[5]);
			mysql_free_result(result);
			return true;
		}

		mysql_free_result(result);
	}
	else
	{
		cerr << "Error in GetSafePoint query '" << query << "' " << errbuf << endl;
		cerr << "If it errors, run the following querys:\n";
		cerr << "ALTER TABLE `zone` CHANGE `minium_level` `min_level` TINYINT(3)  UNSIGNED DEFAULT \"0\" NOT NULL;\n";
		cerr << "ALTER TABLE `zone` CHANGE `minium_status` `min_status` TINYINT(3)  UNSIGNED DEFAULT \"0\" NOT NULL;\n";
		cerr << "ALTER TABLE `zone` ADD flag_needed VARCHAR(128) NOT NULL DEFAULT '';\n";

		safe_delete_array(query);
	}
	return false;
}


bool Database::GetZoneLongName(const char* short_name, char** long_name, char* file_name, float* safe_x, float* safe_y, float* safe_z, int32* graveyard_id, int32* maxclients) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT long_name, file_name, safe_x, safe_y, safe_z, graveyard_id, maxclients FROM zone WHERE short_name='%s'", short_name), errbuf, &result))
	{
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			if (long_name != 0) {
				*long_name = strcpy(new char[strlen(row[0])+1], row[0]);
			}
			if (file_name != 0) {
				if (row[1] == 0)
					strcpy(file_name, short_name);
				else
					strcpy(file_name, row[1]);
			}
			if (safe_x != 0)
				*safe_x = atof(row[2]);
			if (safe_y != 0)
				*safe_y = atof(row[3]);
			if (safe_z != 0)
				*safe_z = atof(row[4]);
			if (graveyard_id != 0)
				*graveyard_id = atoi(row[5]);
			if (maxclients)
				*maxclients = atoi(row[6]);
			mysql_free_result(result);
			return true;
		}
		mysql_free_result(result);
	}
	else
	{
		cerr << "Error in GetZoneLongName query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
		return false;
	}

	return false;
}
int32 Database::GetZoneGraveyardID(int32 zone_id) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;
    int32 GraveyardID = 0;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT graveyard_id FROM zone WHERE zoneidnumber='%u'", zone_id), errbuf, &result))
	{
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			GraveyardID = atoi(row[0]);
		}
		mysql_free_result(result);
	}
	else
	{
		cerr << "Error in GetZoneGraveyardID query '" << query << "' " << errbuf << endl;
	}
	safe_delete_array(query);
	return GraveyardID;
}
bool Database::GetZoneGraveyard(const int32 graveyard_id, int32* graveyard_zoneid, float* graveyard_x, float* graveyard_y, float* graveyard_z, float* graveyard_heading) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT zone_id, x, y, z, heading FROM graveyard WHERE id=%i", graveyard_id), errbuf, &result))
	{
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			if(graveyard_zoneid != 0)
				*graveyard_zoneid = atoi(row[0]);
			if(graveyard_x != 0)
				*graveyard_x = atof(row[1]);
			if(graveyard_y != 0)
				*graveyard_y = atof(row[2]);
			if(graveyard_z != 0)
				*graveyard_z = atof(row[3]);
			if(graveyard_heading != 0)
				*graveyard_heading = atof(row[4]);
			mysql_free_result(result);
			return true;
		}
		mysql_free_result(result);
	}
	else
	{
		cerr << "Error in GetZoneGraveyard query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
		return false;
	}

	return false;
}

bool Database::LoadZoneNames() {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;
	query = new char[256];
	strcpy(query, "SELECT MAX(zoneidnumber) FROM zone");

	if (RunQuery(query, strlen(query), errbuf, &result)) {
		safe_delete_array(query);
		row = mysql_fetch_row(result);
		if (row && row[0])
		{
			max_zonename = atoi(row[0]);
			zonename_array = new char*[max_zonename+1];
			for(unsigned int i=0; i<max_zonename; i++) {
				zonename_array[i] = 0;
			}
			mysql_free_result(result);

			MakeAnyLenString(&query, "SELECT zoneidnumber, short_name FROM zone");
			if (RunQuery(query, strlen(query), errbuf, &result)) {
				safe_delete_array(query);
				while((row = mysql_fetch_row(result))) {
					zonename_array[atoi(row[0])] = new char[strlen(row[1]) + 1];
					strcpy(zonename_array[atoi(row[0])], row[1]);
					Sleep(0);
				}
				mysql_free_result(result);
			}
			else {
				cerr << "Error in LoadZoneNames query '" << query << "' " << errbuf << endl;
				safe_delete_array(query);
				return false;
			}
		}
		else {
			mysql_free_result(result);
		}
	}
	else {
		cerr << "Error in LoadZoneNames query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
		return false;
	}
	return true;
}

int32 Database::GetZoneID(const char* zonename) {
	if (zonename_array == 0)
		return 0;
	if (zonename == 0)
		return 0;
	for (unsigned int i=0; i<=max_zonename; i++) {
		if (zonename_array[i] != 0 && strcasecmp(zonename_array[i], zonename) == 0) {
			return i;
		}
	}
	return 0;
}

const char* Database::GetZoneName(int32 zoneID, bool ErrorUnknown) {
	if (zonename_array == 0) {
		if (ErrorUnknown)
			return "UNKNOWN";
		else
			return 0;
	}
	
	if (zoneID <= max_zonename) {
  		if (zonename_array[zoneID])
  			return zonename_array[zoneID];
  		else {
  			if (ErrorUnknown)
  				return "UNKNOWN";
  			else
  				return 0;
  		}
  	}
	else {
		if (ErrorUnknown)
			return "UNKNOWN";
		else
			return 0;
	}
	return 0;
}

int8 Database::GetPEQZone(int32 zoneID){
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
	MYSQL_ROW row;
	int peqzone=0;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT peqzone from zone where zoneidnumber='%i'", zoneID), errbuf, &result)) 
	{
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			peqzone = atoi(row[0]);
		}
			mysql_free_result(result);
		}
		else
		{
			cerr << "Error in GetPEQZone query '" << query << "' " << errbuf << endl;
	}
	safe_delete_array(query);
	return peqzone;
}

bool Database::CheckNameFilter(const char* name) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int i;

	// the minimum 4 is enforced by the client too
	if(!name || strlen(name) < 4 || strlen(name) > 64)
		return false;

	for (i = 0; name[i]; i++)
	{
		if(!isalpha(name[i]))
			return false;
	}

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT count(*) FROM name_filter WHERE '%s' like name", name), errbuf, &result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			if (row[0] != 0) {
				if (atoi(row[0]) == 0) {

					mysql_free_result(result);
					return false;
				}
			}
		}
		mysql_free_result(result);
		return true;
	}
	else
	{
		cerr << "Error in CheckNameFilter query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
	}

	return false;
}

bool Database::AddToNameFilter(const char* name) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
	int32 affected_rows = 0;

	if (!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO name_filter (name) values ('%s')", name), errbuf, 0, &affected_rows)) {
		cerr << "Error in AddToNameFilter query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
		return false;
	}

	safe_delete_array(query);

	if (affected_rows == 0) {
		return false;
	}

	return true;
}

int32 Database::GetAccountIDFromLSID(int32 iLSID, char* oAccountName, sint16* oStatus) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT id, name, status FROM account WHERE lsaccount_id=%i", iLSID), errbuf, &result))
	{
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			int32 account_id = atoi(row[0]);
			if (oAccountName)
				strcpy(oAccountName, row[1]);
			if (oStatus)
				*oStatus = atoi(row[2]);
			mysql_free_result(result);
			return account_id;
		}
		else
		{
			mysql_free_result(result);
			return 0;
		}
		mysql_free_result(result);
	}
	else {
		cerr << "Error in GetAccountIDFromLSID query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
		return 0;
	}

	return 0;
}

void Database::GetAccountFromID(int32 id, char* oAccountName, sint16* oStatus) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT name, status FROM account WHERE id=%i", id), errbuf, &result))
	{
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			if (oAccountName)
				strcpy(oAccountName, row[0]);
			if (oStatus)
				*oStatus = atoi(row[1]);
		}
		mysql_free_result(result);
	}
	else
		cerr << "Error in GetAccountFromID query '" << query << "' " << errbuf << endl;
	safe_delete_array(query);
}

void Database::ClearMerchantTemp(){
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;

	if (!RunQuery(query, MakeAnyLenString(&query, "delete from merchantlist_temp"), errbuf)) {
		cerr << "Error in ClearMerchantTemp query '" << query << "' " << errbuf << endl;
	}
	safe_delete_array(query);
}

bool Database::UpdateName(const char* oldname, const char* newname) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
	int32	affected_rows = 0;

	cout << "Renaming " << oldname << " to " << newname << "..." << endl;
	if (!RunQuery(query, MakeAnyLenString(&query, "UPDATE character_ SET name='%s' WHERE name='%s';", newname, oldname), errbuf, 0, &affected_rows)) {
		safe_delete_array(query);
		return false;
	}
	safe_delete_array(query);

	if (affected_rows == 0)
	{
		return false;
	}

	return true;
}

// If the name is used or an error occurs, it returns false, otherwise it returns true
bool Database::CheckUsedName(const char* name)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
    MYSQL_RES *result;
	//if (strlen(name) > 15)
	//	return false;
	if (!RunQuery(query, MakeAnyLenString(&query, "SELECT id FROM character_ where name='%s'", name), errbuf, &result)) {
		cerr << "Error in CheckUsedName query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
		return false;
	}
	else { // It was a valid Query, so lets do our counts!
		safe_delete_array(query);
		int32 tmp = mysql_num_rows(result);
		mysql_free_result(result);
		if (tmp > 0) // There is a Name!  No change (Return False)
			return false;
		else // Everything is okay, so we go and do this.
			return true;
	}
}

int8 Database::GetServerType()
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT value FROM variables WHERE varname='ServerType'"), errbuf, &result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1)
		{
			row = mysql_fetch_row(result);
			int8 ServerType = atoi(row[0]);
			mysql_free_result(result);
			return ServerType;
		}
		else
		{
			mysql_free_result(result);
			return 0;
		}
		mysql_free_result(result);
	}
	else

	{


		cerr << "Error in GetServerType query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
		return false;
	}

	return 0;

}

bool Database::MoveCharacterToZone(const char* charname, const char* zonename,int32 zoneid) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	int32	affected_rows = 0;

	if(zonename == NULL || strlen(zonename) == 0)
		return(false);

	if (!RunQuery(query, MakeAnyLenString(&query, "UPDATE character_ SET zonename = '%s',zoneid=%i,x=-1, y=-1, z=-1 WHERE name='%s'", zonename,zoneid, charname), errbuf, 0,&affected_rows)) {
		cerr << "Error in MoveCharacterToZone(name) query '" << query << "' " << errbuf << endl;
		return false;
	}
	safe_delete_array(query);

	if (affected_rows == 0)
		return false;

	return true;
}

bool Database::MoveCharacterToZone(const char* charname, const char* zonename) {
	return MoveCharacterToZone(charname, zonename, GetZoneID(zonename));
}

bool Database::MoveCharacterToZone(int32 iCharID, const char* iZonename) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	int32	affected_rows = 0;
	if (!RunQuery(query, MakeAnyLenString(&query, "UPDATE character_ SET zonename = '%s', zoneid=%i, x=-1, y=-1, z=-1 WHERE id=%i", iZonename, GetZoneID(iZonename), iCharID), errbuf, 0,&affected_rows)) {
		cerr << "Error in MoveCharacterToZone(id) query '" << query << "' " << errbuf << endl;
		return false;
	}
	safe_delete_array(query);

	if (affected_rows == 0)
		return false;

	return true;
}

int8 Database::CopyCharacter(const char* oldname, const char* newname, int32 acctid) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	PlayerProfile_Struct* pp;
	ExtendedProfile_Struct* ext;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT profile, guild, guildrank, extprofile FROM character_ WHERE name='%s'", oldname), errbuf, &result)) {
		safe_delete_array(query);

		row = mysql_fetch_row(result);

		pp = (PlayerProfile_Struct*)row[0];
		strcpy(pp->name, newname);

		ext = (ExtendedProfile_Struct*)row[3];

		mysql_free_result(result);
	}

	else {
		cerr << "Error in CopyCharacter read query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
		return 0;
	}

	int32 affected_rows = 0;
	char query2[276 + sizeof(PlayerProfile_Struct)*2 + sizeof(ExtendedProfile_Struct)*2 + 1];
	char* end=query2;

	end += sprintf(end, "INSERT INTO character_ SET zonename=\'%s\', x = %f, y = %f, z = %f, profile=\'", GetZoneName(pp->zone_id), pp->x, pp->y, pp->z);
    end += DoEscapeString(end, (char*) pp, sizeof(PlayerProfile_Struct));
	end += sprintf(end,"\', extprofile=\'");
	end += DoEscapeString(end, (char*) ext, sizeof(ExtendedProfile_Struct));
    end += sprintf(end, "\', account_id=%d, name='%s'", acctid, newname);

	if (!RunQuery(query2, (int32) (end - query2), errbuf, 0, &affected_rows)) {
        cerr << "Error in CopyCharacter query '" << query << "' " << errbuf << endl;
		return 0;
    }

	// @merth: Need to copy inventory as well (and shared bank?)
	if (affected_rows == 0) {
		return 0;
	}

	return 1;
}

bool Database::SetHackerFlag(const char* accountname, const char* charactername, const char* hacked) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	int32	affected_rows = 0;
	if (!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO hackers(account,name,hacked) values('%s','%s','%s')", accountname, charactername, hacked), errbuf, 0,&affected_rows)) {
		cerr << "Error in SetHackerFlag query '" << query << "' " << errbuf << endl;
		return false;
	}
	safe_delete_array(query);

	if (affected_rows == 0)
	{
		return false;
	}

	return true;
}

bool Database::SetMQDetectionFlag(const char* accountname, const char* charactername, const char* hacked, const char* zone) { //Lieka:  Utilize the "hacker" table, but also give zone information.

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	int32	affected_rows = 0;

	if (!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO hackers(account,name,hacked,zone) values('%s','%s','%s','%s')", accountname, charactername, hacked, zone), errbuf, 0,&affected_rows)) {
		cerr << "Error in SetMQDetectionFlag query '" << query << "' " << errbuf << endl;
		return false;
	}

	safe_delete_array(query);

	if (affected_rows == 0)
	{
		return false;
	}

	return true;
}

int8 Database::GetRaceSkill(int8 skillid, int8 in_race)
{
	int16 race_cap = 0;
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	int32	affected_rows = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	//Check for a racial cap!
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT skillcap from race_skillcaps where skill = %i && race = %i", skillid, in_race), errbuf, &result, &affected_rows))
	{
		if (affected_rows != 0)
		{
			row = mysql_fetch_row(result);
			race_cap = atoi(row[0]);
		}
		delete[] query;
		mysql_free_result(result);
	}

	return race_cap;
}

int8 Database::GetSkillCap(int8 skillid, int8 in_race, int8 in_class, int16 in_level)
{
	int8 skill_level = 0, skill_formula = 0;
	int16 base_cap = 0, skill_cap = 0, skill_cap2 = 0, skill_cap3 = 0;
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	int32	affected_rows = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	//Fetch the data from DB.
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT level, formula, pre50cap, post50cap, post60cap from skillcaps where skill = %i && class = %i", skillid, in_class), errbuf, &result, &affected_rows))
	{
		if (affected_rows != 0)
		{
			row = mysql_fetch_row(result);
			skill_level = atoi(row[0]);
			skill_formula = atoi(row[1]);
			skill_cap = atoi(row[2]);
			if (atoi(row[3]) > skill_cap)
				skill_cap2 = (atoi(row[3])-skill_cap)/10; //Split the post-50 skill cap into difference between pre-50 cap and post-50 cap / 10 to determine amount of points per level.
			skill_cap3 = atoi(row[4]);
		}
		delete[] query;
		mysql_free_result(result);
	}

	int race_skill = GetRaceSkill(skillid,in_race);

	if (race_skill > 0 && (race_skill > skill_cap || skill_cap == 0 || in_level < skill_level))
		return race_skill;

	if (skill_cap == 0) //Can't train this skill at all.
		return 255; //Untrainable

	if (in_level < skill_level)
		return 254; //Untrained

	//Determine pre-51 level-based cap
	if (skill_formula > 0)
		base_cap = in_level*skill_formula+skill_formula;
	if (base_cap > skill_cap || skill_formula == 0)
		base_cap = skill_cap;
	//If post 50, add post 50 cap to base cap.
	if (in_level > 50 && skill_cap2 > 0)
		base_cap += skill_cap2*(in_level-50);
	//No cap should ever go above its post50cap
	if (skill_cap3 > 0 && base_cap > skill_cap3)
		base_cap = skill_cap3;
	//Base cap is now the max value at the person's level, return it!
	return base_cap;
}

int32 Database::GetCharacterInfo(const char* iName, int32* oAccID, int32* oZoneID, int32* oInstanceID, float* oX, float* oY, float* oZ) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT id, account_id, zonename, instanceid, x, y, z FROM character_ WHERE name='%s'", iName), errbuf, &result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			int32 charid = atoi(row[0]);
			if (oAccID)
				*oAccID = atoi(row[1]);
			if (oZoneID)
				*oZoneID = GetZoneID(row[2]);
			if(oInstanceID)
				*oInstanceID = atoi(row[3]);
			if (oX)
				*oX = atof(row[4]);
			if (oY)
				*oY = atof(row[5]);
			if (oZ)
				*oZ = atof(row[6]);
			mysql_free_result(result);
			return charid;
		}
		mysql_free_result(result);
	}
	else {
		cerr << "Error in GetCharacterInfo query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
	}
	return 0;
}

bool Database::UpdateLiveChar(char* charname,int32 lsaccount_id) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
	if (!RunQuery(query, MakeAnyLenString(&query, "UPDATE account SET charname='%s' WHERE id=%i;",charname, lsaccount_id), errbuf)) {
		cerr << "Error in UpdateLiveChar query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
		return false;
	}

	safe_delete_array(query);
	return true;
}

bool Database::GetLiveChar(int32 account_id, char* cname) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT charname FROM account WHERE id=%i", account_id), errbuf, &result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			strcpy(cname,row[0]);
			mysql_free_result(result);
			return true;
		}
		mysql_free_result(result);
	}
	else {
		cerr << "Error in GetLiveChar query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
	}

	return false;
}

void Database::SetLFP(int32 CharID, bool LFP) {

	char ErrBuf[MYSQL_ERRMSG_SIZE];
	char *Query = 0;

	if (!RunQuery(Query, MakeAnyLenString(&Query, "update character_ set lfp=%i where id=%i",LFP, CharID), ErrBuf))
		LogFile->write(EQEMuLog::Error, "Error updating LFP for character %i : %s", CharID, ErrBuf);

	safe_delete_array(Query);

}

void Database::SetLFG(int32 CharID, bool LFG) {

	char ErrBuf[MYSQL_ERRMSG_SIZE];
	char *Query = 0;

	if (!RunQuery(Query, MakeAnyLenString(&Query, "update character_ set lfg=%i where id=%i",LFG, CharID), ErrBuf))
		LogFile->write(EQEMuLog::Error, "Error updating LFP for character %i : %s", CharID, ErrBuf);

	safe_delete_array(Query);

}

void  Database::SetGroupID(const char* name,int32 id, int32 charid){
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
	if(id == 0){ //removing you from table
	if (!RunQuery(query, MakeAnyLenString(&query, "delete from group_id where charid=%i",charid), errbuf))
		printf("Unable to get group id: %s\n",errbuf);
	}
	else{
	if (!RunQuery(query, MakeAnyLenString(&query, "replace into group_id set charid=%i, groupid=%i, name='%s'",charid, id, name), errbuf))
		printf("Unable to get group id: %s\n",errbuf);
	}
#ifdef _EQDEBUG
	printf("Set group id on '%s' to %d\n", name, id);
#endif
	safe_delete_array(query);
}

void Database::ClearGroup(int32 gid) {
	ClearGroupLeader(gid);
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
	if(gid == 0) {  //clear all groups
		//if (!RunQuery(query, MakeAnyLenString(&query, "update group_id set groupid=0 where groupid!=0"), errbuf))
		if (!RunQuery(query, MakeAnyLenString(&query, "delete from group_id"), errbuf))
			printf("Unable to clear groups: %s\n",errbuf);
	} else {	//clear a specific group
		//if (!RunQuery(query, MakeAnyLenString(&query, "update group_id set groupid=0 where groupid = %lu", gid), errbuf))
		if (!RunQuery(query, MakeAnyLenString(&query, "delete from group_id where groupid = %lu", gid), errbuf))
			printf("Unable to clear groups: %s\n",errbuf);
	}
	safe_delete_array(query);
}

int32 Database::GetGroupID(const char* name){
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
	MYSQL_ROW row;
	int32 groupid=0;
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT groupid from group_id where name='%s'", name), errbuf, &result)) {
		if((row = mysql_fetch_row(result)))
		{
			if(row[0])
				groupid=atoi(row[0]);
		}
		else
			printf("Unable to get group id, char not found!\n");
		mysql_free_result(result);
	}
	else
			printf("Unable to get group id: %s\n",errbuf);
	safe_delete_array(query);
	return groupid;
}

char* Database::GetGroupLeaderForLogin(const char* name,char* leaderbuf){
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
	MYSQL_ROW row;
	PlayerProfile_Struct pp;
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT profile from character_ where name='%s'", name), errbuf, &result)) {
		row = mysql_fetch_row(result);
		unsigned long* lengths = mysql_fetch_lengths(result);
		if (lengths[0] == sizeof(PlayerProfile_Struct)) {
			memcpy(&pp, row[0], sizeof(PlayerProfile_Struct));
			strcpy(leaderbuf,pp.groupMembers[0]);
		}
		mysql_free_result(result);
	}
	else{
			printf("Unable to get leader name: %s\n",errbuf);
	}
	safe_delete_array(query);
	return leaderbuf;
}

void Database::SetGroupLeaderName(int32 gid, const char* name){
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
	if (!RunQuery(query, MakeAnyLenString(&query, "Replace into group_leaders set gid=%i, leadername='%s'",gid,name), errbuf))
		printf("Unable to set group leader: %s\n",errbuf);

	safe_delete_array(query);
}

char *Database::GetGroupLeadershipInfo(int32 gid, char* leaderbuf, char* assist, char *marknpc, GroupLeadershipAA_Struct* GLAA){
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES* result;
	MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT leadername, assist, marknpc, leadershipaa FROM group_leaders WHERE gid=%i",gid),
		     errbuf, &result)) {

		safe_delete_array(query);

		row = mysql_fetch_row(result);
		if(row != NULL){

			if(leaderbuf)
				strcpy(leaderbuf, row[0]);

			if(assist)
				strcpy(assist, row[1]);

			if(marknpc)
				strcpy(marknpc, row[2]);

			if(GLAA)
				memcpy(GLAA, row[3], sizeof(GroupLeadershipAA_Struct));

			mysql_free_result(result);
			return leaderbuf;
		}
	}
	else
		safe_delete_array(query);

	if(leaderbuf)
		strcpy(leaderbuf, "UNKNOWN");

	if(assist)
		assist[0] = 0;

	if(marknpc)
		marknpc[0] = 0;

	return leaderbuf;
}

void Database::ClearGroupLeader(int32 gid){
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
	if(gid == 0) {  //clear all group leaders
		if (!RunQuery(query, MakeAnyLenString(&query, "DELETE from group_leaders"), errbuf))
			printf("Unable to clear group leaders: %s\n",errbuf);
	} else {	//clear a specific group leader
		if (!RunQuery(query, MakeAnyLenString(&query, "DELETE from group_leaders where gid = %lu", gid), errbuf))
			printf("Unable to clear group leader: %s\n",errbuf);
	}
	safe_delete_array(query);
}

bool FetchRowMap(MYSQL_RES *result, map<string,string> &rowmap)
{
MYSQL_FIELD *fields;
MYSQL_ROW row;
unsigned long num_fields,i;
bool  retval=false;
	rowmap.clear();
	if (result && (num_fields=mysql_num_fields(result)) && (row = mysql_fetch_row(result))!=NULL && (fields = mysql_fetch_fields(result))!=NULL) {
		retval=true;
		for(i=0;i<num_fields;i++) {
			rowmap[fields[i].name]=(row[i] ? row[i] : "");
		}
	}

	return retval;
}

int8 Database::GetAgreementFlag(int32 acctid)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES* result;
	MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT rulesflag FROM account WHERE id=%i",acctid), errbuf, &result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1)
		{
			row = mysql_fetch_row(result);
			int8 flag = atoi(row[0]);
			mysql_free_result(result);
			return flag;
		}
	}
	else
	{
		safe_delete_array(query);
	}
	return 0;
}

void Database::SetAgreementFlag(int32 acctid)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	int32	affected_rows = 0;

	if (!RunQuery(query, MakeAnyLenString(&query, "UPDATE account SET rulesflag=1 where id=%i",acctid), errbuf, 0, &affected_rows)) {
		safe_delete_array(query);
	}
	else
	safe_delete_array(query);
}

#ifdef EQBOTS

// Franck-adds: EQoffline

// Change the isbot field in the npc_types entries for a given mob: it will become 'bottable'.
// Then, you will make it your bot.
void Database::AddBot(int32 mobidtmp) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	int32 affected_rows = 0;

	if(!RunQuery(query, MakeAnyLenString(&query, "UPDATE npc_types SET isbot=1 where id=%i", mobidtmp), errbuf, 0, &affected_rows)) {
		cerr << "Error in AddBot query '" << query << "' " << errbuf << endl;
	}
	safe_delete_array(query);
}

MYSQL_RES* Database::ListSpawnedBots(int32 id) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES* result;
	MYSQL_ROW row;

	if(!RunQuery(query, MakeAnyLenString(&query, "SELECT bot_name, zone_name FROM botleader WHERE leaderid=%i", id), errbuf, &result)) {
		cerr << "Error in ListSpawnedBots query '" << query << "' " << errbuf << endl;
	}
    safe_delete_array(query);
	return result;
}

// See if a mob is bottable or no by checking the isbot field in the npc_types entrie.
int Database::GetBotStatus(int32 mobidtmp) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES* result;
	MYSQL_ROW row;
    int botstatus = 0;

	if(RunQuery(query, MakeAnyLenString(&query, "SELECT isbot FROM npc_types WHERE id=%i", mobidtmp), errbuf, &result)) {
		if(mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			botstatus = atoi(row[0]);
		}
	}
	else {
		cerr << "Error in GetBotStatus query '" << query << "' " << errbuf << endl;
	}
	mysql_free_result(result);
    safe_delete_array(query);
    return botstatus;
}

bool Database::DeleteBot(int32 mobid) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
	bool success = true;
	
	if(!RunQuery(query, MakeAnyLenString(&query, "DELETE from npc_types where id=%i", mobid), errbuf)) {
		success = false;
	}
	if(!RunQuery(query, MakeAnyLenString(&query, "DELETE from botinventory where npctypeid=%i", mobid), errbuf)) {
		success = false;
	}
	if(!RunQuery(query, MakeAnyLenString(&query, "DELETE from botsowners where botnpctypeid=%i", mobid), errbuf)) {
		success = false;
	}
	safe_delete_array(query);
	return success;
}

void Database::SaveBotGroups(int32 groupid, int32 charid, int32 botid, int16 slot) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	int32 affected_rows = 0;

	if(!RunQuery(query, MakeAnyLenString(&query, "INSERT into botgroups (groupid, charid, botid, slot) values (%i, %i, %i, %i)", groupid, charid, botid, slot), errbuf, 0, &affected_rows)) {
		cerr << "Error in SaveBotGroups query '" << query << "' " << errbuf << endl;
	}
	safe_delete_array(query);
}

void Database::DeleteBotGroups(int32 charid) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	int32 affected_rows = 0;

	if(!RunQuery(query, MakeAnyLenString(&query, "DELETE FROM botgroups where charid=%i", charid), errbuf, 0, &affected_rows)) {
		cerr << "Error in DeleteBotGroups query '" << query << "' " << errbuf << endl;
	}
	safe_delete_array(query);
}

MYSQL_RES* Database::LoadBotGroups(int32 charid) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES* result;
	MYSQL_ROW row;

	if(!RunQuery(query, MakeAnyLenString(&query, "SELECT groupid, botid from botgroups WHERE charid=%i order by charid, groupid, slot", charid), errbuf, &result)) {
		cerr << "Error in ListSpawnedBots query '" << query << "' " << errbuf << endl;
	}
    safe_delete_array(query);
	return result;
}

// Set the bot leader once it got invited in the group
void Database::SetBotLeader(int32 mobidtmp, int32 leaderid, const char* botName, const char* zoneName) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	int32 affected_rows = 0;

	if(!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO botleader SET botid=%i, leaderid=%i, bot_name='%s', zone_name='%s'", mobidtmp, leaderid, botName, zoneName), errbuf, 0, &affected_rows)) {
		cerr << "Error in SetBotLeader query '" << query << "' " << errbuf << endl;
	}
	safe_delete_array(query);
}

// Who's the bot leader ?
int32 Database::GetBotLeader(int32 mobidtmp) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES* result;
	MYSQL_ROW row;
    int32 botleader = 0;

	if(RunQuery(query, MakeAnyLenString(&query, "SELECT leaderid FROM botleader WHERE botid=%i", mobidtmp), errbuf, &result)) {
		if(mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			botleader = atoi(row[0]);
		}
	}
	else {
		cerr << "Error in GetBotLeader query '" << query << "' " << errbuf << endl;
	}
	mysql_free_result(result);
    safe_delete_array(query);
    return botleader;
}

bool Database::IsBotSpawned(int32 id, int botid) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES* result;
	MYSQL_ROW row;
    bool isSpawned = false;

	if(RunQuery(query, MakeAnyLenString(&query, "SELECT botid FROM botleader WHERE leaderid=%i", id), errbuf, &result)) {
		if(mysql_num_rows(result) > 0) {
			while(row = mysql_fetch_row(result)) {
				if(botid == atoi(row[0])) {
					isSpawned = true;
				}
			}
		}
	}
	else {
		cerr << "Error in AllowedBotSpawns query '" << query << "' " << errbuf << endl;
	}
	mysql_free_result(result);
    safe_delete_array(query);
    return isSpawned;
}

int Database::AllowedBotSpawns(int32 id) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES* result;
	MYSQL_ROW row;
    int numberAllowed = 0;

	if(RunQuery(query, MakeAnyLenString(&query, "SELECT value FROM quest_globals WHERE name='bot_spawn_limit' and charid=%i", id), errbuf, &result)) {
		if(mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			numberAllowed = atoi(row[0]);
		}
	}
	else {
		cerr << "Error in AllowedBotSpawns query '" << query << "' " << errbuf << endl;
	}
	mysql_free_result(result);
    safe_delete_array(query);
    return numberAllowed;
}

int Database::SpawnedBotCount(int32 id) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES* result;
	MYSQL_ROW row;
    int spawnedBots = 0;

	if(RunQuery(query, MakeAnyLenString(&query, "SELECT COUNT(*) FROM botleader WHERE leaderid=%i", id), errbuf, &result)) {
		if(mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			spawnedBots = atoi(row[0]);
		}
	}
	else {
		cerr << "Error in SpawnedBotCount query '" << query << "' " << errbuf << endl;
	}
	mysql_free_result(result);
    safe_delete_array(query);
    return spawnedBots;
}

int Database::GetBotOwner(int32 mobid) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES* result;
	MYSQL_ROW row;
    int botowner = 0;

	if(RunQuery(query, MakeAnyLenString(&query, "SELECT botleadercharacterid FROM botsowners WHERE botnpctypeid=%i", mobid), errbuf, &result)) {
		if(mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			botowner = atoi(row[0]);
		}
	}
	else {
		cerr << "Error in GetBotOwner query '" << query << "' " << errbuf << endl;
	}
	mysql_free_result(result);
    safe_delete_array(query);
    return botowner;
}

void Database::SetBotOwner(int32 mobid, int32 ownerid) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	int32 affected_rows = 0;

	if(!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO botsowners(botleadercharacterid, botnpctypeid) values(%i, %i)", ownerid, mobid), errbuf, 0, &affected_rows)) {
		cerr << "Error in SetBotOwner query '" << query << "' " << errbuf << endl;
	}
	safe_delete_array(query);
}

void Database::UpdateBotOwner(int32 accountid, int32 ownerid) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	int32 affected_rows = 0;

	if(!RunQuery(query, MakeAnyLenString(&query, "UPDATE botsowners set botleadercharacterid=%i where botleadercharacterid=%i", accountid, ownerid), errbuf, 0, &affected_rows)) {
		cerr << "Error in SetBotOwner query '" << query << "' " << errbuf << endl;
	}
	safe_delete_array(query);
}

// Clean all the bots leader entries when the leader disconnects, zones or LD
void Database::CleanBotLeader(int32 leaderid) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	int32 affected_rows = 0;

	if(!RunQuery(query, MakeAnyLenString(&query, "DELETE FROM botleader where leaderid=%i", leaderid), errbuf, 0, &affected_rows)) {
		cerr << "Error in CleanBotLeader query '" << query << "' " << errbuf << endl;
	}
	safe_delete_array(query);
}

// Clean the leader entrie for a given mob
void Database::CleanBotLeaderEntries(int32 mobidtmp) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	int32 affected_rows = 0;

    if(!RunQuery(query, MakeAnyLenString(&query, "DELETE FROM botleader WHERE botid=%i", mobidtmp), errbuf, 0, &affected_rows)) {
		cerr << "Error in CleanBotLeaderEntries query '" << query << "' " << errbuf << endl;
	}
	safe_delete_array(query);
}

// How many bots have they created?
int Database::CountBots(int32 accountID) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES* result;
	MYSQL_ROW row;
    int botCount = 0;

	if(RunQuery(query, MakeAnyLenString(&query, "SELECT COUNT(*) FROM botsowners WHERE botleadercharacterid=%i", accountID), errbuf, &result)) {
		if(mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			botCount = atoi(row[0]);
		}
	}
	else {
		cerr << "Error in CountBots query '" << query << "' " << errbuf << endl;
	}
	mysql_free_result(result);
    safe_delete_array(query);
    return botCount;
}

// Find an item in the bot inventory at a given slot
int Database::GetBotItemBySlot(int32 botid, int32 slot) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES* result;
	MYSQL_ROW row;
    int iteminslot = 0;

	if(RunQuery(query, MakeAnyLenString(&query, "SELECT itemid FROM botinventory WHERE npctypeid=%i AND botslotid=%i", botid, slot), errbuf, &result)) {
		if(mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			iteminslot = atoi(row[0]);
		}
	}
	else {
		cerr << "Error in GetBotItemBySlot query '" << query << "' " << errbuf << endl;
	}
	mysql_free_result(result);
    safe_delete_array(query);
    return iteminslot;
}

MYSQL_RES* Database::GetBotItems(int32 botid) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES* result;
	MYSQL_ROW row;
    int iteminslot = 0;

	if(RunQuery(query, MakeAnyLenString(&query, "SELECT botslotid, itemid FROM botinventory WHERE npctypeid=%i order by botslotid", botid), errbuf, &result)) {
		if(mysql_num_rows(result) > 0) {
			safe_delete_array(query);
			return result;
		}
	}
	else {
		cerr << "Error in GetBotItemBySlot query '" << query << "' " << errbuf << endl;
	}
	mysql_free_result(result);
    safe_delete_array(query);
    return 0;
}

// Remove an item in the given slot
void Database::RemoveBotItemBySlot(int32 botid, int32 slot) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	int32 affected_rows = 0;

	if(!RunQuery(query, MakeAnyLenString(&query, "DELETE FROM botinventory WHERE npctypeid=%i AND botslotid=%i", botid, slot), errbuf, 0, &affected_rows)){
		cerr << "Error in RemoveBotItemBySlot query '" << query << "' " << errbuf << endl;
	}
	safe_delete_array(query);
}

// Add or change an item at a given slot in the bot inventory
void Database::SetBotItemInSlot(int32 botid, int32 slot, int32 itemid) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	int32 affected_rows = 0;

	if(GetBotItemBySlot(botid, slot) == 0) {
		if(!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO botinventory(npctypeid, botslotid, itemid) VALUES(%i, %i, %i)", botid, slot, itemid), errbuf, 0, &affected_rows)) {
            cerr << "Error in SetBotItemInSlot query '" << query << "' " << errbuf << endl;
		}
	}
	else {
		if(!RunQuery(query, MakeAnyLenString(&query, "UPDATE botinventory SET itemid=%i where npctypeid=%i AND botslotid=%i", itemid, botid, slot), errbuf, 0, &affected_rows)) {
            cerr << "Error in SetBotItemInSlot query '" << query << "' " << errbuf << endl;
		}
	}
    safe_delete_array(query);
}

// How many items does the bot have in its inventory ?
int Database::GetBotItemsNumber(int32 botid) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES* result;
	MYSQL_ROW row;
    int itemsnbre = 0;

	if(RunQuery(query, MakeAnyLenString(&query, "SELECT COUNT(*) FROM botinventory WHERE npctypeid=%i", botid), errbuf, &result)) {
		if(mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			itemsnbre = atoi(row[0]);
		}
	}
	else {
        cerr << "Error in GetBotItemsNumber query '" << query << "' " << errbuf << endl;
	}
	mysql_free_result(result);
    safe_delete_array(query);
    return itemsnbre;
}

#endif //EQBOTS

void Database::ClearRaid(int32 rid) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
	if(rid == 0) {  //clear all raids
		if (!RunQuery(query, MakeAnyLenString(&query, "delete from raid_members"), errbuf))
			printf("Unable to clear raids: %s\n",errbuf);
	} else {	//clear a specific group
		if (!RunQuery(query, MakeAnyLenString(&query, "delete from raid_members where raidid = %lu", rid), errbuf))
			printf("Unable to clear raids: %s\n",errbuf);
	}
	safe_delete_array(query);
}

void Database::ClearRaidDetails(int32 rid) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
	if(rid == 0) {  //clear all raids
		if (!RunQuery(query, MakeAnyLenString(&query, "delete from raid_details"), errbuf))
			printf("Unable to clear raid details: %s\n",errbuf);
	} else {	//clear a specific group
		if (!RunQuery(query, MakeAnyLenString(&query, "delete from raid_details where raidid = %lu", rid), errbuf))
			printf("Unable to clear raid details: %s\n",errbuf);
	}
	safe_delete_array(query);
}

int32 Database::GetRaidID(const char* name){
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
	MYSQL_ROW row;
	int32 raidid=0;
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT raidid from raid_members where name='%s'", name), 
		errbuf, &result)) {
		if((row = mysql_fetch_row(result)))
		{
			if(row[0])
				raidid=atoi(row[0]);
		}
		else
			printf("Unable to get raid id, char not found!\n");
		mysql_free_result(result);
	}
	else
			printf("Unable to get raid id: %s\n",errbuf);
	safe_delete_array(query);
	return raidid;
}

const char *Database::GetRaidLeaderName(int32 rid)
{
	static char name[128];

	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
	MYSQL_ROW row;
	
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT name FROM raid_members WHERE raidid=%u AND israidleader=1", 
		rid), errbuf, &result)) {
		if((row = mysql_fetch_row(result)) != NULL)
		{
			memset(name, 0, 128);
			strcpy(name, row[0]);
			mysql_free_result(result);
			safe_delete_array(query);
			return name;
		}
		else
			printf("Unable to get raid id, char not found!\n");
		mysql_free_result(result);
	}
	else
		printf("Unable to get raid id: %s\n",errbuf);
	safe_delete_array(query);
	return "UNKNOWN";
}

bool Database::VerifyInstanceAlive(int16 instance_id, int32 char_id)
{

	//we are not saved to this instance so set our instance to 0
	if(!CharacterInInstanceGroup(instance_id, char_id))
	{
		SetCharacterInstance(0, char_id);
		return false;
	}

	if(CheckInstanceExpired(instance_id))
	{
		DeleteInstance(instance_id);
		SetCharacterInstance(0, char_id);
		return false;
	}
	return true;
}

bool Database::VerifyZoneInstance(int32 zone_id, int16 instance_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT id FROM instance_lockout where id=%u AND zone=%u", 
		instance_id, zone_id), errbuf, &result))
	{
		safe_delete_array(query);
		if (mysql_num_rows(result) != 0) 
		{
			mysql_free_result(result);
			return true;
		}
		else
		{
			mysql_free_result(result);
			return false;
		}
	}
	else 
	{
		safe_delete_array(query);
		return false;
	}
	return false;
}

bool Database::CharacterInInstanceGroup(int16 instance_id, int32 char_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	bool lockout_instance_player = false;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT charid FROM instance_lockout_player where id=%u AND charid=%u", 
		instance_id, char_id), errbuf, &result))
	{
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1) 
		{
			lockout_instance_player = true;
		}
		mysql_free_result(result);
	}
	else 
	{
		safe_delete_array(query);
	}
	return lockout_instance_player;
}

void Database::SetCharacterInstance(int16 instance_id, int32 char_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	if(RunQuery(query, MakeAnyLenString(&query, "UPDATE character_ SET instanceid=%u WHERE id=%u", instance_id, 
		char_id), errbuf, &result))
	{
		safe_delete_array(query);
		mysql_free_result(result);
	}
	else 
	{
		safe_delete_array(query);
	}
}

void Database::DeleteInstance(uint16 instance_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if(RunQuery(query, MakeAnyLenString(&query, "DELETE FROM instance_lockout WHERE id=%u", instance_id), errbuf))
	{
		safe_delete_array(query);
	}
	else 
	{
		safe_delete_array(query);
	}

	if(RunQuery(query, MakeAnyLenString(&query, "DELETE FROM instance_lockout_player WHERE id=%u", instance_id), errbuf))
	{
		safe_delete_array(query);
	}
	else 
	{
		safe_delete_array(query);
	}

	if(RunQuery(query, MakeAnyLenString(&query, "DELETE FROM respawn_times WHERE instance_id=%u", instance_id), errbuf))
	{
		safe_delete_array(query);
	}
	else 
	{
		safe_delete_array(query);
	}
	BuryCorpsesInInstance(instance_id);
}

bool Database::CheckInstanceExpired(uint16 instance_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	int32 start_time = 0;
	int32 duration = 0;
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT start_time, duration FROM instance_lockout WHERE id=%u", 
		instance_id), errbuf, &result))
	{
		safe_delete_array(query);
		if (mysql_num_rows(result) != 0) 
		{
			row = mysql_fetch_row(result);
			start_time = atoi(row[0]);
			duration = atoi(row[1]);
		}
		else
		{
			mysql_free_result(result);
			return true;
		}
		mysql_free_result(result);
	}
	else 
	{
		safe_delete_array(query);
		return true;
	}

	timeval tv;
	gettimeofday(&tv, NULL);
	if((start_time + duration) <= tv.tv_sec)
	{
		return true;
	}
	return false;
}

int32 Database::ZoneIDFromInstanceID(uint16 instance_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int32 ret;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT zone FROM instance_lockout where id=%u", instance_id), 
		errbuf, &result))
	{
		safe_delete_array(query);
		if (mysql_num_rows(result) != 0) 
		{
			row = mysql_fetch_row(result);
			ret = atoi(row[0]);
			mysql_free_result(result);
			return ret;			
		}
		else
		{
			mysql_free_result(result);
			return 0;
		}
	}
	else 
	{
		safe_delete_array(query);
		return 0;
	}
	return 0;
}

int32 Database::VersionFromInstanceID(uint16 instance_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int32 ret;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT version FROM instance_lockout where id=%u", instance_id), 
		errbuf, &result))
	{
		safe_delete_array(query);
		if (mysql_num_rows(result) != 0) 
		{
			row = mysql_fetch_row(result);
			ret = atoi(row[0]);
			mysql_free_result(result);
			return ret;			
		}
		else
		{
			mysql_free_result(result);
			return 0;
		}
	}
	else 
	{
		safe_delete_array(query);
		return 0;
	}
	return 0;
}

int32 Database::GetTimeRemainingInstance(uint16 instance_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int32 start_time = 0;
	int32 duration = 0;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT start_time, duration FROM instance_lockout WHERE id=%u", 
		instance_id), errbuf, &result))
	{
		safe_delete_array(query);
		if (mysql_num_rows(result) != 0) 
		{
			row = mysql_fetch_row(result);
			start_time = atoi(row[0]);
			duration = atoi(row[1]);
		}
		else
		{
			mysql_free_result(result);
			return 0;
		}
		mysql_free_result(result);
	}
	else 
	{
		safe_delete_array(query);
		return 0;
	}

	timeval tv;
	gettimeofday(&tv, NULL);
	return ((start_time + duration) - tv.tv_sec);
}

bool Database::GetUnusedInstanceID(uint16 &instance_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT COUNT(*) FROM instance_lockout"), errbuf, &result))
	{
		safe_delete_array(query);
		if (mysql_num_rows(result) != 0) 
		{
			row = mysql_fetch_row(result);
			int count = atoi(row[0]);
			if(count == 0)
			{
				mysql_free_result(result);
				instance_id = 1;
				return true;
			}
		}
		else
		{
			mysql_free_result(result);
		}
		mysql_free_result(result);
	}
	else 
	{
		safe_delete_array(query);
		instance_id = 0;
		return false;
	}

	int32 count = 1;
	int32 max = 65535;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT id FROM instance_lockout ORDER BY id"), errbuf, &result))
	{
		safe_delete_array(query);
		if (mysql_num_rows(result) != 0) 
		{
			while(row = mysql_fetch_row(result))
			{
				if(count < atoi(row[0]))
				{
					instance_id = count;
					mysql_free_result(result);
					return true;
				}
				else if(count > max)
				{
					instance_id = 0;
					mysql_free_result(result);
					return false;
				}
				else
				{
					count++;
				}
			}
		}
		else
		{
			mysql_free_result(result);
		}
		mysql_free_result(result);
	}
	else 
	{
		safe_delete_array(query);
	}
	instance_id = count;
	return true;
}

//perhaps purge any expireds too
bool Database::CreateInstance(uint16 instance_id, uint32 zone_id, uint32 version, uint32 duration)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if(RunQuery(query, MakeAnyLenString(&query, "INSERT INTO instance_lockout (id, zone, version, start_time, duration)" 
		" values(%lu, %lu, %lu, UNIX_TIMESTAMP(), %lu)", instance_id, zone_id, version, duration), errbuf))
	{
		safe_delete_array(query);
		return true;
	}
	else 
	{
		safe_delete_array(query);
		return false;
	}
}

void Database::PurgeExpiredInstances()
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	int16 id = 0;
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT id FROM instance_lockout where "
			"(start_time+duration)<=UNIX_TIMESTAMP()"), errbuf, &result))
	{
		safe_delete_array(query);
		if (mysql_num_rows(result) > 0) 
		{
			row = mysql_fetch_row(result);
			while(row != NULL)
			{
				id = atoi(row[0]);
				DeleteInstance(id);
				row = mysql_fetch_row(result);
			}
		}
		mysql_free_result(result);
	}
	else 
	{
		safe_delete_array(query);
	}
}

bool Database::AddClientToInstance(uint16 instance_id, uint32 char_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if(RunQuery(query, MakeAnyLenString(&query, "INSERT INTO instance_lockout_player(id, charid) "
			"values(%lu, %lu)", instance_id, char_id), errbuf))
	{
		safe_delete_array(query);
		return true;
	}
	else 
	{
		safe_delete_array(query);
		return false;
	}
}

bool Database::RemoveClientFromInstance(uint16 instance_id, uint32 char_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if(RunQuery(query, MakeAnyLenString(&query, "DELETE FROM instance_lockout_player WHERE id=%lu AND charid=%lu", 
		instance_id, char_id), errbuf))
	{
		safe_delete_array(query);
		return true;
	}
	else 
	{
		safe_delete_array(query);
		return false;
	}
}

bool Database::CheckInstanceExists(uint16 instance_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT * FROM instance_lockout where id=%u", instance_id), 
		errbuf, &result))
	{
		safe_delete_array(query);
		if (mysql_num_rows(result) != 0) 
		{
			mysql_free_result(result);
			return true;
		}
		mysql_free_result(result);
		return false;
	}
	else 
	{
		safe_delete_array(query);
		return false;
	}
	return false;
}

void Database::BuryCorpsesInInstance(uint16 instance_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if(RunQuery(query, MakeAnyLenString(&query, "UPDATE player_corpses SET IsBurried=1, instanceid=0 WHERE instanceid=%u", 
		instance_id), errbuf, &result))
	{
		mysql_free_result(result);
	}
	safe_delete_array(query);
}

int16 Database::GetInstanceVersion(uint16 instance_id)
{
	if(instance_id < 1)
		return 0;

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int32 ret;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT version FROM instance_lockout where id=%u", instance_id), 
		errbuf, &result))
	{
		safe_delete_array(query);
		if (mysql_num_rows(result) != 0) 
		{
			row = mysql_fetch_row(result);
			ret = atoi(row[0]);
			mysql_free_result(result);
			return ret;			
		}
		else
		{
			mysql_free_result(result);
			return 0;
		}
	}
	else 
	{
		safe_delete_array(query);
		return 0;
	}
	return 0;
}

int16 Database::GetInstanceID(const char* zone, int32 charid, int16 version)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int16 ret;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT instance_lockout.id FROM instance_lockout, instance_lockout_player "
		"WHERE instance_lockout.zone=%u AND instance_lockout.version=%u AND instance_lockout.id=instance_lockout_player.id AND "
		"instance_lockout_player.charid=%u LIMIT 1;", GetZoneID(zone), version, charid, charid), errbuf, &result))
	{
		safe_delete_array(query);
		if (mysql_num_rows(result) != 0) 
		{
			row = mysql_fetch_row(result);
			ret = atoi(row[0]);
			mysql_free_result(result);
			return ret;		
		}
		else
		{
			mysql_free_result(result);
			return 0;
		}
	}
	else 
	{
		safe_delete_array(query);
		return 0;
	}
	return 0;
}

int16 Database::GetInstanceID(int32 zone, int32 charid, int16 version)
{
	if(!zone)
		return 0;

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int16 ret;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT instance_lockout.id FROM instance_lockout, instance_lockout_player "
		"WHERE instance_lockout.zone=%u AND instance_lockout.version=%u AND instance_lockout.id=instance_lockout_player.id AND "
		"instance_lockout_player.charid=%u LIMIT 1;", zone, version, charid), errbuf, &result))
	{
		safe_delete_array(query);
		if (mysql_num_rows(result) != 0) 
		{
			row = mysql_fetch_row(result);
			ret = atoi(row[0]);
			mysql_free_result(result);
			return ret;		
		}
		else
		{
			mysql_free_result(result);
			return 0;
		}
	}
	else 
	{
		safe_delete_array(query);
		return 0;
	}
	return 0;
}

void Database::AssignGroupToInstance(int32 gid, int32 instance_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int32 zone_id = ZoneIDFromInstanceID(instance_id);
	int16 version = VersionFromInstanceID(instance_id);

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT charid FROM group_id WHERE groupid=%u", gid), 
		errbuf, &result))
	{
		safe_delete_array(query);
		while((row = mysql_fetch_row(result)) != NULL)
		{
			int32 charid = atoi(row[0]);
			if(GetInstanceID(zone_id, charid, version) == 0)
			{
				AddClientToInstance(instance_id, charid);
			}
		}
		mysql_free_result(result);
	}
	else 
	{
		safe_delete_array(query);
	}
}

void Database::AssignRaidToInstance(int32 rid, int32 instance_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int32 zone_id = ZoneIDFromInstanceID(instance_id);
	int16 version = VersionFromInstanceID(instance_id);

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT charid FROM raid_members WHERE raidid=%u", rid), 
		errbuf, &result))
	{
		safe_delete_array(query);
		while((row = mysql_fetch_row(result)) != NULL)
		{
			int32 charid = atoi(row[0]);
			if(GetInstanceID(zone_id, charid, version) == 0)
			{
				AddClientToInstance(instance_id, charid);
			}
		}
		mysql_free_result(result);
	}
	else 
	{
		safe_delete_array(query);
	}
}

void Database::FlagInstanceByGroupLeader(int32 zone, int16 version, int32 charid, int32 gid)
{
	int16 id = GetInstanceID(zone, charid, version);
	if(id != 0)
		return;

	char ln[128];
	memset(ln, 0, 128);
	strcpy(ln, GetGroupLeadershipInfo(gid, ln));
	int32 l_charid = GetCharacterID((const char*)ln);
	int16 l_id = GetInstanceID(zone, l_charid, version);

	if(l_id == 0)
		return;

	AddClientToInstance(l_id, charid);
}

void Database::FlagInstanceByRaidLeader(int32 zone, int16 version, int32 charid, int32 rid)
{
	int16 id = GetInstanceID(zone, charid, version);
	if(id != 0)
		return;

	int32 l_charid = GetCharacterID(GetRaidLeaderName(rid));
	int16 l_id = GetInstanceID(zone, l_charid, version);

	if(l_id == 0)
		return;

	AddClientToInstance(l_id, charid);
}

void Database::SetInstanceDuration(int16 instance_id, int32 new_duration)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if(RunQuery(query, MakeAnyLenString(&query, "UPDATE `instance_lockout` SET start_time=UNIX_TIMESTAMP(), "
		"duration=%u WHERE id=%u", new_duration, instance_id), errbuf))
	{
		safe_delete_array(query);
	}
	else
	{
		//error
		safe_delete_array(query);
	}
}

void Database::GroupAdventureLevelAndRange(int32 gid, int32 &avg_level, int32 &range)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int16 m_avg_level = 0;
	int8 num_in_group = 0;
	int16 min_level = 2000;
	int16 max_level = 0;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT character_.level FROM character_, group_id"
		" WHERE character_.id=group_id.charid AND group_id.groupid=%u", gid), errbuf, &result))
	{
		safe_delete_array(query);
		while((row = mysql_fetch_row(result)) != NULL)
		{
			int16 m_lvl = atoi(row[0]);
			m_avg_level += m_lvl;
			if(m_lvl < min_level)
				min_level = m_lvl;

			if(m_lvl > max_level)
				max_level = m_lvl;
			num_in_group++;
		}
		mysql_free_result(result);
	}
	else 
	{
		safe_delete_array(query);
	}
	avg_level = (m_avg_level / num_in_group);
	range = max_level-min_level;
}

void Database::RaidAdventureLevelAndRange(int32 rid, int32 &avg_level, int32 &range)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int16 m_avg_level = 0;
	int8 num_in_group = 0;
	int16 min_level = 2000;
	int16 max_level = 0;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT raid_members.level FROM raid_members "
		"WHERE raid_members.raidid=%u", rid), errbuf, &result))
	{
		safe_delete_array(query);
		while((row = mysql_fetch_row(result)) != NULL)
		{
			int16 m_lvl = atoi(row[0]);
			m_avg_level += m_lvl;
			if(m_lvl < min_level)
				min_level = m_lvl;

			if(m_lvl > max_level)
				max_level = m_lvl;
			num_in_group++;
		}
		mysql_free_result(result);
	}
	else 
	{
		safe_delete_array(query);
	}
	avg_level = (m_avg_level / num_in_group);
	range = max_level-min_level;
}

int32 Database::CreateAdventure(int32 adventure_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	int32 affected_rows = 0;
	int32 last_insert_id = 0;

    if (!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO `adventure_details` SET adventure_id=%u,"
		" time_created=UNIX_TIMESTAMP()", adventure_id), errbuf, 0, &affected_rows, &last_insert_id)) {
		safe_delete_array(query);
		return 0;
    }
	safe_delete_array(query);
	
	if (affected_rows == 0) 
	{
		return 0;
	}

	if (last_insert_id == 0) 
	{
		return 0;
	}
	return last_insert_id;
}

void Database::AddPlayerToAdventure(int32 id, int32 charid)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if(RunQuery(query, MakeAnyLenString(&query, "INSERT INTO `adventure_members` SET"
		" id=%u, charid=%u", id, charid), errbuf))
	{
		safe_delete_array(query);
	}
	else
	{
		//error
		safe_delete_array(query);
	}
}

void Database::RemovePlayerFromAdventure(int32 id, int32 charid)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if(RunQuery(query, MakeAnyLenString(&query, "DELETE FROM `adventure_members` WHERE"
		" id=%u AND charid=%u", id, charid), errbuf))
	{
		safe_delete_array(query);
	}
	else
	{
		//error
		safe_delete_array(query);
	}
}

void Database::AddGroupToAdventure(int32 id, int32 gid)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT charid FROM group_id "
		"WHERE groupid=%u", gid), errbuf, &result))
	{
		safe_delete_array(query);
		while((row = mysql_fetch_row(result)) != NULL)
		{
			int32 charid = atoi(row[0]);
			AddPlayerToAdventure(id, charid);
		}
		mysql_free_result(result);
	}
	else 
	{
		safe_delete_array(query);
	}
}

void Database::AddRaidToAdventure(int32 id, int32 rid)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT charid FROM raid_members "
		"WHERE raidid=%u", rid), errbuf, &result))
	{
		safe_delete_array(query);
		while((row = mysql_fetch_row(result)) != NULL)
		{
			int32 charid = atoi(row[0]);
			AddPlayerToAdventure(id, charid);
		}
		mysql_free_result(result);
	}
	else 
	{
		safe_delete_array(query);
	}
}

void Database::DestroyAdventure(int32 id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if(RunQuery(query, MakeAnyLenString(&query, "DELETE FROM `adventure_details` WHERE id=%u", id), errbuf))
	{
		safe_delete_array(query);
	}
	else
	{
		//error
		safe_delete_array(query);
	}

	if(RunQuery(query, MakeAnyLenString(&query, "DELETE FROM `adventure_members` WHERE id=%u", id), errbuf))
	{
		safe_delete_array(query);
	}
	else
	{
		//error
		safe_delete_array(query);
	}
}

bool Database::GetAdventureDetails(int32 charid, int32 &id, int32 &adventure_id, int32 &instance_id, int32 &count, 
								   int32 &ass_count, int32 &status, int32 &time_c, int32 &time_z, int32 &time_comp)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int32 adv_id = 0;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT `id` FROM `adventure_members` WHERE charid=%u LIMIT 1", 
		charid), errbuf, &result))
	{
		safe_delete_array(query);
		while((row = mysql_fetch_row(result)) != NULL)
		{
			adv_id = atoi(row[0]);
		}
		mysql_free_result(result);
	}
	else 
	{
		safe_delete_array(query);
	}

	if(adv_id == 0)
		return false;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT `adventure_id`, `instance_id`, `count`, `assassinate_count`, `status`, "
		"`time_created`, `time_zoned`, `time_completed` FROM `adventure_details` WHERE id=%u LIMIT 1", adv_id), errbuf, &result))
	{
		safe_delete_array(query);
		while((row = mysql_fetch_row(result)) != NULL)
		{
			adventure_id = atoi(row[0]);
			instance_id = atoi(row[1]);
			count = atoi(row[2]);
			ass_count = atoi(row[3]);
			status = atoi(row[4]);
			time_c = atoi(row[5]);
			time_z = atoi(row[6]);
			time_comp = atoi(row[7]);
			id = adv_id;
		}
		mysql_free_result(result);
		return true;
	}
	else 
	{
		safe_delete_array(query);
		return false;
	}
}

int32 Database::CountPlayersInAdventure(int32 id) 
{ 
	//SELECT `charid` FROM `adventure_members` WHERE id=%u
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	int count = 0;
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT `charid` FROM `adventure_members` WHERE "
		"id=%u", id), errbuf, &result))
	{
		safe_delete_array(query);
		while((row = mysql_fetch_row(result)) != NULL)
		{
			count++;
		}
		mysql_free_result(result);
	}
	else 
	{
		safe_delete_array(query);
	}
	return count;
}

void Database::PurgeAdventures() 
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if(RunQuery(query, MakeAnyLenString(&query, "DELETE FROM `adventure_details`"), errbuf))
	{
		safe_delete_array(query);
	}
	else
	{
		//error
		safe_delete_array(query);
	}

	if(RunQuery(query, MakeAnyLenString(&query, "DELETE FROM `adventure_members`"), errbuf))
	{
		safe_delete_array(query);
	}
	else
	{
		//error
		safe_delete_array(query);
	}
}

void Database::AddAdventureToInstance(int32 adv_id, int32 inst_id) 
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT `charid` FROM `adventure_members` WHERE id=%u", 
		adv_id), errbuf, &result))
	{
		safe_delete_array(query);
		while((row = mysql_fetch_row(result)) != NULL)
		{
			int32 id = atoi(row[0]);
			AddClientToInstance(inst_id, id);
		}
		mysql_free_result(result);
	}
	else 
	{
		safe_delete_array(query);
	}
}

void Database::UpdateAdventureStatus(int32 adv_id, int32 status) 
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	if(RunQuery(query, MakeAnyLenString(&query, "UPDATE `adventure_details` SET status=%u WHERE id=%u", 
		status, adv_id), errbuf))
	{
		safe_delete_array(query);
	}
	else
	{
		//error
		safe_delete_array(query);
	}
}

void Database::UpdateAdventureInstance(int32 adv_id, int32 inst_id, int32 time) 
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	if(RunQuery(query, MakeAnyLenString(&query, "UPDATE `adventure_details` SET instance_id=%d, "
		"time_zoned=%u WHERE id=%u", inst_id, time, adv_id), errbuf))
	{
		safe_delete_array(query);
	}
	else
	{
		//error
		safe_delete_array(query);
	}
}

void Database::UpdateAdventureCompleted(int32 adv_id, int32 time)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	if(RunQuery(query, MakeAnyLenString(&query, "UPDATE `adventure_details` SET time_completed=%u "
		"WHERE id=%u", time, adv_id), errbuf))
	{
		safe_delete_array(query);
	}
	else
	{
		//error
		safe_delete_array(query);
	}
}

void Database::UpdateAdventureCount(int32 adv_id, int32 new_count)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	if(RunQuery(query, MakeAnyLenString(&query, "UPDATE `adventure_details` SET count=%u "
		"WHERE id=%u", new_count, adv_id), errbuf))
	{
		safe_delete_array(query);
	}
	else
	{
		//error
		safe_delete_array(query);
	}
}

void Database::IncrementAdventureCount(int32 adv_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	if(RunQuery(query, MakeAnyLenString(&query, "UPDATE `adventure_details` SET count=count+1 "
		"WHERE id=%u", adv_id), errbuf))
	{
		safe_delete_array(query);
	}
	else
	{
		//error
		safe_delete_array(query);
	}
}

int32 Database::GetAdventureCount(int32 adv_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT `count` FROM `adventure_details` WHERE id=%u", 
		adv_id), errbuf, &result))
	{
		safe_delete_array(query);
		while((row = mysql_fetch_row(result)) != NULL)
		{
			int32 count = atoi(row[0]);
			return count;
		}
		mysql_free_result(result);
	}
	else 
	{
		safe_delete_array(query);
		return 0;
	}
	return 0;	
}

bool Database::AdventureStatsEntryExists(int32 char_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT `player_id` FROM `adventure_stats` WHERE player_id=%u", 
		char_id), errbuf, &result))
	{
		safe_delete_array(query);
		while((row = mysql_fetch_row(result)) != NULL)
		{
			return true;
		}
		mysql_free_result(result);
	}
	else
	{
		safe_delete_array(query);
		return false;
	}
	return false;
}

bool Database::AdventureExists(int32 adv_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT `id` FROM `adventure_details` WHERE id=%u", 
		adv_id), errbuf, &result))
	{
		safe_delete_array(query);
		while((row = mysql_fetch_row(result)) != NULL)
		{
			return true;
		}
		mysql_free_result(result);
	}
	else
	{
		safe_delete_array(query);
		return false;
	}
	return false;
}

void Database::UpdateAdventureStatsEntry(int32 char_id, int8 theme, bool win)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	std::string field;

	if(win)
	{
		switch(theme)
		{
			case 1:
			{
				field = "guk_wins";
				break;
			}
			case 2:
			{
				field = "mir_wins";
				break;
			}
			case 3:
			{
				field = "mmc_wins";
				break;
			}
			case 4:
			{
				field = "ruj_wins";
				break;
			}
			case 5:
			{
				field = "tak_wins";
				break;
			}
			default:
			{
				return;
			}
		}
	}
	else
	{
		switch(theme)
		{
			case 1:
			{
				field = "guk_losses";
				break;
			}
			case 2:
			{
				field = "mir_losses";
				break;
			}
			case 3:
			{
				field = "mmc_losses";
				break;
			}
			case 4:
			{
				field = "ruj_losses";
				break;
			}
			case 5:
			{
				field = "tak_losses";
				break;
			}
			default:
			{
				return;
			}
		}
	}

	if(AdventureStatsEntryExists(char_id))
	{
		if(RunQuery(query, MakeAnyLenString(&query, "UPDATE `adventure_stats` SET %s=%s+1 WHERE player_id=%u",
			field.c_str(), field.c_str(), char_id), errbuf))
		{
			safe_delete_array(query);
		}
		else
		{
			//error
			safe_delete_array(query);
		}
	}
	else
	{
		if(RunQuery(query, MakeAnyLenString(&query, "INSERT INTO `adventure_stats` SET %s=1, player_id=%u",
			field.c_str(), char_id), errbuf))
		{
			safe_delete_array(query);
		}
		else
		{
			//error
			safe_delete_array(query);
		}
	}
}

void Database::UpdateAllAdventureStatsEntry(int32 adv_id, int8 theme, bool win)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if(!AdventureExists(adv_id))
		return;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT `charid` FROM `adventure_members` WHERE id=%u", 
		adv_id), errbuf, &result))
	{
		safe_delete_array(query);
		while((row = mysql_fetch_row(result)) != NULL)
		{
			int32 charid = atoi(row[0]);
			UpdateAdventureStatsEntry(charid, theme, win);
		}
		mysql_free_result(result);
	}
	else
	{
		safe_delete_array(query);
	}
}

bool Database::GetAdventureStats(int32 char_id, int32 &guk_w, int32 &mir_w, int32 &mmc_w, int32 &ruj_w, 
								 int32 &tak_w, int32 &guk_l, int32 &mir_l, int32 &mmc_l, int32 &ruj_l, int32 &tak_l)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int32 adv_id = 0;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT `guk_wins`, `mir_wins`, `mmc_wins`, `ruj_wins`, `tak_wins`, "
		"`guk_losses`, `mir_losses`, `mmc_losses`, `ruj_losses`, `tak_losses` FROM `adventure_stats` WHERE player_id=%u", 
		char_id), errbuf, &result))
	{
		safe_delete_array(query);
		while((row = mysql_fetch_row(result)) != NULL)
		{
			guk_w = atoi(row[0]);
			mir_w = atoi(row[1]);
			mmc_w = atoi(row[2]);
			ruj_w = atoi(row[3]);
			tak_w = atoi(row[4]);
			guk_l = atoi(row[5]);
			mir_l = atoi(row[6]);
			mmc_l = atoi(row[7]);
			ruj_l = atoi(row[8]);
			tak_l = atoi(row[9]);
		}
		mysql_free_result(result);
		return true;
	}
	else 
	{
		safe_delete_array(query);
		return false;
	}
}