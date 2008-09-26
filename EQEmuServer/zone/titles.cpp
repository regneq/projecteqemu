/*  EQEMu:  Everquest Server Emulator
	Copyright (C) 2001-2005  EQEMu Development Team (http://eqemulator.net)

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
#include "../common/eq_packet_structs.h"
#include "masterentity.h"
#include "titles.h"
#include "../common/MiscFunctions.h"

/*

create table titles (
	id int UNSIGNED primary key AUTO_INCREMENT,
	skill_id tinyint UNSIGNED NOT NULL,
	skill_value mediumint UNSIGNED NOT NULL,
	aa_points TINYINT UNSIGNED NOT NULL,
	title varchar(32) NOT NULL
);

*/


TitleManager::TitleManager() {
}

bool TitleManager::LoadTitles() {
	TitleEntry e;
	
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = NULL;
	MYSQL_RES *result;
	MYSQL_ROW row;
	if (!database.RunQuery(query, MakeAnyLenString(&query, 
		"SELECT skill_id, skill_value, title FROM titles"
	), errbuf, &result)) {
		safe_delete_array(query);
		return(false);
	}
	safe_delete_array(query);
	
	while ((row = mysql_fetch_row(result))) {
		int tmp_skill = atoi(row[0]);
		if(tmp_skill < 0 || tmp_skill > HIGHEST_SKILL)
			continue;
		e.skill_id = (SkillType) tmp_skill;
		e.skill_value = atoi(row[1]);
		e.title = row[2];
		titles.push_back(e);
	}
	mysql_free_result(result);
	
	return(true);
}

EQApplicationPacket *TitleManager::MakeTitlesPacket(Client *who) {
	EQApplicationPacket *outapp = NULL;
	vector<TitleEntry>::iterator cur,end;
	vector< vector<TitleEntry>::iterator > avaliable;
	uint32 len = 0;

	cur = titles.begin();
	end = titles.end();
	for(; cur != end; cur++) {
		uint32 v = who->GetSkill(cur->skill_id);
		if(v < cur->skill_value)
			continue;	//not high enough
		avaliable.push_back(cur);
		len += cur->title.length();
	}

	uint32 count = avaliable.size();
	if(count == 0) {
		//no titles avaliable...
		outapp = new EQApplicationPacket(OP_CustomTitles, 4);
		return(outapp);
	}
	
	uint32 pos = 0;
	uint32 total_len = sizeof(Titles_Struct) + sizeof(TitleEntry_Struct)*count + len;
	outapp = new EQApplicationPacket(OP_CustomTitles, total_len);
	
	Titles_Struct *header = (Titles_Struct *) outapp->pBuffer;
	header->title_count = count;
	pos += sizeof(Titles_Struct);

	TitleEntry_Struct *e;
	vector< vector<TitleEntry>::iterator >::iterator cura, enda;
	cura = avaliable.begin();
	enda = avaliable.end();
	for(; cura != enda; cura++) {
		//get the current entry
		cur = *cura;
		e = (TitleEntry_Struct *) (outapp->pBuffer + pos);
		
		//fill out the packet
		e->skill_id = cur->skill_id;
		e->skill_value = cur->skill_value;
		len = cur->title.length();
		strncpy(e->title, cur->title.c_str(), len+1);
		
		//advance our position in the buffer
		pos += sizeof(TitleEntry_Struct) + len;
	}
	return(outapp);
}
	
bool TitleManager::IsValidTitle(Client *who, const char *title) {
	vector<TitleEntry>::iterator cur,end;

	cur = titles.begin();
	end = titles.end();
	for(; cur != end; cur++) {
		if((*cur).title != title)
			continue;	//not this title.
		uint32 v = who->GetSkill((*cur).skill_id);
		if(v < (*cur).skill_value) {
			return(false);	//requirement not met
		}
		return(true);	//requirement met
	}
	//title not found
	return(false);
}














