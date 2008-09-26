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
#ifndef TITLES_H
#define TITLES_H

#include "../common/types.h"
#include <vector>
#include <string>

class Client;
class EQApplicationPacket;

using namespace std;

class TitleManager {
public:
	TitleManager();

	bool LoadTitles();
	
	EQApplicationPacket *MakeTitlesPacket(Client *who);
	
	bool IsValidTitle(Client *who, const char *title);
	
protected:
	class TitleEntry {
	public:
		SkillType skill_id;			//the skill which skill_value applies to
		uint16 skill_value;		//the minimum value of skill `skill_id` to qualify for this title
		uint8 aa_points;		//minimum number of AA points needed to qualify for this title
		string title;
	};
	vector<TitleEntry> titles;
};

extern TitleManager title_manager;

#endif

