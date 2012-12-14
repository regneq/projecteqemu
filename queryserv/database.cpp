/*
	EQEMu:  Everquest Server Emulator

	Copyright (C) 2001-2008 EQEMu Development Team (http://eqemulator.net)

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
#ifdef _WINDOWS
#include <windows.h>
#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#else
#include "../common/unix.h"
#include <netinet/in.h>
#endif

#include "database.h"
#include "../common/eq_packet_structs.h"
#include "../common/MiscFunctions.h"
#include "../common/servertalk.h"

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

}



void Database::HandleMysqlError(int32 errnum) {
}

/*

Close the connection to the database
*/
Database::~Database()
{
}

bool Database::GetVariable(const char* varname, char* varvalue, int16 varvalue_len) {

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (!RunQuery(query,MakeAnyLenString(&query, "select `value` from `variables` where `varname`='%s'", varname), errbuf, &result)) {

		_log(UCS__ERROR, "Unable to get message count from database. %s %s", query, errbuf);

		safe_delete_array(query);

		return false;
	}

	safe_delete_array(query);

	if (mysql_num_rows(result) != 1) {

		mysql_free_result(result);

		return false;
	}

	row = mysql_fetch_row(result);

	snprintf(varvalue, varvalue_len, "%s", row[0]);

	mysql_free_result(result);

	return true;
}


void Database::AddSpeech(const char* from, const char* to, const char* message, int16 minstatus, uint32 guilddbid, int8 type) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;

	char *S1 = new char[strlen(from) * 2 + 1];
	char *S2 = new char[strlen(to) * 2 + 1];
	char *S3 = new char[strlen(message) * 2 + 1];
	DoEscapeString(S1, from, strlen(from));
	DoEscapeString(S2, to, strlen(to));
	DoEscapeString(S3, message, strlen(message));

	if(!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO `qs_player_speech` SET `from`='%s', `to`='%s', `message`='%s', `minstatus`='%i', `guilddbid`='%i', `type`='%i'", S1, S2, S3, minstatus, guilddbid, type), errbuf, 0, 0)) {
		_log(NET__WORLD, "Failed Speech Entry Insert: %s", errbuf);
		_log(NET__WORLD, "%s", query);
	}

	safe_delete_array(query);
	safe_delete_array(S1);
	safe_delete_array(S2);
	safe_delete_array(S3);
}

void Database::LogPlayerTrade(const char* from, const char* to, const char* ItemName, uint32 ItemID, sint16 SlotID, sint16 Charges){
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	char *S1 = new char[strlen(from) * 2 + 1];
	char *S2 = new char[strlen(to) * 2 + 1];
	char *S3 = new char[strlen(ItemName) * 2 + 1];

	DoEscapeString(S1, from, strlen(from));
	DoEscapeString(S2, to, strlen(to));
	DoEscapeString(S3, ItemName, strlen(ItemName));

	if(!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO `qs_player_trades` SET `from`='%s', `to`='%s', `item_name`='%s', `item_id`='%i', `slot_id`='%i', `charges`='%i', `time`=NOW()", S1, S2, S3, ItemID, SlotID, Charges), errbuf, 0, 0)) {
		_log(NET__WORLD, "Failed player log trade Insert: %s", errbuf);
		_log(NET__WORLD, "%s", query);
	}

	safe_delete_array(query); safe_delete_array(S1); safe_delete_array(S2); safe_delete_array(S3);
}

void Database::LogPlayerMoneyTrade(const char* from, const char* to, uint32 Copper, uint32 Silver, uint32 Gold, uint32 Platinum){
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	char *S1 = new char[strlen(from) * 2 + 1];
	char *S2 = new char[strlen(to) * 2 + 1];
	DoEscapeString(S1, from, strlen(from));
	DoEscapeString(S2, to, strlen(to));

	if(!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO `qs_player_money_trades` SET `from`='%s', `to`='%s', `copper`='%i', `silver`='%i', `gold`='%i', `platinum`='%i', `time`=NOW()", S1, S2, Copper, Silver, Gold, Platinum), errbuf, 0, 0)) {
		_log(NET__WORLD, "Failed player money log trade Insert: %s", errbuf);
		_log(NET__WORLD, "%s", query);
	}

	safe_delete_array(query); safe_delete_array(S1); safe_delete_array(S2);
}

void Database::LogPlayerNPCKill(QSPlayerLogNPCKill_Struct* QS, int32 Members){
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	int32 lastid = 0;
	if(!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO `qs_player_npc_kill_record` SET `npc_id`='%i', `type`='%i', `zone_id`='%i', `time`=NOW()", QS->s1.NPCID, QS->s1.Type, QS->s1.ZoneID), errbuf, 0, 0, &lastid)) {
		_log(NET__WORLD, "Failed NPC Kill Log Record Insert: %s", errbuf);
		_log(NET__WORLD, "%s", query);
	}

	if(Members > 0){
		for (int i = 0; i < Members; i++) {
			if(!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO `qs_player_npc_kill_record_entries` SET `event_id`='%i', `char_id`='%i'", lastid, QS->Chars[i].char_id, errbuf, 0, 0))) {
				_log(NET__WORLD, "Failed NPC Kill Log Entry Insert: %s", errbuf);
				_log(NET__WORLD, "%s", query);
			}
		}
	}
}