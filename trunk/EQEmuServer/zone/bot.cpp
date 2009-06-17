#ifdef BOTS

#include "bot.h"

Bot::Bot(uint32 botID, uint32 botOwnerCharacterID, uint32 botInventoryID, uint32 botSpellsID, std::string botName, std::string botLastName, uint8 botLevel, uint16 botRace, uint8 botClass, uint8 botBodyType, sint32 hitPoints, uint8 gender, float size, uint32 hitPointsRegenRate, uint32 manaRegenRate, uint32 face, uint32 hairStyle, uint32 hairColor, uint32 eyeColor, uint32 eyeColor2, uint32 beardColor, uint32 beard, uint32 drakkinHeritage, uint32 drakkinTattoo, uint32 drakkinDetails, float runSpeed, sint16 mr, sint16 cr, sint16 dr, sint16 fr, sint16 pr, sint16 ac, uint16 str, uint16 sta, uint16 dex, uint16 agi, uint16 _int, uint16 wis, uint16 cha, uint16 attack)
: Mob(botName.c_str(),
	  botLastName.c_str(),
	  hitPoints,
	  0,
	  gender,
	  botRace,
	  botClass,
	  (bodyType)botBodyType,
	  0,
	  botLevel,
	  0,
	  size,
	  runSpeed,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  ac,
	  attack,
	  str,
	  sta,
	  dex,
	  agi,
	  _int,
	  wis,
	  cha,
	  hairColor,
	  beardColor,
	  eyeColor,
	  eyeColor2,
	  hairStyle,
	  face,
	  beard,
	  drakkinHeritage,
	  drakkinTattoo,
	  drakkinDetails,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  hitPointsRegenRate,
	  manaRegenRate,
	  0) {
		  this->_botID = botID;
		  this->_botOwnerCharacterID = botOwnerCharacterID;
		  this->_botSpellID = botSpellsID;
		  this->_botInventoryID = botInventoryID;
		  this->_isSpawned = false;

		  GenerateBaseStats();
		  GenerateAppearance();
		  GenerateArmorClass();

		  // Calculate HitPoints Last As It Uses Base Stats
		  GenerateBaseHitPoints();
}

Bot::Bot(std::string botName, uint8 botClass, uint16 botRace, uint8 botGender, Client* botOwner)
: Mob(botName.c_str(),
	  0,
	  0,
	  0,
	  botGender,
	  botRace,
	  botClass,
	  (bodyType)0,
	  0,
	  botOwner->GetLevel(),
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0,
	  0) {
		  if(botOwner) {
			  this->_botOwnerCharacterID = botOwner->AccountID();
		  }
		  else
			  this->_botOwnerCharacterID = 0;

		  this->_botID = 0;
		  this->_botSpellID = 0;
		  this->_botInventoryID = 0;
		  this->_isSpawned = false;

		  GenerateBaseStats();
		  GenerateAppearance();
		  GenerateArmorClass();

		  // Calculate HitPoints Last As It Uses Base Stats
		  GenerateBaseHitPoints();
}

Bot::~Bot() {
	Mob::~Mob();
}

void Bot::GenerateBaseStats() {
	int BotSpellID = 0;

	// base stats
	uint16 Strength = 75;
	uint16 Stamina = 75;
	uint16 Dexterity = 75;
	uint16 Agility = 75;
	uint16 Wisdom = 75;
	uint16 Intelligence = 75;
	uint16 Charisma = 75;
	uint16 Attack = 5;
	sint16 MagicResist = 25;
	sint16 FireResist = 25;
	sint16 DiseaseResist = 15;
	sint16 PoisonResist = 15;
	sint16 ColdResist = 25;

	switch(this->GetClass()) {
			case 1: // Warrior
				Strength += 10;
				Stamina += 20;
				Agility += 10;
				Dexterity += 10;
				Attack += 12;
				MagicResist += (1 / 2 + 1);
				break;
			case 2: // Cleric
				BotSpellID = 701;
				Strength += 5;
				Stamina += 5;
				Agility += 10;
				Wisdom += 30;
				Attack += 8;
				break;
			case 3: // Paladin
				BotSpellID = 708;
				Strength += 15;
				Stamina += 5;
				Wisdom += 15;
				Charisma += 10;
				Dexterity += 5;
				Attack =+ 17;
				DiseaseResist += 8;
				break;
			case 4: // Ranger
				BotSpellID = 710;
				Strength += 15;
				Stamina += 10;
				Agility += 10;
				Wisdom += 15;
				Attack += 17;
				FireResist += 4;
				ColdResist += 4;
				break;
			case 5: // Shadowknight
				BotSpellID = 709;
				Strength += 10;
				Stamina += 15;
				Intelligence += 20;
				Charisma += 5;
				Attack += 17;
				PoisonResist += 4;
				DiseaseResist += 4;
				break;
			case 6: // DiseaseResistuid
				BotSpellID = 707;
				Stamina += 15;
				Wisdom += 35;
				Attack += 5;
				break;
			case 7: // Monk
				Strength += 5;
				Stamina += 15;
				Agility += 15;
				Dexterity += 15;
				Attack += 17;
				break;
			case 8: // Bard
				BotSpellID = 711;
				Strength += 15;
				Dexterity += 10;
				Charisma += 15;
				Intelligence += 10;
				Attack += 17;
				break;
			case 9: // Rogue
				Strength += 10;
				Stamina += 20;
				Agility += 10;
				Dexterity += 10;
				Attack += 12;
				PoisonResist += 8;
				break;
			case 10: // Shaman
				BotSpellID = 706;
				Stamina += 10;
				Wisdom += 30;
				Charisma += 10;
				Attack += 28;
				break;
			case 11: // NeColdResistomancer
				BotSpellID = 703;
				Dexterity += 10;
				Agility += 10;
				Intelligence += 30;
				Attack += 5;
				break;
			case 12: // Wizard
				BotSpellID = 702;
				Stamina += 20;
				Intelligence += 30;
				Attack += 5;
				break;
			case 13: // Magician
				BotSpellID = 704;
				Stamina += 20;
				Intelligence += 30;
				Attack += 5;
				break;
			case 14: // Enchanter
				BotSpellID = 705;
				Intelligence += 25;
				Charisma += 25;
				Attack += 5;
				break;
			case 15: // Beastlord
				BotSpellID = 712;
				Stamina += 10;
				Agility += 10;
				Dexterity += 5;
				Wisdom += 20;
				Charisma += 5;
				Attack += 31;
				break;
			case 16: // Berserker
				Strength += 10;
				Stamina += 15;
				Dexterity += 15;
				Agility += 10;
				Attack += 25;
				break;
	}

	float BotSize = 6;

	switch(this->GetRace()) {
			case 1: // Humans have no race bonus
				break;
			case 2: // Barbarian
				Strength += 28;
				Stamina += 20;
				Agility += 7;
				Dexterity -= 5;
				Wisdom -= 5;
				Intelligence -= 10;
				Charisma -= 20;
				BotSize = 7;
				ColdResist += 10;
				break;
			case 3: // Erudite
				Strength -= 15;
				Stamina -= 5;
				Agility -= 5;
				Dexterity -= 5;
				Wisdom += 8;
				Intelligence += 32;
				Charisma -= 5;
				MagicResist += 5;
				DiseaseResist -= 5;
				break;
			case 4: // Wood Elf
				Strength -= 10;
				Stamina -= 10;
				Agility += 20;
				Dexterity += 5;
				Wisdom += 5;
				BotSize = 5;
				break;
			case 5: // High Elf
				Strength -= 20;
				Stamina -= 10;
				Agility += 10;
				Dexterity -= 5;
				Wisdom += 20;
				Intelligence += 12;
				Charisma += 5;
				break;
			case 6: // Dark Elf
				Strength -= 15;
				Stamina -= 10;
				Agility += 15;
				Wisdom += 8;
				Intelligence += 24;
				Charisma -= 15;
				BotSize = 5;
				break;
			case 7: // Half Elf
				Strength -= 5;
				Stamina -= 5;
				Agility += 15;
				Dexterity += 10;
				Wisdom -= 15;
				BotSize = 5.5;
				break;
			case 8: // Dwarf
				Strength += 15;
				Stamina += 15;
				Agility -= 5;
				Dexterity += 15;
				Wisdom += 8;
				Intelligence -= 15;
				Charisma -= 30;
				BotSize = 4;
				MagicResist -= 5;
				PoisonResist += 5;
				break;
			case 9: // Troll
				Strength += 33;
				Stamina += 34;
				Agility += 8;
				Wisdom -= 15;
				Intelligence -= 23;
				Charisma -= 35;
				BotSize = 8;
				FireResist -= 20;
				break;
			case 10: // Ogre
				Strength += 55;
				Stamina += 77;
				Agility -= 5;
				Dexterity -= 5;
				Wisdom -= 8;
				Intelligence -= 15;
				Charisma -= 38;
				BotSize = 9;
				break;
			case 11: // Halfling
				Strength -= 5;
				Agility += 20;
				Dexterity += 15;
				Wisdom += 5;
				Intelligence -= 8;
				Charisma -= 25;
				BotSize = 3.5;
				PoisonResist += 5;
				DiseaseResist += 5;
				break;
			case 12: // Gnome
				Strength -= 15;
				Stamina -= 5;
				Agility += 10;
				Dexterity += 10;
				Wisdom -= 8;
				Intelligence += 23;
				Charisma -= 15;
				BotSize = 3;
				break;
			case 128: // Iksar
				Strength -= 5;
				Stamina -= 5;
				Agility += 15;
				Dexterity += 10;
				Wisdom += 5;
				Charisma -= 20;
				MagicResist -= 5;
				FireResist -= 5;
				break;
			case 130: // Vah Shir
				Strength += 15;
				Agility += 15;
				Dexterity -= 5;
				Wisdom -= 5;
				Intelligence -= 10;
				Charisma -= 10;
				BotSize = 7;
				MagicResist -= 5;
				FireResist -= 5;
				break;
			case 330: // FireResistoglok
				Strength -= 5;
				Stamina += 5;
				Agility += 25;
				Dexterity += 25;
				Charisma -= 25;
				BotSize = 5;
				MagicResist -= 5;
				FireResist -= 5;
				break;
	}

	this->STR = Strength;
	this->STA = Stamina;
	this->DEX = Dexterity;
	this->AGI = Agility;
	this->WIS = Wisdom;
	this->INT = Intelligence;
	this->CHA = Charisma;
	this->ATK = Attack;
	this->MR = MagicResist;
	this->FR = FireResist;
	this->DR = DiseaseResist;
	this->PR = PoisonResist;
	this->CR = ColdResist;
	this->_botSpellID = BotSpellID;
	this->size = BotSize;
}

void Bot::GenerateAppearance() {
	// Randomize facial appearance
	int iFace = 0;
	if(this->GetRace() == 2) { // Barbarian w/Tatoo
		iFace = MakeRandomInt(0, 79);
	}
	else {
		iFace = MakeRandomInt(0, 7);
	}
	int iHair = 0;
	int iBeard = 0;
	int iBeardColor = 1;
	if(this->GetGender()) {
		iHair = MakeRandomInt(0, 2);
		if(this->GetRace() == 8) { // Dwarven Females can have a beard
			if(MakeRandomInt(1, 100) < 50) {
				iFace += 10;
			}
		}
	}
	else {
		iHair = MakeRandomInt(0, 3);
		iBeard = MakeRandomInt(0, 5);
		iBeardColor = MakeRandomInt(0, 19);
	}
	int iHairColor = MakeRandomInt(0, 19);
	int iEyeColor1 = MakeRandomInt(0, 9);
	int iEyeColor2 = 0;
	if(MakeRandomInt(1, 100) > 96) {
		iEyeColor2 = MakeRandomInt(0, 9);
	}
	else {
		iEyeColor2 = iEyeColor1;
	}

	this->luclinface = iFace;
	this->hairstyle = iHair;
	this->beard = iBeard;
	this->beardcolor = iBeardColor;
	this->haircolor = iHairColor;
	this->eyecolor1 = iEyeColor1;
	this->eyecolor2 = iEyeColor2;
}

void Bot::GenerateArmorClass() {
	// Base AC
	int bac = (1 * 3) * 4;
	switch(this->GetClass()) {
			case WARRIOR:
			case SHADOWKNIGHT:
			case PALADIN:
				bac = bac*1.5;
	}

	this->AC = bac;
}

void Bot::GenerateBaseHitPoints() {
	// Calc Base Hit Points
	int16 multiplier = 1;
	switch(this->GetClass()) {
			case WARRIOR:
				multiplier = 220;
				break;
			case DRUID:
			case CLERIC:
			case SHAMAN:
				multiplier = 150;
				break;
			case BERSERKER:
			case PALADIN:
			case SHADOWKNIGHT:
				multiplier = 210;
				break;
			case MONK:
			case BARD:
			case ROGUE:
			case BEASTLORD:
				multiplier = 180;
				break;
			case RANGER:
				multiplier = 200;
				break;
			case MAGICIAN:
			case WIZARD:
			case NECROMANCER:
			case ENCHANTER:
				multiplier = 120;
				break;
	}
	int16 lm = multiplier;
	int16 Post255;
	if((this->GetSTA()-255)/2 > 0)
		Post255 = (this->GetSTA()-255)/2;
	else
		Post255 = 0;
	int base_hp = (5)+(1*lm/10) + (((this->GetSTA()-Post255)*1*lm/3000)) + ((Post255*1)*lm/6000);

	this->base_hp = base_hp;
	this->cur_hp = base_hp;
}

bool Bot::IsValidRaceClassCombo() {
	bool Result = false;

	switch(GetRace()) {
		case 1: // Human
			switch(GetClass()) {
				case 1: // Warrior
				case 2: // Cleric
				case 3: // Paladin
				case 4: // Ranger
				case 5: // Shadowknight
				case 6: // Druid
				case 7: // Monk
				case 8: // Bard
				case 9: // Rogue
				case 11: // Necromancer
				case 12: // Wizard
				case 13: // Magician
				case 14: // Enchanter
					Result = true;
					break;
			}
			break;
		case 2: // Barbarian
			switch(GetClass()) {
				case 1: // Warrior
				case 9: // Rogue
				case 10: // Shaman
				case 15: // Beastlord
				case 16: // Berserker
					Result = true;
					break;
			}
			break;
		case 3: // Erudite
				switch(GetClass()) {
			case 2: // Cleric
			case 3: // Paladin
			case 5: // Shadowknight
			case 11: // Necromancer
			case 12: // Wizard
			case 13: // Magician
			case 14: // Enchanter
				Result = true;
				break;
				}
				break;
			case 4: // Wood Elf
				switch(GetClass()) {
			case 1: // Warrior
			case 4: // Ranger
			case 6: // Druid
			case 8: // Bard
			case 9: // Rogue
				Result = true;
				break;
				}
				break;
			case 5: // High Elf
				switch(GetClass()) {
			case 2: // Cleric
			case 3: // Paladin
			case 12: // Wizard
			case 13: // Magician
			case 14: // Enchanter
				Result = true;
				break;
				}
				break;
			case 6: // Dark Elf
				switch(GetClass()) {
			case 1: // Warrior
			case 2: // Cleric
			case 5: // Shadowknight
			case 9: // Rogue
			case 11: // Necromancer
			case 12: // Wizard
			case 13: // Magician
			case 14: // Enchanter
				Result = true;
				break;
				}
				break;
			case 7: // Half Elf
				switch(GetClass()) {
			case 1: // Warrior
			case 3: // Paladin
			case 4: // Ranger
			case 6: // Druid
			case 8: // Bard
			case 9: // Rogue
				Result = true;
				break;
				}
				break;
			case 8: // Dwarf
				switch(GetClass()) {
			case 1: // Warrior
			case 2: // Cleric
			case 3: // Paladin
			case 9: // Rogue
			case 16: // Berserker
				Result = true;
				break;
				}
				break;
			case 9: // Troll
				switch(GetClass()) {
			case 1: // Warrior
			case 5: // Shadowknight
			case 10: // Shaman
			case 15: // Beastlord
			case 16: // Berserker
				Result = true;
				break;
				}
				break;
			case 10: // Ogre
				switch(GetClass()) {
			case 1: // Warrior
			case 5: // Shadowknight
			case 10: // Shaman
			case 15: // Beastlord
			case 16: // Berserker
				Result = true;
				break;
				}
				break;
			case 11: // Halfling
				switch(GetClass()) {
			case 1: // Warrior
			case 2: // Cleric
			case 3: // Paladin
			case 4: // Ranger
			case 6: // Druid
			case 9: // Rogue
				Result = true;
				break;
				}
				break;
			case 12: // Gnome
				switch(GetClass()) {
			case 1: // Warrior
			case 2: // Cleric
			case 3: // Paladin
			case 5: // Shadowknight
			case 9: // Rogue
			case 11: // Necromancer
			case 12: // Wizard
			case 13: // Magician
			case 14: // Enchanter
				Result = true;
				break;
				}
				break;
			case 128: // Iksar
				switch(GetClass()) {
			case 1: // Warrior
			case 5: // Shadowknight
			case 7: // Monk
			case 10: // Shaman
			case 11: // Necromancer
			case 15: // Beastlord
				Result = true;
				break;
				}
				break;
			case 130: // Vah Shir
				switch(GetClass()) {
			case 1: // Warrior
			case 8: // Bard
			case 9: // Rogue
			case 10: // Shaman
			case 15: // Beastlord
			case 16: // Berserker
				Result = true;
				break;
				}
				break;
			case 330: // Froglok
				switch(GetClass()) {
			case 1: // Warrior
			case 2: // Cleric
			case 3: // Paladin
			case 5: // Shadowknight
			case 9: // Rogue
			case 10: // Shaman
			case 11: // Necromancer
			case 12: // Wizard
				Result = true;
				break;
				}
				break;
	}

	return Result;
}

bool Bot::IsValidName() {
	bool Result = false;
	std::string TempBotName = std::string(this->GetName());

	for(int iCounter = 0; iCounter < TempBotName.length(); iCounter++) {
		if(isalpha(TempBotName[iCounter]) || TempBotName[iCounter] == '_') {
			Result = true;
		}
	}

	return Result;
}

bool Bot::IsBotNameAvailable(std::string* errorMessage) {
	bool Result = false;

	if(this->GetName()) {
		char* Query = 0;
		char TempErrorMessageBuffer[MYSQL_ERRMSG_SIZE];
		MYSQL_RES* DatasetResult;
		MYSQL_ROW DataRow;

		if(!database.RunQuery(Query, MakeAnyLenString(&Query, "SELECT COUNT(BotID) FROM bots WHERE Name LIKE '%s'", this->GetName()), TempErrorMessageBuffer, &DatasetResult)) {
			*errorMessage = std::string(TempErrorMessageBuffer);
		}
		else {
			uint32 ExistingNameCount = 0;

			while(DataRow = mysql_fetch_row(DatasetResult)) {
				ExistingNameCount = atoi(DataRow[0]);
				break;
			}

			if(ExistingNameCount == 0)
				Result = true;

			mysql_free_result(DatasetResult);
		}

		safe_delete(Query);
	}

	return Result;
}

uint32 Bot::CreateNewBotRecord(std::string *errorMessage) {
	uint32 Result = 0;
	char* Query = 0;
	char TempErrorMessageBuffer[MYSQL_ERRMSG_SIZE];

	if(!database.RunQuery(Query, MakeAnyLenString(&Query, "INSERT INTO bots (BotOwnerCharacterID, BotSpellsID, Name, LastName, BotLevel, Race, Class, BodyType, HitPoints, Gender, Size, HitPointsRegenRate, ManaRegenRate, Face, LuclinHairStyle, LuclinHairColor, LuclinEyeColor, LuclinEyeColor2, LuclinBeardColor, LuclinBeard, DrakkinHeritage, DrakkinDetails, RunSpeed, MR, CR, DR, FR, PR, AC, STR, STA, DEX, AGI, _INT, WIS, CHA, ATK) VALUES('%u', '%u', '%s', '%s', '%u', '%u', '%u', '%i', '%i', '%u', '%f', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%f', '%i', '%i', '%i', '%i', '%i', '%i', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u')", this->_botOwnerCharacterID, 0, this->GetName(), this->lastname, this->GetLevel(), GetRace(), GetClass(), GetBodyType(), this->GetMaxHP(), GetGender(), GetSize(), this->hp_regen, this->mana_regen, this->GetLuclinFace(), this->GetHairStyle(), GetHairColor(), this->GetEyeColor1(), GetEyeColor2(), this->GetBeardColor(), this->GetBeard(), this->GetDrakkinHeritage(), this->GetDrakkinTattoo(), GetDrakkinDetails(), this->GetRunspeed(), GetMR(), GetCR(), GetDR(), GetFR(), GetPR(), GetAC(), GetSTR(), GetSTA(), GetDEX(), GetAGI(), GetINT(), GetWIS(), GetCHA(), GetATK()), TempErrorMessageBuffer, 0, 0, &Result)) {
		*errorMessage = std::string(TempErrorMessageBuffer);
	}

	safe_delete(Query);

	return Result;
}

std::string Bot::SaveBot() {
	// Result is a string buffer for an error message. Return value of NULL indicates a successful database operation.
	std::string Result;

	if(_botID > 0) {
		char* Query = 0;
		char TempErrorMessageBuffer[MYSQL_ERRMSG_SIZE];

		// TODO: Finish filling out this UPDATE query

		if(!database.RunQuery(Query, MakeAnyLenString(&Query, "UPDATE bots SET BotOwnerCharacterID = '%u', BotSpellsID = '%u', Name = '%s', LastName = '%s', BotLevel = '%u' WHERE BotID = '%u'", _botOwnerCharacterID, _botSpellID, this->GetName(), this->lastname, this->GetLevel(), _botID), TempErrorMessageBuffer)) {
			Result = std::string(TempErrorMessageBuffer);
		}
	}

	return Result;
}

std::string Bot::DeleteBot() {
	// Result is a string buffer for an error message. Return value of NULL indicates a successful database operation.
	std::string Result;

	if(_botID > 0) {
		char* Query = 0;
		char TempErrorMessageBuffer[MYSQL_ERRMSG_SIZE];

		// TODO: These queries need to be ran together as a transaction.. ie, if one or more fail then they all will fail to commit to the database.

		if(!database.RunQuery(Query, MakeAnyLenString(&Query, "DELETE FROM bots WHERE BotID = '%u'", this->_botID), TempErrorMessageBuffer)) {
			Result = std::string(TempErrorMessageBuffer);
		}

		// TODO: alter table botinventory modify npctypeid to botid
		if(!database.RunQuery(Query, MakeAnyLenString(&Query, "DELETE FROM botinventory WHERE npctypeid = '%u'", this->_botID), TempErrorMessageBuffer)) {
			Result = std::string(TempErrorMessageBuffer);
		}

		// TODO: alter table botsowners modify botnpctypeid to botid
		if(!database.RunQuery(Query, MakeAnyLenString(&Query, "DELETE FROM botsowners WHERE botnpctypeid = '%u'", this->_botID), TempErrorMessageBuffer)) {
			Result = std::string(TempErrorMessageBuffer);
		}
	}

	return Result;
}

void Bot::Spawn(float xPos, float yPos, float zPos, float heading) {
	if(this->_botID > 0 && this->_botOwnerCharacterID > 0) {
		// TODO: Spawn code goes here
	}
}

void Bot::SetBotLeader(uint32 botID, uint32 botOwnerCharacterID, std::string botName, std::string zoneShortName, std::string* errorMessage) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if(!database.RunQuery(query, MakeAnyLenString(&query, "INSERT INTO botleader SET botid=%i, leaderid=%i, bot_name='%s', zone_name='%s'", botID, botOwnerCharacterID, botName.c_str(), zoneShortName.c_str()), errbuf)) {
		*errorMessage = std::string(errbuf);
	}

	safe_delete_array(query);
}

uint32 Bot::GetBotLeader(uint32 botID, std::string* errorMessage) {
	uint32 Result = 0;

	if(botID > 0) {
		char ErrBuf[MYSQL_ERRMSG_SIZE];
		char* Query = 0;
		MYSQL_RES* DatasetResult;
		MYSQL_ROW DataRow;

		if(database.RunQuery(Query, MakeAnyLenString(&Query, "SELECT leaderid FROM botleader WHERE botid=%i", botID), ErrBuf, &DatasetResult)) {
			if(mysql_num_rows(DatasetResult) == 1) {
				DataRow = mysql_fetch_row(DatasetResult);
				Result = atoi(DataRow[0]);
			}

			mysql_free_result(DatasetResult);
		}
		else
			*errorMessage = std::string(ErrBuf);

		safe_delete_array(Query);
	}

	return Result;
}

static Bot* LoadBot(uint32 botID, std::string* errorMessage) {
	Bot* Result = 0;

	if(botID > 0) {
		char* Query = 0;
		char TempErrorMessageBuffer[MYSQL_ERRMSG_SIZE];
		MYSQL_RES* DatasetResult;
		MYSQL_ROW DataRow;

		if(!database.RunQuery(Query, MakeAnyLenString(&Query, "SELECT BotOwnerCharacterID, BotInventoryID, BotSpellsID, Name, LastName, BotLevel, Race, Class, BodyType, HitPoints, Gender, Size, HitPointsRegenRate, ManaRegenRate, Face, LuclinHairStyle, LuclinHairColor, LuclinEyeColor, LuclinEyeColor2, LuclinBeardColor, LuclinBeard, DrakkinHeritage, DrakkinTattoo, DrakkinDetails, RunSpeed, MR, CR, DR, FR, PR, AC, STR, STA, DEX, AGI, _INT, WIS, CHA, ATK FROM bots WHERE BotID = '%u'", botID), TempErrorMessageBuffer, &DatasetResult)) {
			*errorMessage = std::string(TempErrorMessageBuffer);
		}
		else {
			while(DataRow = mysql_fetch_row(DatasetResult)) {
				Result = new Bot(botID, atoi(DataRow[0]), atoi(DataRow[1]), atoi(DataRow[2]), std::string(DataRow[3]), std::string(DataRow[4]), atoi(DataRow[5]), atoi(DataRow[6]), atoi(DataRow[7]), atoi(DataRow[8]), atoi(DataRow[9]), atoi(DataRow[10]), atof(DataRow[11]), atoi(DataRow[12]), atoi(DataRow[13]), atoi(DataRow[14]), atoi(DataRow[15]), atoi(DataRow[16]), atoi(DataRow[17]), atoi(DataRow[18]), atoi(DataRow[19]), atoi(DataRow[20]), atoi(DataRow[21]), atoi(DataRow[22]), atoi(DataRow[23]), atof(DataRow[24]), atoi(DataRow[25]), atoi(DataRow[26]), atoi(DataRow[27]), atoi(DataRow[28]), atoi(DataRow[29]), atoi(DataRow[30]), atoi(DataRow[31]), atoi(DataRow[32]), atoi(DataRow[33]), atoi(DataRow[34]), atoi(DataRow[35]), atoi(DataRow[36]), atoi(DataRow[37]), atoi(DataRow[37]));
				break;
			}

			mysql_free_result(DatasetResult);
			safe_delete_array(Query);
		}
	}

	return Result;
}

static std::list<BotsAvailableList> GetBotList(uint32 botOwnerCharacterID, std::string* errorMessage) {
	std::list<BotsAvailableList> Result;

	if(botOwnerCharacterID > 0) {
		// TODO:
	}

	return Result;
}

static std::list<SpawnedBotsList> ListSpawnedBots(uint32 characterID, std::string* errorMessage) {
	std::list<SpawnedBotsList> Result;
	char ErrBuf[MYSQL_ERRMSG_SIZE];
	char* Query = 0;
	MYSQL_RES* DatasetResult;
	MYSQL_ROW DataRow;

	if(characterID > 0) {
		if(!database.RunQuery(Query, MakeAnyLenString(&Query, "SELECT bot_name, zone_name FROM botleader WHERE leaderid=%i", characterID), ErrBuf, &DatasetResult)) {
			*errorMessage = std::string(ErrBuf);
		}

		uint32 RowCount = mysql_num_rows(DatasetResult);
		if(RowCount > 0) {
			for(int iCounter = 0; iCounter < RowCount; iCounter++) {
				DataRow = mysql_fetch_row(DatasetResult);
				SpawnedBotsList TempSpawnedBotsList;
				TempSpawnedBotsList.BotLeaderCharID = characterID;
				strcpy(TempSpawnedBotsList.BotName, DataRow[0]);
				strcpy(TempSpawnedBotsList.ZoneName, DataRow[1]);

				Result.push_back(TempSpawnedBotsList);
			}
		}

		mysql_free_result(DatasetResult);
		safe_delete_array(Query);
	}

	return Result;
}

static void SaveBotGroups(uint32 groupID, uint32 characterID, uint32 botID, uint16 slotID, std::string* errorMessage) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if(!database.RunQuery(query, MakeAnyLenString(&query, "INSERT into botgroups (groupid, charid, botid, slot) values (%i, %i, %i, %i)", groupID, characterID, botID, slotID), errbuf)) {
		*errorMessage = std::string(errbuf);
	}

	safe_delete_array(query);
}

static void DeleteBotGroups(uint32 characterID, std::string* errorMessage) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if(!database.RunQuery(query, MakeAnyLenString(&query, "DELETE FROM botgroups where charid=%i", characterID), errbuf)) {
		*errorMessage = std::string(errbuf);
	}

	safe_delete_array(query);
}

static std::list<BotGroup> LoadBotGroups(uint32 characterID, std::string* errorMessage) {
	std::list<BotGroup> Result;
	char ErrBuf[MYSQL_ERRMSG_SIZE];
	char* Query = 0;
	MYSQL_RES* DatasetResult;
	MYSQL_ROW DataRow;

	if(characterID > 0) {
		if(!database.RunQuery(Query, MakeAnyLenString(&Query, "SELECT groupid, botid FROM botgroups WHERE charid=%i ORDER BY charid, groupid, slot", characterID), ErrBuf, &DatasetResult)) {
			*errorMessage = std::string(ErrBuf);
		}
		else {
			uint32 RowCount = mysql_num_rows(DatasetResult);

			if(RowCount > 0) {
				for(int iCounter = 0; iCounter < RowCount; iCounter++) {
					DataRow = mysql_fetch_row(DatasetResult);
					BotGroup TempBotGroup;
					TempBotGroup.CharacterID = characterID;
					TempBotGroup.GroupID = atoi(DataRow[0]);
					TempBotGroup.BotID = atoi(DataRow[1]);

					Result.push_back(TempBotGroup);
				}
			}

			mysql_free_result(DatasetResult);
		}

		safe_delete_array(Query);
	}

	return Result;
}

static uint32 AllowedBotSpawns(uint32 botOwnerCharacterID, std::string* errorMessage) {
	uint32 Result = 0;

	if(botOwnerCharacterID > 0) {
		char ErrBuf[MYSQL_ERRMSG_SIZE];
		char* Query = 0;
		MYSQL_RES* DatasetResult;
		MYSQL_ROW DataRow;

		if(database.RunQuery(Query, MakeAnyLenString(&Query, "SELECT value FROM quest_globals WHERE name='bot_spawn_limit' and charid=%i", botOwnerCharacterID), ErrBuf, &DatasetResult)) {
			if(mysql_num_rows(DatasetResult) == 1) {
				DataRow = mysql_fetch_row(DatasetResult);
				Result = atoi(DataRow[0]);
			}

			mysql_free_result(DatasetResult);
		}
		else
			*errorMessage = std::string(ErrBuf);

		safe_delete_array(Query);
	}

	return Result;
}
	
static uint32 SpawnedBotCount(uint32 botOwnerCharacterID, std::string* errorMessage) {
	uint32 Result = 0;

	if(botOwnerCharacterID > 0) {
		char ErrBuf[MYSQL_ERRMSG_SIZE];
		char* Query = 0;
		MYSQL_RES* DatasetResult;
		MYSQL_ROW DataRow;

		if(database.RunQuery(Query, MakeAnyLenString(&Query, "SELECT COUNT(*) FROM botleader WHERE leaderid=%i", botOwnerCharacterID), ErrBuf, &DatasetResult)) {
			if(mysql_num_rows(DatasetResult) == 1) {
				DataRow = mysql_fetch_row(DatasetResult);
				Result = atoi(DataRow[0]);
			}

			mysql_free_result(DatasetResult);
		}
		else
			*errorMessage = std::string(ErrBuf);

		safe_delete_array(Query);
	}

	return Result;
}

static void ProcessBotCommands(Client *c, const Seperator *sep) {
	// TODO: All bot command processing occurs here now instead of in command.cpp

	if(sep->arg[1][0] == '\0') {
		c->Message(15, "Bad argument, type #bot help");
		return;
	}
	if(!strcasecmp( sep->arg[1], "help") && !strcasecmp( sep->arg[2], "\0")){
		c->Message(15, "List of commands availables for bots :");
		c->Message(15, "#bot help - show this");
		c->Message(15, "#bot create [name] [class (id)] [race (id)] [model (male/female)] - create a permanent bot. See #bot help create.");
		c->Message(15, "#bot help create - show all the race/class id. (make it easier to create bots)");
		c->Message(15, "#bot delete - completely destroy forever the targeted bot and all its items.");
		c->Message(15, "#bot list [all/class(1-16)] - list your bots all or by class. Classes: 1(Warrior), 2(Cleric), 3(Paladin), 4(Ranger), 5(Sk), 6(Druid), 7(Monk), 8(Bard), 9(Rogue), 10(Shaman), 11(Necro), 12(Wiz), 13(Mag), 14(Ench), 15(Beast), 16(Bersek)");
		c->Message(15, "#bot spawn [botid] - spawn a bot from its ID (use list to see all the bots). ");
		c->Message(15, "#bot group add - make the targetted bot joigning your group.");
		c->Message(15, "#bot group remove [target} - kick the targetted bot from your group (it will die also).");
		c->Message(15, "#bot group order [follow/guard/attack (target)] - Give orders [follow/guard/attack (target)] to your grouped bots.");
		c->Message(15, "#bot inventory list - show the inventory (and the slots IDs) of the targetted bot.");
		c->Message(15, "#bot inventory remove [slotid] - remove the item at the given slot in the inventory of the targetted bot.");
		c->Message(15, "#bot update - you must type that command once you gain a level.");
		c->Message(15, "#bot group summon - It will summon all your grouped bots to you.");
		c->Message(15, "#bot summon - It will summon your targeted bot to you.");
		c->Message(15, "#bot ai mez - If you're grouped with an enchanter, he will mez your target.");
		c->Message(15, "#bot picklock - You must have a targeted rogue bot in your group and be right on the door.");
		c->Message(15, "#bot cure [poison|disease|curse|blindness] Cleric has most options");
		c->Message(15, "#bot bindme - You must have a Cleric in your group to get Bind Affinity cast on you.");
		c->Message(15, "#bot raid [commands] (#bot raid help will show some help).");
		c->Message(15, "#bot track - look at mobs in the zone (ranger has options)");
		c->Message(15, "#bot target calm - attempts to pacify your target mob.");
		c->Message(15, "#bot evac - transports your pc group to safe location in the current zone. bots are lost");
		c->Message(15, "#bot resurrectme - Your bot Cleric will rez you.");
		c->Message(15, "#bot corpse summon - Necromancers summon corpse.");
		c->Message(15, "#bot lore - cast Identify on the item on your mouse pointer.");
		c->Message(15, "#bot sow - Bot sow on you (Druid has options)");
		c->Message(15, "#bot invis - Bot invisiblity (must have proper class in group)");
		c->Message(15, "#bot levitate - Bot levitation (must have proper class in group)");
		c->Message(15, "#bot resist - Bot resist buffs (must have proper class in group)");
		c->Message(15, "#bot runeme - Enchanter Bot cast Rune spell on you");
		c->Message(15, "#bot shrinkme - Shaman Bot will shrink you");
		c->Message(15, "#bot endureb - Bot enduring breath (must have proper class in group)");
		c->Message(15, "#bot charm - (must have proper class in group)");
		c->Message(15, "#bot dire charm - (must have proper class in group)");
		c->Message(15, "#bot pet remove - (remove pet before charm)");
		c->Message(15, "#bot gate - you need a Druid or Wizard in your group)");
		c->Message(15, "#bot saveraid - Save your current group(s) of bots.");
		c->Message(15, "#bot spawnraid - Spawns your saved bots.");
		c->Message(15, "#bot groupraid - Groups your spawned bots.");
		c->Message(15, "#bot archery - Toggle Archery Skilled bots between using a Bow or using Melee weapons.");
		c->Message(15, "#bot magepet [earth|water|air|fire|monster] - Select the pet type you want your Mage bot to use.");
		return;
	}

//	if(!strcasecmp(sep->arg[1], "create")) {
//		if(sep->arg[2][0] == '\0' || sep->arg[3][0] == '\0' || sep->arg[4][0] == '\0' || sep->arg[5][0] == '\0' || sep->arg[6][0] != '\0') {
//			c->Message(15, "Usage: #bot create [name] [class(id)] [race(id)] [gender (male/female)]");
//			return;
//		}
//		else if(strcasecmp(sep->arg[3],"1") && strcasecmp(sep->arg[3],"2") && strcasecmp(sep->arg[3],"3") && strcasecmp(sep->arg[3],"4") && strcasecmp(sep->arg[3],"5") && strcasecmp(sep->arg[3],"6") && strcasecmp(sep->arg[3],"7") && strcasecmp(sep->arg[3],"8") && strcasecmp(sep->arg[3],"9") && strcasecmp(sep->arg[3],"10") && strcasecmp(sep->arg[3],"11") && strcasecmp(sep->arg[3],"12") && strcasecmp(sep->arg[3],"13") && strcasecmp(sep->arg[3],"14") && strcasecmp(sep->arg[3],"15") && strcasecmp(sep->arg[3],"16")) {
//			c->Message(15, "Usage: #bot create [name] [class(id)] [race(id)] [gender (male/female)]");
//			return;
//		}		
//		else if(strcasecmp(sep->arg[4],"1") && strcasecmp(sep->arg[4],"2") && strcasecmp(sep->arg[4],"3") && strcasecmp(sep->arg[4],"4") && strcasecmp(sep->arg[4],"5") && strcasecmp(sep->arg[4],"6") && strcasecmp(sep->arg[4],"7") && strcasecmp(sep->arg[4],"8") && strcasecmp(sep->arg[4],"9") && strcasecmp(sep->arg[4],"10") && strcasecmp(sep->arg[4],"11") && strcasecmp(sep->arg[4],"12") && strcasecmp(sep->arg[4],"330") && strcasecmp(sep->arg[4],"128") && strcasecmp(sep->arg[4],"130")) {
//			c->Message(15, "Usage: #bot create [name] [class(1-16)] [race(1-12,128,130,330)] [gender (male/female)]");
//			return;
//		}
//		else if(strcasecmp(sep->arg[5],"male") && strcasecmp(sep->arg[5],"female")) {
//			c->Message(15, "Usage: #bot create [name] [class(1-16)] [race(1-12,128,130,330)] [gender (male/female)]");
//			return;
//		}
//
//		if(database.CountBots(c->AccountID()) >= RuleI(EQOffline, CreateBotCount)) {
//			c->Message(15, "You cannot create more than %i bots.", RuleI(EQOffline, CreateBotCount));
//			return;
//		}
//
//		// Check Race/Class combos
//		int choosebclass = atoi(sep->arg[3]);
//		int iRace = atoi(sep->arg[4]);
//		bool isComboAllowed = false;
//		switch(iRace) {
//			case 1: // Human
//				switch(choosebclass) {
//			case 1: // Warrior
//			case 2: // Cleric
//			case 3: // Paladin
//			case 4: // Ranger
//			case 5: // Shadowknight
//			case 6: // Druid
//			case 7: // Monk
//			case 8: // Bard
//			case 9: // Rogue
//			case 11: // Necromancer
//			case 12: // Wizard
//			case 13: // Magician
//			case 14: // Enchanter
//				isComboAllowed = true;
//				break;
//				}
//				break;
//			case 2: // Barbarian
//				switch(choosebclass) {
//			case 1: // Warrior
//			case 9: // Rogue
//			case 10: // Shaman
//			case 15: // Beastlord
//			case 16: // Berserker
//				isComboAllowed = true;
//				break;
//				}
//				break;
//			case 3: // Erudite
//				switch(choosebclass) {
//			case 2: // Cleric
//			case 3: // Paladin
//			case 5: // Shadowknight
//			case 11: // Necromancer
//			case 12: // Wizard
//			case 13: // Magician
//			case 14: // Enchanter
//				isComboAllowed = true;
//				break;
//				}
//				break;
//			case 4: // Wood Elf
//				switch(choosebclass) {
//			case 1: // Warrior
//			case 4: // Ranger
//			case 6: // Druid
//			case 8: // Bard
//			case 9: // Rogue
//				isComboAllowed = true;
//				break;
//				}
//				break;
//			case 5: // High Elf
//				switch(choosebclass) {
//			case 2: // Cleric
//			case 3: // Paladin
//			case 12: // Wizard
//			case 13: // Magician
//			case 14: // Enchanter
//				isComboAllowed = true;
//				break;
//				}
//				break;
//			case 6: // Dark Elf
//				switch(choosebclass) {
//			case 1: // Warrior
//			case 2: // Cleric
//			case 5: // Shadowknight
//			case 9: // Rogue
//			case 11: // Necromancer
//			case 12: // Wizard
//			case 13: // Magician
//			case 14: // Enchanter
//				isComboAllowed = true;
//				break;
//				}
//				break;
//			case 7: // Half Elf
//				switch(choosebclass) {
//			case 1: // Warrior
//			case 3: // Paladin
//			case 4: // Ranger
//			case 6: // Druid
//			case 8: // Bard
//			case 9: // Rogue
//				isComboAllowed = true;
//				break;
//				}
//				break;
//			case 8: // Dwarf
//				switch(choosebclass) {
//			case 1: // Warrior
//			case 2: // Cleric
//			case 3: // Paladin
//			case 9: // Rogue
//			case 16: // Berserker
//				isComboAllowed = true;
//				break;
//				}
//				break;
//			case 9: // Troll
//				switch(choosebclass) {
//			case 1: // Warrior
//			case 5: // Shadowknight
//			case 10: // Shaman
//			case 15: // Beastlord
//			case 16: // Berserker
//				isComboAllowed = true;
//				break;
//				}
//				break;
//			case 10: // Ogre
//				switch(choosebclass) {
//			case 1: // Warrior
//			case 5: // Shadowknight
//			case 10: // Shaman
//			case 15: // Beastlord
//			case 16: // Berserker
//				isComboAllowed = true;
//				break;
//				}
//				break;
//			case 11: // Halfling
//				switch(choosebclass) {
//			case 1: // Warrior
//			case 2: // Cleric
//			case 3: // Paladin
//			case 4: // Ranger
//			case 6: // Druid
//			case 9: // Rogue
//				isComboAllowed = true;
//				break;
//				}
//				break;
//			case 12: // Gnome
//				switch(choosebclass) {
//			case 1: // Warrior
//			case 2: // Cleric
//			case 3: // Paladin
//			case 5: // Shadowknight
//			case 9: // Rogue
//			case 11: // Necromancer
//			case 12: // Wizard
//			case 13: // Magician
//			case 14: // Enchanter
//				isComboAllowed = true;
//				break;
//				}
//				break;
//			case 128: // Iksar
//				switch(choosebclass) {
//			case 1: // Warrior
//			case 5: // Shadowknight
//			case 7: // Monk
//			case 10: // Shaman
//			case 11: // Necromancer
//			case 15: // Beastlord
//				isComboAllowed = true;
//				break;
//				}
//				break;
//			case 130: // Vah Shir
//				switch(choosebclass) {
//			case 1: // Warrior
//			case 8: // Bard
//			case 9: // Rogue
//			case 10: // Shaman
//			case 15: // Beastlord
//			case 16: // Berserker
//				isComboAllowed = true;
//				break;
//				}
//				break;
//			case 330: // Froglok
//				switch(choosebclass) {
//			case 1: // Warrior
//			case 2: // Cleric
//			case 3: // Paladin
//			case 5: // Shadowknight
//			case 9: // Rogue
//			case 10: // Shaman
//			case 11: // Necromancer
//			case 12: // Wizard
//				isComboAllowed = true;
//				break;
//				}
//				break;
//		}
//		if(!isComboAllowed) {
//			c->Message(15, "That Race/Class combination cannot be created.");
//			return;
//		}
//
//		const char* botName = sep->arg[2];
//		if(!botName || (strlen(botName) < 4) || (strlen(botName) > 40)) {
//			c->Message(15, "%s is too many characters.", botName);
//			return;
//		}
//
//		for(int i=0; botName[i]; i++) {
//			if(!isalpha(botName[i])) {
//				if(botName[i] != '_') {
//					c->Message(15, "%s can only use A-Z, a-z and _ ", botName);
//					return;
//				}
//			}
//		}
//
//		int spellid = 0;
//		// base stats
//		uint16 bstr = 75;
//		uint16 bsta = 75;
//		uint16 bdex = 75;
//		uint16 bagi = 75;
//		uint16 bwis = 75;
//		uint16 bint = 75;
//		uint16 bcha = 75;
//		uint16 ATK = 5;
//		sint16 MR = 25;
//		sint16 FR = 25;
//		sint16 DR = 15;
//		sint16 PR = 15;
//		sint16 CR = 25;
//
//		switch(choosebclass) {
//			case 1: // Warrior
//				bstr += 10;
//				bsta += 20;
//				bagi += 10;
//				bdex += 10;
//				ATK += 12;
//				MR += (1 / 2 + 1);
//				break;
//			case 2: // Cleric
//				spellid = 701;
//				bstr += 5;
//				bsta += 5;
//				bagi += 10;
//				bwis += 30;
//				ATK += 8;
//				break;
//			case 3: // Paladin
//				spellid = 708;
//				bstr += 15;
//				bsta += 5;
//				bwis += 15;
//				bcha += 10;
//				bdex += 5;
//				ATK =+ 17;
//				DR += 8;
//				break;
//			case 4: // Ranger
//				spellid = 710;
//				bstr += 15;
//				bsta += 10;
//				bagi += 10;
//				bwis += 15;
//				ATK += 17;
//				FR += 4;
//				CR += 4;
//				break;
//			case 5: // Shadowknight
//				spellid = 709;
//				bstr += 10;
//				bsta += 15;
//				bint += 20;
//				bcha += 5;
//				ATK += 17;
//				PR += 4;
//				DR += 4;
//				break;
//			case 6: // Druid
//				spellid = 707;
//				bsta += 15;
//				bwis += 35;
//				ATK += 5;
//				break;
//			case 7: // Monk
//				bstr += 5;
//				bsta += 15;
//				bagi += 15;
//				bdex += 15;
//				ATK += 17;
//				break;
//			case 8: // Bard
//				spellid = 711;
//				bstr += 15;
//				bdex += 10;
//				bcha += 15;
//				bint += 10;
//				ATK += 17;
//				break;
//			case 9: // Rogue
//				bstr += 10;
//				bsta += 20;
//				bagi += 10;
//				bdex += 10;
//				ATK += 12;
//				PR += 8;
//				break;
//			case 10: // Shaman
//				spellid = 706;
//				bsta += 10;
//				bwis += 30;
//				bcha += 10;
//				ATK += 28;
//				break;
//			case 11: // Necromancer
//				spellid = 703;
//				bdex += 10;
//				bagi += 10;
//				bint += 30;
//				ATK += 5;
//				break;
//			case 12: // Wizard
//				spellid = 702;
//				bsta += 20;
//				bint += 30;
//				ATK += 5;
//				break;
//			case 13: // Magician
//				spellid = 704;
//				bsta += 20;
//				bint += 30;
//				ATK += 5;
//				break;
//			case 14: // Enchanter
//				spellid = 705;
//				bint += 25;
//				bcha += 25;
//				ATK += 5;
//				break;
//			case 15: // Beastlord
//				spellid = 712;
//				bsta += 10;
//				bagi += 10;
//				bdex += 5;
//				bwis += 20;
//				bcha += 5;
//				ATK += 31;
//				break;
//			case 16: // Berserker
//				bstr += 10;
//				bsta += 15;
//				bdex += 15;
//				bagi += 10;
//				ATK += 25;
//				break;
//		}
//
//		int gender = 0;
//		if(!strcasecmp(sep->arg[5], "female"))
//			gender = 1;
//
//		float bsize = 6;
//		switch(iRace) {
//			case 1: // Humans have no race bonus
//				break;
//			case 2: // Barbarian
//				bstr += 28;
//				bsta += 20;
//				bagi += 7;
//				bdex -= 5;
//				bwis -= 5;
//				bint -= 10;
//				bcha -= 20;
//				bsize = 7;
//				CR += 10;
//				break;
//			case 3: // Erudite
//				bstr -= 15;
//				bsta -= 5;
//				bagi -= 5;
//				bdex -= 5;
//				bwis += 8;
//				bint += 32;
//				bcha -= 5;
//				MR += 5;
//				DR -= 5;
//				break;
//			case 4: // Wood Elf
//				bstr -= 10;
//				bsta -= 10;
//				bagi += 20;
//				bdex += 5;
//				bwis += 5;
//				bsize = 5;
//				break;
//			case 5: // High Elf
//				bstr -= 20;
//				bsta -= 10;
//				bagi += 10;
//				bdex -= 5;
//				bwis += 20;
//				bint += 12;
//				bcha += 5;
//				break;
//			case 6: // Dark Elf
//				bstr -= 15;
//				bsta -= 10;
//				bagi += 15;
//				bwis += 8;
//				bint += 24;
//				bcha -= 15;
//				bsize = 5;
//				break;
//			case 7: // Half Elf
//				bstr -= 5;
//				bsta -= 5;
//				bagi += 15;
//				bdex += 10;
//				bwis -= 15;
//				bsize = 5.5;
//				break;
//			case 8: // Dwarf
//				bstr += 15;
//				bsta += 15;
//				bagi -= 5;
//				bdex += 15;
//				bwis += 8;
//				bint -= 15;
//				bcha -= 30;
//				bsize = 4;
//				MR -= 5;
//				PR += 5;
//				break;
//			case 9: // Troll
//				bstr += 33;
//				bsta += 34;
//				bagi += 8;
//				bwis -= 15;
//				bint -= 23;
//				bcha -= 35;
//				bsize = 8;
//				FR -= 20;
//				break;
//			case 10: // Ogre
//				bstr += 55;
//				bsta += 77;
//				bagi -= 5;
//				bdex -= 5;
//				bwis -= 8;
//				bint -= 15;
//				bcha -= 38;
//				bsize = 9;
//				break;
//			case 11: // Halfling
//				bstr -= 5;
//				bagi += 20;
//				bdex += 15;
//				bwis += 5;
//				bint -= 8;
//				bcha -= 25;
//				bsize = 3.5;
//				PR += 5;
//				DR += 5;
//				break;
//			case 12: // Gnome
//				bstr -= 15;
//				bsta -= 5;
//				bagi += 10;
//				bdex += 10;
//				bwis -= 8;
//				bint += 23;
//				bcha -= 15;
//				bsize = 3;
//				break;
//			case 128: // Iksar
//				bstr -= 5;
//				bsta -= 5;
//				bagi += 15;
//				bdex += 10;
//				bwis += 5;
//				bcha -= 20;
//				MR -= 5;
//				FR -= 5;
//				break;
//			case 130: // Vah Shir
//				bstr += 15;
//				bagi += 15;
//				bdex -= 5;
//				bwis -= 5;
//				bint -= 10;
//				bcha -= 10;
//				bsize = 7;
//				MR -= 5;
//				FR -= 5;
//				break;
//			case 330: // Froglok
//				bstr -= 5;
//				bsta += 5;
//				bagi += 25;
//				bdex += 25;
//				bcha -= 25;
//				bsize = 5;
//				MR -= 5;
//				FR -= 5;
//				break;
//		}
//
//		// Randomize facial appearance
//		int iFace = 0;
//		if(iRace == 2) { // Barbarian w/Tatoo
//			iFace = MakeRandomInt(0, 79);
//		}
//		else {
//			iFace = MakeRandomInt(0, 7);
//		}
//		int iHair = 0;
//		int iBeard = 0;
//		int iBeardColor = 1;
//		if(gender) {
//			iHair = MakeRandomInt(0, 2);
//			if(iRace == 8) { // Dwarven Females can have a beard
//				if(MakeRandomInt(1, 100) < 50) {
//					iFace += 10;
//				}
//			}
//		}
//		else {
//			iHair = MakeRandomInt(0, 3);
//			iBeard = MakeRandomInt(0, 5);
//			iBeardColor = MakeRandomInt(0, 19);
//		}
//		int iHairColor = MakeRandomInt(0, 19);
//		int iEyeColor1 = MakeRandomInt(0, 9);
//		int iEyeColor2 = 0;
//		if(MakeRandomInt(1, 100) > 96) {
//			iEyeColor2 = MakeRandomInt(0, 9);
//		}
//		else {
//			iEyeColor2 = iEyeColor1;
//		}
//
//		// Base AC
//		int bac = (1 * 3) * 4;
//		switch(choosebclass) {
//			case WARRIOR:
//			case SHADOWKNIGHT:
//			case PALADIN:
//				bac = bac*1.5;
//		}
//
//		// Calc Base Hit Points
//		int16 multiplier = 1;
//		switch(choosebclass) {
//			case WARRIOR:
//				multiplier = 220;
//				break;
//			case DRUID:
//			case CLERIC:
//			case SHAMAN:
//				multiplier = 150;
//				break;
//			case BERSERKER:
//			case PALADIN:
//			case SHADOWKNIGHT:
//				multiplier = 210;
//				break;
//			case MONK:
//			case BARD:
//			case ROGUE:
//			case BEASTLORD:
//				multiplier = 180;
//				break;
//			case RANGER:
//				multiplier = 200;
//				break;
//			case MAGICIAN:
//			case WIZARD:
//			case NECROMANCER:
//			case ENCHANTER:
//				multiplier = 120;
//				break;
//		}
//		int16 lm = multiplier;
//		int16 Post255;
//		if((bsta-255)/2 > 0)
//			Post255 = (bsta-255)/2;
//		else
//			Post255 = 0;
//		int base_hp = (5)+(1*lm/10) + (((bsta-Post255)*1*lm/3000)) + ((Post255*1)*lm/6000);
//
//		// save bot to the database
//		char errbuf[MYSQL_ERRMSG_SIZE];
//		char *query = 0;
//		MYSQL_RES *result;
//		MYSQL_ROW row;
//		const char* lname = "";
//
//		if(database.RunQuery(query, MakeAnyLenString(&query, "SELECT count(*) FROM npc_types WHERE name like '%s'", sep->arg[2]), errbuf, &result)) {
//			row = mysql_fetch_row(result);
//			if(atoi(row[0]) != 0) {
//				c->Message(15, "%s already exists, try a different name.", sep->arg[2]);
//			}
//			else if(database.RunQuery(query, MakeAnyLenString(&query, "SELECT count(*) FROM character_ WHERE name like '%s'", sep->arg[2]), errbuf, &result)) {
//				row = mysql_fetch_row(result);
//				if(atoi(row[0]) != 0) {
//					c->Message(15, "%s already exists, try a different name.", sep->arg[2]);
//				}
//				else if(database.RunQuery(query, MakeAnyLenString(&query, "INSERT INTO npc_types (name,lastname,level,race,class,bodytype,hp,gender,size,hp_regen_rate,mana_regen_rate,npc_spells_id,npc_faction_id,face,luclin_hairstyle,luclin_haircolor,luclin_eyecolor,luclin_eyecolor2,luclin_beardcolor,luclin_beard,runspeed,MR,CR,DR,FR,PR,AC,STR,STA,DEX,AGI,_INT,WIS,CHA,isbot,ATK) VALUES ('%s','%s', %i, %i, %i, %i, %i, %i, %f, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %f, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i)", botName,lname,1,atoi(sep->arg[4]),atoi(sep->arg[3]),1,base_hp,gender,bsize,0,0,spellid,0,iFace,iHair,iHairColor,iEyeColor1,iEyeColor2,iBeardColor,iBeard,2.501f,MR,CR,DR,FR,PR,bac,bstr,bsta,bdex,bagi,bint,bwis,bcha,1,ATK), errbuf, 0)) {
//					if(database.RunQuery(query, MakeAnyLenString(&query, "SELECT MAX(id) from npc_types where name='%s' and isBot=1", sep->arg[2]), errbuf, &result)) {
//						if(row = mysql_fetch_row(result)) {
//							database.SetBotOwner(atoi(row[0]), c->AccountID());
//							c->Message(15, "Bot created: %s", row[0]);
//						}
//					}
//				}
//				else {
//					c->Message(15, "Error while creating your bot... %s", errbuf);
//				}
//			}
//		}
//		else {
//			c->Message(15, "Error while creating your bot... %s", errbuf);
//		}
//		safe_delete_array(query);
//		mysql_free_result(result);
//		return;
//	}

//
//	if(!strcasecmp(sep->arg[1], "help") && !strcasecmp(sep->arg[2], "create") ){
//		c->Message(15, "Classes:  1(Warrior), 2(Cleric), 3(Paladin), 4(Ranger), 5(Sk), 6(Druid), 7(Monk), 8(Bard), 9(Rogue), 10(Shaman), 11(Necro), 12(Wiz), 13(Mag), 14(Ench), 15(Beast), 16(Bersek)");
//		c->Message(15, "------------------------------------------------------------------");
//		c->Message(15, "Races: 1(Human), 2(Barb), 3(Erudit), 4(Wood elf), 5(High elf), 6(Dark elf), 7(Half elf), 8(Dwarf), 9(Troll), 10(Ogre), 11(Halfling), 12(Gnome), 330(Froglok), 128(Iksar), 130(Vah shir)");
//		c->Message(15, "------------------------------------------------------------------");
//		c->Message(15, "Usage: #bot create [name] [class(1-16)] [race(1-12,128,130,330)] [gender(male/female)]");
//		c->Message(15, "Example: #bot create Gandolf 12 1 male");
//		return;
//	}
//
//	if(!strcasecmp(sep->arg[1], "delete") ) {
//		if((c->GetTarget() == NULL) || !c->GetTarget()->IsBot())
//		{
//			c->Message(15, "You must target a bot!");
//			return;
//		}
//		else if(database.GetBotOwner(c->GetTarget()->GetNPCTypeID()) != c->AccountID())
//		{
//			c->Message(15, "You can't delete a bot that you don't own.");
//			return;
//		}
//
//		if(database.DeleteBot(c->GetTarget()->GetNPCTypeID())) {
//			c->GetTarget()->Say("...but why?!! We had such good adventures together! gaahhh...glrrrk...");
//			c->GetTarget()->BotOwner = NULL;
//			c->GetTarget()->Kill();
//		}
//		else {
//			c->Message(15, "Error deleting Bot!");
//		}
//		return;
//	}
//
//	if(!strcasecmp(sep->arg[1], "list")) {
//
//		bool listAll = true;
//		int iClass = atoi(sep->arg[2]);
//		switch(iClass) {
//			case 1:
//			case 2:
//			case 3:
//			case 4:
//			case 5:
//			case 6:
//			case 7:
//			case 8:
//			case 9:
//			case 10:
//			case 11:
//			case 12:
//			case 13:
//			case 14:
//			case 15:
//			case 16:
//				listAll = false;
//				break;
//			default:
//				break;
//		}
//
//		char errbuf[MYSQL_ERRMSG_SIZE];
//		char *query = 0;
//		int32 affected_rows = 0;
//		MYSQL_RES *result;
//		MYSQL_ROW row;
//
//		if(database.RunQuery(query, MakeAnyLenString(&query, "SELECT id, name, class, race from npc_types where isbot=1"), errbuf, &result, &affected_rows))
//		{
//
//			while(row = mysql_fetch_row(result))
//			{
//				// change the class ID by the name
//				int irow = atoi(row[2]);
//				const char *crow;
//
//				switch(irow) {
//		case 1:
//			crow = "Warrior";
//			break;
//		case 2:
//			crow = "Cleric";
//			break;
//		case 3:
//			crow = "Paladin";
//			break;
//		case 4:
//			crow = "Ranger";
//			break;
//		case 5:
//			crow = "Shadows Knight";
//			break;
//		case 6:
//			crow = "Druid";
//			break;
//		case 7:
//			crow = "Monk";
//			break;
//		case 8:
//			crow = "Bard";
//			break;
//		case 9:
//			crow = "Rogue";
//			break;
//		case 10:
//			crow = "Shaman";
//			break;
//		case 11:
//			crow = "Necromancer";
//			break;
//		case 12:
//			crow = "Wizard";
//			break;
//		case 13:
//			crow = "Magician";
//			break;
//		case 14:
//			crow = "Enchanter";
//			break;
//		case 15:
//			crow = "Beastlord";
//			break;
//		case 16:
//			crow = "Berserker";
//			break;
//		default:
//			crow = "Warrior";
//				}
//
//				// change the race ID by the name
//				int rrow = atoi(row[3]);
//				const char *rrrow;
//
//				switch(rrow) {
//		case 1:
//			rrrow = "Human";
//			break;
//		case 2:
//			rrrow = "Barbarian";
//			break;
//		case 3:
//			rrrow = "Erudite";
//			break;
//		case 4:
//			rrrow = "Wood Elf";
//			break;
//		case 5:
//			rrrow = "High Elf";
//			break;
//		case 6:
//			rrrow = "Dark Elf";
//			break;
//		case 7:
//			rrrow = "Half Elf";
//			break;
//		case 8:
//			rrrow = "Dwarf";
//			break;
//		case 9:
//			rrrow = "Troll";
//			break;
//		case 10:
//			rrrow = "Ogre";
//			break;
//		case 11:
//			rrrow = "Halfling";
//			break;
//		case 12:
//			rrrow = "Gnome";
//			break;
//		case 330:
//			rrrow = "Froglok";
//			break;
//		case 128:
//			rrrow = "Iksar";
//			break;
//		case 130:
//			rrrow = "Vah Shir";
//			break;
//		default:
//			rrrow = "Human";
//				}
//
//				if(listAll && database.GetBotOwner(atoi(row[0])) == c->AccountID()) {
//					c->Message(15,"ID: %s -- Class: %s -- Name: %s -- Race: %s -- ", row[0], crow, row[1], rrrow);
//				}
//				else if((database.GetBotOwner(atoi(row[0])) == c->AccountID()) && (irow == iClass)) {
//					c->Message(15,"ID: %s -- Class: %s -- Name: %s -- Race: %s -- ", row[0], crow, row[1], rrrow);
//				}
//			}				
//		}
//		mysql_free_result(result);
//		safe_delete_array(query);
//		return;
//	}
//
//	if(!strcasecmp(sep->arg[1], "spawn") ){
//		if(database.GetBotOwner(atoi(sep->arg[2])) != c->AccountID())
//		{
//			c->Message(15,"You can't spawn a bot that you don't own.");
//			return;
//		}
//
//		if(c->GetFeigned())
//		{
//			c->Message(15, "You can't summon bots while you are feigned.");
//			return;
//		}
//
//		if(c->IsBotRaiding()) {
//			BotRaids *br = entity_list.GetBotRaidByMob(c->CastToMob());
//			if(br) {
//				if(br->GetBotRaidAggro()) {
//					c->Message(15, "You can't summon bots while you are engaged.");
//					return;
//				}
//			}
//		}
//
//		if(c->IsGrouped())
//		{
//			Group *g = entity_list.GetGroupByClient(c);
//			for (int i=0; i<MAX_GROUP_MEMBERS; i++)
//			{
//				if(g && g->members[i] && !g->members[i]->qglobal && (g->members[i]->GetAppearance() != eaDead) && g->members[i]->IsEngaged())
//				{
//					c->Message(15, "You can't summon bots while you are engaged.");
//					return;
//				}
//				if(g && g->members[i] && g->members[i]->qglobal) {
//					return;
//				}
//			}
//		}
//
//		const NPCType* tmp = 0;
//		if ((tmp = database.GetNPCType(atoi(sep->arg[2]))))
//		{
//			Mob *mtmp = entity_list.GetMobByNpcTypeID(atoi(sep->arg[2]));
//			if(mtmp && entity_list.IsMobInZone(mtmp))
//			{
//				c->Message(15, "This bot is already in the zone.");
//				tmp = 0;
//				mtmp = 0;
//				return;
//			}
//
//			const int spawnedBots = database.SpawnedBotCount(c->CharacterID());
//			if(spawnedBots && database.IsBotSpawned(c->CharacterID(), atoi(sep->arg[2]))) {
//				c->Message(15, "That bot is already spawned.");
//				MYSQL_RES* total = database.ListSpawnedBots(c->CharacterID());
//				MYSQL_ROW row;
//				if(mysql_num_rows(total) == spawnedBots) {
//					for(int i=0; i<spawnedBots; i++) {
//						row = mysql_fetch_row(total);
//						char* longname;
//						if(database.GetZoneLongName((char*)row[1], &longname, NULL, NULL, NULL, NULL, NULL, NULL)) {
//							c->Message(15, "%s is in %s", row[0], longname);
//							safe_delete(longname);
//						}
//					}
//				}
//				mysql_free_result(total);
//				tmp = 0;
//				mtmp = 0;
//				return;
//			}
//
//			if(RuleB(EQOffline, BotQuest)) {
//				const int allowedBots = database.AllowedBotSpawns(c->CharacterID());
//				if(allowedBots == 0) {
//					c->Message(15, "You cannot spawn any bots.");
//					tmp = 0;
//					mtmp = 0;
//					return;
//				}
//				if(spawnedBots >= allowedBots) {
//					c->Message(15, "You cannot spawn more than %i bot(s).", spawnedBots);
//					MYSQL_RES* total = database.ListSpawnedBots(c->CharacterID());
//					MYSQL_ROW row;
//					if(mysql_num_rows(total) == spawnedBots) {
//						for(int i=0; i<spawnedBots; i++) {
//							row = mysql_fetch_row(total);
//							char* longname;
//							if(database.GetZoneLongName((char*)row[1], &longname, NULL, NULL, NULL, NULL, NULL, NULL)) {
//								c->Message(15, "%s is in %s", row[0], longname);
//								safe_delete(longname);
//							}
//						}
//					}
//					mysql_free_result(total);
//					tmp = 0;
//					mtmp = 0;
//					return;
//				}
//			}
//
//			if(spawnedBots >= RuleI(EQOffline, SpawnBotCount)) {
//				c->Message(15, "You cannot spawn more than %i bots.", spawnedBots);
//				tmp = 0;
//				mtmp = 0;
//				return;
//			}
//
//			NPC* npc = new NPC(tmp, 0, c->GetX(), c->GetY(), c->GetZ(), c->GetHeading());
//
//			// As the mob is in the DB, we need to calc its level, HP, Mana.
//			// First, the mob must have the same level as his leader
//			npc->SetLevel(c->GetLevel());
//			entity_list.AddNPC(npc);
//			database.SetBotLeader(npc->GetNPCTypeID(), c->CharacterID(), npc->GetName(), zone->GetShortName());
//			npc->CastToMob()->Say("I am ready for battle.");
//		}
//		else {
//			c->Message(15, "BotID: %i not found", atoi(sep->arg[2]));
//			tmp = 0;
//		}
//		return;
//	}
//
//	if(!strcasecmp(sep->arg[1], "archery")) {
//		if((c->GetTarget() == NULL) || (c->GetTarget() == c) || !c->GetTarget()->IsBot()) {
//			c->Message(15, "You must target a bot!");
//			return;
//		}
//		Mob *archerbot = c->GetTarget();
//		if((archerbot->GetClass()==WARRIOR)||(archerbot->GetClass()==PALADIN)||(archerbot->GetClass()==RANGER)||(archerbot->GetClass()==SHADOWKNIGHT)||(archerbot->GetClass()==ROGUE)) {
//			const Item_Struct* botweapon = database.GetItem(archerbot->CastToNPC()->GetItemID(SLOT_RANGE));
//			uint32 archeryMaterial;
//			uint32 archeryColor;
//			uint32 archeryBowID;
//			uint32 archeryAmmoID;
//			uint32 range = 0;
//			if(botweapon && (botweapon->ItemType == ItemTypeBow)) {
//				archeryMaterial = atoi(botweapon->IDFile+2);
//				archeryBowID = botweapon->ID;
//				archeryColor = botweapon->Color;
//				range =+ botweapon->Range;
//				botweapon = database.GetItem(archerbot->CastToNPC()->GetItemID(SLOT_AMMO));
//				if(!botweapon || (botweapon->ItemType != ItemTypeArrow)) {
//					archerbot->Say("I don't have any arrows.");
//					archerbot->SetBotArcheryRange(0);
//					return;
//				}
//				range += botweapon->Range;
//				archeryAmmoID = botweapon->ID;
//			}
//			else {
//				archerbot->Say("I don't have a bow.");
//				archerbot->SetBotArcheryRange(0);
//				return;
//			}
//			if(archerbot->IsBotArcher()) {
//				archerbot->SetBotArcher(false);
//				archerbot->Say("Using melee skills.");
//				archerbot->CastToNPC()->BotAddEquipItem(MATERIAL_PRIMARY, database.GetBotItemBySlot(archerbot->GetNPCTypeID(), SLOT_PRIMARY));
//				archerbot->CastToNPC()->SendWearChange(MATERIAL_PRIMARY);
//				archerbot->CastToNPC()->BotAddEquipItem(MATERIAL_SECONDARY, database.GetBotItemBySlot(archerbot->GetNPCTypeID(), SLOT_SECONDARY));
//				archerbot->CastToNPC()->SendWearChange(MATERIAL_SECONDARY);
//				archerbot->SetBotArcheryRange(0);
//			}
//			else {
//				archerbot->SetBotArcher(true);
//				archerbot->Say("Using archery skills.");
//				archerbot->CastToNPC()->BotRemoveEquipItem(MATERIAL_PRIMARY);
//				archerbot->SendWearChange(MATERIAL_PRIMARY);
//				archerbot->CastToNPC()->BotRemoveEquipItem(MATERIAL_SECONDARY);
//				archerbot->SendWearChange(MATERIAL_SECONDARY);
//				archerbot->CastToNPC()->BotAddEquipItem(MATERIAL_SECONDARY, archeryBowID);
//				archerbot->CastToNPC()->SendBotArcheryWearChange(MATERIAL_SECONDARY, archeryMaterial, archeryColor);
//				archerbot->CastToNPC()->BotAddEquipItem(MATERIAL_PRIMARY, archeryAmmoID);
//				archerbot->SetBotArcheryRange(range);
//			}
//		}
//		else {
//			archerbot->Say("I don't know how to use a bow.");
//		}
//		return;
//	}
//
//	if(!strcasecmp(sep->arg[1], "picklock")) {
//		if((c->GetTarget() == NULL) || (c->GetTarget() == c) || !c->GetTarget()->IsBot() || (c->GetTarget()->GetClass() != ROGUE)) {
//			c->Message(15, "You must target a rogue bot!");
//			return;
//		}
//		entity_list.OpenDoorsNear(c->GetTarget()->CastToNPC());
//		return;
//	}
//
//	if(!strcasecmp(sep->arg[1], "summon")) {
//		if((c->GetTarget() == NULL) || (c->GetTarget() == c) || !c->GetTarget()->IsBot() || c->GetTarget()->IsPet())
//		{
//			c->Message(15, "You must target a bot!");
//			return;
//		}
//		if(c->GetTarget()->IsMob() && !c->GetTarget()->IsPet())
//		{
//			Mob *b = c->GetTarget();
//
//			// Is our target "botable" ?
//			if(b && !b->IsBot()){
//				c->Message(15, "You must target a bot!");
//				return;
//			}
//			if(b && (database.GetBotOwner(b->GetNPCTypeID()) != c->AccountID()))
//			{
//				b->Say("You can only summon your own bots.");
//				return;
//			}
//			if(b) {
//				b->SetTarget(b->BotOwner);
//				b->Warp(c->GetX(), c->GetY(), c->GetZ());
//			}
//		}
//		return;
//	}
//
//	if(!strcasecmp(sep->arg[1], "groupraid")) {
//		if(c->GetFeigned())
//		{
//			c->Message(15, "You can't load your raid while you are feigned.");
//			return;
//		}
//
//		if(c->IsBotRaiding() || c->IsGrouped()) {
//			c->Message(15, "You cannot be in a group.");
//			return;
//		}
//
//		MYSQL_RES* groups = database.LoadBotGroups(c->CharacterID());
//		MYSQL_ROW row = 0;
//		int bots = mysql_num_rows(groups);
//		int group = 0;
//		int16 id = c->GetID();
//		Mob* mob = c->CastToMob();
//		mob->SetOwnerID(0);
//		Group *g = new Group(mob);
//		entity_list.AddGroup(g);
//		bool followid = false;
//		BotRaids *br = 0;
//		MYSQL_RES* itemIDs = 0;
//		MYSQL_ROW rows = 0;
//		const Item_Struct* item2 = NULL;
//		int numitems = 0;
//		uint32 itemID = 0;
//		Mob *mtmp = 0;
//		for(int i=0; i<bots; ++i) {
//			row = mysql_fetch_row(groups);
//			if((g->BotGroupCount() < 6) && (atoi(row[0]) == group)) {
//				mtmp = entity_list.GetMobByNpcTypeID(atoi(row[1]));
//				if(mtmp) {
//					g->AddMember(mtmp);
//					if(!followid) {
//						mtmp->SetFollowID(id);
//						id = mtmp->GetID();
//						followid = true;
//					}
//					else {
//						mtmp->SetFollowID(id);
//					}
//					mtmp->BotOwner = mob;
//					mtmp->SetOwnerID(0);
//					if(br) {
//						mtmp->SetBotRaiding(true);
//						mtmp->SetBotRaidID(br->GetBotRaidID());
//					}
//					itemIDs = database.GetBotItems(mtmp->GetNPCTypeID());
//					if(itemIDs) {
//						numitems = mysql_num_rows(itemIDs);
//						for(int j=0; j<numitems; ++j) {
//							rows = mysql_fetch_row(itemIDs);
//							itemID = atoi(rows[1]);
//							if(itemID != 0) {
//								item2 = database.GetItem(itemID);
//								c->BotTradeAddItem(itemID, item2->MaxCharges, item2->Slots, atoi(rows[0]), mtmp->CastToNPC(), false);
//							}
//						}
//						mtmp->CalcBotStats(false);
//					}
//					mysql_free_result(itemIDs);
//				}
//			}
//			else {
//				group++;
//				mtmp = entity_list.GetMobByNpcTypeID(atoi(row[1]));
//				if(mtmp) {
//					mtmp->SetFollowID(id);
//					id = mtmp->GetID();
//					mtmp->BotOwner = mob;
//					mtmp->SetOwnerID(0);
//					if(!br) {
//						br = new BotRaids(mob);
//					}
//					for(int k=0; k<MAX_GROUP_MEMBERS; ++k) {
//						if(g->members[k]) {
//							g->members[k]->SetBotRaiding(true);
//							g->members[k]->SetBotRaidID(br->GetBotRaidID());
//						}
//					}
//					br->AddBotGroup(g);
//					g = new Group(mtmp);
//					entity_list.AddGroup(g);
//					if(g->members[0] && !g->members[0]->IsBotRaiding())
//					{
//						for(int n=0; n<MAX_GROUP_MEMBERS; ++n) {
//							if(g->members[n]) {
//								g->members[n]->SetBotRaiding(true);
//								g->members[n]->SetBotRaidID(br->GetBotRaidID());
//							}
//						}
//					}
//					br->AddBotGroup(g);
//					itemIDs = database.GetBotItems(mtmp->GetNPCTypeID());
//					if(itemIDs) {
//						numitems = mysql_num_rows(itemIDs);						
//						for(int j=0; j<numitems; ++j) {
//							rows = mysql_fetch_row(itemIDs);
//							itemID = atoi(rows[1]);
//							if(itemID != 0) {
//								item2 = database.GetItem(itemID);
//								c->BotTradeAddItem(itemID, item2->MaxCharges, item2->Slots, atoi(rows[0]), mtmp->CastToNPC(), false);
//							}
//						}
//						mtmp->CalcBotStats(false);
//					}
//					mysql_free_result(itemIDs);
//				}
//			}
//		}
//		mysql_free_result(groups);
//		return;
//	}
//
//	if(!strcasecmp(sep->arg[1], "spawnraid")) {
//		if(c->GetFeigned())
//		{
//			c->Message(15, "You can't load your raid while you are feigned.");
//			return;
//		}
//
//		const int spawnedBots = database.SpawnedBotCount(c->CharacterID());
//		if(c->IsBotRaiding() || c->IsGrouped() || spawnedBots) {
//			c->Message(15, "You already have spawned bots.");
//			MYSQL_RES* total = database.ListSpawnedBots(c->CharacterID());
//			MYSQL_ROW row;
//			if(mysql_num_rows(total) == spawnedBots) {
//				for(int i=0; i<spawnedBots; i++) {
//					row = mysql_fetch_row(total);
//					char* longname;
//					if(database.GetZoneLongName((char*)row[1], &longname, NULL, NULL, NULL, NULL, NULL, NULL)) {
//						c->Message(15, "%s is in %s", row[0], longname);
//						safe_delete(longname);
//					}
//				}
//			}
//			mysql_free_result(total);
//			return;
//		}
//
//		MYSQL_RES* groups = database.LoadBotGroups(c->CharacterID());
//		MYSQL_ROW row;
//		int bots = mysql_num_rows(groups);
//		const NPCType* tmp = 0;
//		NPC* npc = 0;
//		float myX = c->GetX();
//		float myY = c->GetY();
//		float myZ = c->GetZ();
//		float myHeading = c->GetHeading();
//		uint8 myLevel = c->GetLevel();
//		for(int i=0; i<bots; i++) {
//			row = mysql_fetch_row(groups);
//			if(tmp = database.GetNPCType(atoi(row[1]))) {
//				npc = new NPC(tmp, 0, myX, myY, myZ, myHeading);
//				tmp = 0;
//				npc->SetLevel(myLevel);
//				entity_list.AddNPC(npc);
//				database.SetBotLeader(npc->GetNPCTypeID(), c->CharacterID(), npc->GetName(), zone->GetShortName());
//			}
//		}
//		mysql_free_result(groups);
//		return;
//	}
//
//	if(!strcasecmp(sep->arg[1], "saveraid")) {
//		if(c->GetFeigned())
//		{
//			c->Message(15, "You can't save your raid while you are feigned.");
//			return;
//		}
//
//		if(c->IsBotRaiding()) {
//			BotRaids *br = entity_list.GetBotRaidByMob(c->CastToMob());
//			if(br) {
//				if(br->GetBotRaidAggro()) {
//					c->Message(15, "You can't save your raid while you are engaged.");
//					return;
//				}
//			}
//		}
//
//		if(c->IsGrouped())
//		{
//			Group *g = entity_list.GetGroupByClient(c);
//			for (int i=0; i<MAX_GROUP_MEMBERS; i++)
//			{
//				if(g && g->members[i] && !g->members[i]->qglobal && (g->members[i]->GetAppearance() != eaDead) && g->members[i]->IsEngaged())
//				{
//					c->Message(15, "You can't save your raid while you are engaged.");
//					return;
//				}
//				if(g && g->members[i] && g->members[i]->qglobal) {
//					return;
//				}
//			}
//		}
//		BotRaids *br = entity_list.GetBotRaidByMob(c->CastToMob());
//		if(br) {
//			br->SaveGroups(c);
//			c->Message(15, "Your raid is saved.");
//		}
//		else {
//			Group *g = entity_list.GetGroupByClient(c);
//			if(g) {
//				database.DeleteBotGroups(c->CharacterID());
//				for(int j=0; j<MAX_GROUP_MEMBERS; j++) {
//					if(g->members[j]) {
//						if(g->members[j]->IsClient()) {
//							continue;
//						}
//						database.SaveBotGroups(0, c->CharacterID(), g->members[j]->GetNPCTypeID(), j);
//					}
//				}
//				c->Message(15, "Your raid is saved.");
//			}
//			else {
//				c->Message(15, "You need to have a raid to save a raid.");
//			}
//		}
//		return;
//	}
//
//	if(!strcasecmp(sep->arg[1], "group") && !strcasecmp(sep->arg[2], "add"))
//	{
//		if(c->GetFeigned()) {
//			c->Message(15, "You can't create bot groups while feigned!");
//			return;
//		}
//
//		if(c->IsBotRaiding()) {
//			BotRaids *br = entity_list.GetBotRaidByMob(c->CastToMob());
//			if(br) {
//				if(br->GetBotRaidAggro()) {
//					c->Message(15, "You can't create bot groups while you are engaged.");
//					return;
//				}
//			}
//		}
//
//		if(c->IsGrouped())
//		{
//			Group *g = entity_list.GetGroupByClient(c);
//			for (int i=0; i<MAX_GROUP_MEMBERS; i++)
//			{
//				if(g && g->members[i] && g->members[i]->IsEngaged())
//				{
//					c->Message(15, "You can't create bot groups while you are engaged.");
//					return;
//				}
//			}
//		}
//
//		if((c->GetTarget() == NULL) || !c->GetTarget()->IsBot())
//		{
//			c->Message(15, "You must target a bot!");
//			return;
//		}
//
//		if(c->GetTarget()->IsClient())
//		{
//			c->Message(15, "You can't invite clients this way.");
//			return;
//		}
//
//		if ( c->IsGrouped() )
//		{
//			Group *g = entity_list.GetGroupByClient(c);
//			if(g && (g->BotGroupCount() > 5))
//			{
//				c->Message(15, "There is no more room in your group.");
//				Mob* kmob = c->GetTarget();
//				if(kmob != NULL) {
//					kmob->BotOwner = NULL;
//					kmob->Kill();
//				}
//				return;
//			}
//		}
//
//		if(c->IsGrouped()) {
//			Group *g = entity_list.GetGroupByClient(c);
//			if(g && (c->CastToMob() != g->members[0])) {
//				c->Message(15, "Only the group leader can invite bots.");
//				Mob* kmob = c->GetTarget();
//				if(kmob != NULL) {
//					kmob->BotOwner = NULL;
//					kmob->Kill();
//				}
//				return;
//			}
//		}
//
//		if(c->GetTarget()->IsMob() && !c->GetTarget()->IsPet())
//		{
//			Mob *b = c->GetTarget();
//
//			// Is our target "botable" ?
//			if(b && !b->IsBot()){
//				b->Say("I can't be a bot!");
//				return;
//			}
//
//			if(database.GetBotOwner(b->GetNPCTypeID()) != c->AccountID())
//			{
//				b->Say("I can't be your bot, you are not my owner.");
//				return;
//			}
//
//			// Is he already grouped ?
//			if(b->IsGrouped())
//			{
//				b->Say("I'm already grouped!");
//				return;
//			}
//
//			// else, we do:
//			//1: Set its leader
//			b->Say("I'm becoming %s\'s bot!", c->GetName());
//
//			//2: Set the follow ID so he's following its leader
//			b->SetFollowID(c->GetID());
//			b->BotOwner = c->CastToMob();
//			b->SetOwnerID(0);
//			c->CastToMob()->SetOwnerID(0);
//
//			//3:  invite it to the group
//			if(!c->IsGrouped()) {
//				Group *g = new Group(c->CastToMob());
//				g->AddMember(b);
//				entity_list.AddGroup(g);
//			}
//			else {
//				c->GetGroup()->AddMember(b);
//			}
//
//			if(c->IsBotRaiding()) {
//				b->SetBotRaiding(true);
//				b->SetBotRaidID(c->CastToMob()->GetBotRaidID());
//			}
//
//			uint32 itemID = 0;
//			const Item_Struct* item2 = NULL;
//			for(int i=0; i<22; i++) {
//				itemID = database.GetBotItemBySlot(b->GetNPCTypeID(), i);
//				if(itemID != 0) {
//					item2 = database.GetItem(itemID);
//					c->BotTradeAddItem(itemID, item2->MaxCharges, item2->Slots, i, b->CastToNPC(), false);
//				}
//			}
//			b->CalcBotStats();
//		}
//		return;
//	}
//
//	if(!strcasecmp(sep->arg[1], "group") && !strcasecmp(sep->arg[2], "remove")) {
//		if(c->GetTarget() != NULL) {
//			if(c->GetTarget()->IsBot() && (c->GetTarget()->GetBotLeader() == c->CharacterID())) {
//				if(c->GetTarget()->IsGrouped()) {
//					Group *g = entity_list.GetGroupByMob(c->GetTarget());
//					if(g && g->members[0]) {
//						if(g->members[0] == c->GetTarget()) {
//							for(int i=5; i>=0; i--) {
//								if(g->members[i]) {
//									g->members[i]->BotOwner = NULL;
//									g->members[i]->Kill();
//								}
//							}
//						}
//						else {
//							c->GetTarget()->BotOwner = NULL;
//							c->GetTarget()->Kill();
//						}
//						if(g->BotGroupCount() < 2) {
//							g->DisbandGroup();
//						}
//					}
//				}
//				else {
//					c->GetTarget()->BotOwner = NULL;
//					c->GetTarget()->Kill();
//				}
//				if(c->IsBotRaiding()) {
//					if(database.SpawnedBotCount(c->CharacterID()) < 6) {
//						entity_list.RemoveBotRaid(c->CastToMob()->GetBotRaidID());
//						Group *g = c->GetGroup();
//						if(g) {
//							for(int i=0; i<MAX_GROUP_MEMBERS; i++) {
//								if(g->members[i]) {
//									g->members[i]->SetBotRaidID(0);
//									g->members[i]->SetBotRaiding(false);
//								}
//							}
//						}
//					}
//				}
//			}
//			else {
//				c->Message(15, "You must target a bot first.");
//			}
//		}
//		else {
//			c->Message(15, "You must target a bot first.");
//		}
//		return;
//	}
//
//	if(!strcasecmp(sep->arg[1], "group") && !strcasecmp(sep->arg[2], "order"))
//	{
//		if(!strcasecmp(sep->arg[3], "follow"))
//		{
//			if(c->IsBotRaiding()) {
//				BotRaids *br = entity_list.GetBotRaidByMob(c->CastToMob());
//				br->FollowGuardCmd(c, false);
//			}
//			else if(c->IsGrouped())
//			{
//				Group *g = c->GetGroup();
//				if(g) {
//					int32 botfollowid = 0;
//					const char* botfollowname;
//					for(int i=0; i<MAX_GROUP_MEMBERS; i++) {
//						if(g->members[i] && g->members[i]->IsBot()) {
//							if(botfollowid == 0) {
//								botfollowid = g->members[i]->GetID();
//								botfollowname = g->members[i]->GetName();
//								g->members[i]->SetFollowID(c->GetID());
//								g->members[i]->Say("Following %s.", c->GetName());
//							}
//							else {
//								g->members[i]->SetFollowID(botfollowid);
//								g->members[i]->Say("Following %s.", botfollowname);
//							}
//							g->members[i]->WipeHateList();
//						}
//					}
//				}
//			}
//		}
//		else if(!strcasecmp(sep->arg[3], "guard"))
//		{
//			if(c->IsBotRaiding()) {
//				BotRaids *br = entity_list.GetBotRaidByMob(c->CastToMob());
//				br->FollowGuardCmd(c, true);
//			}
//			else if(c->IsGrouped())
//			{
//				Group *g = c->GetGroup();
//				if(g) {
//					for(int i=0; i<MAX_GROUP_MEMBERS; i++)
//					{
//						if(g->members[i] && g->members[i]->IsBot()) {
//							g->members[i]->SetFollowID(0);
//							g->members[i]->WipeHateList();
//							g->members[i]->Say("Guarding here.");
//						}
//					}
//				}
//			}
//		}
//		else if(!strcasecmp(sep->arg[3], "attack"))
//		{
//			if(c->IsGrouped() && (c->GetTarget() != NULL) && c->IsAttackAllowed(c->GetTarget())) {
//				c->SetOrderBotAttack(true);
//				if(c->IsBotRaiding()) {
//					BotRaids *br = entity_list.GetBotRaidByMob(c->CastToMob());
//					if(br) {
//						c->SetOrderBotAttack(true);
//						br->AddBotRaidAggro(c->GetTarget());
//						c->SetOrderBotAttack(false);
//					}
//				}
//				else {
//					Group *g = entity_list.GetGroupByMob(c->CastToMob());
//					if(g) {
//						for(int i=0; i<MAX_GROUP_MEMBERS; i++) {
//							if(g->members[i] && g->members[i]->IsBot()) {
//								c->SetOrderBotAttack(true);
//								g->members[i]->AddToHateList(c->GetTarget(), 1);
//								c->SetOrderBotAttack(false);
//							}
//						}
//					}
//				}
//				c->SetOrderBotAttack(false);
//			}
//			else {
//				c->Message(15, "You must target a monster.");
//			}
//		}
//		return;
//	}
//
//	if(!strcasecmp(sep->arg[1], "inventory") && !strcasecmp(sep->arg[2], "list"))
//	{
//		if(c->GetTarget() != NULL)
//		{
//			if(c->GetTarget()->IsBot() && (c->GetTarget()->BotOwner == c->CastToMob()))
//			{
//				Mob* b = c->GetTarget();	
//				int x = database.GetBotItemsNumber(b->GetNPCTypeID() );
//				const char* equipped[22] = {"Charm", "Left Ear", "Head", "Face", "Right Ear", "Neck", "Shoulders", "Arms", "Back",
//					"Left Wrist", "Right Wrist", "Range", "Hands", "Primary Hand", "Secondary Hand",
//					"Left Finger", "Right Finger", "Chest", "Legs", "Feet", "Waist", "Ammo" };
//				const Item_Struct* item2 = NULL;
//				bool is2Hweapon = false;
//				for(int i=0; i<22 ; i++)
//				{
//					if((i == 14) && is2Hweapon) {
//						continue;
//					}
//					item2 = database.GetItem(database.GetBotItemBySlot(b->GetNPCTypeID(), i));
//					if(item2 == 0) {
//						c->Message(15, "I need something for my %s (Item %i)", equipped[i], i);
//						continue;
//					}
//					if((i == 13) && ((item2->ItemType == ItemType2HS) || (item2->ItemType == ItemType2HB) || (item2->ItemType == ItemType2HPierce))) {
//						is2Hweapon = true;
//					}
//					if((i == 0) || (i == 11) || (i == 13) || (i == 14) || (i == 21)) {
//						if (c->GetClientVersion() == EQClientSoF)
//						{
//							c->Message(15, "Using %c%06X00000000000000000000000000000000000000000000%s%c in my %s (Item %i)", 0x12, item2->ID, item2->Name, 0x12, equipped[i], i);
//						}
//						else
//						{
//							c->Message(15, "Using %c%06X000000000000000000000000000000000000000%s%c in my %s (Item %i)", 0x12, item2->ID, item2->Name, 0x12, equipped[i], i);
//						}
//					}
//					else {
//						if (c->GetClientVersion() == EQClientSoF)
//						{
//							c->Message(15, "Using %c%06X00000000000000000000000000000000000000000000%s%c on my %s (Item %i)", 0x12, item2->ID, item2->Name, 0x12, equipped[i], i);
//						}
//						else
//						{
//							c->Message(15, "Using %c%06X000000000000000000000000000000000000000%s%c on my %s (Item %i)", 0x12, item2->ID, item2->Name, 0x12, equipped[i], i);
//						}
//					}
//				}
//			}
//			else {
//				c->Message(15, "You must group your bot first.");
//			}
//		}
//		else {
//			c->Message(15, "You must target a bot first.");
//		}
//		return;
//	}
//
//	if(!strcasecmp(sep->arg[1], "inventory") && !strcasecmp(sep->arg[2], "remove")) {
//		if((c->GetTarget() == NULL) || (sep->arg[3] == '\0') || !c->GetTarget()->IsBot())
//		{
//			c->Message(15, "Usage: #bot inventory remove [slotid] (You must have a bot targetted) ");
//			return;
//		}		
//		else if(c->GetTarget()->IsBot() && (c->GetTarget()->BotOwner == c->CastToMob()))
//		{
//			int slotId = atoi(sep->arg[3]);
//			if(slotId > 21 || slotId < 0) {
//				c->Message(15, "A bot has 21 slots in its inventory, please choose a slot between 0 and 21.");
//				return;
//			}
//			const char* equipped[22] = {"Charm", "Left Ear", "Head", "Face", "Right Ear", "Neck", "Shoulders", "Arms", "Back",
//				"Left Wrist", "Right Wrist", "Range", "Hands", "Primary Hand", "Secondary Hand",
//				"Left Finger", "Right Finger", "Chest", "Legs", "Feet", "Waist", "Ammo" };
//			const Item_Struct *itm = database.GetItem(database.GetBotItemBySlot(c->GetTarget()->GetNPCTypeID(), slotId));
//			// Don't allow the player to remove a lore item they already possess and cause a crash
//			if(!c->CheckLoreConflict(itm)) {
//				if(itm) {
//					const ItemInst* itminst = new ItemInst(itm, itm->MaxCharges);
//					c->PushItemOnCursor(*itminst, true);
//					safe_delete(itminst);
//					Mob *gearbot = c->GetTarget();
//					if((slotId == SLOT_RANGE)||(slotId == SLOT_AMMO)||(slotId == SLOT_PRIMARY)||(slotId == SLOT_SECONDARY)) {
//						gearbot->SetBotArcher(false);
//					}
//					database.RemoveBotItemBySlot(gearbot->GetNPCTypeID(), slotId);
//					gearbot->CastToNPC()->RemoveItem(itm->ID);
//					int8 materialFromSlot = Inventory::CalcMaterialFromSlot(slotId);
//					if(materialFromSlot != 0xFF) {
//						gearbot->CastToNPC()->BotRemoveEquipItem(materialFromSlot);
//						gearbot->CastToNPC()->SendWearChange(materialFromSlot);
//					}
//					gearbot->CalcBotStats();
//					switch(slotId) {
//						case 0:
//						case 1:
//						case 2:
//						case 3:
//						case 4:
//						case 5:
//						case 8:
//						case 9:
//						case 10:
//						case 11:
//						case 13:
//						case 14:
//						case 15:
//						case 16:
//						case 17:
//						case 20:
//						case 21:
//							gearbot->Say("My %s is now unequipped.", equipped[slotId]);
//							break;
//						case 6:
//						case 7:
//						case 12:
//						case 18:
//						case 19:
//							gearbot->Say("My %s are now unequipped.", equipped[slotId]);
//							break;
//						default:
//							break;
//					}
//				}
//				else {
//					switch(slotId) {
//						case 0:
//						case 1:
//						case 2:
//						case 3:
//						case 4:
//						case 5:
//						case 8:
//						case 9:
//						case 10:
//						case 11:
//						case 13:
//						case 14:
//						case 15:
//						case 16:
//						case 17:
//						case 20:
//						case 21:
//							c->GetTarget()->Say("My %s is already unequipped.", equipped[slotId]);
//							break;
//						case 6:
//						case 7:
//						case 12:
//						case 18:
//						case 19:
//							c->GetTarget()->Say("My %s are already unequipped.", equipped[slotId]);
//							break;
//						default:
//							break;
//					}
//				}
//			}
//			else {
//				c->Message(15, "Duplicate Lore item.");
//			}
//		}
//		return;
//	}
//
//	if(!strcasecmp(sep->arg[1], "update")) {
//		// Congdar: add IsEngaged check for exploit to keep bots alive by repeatedly using #bot update.
//		if((c->GetTarget() != NULL) && c->GetTarget()->IsBot()) {
//			if(c->GetLevel() <= c->GetTarget()->GetLevel()) {
//				c->Message(15, "This bot has already been updated.");
//				return;
//			}
//
//			if(c->IsBotRaiding()) {
//				BotRaids *br = entity_list.GetBotRaidByMob(c->CastToMob());
//				if(br) {
//					if(br->GetBotRaidAggro()) {
//						c->Message(15, "You can't update bots while you are engaged.");
//						return;
//					}
//				}
//			}
//
//			if(c->IsGrouped())
//			{
//				Group *g = entity_list.GetGroupByClient(c);
//				for (int i=0; i<MAX_GROUP_MEMBERS; i++)
//				{
//					if(g && g->members[i] && g->members[i]->IsEngaged())
//					{
//						c->Message(15, "You can't update bots while you are engaged.");
//						return;
//					}
//				}
//			}
//
//			if((c->GetTarget()->BotOwner == c->CastToMob()) && !c->GetFeigned()) {
//				Mob *bot = c->GetTarget();
//				bot->SetLevel(c->GetLevel());
//				bot->SetPetChooser(false);
//				bot->CalcBotStats();
//			}
//			else {
//				if(c->GetFeigned()) {
//					c->Message(15, "You cannot update bots while feigned.");
//				}
//				else {
//					c->Message(15, "You must target your bot first");
//				}
//			}
//		}
//		else {
//			c->Message(15, "You must target a bot first");
//		}
//		return;
//	}
//
//	if(!strcasecmp(sep->arg[1], "group") && !strcasecmp(sep->arg[2], "summon") ) {
//		if(c->IsBotRaiding()) {
//			BotRaids *brsummon = entity_list.GetBotRaidByMob(c->CastToMob());
//			if(brsummon) {
//				brsummon->SummonRaidBots(c->CastToMob(), false);
//			}
//		}
//		else if(c->IsGrouped())
//		{
//			Group *g = c->GetGroup();
//			if(g) {
//				for(int i=0; i<MAX_GROUP_MEMBERS; i++)
//				{
//					if(g->members[i] && g->members[i]->IsBot()) {
//						g->members[i]->SetTarget(g->members[i]->BotOwner);
//						g->members[i]->Warp(c->GetX(), c->GetY(), c->GetZ());
//						if(g->members[i]->GetPetID()) {
//							g->members[i]->GetPet()->SetTarget(g->members[i]);
//							g->members[i]->GetPet()->Warp(c->GetX(), c->GetY(), c->GetZ());
//						}
//					}
//				}
//			}
//		}
//		return;
//	}
//
//	//Bind
//	if(!strcasecmp(sep->arg[1], "bindme")) {
//		Mob *binder = NULL;
//		bool hasbinder = false;
//		if(c->IsGrouped())
//		{
//			Group *g = c->GetGroup();
//			if(g) {
//				for(int i=0; i<MAX_GROUP_MEMBERS; i++)
//				{
//					if(g->members[i] && g->members[i]->IsBot() && (g->members[i]->GetClass() == CLERIC))
//					{
//						hasbinder = true;
//						binder = g->members[i];
//					}
//				}
//				if(!hasbinder) {
//					c->Message(15, "You must have a Cleric in your group.");
//				}
//			}
//		}
//		if(hasbinder) {
//			binder->Say("Attempting to bind you %s.", c->GetName());
//			binder->CastToNPC()->CastSpell(35, c->GetID(), 1, -1, -1);
//		}
//		return;
//	}
//
//	// Rune
//	if(!strcasecmp(sep->arg[1], "runeme")) {
//		Mob *runeer = NULL;
//		bool hasruneer = false;
//		if(c->IsGrouped())
//		{
//			Group *g = c->GetGroup();
//			if(g) {
//				for(int i=0; i<MAX_GROUP_MEMBERS; i++)
//				{
//					if(g->members[i] && g->members[i]->IsBot() && (g->members[i]->GetClass() == ENCHANTER))
//					{
//						hasruneer = true;
//						runeer = g->members[i];
//					}
//				}
//				if(!hasruneer) {
//					c->Message(15, "You must have an Enchanter in your group.");
//				}
//			}
//		}
//		if(hasruneer) {
//			if      (c->GetLevel() <= 12) {
//				runeer->Say("I need to be level 13 or higher for this...");
//			}
//			else if ((c->GetLevel() >= 13) && (c->GetLevel() <= 21)) {
//				runeer->Say("Casting Rune I...");
//				runeer->CastSpell(481, c->GetID(), 1, -1, -1);
//			}
//			else if ((c->GetLevel() >= 22) && (c->GetLevel() <= 32)) {
//				runeer->Say("Casting Rune II...");
//				runeer->CastSpell(482, c->GetID(), 1, -1, -1);
//			}
//			else if ((c->GetLevel() >= 33) && (c->GetLevel() <= 39)) { 
//				runeer->Say("Casting Rune III...");
//				runeer->CastSpell(483, c->GetID(), 1, -1, -1);
//			}
//			else if ((c->GetLevel() >= 40) && (c->GetLevel() <= 51)) { 
//				runeer->Say("Casting Rune IV...");
//				runeer->CastSpell(484, c->GetID(), 1, -1, -1);
//			}
//			else if ((c->GetLevel() >= 52) && (c->GetLevel() <= 60)) { 
//				runeer->Say("Casting Rune V...");
//				runeer->CastSpell(1689, c->GetID(), 1, -1, -1);
//			}
//			else if (c->GetLevel() >= 61){ 
//				runeer->Say("Casting Rune of Zebuxoruk...");
//				runeer->CastSpell(3343, c->GetID(), 1, -1, -1);
//			}
//		}
//		return;
//	}
//
//	//Tracking
//	if(!strcasecmp(sep->arg[1], "track") && c->IsGrouped()) {
//		Mob *Tracker;
//		int32 TrackerClass = 0;
//
//		Group *g = c->GetGroup();
//		if(g) {
//			for(int i=0; i<MAX_GROUP_MEMBERS; i++) {
//				if(g->members[i] && g->members[i]->IsBot()) {
//					switch(g->members[i]->GetClass()) {
//						case RANGER:
//							Tracker = g->members[i];
//							TrackerClass = RANGER;
//							break;
//						case BARD:
//							// If we haven't found a tracker yet, use bard.
//							if(TrackerClass == 0) {
//								Tracker = g->members[i];
//								TrackerClass = BARD;
//							}
//							break;
//						case DRUID:
//							// Unless we have a ranger, druid is next best.
//							if(TrackerClass != RANGER) {
//								Tracker = g->members[i];
//								TrackerClass = DRUID;
//							}
//							break;
//						default:
//							break;
//					}
//				}
//			}
//
//			int Level = (c->GetLevel());
//			int RangeR = (Level*80); //Ranger
//			int RangeD = (Level*30); //Druid
//			int RangeB = (Level*20); //Bard
//			switch(TrackerClass) {
//				case RANGER:
//					if(!strcasecmp(sep->arg[2], "all")) {
//						Tracker->Say("Tracking everything", c->GetName());
//						entity_list.ShowSpawnWindow(c, RangeR, false);
//					}
//					else if(!strcasecmp(sep->arg[2], "rare")) { 
//						Tracker->Say("Selective tracking", c->GetName());
//						entity_list.ShowSpawnWindow(c, RangeR, true);
//					}
//					else if(!strcasecmp(sep->arg[2], "near")) { 
//						Tracker->Say("Tracking mobs nearby", c->GetName());
//						entity_list.ShowSpawnWindow(c, RangeD, false);
//					}
//					else 
//						Tracker->Say("You want to [track all], [track near], or [track rare]?", c->GetName());
//
//					break;
//
//				case BARD:
//
//					if(TrackerClass != RANGER)
//						Tracker->Say("Tracking up", c->GetName());
//					entity_list.ShowSpawnWindow(c, RangeB, false);
//					break;
//
//				case DRUID:
//
//					if(TrackerClass = BARD)
//						Tracker->Say("Tracking up", c->GetName());
//					entity_list.ShowSpawnWindow(c, RangeD, false);
//					break;
//
//				default:
//					c->Message(15, "You must have a Ranger, Druid, or Bard in your group.");
//					break;
//			}
//		}
//	}
//
//	//Cure
//	if ((!strcasecmp(sep->arg[1], "cure")) && (c->IsGrouped())) {
//		Mob *Curer;
//		int32 CurerClass = 0;
//		Group *g = c->GetGroup();
//		if(g) {
//			for(int i=0; i<MAX_GROUP_MEMBERS; i++){
//				if(g->members[i] && g->members[i]->IsBot()) {
//					switch(g->members[i]->GetClass()) {
//						case CLERIC:
//							Curer = g->members[i];
//							CurerClass = CLERIC;
//							break;
//						case SHAMAN:
//							if(CurerClass != CLERIC){
//								Curer = g->members[i];
//								CurerClass = SHAMAN;
//							}
//						case DRUID:
//							if (CurerClass == 0){
//								Curer = g->members[i];
//								CurerClass = DRUID;
//							}
//							break;
//							break;
//						default:
//							break;
//					}
//				}
//			}
//			switch(CurerClass) {
//				case CLERIC:
//					if	(!strcasecmp(sep->arg[2], "poison") && (c->GetLevel() >= 1))  {
//						Curer->Say("Trying to cure us of %s.", sep->arg[2]);
//						Curer->CastToNPC()->Bot_Command_Cure(1, Curer->GetLevel());
//					}
//					else if (!strcasecmp(sep->arg[2], "disease") && (c->GetLevel() >= 4)) {
//						Curer->Say("Trying to cure us of %s.", sep->arg[2]);
//						Curer->CastToNPC()->Bot_Command_Cure(2, Curer->GetLevel());
//					}
//					else if(!strcasecmp(sep->arg[2], "curse") && (c->GetLevel() >= 8)) {
//						Curer->Say("Trying to cure us of %s.", sep->arg[2]);
//						Curer->CastToNPC()->Bot_Command_Cure(3, Curer->GetLevel());
//					}
//					else if(!strcasecmp(sep->arg[2], "blindness") && (c->GetLevel() >= 3)) {
//						Curer->Say("Trying to cure us of %s.", sep->arg[2]);
//						Curer->CastToNPC()->Bot_Command_Cure(4, Curer->GetLevel());
//					}
//					else if (!strcasecmp(sep->arg[2], "curse") && (c->GetLevel() <= 8)
//						|| !strcasecmp(sep->arg[2], "blindness") && (c->GetLevel() <= 3) 
//						|| !strcasecmp(sep->arg[2], "disease") && (c->GetLevel() <= 4)
//						|| !strcasecmp(sep->arg[2], "poison") && (c->GetLevel() <= 1)) {
//							Curer->Say("I don't have the needed level yet", sep->arg[2]);
//					}
//					else
//						Curer->Say("Do you want [cure poison], [cure disease], [cure curse], or [cure blindness]?", c->GetName());
//
//					break;
//
//				case SHAMAN:
//					if	(!strcasecmp(sep->arg[2], "poison") && (c->GetLevel() >= 2))  {
//						Curer->Say("Trying to cure us of %s.", sep->arg[2]);
//						Curer->CastToNPC()->Bot_Command_Cure(1, Curer->GetLevel());
//					}
//					else if (!strcasecmp(sep->arg[2], "disease") && (c->GetLevel() >= 1)) {
//						Curer->Say("Trying to cure us of %s.", sep->arg[2]);
//						Curer->CastToNPC()->Bot_Command_Cure(2, Curer->GetLevel());
//					}
//					else if(!strcasecmp(sep->arg[2], "curse")) {
//						Curer->Say("I don't have that spell", sep->arg[2]);
//					}
//					else if(!strcasecmp(sep->arg[2], "blindness") && (c->GetLevel() >= 7)) {
//						Curer->Say("Trying to cure us of %s.", sep->arg[2]);
//						Curer->CastToNPC()->Bot_Command_Cure(4, Curer->GetLevel());
//					}
//					else if (!strcasecmp(sep->arg[2], "blindness") && (c->GetLevel() <= 7) 
//						|| !strcasecmp(sep->arg[2], "disease") && (c->GetLevel() <= 1)
//						|| !strcasecmp(sep->arg[2], "poison") && (c->GetLevel() <= 2)) {
//							Curer->Say("I don't have the needed level yet", sep->arg[2]);
//					}
//					else 
//						Curer->Say("Do you want [cure poison], [cure disease], or [cure blindness]?", c->GetName());
//
//					break;
//
//				case DRUID:
//
//					if	(!strcasecmp(sep->arg[2], "poison") && (c->GetLevel() >= 5)) {
//						Curer->Say("Trying to cure us of %s.", sep->arg[2]);
//						Curer->CastToNPC()->Bot_Command_Cure(1, Curer->GetLevel());
//					}
//					else if (!strcasecmp(sep->arg[2], "disease") && (c->GetLevel() >= 4)) {
//						Curer->Say("Trying to cure us of %s.", sep->arg[2]);
//						Curer->CastToNPC()->Bot_Command_Cure(2, Curer->GetLevel());
//					}
//					else if(!strcasecmp(sep->arg[2], "curse")) { // Fire level 1
//						Curer->Say("I don't have that spell", sep->arg[2]);
//					}
//					else if(!strcasecmp(sep->arg[2], "blindness") && (c->GetLevel() >= 13)) {
//						Curer->Say("I don't have that spell", sep->arg[2]);
//					}
//					else if (!strcasecmp(sep->arg[2], "disease") && (c->GetLevel() <= 4)
//						|| !strcasecmp(sep->arg[2], "poison") && (c->GetLevel() <= 5)) {
//							Curer->Say("I don't have the needed level yet", sep->arg[2]) ;
//					}
//					else 
//						Curer->Say("Do you want [cure poison], or [cure disease]?", c->GetName());
//
//					break;
//
//				default:
//					c->Message(15, "You must have a Cleric, Shaman, or Druid in your group.");
//					break;
//			}
//		}
//	}
//
//	//Mez
//	if(!strcasecmp(sep->arg[1], "ai") && !strcasecmp(sep->arg[2], "mez"))
//	{
//		Mob *target = c->GetTarget();
//		if(target == NULL || target == c || target->IsBot() || target->IsPet() && target->GetOwner()->IsBot())
//		{
//			c->Message(15, "You must select a monster");
//			return;
//		}
//
//		if(c->IsGrouped())
//		{
//			bool hasmezzer = false;
//			Group *g = c->GetGroup();
//			for(int i=0; i<MAX_GROUP_MEMBERS; i++)
//			{
//				if(g && g->members[i] && g->members[i]->IsBot() && (g->members[i]->GetClass() == ENCHANTER))
//				{
//					hasmezzer = true;
//					Mob *mezzer = g->members[i];
//					mezzer->Say("Trying to mez %s \n", target->GetCleanName());
//					mezzer->CastToNPC()->Bot_Command_MezzTarget(target);
//				}
//			}
//			if(!hasmezzer) {
//				c->Message(15, "You must have an Enchanter in your group.");
//			}
//		}
//		return;
//	}
//
//	//Lore (Identify item)
//	if(!strcasecmp(sep->arg[1], "lore")) {
//		if(c->IsGrouped())
//		{
//			bool hascaster = false;
//			Group *g = c->GetGroup();
//			for(int i=0; i<MAX_GROUP_MEMBERS; i++)
//			{
//				if(g && g->members[i] && g->members[i]->IsBot()) {
//					uint8 casterlevel = g->members[i]->GetLevel();
//					switch(g->members[i]->GetClass()) {
//						case ENCHANTER:
//							if(casterlevel >= 15) {
//								hascaster = true;
//							}
//							break;
//						case WIZARD:
//							if(casterlevel >= 14) {
//								hascaster = true;
//							}
//							break;
//						case NECROMANCER:
//							if(casterlevel >= 17) {
//								hascaster = true;
//							}
//							break;
//						case MAGICIAN:
//							if(casterlevel >= 13) {
//								hascaster = true;
//							}
//							break;
//						default:
//							break;
//					}
//					if(hascaster) {
//						g->members[i]->Say("Trying to Identify your item...");
//						g->members[i]->CastSpell(305, c->GetID(), 1, -1, -1);
//						break;
//					}
//				}
//			}
//			if(!hascaster) {
//				c->Message(15, "You don't see anyone in your group that can cast Identify.");
//			}
//		}
//		else {
//			c->Message(15, "You don't see anyone in your group that can cast Identify.");
//		}
//		return;
//	}
//
//	//Resurrect
//	if(!strcasecmp(sep->arg[1], "resurrectme"))
//	{
//		Mob *target = c->GetTarget();
//		if(target == NULL || !target->IsCorpse())
//		{
//			c->Message(15, "You must select a corpse");
//			return;
//		}
//
//		if(c->IsGrouped())
//		{
//			bool hasrezzer = false;
//			Group *g = c->GetGroup();
//			for(int i=0; i<MAX_GROUP_MEMBERS; i++)
//			{
//				if(g && g->members[i] && g->members[i]->IsBot() && (g->members[i]->GetClass() == CLERIC))
//				{
//					hasrezzer = true;
//					Mob *rezzer = g->members[i];
//					rezzer->Say("Trying to rez %s", target->GetCleanName());
//					rezzer->CastToNPC()->Bot_Command_RezzTarget(target);
//					break;
//				}
//			}
//			if(!hasrezzer) {
//				c->Message(15, "You must have a Cleric in your group.");
//			}
//		}
//		else {
//			c->Message(15, "You must have a Cleric in your group.");
//		}
//		return;
//	}
//
//	if(!strcasecmp(sep->arg[1], "magepet"))
//	{
//		if(c->GetTarget() && c->GetTarget()->IsBot() && (c->GetTarget()->GetClass() == MAGICIAN))
//		{
//			if(database.GetBotOwner(c->GetTarget()->GetNPCTypeID()) == c->AccountID())
//			{
//				int botlevel = c->GetTarget()->GetLevel();
//				c->GetTarget()->SetPetChooser(true);
//				if(botlevel == 1)
//				{
//					c->GetTarget()->Say("I don't have any pets yet.");
//					return;
//				}
//				if(!strcasecmp(sep->arg[2], "water"))
//				{
//					c->GetTarget()->SetPetChooserID(0);
//				}
//				else if(!strcasecmp(sep->arg[2], "fire"))
//				{
//					if(botlevel < 3)
//					{
//						c->GetTarget()->Say("I don't have that pet yet.");
//						return;
//					}
//					else
//					{
//						c->GetTarget()->SetPetChooserID(1);
//					}
//				}
//				else if(!strcasecmp(sep->arg[2], "air"))
//				{
//					if(botlevel < 4)
//					{
//						c->GetTarget()->Say("I don't have that pet yet.");
//						return;
//					}
//					else
//					{
//						c->GetTarget()->SetPetChooserID(2);
//					}
//				}
//				else if(!strcasecmp(sep->arg[2], "earth"))
//				{
//					if(botlevel < 5)
//					{
//						c->GetTarget()->Say("I don't have that pet yet.");
//						return;
//					}
//					else
//					{
//						c->GetTarget()->SetPetChooserID(3);
//					}
//				}
//				else if(!strcasecmp(sep->arg[2], "monster"))
//				{
//					if(botlevel < 30)
//					{
//						c->GetTarget()->Say("I don't have that pet yet.");
//						return;
//					}
//					else
//					{
//						c->GetTarget()->SetPetChooserID(4);
//					}
//				}
//				if(c->GetTarget()->GetPet())
//				{
//					// cast reclaim energy
//					int16 id = c->GetTarget()->GetPetID();
//					c->GetTarget()->SetPetID(0);
//					c->GetTarget()->CastSpell(331, id);
//				}
//			}
//		}
//		else
//		{
//			c->Message(15, "You must target your Magician bot.");
//		}
//		return;
//	}
//
//	//Summon Corpse
//	if(!strcasecmp(sep->arg[1], "corpse") && !strcasecmp(sep->arg[2], "summon")) {
//		if(c->GetTarget() == NULL) {
//			c->Message(15, "You must select player with his corpse in the zone.");
//			return;
//		}
//		if(c->IsGrouped()) {
//			bool hassummoner = false;
//			Mob *t = c->GetTarget();
//			Group *g = c->GetGroup();
//			int summonerlevel = 0;
//			for(int i=0; i<MAX_GROUP_MEMBERS; i++) {
//				if(g && g->members[i] && g->members[i]->IsBot() && ((g->members[i]->GetClass() == NECROMANCER)||(g->members[i]->GetClass() == SHADOWKNIGHT))) {
//					hassummoner = true;
//					summonerlevel = g->members[i]->GetLevel();
//					if(!t->IsClient()) {
//						g->members[i]->Say("You have to target a player with a corpse in the zone");
//						return;
//					}
//					else if(summonerlevel < 12) {
//						g->members[i]->Say("I don't have that spell yet.");
//					}
//					else if((summonerlevel > 11) && (summonerlevel < 35)) {
//						g->members[i]->Say("Attempting to summon %s\'s corpse.", t->GetCleanName());
//						g->members[i]->CastSpell(2213, t->GetID(), 1, -1, -1);
//						return;
//					}
//					else if((summonerlevel > 34) && (summonerlevel < 71)) {
//						g->members[i]->Say("Attempting to summon %s\'s corpse.", t->GetCleanName());
//						g->members[i]->CastSpell(3, t->GetID(), 1, -1, -1);
//						return;
//					}
//					else if((summonerlevel > 70) && (summonerlevel < 76)) {
//						g->members[i]->Say("Attempting to summon %s\'s corpse.", t->GetCleanName());
//						g->members[i]->CastSpell(10042, t->GetID(), 1, -1, -1);
//						return;
//					}
//					else if((summonerlevel > 75) && (summonerlevel < 81)) {
//						g->members[i]->Say("Attempting to summon %s\'s corpse.", t->GetCleanName());
//						g->members[i]->CastSpell(14823, t->GetID(), 1, -1, -1);
//						return;
//					}
//				}
//			}
//			if (!hassummoner) {
//				c->Message(15, "You must have a Necromancer or Shadowknight in your group.");
//			}
//			return;
//		}
//	}
//
//	//Pacify
//	if(!strcasecmp(sep->arg[1], "target") && !strcasecmp(sep->arg[2], "calm"))
//	{
//		Mob *target = c->GetTarget();
//		if(target == NULL || target->IsClient() || target->IsBot() || target->IsPet() && target->GetOwner()->IsBot())
//		{
//			c->Message(15, "You must select a monster");
//			return;
//		}
//		if(c->IsGrouped())
//		{
//			bool haspacer = false;
//			Group *g = c->GetGroup();
//			for(int i=0; i<MAX_GROUP_MEMBERS; i++)
//			{
//				if(g && g->members[i] && g->members[i]->IsBot() && (g->members[i]->GetClass() == ENCHANTER) && (!haspacer))
//				{
//					haspacer = true;
//					Mob *pacer = g->members[i];
//					pacer->Say("Trying to calm %s \n", target->GetCleanName());
//					pacer->CastToNPC()->Bot_Command_CalmTarget(target);
//					c->GetTarget()->CastToMob()->BotEffect(c);
//				}
//				else if(g && g->members[i] && g->members[i]->IsBot() && (g->members[i]->GetClass() == CLERIC) && (!haspacer))
//				{
//					haspacer = true;
//					Mob *pacer = g->members[i];
//					pacer->Say("Trying to calm %s \n", target->GetCleanName());
//					pacer->CastToNPC()->Bot_Command_CalmTarget(target);
//					c->GetTarget()->CastToMob()->BotEffect(c);
//				}
//			}
//			if(!haspacer) {
//				c->Message(15, "You must have an Enchanter or Cleric in your group.");
//			}
//			return;
//		}
//	}
//
//	//Charm
//	if(!strcasecmp(sep->arg[1], "charm"))
//	{
//		Mob *target = c->GetTarget();
//		if(target == NULL || target->IsClient() || target->IsBot() || (target->IsPet() && target->GetOwner()->IsBot()))
//		{
//			c->Message(15, "You must select a monster");
//			return;
//		}
//		int32 DBtype = c->GetTarget()->GetBodyType();
//		Mob *Charmer;
//		int32 CharmerClass = 0;
//		Group *g = c->GetGroup();
//		if(g) {
//			for(int i=0; i<MAX_GROUP_MEMBERS; i++){
//				if(g->members[i] && g->members[i]->IsBot()) {
//					switch(g->members[i]->GetClass()) {
//						case ENCHANTER:
//							Charmer = g->members[i];
//							CharmerClass = ENCHANTER;
//							break;
//						case NECROMANCER:
//							if(CharmerClass != ENCHANTER){
//								Charmer = g->members[i];
//								CharmerClass = NECROMANCER;
//							}
//						case DRUID:
//							if (CharmerClass == 0){
//								Charmer = g->members[i];
//								CharmerClass = DRUID;
//							}
//							break;
//							break;
//						default:
//							break;
//					}
//				}
//			}
//			switch(CharmerClass) {
//				case ENCHANTER:
//					if	(c->GetLevel() >= 11) {
//						Charmer->Say("Trying to charm %s \n", target->GetCleanName(), sep->arg[2]);
//						Charmer->CastToNPC()->Bot_Command_CharmTarget (1,target);
//					}
//					else if (c->GetLevel() <= 10){
//						Charmer->Say("I don't have the needed level yet", sep->arg[2]);
//					}
//					else
//						Charmer->Say("Mob level is too high or can't be charmed", c->GetName());
//					break;
//
//				case NECROMANCER:
//					if	((c->GetLevel() >= 18) && (DBtype == 3)) {
//						Charmer->Say("Trying to Charm %s \n", target->GetCleanName(), sep->arg[2]);
//						Charmer->CastToNPC()->Bot_Command_CharmTarget (2,target);
//					}
//					else if (c->GetLevel() <= 17){
//						Charmer->Say("I don't have the needed level yet", sep->arg[2]);
//					}
//					else
//						Charmer->Say("Mob Is not undead...", c->GetName());
//					break;
//
//				case DRUID:
//					if	((c->GetLevel() >= 13) && (DBtype == 21)) {
//						Charmer->Say("Trying to charm %s \n", target->GetCleanName(), sep->arg[2]);
//						Charmer->CastToNPC()->Bot_Command_CharmTarget (3,target);
//					}
//					else if (c->GetLevel() <= 12){
//						Charmer->Say("I don't have the needed level yet", sep->arg[2]);
//					}
//					else
//						Charmer->Say("Mob is not an animal...", c->GetName());
//					break;
//
//				default:
//					c->Message(15, "You must have an Enchanter, Necromancer or Druid in your group.");
//					break;
//			}
//		}
//	}
//
//	// Remove Bot's Pet
//	if(!strcasecmp(sep->arg[1], "pet") && !strcasecmp(sep->arg[2], "remove")) {
//		if(c->GetTarget() != NULL) {
//			if (c->IsGrouped() && c->GetTarget()->IsBot() && (database.GetBotOwner(c->GetTarget()->GetNPCTypeID()) == c->AccountID()) &&
//				((c->GetTarget()->GetClass() == NECROMANCER) || (c->GetTarget()->GetClass() == ENCHANTER) || (c->GetTarget()->GetClass() == DRUID))) {
//					if(c->GetTarget()->IsBotCharmer()) {
//						c->GetTarget()->SetBotCharmer(false);
//						c->GetTarget()->Say("Using a summoned pet.");
//					}
//					else {
//						if(c->GetTarget()->GetPet())
//						{
//							c->GetTarget()->GetPet()->Say_StringID(PET_GETLOST_STRING);
//							c->GetTarget()->GetPet()->Kill();
//							c->GetTarget()->SetPetID(0);
//						}
//						c->GetTarget()->SetBotCharmer(true);
//						c->GetTarget()->Say("Available for Dire Charm command.");
//					}
//			}
//			else {
//				c->Message(15, "You must target your Enchanter, Necromancer, or Druid bot.");
//			}
//		}
//		else {
//			c->Message(15, "You must target an Enchanter, Necromancer, or Druid bot.");
//		}
//		return;
//	}
//
//	//Dire Charm
//	if(!strcasecmp(sep->arg[1], "Dire") && !strcasecmp(sep->arg[2], "Charm"))
//	{
//		Mob *target = c->GetTarget();
//		if(target == NULL || target->IsClient() || target->IsBot() || (target->IsPet() && target->GetOwner()->IsBot()))
//		{
//			c->Message(15, "You must select a monster");
//			return;
//		}
//		int32 DBtype = c->GetTarget()->GetBodyType();
//		Mob *Direr;
//		int32 DirerClass = 0;
//		Group *g = c->GetGroup();
//		if(g) {
//			for(int i=0; i<MAX_GROUP_MEMBERS; i++){
//				if(g->members[i] && g->members[i]->IsBot()) {
//					switch(g->members[i]->GetClass()) {
//						case ENCHANTER:
//							Direr = g->members[i];
//							DirerClass = ENCHANTER;
//							break;
//						case NECROMANCER:
//							if(DirerClass != ENCHANTER){
//								Direr = g->members[i];
//								DirerClass = NECROMANCER;
//							}
//						case DRUID:
//							if (DirerClass == 0){
//								Direr = g->members[i];
//								DirerClass = DRUID;
//							}
//							break;
//							break;
//						default:
//							break;
//					}
//				}
//			}
//			switch(DirerClass) {
//				case ENCHANTER:
//					if	(c->GetLevel() >= 55) {
//						Direr->Say("Trying to dire charm %s \n", target->GetCleanName(), sep->arg[2]);
//						Direr->CastToNPC()->Bot_Command_DireTarget (1,target);
//					}
//					else if (c->GetLevel() <= 55){
//						Direr->Say("I don't have the needed level yet", sep->arg[2]);
//					}
//					else
//						Direr->Say("Mob level is too high or can't be charmed", c->GetName());
//					break;
//
//				case NECROMANCER:
//					if	((c->GetLevel() >= 55) && (DBtype == 3)) {
//						Direr->Say("Trying to dire charm %s \n", target->GetCleanName(), sep->arg[2]);
//						Direr->CastToNPC()->Bot_Command_DireTarget (2,target);
//					}
//					else if (c->GetLevel() <= 55){
//						Direr->Say("I don't have the needed level yet", sep->arg[2]);
//					}
//					else
//						Direr->Say("Mob Is not undead...", c->GetName());
//					break;
//
//				case DRUID:
//					if	((c->GetLevel() >= 55) && (DBtype == 21)) {
//						Direr->Say("Trying to dire charm %s \n", target->GetCleanName(), sep->arg[2]);
//						Direr->CastToNPC()->Bot_Command_DireTarget (3,target);
//					}
//					else if (c->GetLevel() <= 55){
//						Direr->Say("I don't have the needed level yet", sep->arg[2]);
//					}
//					else
//						Direr->Say("Mob is not an animal...", c->GetName());
//					break;
//
//				default:
//					c->Message(15, "You must have an Enchanter, Necromancer or Druid in your group.");
//					break;
//			}
//		}
//	}
//
//	// Evacuate
//	if(!strcasecmp(sep->arg[1], "evac")) {
//		Mob *evac = NULL;
//		bool hasevac = false;
//		if(c->IsGrouped())
//		{
//			Group *g = c->GetGroup();
//			if(g) {
//				for(int i=0; i<MAX_GROUP_MEMBERS; i++)
//				{
//					if((g->members[i] && g->members[i]->IsBot() && (g->members[i]->GetClass() == DRUID)) 
//						|| (g->members[i] && g->members[i]->IsBot() && (g->members[i]->GetClass() == WIZARD)))
//					{
//						hasevac = true;
//						evac = g->members[i];
//					}
//				}
//				if(!hasevac) {
//					c->Message(15, "You must have a Druid in your group.");
//				}
//			}
//		}
//		if((hasevac)  && (c->GetLevel() >= 18)) {
//			evac->Say("Attempting to Evac you %s.", c->GetName());
//			evac->CastToClient()->CastSpell(2183, c->GetID(), 1, -1, -1);
//		}
//		else if((hasevac)  && (c->GetLevel() <= 17)) {
//			evac->Say("I'm not level 18 yet.", c->GetName());
//		}
//		return;
//	}
//
//	// Sow
//	if ((!strcasecmp(sep->arg[1], "sow")) && (c->IsGrouped())) {
//		Mob *Sower;
//		int32 SowerClass = 0;
//		Group *g = c->GetGroup();
//		if(g) {
//			for(int i=0; i<MAX_GROUP_MEMBERS; i++){
//				if(g->members[i] && g->members[i]->IsBot()) {
//					switch(g->members[i]->GetClass()) {
//						case DRUID:
//							Sower = g->members[i];
//							SowerClass = DRUID;
//							break;
//						case SHAMAN:
//							if (SowerClass != DRUID){
//								Sower = g->members[i];
//								SowerClass = SHAMAN;
//							}
//							break;
//						case RANGER:
//							if (SowerClass == 0){
//								Sower = g->members[i];
//								SowerClass = RANGER;
//							}
//							break;
//						case BEASTLORD:
//							if (SowerClass == 0){
//								Sower = g->members[i];
//								SowerClass = BEASTLORD;
//							}
//							break;
//						default:
//							break;
//					}
//				}
//			}
//			switch(SowerClass) {
//				case DRUID:
//					if      ((!strcasecmp(sep->arg[2], "regular")) && (zone->CanCastOutdoor())  && (c->GetLevel() >= 10) ) {
//						Sower->Say("Casting sow...");
//						Sower->CastSpell(278, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "regular")) && (zone->CanCastOutdoor())  && (c->GetLevel() <= 10) ) {
//						Sower->Say("I'm not level 10 yet.");
//					}
//					else if ((!strcasecmp(sep->arg[2], "wolf")) && zone->CanCastOutdoor() && (c->GetLevel() >= 20)) {
//						Sower->Say("Casting group wolf...");
//						Sower->CastSpell(428, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "wolf")) && (c->GetLevel() <= 20)) {
//						Sower->Say("I'm not level 20 yet.");
//					}
//					else if ((!strcasecmp(sep->arg[2], "feral")) && (c->GetLevel() >= 50)) { 
//						Sower->Say("Casting Feral Pack...");
//						Sower->CastSpell(4058, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "feral")) && (c->GetLevel() <= 50)) {
//						Sower->Say("I'm not level 50 yet.");
//					}
//					else if ((!strcasecmp(sep->arg[2], "shrew")) && (c->GetLevel() >= 35)) { 
//						Sower->Say("Casting Pack Shrew...");
//						Sower->CastSpell(4055, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "wolf")) && (c->GetLevel() <= 35)) {
//						Sower->Say("I'm not level 35 yet.");
//					}
//					else if ((!zone->CanCastOutdoor()) && (!strcasecmp(sep->arg[2], "regular")) ||
//						(!zone->CanCastOutdoor()) && (!strcasecmp(sep->arg[2], "wolf"))) {
//							Sower->Say("I can't cast this spell indoors, try [sow shrew] if you're 35 or higher, or [sow feral] if you're 50 or higher,", c->GetName());
//					}
//					else if (!zone->CanCastOutdoor()) {
//						Sower->Say("I can't cast this spell indoors, try [sow shrew] if you're 35 or higher, or [sow feral] if you're 50 or higher,", c->GetName());
//					}
//					else if (zone->CanCastOutdoor()) {
//						Sower->Say("Do you want [sow regular] or [sow wolf]?", c->GetName());
//					}
//					else if (!zone->CanCastOutdoor()) {
//						Sower->Say("I can't cast this spell indoors, try [sow shrew] if you're 35 or higher, or [sow feral] if you're 50 or higher,", c->GetName());
//					}
//					break;
//
//				case SHAMAN:
//
//					if ((zone->CanCastOutdoor()) && (c->GetLevel() >= 9)) { 
//						Sower->Say("Casting SoW...");
//						Sower->CastToClient()->CastSpell(278, c->GetID(), 1, -1, -1);
//					}
//					else if (!zone->CanCastOutdoor()) {
//						Sower->Say("I can't cast this spell indoors", c->GetName());
//					}
//					else if (c->GetLevel() <= 9) {
//						Sower->Say("I'm not level 9 yet.");
//					}
//					break;
//
//				case RANGER:
//
//					if ((zone->CanCastOutdoor()) && (c->GetLevel() >= 28)){
//						Sower->Say("Casting SoW...");
//						Sower->CastToClient()->CastSpell(278, c->GetID(), 1, -1, -1);
//					}
//					else if (!zone->CanCastOutdoor()) {
//						Sower->Say("I can't cast this spell indoors", c->GetName());
//					}
//					else if (c->GetLevel() <= 28) {
//						Sower->Say("I'm not level 28 yet.");
//					}
//					break;
//
//				case BEASTLORD:
//
//					if((zone->CanCastOutdoor()) && (c->GetLevel() >= 24)) {
//						Sower->Say("Casting SoW...");
//						Sower->CastToClient()->CastSpell(278, c->GetID(), 1, -1, -1);
//					}
//					else if (!zone->CanCastOutdoor()) {
//						Sower->Say("I can't cast this spell indoors", c->GetName());
//					}
//					else if (c->GetLevel() <= 24) {
//						Sower->Say("I'm not level 24 yet.");
//					}
//					break;
//
//
//				default:
//					c->Message(15, "You must have a Druid, Shaman, Ranger,  or Beastlord in your group.");
//					break;
//			}
//		}
//	}
//
//	//Shrink
//	if ((!strcasecmp(sep->arg[1], "shrinkme")) && (c->IsGrouped())) {
//		Mob *Shrinker;
//		int32 ShrinkerClass = 0;
//		Group *g = c->GetGroup();
//
//		if(g) {
//			for(int i=0; i<MAX_GROUP_MEMBERS; i++){
//				if(g->members[i] && g->members[i]->IsBot()) {
//					switch(g->members[i]->GetClass()) {
//						case SHAMAN:
//							Shrinker = g->members[i];
//							ShrinkerClass = SHAMAN;
//							break;
//						case BEASTLORD:
//							if (ShrinkerClass != SHAMAN){
//								Shrinker = g->members[i];
//								ShrinkerClass = BEASTLORD;
//							}
//							break;
//						default:
//							break;
//					}
//				}
//			}
//			switch(ShrinkerClass) {
//				case SHAMAN:
//
//					if (c->GetLevel() >= 15) { 
//						Shrinker->Say("Casting Shrink...");
//						Shrinker->CastToNPC()->BotRaidSpell(345);
//					}
//					else if (c->GetLevel() <= 14) {
//						Shrinker->Say("I'm not level 15 yet.");
//					}
//					break;
//
//				case BEASTLORD:
//
//					if (c->GetLevel() >= 23) {
//						Shrinker->Say("Casting Shrink...");
//						Shrinker->CastToNPC()->BotRaidSpell(345);
//					}
//					else if (c->GetLevel() <= 22) {
//						Shrinker->Say("I'm not level 23 yet.");
//					}
//					break;
//
//				default:
//					c->Message(15, "You must have a Shaman or Beastlord in your group.");
//					break;
//			}
//		}
//	}
//
//	// Gate
//	if ((!strcasecmp(sep->arg[1], "gate")) && (c->IsGrouped())) {
//		Mob *Gater;
//		int32 GaterClass = 0;
//		Group *g = c->GetGroup();
//		if(g) {
//			for(int i=0; i<MAX_GROUP_MEMBERS; i++){
//				if(g->members[i] && g->members[i]->IsBot()) {
//					switch(g->members[i]->GetClass()) {
//						case DRUID:
//							Gater = g->members[i];
//							GaterClass = DRUID;
//							break;
//						case WIZARD:
//							if (GaterClass == 0){
//								Gater = g->members[i];
//								GaterClass = WIZARD;
//							}
//							break;
//						default:
//							break;
//					}
//				}
//			}
//			switch(GaterClass) {
//				case DRUID:
//					if      ((!strcasecmp(sep->arg[2], "karana")) && (c->GetLevel() >= 25) ) {
//						Gater->Say("Casting Circle of Karana...");
//						Gater->CastSpell(550, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "commons")) && (c->GetLevel() >= 27)) {
//						Gater->Say("Casting Circle of Commons...");
//						Gater->CastSpell(551, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "tox")) && (c->GetLevel() >= 25)) { 
//						Gater->Say("Casting Circle of Toxxulia...");
//						Gater->CastSpell(552, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "butcher")) && (c->GetLevel() >= 25)) { 
//						Gater->Say("Casting Circle of Butcherblock...");
//						Gater->CastSpell(553, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "lava")) && (c->GetLevel() >= 30)) { 
//						Gater->Say("Casting Circle of Lavastorm...");
//						Gater->CastSpell(554, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "ro")) && (c->GetLevel() >= 32)) { 
//						Gater->Say("Casting Circle of Ro...");
//						Gater->CastSpell(555, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "feerrott")) && (c->GetLevel() >= 32)) { 
//						Gater->Say("Casting Circle of feerrott...");
//						Gater->CastSpell(556, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "steamfont")) && (c->GetLevel() >= 31)) { 
//						Gater->Say("Casting Circle of Steamfont...");
//						Gater->CastSpell(557, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "misty")) && (c->GetLevel() >= 36)) { 
//						Gater->Say("Casting Circle of Misty...");
//						Gater->CastSpell(558, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "wakening")) && (c->GetLevel() >= 40)) { 
//						Gater->Say("Casting Circle of Wakening Lands...");
//						Gater->CastSpell(1398, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "iceclad")) && (c->GetLevel() >= 32)) { 
//						Gater->Say("Casting Circle of Iceclad Ocean...");
//						Gater->CastSpell(1434, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "divide")) && (c->GetLevel() >= 36)) { 
//						Gater->Say("Casting Circle of The Great Divide...");
//						Gater->CastSpell(1438, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "cobalt")) && (c->GetLevel() >= 42)) { 
//						Gater->Say("Casting Circle of Cobalt Scar...");
//						Gater->CastSpell(1440, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "combines")) && (c->GetLevel() >= 33)) { 
//						Gater->Say("Casting Circle of The Combines...");
//						Gater->CastSpell(1517, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "surefall")) && (c->GetLevel() >= 26)) { 
//						Gater->Say("Casting Circle of Surefall Glade...");
//						Gater->CastSpell(2020, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "grimling")) && (c->GetLevel() >= 29)) { 
//						Gater->Say("Casting Circle of Grimling Forest...");
//						Gater->CastSpell(2419, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "twilight")) && (c->GetLevel() >= 33)) { 
//						Gater->Say("Casting Circle of Twilight...");
//						Gater->CastSpell(2424, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "dawnshroud")) && (c->GetLevel() >= 37)) { 
//						Gater->Say("Casting Circle of Dawnshroud...");
//						Gater->CastSpell(2429, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "nexus")) && (c->GetLevel() >= 26)) { 
//						Gater->Say("Casting Circle of The Nexus...");
//						Gater->CastSpell(2432, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "pok")) && (c->GetLevel() >= 38)) { 
//						Gater->Say("Casting Circle of Knowledge...");
//						Gater->CastSpell(3184, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "stonebrunt")) && (c->GetLevel() >= 28)) { 
//						Gater->Say("Casting Circle of Stonebrunt Mountains...");
//						Gater->CastSpell(3792, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "bloodfields")) && (c->GetLevel() >= 55)) { 
//						Gater->Say("Casting Circle of Bloodfields...");
//						Gater->CastSpell(6184, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "emerald")) && (c->GetLevel() >= 39)) { 
//						Gater->Say("Casting Wind of the South...");
//						Gater->CastSpell(1737, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "skyfire")) && (c->GetLevel() >= 44)) { 
//						Gater->Say("Casting Wind of the North...");
//						Gater->CastSpell(1736, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "slaughter")) && (c->GetLevel() >= 64)) { 
//						Gater->Say("Casting Circle of Slaughter...");
//						Gater->CastSpell(6179, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "karana") 
//						|| !strcasecmp(sep->arg[2], "tox") 
//						|| !strcasecmp(sep->arg[2], "butcher") && (c->GetLevel() <= 25))
//						|| !strcasecmp(sep->arg[2], "commons") && (c->GetLevel() <= 27)
//						|| (!strcasecmp(sep->arg[2], "ro") 
//						|| !strcasecmp(sep->arg[2], "feerrott") && (c->GetLevel() <= 32))
//						|| !strcasecmp(sep->arg[2], "steamfont") && (c->GetLevel() <= 31)
//						|| !strcasecmp(sep->arg[2], "misty") && (c->GetLevel() <= 36)
//						|| !strcasecmp(sep->arg[2], "lava") && (c->GetLevel() <= 30)
//						|| !strcasecmp(sep->arg[2], "wakening") && (c->GetLevel() <= 40)
//						|| !strcasecmp(sep->arg[2], "iceclad") && (c->GetLevel() <= 32)
//						|| !strcasecmp(sep->arg[2], "divide") && (c->GetLevel() <= 38)
//						|| !strcasecmp(sep->arg[2], "cobalt") && (c->GetLevel() <= 42)
//						|| !strcasecmp(sep->arg[2], "combines") && (c->GetLevel() <= 33)
//						|| !strcasecmp(sep->arg[2], "surefall") && (c->GetLevel() <= 26)
//						|| !strcasecmp(sep->arg[2], "grimling") && (c->GetLevel() <= 29)
//						|| !strcasecmp(sep->arg[2], "twilight") && (c->GetLevel() <= 33)
//						|| !strcasecmp(sep->arg[2], "dawnshroud") && (c->GetLevel() <= 37)
//						|| !strcasecmp(sep->arg[2], "nexus") && (c->GetLevel() <= 26)
//						|| !strcasecmp(sep->arg[2], "pok") && (c->GetLevel() <= 38)
//						|| !strcasecmp(sep->arg[2], "stonebrunt") && (c->GetLevel() <= 28)
//						|| !strcasecmp(sep->arg[2], "bloodfields") && (c->GetLevel() <= 55)
//						|| !strcasecmp(sep->arg[2], "emerald") && (c->GetLevel() <= 38)
//						|| !strcasecmp(sep->arg[2], "skyfire") && (c->GetLevel() <= 43)
//						|| !strcasecmp(sep->arg[2], "wos") && (c->GetLevel() <= 64)) {
//							Gater->Say("I don't have the needed level yet", sep->arg[2]);
//					}
//					else {
//						Gater->Say("With the proper level I can [gate] to [karana],[commons],[tox],[butcher],[lava],[ro],[feerrott],[steamfont],[misty],[wakening],[iceclad],[divide],[cobalt],[combines],[surefall],[grimling],[twilight],[dawnshroud],[nexus],[pok],[stonebrunt],[bloodfields],[emerald],[skyfire] or [wos].", c->GetName());
//					}
//					break;
//
//				case WIZARD:
//
//					if      ((!strcasecmp(sep->arg[2], "commons")) && (c->GetLevel() >= 35) ) {
//						Gater->Say("Casting Common Portal...");
//						Gater->CastSpell(566, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "fay")) && (c->GetLevel() >= 27)) {
//						Gater->Say("Casting Fay Portal...");
//						Gater->CastSpell(563, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "ro")) && (c->GetLevel() >= 37)) {
//						Gater->Say("Casting Ro Portal...");
//						Gater->CastSpell(567, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "tox")) && (c->GetLevel() >= 25)) {
//						Gater->Say("Casting Toxxula Portal...");
//						Gater->CastSpell(561, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "nk")) && (c->GetLevel() >= 27)) {
//						Gater->Say("Casting North Karana Portal...");
//						Gater->CastSpell(562, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "nek")) && (c->GetLevel() >= 32)) {
//						Gater->Say("Casting Nektulos Portal...");
//						Gater->CastSpell(564, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "wakening")) && (c->GetLevel() >= 43)) {
//						Gater->Say("Casting Wakening Lands Portal...");
//						Gater->CastSpell(1399, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "iceclad")) && (c->GetLevel() >= 33)) {
//						Gater->Say("Casting Iceclad Ocean Portal...");
//						Gater->CastSpell(1418, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "divide")) && (c->GetLevel() >= 36)) {
//						Gater->Say("Casting Great Divide Portal...");
//						Gater->CastSpell(1423, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "cobalt")) && (c->GetLevel() >= 43)) {
//						Gater->Say("Casting Cobalt Scar Portal...");
//						Gater->CastSpell(1425, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "combines")) && (c->GetLevel() >= 34)) {
//						Gater->Say("Casting Combines Portal...");
//						Gater->CastSpell(1516, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "wk")) && (c->GetLevel() >= 27)) {
//						Gater->Say("Casting West Karana Portal...");
//						Gater->CastSpell(568, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "twilight")) && (c->GetLevel() >= 27)) {
//						Gater->Say("Casting Twilight Portal...");
//						Gater->CastSpell(2425, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "dawnshroud")) && (c->GetLevel() >= 27)) {
//						Gater->Say("Casting Dawnshroud Portal...");
//						Gater->CastSpell(2430, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "nexus")) && (c->GetLevel() >= 29)) {
//						Gater->Say("Casting Nexus Portal...");
//						Gater->CastSpell(2944, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "pok")) && (c->GetLevel() >= 27)) {
//						Gater->Say("Casting Plane of Knowledge Portal...");
//						Gater->CastSpell(3180, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "wos")) && (c->GetLevel() >= 27)) {
//						Gater->Say("Casting Wall of Slaughter Portal...");
//						Gater->CastSpell(6178, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "grimling")) && (c->GetLevel() >= 29)) {
//						Gater->Say("Casting Fay Portal...");
//						Gater->CastSpell(2420, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "emerald")) && (c->GetLevel() >= 37)) {
//						Gater->Say("Porting to Emerald Jungle...");
//						Gater->CastSpell(1739, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "hateplane")) && (c->GetLevel() >= 39)) {
//						Gater->Say("Porting to Hate Plane...");
//						Gater->CastSpell(666, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "airplane")) && (c->GetLevel() >= 39)) {
//						Gater->Say("Porting to airplane...");
//						Gater->CastSpell(674, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "skyfire")) && (c->GetLevel() >= 36)) {
//						Gater->Say("Porting to Skyfire...");
//						Gater->CastSpell(1738, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "bloodfields")) && (c->GetLevel() >= 55)) {
//						Gater->Say("Casting Bloodfields Portal...");
//						Gater->CastSpell(6183, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "stonebrunt")) && (c->GetLevel() >= 27)) {
//						Gater->Say("Casting Stonebrunt Portal...");
//						Gater->CastSpell(3793, c->GetID(), 1, -1, -1);
//					}
//					else if ((!strcasecmp(sep->arg[2], "commons") && (c->GetLevel() <= 35))
//						|| !strcasecmp(sep->arg[2], "fay") && (c->GetLevel() <= 27)
//						|| (!strcasecmp(sep->arg[2], "ro") && (c->GetLevel() <= 37))
//						|| !strcasecmp(sep->arg[2], "tox") && (c->GetLevel() <= 25)
//						|| !strcasecmp(sep->arg[2], "nk") && (c->GetLevel() <= 25)
//						|| !strcasecmp(sep->arg[2], "nek") && (c->GetLevel() <= 32)
//						|| !strcasecmp(sep->arg[2], "wakening") && (c->GetLevel() <= 43)
//						|| !strcasecmp(sep->arg[2], "iceclad") && (c->GetLevel() <= 33)
//						|| !strcasecmp(sep->arg[2], "divide") && (c->GetLevel() <= 36)
//						|| !strcasecmp(sep->arg[2], "cobalt") && (c->GetLevel() <= 43)
//						|| !strcasecmp(sep->arg[2], "combines") && (c->GetLevel() <= 34)
//						|| !strcasecmp(sep->arg[2], "wk") && (c->GetLevel() <= 37)
//						|| !strcasecmp(sep->arg[2], "twilight") && (c->GetLevel() <= 33)
//						|| !strcasecmp(sep->arg[2], "dawnshroud") && (c->GetLevel() <= 39)
//						|| !strcasecmp(sep->arg[2], "nexus") && (c->GetLevel() <= 29)
//						|| (!strcasecmp(sep->arg[2], "pok")
//						|| !strcasecmp(sep->arg[2], "hateplane")
//						|| !strcasecmp(sep->arg[2], "airplane") && (c->GetLevel() <= 38))
//						|| !strcasecmp(sep->arg[2], "grimling") && (c->GetLevel() <= 29)
//						|| !strcasecmp(sep->arg[2], "bloodfields") && (c->GetLevel() <= 55)
//						|| !strcasecmp(sep->arg[2], "stonebrunt") && (c->GetLevel() <= 27)
//						|| !strcasecmp(sep->arg[2], "emerald") && (c->GetLevel() <= 36)
//						|| !strcasecmp(sep->arg[2], "skyfire") && (c->GetLevel() <= 36)
//						|| !strcasecmp(sep->arg[2], "wos") && (c->GetLevel() <= 64)) {
//							Gater->Say("I don't have the needed level yet", sep->arg[2]);
//					}
//					else {
//						Gater->Say("With the proper level I can [gate] to [commons],[fay],[ro],[tox],[nk],[wakening],[iceclad],[divide],[cobalt],[combines],[wk],[grimling],[twilight],[dawnshroud],[nexus],[pok],[stonebrunt],[bloodfields],[emerald],[skyfire],[hateplane],[airplane] or [wos].", c->GetName());
//					}
//					break;
//				default:
//					c->Message(15, "You must have a Druid or Wizard in your group.");
//					break;
//			}
//		}
//	}
//
//	//Endure Breath
//	if ((!strcasecmp(sep->arg[1], "endureb")) && (c->IsGrouped())) {
//		Mob *Endurer;
//		int32 EndurerClass = 0;
//		Group *g = c->GetGroup();
//		if(g) {
//			for(int i=0; i<MAX_GROUP_MEMBERS; i++){
//				if(g->members[i] && g->members[i]->IsBot()) {
//					switch(g->members[i]->GetClass()) {
//						case DRUID:
//							Endurer = g->members[i];
//							EndurerClass = DRUID;
//							break;
//						case SHAMAN:
//							if (EndurerClass != DRUID){
//								Endurer = g->members[i];
//								EndurerClass = SHAMAN;
//							}
//							break;
//						case ENCHANTER:
//							if(EndurerClass == 0){
//								Endurer = g->members[i];
//								EndurerClass = ENCHANTER;
//							}
//							break;
//						case RANGER:
//							if(EndurerClass == 0) {
//								Endurer = g->members[i];
//								EndurerClass = RANGER;
//							}
//							break;
//						default:
//							break;
//					}
//				}
//			}
//			switch(EndurerClass) {
//				case DRUID:
//
//					if  (c->GetLevel() <= 6) {
//						Endurer->Say("I'm not level 6 yet.");
//					}
//					else if (zone->CanCastOutdoor()) {
//						Endurer->Say("Casting Enduring Breath...");
//						Endurer->CastSpell(86, c->GetID(), 1, -1, -1);
//						break;
//					}
//					break;
//				case SHAMAN:
//
//					if ((zone->CanCastOutdoor()) && (c->GetLevel() >= 12)) { 
//						Endurer->Say("Casting Enduring Breath...");
//						Endurer->CastToClient()->CastSpell(86, c->GetID(), 1, -1, -1);
//					}
//					else if (c->GetLevel() <= 12) {
//						Endurer->Say("I'm not level 12 yet.");
//					}
//					break;
//				case RANGER:
//
//					if((zone->CanCastOutdoor()) && (c->GetLevel() >= 20)){
//						Endurer->Say("Casting Enduring Breath...");
//						Endurer->CastToClient()->CastSpell(86, c->GetID(), 1, -1, -1);
//					}
//					else if (c->GetLevel() <= 20) {
//						Endurer->Say("I'm not level 20 yet.");
//					}
//					break;
//				case ENCHANTER:
//
//					if((zone->CanCastOutdoor()) && (c->GetLevel() >= 12)) {
//						Endurer->Say("Casting Enduring Breath...");
//						Endurer->CastToClient()->CastSpell(86, c->GetID(), 1, -1, -1);
//					}
//					else if (c->GetLevel() <= 12) {
//						Endurer->Say("I'm not level 12 yet.");
//					}
//					break;
//				default:
//					c->Message(15, "You must have a Druid, Shaman, Ranger, or Enchanter in your group.");
//					break;
//			}
//		}
//	}
//
//	//Invisible
//	if ((!strcasecmp(sep->arg[1], "invis")) && (c->IsGrouped())) {
//		Mob *Inviser;
//		int32 InviserClass = 0;
//		Group *g = c->GetGroup();
//		if(g) {
//			for(int i=0; i<MAX_GROUP_MEMBERS; i++){
//				if(g->members[i] && g->members[i]->IsBot()) {
//					switch(g->members[i]->GetClass()) {
//						case ENCHANTER:
//							Inviser = g->members[i];
//							InviserClass = ENCHANTER;
//							break;
//						case MAGICIAN:
//							if (InviserClass != ENCHANTER){
//								Inviser = g->members[i];
//								InviserClass = MAGICIAN;
//							}
//							break;
//						case WIZARD:
//							if((InviserClass != ENCHANTER) || (InviserClass != MAGICIAN)){
//								Inviser = g->members[i];
//								InviserClass = WIZARD;
//							}
//							break;
//						case NECROMANCER:
//							if(InviserClass == 0){
//								Inviser = g->members[i];
//								InviserClass = NECROMANCER;
//							}
//							break;
//						case DRUID:
//							if((InviserClass != ENCHANTER) || (InviserClass != WIZARD)
//								|| (InviserClass != MAGICIAN)){
//									Inviser = g->members[i];
//									InviserClass = DRUID;
//							}
//							break;
//						default:
//							break;
//					}
//				}
//			}
//			switch(InviserClass) {
//				case ENCHANTER:
//					if  ((c->GetLevel() <= 14) && (!strcasecmp(sep->arg[2], "undead"))) {
//						Inviser->Say("I'm not level 14 yet.");
//					}
//					else if ((!c->IsInvisible(c)) && (!c->invisible_undead) && (c->GetLevel() >= 14) && (!strcasecmp(sep->arg[2], "undead"))) {
//						Inviser->Say("Casting invis undead...");
//						Inviser->CastSpell(235, c->GetID(), 1, -1, -1);
//					}
//					else if  ((c->GetLevel() <= 4) && (!strcasecmp(sep->arg[2], "live"))) {
//						Inviser->Say("I'm not level 4 yet.");
//					}
//					else if ((!c->IsInvisible(c))&& (!c->invisible_undead) && (c->GetLevel() >= 4) && (!strcasecmp(sep->arg[2], "live"))) { 
//						Inviser->Say("Casting invisibilty...");
//						Inviser->CastSpell(42, c->GetID(), 1, -1, -1);
//					}
//					else if  ((c->GetLevel() <= 6) && (!strcasecmp(sep->arg[2], "see"))) {
//						Inviser->Say("I'm not level 6 yet.");
//					}
//					else if ((c->GetLevel() >= 6) && (!strcasecmp(sep->arg[2], "see"))) { 
//						Inviser->Say("Casting see invisible...");
//						Inviser->CastSpell(80, c->GetID(), 1, -1, -1);
//					}
//					else if ((c->IsInvisible(c)) || (c->invisible_undead)) { 
//						Inviser->Say("I can't cast this if you're already invis-buffed...");
//					}
//					else {
//						Inviser->Say("Do you want [invis undead], [invis live] or [invis see] ?", c->GetName());
//					}
//					break;
//				case MAGICIAN:
//					if  (!strcasecmp(sep->arg[2], "undead")) {
//						Inviser->Say("I don't have that spell.");
//					}
//					else if  ((c->GetLevel() <= 8) && (!strcasecmp(sep->arg[2], "live"))) {
//						Inviser->Say("I'm not level 8 yet.");
//					}
//					else if ((!c->IsInvisible(c))&& (!c->invisible_undead) && (c->GetLevel() >= 8) && (!strcasecmp(sep->arg[2], "live"))) { 
//						Inviser->Say("Casting invisibilty...");
//						Inviser->CastSpell(42, c->GetID(), 1, -1, -1);
//					}
//					else if  ((c->GetLevel() <= 16) && (!strcasecmp(sep->arg[2], "see"))) {
//						Inviser->Say("I'm not level 16 yet.");
//					}
//					else if ((c->GetLevel() >= 16) && (!strcasecmp(sep->arg[2], "see"))) { 
//						Inviser->Say("Casting see invisible...");
//						Inviser->CastSpell(80, c->GetID(), 1, -1, -1);
//					}
//					else if ((c->IsInvisible(c)) || (c->invisible_undead)) { 
//						Inviser->Say("I can't cast this if you're already invis-buffed...");
//					}
//					else {
//						Inviser->Say("Do you want [invis live] or [invis see] ?", c->GetName());
//					}
//					break;
//				case WIZARD:
//					if  ((c->GetLevel() <= 39) && (!strcasecmp(sep->arg[2], "undead"))) {
//						Inviser->Say("I'm not level 39 yet.");
//					}
//					else if ((!c->IsInvisible(c))&& (!c->invisible_undead) && (c->GetLevel() >= 39) && (!strcasecmp(sep->arg[2], "undead"))) {
//						Inviser->Say("Casting invis undead...");
//						Inviser->CastSpell(235, c->GetID(), 1, -1, -1);
//					}
//					else if  ((c->GetLevel() <= 16) && (!strcasecmp(sep->arg[2], "live"))) {
//						Inviser->Say("I'm not level 16 yet.");
//					}
//					else if ((!c->IsInvisible(c))&& (!c->invisible_undead) && (c->GetLevel() >= 16) && (!strcasecmp(sep->arg[2], "live"))) { 
//						Inviser->Say("Casting invisibilty...");
//						Inviser->CastSpell(42, c->GetID(), 1, -1, -1);
//					}
//					else if  ((c->GetLevel() <= 4) && (!strcasecmp(sep->arg[2], "see"))) {
//						Inviser->Say("I'm not level 6 yet.");
//					}
//					else if ((c->GetLevel() >= 4) && (!strcasecmp(sep->arg[2], "see"))) { 
//						Inviser->Say("Casting see invisible...");
//						Inviser->CastSpell(80, c->GetID(), 1, -1, -1);
//					}
//					else if ((c->IsInvisible(c)) || (c->invisible_undead)) { 
//						Inviser->Say("I can't cast this if you're already invis-buffed...");
//					}
//					else {
//						Inviser->Say("Do you want [invis undead], [invis live] or [invis see] ?", c->GetName());
//					}
//					break;
//				case NECROMANCER:
//					if ((!c->IsInvisible(c))&& (!c->invisible_undead) && (!strcasecmp(sep->arg[2], "undead"))) {
//						Inviser->Say("Casting invis undead...");
//						Inviser->CastSpell(235, c->GetID(), 1, -1, -1);
//					}
//					else if (!strcasecmp(sep->arg[2], "see")) { 
//						Inviser->Say("I don't have that spell...");
//					}
//					else if (!strcasecmp(sep->arg[2], "live")) { 
//						Inviser->Say("I don't have that spell...");
//					}
//					else if ((c->IsInvisible(c))|| (c->invisible_undead)) { 
//						Inviser->Say("I can't cast this if you're already invis-buffed...");
//					}
//					else {
//						Inviser->Say("I only have [invis undead]", c->GetName());
//					}
//					break;
//				case DRUID:
//					if  (!strcasecmp(sep->arg[2], "undead")) {
//						Inviser->Say("I don't have that spell...");
//					}
//					else if  ((c->GetLevel() <= 4) && (!strcasecmp(sep->arg[2], "live"))) {
//						Inviser->Say("I'm not level 4 yet.");
//					}
//					else if ((!c->IsInvisible(c))&& (!c->invisible_undead) && (c->GetLevel() >= 18) && (!strcasecmp(sep->arg[2], "live"))) { 
//						Inviser->Say("Casting Superior Camouflage...");
//						Inviser->CastSpell(34, c->GetID(), 1, -1, -1);
//					}
//					else if ((!c->IsInvisible(c))&& (!c->invisible_undead) && (c->GetLevel() >= 4) && (!strcasecmp(sep->arg[2], "live")) && (zone->CanCastOutdoor())) { 
//						Inviser->Say("Casting Camouflage...");
//						Inviser->CastSpell(247, c->GetID(), 1, -1, -1);
//					}
//					else if ((c->GetLevel() >= 4) && (!strcasecmp(sep->arg[2], "live")) && (!zone->CanCastOutdoor())) { 
//						Inviser->Say("I can't cast this spell indoors...");
//					}
//					else if  ((c->GetLevel() <= 13) && (!strcasecmp(sep->arg[2], "see"))) {
//						Inviser->Say("I'm not level 13 yet.");
//					}
//					else if ((c->GetLevel() >= 13) && (!strcasecmp(sep->arg[2], "see"))) { 
//						Inviser->Say("Casting see invisible...");
//						Inviser->CastSpell(80, c->GetID(), 1, -1, -1);
//					}
//					else if ((c->IsInvisible(c)) || (c->invisible_undead)) { 
//						Inviser->Say("I can't cast this if you're already invis-buffed...");
//					}
//					else {
//						Inviser->Say("Do you want [invis live] or [invis see] ?", c->GetName());
//					}
//					break;
//				default:
//					c->Message(15, "You must have a Enchanter, Magician, Wizard, Druid, or Necromancer in your group.");
//					break;
//			}
//		}
//	}
//
//	//Levitate
//	if ((!strcasecmp(sep->arg[1], "levitate")) && (c->IsGrouped())) {
//		Mob *Lever;
//		int32 LeverClass = 0;
//		Group *g = c->GetGroup();
//		if(g) {
//			for(int i=0; i<MAX_GROUP_MEMBERS; i++){
//				if(g->members[i] && g->members[i]->IsBot()) {
//					switch(g->members[i]->GetClass()) {
//						case DRUID:
//							Lever = g->members[i];
//							LeverClass = DRUID;
//							break;
//						case SHAMAN:
//							if (LeverClass != DRUID){
//								Lever = g->members[i];
//								LeverClass = SHAMAN;
//							}
//							break;
//						case WIZARD:
//							if(LeverClass == 0){
//								Lever = g->members[i];
//								LeverClass = WIZARD;
//							}
//							break;
//						case ENCHANTER:
//							if (LeverClass == 0) {
//								Lever = g->members[i];
//								LeverClass = ENCHANTER;
//							}
//							break;
//						default:
//							break;
//					}
//				}
//			}
//			switch(LeverClass) {
//				case DRUID:
//					if  (c->GetLevel() <= 14) {
//						Lever->Say("I'm not level 14 yet.");
//					}
//					else if (zone->CanCastOutdoor()) {
//						Lever->Say("Casting Levitate...");
//						Lever->CastSpell(261, c->GetID(), 1, -1, -1);
//						break;
//					}
//					else if (!zone->CanCastOutdoor()) {
//						Lever->Say("I can't cast this spell indoors", c->GetName());
//					}
//					break;
//
//				case SHAMAN:
//
//					if ((zone->CanCastOutdoor()) && (c->GetLevel() >= 10)) { 
//						Lever->Say("Casting Levitate...");
//						Lever->CastToClient()->CastSpell(261, c->GetID(), 1, -1, -1);
//					}
//					else if (!zone->CanCastOutdoor()) {
//						Lever->Say("I can't cast this spell indoors", c->GetName());
//					}
//					else if (c->GetLevel() <= 10) {
//						Lever->Say("I'm not level 10 yet.");
//					}
//					break;
//
//				case WIZARD:
//
//					if((zone->CanCastOutdoor()) && (c->GetLevel() >= 22)){
//						Lever->Say("Casting Levitate...");
//						Lever->CastToClient()->CastSpell(261, c->GetID(), 1, -1, -1);
//					}
//					else if (!zone->CanCastOutdoor()) {
//						Lever->Say("I can't cast this spell indoors", c->GetName());
//					}
//					else if (c->GetLevel() <= 22) {
//						Lever->Say("I'm not level 22 yet.");
//					}
//					break;
//
//				case ENCHANTER:
//
//					if((zone->CanCastOutdoor()) && (c->GetLevel() >= 15)) {
//						Lever->Say("Casting Levitate...");
//						Lever->CastToClient()->CastSpell(261, c->GetID(), 1, -1, -1);
//					}
//					else if (!zone->CanCastOutdoor()) {
//						Lever->Say("I can't cast this spell indoors", c->GetName());
//					}
//					else if (c->GetLevel() <= 15) {
//						Lever->Say("I'm not level 15 yet.");
//					}
//					break;
//
//
//				default:
//					c->Message(15, "You must have a Druid, Shaman, Wizard, or Enchanter in your group.");
//					break;
//			}
//		}
//	}
//
//	//Resists
//	if ((!strcasecmp(sep->arg[1], "resist")) && (c->IsGrouped())) {
//		Mob *Resister;
//		int32 ResisterClass = 0;
//		Group *g = c->GetGroup();
//		if(g) {
//			for(int i=0; i<MAX_GROUP_MEMBERS; i++){
//				if(g->members[i] && g->members[i]->IsBot()) {
//					switch(g->members[i]->GetClass()) {
//						case CLERIC:
//							Resister = g->members[i];
//							ResisterClass = CLERIC;
//							break;
//						case SHAMAN:
//							if(ResisterClass != CLERIC){
//								Resister = g->members[i];
//								ResisterClass = SHAMAN;
//							}
//						case DRUID:
//							if (ResisterClass == 0){
//								Resister = g->members[i];
//								ResisterClass = DRUID;
//							}
//							break;
//							break;
//						default:
//							break;
//					}
//				}
//			}
//			switch(ResisterClass) {
//				case CLERIC:
//					if	(!strcasecmp(sep->arg[2], "poison") && (c->GetLevel() >= 6))  {
//						Resister->Say("Casting poison protection...", sep->arg[2]);
//						Resister->CastToNPC()->Bot_Command_Resist(1, Resister->GetLevel());
//					}
//					else if (!strcasecmp(sep->arg[2], "disease") && (c->GetLevel() >= 11)) {
//						Resister->Say("Casting disease protection...", sep->arg[2]);
//						Resister->CastToNPC()->Bot_Command_Resist(2, Resister->GetLevel());
//					}
//					else if(!strcasecmp(sep->arg[2], "fire") && (c->GetLevel() >= 8)) {
//						Resister->Say("Casting fire protection...", sep->arg[2]);
//						Resister->CastToNPC()->Bot_Command_Resist(3, Resister->GetLevel());
//					}
//					else if(!strcasecmp(sep->arg[2], "cold") && (c->GetLevel() >= 13)) {
//						Resister->Say("Casting cold protection...", sep->arg[2]);
//						Resister->CastToNPC()->Bot_Command_Resist(4, Resister->GetLevel());
//					}
//					else if(!strcasecmp(sep->arg[2], "magic") && (c->GetLevel() >= 16)) {
//						Resister->Say("Casting magic protection...", sep->arg[2]);
//						Resister->CastToNPC()->Bot_Command_Resist(5, Resister->GetLevel());
//					}
//					else if (!strcasecmp(sep->arg[2], "magic") && (c->GetLevel() <= 16)
//						|| !strcasecmp(sep->arg[2], "cold") && (c->GetLevel() <= 13)
//						|| !strcasecmp(sep->arg[2], "fire") && (c->GetLevel() <= 8) 
//						|| !strcasecmp(sep->arg[2], "disease") && (c->GetLevel() <= 11)
//						|| !strcasecmp(sep->arg[2], "poison") && (c->GetLevel() <= 6)) {
//							Resister->Say("I don't have the needed level yet", sep->arg[2]);
//					}
//					else 
//						Resister->Say("Do you want [resist poison], [resist disease], [resist fire], [resist cold], or [resist magic]?", c->GetName());
//
//					break;
//
//				case SHAMAN:
//					if	(!strcasecmp(sep->arg[2], "poison") && (c->GetLevel() >= 20))  {
//						Resister->Say("Casting poison protection...", sep->arg[2]);
//						Resister->CastToNPC()->Bot_Command_Resist(12, Resister->GetLevel());
//					}
//					else if (!strcasecmp(sep->arg[2], "disease") && (c->GetLevel() >= 8)) {
//						Resister->Say("Casting disease protection...", sep->arg[2]);
//						Resister->CastToNPC()->Bot_Command_Resist(13, Resister->GetLevel());
//					}
//					else if(!strcasecmp(sep->arg[2], "fire") && (c->GetLevel() >= 5)) {
//						Resister->Say("Casting fire protection...", sep->arg[2]);
//						Resister->CastToNPC()->Bot_Command_Resist(14, Resister->GetLevel());
//					}
//					else if(!strcasecmp(sep->arg[2], "cold") && (c->GetLevel() >= 1)) {
//						Resister->Say("Casting cold protection...", sep->arg[2]);
//						Resister->CastToNPC()->Bot_Command_Resist(15, Resister->GetLevel());
//					}
//					else if(!strcasecmp(sep->arg[2], "magic") && (c->GetLevel() >= 19)) {
//						Resister->Say("Casting magic protection...", sep->arg[2]);
//						Resister->CastToNPC()->Bot_Command_Resist(16, Resister->GetLevel());
//					}
//					else if (!strcasecmp(sep->arg[2], "magic") && (c->GetLevel() <= 19)
//						|| !strcasecmp(sep->arg[2], "cold") && (c->GetLevel() <= 1)
//						|| !strcasecmp(sep->arg[2], "fire") && (c->GetLevel() <= 5) 
//						|| !strcasecmp(sep->arg[2], "disease") && (c->GetLevel() <= 8)
//						|| !strcasecmp(sep->arg[2], "poison") && (c->GetLevel() <= 20)) {
//							Resister->Say("I don't have the needed level yet", sep->arg[2]);
//					}
//					else 
//						Resister->Say("Do you want [resist poison], [resist disease], [resist fire], [resist cold], or [resist magic]?", c->GetName());
//
//					break;
//
//				case DRUID:
//
//					if	(!strcasecmp(sep->arg[2], "poison") && (c->GetLevel() >= 19)) {
//						Resister->Say("Casting poison protection...", sep->arg[2]);
//						Resister->CastToNPC()->Bot_Command_Resist(7, Resister->GetLevel());
//					}
//					else if (!strcasecmp(sep->arg[2], "disease") && (c->GetLevel() >= 19)) {
//						Resister->Say("Casting disease protection...", sep->arg[2]);
//						Resister->CastToNPC()->Bot_Command_Resist(8, Resister->GetLevel());
//					}
//					else if(!strcasecmp(sep->arg[2], "fire")) { // Fire level 1
//						Resister->Say("Casting fire protection...", sep->arg[2]);
//						Resister->CastToNPC()->Bot_Command_Resist(9, Resister->GetLevel());
//					}
//					else if(!strcasecmp(sep->arg[2], "cold") && (c->GetLevel() >= 13)) {
//						Resister->Say("Casting cold protection...", sep->arg[2]);
//						Resister->CastToNPC()->Bot_Command_Resist(10, Resister->GetLevel());
//					}
//					else if(!strcasecmp(sep->arg[2], "magic") && (c->GetLevel() >= 16)) {
//						Resister->Say("Casting magic protection...", sep->arg[2]);
//						Resister->CastToNPC()->Bot_Command_Resist(11, Resister->GetLevel());
//					}
//					else if (!strcasecmp(sep->arg[2], "magic") && (c->GetLevel() <= 16)
//						|| !strcasecmp(sep->arg[2], "cold") && (c->GetLevel() <= 9)
//						|| !strcasecmp(sep->arg[2], "disease") && (c->GetLevel() <= 19)
//						|| !strcasecmp(sep->arg[2], "poison") && (c->GetLevel() <= 19)) {
//							Resister->Say("I don't have the needed level yet", sep->arg[2]) ;
//					}
//					else 
//						Resister->Say("Do you want [resist poison], [resist disease], [resist fire], [resist cold], or [resist magic]?", c->GetName());
//
//					break;
//
//				default:
//					c->Message(15, "You must have a Cleric, Shaman, or Druid in your group.");
//					break;
//			}
//		}
//	}
//	// debug commands
//	if(!strcasecmp(sep->arg[1], "debug") && !strcasecmp(sep->arg[2], "inventory")) {
//		Mob *target = c->GetTarget();
//		if(target && target->IsBot())
//		{
//			for(int i=0; i<9; i++)
//			{
//				c->Message(15,"Equiped slot: %i , item: %i \n", i, target->CastToNPC()->GetEquipment(i));
//			}
//			if(target->CastToNPC()->GetEquipment(8) > 0)
//				c->Message(15,"This bot has an item in off-hand.");
//		}
//		return;
//	}
//
//	if(!strcasecmp(sep->arg[1], "debug") && !strcasecmp(sep->arg[2], "botcaracs"))
//	{
//		Mob *target = c->GetTarget();
//		if(target && target->IsBot())
//		{
//			if(target->CanThisClassDualWield())
//				c->Message(15, "This class can dual wield.");
//			if(target->CanThisClassDoubleAttack())
//				c->Message(15, "This class can double attack.");
//		}
//		if(target->GetPetID())
//			c->Message(15, "I've a pet and its name is %s", target->GetPet()->GetName() );
//		return;
//	}
//
//	if(!strcasecmp(sep->arg[1], "debug") && !strcasecmp(sep->arg[2], "spells"))
//	{
//		Mob *target = c->GetTarget();
//		if(target && target->IsBot())
//		{
//			for(int i=0; i<16; i++)
//			{
//				if(target->CastToNPC()->BotGetSpells(i) != 0)
//				{
//					SPDat_Spell_Struct botspell = spells[target->CastToNPC()->BotGetSpells(i)];
//					c->Message(15, "(DEBUG) %s , Slot(%i), Spell (%s) Priority (%i) \n", target->GetName(), i, botspell.name, target->CastToNPC()->BotGetSpellPriority(i));
//				}
//			}
//		}
//		return;
//	}
//
//	// EQoffline - Raids
//	if(!strcasecmp(sep->arg[1], "raid") && !strcasecmp(sep->arg[2], "help"))
//	{
//		c->Message(15, "#bot raid help - will show this help");
//		c->Message(15, "#bot raid info - will give info of your raid.");
//		c->Message(15, "#bot raid create - will create your raid (you will be the raid leader)");
//		c->Message(15, "#bot raid group create - create a group. Your target will be the leader.");
//		c->Message(15, "#bot raid invite bot [group leader's name] - Invite your target into that group leader's group.");	
//		c->Message(15, "#bot raid disband - Disband the raid.");
//		c->Message(15, "#bot raid order maintank - Your target will be flagged as the main tank.");
//		c->Message(15, "#bot raid order secondtank - Your target will be flagged as the second tank.");
//		c->Message(15, "#bot raid order maintarget - Your target will be flagged as the main raid's target.");
//		c->Message(15, "#bot raid order secondtarget - Your target will be flagged as the second raid's target.");
//		c->Message(15, "#bot raid order grouptarget [group leader's name] - Your target will be flagged as the target of a specific group.");
//		c->Message(15, "#bot raid order task [attack/guard] [group leader's name] - You will give a specific task [attack/guard].");
//		c->Message(15, "#bot raid order task [follow/assist] [group1 leader's name] [group2 leader's name] - Group 1 will [follow/assist] Group 2.");
//		c->Message(15, "#bot raid order task enraged - Tell your raid to stop attacking to defend against ENRAGED mobs.");
//		return;
//	}
//
//	if(!strcasecmp(sep->arg[1], "raid"))
//	{
//
//		if(!strcasecmp(sep->arg[2], "info"))
//		{
//			if(c->CastToMob()->IsBotRaiding())
//			{
//				BotRaids *br = entity_list.GetBotRaidByMob(c->CastToMob());
//				if(br) { 
//					br->BotRaidInfo(c);
//				}
//			}
//			else {
//				c->Message(15, "You are not in a bot raid.");
//			}
//			return;
//		}
//
//		else if(!strcasecmp(sep->arg[2], "create"))
//		{
//			if(c->IsBotRaiding())
//			{
//				c->Message(15, "You are already in a raid!");
//				return;
//			}
//			if(!c->IsGrouped() || ( c->IsGrouped() &&  entity_list.GetGroupByMob(c)->BotGroupCount() < 6))
//			{
//				c->Message(15, "You must be grouped and have a full group to create a raid.");
//				return;
//			}
//			else {
//				BotRaids *br = new BotRaids(c->CastToMob());
//				if(br) {
//					Group *g = c->GetGroup();
//					for(int i=0; i<MAX_GROUP_MEMBERS; i++) {
//						if(g->members[i]) {
//							g->members[i]->SetBotRaidID(br->GetBotRaidID());
//							g->members[i]->SetBotRaiding(true);
//							if(g->members[i]->GetPetID())
//							{
//								g->members[i]->GetPet()->SetBotRaidID(br->GetBotRaidID());
//								g->members[i]->GetPet()->SetBotRaiding(true);
//							}
//						}
//					}
//					br->AddBotGroup(g);
//				}
//			}
//			return;
//		}
//
//		else if(!strcasecmp(sep->arg[2], "group") && !strcasecmp(sep->arg[3], "create"))
//		{
//			if((c->GetTarget() == NULL) || !c->GetTarget()->IsBot() || c->GetFeigned()) {
//				return;
//			}
//
//			if(c->IsBotRaiding()) {
//				BotRaids *br = entity_list.GetBotRaidByMob(c->CastToMob());
//				if(br) {
//					if(br->GetBotRaidAggro()) {
//						c->Message(15, "You can't create bot groups while you are engaged.");
//						return;
//					}
//				}
//			}
//			else if(c->IsGrouped())
//			{
//				Group *g = entity_list.GetGroupByClient(c);
//				for (int i=0; i<MAX_GROUP_MEMBERS; i++)
//				{
//					if(g && g->members[i] && g->members[i]->IsEngaged())
//					{
//						c->Message(15, "You can't create bot groups while you are engaged.");
//						return;
//					}
//				}
//			}
//
//			if(!c->CastToMob()->IsBotRaiding() && c->GetTarget()->IsBot())
//			{
//				c->Message(15, "You must have created your raid and your group must be full before doing that!");
//				Mob* kmob = c->GetTarget();
//				if(kmob != NULL) {
//					kmob->BotOwner = NULL;
//					kmob->Kill();
//				}
//				return;
//			}
//
//			if(database.GetBotOwner(c->GetTarget()->GetNPCTypeID()) != c->AccountID())
//			{
//				c->GetTarget()->Say("I can't be your bot, you are not my owner.");
//				return;
//			}
//
//			if((c->GetTarget() != NULL) && !c->GetTarget()->IsGrouped() && c->GetTarget()->IsBot())
//			{
//				BotRaids *br = entity_list.GetBotRaidByMob(c->CastToMob());
//				if(br && (br->RaidBotGroupsCount() >= MAX_BOT_RAID_GROUPS)) {
//					Mob *kmob = c->GetTarget();
//					if(kmob != NULL) {
//						kmob->BotOwner = NULL;
//						kmob->Kill();
//					}
//					return;
//				}
//
//				Mob *gleader = c->GetTarget();
//				gleader->SetFollowID(c->GetID());
//				gleader->BotOwner = c->CastToMob();
//				gleader->SetOwnerID(0);
//				gleader->SetBotRaidID(br->GetBotRaidID());
//				gleader->SetBotRaiding(true);
//
//				Group *g = new Group(gleader);
//				entity_list.AddGroup(g);
//				br->AddBotGroup(g);
//
//				// load up leaders gear
//				uint32 itemID = 0;
//				const Item_Struct* item2 = NULL;
//				for(int i=0; i<22; i++) {
//					itemID = database.GetBotItemBySlot(gleader->GetNPCTypeID(), i);
//					if(itemID != 0) {
//						item2 = database.GetItem(itemID);
//						c->BotTradeAddItem(itemID, item2->MaxCharges, item2->Slots, i, gleader->CastToNPC(), false);
//					}
//				}
//				gleader->CalcBotStats();
//				c->Message(15, "-- RAID -- Group Leader is: %s\n", gleader->GetName());
//			}
//			else {
//				c->Message(15, "You must target your bot first.");
//			}
//			return;
//		}
//
//		else if(!strcasecmp(sep->arg[2], "invite") && !strcasecmp(sep->arg[3], "bot"))
//		{
//			if(c->GetFeigned()) {
//				c->Message(15, "You cannot create raid groups while feigned.");
//			}
//
//			if((c->GetTarget() == NULL) || !c->GetTarget()->IsBot()) {
//				c->Message(15, "You must target a bot first.");
//				return;
//			}
//
//			if(database.GetBotOwner(c->GetTarget()->GetNPCTypeID()) != c->AccountID())
//			{
//				c->GetTarget()->Say("I can not join your bot raid, you are not my owner.");
//				return;
//			}
//
//			Mob* sictar = entity_list.GetMob(sep->argplus[4]);
//			if(!sictar || !sictar->IsGrouped() || entity_list.GetGroupByMob(sictar) == NULL || entity_list.GetGroupByMob(sictar)->GetLeader() != sictar)
//			{
//				c->Message(15, "You didn't type the correct group leader name.");
//				Mob* kmob = c->GetTarget();
//				if(kmob != NULL) {
//					kmob->BotOwner = NULL;
//					kmob->Kill();
//				}
//				return;
//			}
//
//			if(c->GetTarget()->IsGrouped()) {
//				c->Message(15, "You must target an ungrouped bot first.");
//				Mob* kmob = c->GetTarget();
//				if(kmob != NULL) {
//					kmob->BotOwner = NULL;
//					kmob->Kill();
//				}
//				return;
//			}                
//			else {
//				Mob *inv = c->GetTarget();
//
//				Group *g = entity_list.GetGroupByMob(sictar);
//				if(g && (g->BotGroupCount() > 5)) {
//					inv->Say("I can't get into the group, it's full already.");
//					inv->BotOwner = NULL;
//					inv->Kill();
//					return;
//				}
//				if(g && (g->BotGroupCount() < 6))
//				{
//					inv->SetFollowID(sictar->GetID());
//					inv->BotOwner = c->CastToMob();
//					inv->SetOwnerID(0);
//					g->AddMember(inv);
//					inv->SetBotRaiding(true);
//					inv->SetBotRaidID(sictar->GetBotRaidID());
//
//					// Equip newly raid grouped bot
//					uint32 itemID = 0;
//					const Item_Struct* item2 = NULL;
//					for(int i=0; i<22; i++) {
//						itemID = database.GetBotItemBySlot(inv->GetNPCTypeID(), i);
//						if(itemID != 0) {
//							item2 = database.GetItem(itemID);
//							c->BotTradeAddItem(itemID, item2->MaxCharges, item2->Slots, i, inv->CastToNPC(), false);
//						}
//					}
//					inv->CalcBotStats();
//					inv->Say("I have joined %s's raid group.", g->GetLeader()->GetName());
//				}
//				else
//					inv->Say("I can't join the group (You didn't enter the group leader's name or the group is full already. Type #bot raid info\n");
//			}
//			return;
//		}
//		else if(!strcasecmp(sep->arg[2], "disband"))
//		{
//			if(c->IsBotRaiding()) {
//				database.CleanBotLeader(c->CharacterID());
//				BotRaids *brd = entity_list.GetBotRaidByMob(c->CastToMob());
//				if(brd) {
//					brd->RemoveRaidBots();
//					brd = NULL;
//				}
//				Group *g = entity_list.GetGroupByMob(c->CastToMob());
//				if(g) {
//					bool hasBots = false;
//					for(int i=5; i>=0; i--) {
//						if(g->members[i] && g->members[i]->IsBot()) {
//							hasBots = true;
//							g->members[i]->BotOwner = NULL;
//							g->members[i]->Kill();
//						}
//					}
//					if(hasBots) {
//						hasBots = false;
//						if(g->BotGroupCount() <= 1) {
//							g->DisbandGroup();
//						}
//					}
//				}
//				c->Message(15, "Raid disbanded.");
//			}
//			c->SetBotRaidID(0);
//			c->SetBotRaiding(false);
//			database.CleanBotLeader(c->CharacterID());
//			return;
//		}
//
//		else if(!strcasecmp(sep->arg[2], "order"))
//		{
//			if(!strcasecmp(sep->arg[3], "maintank"))
//			{
//				if(c->GetTarget() == NULL)
//					return;
//				else {
//					BotRaids *brc = entity_list.GetBotRaidByMob(c->CastToMob());
//					BotRaids *brm = entity_list.GetBotRaidByMob(c->GetTarget());
//					if(brc == NULL || brm == NULL || brc != brm)
//						return;
//					else {
//						brc->SetBotMainTank(c->GetTarget());
//						c->GetTarget()->Say("I am the Raid Primary Tank, /assist me!");
//					}
//				}
//			}
//			else if(!strcasecmp(sep->arg[3], "secondtank"))
//			{
//				if(c->GetTarget() == NULL)
//					return;
//				else {
//					BotRaids *brc = entity_list.GetBotRaidByMob(c->CastToMob());
//					BotRaids *brm = entity_list.GetBotRaidByMob(c->GetTarget());
//					if(brc == NULL || brm == NULL || brc != brm)
//						return;
//					else {
//						brc->SetBotSecondTank(c->GetTarget());
//						c->GetTarget()->Say("I am the Raid Secondary Tank, /assist me if the Primary Tank dies!");
//					}
//				}
//			}
//			else if(!strcasecmp(sep->arg[3], "maintarget"))
//			{
//				if(entity_list.GetBotRaidByMob(c->CastToMob()) == NULL)
//					return;
//				else{
//					BotRaids *brc = entity_list.GetBotRaidByMob(c->CastToMob());
//					if(!brc || !c->GetTarget() || !c->IsAttackAllowed(c->GetTarget()))
//					{
//						if(brc)
//							brc->SetBotMainTarget(NULL);
//						return;
//					}
//					else
//					{
//						brc->SetBotMainTarget(c->GetTarget());
//						c->Message(15, "Main target is %s", c->GetTarget()->GetCleanName());
//					}
//				}
//			}
//			else if(!strcasecmp(sep->arg[3], "secondtarget"))
//			{
//				if(entity_list.GetBotRaidByMob(c->CastToMob()) == NULL)
//					return;
//				else {
//					BotRaids *brc = entity_list.GetBotRaidByMob(c->CastToMob());
//					if(c->GetTarget() == NULL || !c->GetTarget()->IsAttackAllowed(c) || brc == NULL)
//						return;
//					else {
//						brc->SetBotSecondTarget(c->GetTarget());
//					}
//				}
//			}
//			else if(!strcasecmp(sep->arg[3], "grouptarget"))
//			{
//				if(entity_list.GetBotRaidByMob(c->CastToMob()) == NULL)
//					return;
//				else {
//					BotRaids *brc = entity_list.GetBotRaidByMob(c->CastToMob());
//					if(c->GetTarget() == NULL ||c->GetTarget()->IsBot())
//					{
//						c->Message(15, "You don't have a target or your target is a bot.");
//						return;
//					}
//					if(brc) {
//						brc->SetBotGroupTarget(c->GetTarget(), entity_list.GetGroupByLeaderName(sep->arg[4]));
//					}
//				}
//			}
//			else if(!strcasecmp(sep->arg[3], "task"))
//			{
//				BotRaids *brc = entity_list.GetBotRaidByMob(c->CastToMob());
//				if(brc) {
//					if(!strcasecmp(sep->arg[4], "attack"))
//					{
//						Mob *ctarget = c->GetTarget();
//						if(ctarget != NULL) {
//							c->SetOrderBotAttack(true);
//							brc->GroupAssignTask(entity_list.GetGroupByLeaderName(sep->arg[5]), 2, ctarget);
//							c->SetOrderBotAttack(false);
//						}
//						else {
//							c->Message(15, "You must target a monster.");
//						}
//					}
//					else if(!strcasecmp(sep->arg[4], "guard"))
//					{
//						Mob *ctarget = NULL;
//						brc->GroupAssignTask(entity_list.GetGroupByLeaderName(sep->arg[5]), 4, ctarget);
//					}
//					else if(!strcasecmp(sep->arg[4], "assist"))
//					{
//						brc->GroupAssignTask(entity_list.GetGroupByLeaderName(sep->arg[5]), 3, entity_list.GetGroupByLeaderName(sep->arg[6]));
//					}				
//					else if(!strcasecmp(sep->arg[4], "follow"))
//					{
//						brc->GroupAssignTask(entity_list.GetGroupByLeaderName(sep->arg[5]), 1, entity_list.GetGroupByLeaderName(sep->arg[6]));
//					}
//					else if(!strcasecmp(sep->arg[4], "enraged")) {
//						brc->RaidDefendEnraged();
//					}
//				}
//			}
//		}
//		return;
//	}
}

#endif