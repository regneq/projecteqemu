#ifdef BOTS

#include "bot.h"

// TODO: The following declarations are redudant to declarations made in MobAI.cpp. Best move both blocks to a common header file.
const int Z_AGGRO=10;

const int MobAISpellRange=100; // max range of buffs
const int SpellType_Nuke=1;
const int SpellType_Heal=2;
const int SpellType_Root=4;
const int SpellType_Buff=8;
const int SpellType_Escape=16;
const int SpellType_Pet=32;
const int SpellType_Lifetap=64;
const int SpellType_Snare=128;
const int SpellType_DOT=256;
const int SpellType_Dispel=512;

const int SpellTypes_Detrimental = SpellType_Nuke|SpellType_Root|SpellType_Lifetap|SpellType_Snare|SpellType_DOT|SpellType_Dispel;
const int SpellTypes_Beneficial = SpellType_Heal|SpellType_Buff|SpellType_Escape|SpellType_Pet;

#define SpellType_Any		0xFFFF
#ifdef _EQDEBUG
	#define MobAI_DEBUG_Spells	-1
#else
	#define MobAI_DEBUG_Spells	-1
#endif

// This constructor is used during the bot create command
Bot::Bot(NPCType npcTypeData, Client* botOwner) : NPC(&npcTypeData, 0, 0, 0, 0, 0, 0, false) {
	if(botOwner) {
		this->_botOwnerCharacterID = botOwner->CharacterID();
	}
	else
		this->_botOwnerCharacterID = 0;

	SetBotID(0);
	SetBotSpellID(0);
	SetBotRaiding(false);
	SetSpawnStatus(false);

	GenerateBaseStats();
	GenerateAppearance();
	GenerateArmorClass();

	// Calculate HitPoints Last As It Uses Base Stats
	GenerateBaseHitPoints();

	strcpy(this->name, this->GetCleanName());
}

// This constructor is used when the bot is loaded out of the database
Bot::Bot(uint32 botID, uint32 botOwnerCharacterID, uint32 botSpellsID, NPCType npcTypeData) : NPC(&npcTypeData, 0, 0, 0, 0, 0, 0, false) {
	this->_botOwnerCharacterID = botOwnerCharacterID;

	SetBotID(botID);
	SetBotSpellID(botSpellsID);
	SetBotRaiding(false);
	SetSpawnStatus(false);
	
	GenerateBaseStats();
	GenerateAppearance();
	GenerateArmorClass();

	// Calculate HitPoints Last As It Uses Base Stats
	GenerateBaseHitPoints();

	strcpy(this->name, this->GetCleanName());
}

Bot::~Bot() {
	// TODO: dependancy resource deallocation goes here (if any!)
}

void Bot::SetBotID(uint32 botID) {
	this->_botID = botID;
	this->npctype_id = botID;
}

void Bot::SetBotSpellID(uint32 newSpellID) {
	this->_botSpellID = newSpellID;
	this->npc_spells_id = newSpellID;
}

NPCType Bot::FillNPCTypeStruct(std::string botName, std::string botLastName, uint8 botLevel, uint16 botRace, uint8 botClass, uint8 botBodyType, sint32 hitPoints, uint8 gender, float size, uint32 hitPointsRegenRate, uint32 manaRegenRate, uint32 face, uint32 hairStyle, uint32 hairColor, uint32 eyeColor, uint32 eyeColor2, uint32 beardColor, uint32 beard, uint32 drakkinHeritage, uint32 drakkinTattoo, uint32 drakkinDetails, float runSpeed, sint16 mr, sint16 cr, sint16 dr, sint16 fr, sint16 pr, sint16 ac, uint16 str, uint16 sta, uint16 dex, uint16 agi, uint16 _int, uint16 wis, uint16 cha, uint16 attack) {
	NPCType BotNPCType;
	int CopyLength = 0;

	CopyLength = botName.copy(BotNPCType.name, 63);
	BotNPCType.name[CopyLength] = '\0';
	CopyLength = 0;

	CopyLength = botLastName.copy(BotNPCType.lastname, 69);
	BotNPCType.lastname[CopyLength] = '\0';
	CopyLength = 0;

	BotNPCType.level = botLevel;
	BotNPCType.race = botRace;
	BotNPCType.class_ = botClass;
	BotNPCType.bodytype = botBodyType;
	BotNPCType.cur_hp = hitPoints;
	BotNPCType.gender = gender;
	BotNPCType.size = size;
	BotNPCType.hp_regen = hitPointsRegenRate;
	BotNPCType.mana_regen = manaRegenRate;
	BotNPCType.luclinface = face;
	BotNPCType.hairstyle = hairStyle;
	BotNPCType.haircolor = hairColor;
	BotNPCType.eyecolor1 = eyeColor;
	BotNPCType.eyecolor2 = eyeColor2;
	BotNPCType.beardcolor = beardColor;
	BotNPCType.beard = beard;
	BotNPCType.drakkin_heritage = drakkinHeritage;
	BotNPCType.drakkin_tattoo = drakkinTattoo;
	BotNPCType.drakkin_details = drakkinDetails;
	BotNPCType.runspeed = runSpeed;
	BotNPCType.MR = mr;
	BotNPCType.CR = cr;
	BotNPCType.DR = dr;
	BotNPCType.FR = fr;
	BotNPCType.PR = pr;
	BotNPCType.AC = ac;
	BotNPCType.STR = str;
	BotNPCType.STA = sta;
	BotNPCType.DEX = dex;
	BotNPCType.AGI = agi;
	BotNPCType.INT = _int;
	BotNPCType.WIS = wis;
	BotNPCType.CHA = cha;
	BotNPCType.ATK = attack;

	BotNPCType.npc_id = 0;
	BotNPCType.texture = 0;
	BotNPCType.d_meele_texture1 = 0;
	BotNPCType.d_meele_texture2 = 0;
	BotNPCType.qglobal = false;

	return BotNPCType;
}

NPCType Bot::CreateDefaultNPCTypeStructForBot(std::string botName, std::string botLastName, uint8 botLevel, uint16 botRace, uint8 botClass, uint8 gender) {
	NPCType Result;
	int CopyLength = 0;

	CopyLength = botName.copy(Result.name, 63);
	Result.name[CopyLength] = '\0';
	CopyLength = 0;

	CopyLength = botLastName.copy(Result.lastname, 69);
	Result.lastname[CopyLength] = '\0';
	CopyLength = 0;

	Result.level = botLevel;
	Result.race = botRace;
	Result.class_ = botClass;
	Result.gender = gender;

	// default values just to initialize them to values that wont crash the client until they get calculated
	Result.size = 6;
	Result.npc_id = 0;
	Result.bodytype = 1;
	Result.cur_hp = 0;
	Result.drakkin_details = 0;
	Result.drakkin_heritage = 0;
	Result.drakkin_tattoo = 0;
	Result.runspeed = 2.501;
	Result.texture = 0;
	Result.d_meele_texture1 = 0;
	Result.d_meele_texture2 = 0;
	Result.qglobal = false;

	return Result;
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
	SetBotSpellID(BotSpellID);
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
	std::string TempBotName = std::string(this->GetCleanName());

	for(int iCounter = 0; iCounter < TempBotName.length(); iCounter++) {
		if(isalpha(TempBotName[iCounter]) || TempBotName[iCounter] == '_') {
			Result = true;
		}
	}

	return Result;
}

bool Bot::IsBotNameAvailable(std::string* errorMessage) {
	bool Result = false;

	if(this->GetCleanName()) {
		char* Query = 0;
		char TempErrorMessageBuffer[MYSQL_ERRMSG_SIZE];
		MYSQL_RES* DatasetResult;
		MYSQL_ROW DataRow;

		if(!database.RunQuery(Query, MakeAnyLenString(&Query, "SELECT COUNT(BotID) FROM bots WHERE Name LIKE '%s'", this->GetCleanName()), TempErrorMessageBuffer, &DatasetResult)) {
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

bool Bot::Save() {
	bool Result = false;
	std::string errorMessage;

	char* Query = 0;
	char TempErrorMessageBuffer[MYSQL_ERRMSG_SIZE];

	if(this->GetBotID() == 0) {
		// New bot record
		uint32 TempNewBotID = 0;
		if(!database.RunQuery(Query, MakeAnyLenString(&Query, "INSERT INTO bots (BotOwnerCharacterID, BotSpellsID, Name, LastName, BotLevel, Race, Class, BodyType, HitPoints, Gender, Size, HitPointsRegenRate, ManaRegenRate, Face, LuclinHairStyle, LuclinHairColor, LuclinEyeColor, LuclinEyeColor2, LuclinBeardColor, LuclinBeard, DrakkinHeritage, DrakkinTattoo, DrakkinDetails, RunSpeed, MR, CR, DR, FR, PR, AC, STR, STA, DEX, AGI, _INT, WIS, CHA, ATK) VALUES('%u', '%u', '%s', '%s', '%u', '%u', '%u', '%i', '%i', '%u', '%f', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%f', '%i', '%i', '%i', '%i', '%i', '%i', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u')", this->_botOwnerCharacterID, this->GetBotSpellID(), this->GetCleanName(), this->lastname, this->GetLevel(), GetRace(), GetClass(), GetBodyType(), this->GetHP(), GetGender(), GetSize(), this->hp_regen, this->mana_regen, this->GetLuclinFace(), this->GetHairStyle(), GetHairColor(), this->GetEyeColor1(), GetEyeColor2(), this->GetBeardColor(), this->GetBeard(), this->GetDrakkinHeritage(), this->GetDrakkinTattoo(), GetDrakkinDetails(), this->GetRunspeed(), GetMR(), GetCR(), GetDR(), GetFR(), GetPR(), GetAC(), GetSTR(), GetSTA(), GetDEX(), GetAGI(), GetINT(), GetWIS(), GetCHA(), GetATK()), TempErrorMessageBuffer, 0, 0, &TempNewBotID)) {
			errorMessage = std::string(TempErrorMessageBuffer);
		}
		else {
			SetBotID(TempNewBotID);
			Result = true;
		}
	}
	else {
		// Update existing bot record
		if(!database.RunQuery(Query, MakeAnyLenString(&Query, "UPDATE bots SET BotOwnerCharacterID = '%u', BotSpellsID = '%u', Name = '%s', LastName = '%s', BotLevel = '%u', Race = '%u', Class = '%u', BodyType = '%i', HitPoints = '%i', Gender = '%u', Size = '%d', HitPointsRegenRate = '%u', ManaRegenRate = '%u', Face = '%u', LuclinHairStyle = '%u', LuclinHairColor = '%u', LuclinEyeColor = '%u', LuclinEyeColor2 = '%u', LuclinBeardColor = '%u', LuclinBeard = '%u', DrakkinHeritage = '%u', DrakkinTattoo = '%u', DrakkinDetails = '%u', RunSpeed = '%d', MR = '%i', CR = '%i', DR = '%i', FR = '%i', PR = '%i', AC = '%i', STR = '%u', STA = '%u', DEX = '%u', AGI = '%u', _INT = '%u', WIS = '%u', CHA = '%u', ATK = '%i' WHERE BotID = '%u'", _botOwnerCharacterID, this->GetBotSpellID(), this->GetCleanName(), this->lastname, this->GetLevel(), this->GetRace(), this->GetClass(), GetBodyType(), this->GetHP(), GetGender(), GetSize(), this->hp_regen, this->mana_regen, this->GetLuclinFace(), this->GetHairStyle(), GetHairColor(), this->GetEyeColor1(), GetEyeColor2(), this->GetBeardColor(), this->GetBeard(), this->GetDrakkinHeritage(), this->GetDrakkinTattoo(), GetDrakkinDetails(), this->GetRunspeed(), GetMR(), GetCR(), GetDR(), GetFR(), GetPR(), GetAC(), GetSTR(), GetSTA(), GetDEX(), GetAGI(), GetINT(), GetWIS(), GetCHA(), GetATK(), this->GetBotID()), TempErrorMessageBuffer)) {
			errorMessage = std::string(TempErrorMessageBuffer);
		}
		else {
			Result = true;
		}
	}

	safe_delete(Query);

	if(!errorMessage.empty()) {
		// TODO: log this
	}

	return Result;
}

bool Bot::Process() {
	bool Result = false;

	Result = NPC::Process();

	DoAIProcessing();

	return Result;
}

// franck: EQoffline
// Depending of the class:
// -- Cleric, Druid, Shaman, Paladin will first check if there is someone to heal.
// -- Wizard, Mage, Necro will start the nuke.  
// -- TODO : Enchanter will nuke untill it it sees if there is an add. 
bool Bot::AI_EngagedCastCheck() {
	if (target && AIautocastspell_timer->Check(false)) {
		_ZP(Bot_AI_Process_engaged_cast);
		
		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.

		int8 botClass = GetClass();
		uint8 botLevel = GetLevel();

		mlog(AI__SPELLS, "Engaged autocast check triggered. Trying to cast healing spells then maybe offensive spells.");

		//BotRaids *br = entity_list.GetBotRaidByMob(this);
		
		if(botClass == CLERIC)
		{
			//if(br && IsBotRaiding()) {
			//	// try to heal the raid main tank
			//	if(br->GetBotMainTank() && (br->GetBotMainTank()->GetHPRatio() < 80)) {
			//		if(!Bot_AICastSpell(br->GetBotMainTank(), 80, SpellType_Heal)) {
			//			if(!entity_list.Bot_AICheckCloseBeneficialSpells(this, 80, MobAISpellRange, SpellType_Heal)) {
			//				if(!Bot_AICastSpell(this, 100, SpellType_Heal)) {
			//					AIautocastspell_timer->Start(RandomTimer(500, 2000), false);
			//					return true;
			//				}
			//			}
			//		}
			//	}
			//	// try to heal the raid secondar tank
			//	else if(br->GetBotSecondTank() && (br->GetBotSecondTank()->GetHPRatio() < 80)) {
			//		if(!Bot_AICastSpell(br->GetBotSecondTank(), 80, SpellType_Heal)) {
			//			if(!entity_list.Bot_AICheckCloseBeneficialSpells(this, 80, MobAISpellRange, SpellType_Heal)) {
			//				if(!Bot_AICastSpell(this, 100, SpellType_Heal)) {
			//					AIautocastspell_timer->Start(RandomTimer(500, 2000), false);
			//					return true;
			//				}
			//			}
			//		}
			//	}
			//}
			if(!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, MobAISpellRange, SpellType_Heal)) {
				if(!Bot_AICastSpell(this, 100, SpellType_Escape)) {
					if(!Bot_AICastSpell(this, 100, SpellType_Heal)) {
						if(!Bot_AICastSpell(target, 5, SpellType_DOT | SpellType_Nuke | SpellType_Lifetap | SpellType_Dispel)) {
							AIautocastspell_timer->Start(RandomTimer(500, 2000), false);
							return true;
						}
					}
				}
			}
		}
		else if((botClass == DRUID) || (botClass == SHAMAN) || (botClass == PALADIN) || (botClass == SHADOWKNIGHT) || (botClass == BEASTLORD) || (botClass == RANGER))
		{
			if (!Bot_AICastSpell(this, 100, SpellType_Escape | SpellType_Pet)) {
				if (!Bot_AICastSpell(this, 100, SpellType_Heal)) {
					if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 80, MobAISpellRange, SpellType_Heal)) {
						if(!Bot_AICastSpell(target, 80, SpellType_Root | SpellType_Snare | SpellType_DOT | SpellType_Nuke | SpellType_Lifetap | SpellType_Dispel)) {
							AIautocastspell_timer->Start(RandomTimer(1000, 5000), false);
							return true;
						}
					}
				}
			}
		}
		else if((botClass == WIZARD) || (botClass == MAGICIAN) || (botClass == NECROMANCER)) {
			if (!Bot_AICastSpell(this, 100, SpellType_Escape | SpellType_Pet)) {
				if(!Bot_AICastSpell(target, 80, SpellType_Root | SpellType_Snare | SpellType_DOT | SpellType_Nuke | SpellType_Lifetap | SpellType_Dispel)) {
					//no spell to cast, try again soon.
					AIautocastspell_timer->Start(RandomTimer(500, 2000), false);
					return true;
				}
			}
		}

		// TODO: Make enchanter to be able to mez
		else if(botClass == ENCHANTER) {
			if (!Bot_AICastSpell(this, 100, SpellType_Escape | SpellType_Pet)) {
				if(!Bot_AICastSpell(target, 80, SpellType_DOT | SpellType_Nuke | SpellType_Dispel)) {
					AIautocastspell_timer->Start(RandomTimer(500, 2000), false);
					return true;
				}
			}
		}
		else if(botClass == BARD) {
			if(!Bot_AICastSpell(this, 100, SpellType_Buff)) {
				if(!Bot_AICastSpell(target, 100, SpellType_Nuke | SpellType_Dispel | SpellType_Escape)) {// Bards will use their debuff songs
					AIautocastspell_timer->Start(RandomTimer(10, 50), false);
					return true;
				}					
			}
		}
		// And for all the others classes..
		else {
			if(!Bot_AICastSpell(this, 100, SpellType_Heal | SpellType_Escape)) {                                 // heal itself
				if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 80, MobAISpellRange, SpellType_Heal)) {	// heal others
					if(!Bot_AICastSpell(target, 80, SpellTypes_Detrimental)) {		// nuke..
						AIautocastspell_timer->Start(RandomTimer(500, 2000), false);							// timer 5 t 20 seconds
						return true;
					}
				}
			}
		}

		if(botClass != BARD) {
			AIautocastspell_timer->Start(RandomTimer(500, 2000), false);
		}

		return true;
	}

	return false;
}

void Bot::BotMeditate(bool isSitting) {
	if(isSitting) {
		// If the bot is a caster has less than 99% mana while its not engaged, he needs to sit to meditate
		if(GetManaRatio() < 99.0f) {
			if(mana_timer.Check(true)) {
				SetAppearance(eaSitting, false);
				if(!((int)GetManaRatio() % 24)) {
					Say("Medding for Mana. I have %3.1f%% of %d mana. It is: %d", GetManaRatio(), GetMaxMana(), GetMana());
				}
				int32 level = GetLevel();
				int32 regen = (((GetSkill(MEDITATE)/10)+(level-(level/4)))/4)+4;
				spellbonuses.ManaRegen = 0;
				for(int j=0; j<BUFF_COUNT; j++) {
					if(buffs[j].spellid != 65535) {
						const SPDat_Spell_Struct &spell = spells[buffs[j].spellid];
						for(int i=0; i<EFFECT_COUNT; i++) {
							if(IsBlankSpellEffect(buffs[j].spellid, i))
								continue;
							int effect = spell.effectid[i];
							switch(effect) {
								case SE_CurrentMana:
									spellbonuses.ManaRegen += CalcSpellEffectValue(buffs[j].spellid, i, buffs[j].casterlevel);
									break;
							}
						}
					}
				}
				regen += (spellbonuses.ManaRegen + itembonuses.ManaRegen);
				if(level >= 55) {
					regen += 1;//GetAA(aaMentalClarity);
				}
				if(level >= 56) {
					regen += 1;//GetAA(aaMentalClarity);
				}
				if(level >= 57) {
					regen += 1;//GetAA(aaMentalClarity);
				}
				if(level >= 71) {
					regen += 1;//GetAA(aaBodyAndMindRejuvenation);
				}
				if(level >= 72) {
					regen += 1;//GetAA(aaBodyAndMindRejuvenation);
				}
				if(level >= 73) {
					regen += 1;//GetAA(aaBodyAndMindRejuvenation);
				}
				if(level >= 74) {
					regen += 1;//GetAA(aaBodyAndMindRejuvenation);
				}
				if(level >= 75) {
					regen += 1;//GetAA(aaBodyAndMindRejuvenation);
				}
				regen = (regen * RuleI(Character, ManaRegenMultiplier)) / 100;

				float mana_regen_rate = RuleR(EQOffline, BotManaRegen);
				if(mana_regen_rate < 1.0f)
					mana_regen_rate = 1.0f;

				regen = regen / mana_regen_rate;

				SetMana(GetMana() + regen);
			}
		}
		else {
			SetAppearance(eaStanding, false);
		}
	}
	else {
		// Let's check our mana in fights..
		if(mana_timer.Check(true)) {
			if((!((int)GetManaRatio() % 12)) && ((int)GetManaRatio() < 10)) {
				Say("Medding for Mana. I have %3.1f%% of %d mana. It is: %d", GetManaRatio(), GetMaxMana(), GetMana());
			}
			int32 level = GetLevel();
			spellbonuses.ManaRegen = 0;
			for(int j=0; j<BUFF_COUNT; j++) {
				if(buffs[j].spellid != 65535) {
					const SPDat_Spell_Struct &spell = spells[buffs[j].spellid];
					for(int i=0; i<EFFECT_COUNT; i++) {
						if(IsBlankSpellEffect(buffs[j].spellid, i))
							continue;
						int effect = spell.effectid[i];
						switch(effect) {
							case SE_CurrentMana:
								spellbonuses.ManaRegen += CalcSpellEffectValue(buffs[j].spellid, i, buffs[j].casterlevel);
								break;
						}
					}
				}
			}
			int32 regen = 2 + spellbonuses.ManaRegen + itembonuses.ManaRegen + (level/5);
			if(level >= 55) {
				regen += 1;//GetAA(aaMentalClarity);
			}
			if(level >= 56) {
				regen += 1;//GetAA(aaMentalClarity);
			}
			if(level >= 57) {
				regen += 1;//GetAA(aaMentalClarity);
			}
			if(level >= 71) {
				regen += 1;//GetAA(aaBodyAndMindRejuvenation);
			}
			if(level >= 72) {
				regen += 1;//GetAA(aaBodyAndMindRejuvenation);
			}
			if(level >= 73) {
				regen += 1;//GetAA(aaBodyAndMindRejuvenation);
			}
			if(level >= 74) {
				regen += 1;//GetAA(aaBodyAndMindRejuvenation);
			}
			if(level >= 75) {
				regen += 1;//GetAA(aaBodyAndMindRejuvenation);
			}
			regen = (regen * RuleI(Character, ManaRegenMultiplier)) / 100;

			float mana_regen_rate = RuleR(EQOffline, BotManaRegen);
			if(mana_regen_rate < 1.0f)
				mana_regen_rate = 1.0f;

			regen = regen / mana_regen_rate;

			SetMana(GetMana() + regen);
		}
	}
}

bool Bot::BotRangedAttack(Mob* other) {
	//make sure the attack and ranged timers are up
	//if the ranged timer is disabled, then they have no ranged weapon and shouldent be attacking anyhow
	if((attack_timer.Enabled() && !attack_timer.Check(false)) || (ranged_timer.Enabled() && !ranged_timer.Check())) {
		mlog(COMBAT__RANGED, "Bot Archery attack canceled. Timer not up. Attack %d, ranged %d", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		Message(0, "Error: Timer not up. Attack %d, ranged %d", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		return false;
	}

	const Item_Struct* RangeWeapon = database.GetItem(equipment[MATERIAL_SECONDARY]);
	const Item_Struct* Ammo = database.GetItem(equipment[MATERIAL_PRIMARY]);
	ItemInst* RangeItem = new ItemInst(RangeWeapon);
	ItemInst* AmmoItem = new ItemInst(Ammo);
	mlog(COMBAT__RANGED, "Shooting %s with bow %s (%d) and arrow %s (%d)", other->GetCleanName(), RangeWeapon->Name, RangeWeapon->ID, Ammo->Name, Ammo->ID);
	
	if(!IsAttackAllowed(other) || 
		IsCasting() || 
		DivineAura() ||
		IsStunned() ||
		IsMezzed() ||
		(GetAppearance() == eaDead))
	{
		safe_delete(RangeItem);
		safe_delete(AmmoItem);
		return false;
	}
	
	SendItemAnimation(other, Ammo, ARCHERY);

	// Hit?
	if(!other->CheckHitChance(this, ARCHERY, SLOT_PRIMARY)) {
		mlog(COMBAT__RANGED, "Ranged attack missed %s.", other->GetCleanName());
		other->Damage(this, 0, SPELL_UNKNOWN, ARCHERY);
	}
	else {
		mlog(COMBAT__RANGED, "Ranged attack hit %s.", other->GetCleanName());
		
		if(!TryHeadShot(other, ARCHERY)) {
			sint16 WDmg = GetWeaponDamage(other, RangeWeapon);
			sint16 ADmg = GetWeaponDamage(other, Ammo);
			if((WDmg > 0) || (ADmg > 0)){
				if(WDmg < 0)
					WDmg = 0;
				if(ADmg < 0)
					ADmg = 0;

				uint32 MaxDmg = (2*(WDmg+ADmg)*GetDamageTable(ARCHERY)) / 100;

				if(GetLevel() >= 61) { // Archery Mastery 3 AA
					MaxDmg = MaxDmg * 150/100;
				}
				else if(GetLevel() == 60) { // Archery Mastery 2 AA
					MaxDmg = MaxDmg * 125/100;
				}
				else if(GetLevel() == 59) { // Archery Mastery 1 AA
					MaxDmg = MaxDmg * 115/100;
				}
				mlog(COMBAT__RANGED, "Bow DMG %d, Arrow DMG %d, Max Damage %d.", WDmg, ADmg, MaxDmg);
				
				if(GetClass()==RANGER && other->IsNPC() && !other->IsMoving() && !other->IsRooted() && GetLevel() > 50){
					MaxDmg *= 2;
					mlog(COMBAT__RANGED, "Ranger. Target is stationary, doubling max damage to %d", MaxDmg);
				}

				sint32 TotalDmg = 0;

				if (MaxDmg == 0)
					MaxDmg = 1;

				if(RuleB(Combat, UseIntervalAC))
					TotalDmg = MaxDmg;
				else
					TotalDmg = MakeRandomInt(1, MaxDmg);

				int minDmg = 1;
				if(GetLevel() > 25){
					//twice, for ammo and weapon
					TotalDmg += (2*((GetLevel()-25)/3));
					minDmg += (2*((GetLevel()-25)/3));
				}

				other->MeleeMitigation(this, TotalDmg, minDmg);
				ApplyMeleeDamageBonus(ARCHERY, TotalDmg);
				TryCriticalHit(other, ARCHERY, TotalDmg);
				other->Damage(this, TotalDmg, SPELL_UNKNOWN, ARCHERY);
			}
			else {
				other->Damage(this, -5, SPELL_UNKNOWN, ARCHERY);
			}
		}
	}

	//try proc on hits and misses
	if(other && (other->GetHP() > -10)) {
		TryWeaponProc(RangeWeapon, other);
	}
	
	safe_delete(RangeItem);
	safe_delete(AmmoItem);

	return true;
}

bool Bot::BotAttackMelee(Mob* other, int Hand, bool bRiposte)
{
	_ZP(Bot_BotAttackMelee);

	if (!other) {
		SetTarget(NULL);
		LogFile->write(EQEMuLog::Error, "A null Mob object was passed to NPC::BotAttackMelee for evaluation!");
		return false;
	}
	
	if(!GetTarget())
		SetTarget(other);
	
	mlog(COMBAT__ATTACKS, "Attacking %s with hand %d %s", other?other->GetCleanName():"(NULL)", Hand, bRiposte?"(this is a riposte)":"");
	
	if ((IsCasting() && (GetClass() != BARD)) ||
		other == NULL ||
		(GetHP() < 0) ||
		(!IsAttackAllowed(other)))
	{
		if(this->GetOwnerID())
			entity_list.MessageClose(this, 1, 200, 10, "%s says, 'That is not a legal target master.'", this->GetCleanName());
		if(other)
			RemoveFromHateList(other);
		mlog(COMBAT__ATTACKS, "I am not allowed to attack %s", other->GetCleanName());
		return false;
	}

	if(DivineAura()) {//cant attack while invulnerable
		mlog(COMBAT__ATTACKS, "Attack canceled, Divine Aura is in effect.");
		return false;
	}
	
	FaceTarget(target);

	ItemInst* weapon = NULL;
	const Item_Struct* botweapon = NULL;
	if((Hand == SLOT_PRIMARY) && equipment[MATERIAL_PRIMARY])
	    botweapon = database.GetItem(equipment[MATERIAL_PRIMARY]);
	if((Hand == SLOT_SECONDARY) && equipment[MATERIAL_SECONDARY])
	    botweapon = database.GetItem(equipment[MATERIAL_SECONDARY]);
	if(botweapon != NULL)
		weapon = new ItemInst(botweapon);

	if(weapon != NULL) {
		if (!weapon->IsWeapon()) {
			mlog(COMBAT__ATTACKS, "Attack canceled, Item %s (%d) is not a weapon.", weapon->GetItem()->Name, weapon->GetID());
			safe_delete(weapon);
			return(false);
		}
		mlog(COMBAT__ATTACKS, "Attacking with weapon: %s (%d)", weapon->GetItem()->Name, weapon->GetID());
	} else {
		mlog(COMBAT__ATTACKS, "Attacking without a weapon.");
	}
	
	// calculate attack_skill and skillinuse depending on hand and weapon
	// also send Packet to near clients
	SkillType skillinuse;
	AttackAnimation(skillinuse, Hand, weapon);
	mlog(COMBAT__ATTACKS, "Attacking with %s in slot %d using skill %d", weapon?weapon->GetItem()->Name:"Fist", Hand, skillinuse);
	
	/// Now figure out damage
	int damage = 0;
	int weapon_damage = GetWeaponDamage(other, weapon);
	
	//if weapon damage > 0 then we know we can hit the target with this weapon
	//otherwise we cannot and we set the damage to -5 later on
	if(weapon_damage > 0){
		
		//Berserker Berserk damage bonus
		if((GetHPRatio() < 30) && (GetClass() == BERSERKER)){
			int bonus = 3 + GetLevel()/10;		//unverified
			weapon_damage = weapon_damage * (100+bonus) / 100;
			mlog(COMBAT__DAMAGE, "Berserker damage bonus increases DMG to %d", weapon_damage);
		}

		//try a finishing blow.. if successful end the attack
		if(TryFinishingBlow(other, skillinuse)) {
			safe_delete(weapon);
			return (true);
		}
		
		//damage formula needs some work
		int min_hit = 1;
		int max_hit = (2*weapon_damage*GetDamageTable(skillinuse)) / 100;

		if(GetLevel() < 10 && max_hit > 20)
			max_hit = 20;
		else if(GetLevel() < 20 && max_hit > 40)
			max_hit = 40;

		//if mainhand only, get the bonus damage from level
		if((Hand == SLOT_PRIMARY) && (GetLevel() >= 28) && IsWarriorClass())
		{
			int8 ucDamageBonus = GetWeaponDamageBonus( weapon ? weapon->GetItem() : (const Item_Struct*) NULL );

			min_hit += (int) ucDamageBonus;
			max_hit += (int) ucDamageBonus;
		}

		min_hit = min_hit * (100 + itembonuses.MinDamageModifier + spellbonuses.MinDamageModifier) / 100;

		if(Hand == SLOT_SECONDARY) {
			if((GetClass() == WARRIOR) ||
				(GetClass() == ROGUE) ||
				(GetClass() == MONK) ||
				(GetClass() == RANGER) ||
				(GetClass() == BARD) ||
				(GetClass() == BEASTLORD))
			{
				if(GetLevel() >= 65)
				{ // Sinister Strikes AA
					int sinisterBonus = MakeRandomInt(5, 10);
					min_hit += (min_hit * sinisterBonus / 100);
					max_hit += (max_hit * sinisterBonus / 100);
				}
			}
		}

		if(max_hit < min_hit)
			max_hit = min_hit;

		if(RuleB(Combat, UseIntervalAC))
			damage = max_hit;
		else
			damage = MakeRandomInt(min_hit, max_hit);

		mlog(COMBAT__DAMAGE, "Damage calculated to %d (min %d, max %d, str %d, skill %d, DMG %d, lv %d)",
			damage, min_hit, max_hit, GetSTR(), GetSkill(skillinuse), weapon_damage, GetLevel());

		//check to see if we hit..
		if(!other->CheckHitChance(this, skillinuse, Hand)) {
			mlog(COMBAT__ATTACKS, "Attack missed. Damage set to 0.");
			damage = 0;
			other->AddToHateList(this, 0);
		} else {	//we hit, try to avoid it
			other->AvoidDamage(this, damage);
			other->MeleeMitigation(this, damage, min_hit);
			ApplyMeleeDamageBonus(skillinuse, damage);
			TryCriticalHit(other, skillinuse, damage);
			mlog(COMBAT__DAMAGE, "Final damage after all reductions: %d", damage);

			if(damage != 0){
				sint32 hate = max_hit;
				mlog(COMBAT__HITS, "Generating hate %d towards %s", hate, GetCleanName());
				// now add done damage to the hate list
				other->AddToHateList(this, hate);
			}
			else
				other->AddToHateList(this, 0);
		}

		//riposte
		bool slippery_attack = false; // Part of hack to allow riposte to become a miss, but still allow a Strikethrough chance (like on Live)
		if (damage == -3)  {
			if(bRiposte) {
				safe_delete(weapon);
				return false;
			}
			else {
				
				int saChance = 0;
				if(IsWarriorClass()) {
					if(GetLevel() >= 70)
					{ // Slippery Attacks AA 5
						saChance = 5;
					}
					else if(GetLevel() >= 69)
					{ // Slippery Attacks AA 4
						saChance = 4;
					}
					else if(GetLevel() >= 68)
					{ // Slippery Attacks AA 3
						saChance = 3;
					}
					else if(GetLevel() >= 67)
					{ // Slippery Attacks AA 2
						saChance = 2;
					}
					else if(GetLevel() >= 66)
					{ // Slippery Attacks AA 1
						saChance = 1;
					}
				}
				if ((Hand == SLOT_SECONDARY) && saChance) {// Do we even have it & was attack with mainhand? If not, don't bother with other calculations
					if (MakeRandomInt(0, 100) < (saChance * 20)) {
						damage = 0; // Counts as a miss
						slippery_attack = true;
					} else DoRiposte(other);
				}
				else DoRiposte(other);
			}
		}
		
		int aaStrikethroughBonus = 0;
		if(GetClass() == MONK)
		{
			if(GetLevel() >= 67)
			{ // Strikethrough AA 3
				aaStrikethroughBonus = 6;
			}
			else if(GetLevel() >= 66)
			{ // Strikethrough AA 2
				aaStrikethroughBonus = 4;
			}
			else if(GetLevel() >= 65)
			{ // Strikethrough AA 1
				aaStrikethroughBonus = 2;
			}
		}

		//strikethrough..
		if (((damage < 0) || slippery_attack) && !bRiposte) { // Hack to still allow Strikethrough chance w/ Slippery Attacks AA
			if(MakeRandomInt(0, 100) < (itembonuses.StrikeThrough + spellbonuses.StrikeThrough + aaStrikethroughBonus)) {
				BotAttackMelee(other, Hand, true); // Strikethrough only gives another attempted hit
				safe_delete(weapon);
				return false;
			}
		}
	}
	else{
		damage = -5;
	}
	
	///////////////////////////////////////////////////////////
	//////    Send Attack Damage
	///////////////////////////////////////////////////////////
	other->Damage(this, damage, SPELL_UNKNOWN, skillinuse);
	if(damage > 0 && (spellbonuses.MeleeLifetap || itembonuses.MeleeLifetap)) {
		mlog(COMBAT__DAMAGE, "Melee lifetap healing for %d damage.", damage);
		//heal self for damage done..
		HealDamage(damage);
	}
	
	//break invis when you attack
	if(invisible) {
		mlog(COMBAT__ATTACKS, "Removing invisibility due to melee attack.");
		BuffFadeByEffect(SE_Invisibility);
		BuffFadeByEffect(SE_Invisibility2);
		invisible = false;
	}
	if(invisible_undead) {
		mlog(COMBAT__ATTACKS, "Removing invisibility vs. undead due to melee attack.");
		BuffFadeByEffect(SE_InvisVsUndead);
		BuffFadeByEffect(SE_InvisVsUndead2);
		invisible_undead = false;
	}
	if(invisible_animals){
		mlog(COMBAT__ATTACKS, "Removing invisibility vs. animals due to melee attack.");
		BuffFadeByEffect(SE_InvisVsAnimals);
		invisible_animals = false;
	}

	if(hidden || improved_hidden){
		hidden = false;
		improved_hidden = false;
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
		SpawnAppearance_Struct* sa_out = (SpawnAppearance_Struct*)outapp->pBuffer;
		sa_out->spawn_id = GetID();
		sa_out->type = 0x03;
		sa_out->parameter = 0;
		entity_list.QueueClients(this, outapp, true);
		safe_delete(outapp);
	}
	
	////////////////////////////////////////////////////////////
	////////  PROC CODE
	////////  Kaiyodo - Check for proc on weapon based on DEX
	///////////////////////////////////////////////////////////
	if(other->GetHP() > -10 && !bRiposte && this) {
		TryWeaponProc(weapon, other);
	}
	
	if(damage > 0) {
		// Give the opportunity to throw back a defensive proc, if we are successful in affecting damage on our target
		other->TriggerDefensiveProcs(this);
		safe_delete(weapon);
		return true;
	}
	else {
		safe_delete(weapon);
		return false;
	}
}

bool Bot::CheckBotDoubleAttack(bool tripleAttack) {
	// If you don't have the double attack skill, return
	if(!GetSkill(DOUBLE_ATTACK))
		return false;
	
	// You start with no chance of double attacking
	int chance = 0;
	
	// Used for maxSkill and triple attack calcs
	int8 classtype = GetClass();
	
	// The current skill level
	uint16 skill = GetSkill(DOUBLE_ATTACK);

	// Discipline bonuses give you 100% chance to double attack
	sint16 buffs = spellbonuses.DoubleAttackChance + itembonuses.DoubleAttackChance;
	
	Mob* BotOwner = this->GetBotOwner();

	// The maximum value for the Class based on the server rule of MaxLevel
	if(!BotOwner || BotOwner->qglobal || (GetAppearance() == eaDead))
		return false;
	int16 maxSkill = BotOwner->CastToClient()->MaxSkill(DOUBLE_ATTACK, classtype, RuleI(Character, MaxLevel));

	int32 aaBonus = 0;
	uint8 aalevel = GetLevel();
	if((classtype == BEASTLORD)||(classtype == BARD)) { // AA's Beastial Frenzy, Harmonious Attacks
		if(aalevel >= 65) {
			aaBonus += 5;
		}
		else if(aalevel >= 64) {
			aaBonus += 4;
		}
		else if(aalevel >= 63) {
			aaBonus += 3;
		}
		else if(aalevel >= 62) {
			aaBonus += 2;
		}
		else if(aalevel >= 61) {
			aaBonus += 1;
		}
	}
	if((classtype == PALADIN)||(classtype == SHADOWKNIGHT)) { // AA Knights Advantage
		if(aalevel >= 65) {
			aaBonus += 30;
		}
		else if(aalevel >= 63) {
			aaBonus += 20;
		}
		else if(aalevel >= 61) {
			aaBonus += 10;
		}
	}
	if((classtype == ROGUE)||(classtype == WARRIOR)||(classtype == RANGER)||(classtype == MONK)) { // AA Ferocity and Relentless Assault
		if(aalevel >= 70) {
			aaBonus += 60;
		}
		else if(aalevel >= 65) {
			aaBonus += 30;
		}
		else if(aalevel >= 63) {
			aaBonus += 20;
		}
		else if(aalevel >= 61) {
			aaBonus += 10;
		}
	}

	// Half of Double Attack Skill used to check chance for Triple Attack
	if(tripleAttack) {
		// Only some Double Attack classes get Triple Attack
		if((classtype == MONK) || (classtype == WARRIOR) || (classtype == RANGER) || (classtype == BERSERKER)) {
			// We only get half the skill, but should get all the bonuses
			chance = (skill/2) + buffs + aaBonus;
		}
		else {
			return false;
		}
	}
	else {
		// This is the actual Double Attack chance
		chance = skill + buffs + aaBonus;
	}
	
	// If your chance is greater than the RNG you are successful! Always have a 5% chance to fail at max skills+bonuses.
	if(chance > MakeRandomInt(0, (maxSkill + itembonuses.DoubleAttackChance + aaBonus)*1.05)) {
		return true;
	}

	return false;
}

// Franck-add: EQoffline
// This function was reworked a bit for bots.
bool Bot::Bot_AICastSpell(Mob* tar, int8 iChance, int16 iSpellTypes) {
	_ZP(NPC_Bot_AICastSpell);

	if (!tar) {
		return false;
	}

	if(!AI_HasSpells())
		return false;

	if(tar->GetAppearance() == eaDead) {
		if(tar->IsClient() && tar->CastToClient()->GetFeigned()) {
			// do nothing
		}
		else {
			return false;
		}
	}

	if (iChance < 100) {
		if (MakeRandomInt(0, 100) > iChance){
			return false;
		}
	}

	int8 botClass = GetClass();
	uint8 botLevel = GetLevel();

	float dist2;

	if (iSpellTypes & SpellType_Escape) {
		dist2 = 0; 
	} else 
		dist2 = DistNoRoot(*tar);

	bool checked_los = false;	//we do not check LOS until we are absolutely sure we need to, and we only do it once.

	float manaR = GetManaRatio();
	for (int i=AIspells.size()-1; i >= 0; i--) {
		if (AIspells[i].spellid <= 0 || AIspells[i].spellid >= SPDAT_RECORDS) {
			// this is both to quit early to save cpu and to avoid casting bad spells
			// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
			continue;
		}
		if (iSpellTypes & AIspells[i].type) {
			// manacost has special values, -1 is no mana cost, -2 is instant cast (no mana)
			sint32 mana_cost = AIspells[i].manacost;
			if (mana_cost == -1)
				mana_cost = spells[AIspells[i].spellid].mana;
			else if (mana_cost == -2)
				mana_cost = 0;
			bool extraMana = false;
			sint32 hasMana = GetMana();
			if(RuleB(EQOffline, BotFinishBuffing)) {
				if(mana_cost > hasMana) {
					// Let's have the bots complete the buff time process
					if(iSpellTypes & SpellType_Buff) {
						SetMana(mana_cost);
						extraMana = true;
					}
				}
			}
			if (((((spells[AIspells[i].spellid].targettype==ST_GroupTeleport && AIspells[i].type==2)
				|| spells[AIspells[i].spellid].targettype==ST_AECaster
				|| spells[AIspells[i].spellid].targettype==ST_Group
				|| spells[AIspells[i].spellid].targettype==ST_AEBard)
				&& dist2 <= spells[AIspells[i].spellid].aoerange*spells[AIspells[i].spellid].aoerange)
				|| dist2 <= spells[AIspells[i].spellid].range*spells[AIspells[i].spellid].range) && (mana_cost <= GetMana() || GetMana() == GetMaxMana())) {

					switch (AIspells[i].type) {
					case SpellType_Heal: {
						if (
							( (spells[AIspells[i].spellid].targettype==ST_GroupTeleport || spells[AIspells[i].spellid].targettype == ST_Target || tar == this)
							&& tar->DontHealMeBefore() < Timer::GetCurrentTime()
							&& tar->CanBuffStack(AIspells[i].spellid, botLevel, true) >= 0))
						{
							if(botClass == BARD) {
								if(IsEffectInSpell(AIspells[i].spellid, SE_MovementSpeed) && !zone->CanCastOutdoor()) {
									break;
								}
							}
							int8 hpr = (int8)tar->GetHPRatio();
							if(hpr<= 80 || ((tar->IsClient()||tar->IsPet()) && (hpr <= 98)) || (botClass == BARD))
							{
								if(tar->GetClass() == NECROMANCER) {
									// Necro bots use too much cleric mana with thier
									// mana for life spells... give them a chance
									// to lifetap something
									if(hpr > 60) {
										break;
									}
								}

								int32 TempDontHealMeBeforeTime = tar->DontHealMeBefore();

								AIDoSpellCast(i, tar, mana_cost, &TempDontHealMeBeforeTime);

								if(TempDontHealMeBeforeTime != tar->DontHealMeBefore())
									tar->SetDontHealMeBefore(TempDontHealMeBeforeTime);

								// If the healer is casting a HoT don't immediately cast the regular heal afterwards
								// The first HoT is at level 19 and is priority 1
								// The regular heal is priority 2
								// Let the HoT heal for at least 3 tics before checking for the regular heal
								// For non-HoT heals, do a 4 second delay
								if((botClass == CLERIC || botClass == PALADIN) && (botLevel >= 19) && (AIspells[i].priority == 1)) {
									if(tar->GetOwnerID()) {
										tar->SetDontHealMeBefore(Timer::GetCurrentTime() + 18000);
										//tar->pDontHealMeBefore = (Timer::GetCurrentTime() + 18000);
									}
									else {
										tar->SetDontHealMeBefore(Timer::GetCurrentTime() + 12000);
										//tar->pDontHealMeBefore = (Timer::GetCurrentTime() + 12000);
									}
								}
								else if((botClass == CLERIC || botClass == PALADIN) && (botLevel >= 19) && (AIspells[i].priority == 2)) {
									if(AIspells[i].spellid == 13) { 
										// Complete Heal 4 second rotation
										tar->SetDontHealMeBefore(Timer::GetCurrentTime() + 4000);
										//tar->pDontHealMeBefore = (Timer::GetCurrentTime() + 4000);
									}
									else {
										tar->SetDontHealMeBefore(Timer::GetCurrentTime() + 1000);
										//tar->pDontHealMeBefore = (Timer::GetCurrentTime() + 1000);
									}
								}
								return true;
							}
						}
						break;
										 }
					case SpellType_Root: {
						if (
							!tar->IsRooted() 
							&& tar->DontRootMeBefore() < Timer::GetCurrentTime()
							&& tar->CanBuffStack(AIspells[i].spellid, botLevel, true) >= 0
							) {
								if(!checked_los) {
									if(!CheckLosFN(tar))
										return(false);	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
									checked_los = true;
								}
								int32 TempDontRootMeBefore = tar->DontRootMeBefore();
								AIDoSpellCast(i, tar, mana_cost, &TempDontRootMeBefore);
								if(TempDontRootMeBefore != tar->DontRootMeBefore())
									tar->SetDontRootMeBefore(TempDontRootMeBefore);
								return true;
						}
						break;
										 }
					case SpellType_Buff: {
						if (
							(spells[AIspells[i].spellid].targettype == ST_Target || tar == this)
							&& tar->DontBuffMeBefore() < Timer::GetCurrentTime()
							&& !tar->IsImmuneToSpell(AIspells[i].spellid, this)
							&& (tar->CanBuffStack(AIspells[i].spellid, botLevel, true) >= 0)
							&&  !(tar->IsPet() && tar->GetOwner()->IsClient() && this != tar)	//no buffing PC's pets, but they can buff themself

							) {
								// Put the zone levitate and movement check here since bots are able to bypass the client casting check
								if(	(IsEffectInSpell(AIspells[i].spellid, SE_Levitate) && !zone->CanLevitate()) ||
									(IsEffectInSpell(AIspells[i].spellid, SE_MovementSpeed) && !zone->CanCastOutdoor())) {
										break;
								}
								// when a pet class buffs its pet, it only needs to do it once
								if(spells[AIspells[i].spellid].targettype == ST_Pet) {
									Mob* newtar = GetPet();
									if(newtar) {
										if(!(newtar->CanBuffStack(AIspells[i].spellid, botLevel, true) >= 0)) {
											break;
										}
									}
									else {
										break;
									}
								}
								int32 TempDontBuffMeBefore = tar->DontBuffMeBefore();
								AIDoSpellCast(i, tar, mana_cost, &TempDontBuffMeBefore);
								if(TempDontBuffMeBefore != tar->DontBuffMeBefore())
									tar->SetDontBuffMeBefore(TempDontBuffMeBefore);

								if(extraMana) {
									// If the bot is just looping through spells and not casting
									// then don't let them keep the extra mana we gave them during
									// buff time
									SetMana(0);
									extraMana = false;
								}
								return true;
						}
						break;
										 }
					case SpellType_Escape: {
						int8 hpr = (int8)GetHPRatio();
#ifdef IPC          
						if (hpr <= 5 || (IsNPC() && CastToNPC()->IsInteractive() && tar != this) )
#else
						if ((hpr <= 15) && (tar == this))
#endif
						{
							AIDoSpellCast(i, this, mana_cost);
							return true;
						}
						break;
										   }
					case SpellType_Nuke: {
						if(((MakeRandomInt(1, 100) < 50) || ((botClass == BARD) || (botClass == SHAMAN) || (botClass == ENCHANTER)))
							&& ((tar->GetHPRatio() <= 95.0f) || ((botClass == BARD) || (botClass == SHAMAN) || (botClass == ENCHANTER)))
							&& !tar->IsImmuneToSpell(AIspells[i].spellid, this)
							&& (tar->CanBuffStack(AIspells[i].spellid, botLevel, true) >= 0))
						{
							if(!checked_los) {
								if(!CheckLosFN(tar))
									return(false);	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
								checked_los = true;
							}

							if(IsFearSpell(AIspells[i].spellid))
							{ // don't let fear cast if the npc isn't snared or rooted
								if(tar->GetSnaredAmount() == -1)
								{
									if(!tar->IsRooted())
										return false;
								}
							}

							AIDoSpellCast(i, tar, mana_cost);
							return true;
						}
						break;
										 }
					case SpellType_Dispel: {
						if(tar->GetHPRatio() > 95.0f)
						{
							if(!checked_los) {
								if(!CheckLosFN(tar))
									return(false);	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
								checked_los = true;
							}
							if(tar->CountDispellableBuffs() > 0)
							{
								AIDoSpellCast(i, tar, mana_cost);
								return true;
							}
						}
						break;
										   }
					case SpellType_Pet:
						{
							//keep mobs from recasting pets when they have them.
							if (!IsPet() && !GetPetID() && !IsBotCharmer())
							{
								if(botClass == MAGICIAN)
								{
									if(IsPetChooser())
									{
										bool ChoosePet = true;
										while(ChoosePet)
										{
											switch(GetPetChooserID())
											{
											case 0:
												if(!strncmp(spells[AIspells[i].spellid].teleport_zone, "SumWater", 8))
												{
													ChoosePet = false;
												}
												break;
											case 1:
												if(!strncmp(spells[AIspells[i].spellid].teleport_zone, "SumFire", 7))
												{
													ChoosePet = false;
												}
												break;
											case 2:
												if(!strncmp(spells[AIspells[i].spellid].teleport_zone, "SumAir", 6))
												{
													ChoosePet = false;
												}
												break;
											case 3:
												if(!strncmp(spells[AIspells[i].spellid].teleport_zone, "SumEarth", 8))
												{
													ChoosePet = false;
												}
												break;
											default:
												if(!strncmp(spells[AIspells[i].spellid].teleport_zone, "MonsterSum", 10))
												{
													ChoosePet = false;
												}
												break;
											}
											if(ChoosePet)
											{
												--i;
											}
										}
									}
									else
									{
										// have the magician bot randomly summon
										// the air, earth, fire or water pet
										// include monster summoning after they
										// become level 30 magicians
										int randpets;
										if(botLevel >= 30)
										{
											randpets = 4;
										}
										else
										{
											randpets = 3;
										}
										if(GetLevel() == 2)
										{
											// Do nothing
										}
										else if(GetLevel() == 3)
										{
											i = MakeRandomInt(i-1, i);
										}
										else if(GetLevel() == 4)
										{
											i = MakeRandomInt(i-2, i);
										}
										else
										{
											i = MakeRandomInt(i-randpets, i);
										}
									}
								}
								AIDoSpellCast(i, tar, mana_cost);
								return true;
							}
							break;
						}
					case SpellType_Lifetap: {
						if ((tar->GetHPRatio() <= 90.0f)
							&& !tar->IsImmuneToSpell(AIspells[i].spellid, this)
							&& (tar->CanBuffStack(AIspells[i].spellid, botLevel, true) >= 0))
						{
							if(!checked_los) {
								if(!CheckLosFN(tar))
									return(false);	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
								checked_los = true;
							}
							AIDoSpellCast(i, tar, mana_cost);
							return true;
						}
						break;
											}
					case SpellType_Snare: {
						if (
							!tar->IsRooted()
							&& !tar->IsImmuneToSpell(AIspells[i].spellid, this)
							&& tar->DontSnareMeBefore() < Timer::GetCurrentTime()
							&& tar->CanBuffStack(AIspells[i].spellid, botLevel, true) >= 0
							) {
								if(!checked_los) {
									if(!CheckLosFN(tar))
										return(false);	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
									checked_los = true;
								}
								int32 TempDontSnareMeBefore = tar->DontSnareMeBefore();
								AIDoSpellCast(i, tar, mana_cost, &TempDontSnareMeBefore);
								if(TempDontSnareMeBefore != tar->DontSnareMeBefore())
									tar->SetDontSnareMeBefore(TempDontSnareMeBefore);

								return true;
						}
						break;
										  }
					case SpellType_DOT: {
						if (
							((tar->GetHPRatio()<=80.0f)||(!IsBotRaiding()))
							&& !tar->IsImmuneToSpell(AIspells[i].spellid, this)
							&& tar->DontDotMeBefore() < Timer::GetCurrentTime()
							&& tar->CanBuffStack(AIspells[i].spellid, botLevel, true) >= 0
							) {
								if(!checked_los) {
									if(!CheckLosFN(tar))
										return(false);	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
									checked_los = true;
								}
								int32 TempDontDotMeBefore = tar->DontDotMeBefore();
								AIDoSpellCast(i, tar, mana_cost, &TempDontDotMeBefore);
								if(TempDontDotMeBefore != tar->DontDotMeBefore())
									tar->SetDontDotMeBefore(TempDontDotMeBefore);

								return true;
						}
						break;
										}
					default: {
						// TODO: log this error
						//cout<<"Error: Unknown spell type in AICastSpell. caster:"<<this->GetCleanName()<<" type:"<<AIspells[i].type<<" slot:"<<i<<endl;
						break;
							 }
					}
			}
			if(extraMana) {
				// If the bot is just looping through spells and not casting
				// then don't let them keep the extra mana we gave them during
				// buff time
				SetMana(hasMana);
				extraMana = false;
			}
		}
	}
	return false;
}

bool Bot::Bot_AI_PursueCastCheck() {
	if (AIautocastspell_timer->Check(false)) {
		_ZP(Bot_AI_Process_pursue_cast);
		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.
		
		mlog(AI__SPELLS, "Bot Engaged (pursuing) autocast check triggered. Trying to cast offensive spells.");
		if(!Bot_AICastSpell(target, 90, SpellType_Root | SpellType_Nuke | SpellType_Lifetap | SpellType_Snare | SpellType_DOT | SpellType_Dispel)) {
			//no spell cast, try again soon.
			AIautocastspell_timer->Start(RandomTimer(500, 2000), false);
		} //else, spell casting finishing will reset the timer.
		return(true);
	}
	return(false);
}

// Franck-add: EQoffline
// This function was reworked a bit for bots.
bool Bot::Bot_AI_IdleCastCheck() {
	if (AIautocastspell_timer->Check(false)) {
		_ZP(NPC_Bot_AI_IdleCastCheck);
#if MobAI_DEBUG_Spells >= 25
		cout << "Non-Engaged autocast check triggered: " << this->GetCleanName() << endl;
#endif
		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.

		//Ok, IdleCastCheck depends of class. 
		// Healers will check if a heal is needed before buffing.
		int8 botClass = GetClass();
		if(botClass == CLERIC || botClass == PALADIN || botClass == RANGER)
		{
			if (!Bot_AICastSpell(this, 50, SpellType_Heal | SpellType_Buff))
			{
				if(!entity_list.Bot_AICheckCloseBeneficialSpells(this, 50, MobAISpellRange, SpellType_Heal))
				{
					if(!entity_list.Bot_AICheckCloseBeneficialSpells(this, 50, MobAISpellRange, SpellType_Buff))
					{
						if(IsGrouped())
						{
							Group *g = entity_list.GetGroupByMob(this);
							if(g) {
								for(int i=0; i<MAX_GROUP_MEMBERS; ++i)
								{
									if(g->members[i] && !g->members[i]->qglobal && (g->members[i]->GetHPRatio() < 99))
									{
										if(Bot_AICastSpell(g->members[i], 100, SpellType_Heal))
										{
											AIautocastspell_timer->Start(RandomTimer(1000, 5000), false);
											return true;
										}
									}
									if(g->members[i] && !g->members[i]->qglobal && g->members[i]->GetPetID())
									{
										if(Bot_AICastSpell(g->members[i]->GetPet(), 100, SpellType_Heal))
										{
											AIautocastspell_timer->Start(RandomTimer(1000, 5000), false);
											return true;
										}
									}
								}
							}
						}
						AIautocastspell_timer->Start(RandomTimer(1000, 5000), false);
						return(true);
					}
				}
			}
		}
		// Pets class will first cast their pet, then buffs
		else if(botClass == DRUID || botClass == MAGICIAN || botClass == SHADOWKNIGHT || botClass == SHAMAN || botClass == NECROMANCER || botClass == ENCHANTER || botClass == BEASTLORD  || botClass == WIZARD)
		{			
			if (!Bot_AICastSpell(this, 100, SpellType_Pet))
			{
				if (!Bot_AICastSpell(this, 50, SpellType_Heal | SpellType_Buff))
				{
					if(!entity_list.Bot_AICheckCloseBeneficialSpells(this, 50, MobAISpellRange, SpellType_Heal))
					{
						if(!entity_list.Bot_AICheckCloseBeneficialSpells(this, 50, MobAISpellRange, SpellType_Buff)) // then buff the group
						{
							if(IsGrouped())
							{
								Group *g = entity_list.GetGroupByMob(this);
								if(g) {
									for(int i=0; i<MAX_GROUP_MEMBERS; ++i)
									{
										if(g->members[i] && !g->members[i]->qglobal && (g->members[i]->GetHPRatio() < 99))
										{
											if(Bot_AICastSpell(g->members[i], 100, SpellType_Heal))
											{
												AIautocastspell_timer->Start(RandomTimer(1000, 5000), false);
												return true;
											}
										}
										if(g->members[i] && !g->members[i]->qglobal && g->members[i]->GetPetID())
										{
											if(Bot_AICastSpell(g->members[i]->GetPet(), 100, SpellType_Heal))
											{
												AIautocastspell_timer->Start(RandomTimer(1000, 5000), false);
												return true;
											}
										}
									}
								}
							}
							AIautocastspell_timer->Start(RandomTimer(1000, 5000), false);
							return(true);
						}
					}
				}
			}
		}		
		// bard bots
		else if(botClass == BARD)
		{
			Bot_AICastSpell(this, 100, SpellType_Heal);
			AIautocastspell_timer->Start(1000, false);
			return true;
		}

		// and standard buffing for others..
		else {
			if (!Bot_AICastSpell(this, 100, SpellType_Heal | SpellType_Buff | SpellType_Pet))
			{
				if(!entity_list.Bot_AICheckCloseBeneficialSpells(this, 50, MobAISpellRange, SpellType_Heal | SpellType_Buff)) {
					AIautocastspell_timer->Start(RandomTimer(1000, 5000), false);
					return true;
				}
			}
		}
		AIautocastspell_timer->Start(RandomTimer(1000, 5000), false);
		return true;
	}
	return false;
}

// Calls all relevant methods/functions to perform AI logic
void Bot::DoAIProcessing() {
	BotAIProcess();

	if(this->GetPetID() > 0 && this->GetPet())
		PetAIProcess();
}

// AI Processing for the Bot object
void Bot::BotAIProcess() {
	_ZP(Mob_BOT_Process);

	if(!IsAIControlled())
		return;

	if(!(AIthink_timer->Check() || attack_timer.Check(false)))
		return;

	int8 botClass = GetClass();
	uint8 botLevel = GetLevel();

	if(IsCasting() && (botClass != BARD))
		return;

	// A bot wont start its AI if not grouped
	if((IsPet() && !GetOwner()->IsBot()) || (IsBot() && !IsGrouped())) {
		return;
	}

	Mob* BotOwner = this->GetBotOwner();

	// The bots need an owner
	if(!BotOwner || BotOwner->qglobal || (GetAppearance() == eaDead) || BotOwner->IsBot())
		return;

	if(!IsEngaged()) {
		if(GetFollowID()) {
			if(BotOwner && BotOwner->CastToClient()->AutoAttackEnabled() && BotOwner->GetTarget() &&
				BotOwner->GetTarget()->IsNPC() && BotOwner->GetTarget()->GetHateAmount(BotOwner)) {
					this->SetBotOrderAttack(true);
					AddToHateList(BotOwner->GetTarget(), 1);
					this->SetBotOrderAttack(false);
			}
		}
	}

	if(IsEngaged()) {
		_ZP(Mob_BOT_Process_IsEngaged);
		if(IsRooted())
			SetTarget(hate_list.GetClosest(this));
		else
			SetTarget(hate_list.GetTop(this));

		if(!target)
			return;

		if(DivineAura())
			return;

		// TODO: Looks like this block of code doesn't need to be here
		// Lets see if we can let the main tank build a little aggro.  Let healers and slowers in though
		//if((botClass == CLERIC) || (botClass == SHAMAN) || (botClass == ENCHANTER) || (botClass == DRUID))
		//{
		//	// do nothing
		//}


		// TODO: Uncomment this after bot raiding is integrated
		//else if(GetBotRaidID())
		//{
		//	BotRaids *br = entity_list.GetBotRaidByMob(this);
		//	if(br)
		//	{
		//		if(br->GetBotMainTank() && (br->GetBotMainTank() != this))
		//		{
		//			if(br->GetBotMainTarget() && (br->GetBotMainTarget()->GetHateAmount(br->GetBotMainTank()) < 5000))
		//			{
		//				if(target == br->GetBotMainTarget())
		//				{
		//					return;
		//				}
		//			}
		//		}
		//	}
		//}

		if(GetHPRatio() < 15)
			StartEnrage();

		// Let's check if we have a los with our target.
		// If we don't, our hate_list is wiped.
		// Else, it was causing the bot to aggro behind wall etc... causing massive trains.
		if(!CheckLosFN(target) || target->IsMezzed() || !IsAttackAllowed(target))
		{
			WipeHateList();
			SetTarget(BotOwner);
			return;
		}

		bool is_combat_range = CombatRange(target);
		if(IsBotArcher()) {
			float range = GetBotArcheryRange() + 5.0; //Fudge it a little, client will let you hit something at 0 0 0 when you are at 205 0 0
			mlog(COMBAT__RANGED, "Calculated bow range to be %.1f", range);
			range *= range;
			if(DistNoRootNoZ(*target) > range) {
				mlog(COMBAT__RANGED, "Ranged attack out of range... client should catch this. (%f > %f).\n", DistNoRootNoZ(*target), range);
				//target is out of range, client does a message
				is_combat_range = false;
			}
			else if(DistNoRootNoZ(*target) < (RuleI(Combat, MinRangedAttackDist)*RuleI(Combat, MinRangedAttackDist))) {
				is_combat_range = false;
				AImovement_timer->Check();
				if(IsMoving())
				{
					SetRunAnimSpeed(0);
					SetHeading(target->GetHeading());
					if(moved) {
						moved=false;
						SetMoving(false);
						SendPosUpdate();
					}
					tar_ndx = 0;
				}
			}
			else {
				is_combat_range = true;
			}
		}

		// We're engaged, each class type has a special AI
		// Only melee class will go to melee. Casters and healers will stop and stay behind.
		// We 're a melee or any other class lvl<12. Yes, because after it becomes hard to go into melee for casters.. even for bots..
		if((botLevel <= 12) || (botClass == WARRIOR) || (botClass == PALADIN) || (botClass == RANGER) || (botClass == SHADOWKNIGHT) || (botClass == MONK) || (botClass == ROGUE) || (botClass == BEASTLORD) || (botClass == BERSERKER) || (botClass == BARD))
		{
			cast_last_time = true;
		}
		if(is_combat_range && cast_last_time)
		{
			cast_last_time = false;
			AImovement_timer->Check();
			if(IsMoving())
			{
				SetRunAnimSpeed(0);
				SetHeading(target->GetHeading());
				if(moved) {
					moved=false;
					SetMoving(false);
					SendPosUpdate();
				}
				tar_ndx = 0;
			}

			if(IsBotArcher() && ranged_timer.Check(false)) {
				if(MakeRandomInt(1, 100) > 95) {
					this->AI_EngagedCastCheck();
					BotMeditate(false);
				}
				else {
					if(target->GetHPRatio() < 98)
						BotRangedAttack(target);
				}
			}

			// we can't fight if we don't have a target, are stun/mezzed or dead..
			if(!IsBotArcher() && target && !IsStunned() && !IsMezzed() && (GetAppearance() != eaDead))
			{
				// First, special attack per class (kick, backstab etc..)
				DoClassAttacks(target);

				//try main hand first
				if(attack_timer.Check())
				{
					BotAttackMelee(target, SLOT_PRIMARY);
					bool tripleSuccess = false;
					if(BotOwner && target && CanThisClassDoubleAttack()) {

						if(BotOwner && CheckBotDoubleAttack()) {
							BotAttackMelee(target, SLOT_PRIMARY, true);
						}
						if(BotOwner && target && SpecAttacks[SPECATK_TRIPLE] && CheckBotDoubleAttack(true)) {
							tripleSuccess = true;
							BotAttackMelee(target, SLOT_PRIMARY, true);
						}
						//quad attack, does this belong here??
						if(BotOwner && target && SpecAttacks[SPECATK_QUAD] && CheckBotDoubleAttack(true)) {
							BotAttackMelee(target, SLOT_PRIMARY, true);
						}
					}

					// Handle Flurrys
					if((botClass == WARRIOR) && (botLevel >= 59)) {
						int flurrychance = 0;
						if(botLevel >= 61) { // Flurry AA's
							flurrychance += 50;
						}
						else if(botLevel == 60) {
							flurrychance += 25;
						}
						else if(botLevel == 59) {
							flurrychance += 10;
						}
						if(tripleSuccess) {
							tripleSuccess = false;
							if(botLevel >= 65) { // Raging Flurry AA's
								flurrychance += 50;
							}
							else if(botLevel == 64) {
								flurrychance += 25;
							}
							else if(botLevel == 63) {
								flurrychance += 10;
							}
						}
						if(rand()%1000 < flurrychance) {
							Message_StringID(MT_Flurry, 128);
							BotAttackMelee(target, SLOT_PRIMARY, true);
							BotAttackMelee(target, SLOT_PRIMARY, true);
						}
					}

					if(target && (botClass == MONK)) { // Rapid Strikes AA
						int chance_xhit1 = 0;
						int chance_xhit2 = 0;
						if(botLevel >= 69) {
							chance_xhit1 = 20;
							chance_xhit2 = 10;
						}
						else if(botLevel == 68) {
							chance_xhit1 = 16;
							chance_xhit2 = 8;
						}
						else if(botLevel == 67) {
							chance_xhit1 = 14;
							chance_xhit2 = 6;
						}
						else if(botLevel == 66) {
							chance_xhit1 = 12;
							chance_xhit2 = 4;
						}
						else if(botLevel == 65) {
							chance_xhit1 = 10;
							chance_xhit2 = 2;
						}
						if(MakeRandomInt(1,100) < chance_xhit1)
							BotAttackMelee(target, SLOT_PRIMARY, true);
						if(target && (MakeRandomInt(1,100) < chance_xhit2))
							BotAttackMelee(target, SLOT_PRIMARY, true);
					}

					// Handle Punishing Blade and Speed of the Knight and Wicked Blade
					if(target && ((botClass == MONK)||(botClass == RANGER)||(botClass == WARRIOR)||(botClass == PALADIN)||(botClass == SHADOWKNIGHT))) {
						if(botLevel >= 61) {
							ItemInst* weapon = NULL;
							const Item_Struct* botweapon = NULL;
							botweapon = database.GetItem(CastToNPC()->GetEquipment(MATERIAL_PRIMARY));
							if(botweapon != NULL) {
								weapon = new ItemInst(botweapon);
							}
							if(weapon) {
								if( weapon->GetItem()->ItemType == ItemType2HS ||
									weapon->GetItem()->ItemType == ItemType2HB ||
									weapon->GetItem()->ItemType == ItemType2HPierce )
								{
									int extatk = 0;
									if(botLevel >= 61) {
										extatk += 5;
									}
									if(botLevel >= 63) {
										extatk += 5;
									}
									if(botLevel >= 65) {
										extatk += 5;
									}
									if(botLevel >= 70) {
										extatk += 15;
									}
									if(MakeRandomInt(0, 100) < extatk) {
										BotAttackMelee(target, SLOT_PRIMARY, true);
									}
								}
							}
						}
					}
				}

				//now off hand
				if(target && attack_dw_timer.Check() && CanThisClassDualWield())
				{
					//can only dual weild without a weapon if you're a monk
					if((GetEquipment(MATERIAL_SECONDARY) != 0) || (botClass == MONK))
					{
						const Item_Struct* weapon = NULL;
						weapon = database.GetItem(CastToNPC()->GetEquipment(MATERIAL_PRIMARY));
						int weapontype = NULL;
						bool bIsFist = true;
						if(weapon != NULL) {
							weapontype = weapon->ItemType;
							bIsFist = false;
						}
						if(bIsFist || ((weapontype != ItemType2HS) && (weapontype != ItemType2HPierce) && (weapontype != ItemType2HB))) {
							float DualWieldProbability = (GetSkill(DUAL_WIELD) + botLevel) / 400.0f;
							if(botLevel >= 59) { // AA Ambidexterity
								DualWieldProbability += 0.1f;
							}
							//discipline effects:
							DualWieldProbability += (spellbonuses.DualWeildChance + itembonuses.DualWeildChance) / 100.0f;

							float random = MakeRandomFloat(0, 1);
							if (random < DualWieldProbability) { // Max 78% of DW
								BotAttackMelee(target, SLOT_SECONDARY);
								if(target && CanThisClassDoubleAttack() && CheckBotDoubleAttack()) {
									BotAttackMelee(target, SLOT_SECONDARY);
								}
							}
						}
					}
				}

				//Bard, rangers, SKs, Paladin can cast also
				if(botClass == BARD || botClass == RANGER || botClass == SHADOWKNIGHT || botClass == PALADIN || botClass == BEASTLORD) {
					this->AI_EngagedCastCheck();
					BotMeditate(false);
				}
			}
		} //end is within combat range
		// Now, if we re casters, we have a particular AI.
		if((botClass == CLERIC) || (botClass == DRUID) || (botClass == SHAMAN) || (botClass == NECROMANCER) || (botClass == WIZARD) || (botClass == MAGICIAN) || (botClass == ENCHANTER))
		{
			cast_last_time = true;
			// First, let's make them stop
			AImovement_timer->Check();
			if(IsMoving())
			{
				SetRunAnimSpeed(0);
				SetHeading(target->GetHeading());
				if(moved) {
					moved=false;
					SetMoving(false);
					SendPosUpdate();
				}
				tar_ndx = 0;
			}

			BotMeditate(false);

			// Then, use their special engaged AI.
			this->AI_EngagedCastCheck();
		} //end is within combat range
		else {
			//we cannot reach our target...
			// See if we can summon the mob to us
			if(!HateSummon() && !IsBotArcher())
			{
				//could not summon them, start pursuing...
				// TODO: Check here for another person on hate list with close hate value
				if(target && Bot_AI_PursueCastCheck())
				{}
				else if(target && AImovement_timer->Check())
				{
					if(!IsRooted()) {
						mlog(AI__WAYPOINTS, "Pursuing %s while engaged.", target->GetCleanName());
						CalculateNewPosition2(target->GetX(), target->GetY(), target->GetZ(), GetRunspeed(), false);
					} else {
						SetHeading(target->GetHeading());
						if(moved) {
							moved=false;
							SetMoving(false);
							SendPosUpdate();
						}
					}
				}
			}
		}
	}
	else {
		// Franck: EQoffline
		// Ok if we're not engaged, what's happening..
		SetTarget(entity_list.GetMob(GetFollowID()));
		if(!IsMoving()) {
			BotMeditate(true);
			Bot_AI_IdleCastCheck(); // let's rebuff, heal, etc..
		}

		// now the followID: that's what happening as the bots follow their leader.
		if(GetFollowID())
		{
			if(!GetTarget()) {
				SetFollowID(0);
			}
			else if(AImovement_timer->Check()){
				// float dist2 = DistNoRoot(*target);
				float dist2 = DistNoRoot(*GetTarget());
				SetRunAnimSpeed(0);
				if(dist2>184) {
					CalculateNewPosition2(GetTarget()->GetX(), GetTarget()->GetY(), GetTarget()->GetZ(), GetRunspeed(), false);
				}
				else {
					SetHeading(GetTarget()->GetHeading());
					if(moved) {
						moved=false;
						SetMoving(false);
						SendPosUpdate();
					}
				}
			}
		}
	}
}

// AI Processing for a Bot object's pet
void Bot::PetAIProcess() {
	_ZP(Mob_PET_Process);

	// of course, if we're not a pet
	if( !IsPet() || !GetOwner() )
		return;

	Mob* BotOwner = this->GetOwner();

	if(!GetOwner() || !GetID() || !GetOwnerID() || !BotOwner)
	{
		Kill();
		return;
	} 

	if (!IsAIControlled())
		return;

	if (!(AIthink_timer->Check() || attack_timer.Check(false)))
		return;

	if (IsCasting())
		return;

	// if our owner isn't a pet or if he is not a client...
	if (!GetOwner()->IsBot() || ( !GetOwner()->IsBot() && !GetOwner()->IsClient() ) )
		return;

	if (IsEngaged())
	{
		_ZP(Bot_PET_Process_IsEngaged);
		if (IsRooted())
			SetTarget(hate_list.GetClosest(this));
		else
			SetTarget(hate_list.GetTop(this));

		if (!GetTarget())
			return;

		// Let's check if we have a los with our target.
		// If we don't, our hate_list is wiped.
		// It causes some cpu stress but without it, it was causing the bot/pet to aggro behind wall, floor etc... 
		if(!CheckLosFN(target) || GetTarget()->IsMezzed() || !IsAttackAllowed(GetTarget())) {
			WipeHateList();
			SetTarget(GetOwner());
			return;
		}

		// TODO: Uncomment this block after bot raids has been integrated
		// Lets see if we can let the main tank build a little aggro
		/*if(GetBotRaidID())
		{
			BotRaids *br = entity_list.GetBotRaidByMob(GetOwner());
			if(br)
			{
				if(br->GetBotMainTank() && (br->GetBotMainTank() != this))
				{
					if(br->GetBotMainTarget() && (br->GetBotMainTarget()->GetHateAmount(br->GetBotMainTank()) < 5000))
					{
						if(target == br->GetBotMainTarget())
						{
							return;
						}
					}
				}
			}
		}*/

		bool is_combat_range = CombatRange(GetTarget());

		// Ok, we're engaged, each class type has a special AI
		// Only melee class will go to melee. Casters and healers will stay behind, following the leader by default.
		// I should probably make the casters staying in place so they can cast..

		// Ok, we 're a melee or any other class lvl<12. Yes, because after it becomes hard to go in melee for casters.. even for bots..
		if( is_combat_range )
		{
			AImovement_timer->Check();
			if(IsMoving())
			{
				SetRunAnimSpeed(0);
				SetHeading(GetTarget()->GetHeading());
				if(moved) {
					moved=false;
					SetMoving(false);
					SendPosUpdate();
				}
			}
			// we can't fight if we don't have a target, are stun/mezzed or dead..
			if(GetTarget() && !IsStunned() && !IsMezzed() && (GetAppearance() != eaDead)) 
			{
				if(attack_timer.Check())  // check the delay on the attack
				{		
					if(Attack(GetTarget(), 13))			// try the main hand
						if (GetTarget())					// Do we still have a target?
						{
							// We're a pet so we re able to dual attack
							sint32 RandRoll = MakeRandomInt(0, 99);	
							if (CanThisClassDoubleAttack() && (RandRoll < (GetLevel() + NPCDualAttackModifier)))	
							{
								if(Attack(GetTarget(), 13)) 
								{}
							}
						}

						// Ok now, let's check pet's offhand. 
						if (attack_dw_timer.Check() && GetOwnerID() && GetOwner() && ((GetOwner()->GetClass() == MAGICIAN) || (GetOwner()->GetClass() == NECROMANCER) || (GetOwner()->GetClass() == SHADOWKNIGHT) || (GetOwner()->GetClass() == BEASTLORD))) 
						{
							if(GetOwner()->GetLevel() >= 24)
							{
								float DualWieldProbability = (GetSkill(DUAL_WIELD) + GetLevel()) / 400.0f;
								DualWieldProbability -= MakeRandomFloat(0, 1);
								if(DualWieldProbability < 0){
									Attack(GetTarget(), 14);
									if (CanThisClassDoubleAttack())
									{
										sint32 RandRoll = rand()%100;
										if (RandRoll < (GetLevel() + 20))
										{
											Attack(GetTarget(), 14);
										}
									}
								}
							}
						}
						if(!GetOwner())
							return;

						// Special attack
						DoClassAttacks(GetTarget()); 
				}
				// See if the pet can cast any spell
				this->AI_EngagedCastCheck();
			}	
		}// end of the combat in range
		else{
			// Now, if we cannot reach our target
			if (!HateSummon()) 
			{
				if(GetTarget() && Bot_AI_PursueCastCheck()) 
				{}
				else if (target && AImovement_timer->Check()) 
				{
					SetRunAnimSpeed(0);
					if(!IsRooted()) {
						mlog(AI__WAYPOINTS, "Pursuing %s while engaged.", GetTarget()->GetCleanName());
						CalculateNewPosition2(GetTarget()->GetX(), GetTarget()->GetY(), GetTarget()->GetZ(), GetOwner()->GetRunspeed(), false);
					}
					else {
						SetHeading(GetTarget()->GetHeading());
						if(moved) {
							moved=false;
							SetMoving(false);
							SendPosUpdate();
						}
					}
				}
			}
		}
	}
	else{
		// Franck: EQoffline
		// Ok if we're not engaged, what's happening..
		if(GetTarget() != GetOwner()) {
			SetTarget(GetOwner());
		}

		if(!IsMoving()) {
			Bot_AI_IdleCastCheck();
		}

		if(AImovement_timer->Check()) {
			switch(pStandingPetOrder) {
				case SPO_Follow:
					{
						// float dist = DistNoRoot(*target);
						float dist = DistNoRoot(*GetTarget());
						SetRunAnimSpeed(0);
						if(dist > 184) {
							CalculateNewPosition2(GetTarget()->GetX(), GetTarget()->GetY(), GetTarget()->GetZ(), GetTarget()->GetRunspeed(), false);
						}
						else {
							SetHeading(GetTarget()->GetHeading());
							if(moved) {
								moved=false;
								SetMoving(false);
								SendPosUpdate();
							}
						}
					}
					break;
				case SPO_Sit:
					SetAppearance(eaSitting);
					break;
				case SPO_Guard:
					NextGuardPosition();
					break;
			}
		}
	}
}

void Bot::Depop(bool StartSpawnTimer) {
	NPC::Depop(StartSpawnTimer);
}

//void Bot::Depop(std::string* errorMessage) {
//	std::string TempErrorMessage;
//
//	CleanBotLeaderEntries(&TempErrorMessage);
//
//	if(TempErrorMessage.length() > 0) {
//		*errorMessage = TempErrorMessage;
//		return;
//	}
//}

//void Bot::CleanBotLeaderEntries(std::string* errorMessage) {
//	if(this->GetBotID() > 0 && this->_botOwnerCharacterID > 0) {
//		char errbuf[MYSQL_ERRMSG_SIZE];
//		char *query = 0;
//
//		if(!database.RunQuery(query, MakeAnyLenString(&query, "DELETE FROM botleader WHERE botid=%i", this->GetBotID()), errbuf)) {
//			*errorMessage = std::string(errbuf);
//		}
//
//		safe_delete_array(query);
//	}
//}

bool Bot::DeleteBot(std::string* errorMessage) {
	bool Result = false;
	int TempCounter = 0;

	if(this->GetBotID() > 0) {
		char* Query = 0;
		char TempErrorMessageBuffer[MYSQL_ERRMSG_SIZE];

		// TODO: These queries need to be ran together as a transaction.. ie, if one or more fail then they all will fail to commit to the database.

		if(!database.RunQuery(Query, MakeAnyLenString(&Query, "DELETE FROM bots WHERE BotID = '%u'", this->GetBotID()), TempErrorMessageBuffer)) {
			*errorMessage = std::string(TempErrorMessageBuffer);
		}
		else
			TempCounter++;

		if(!database.RunQuery(Query, MakeAnyLenString(&Query, "DELETE FROM botinventory WHERE botid = '%u'", this->GetBotID()), TempErrorMessageBuffer)) {
			*errorMessage = std::string(TempErrorMessageBuffer);
		}
		else
			TempCounter++;

		// TODO: alter table botsowners modify botnpctypeid to botid
		//if(!database.RunQuery(Query, MakeAnyLenString(&Query, "DELETE FROM botsowners WHERE botnpctypeid = '%u'", this->GetBotID()), TempErrorMessageBuffer)) {
		//	*errorMessage = std::string(TempErrorMessageBuffer);
		//}
		//else
		//	TempCounter++;

		if(TempCounter == 2)
			Result = true;
	}

	return Result;
}

void Bot::Spawn(Client* botCharacterOwner, std::string* errorMessage) {
	if(this->GetBotID() > 0 && this->_botOwnerCharacterID > 0 && botCharacterOwner) {
		this->SetLevel(botCharacterOwner->GetLevel());
		entity_list.AddBot(this, true, true);
		this->GMMove(botCharacterOwner->GetX(), botCharacterOwner->GetY(), botCharacterOwner->GetZ(), botCharacterOwner->GetHeading(), true);
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
				if(DataRow)
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

/*
void Bot::SetBotOwnerCharacterID(uint32 botOwnerCharacterID, std::string* errorMessage) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if(this->GetBotID() > 0 && botOwnerCharacterID > 0) {
		if(!database.RunQuery(query, MakeAnyLenString(&query, "REPLACE INTO botsowners SET botleadercharacterid = %i, botnpctypeid = %i", botOwnerCharacterID, this->GetBotID()), errbuf)) {
			*errorMessage = std::string(errbuf);
		}
		else
		{
			this->_botOwnerCharacterID = botOwnerCharacterID;
		}

		safe_delete_array(query);
	}
}
*/

void Bot::SetBotItemInSlot(uint32 slotID, uint32 itemID, std::string *errorMessage) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if(this->GetBotID() > 0 && slotID > 0 && itemID > 0) {
		if(!database.RunQuery(query, MakeAnyLenString(&query, "REPLACE INTO botinventory SET botid = %i, slotid = %i, itemid = %i", this->GetBotID(), slotID, itemID), errbuf)) {
			*errorMessage = std::string(errbuf);
		}

		safe_delete_array(query);
	}
}

void Bot::RemoveBotItemBySlot(uint32 slotID, std::string *errorMessage) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if(this->GetBotID() > 0 && slotID > 0) {
		if(!database.RunQuery(query, MakeAnyLenString(&query, "DELETE FROM botinventory WHERE botid=%i AND slotid=%i", this->GetBotID(), slotID), errbuf)){
			*errorMessage = std::string(errbuf);
		}

		safe_delete_array(query);
	}
}

std::list<BotInventory> Bot::GetBotItems(std::string *errorMessage) {
	std::list<BotInventory> Result;

	if(this->GetBotID() > 0) {
		char errbuf[MYSQL_ERRMSG_SIZE];
		char* query = 0;
		MYSQL_RES* DatasetResult;
		MYSQL_ROW DataRow;

		if(database.RunQuery(query, MakeAnyLenString(&query, "SELECT slotid, itemid FROM botinventory WHERE botid=%i order by slotid", this->GetBotID()), errbuf, &DatasetResult)) {
			while(DataRow = mysql_fetch_row(DatasetResult)) {

				BotInventory TempBotInventoryItem;
				TempBotInventoryItem.BotID = this->GetBotID();
				TempBotInventoryItem.BotSlotID = atoi(DataRow[0]);
				TempBotInventoryItem.ItemID = atoi(DataRow[1]);

				Result.push_back(TempBotInventoryItem);
			}

			mysql_free_result(DatasetResult);
		}
		else
			*errorMessage = std::string(errbuf);

		safe_delete_array(query);
	}

	return Result;
}

uint32 Bot::GetBotItemBySlot(uint32 slotID, std::string *errorMessage) {
	uint32 Result = 0;

	if(this->GetBotID() > 0 && slotID > 0) {
		char errbuf[MYSQL_ERRMSG_SIZE];
		char* query = 0;
		MYSQL_RES* DatasetResult;
		MYSQL_ROW DataRow;

		if(database.RunQuery(query, MakeAnyLenString(&query, "SELECT itemid FROM botinventory WHERE botid=%i AND slotid=%i", this->GetBotID(), slotID), errbuf, &DatasetResult)) {
			if(mysql_num_rows(DatasetResult) == 1) {
				DataRow = mysql_fetch_row(DatasetResult);
				if(DataRow)
					Result = atoi(DataRow[0]);
			}

			mysql_free_result(DatasetResult);
		}
		else
			*errorMessage = std::string(errbuf);

		safe_delete_array(query);
	}

	return Result;
}

uint32 Bot::GetBotItemsCount(std::string *errorMessage) {
	uint32 Result = 0;

	if(this->GetBotID() > 0) {
		char errbuf[MYSQL_ERRMSG_SIZE];
		char* query = 0;
		MYSQL_RES* DatasetResult;
		MYSQL_ROW DataRow;

		if(database.RunQuery(query, MakeAnyLenString(&query, "SELECT COUNT(*) FROM botinventory WHERE botid=%i", this->GetBotID()), errbuf, &DatasetResult)) {
			if(mysql_num_rows(DatasetResult) == 1) {
				DataRow = mysql_fetch_row(DatasetResult);
				if(DataRow)
					Result = atoi(DataRow[0]);
			}

			mysql_free_result(DatasetResult);
		}
		else
			*errorMessage = std::string(errbuf);

		safe_delete_array(query);
	}

	return Result;
}

bool Bot::MesmerizeTarget(Mob* target) {
	bool Result = false;

	if(target) {
		int mezid = 0;
		int mezlevel = GetLevel();

		if(mezlevel >= 69) {
			mezid = 5520;
		}
		else if(mezlevel == 68) {
			mezid = 8035;
		}
		else if(mezlevel == 67) {
			mezid = 5503;
		}
		else if(mezlevel >= 64) {
			mezid = 3358;
		}
		else if(mezlevel == 63) {
			mezid = 3354;
		}
		else if(mezlevel >= 61) {
			mezid = 3341;
		}
		else if(mezlevel == 60) {
			mezid = 2120;
		}
		else if(mezlevel == 59) {
			mezid = 1692;
		}
		else if(mezlevel >= 54) {
			mezid = 1691;
		}
		else if(mezlevel >= 47) {
			mezid = 190;
		}
		else if(mezlevel >= 30) {
			mezid = 188;
		}
		else if(mezlevel >= 13) {
			mezid = 187;
		}
		else if(mezlevel >= 2) {
			mezid = 292;
		}
		if(mezid > 0) {
			int32 DontRootMeBeforeTime = 0;
			CastSpell(mezid, target->GetID(), 1, -1, -1, &DontRootMeBeforeTime);
			target->SetDontRootMeBefore(DontRootMeBeforeTime);
			Result = true;
		}
	}

	return Result;
}

void Bot::SetLevel(uint8 in_level, bool command) {
	if(in_level > 0) {
		Mob::SetLevel(in_level, command);
	}
}

void Bot::FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho) {
	if(ns) {
		NPC::FillSpawnStruct(ns, ForWho);
		ns->spawn.is_npc = 1;
	}
}

void Bot::CleanBotLeader(uint32 botOwnerCharacterID, std::string* errorMessage) {
	if(botOwnerCharacterID > 0) {
		char errbuf[MYSQL_ERRMSG_SIZE];
		char *query = 0;

		if(!database.RunQuery(query, MakeAnyLenString(&query, "DELETE FROM botleader where leaderid=%i", botOwnerCharacterID), errbuf)) {
			*errorMessage = std::string(errbuf);
		}

		safe_delete_array(query);
	}
}

Bot* Bot::LoadBot(uint32 botID, std::string* errorMessage) {
	Bot* Result = 0;

	if(botID > 0) {
		char* Query = 0;
		char TempErrorMessageBuffer[MYSQL_ERRMSG_SIZE];
		MYSQL_RES* DatasetResult;
		MYSQL_ROW DataRow;

		if(!database.RunQuery(Query, MakeAnyLenString(&Query, "SELECT BotOwnerCharacterID, BotSpellsID, Name, LastName, BotLevel, Race, Class, BodyType, HitPoints, Gender, Size, HitPointsRegenRate, ManaRegenRate, Face, LuclinHairStyle, LuclinHairColor, LuclinEyeColor, LuclinEyeColor2, LuclinBeardColor, LuclinBeard, DrakkinHeritage, DrakkinTattoo, DrakkinDetails, RunSpeed, MR, CR, DR, FR, PR, AC, STR, STA, DEX, AGI, _INT, WIS, CHA, ATK FROM bots WHERE BotID = '%u'", botID), TempErrorMessageBuffer, &DatasetResult)) {
			*errorMessage = std::string(TempErrorMessageBuffer);
		}
		else {
			while(DataRow = mysql_fetch_row(DatasetResult)) {
				NPCType TempNPCStruct = FillNPCTypeStruct(std::string(DataRow[2]), std::string(DataRow[3]), atoi(DataRow[4]), atoi(DataRow[5]), atoi(DataRow[6]), atoi(DataRow[7]), atoi(DataRow[8]), atoi(DataRow[9]), atof(DataRow[10]), atoi(DataRow[11]), atoi(DataRow[12]), atoi(DataRow[13]), atoi(DataRow[14]), atoi(DataRow[15]), atoi(DataRow[16]), atoi(DataRow[17]), atoi(DataRow[18]), atoi(DataRow[19]), atoi(DataRow[20]), atoi(DataRow[21]), atoi(DataRow[22]), atof(DataRow[23]), atoi(DataRow[24]), atoi(DataRow[25]), atoi(DataRow[26]), atoi(DataRow[27]), atoi(DataRow[28]), atoi(DataRow[29]), atoi(DataRow[30]), atoi(DataRow[31]), atoi(DataRow[32]), atoi(DataRow[33]), atoi(DataRow[34]), atoi(DataRow[35]), atoi(DataRow[36]), atoi(DataRow[37]));
				Result = new Bot(botID, atoi(DataRow[0]), atoi(DataRow[1]), TempNPCStruct);
				break;
			}

			mysql_free_result(DatasetResult);
		}

		safe_delete_array(Query);
	}

	return Result;
}

std::list<BotsAvailableList> Bot::GetBotList(uint32 botOwnerCharacterID, std::string* errorMessage) {
	std::list<BotsAvailableList> Result;

	if(botOwnerCharacterID > 0) {
		char* Query = 0;
		char TempErrorMessageBuffer[MYSQL_ERRMSG_SIZE];
		MYSQL_RES* DatasetResult;
		MYSQL_ROW DataRow;

		if(!database.RunQuery(Query, MakeAnyLenString(&Query, "SELECT BotID, Name, Class, BotLevel, Race FROM bots WHERE BotOwnerCharacterID = '%u'", botOwnerCharacterID), TempErrorMessageBuffer, &DatasetResult)) {
			*errorMessage = std::string(TempErrorMessageBuffer);
		}
		else {
			while(DataRow = mysql_fetch_row(DatasetResult)) {
				if(DataRow) {
					BotsAvailableList TempAvailableBot;
					TempAvailableBot.BotID = atoi(DataRow[0]);
					strcpy(TempAvailableBot.BotName, DataRow[1]);
					TempAvailableBot.BotClass = atoi(DataRow[2]);
					TempAvailableBot.BotLevel = atoi(DataRow[3]);
					TempAvailableBot.BotRace = atoi(DataRow[4]);

					Result.push_back(TempAvailableBot);
				}
			}

			mysql_free_result(DatasetResult);
		}

		safe_delete_array(Query);
	}

	return Result;
}

std::list<SpawnedBotsList> Bot::ListSpawnedBots(uint32 characterID, std::string* errorMessage) {
	std::list<SpawnedBotsList> Result;
	char ErrBuf[MYSQL_ERRMSG_SIZE];
	char* Query = 0;
	MYSQL_RES* DatasetResult;
	MYSQL_ROW DataRow;

	if(characterID > 0) {
		if(!database.RunQuery(Query, MakeAnyLenString(&Query, "SELECT bot_name, zone_name FROM botleader WHERE leaderid=%i", characterID), ErrBuf, &DatasetResult)) {
			*errorMessage = std::string(ErrBuf);
		}
		else {
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
		}

		safe_delete_array(Query);
	}

	return Result;
}

void Bot::SaveBotGroups(uint32 groupID, uint32 characterID, uint32 botID, uint16 slotID, std::string* errorMessage) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if(!database.RunQuery(query, MakeAnyLenString(&query, "INSERT into botgroups (groupid, charid, botid, slot) values (%i, %i, %i, %i)", groupID, characterID, botID, slotID), errbuf)) {
		*errorMessage = std::string(errbuf);
	}

	safe_delete_array(query);
}

void Bot::DeleteBotGroups(uint32 characterID, std::string* errorMessage) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if(!database.RunQuery(query, MakeAnyLenString(&query, "DELETE FROM botgroups where charid=%i", characterID), errbuf)) {
		*errorMessage = std::string(errbuf);
	}

	safe_delete_array(query);
}

std::list<BotGroup> Bot::LoadBotGroups(uint32 characterID, std::string* errorMessage) {
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

					if(DataRow) {
						BotGroup TempBotGroup;
						TempBotGroup.CharacterID = characterID;
						TempBotGroup.GroupID = atoi(DataRow[0]);
						TempBotGroup.BotID = atoi(DataRow[1]);

						Result.push_back(TempBotGroup);
					}
				}
			}

			mysql_free_result(DatasetResult);
		}

		safe_delete_array(Query);
	}

	return Result;
}

uint32 Bot::AllowedBotSpawns(uint32 botOwnerCharacterID, std::string* errorMessage) {
	uint32 Result = 0;

	if(botOwnerCharacterID > 0) {
		char ErrBuf[MYSQL_ERRMSG_SIZE];
		char* Query = 0;
		MYSQL_RES* DatasetResult;
		MYSQL_ROW DataRow;

		if(database.RunQuery(Query, MakeAnyLenString(&Query, "SELECT value FROM quest_globals WHERE name='bot_spawn_limit' and charid=%i", botOwnerCharacterID), ErrBuf, &DatasetResult)) {
			if(mysql_num_rows(DatasetResult) == 1) {
				DataRow = mysql_fetch_row(DatasetResult);
				if(DataRow)
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
	
uint32 Bot::SpawnedBotCount(uint32 botOwnerCharacterID, std::string* errorMessage) {
	uint32 Result = 0;

	if(botOwnerCharacterID > 0) {
		char ErrBuf[MYSQL_ERRMSG_SIZE];
		char* Query = 0;
		MYSQL_RES* DatasetResult;
		MYSQL_ROW DataRow;

		if(database.RunQuery(Query, MakeAnyLenString(&Query, "SELECT COUNT(*) FROM botleader WHERE leaderid=%i", botOwnerCharacterID), ErrBuf, &DatasetResult)) {
			if(mysql_num_rows(DatasetResult) == 1) {
				DataRow = mysql_fetch_row(DatasetResult);
				if(DataRow)
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

uint32 Bot::GetBotOwnerCharacterID(uint32 botID, std::string* errorMessage) {
	uint32 Result = 0;

	if(botID > 0) {
		char ErrBuf[MYSQL_ERRMSG_SIZE];
		char* Query = 0;
		MYSQL_RES* DatasetResult;
		MYSQL_ROW DataRow;

		if(database.RunQuery(Query, MakeAnyLenString(&Query, "SELECT BotOwnerCharacterID FROM bots WHERE BotID = %u", botID), ErrBuf, &DatasetResult)) {
			if(mysql_num_rows(DatasetResult) == 1) {
				if(DataRow = mysql_fetch_row(DatasetResult))
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

std::string Bot::ClassIdToString(uint16 classId) {
	std::string Result;

	if(classId > 0 && classId < 17) {
		switch(classId) {
			case 1:
				Result = std::string("Warrior");
				break;
			case 2:
				Result = std::string("Cleric");
				break;
			case 3:
				Result = std::string("Paladin");
				break;
			case 4:
				Result = std::string("Ranger");
				break;
			case 5:
				Result = std::string("Shadowknight");
				break;
			case 6:
				Result = std::string("Druid");
				break;
			case 7:
				Result = std::string("Monk");
				break;
			case 8:
				Result = std::string("Bard");
				break;
			case 9:
				Result = std::string("Rogue");
				break;
			case 10:
				Result = std::string("Shaman");
				break;
			case 11:
				Result = std::string("Necromancer");
				break;
			case 12:
				Result = std::string("Wizard");
				break;
			case 13:
				Result = std::string("Magician");
				break;
			case 14:
				Result = std::string("Enchanter");
				break;
			case 15:
				Result = std::string("Beastlord");
				break;
			case 16:
				Result = std::string("Berserker");
				break;
		}
	}

	return Result;
}

std::string Bot::RaceIdToString(uint16 raceId) {
	std::string Result;

	if(raceId > 0) {
		switch(raceId) {
			case 1:
				Result = std::string("Human");
				break;
			case 2:
				Result = std::string("Barbarian");
				break;
			case 3:
				Result = std::string("Erudite");
				break;
			case 4:
				Result = std::string("Wood Elf");
				break;
			case 5:
				Result = std::string("High Elf");
				break;
			case 6:
				Result = std::string("Dark Elf");
				break;
			case 7:
				Result = std::string("Half Elf");
				break;
			case 8:
				Result = std::string("Dwarf");
				break;
			case 9:
				Result = std::string("Troll");
				break;
			case 10:
				Result = std::string("Ogre");
				break;
			case 11:
				Result = std::string("Halfling");
				break;
			case 12:
				Result = std::string("Gnome");
				break;
			case 128:
				Result = std::string("Iksar");
				break;
			case 130:
				Result == std::string("Vah Shir");
				break;
			case 330:
				Result = std::string("Froglok");
				break;
		}
	}

	return Result;
}

void Bot::SendBotArcheryWearChange(int8 material_slot, uint32 material, uint32 color) {
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_WearChange, sizeof(WearChange_Struct));
	WearChange_Struct* wc = (WearChange_Struct*)outapp->pBuffer;

	wc->spawn_id = GetID();
	wc->material = material;
	wc->color.color = color;
	wc->wear_slot_id = material_slot;

	entity_list.QueueClients(this, outapp);
	safe_delete(outapp);
}

uint32 Bot::GetCountBotsInGroup(Group *group) {
	uint32 Result = 0;

	if(group) {
		for(int Count = 0; Count < group->GroupCount(); Count++) {
			if(group->members[Count]) {
				if(group->members[Count]->IsBot())
					Result++;
			}
		}
	}

	return Result;
}

void Bot::BotTradeSwapItem(Client* client, sint16 lootSlot, uint32 id, sint16 maxCharges, uint32 equipableSlots, std::string* errorMessage, bool swap) {
	const Item_Struct* itmtmp = database.GetItem(this->GetBotItemBySlot(lootSlot, errorMessage));
	
	if(!errorMessage->empty())
		return;

	const ItemInst* insttmp = new ItemInst(itmtmp, itmtmp->MaxCharges);
	client->PushItemOnCursor(*insttmp, true);
	safe_delete(insttmp);
	this->RemoveBotItemBySlot(lootSlot, errorMessage);

	if(!errorMessage->empty())
		return;

	RemoveItem(itmtmp->ID);
	int8 materialFromSlot = Inventory::CalcMaterialFromSlot(lootSlot);
	if(materialFromSlot != 0xFF) {
		this->BotRemoveEquipItem(materialFromSlot);
		this->SendWearChange(materialFromSlot);
	}
	if(swap) {
		BotTradeAddItem(id, maxCharges, equipableSlots, lootSlot, client, errorMessage);

		if(!errorMessage->empty())
			return;
	}
}

void Bot::BotTradeAddItem(uint32 id, sint16 maxCharges, uint32 equipableSlots, int16 lootSlot, Client* client, std::string* errorMessage, bool addToDb) {
	if(addToDb) {
		this->SetBotItemInSlot(lootSlot, id, errorMessage);
		if(!errorMessage->empty())
			return;
	}
	ServerLootItem_Struct* item = new ServerLootItem_Struct;
	item->item_id = id;
	item->charges = maxCharges;
	item->aug1 = 0;
	item->aug2 = 0;
	item->aug3 = 0;
	item->aug4 = 0;
	item->aug5 = 0;
	item->equipSlot = equipableSlots;
	item->lootslot = lootSlot;
	this->itemlist.push_back(item);
	int8 materialFromSlot = Inventory::CalcMaterialFromSlot(lootSlot);
	if(materialFromSlot != 0xFF) {
		this->BotAddEquipItem(materialFromSlot, id);
		this->SendWearChange(materialFromSlot);
	}
}

bool Bot::Bot_Command_Resist(int resisttype, int level) {
	int resistid = 0;
	switch(resisttype) {
		case 1: // Poison Cleric
			if(level >= 30) {
				resistid = 62;
			}
			else if(level >= 6) {
				resistid = 227;
			}
			break;
		case 2: // Disease Cleric
			if(level >= 36) {
				resistid = 63;
			}
			else if(level >= 11) {
				resistid = 226;
			}
			break;
		case 3: // Fire Cleric
			if(level >= 33) {
				resistid = 60;
			}
			else if(level >= 8) {
				resistid = 224;
			}
			break;
		case 4: // Cold Cleric
			if(level >= 38) {
				resistid = 61;
			}
			else if(level >= 13) {
				resistid = 225;
			}
			break;
		case 5: // Magic Cleric
			if(level >= 43) {
				resistid = 64;
			}
			else if(level >= 16) {
				resistid = 228;
			}
			break;
		case 6: // Magic Enchanter
			if(level >= 37) {
				resistid = 64;
			}
			else if(level >= 17) {
				resistid = 228;
			}
			break;
		case 7: // Poison Druid
			if(level >= 44) {
				resistid = 62;
			}
			else if(level >= 19) {
				resistid = 227;
			}
			break;
		case 8: // Disease Druid
			if(level >= 44) {
				resistid = 63;
			}
			else if(level >= 19) {
				resistid = 226;
			}
			break;
		case 9: // Fire Druid
			if(level >= 20) {
				resistid = 60;
			}
			else if(level >= 1) {
				resistid = 224;
			}
			break;
		case 10: // Cold Druid
			if(level >= 30) {
				resistid = 61;
			}
			else if(level >= 9) {
				resistid = 225;
			}
			break;
		case 11: // Magic Druid
			if(level >= 49) {
				resistid = 64;
			}
			else if(level >= 34) {
				resistid = 228;
			}
			break;
		case 12: // Poison Shaman
			if(level >= 35) {
				resistid = 62;
			}
			else if(level >= 20) {
				resistid = 227;
			}
			break;
		case 13: // Disease Shaman
			if(level >= 30) {
				resistid = 63;
			}
			else if(level >= 8) {
				resistid = 226;
			}
			break;
		case 14: // Fire Shaman
			if(level >= 27) {
				resistid = 60;
			}
			else if(level >= 5) {
				resistid = 224;
			}
			break;
		case 15: // Cold Shaman
			if(level >= 24) {
				resistid = 61;
			}
			else if(level >= 1) {
				resistid = 225;
			}
			break;
		case 16: // Magic Shaman
			if(level >= 43) {
				resistid = 64;
			}
			else if(level >= 19) {
				resistid = 228;
			}
			break;
	}
	if(resistid > 0) {
		if(IsBotRaiding()) {
			// TODO: Uncomment this block after bot raids have been integrated
			/*BotRaids* br = entity_list.GetBotRaidByMob(this);
			if(br) {
				for(int i=0; i<MAX_BOT_RAID_GROUPS; i++) {
					Group* gr = br->BotRaidGroups[i];
					if(gr) {
						for(int j=0; j<MAX_GROUP_MEMBERS; j++) {
							if(gr->members[j]) {
								SpellOnTarget(resistid, gr->members[j]);
							}
						}
					}
				}
			}*/
		}
		else {
			Group* g = entity_list.GetGroupByMob(this);
			if(g) {
				for(int k=0; k<MAX_GROUP_MEMBERS; k++) {
					if(g->members[k]) {
						SpellOnTarget(resistid, g->members[k]);
					}
				}
				return true;
			}
		}
	}
	return false;
}

bool Bot::AddBotToGroup(Bot* bot, Group* group) {
	bool Result = false;

	int i = 0;

	if(bot && group) {
		//Let's see if the bot is already in the group
		for(i = 0; i < MAX_GROUP_MEMBERS; i++) {
			if(group->members[i] && !strcasecmp(group->members[i]->GetCleanName(), bot->GetCleanName()))
				return false;
		}

		// Put the bot in the group
		for(i = 0; i < MAX_GROUP_MEMBERS; i++) {
			if(group->members[i] == NULL) {
				group->members[i] = bot;
				break;
			}
		}
		
		// We copy the bot name in the group at the slot of the bot
		strcpy(group->membername[i], bot->GetCleanName());
		bot->SetGrouped(true);

		//build the template join packet
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
		GroupJoin_Struct* gj = (GroupJoin_Struct*) outapp->pBuffer;	
		strcpy(gj->membername, bot->GetCleanName());
		gj->action = groupActJoin;
	
		int z = 1;
		for(i=0; i < MAX_GROUP_MEMBERS; i++) {
			if(group->members[i] && group->members[i]->IsClient()) {
				if(group->IsLeader(group->members[i])) {
					strcpy(gj->yourname, group->members[i]->GetName());
					strcpy(group->members[i]->CastToClient()->GetPP().groupMembers[0], group->members[i]->GetName());
					group->members[i]->CastToClient()->QueuePacket(outapp);
				}
				else {
					strcpy(group->members[i]->CastToClient()->GetPP().groupMembers[0+z], group->members[i]->GetName());
					group->members[i]->CastToClient()->QueuePacket(outapp);
				}
			}
			z++;
		}

		safe_delete(outapp);

		// Need to send this only once when a group is formed with a bot so the client knows it is also the group leader
		if(group->GroupCount() == 2) {
			Mob *TempLeader = group->GetLeader();
			group->SendUpdate(groupActUpdate, TempLeader);
		}

		Result = true;
	}

	return Result;
}

bool Bot::Bot_Command_CharmTarget(int charmtype, Mob *target) {
	int charmid = 0;
	int charmlevel = GetLevel();
	if(target) {
		switch(charmtype) {
			case 1: // Enchanter
				if((charmlevel >= 64) && (charmlevel <= 75)) {
					charmid = 3355;
				}
				else if((charmlevel >= 62) && (charmlevel <= 63)) {
					charmid = 3347;
				}
				else if((charmlevel >= 60) && (charmlevel <= 61)) {
					charmid = 1707;
				}
				else if((charmlevel >= 53) && (charmlevel <= 59)) {
					charmid = 1705;
				}
				else if((charmlevel >= 37) && (charmlevel <= 52)) {
					charmid = 183;
				}
				else if((charmlevel >= 23) && (charmlevel <= 36)) {
					charmid = 182;
				}
				else if((charmlevel >= 11) && (charmlevel <= 22)) {
					charmid = 300;
				}
				break;
			case 2: // Necromancer
				if((charmlevel >= 60) && (charmlevel <= 75)) {
					charmid = 1629;
				}
				else if((charmlevel >=47) && (charmlevel <= 59)) {
					charmid = 198;
				}
				else if((charmlevel >= 31) && (charmlevel <= 46)) {
					charmid = 197;
				}
				else if((charmlevel >= 18) && (charmlevel <= 30)) {
					charmid = 196;
				}
				break;
			case 3: // Druid
				if((charmlevel >= 63) && (charmlevel <= 75)) {
					charmid = 3445;
				}
				else if((charmlevel >= 55) && (charmlevel <= 62)) {
					charmid = 1556;
				}
				else if((charmlevel >= 52) && (charmlevel <= 54)) {
					charmid = 1553;
				}
				else if((charmlevel >= 43) && (charmlevel <= 51)) {
					charmid = 142;
				}
				else if((charmlevel >= 33) && (charmlevel <= 42)) {
					charmid = 141;
				}
				else if((charmlevel >= 23) && (charmlevel <= 32)) {
					charmid = 260;
				}
				else if((charmlevel >= 13) && (charmlevel <= 22)) {
					charmid = 242;
				}
				break;
		}
		if(charmid > 0) {
			int32 DontRootMeBeforeTime = 0;
			CastSpell(charmid, target->GetID(), 1, -1, -1, &DontRootMeBeforeTime);
			target->SetDontRootMeBefore(DontRootMeBeforeTime);
			return true;
		}
	}
	return false;
}

bool Bot::Bot_Command_DireTarget(int diretype, Mob *target) {
	int direid = 0;
	int direlevel = GetLevel();
	if(target) {
		switch(diretype) {
			case 1: // Enchanter
				if(direlevel >= 65) {
					direid = 5874;
				}
				else if(direlevel >= 55) {
					direid = 2761;
				}
				break;
			case 2: // Necromancer
				if(direlevel >= 65) {
					direid = 5876;
				}
				else if(direlevel >= 55) {
					direid = 2759;
				}
				break;
			case 3: // Druid
				if(direlevel >= 65) {
					direid = 5875;
				}
				else if(direlevel >= 55) {
					direid = 2760;
				}
				break;
		}
		if(direid > 0) {
			int32 DontRootMeBeforeTime = 0;
			CastSpell(direid, target->GetID(), 1, -1, -1, &DontRootMeBeforeTime);
			target->SetDontRootMeBefore(DontRootMeBeforeTime);
			return true;
		}
	}
	return false;
}

bool Bot::Bot_Command_CalmTarget(Mob *target) {
	if(target) {
		int calmid = 0;
		int calmlevel = GetLevel();
		if((calmlevel >= 67) && (calmlevel <= 75)) {
			calmid = 5274;
		}
		else if((calmlevel >= 62) && (calmlevel <= 66)) {
			calmid = 3197;
		}
		else if((calmlevel >= 35) && (calmlevel <= 61)) {
			calmid = 45;
		}
		else if((calmlevel >= 18) && (calmlevel <= 34)) {
			calmid = 47;
		}
		else if((calmlevel >= 6) && (calmlevel <= 17)) {
			calmid = 501;
		}
		else if((calmlevel >= 1) && (calmlevel <= 5)) {
			calmid = 208;
		}
		if(calmid > 0) {
			int32 DontRootMeBeforeTime = 0;
			CastSpell(calmid, target->GetID(), 1, -1, -1, &DontRootMeBeforeTime);
			target->SetDontRootMeBefore(DontRootMeBeforeTime);
			return true;
		}
	}
	return false;
}

bool Bot::Bot_Command_RezzTarget(Mob *target) {
	if(target) {
		int rezid = 0;
		int rezlevel = GetLevel();
		if(rezlevel >= 56) {
			rezid = 1524;
		}
		else if(rezlevel >= 47) {
			rezid = 392;
		}
		else if(rezlevel >= 42) {
			rezid = 2172;
		}
		else if(rezlevel >= 37) {
			rezid = 388;
		}
		else if(rezlevel >= 32) {
			rezid = 2171;
		}
		else if(rezlevel >= 27) {
			rezid = 391;
		}
		else if(rezlevel >= 22) {
			rezid = 2170;
		}
		else if(rezlevel >= 18) {
			rezid = 2169;
		}
		if(rezid > 0) {
			int32 DontRootMeBeforeTime = 0;
			CastSpell(rezid, target->GetID(), 1, -1, -1, &DontRootMeBeforeTime);
			target->SetDontRootMeBefore(DontRootMeBeforeTime);
			return true;
		}
	}
	return false;
}

bool Bot::Bot_Command_Cure(int curetype, int level) {
	int cureid = 0;
	switch(curetype) {
		case 1: // Poison
			if(level >= 58) {
				cureid = 1525;
			}
			else if(level >= 48) {
				cureid = 97;
			}
			else if(level >= 22) {
				cureid = 95;
			}
			else if(level >= 1) {
				cureid = 203;
			}
			break;
		case 2: // Disease
			if(level >= 51) {
				cureid = 3693;
			}
			else if(level >= 28) {
				cureid = 96;
			}
			else if(level >= 4) {
				cureid = 213;
			}
			break;
		case 3: // Curse
			if(level >= 54) {
				cureid = 2880;
			}
			else if(level >= 38) {
				cureid = 2946;
			}
			else if(level >= 23) {
				cureid = 4057;
			}
			else if(level >= 8) {
				cureid = 4056;
			}
			break;
		case 4: // Blindness
			if(level >= 3) {
				cureid = 212;
			}
			break;
	}
	if(cureid > 0) {
		if(IsBotRaiding()) {
			// TODO: Uncomment this block of code after bot raids are integrated
			/*BotRaids* br = entity_list.GetBotRaidByMob(this);
			if(br) {
				for(int i=0; i<MAX_BOT_RAID_GROUPS; i++) {
					Group* gr = br->BotRaidGroups[i];
					if(gr) {
						for(int j=0; j<MAX_GROUP_MEMBERS; j++) {
							if(gr->members[j]) {
								SpellOnTarget(cureid, gr->members[j]);
							}
						}
					}
				}
			}*/
		}
		else {
			Group* g = entity_list.GetGroupByMob(this);
			if(g) {
				for(int k=0; k<MAX_GROUP_MEMBERS; k++) {
					if(g->members[k]) {
						SpellOnTarget(cureid, g->members[k]);
					}
				}
				return true;
			}
		}
	}
	return false;
}

// This funcion is a bit of a hack.
// Ideally, this function should identify the desired buff by spell effect (SE) type. Like SE_Calm for example, not by specific spell id.
// TODO: reimplement this function so no spell type id is hard-coded and instead the buff is identify by spell effect id.
//bool Bot::IsPacified(Mob* targetMob) {
//	bool Result = false;
//
//	if(targetMob && GetBotOwner() && spells_loaded) {
//		for (int i=0; i < BUFF_COUNT; i++) {
//			if ((buffs[i].spellid == 3197) || (buffs[i].spellid == 45) || (buffs[i].spellid == 47) || (buffs[i].spellid == 501) || (buffs[i].spellid == 208)) {
//				Result = true;
//			}
//		}
//	}
//
//	return Result;
//}

void Bot::FinishTrade(Mob* tradingWith) {
	if(tradingWith && tradingWith->IsClient()) {
		Client* client = tradingWith->CastToClient();
		if(client) {
			int32 items[4]={0};
			int8 charges[4]={0};

			bool botCanWear[4] = {0};
			bool BotCanWear = false;
			for (sint16 i=3000; i<=3003; i++){
				BotCanWear = false;
				botCanWear[i-3000] = BotCanWear;

				Inventory& clientInventory = client->GetInv();
				const ItemInst* inst = clientInventory[i];
				if (inst) {
					items[i-3000]=inst->GetItem()->ID;
					charges[i-3000]=inst->GetCharges();
				}
				//EQoffline: will give the items to the bots and change the bot stats
				if(inst && this->GetBotOwner() == client->CastToMob()) {
					std::string TempErrorMessage;
					const Item_Struct* mWeaponItem = inst->GetItem();
					if(mWeaponItem && inst->IsEquipable(GetBaseRace(), GetClass()) && (GetLevel() >= mWeaponItem->ReqLevel)) { // Angelox
						BotCanWear = true;
						botCanWear[i-3000] = BotCanWear;

						const char* equipped[22] = {"Charm", "Left Ear", "Head", "Face", "Right Ear", "Neck", "Shoulders", "Arms", "Back",
							"Left Wrist", "Right Wrist", "Range", "Hands", "Primary Hand", "Secondary Hand",
							"Left Finger", "Right Finger", "Chest", "Legs", "Feet", "Waist", "Ammo" };
						for(int j=0;j<22;j++) {
							if(inst->IsSlotAllowed(j)) {
								if(j==SLOT_EAR01 || j==SLOT_EAR02) { // earrings
									if(GetBotItemBySlot(SLOT_EAR02, &TempErrorMessage) == 0) {
										// If the right ear is empty lets put the earring there
										BotTradeAddItem(mWeaponItem->ID, mWeaponItem->MaxCharges, mWeaponItem->Slots, SLOT_EAR02, client, &TempErrorMessage);
									}
									else if(GetBotItemBySlot(SLOT_EAR01, &TempErrorMessage) == 0) {
										// The right ear is being used, lets put it in the empty left ear
										BotTradeAddItem(mWeaponItem->ID, mWeaponItem->MaxCharges, mWeaponItem->Slots, SLOT_EAR01, client, &TempErrorMessage);
									}
									else {
										// both ears are equipped, so swap out the left ear
										BotTradeSwapItem(client, SLOT_EAR01, inst->GetID(), mWeaponItem->MaxCharges, mWeaponItem->Slots, &TempErrorMessage);
										this->Say("I was using this in my %s but OK, you can have it back.", equipped[SLOT_EAR01]);
									}
									break;
								}
								else if(j==SLOT_BRACER01 || j==SLOT_BRACER02) { // bracers
									if(GetBotItemBySlot(SLOT_BRACER02, &TempErrorMessage) == 0) {
										// If the right wrist is empty lets put the bracer there
										BotTradeAddItem(mWeaponItem->ID, mWeaponItem->MaxCharges, mWeaponItem->Slots, SLOT_BRACER02, client, &TempErrorMessage);
									}
									else if(GetBotItemBySlot(SLOT_BRACER01, &TempErrorMessage) == 0) {
										// The right wrist is equipped, lets put it in the empty left wrist
										BotTradeAddItem(mWeaponItem->ID, mWeaponItem->MaxCharges, mWeaponItem->Slots, SLOT_BRACER01, client, &TempErrorMessage);
									}
									else {
										// both wrists are equipped, so swap out the left wrist
										BotTradeSwapItem(client, SLOT_BRACER01, inst->GetID(), mWeaponItem->MaxCharges, mWeaponItem->Slots, &TempErrorMessage);
										Say("I was using this in my %s but OK, you can have it back.", equipped[SLOT_BRACER01]);
									}
									break;
								}
								else if(j == SLOT_PRIMARY) { // primary melee weapons
									SetBotArcher(false);
									const Item_Struct* itmwp = database.GetItem(inst->GetID());
									if((GetBotItemBySlot(SLOT_PRIMARY, &TempErrorMessage) == 0)) {
										// if the primary hand is empty, lets put the item there
										BotTradeAddItem(mWeaponItem->ID, mWeaponItem->MaxCharges, mWeaponItem->Slots, SLOT_PRIMARY, client, &TempErrorMessage);
										if((itmwp->ItemType == ItemType2HS) || (itmwp->ItemType == ItemType2HB) || (itmwp->ItemType == ItemType2HPierce)) {
											// if the primary item is a two-hander, and the left hand is equipped, lets remove the item in the left hand
											if(GetBotItemBySlot(SLOT_SECONDARY, &TempErrorMessage) != 0) {
												BotTradeSwapItem(client, SLOT_SECONDARY, inst->GetID(), mWeaponItem->MaxCharges, mWeaponItem->Slots, &TempErrorMessage, false);
												Say("I was using this in my %s but OK, you can have it back.", equipped[SLOT_SECONDARY]);
											}
										}
									}
									else if((GetBotItemBySlot(SLOT_PRIMARY, &TempErrorMessage) != 0)) {
										if((itmwp->ItemType == ItemType2HS) || (itmwp->ItemType == ItemType2HB) || (itmwp->ItemType == ItemType2HPierce)) {
											// if the primary hand is equipped and the new item is a two-hander, lets remove the old primary item
											BotTradeSwapItem(client, SLOT_PRIMARY, inst->GetID(), mWeaponItem->MaxCharges, mWeaponItem->Slots, &TempErrorMessage);
											Say("I was using this in my %s but OK, you can have it back.", equipped[SLOT_PRIMARY]);
											if((GetBotItemBySlot(SLOT_SECONDARY, &TempErrorMessage) != 0)) {
												// if the new primary item is a two-hander, and the secondary hand is equipped, remove the secondary hand item
												BotTradeSwapItem(client, SLOT_SECONDARY, inst->GetID(), mWeaponItem->MaxCharges, mWeaponItem->Slots, &TempErrorMessage, false);
												Say("I was using this in my %s but OK, you can have it back.", equipped[SLOT_SECONDARY]);
											}
										}
										else if((GetBotItemBySlot(SLOT_SECONDARY, &TempErrorMessage) == 0) && inst->IsSlotAllowed(SLOT_SECONDARY)) {
											// Make sure to not equip weapons in the offhand of non-dual wielding classes
											if(inst->IsWeapon() && !CanThisClassDualWield()) {
												Say("I cannot dual wield.");
												client->PushItemOnCursor(*inst, true);
												client->DeleteItemInInventory(i);
												return;
											}
											const Item_Struct* itmtmp = database.GetItem(GetBotItemBySlot(SLOT_PRIMARY, &TempErrorMessage));
											if((itmtmp->ItemType == ItemType2HS) || (itmtmp->ItemType == ItemType2HB) || (itmtmp->ItemType == ItemType2HPierce)) {
												// if the primary hand is equpped with a two-hander and the secondary is free, remove the existing primary hand item
												BotTradeSwapItem(client, SLOT_PRIMARY, inst->GetID(), mWeaponItem->MaxCharges, mWeaponItem->Slots, &TempErrorMessage, false);
												Say("I was using this 2 Handed Weapon... but OK, you can have it back.");
											}
											// put the new item in the secondary hand
											BotTradeAddItem(mWeaponItem->ID, mWeaponItem->MaxCharges, mWeaponItem->Slots, SLOT_SECONDARY, client, &TempErrorMessage);
										}
										else if((GetBotItemBySlot(SLOT_SECONDARY, &TempErrorMessage) != 0) && inst->IsSlotAllowed(SLOT_SECONDARY) ) {
											// Make sure to not equip weapons in the offhand of non-dual wielding classes
											if(inst->IsWeapon() && !CanThisClassDualWield()) {
												Say("I cannot dual wield.");
												client->PushItemOnCursor(*inst, true);
												client->DeleteItemInInventory(i);
												return;
											}
											// the primary and secondary hands are equipped, swap out the secondary hand item with the new item
											BotTradeSwapItem(client, SLOT_SECONDARY, inst->GetID(), mWeaponItem->MaxCharges, mWeaponItem->Slots, &TempErrorMessage);
											Say("I was using this in my %s but OK, you can have it back.", equipped[SLOT_SECONDARY]);
										}
										else {
											Say("Use '#bot inventory remove 13' to remove the primary weapon.");
											client->PushItemOnCursor(*inst, true);
											client->DeleteItemInInventory(i);
											return;
										}
									}
									break;
								}
								else if(j == SLOT_SECONDARY) { // Secondary Hand
									SetBotArcher(false);
									// Make sure to not equip weapons in the offhand of non-dual wielding classes
									if(inst->IsWeapon() && !CanThisClassDualWield()) {
										Say("I cannot dual wield.");
										client->PushItemOnCursor(*inst, true);
										client->DeleteItemInInventory(i);
										return;
									}
									const Item_Struct* itmtmp = database.GetItem(GetBotItemBySlot(SLOT_PRIMARY, &TempErrorMessage));
									if(itmtmp && ((itmtmp->ItemType == ItemType2HS) || (itmtmp->ItemType == ItemType2HB) || (itmtmp->ItemType == ItemType2HPierce))) {
										// If the primary hand item is a two-hander, remove it
										BotTradeSwapItem(client, SLOT_PRIMARY, inst->GetID(), mWeaponItem->MaxCharges, mWeaponItem->Slots, &TempErrorMessage, false);
										Say("I was using a 2 Handed weapon... but OK, you can have it back.");
									}
									if((GetBotItemBySlot(SLOT_SECONDARY, &TempErrorMessage) == 0)) {
										// if the secondary hand is free, equip it with the new item
										BotTradeAddItem(mWeaponItem->ID, mWeaponItem->MaxCharges, mWeaponItem->Slots, SLOT_SECONDARY, client, &TempErrorMessage);
									}
									else {
										// The primary and secondary hands are equipped, just swap out the secondary item with the new item
										BotTradeSwapItem(client, SLOT_SECONDARY, inst->GetID(), mWeaponItem->MaxCharges, mWeaponItem->Slots, &TempErrorMessage);
										Say("I was using this in my %s but OK, you can have it back.", equipped[SLOT_SECONDARY]);
									}
									break;
								}
								else if(j==SLOT_RING01 || j==SLOT_RING02) { // rings
									if(GetBotItemBySlot(SLOT_RING02, &TempErrorMessage) == 0) {
										// If the right finger is empty lets put the ring there
										BotTradeAddItem(mWeaponItem->ID, mWeaponItem->MaxCharges, mWeaponItem->Slots, SLOT_RING02, client, &TempErrorMessage);
									}
									else if(GetBotItemBySlot(SLOT_RING01, &TempErrorMessage) == 0) {
										// The right finger is equipped, lets put it on the empty left finger
										BotTradeAddItem(mWeaponItem->ID, mWeaponItem->MaxCharges, mWeaponItem->Slots, SLOT_RING01, client, &TempErrorMessage);
									}
									else {
										// both fingers are equipped, so swap out the left finger
										BotTradeSwapItem(client, SLOT_RING01, inst->GetID(), mWeaponItem->MaxCharges, mWeaponItem->Slots, &TempErrorMessage);
										Say("I was using this in my %s but OK, you can have it back.", equipped[SLOT_RING01]);
									}
									break;
								}
								if((j == SLOT_AMMO) || (j == SLOT_RANGE)) {
									SetBotArcher(false);
								}
								if(GetBotItemBySlot(j, &TempErrorMessage) != 0) {
									// remove existing item if equipped
									BotTradeSwapItem(client, j, inst->GetID(), mWeaponItem->MaxCharges, mWeaponItem->Slots, &TempErrorMessage, false);
									Say("Thanks! Here, take this other one back.");
								}
								// put the item in the slot
								BotTradeAddItem(mWeaponItem->ID, mWeaponItem->MaxCharges, mWeaponItem->Slots, j, client, &TempErrorMessage);
								break;
							}
						}
						CalcBotStats();
					}
				}
				if(inst) {
					if(!botCanWear[i-3000]) {
						client->PushItemOnCursor(*inst, true);
					}
					client->DeleteItemInInventory(i);
				}
			}

			//		if(!with->IsBot()) { // START This is so Bots don't trigger the EVENT_ITEM
			//
			//			for (sint16 i=3000; i<=3003; i++) {
			//				const ItemInst* inst = m_inv[i];
			//				if (inst) {
			//					items[i-3000]=inst->GetItem()->ID;
			//					charges[i-3000]=inst->GetCharges();
			//					DeleteItemInInventory(i);
			//				}
			//			}
			//
			//			//dont bother with this crap unless we have a quest...
			//			//pets can have quests! (especially charmed NPCs)
			//			bool did_quest = false;
			//#ifdef EMBPERL
			//			if(((PerlembParser *)parse)->HasQuestSub(with->GetNPCTypeID(), "EVENT_ITEM")) {
			//#else
			//			if(parse->HasQuestFile(with->GetNPCTypeID())) {
			//#endif
			//				char temp1[100];
			//				memset(temp1,0x0,100);
			//				char temp2[100];
			//				memset(temp2,0x0,100);
			//				for ( int z=0; z < 4; z++ ) {
			//					snprintf(temp1, 100, "item%d.%d", z+1,with->GetNPCTypeID());
			//					snprintf(temp2, 100, "%d",items[z]);
			//					parse->AddVar(temp1,temp2);
			//					//			memset(temp1,0x0,100);
			//					//			memset(temp2,0x0,100);
			//					snprintf(temp1, 100, "item%d.charges.%d", z+1,with->GetNPCTypeID());
			//					snprintf(temp2, 100, "%d",charges[z]);
			//					parse->AddVar(temp1,temp2);
			//					//			memset(temp1,0x0,100);
			//					//			memset(temp2,0x0,100);
			//				}
			//				snprintf(temp1, 100, "copper.%d",with->GetNPCTypeID());
			//				snprintf(temp2, 100, "%i",trade->cp);
			//				parse->AddVar(temp1,temp2);
			//				//		memset(temp1,0x0,100);
			//				//		memset(temp2,0x0,100);
			//				snprintf(temp1, 100, "silver.%d",with->GetNPCTypeID());
			//				snprintf(temp2, 100, "%i",trade->sp);
			//				parse->AddVar(temp1,temp2);
			//				//		memset(temp1,0x0,100);
			//				//		memset(temp2,0x0,100);
			//				snprintf(temp1, 100, "gold.%d",with->GetNPCTypeID());
			//				snprintf(temp2, 100, "%i",trade->gp);
			//				parse->AddVar(temp1,temp2);
			//				//		memset(temp1,0x0,100);
			//				//		memset(temp2,0x0,100);
			//				snprintf(temp1, 100, "platinum.%d",with->GetNPCTypeID());
			//				snprintf(temp2, 100, "%i",trade->pp);
			//				parse->AddVar(temp1,temp2);
			//				//		memset(temp1,0x0,100);
			//				//		memset(temp2,0x0,100);
			//				parse->Event(EVENT_ITEM, with->GetNPCTypeID(), NULL, with, this);
			//				did_quest = true;
			//			}
			//			if(RuleB(TaskSystem, EnableTaskSystem)) {
			//				int Cash = trade->cp + (trade->sp * 10) + (trade->gp * 100) + (trade->pp * 1000);
			//				if(UpdateTasksOnDeliver(items, Cash, with->GetNPCTypeID())) {
			//					if(!with->IsMoving()) 
			//						with->FaceTarget(this);
			//				}
			//			}
			//			//		Message(0, "Normal NPC: keeping items.");
			//
			//			//else, we do not have a quest, give the items to the NPC
			//			if(did_quest) {
			//				//only continue if we are a charmed NPC
			//				if(!with->HasOwner() || with->GetPetType() != petCharmed)
			//					return;
			//			}
			//		} // END This is so Bots don't trigger the EVENT_ITEM

			int xy = CountLoot();

			for(int y=0; y < 4; y++) {
				if(xy >= 23) {
					break;
				}
				// TODO: Figure out whats with the xy-- followed by xy++... its literally a 0 sum calculation.
				xy--;
				//if(with->IsBot()) { // The xy++ below doesn't work for bot trading.
				//	if(xy >= 23) {
				//		break;
				//	}
				//	xy--;
				//}
				//else {
				//	if (xy >= 20)
				//		break;
				//}

				xy++;
				//NPC* npc=with->CastToNPC();
				const Item_Struct* item2 = database.GetItem(items[y]);
				if (item2) {
					if(!botCanWear[y]) {
						Say("Thank you for the %s, %s.", item2->Name,  client->GetName());
					}
					else {
						Say("I can't use this %s!", item2->Name);
					}

					if(item2->NoDrop != 0)
						AddLootDrop(item2, &itemlist, charges[y], true, true);

					//if((GetGM() && !with->IsBot()) || ((item2->NoDrop != 0) && !with->IsBot()))
					//	with->AddLootDrop(item2, &with->itemlist, charges[y], true, true);
					//// franck-add: you can give nodrop items to bots
					//else if(with->IsBot() && botCanWear[y]) {
					//	with->Say("Thank you for the %s, %s.", item2->Name,  this->GetName());
					//}
					//else if(with->IsBot() && !botCanWear[y]) {
					//	with->Say("I can't use this %s!", item2->Name);
					//}

					////if was not no drop item, let the NPC have it
					//if(GetGM() || item2->NoDrop != 0)
					//	with->AddLootDrop(item2, &with->itemlist, charges[y], true, true);
					//else 
					//	with->AddLootDrop(item2, NULL, charges[y], false, true);
				}
			}
		}
	}
}

void Bot::Death(Mob *killerMob, sint32 damage, int16 spell_id, SkillType attack_skill) {
	NPC::Death(killerMob, damage, spell_id, attack_skill);

	Mob *give_exp = hate_list.GetDamageTop(this);
	Client *give_exp_client = NULL;

	if(give_exp && give_exp->IsClient())
		give_exp_client = give_exp->CastToClient();

	bool IsLdonTreasure = (this->GetClass() == LDON_TREASURE);

	// Uncomment after bot raiding is ready and a decision on cleanbotleader() is made

	//if (give_exp_client && !IsCorpse() && MerchantType == 0)
	//{
	//	Group *kg = entity_list.GetGroupByClient(give_exp_client);
	//	Raid *kr = entity_list.GetRaidByClient(give_exp_client);

	//	if(!kr && give_exp_client->IsClient() && give_exp_client->IsBotRaiding())
	//	{
	//		BotRaids *br = entity_list.GetBotRaidByMob(give_exp_client->CastToMob());
	//		if(br)
	//		{
	//			if(!IsLdonTreasure)
	//				br->SplitExp((EXP_FORMULA), this);

	//			if(br->GetBotMainTarget() == this)
	//				br->SetBotMainTarget(NULL);

	//			/* Send the EVENT_KILLED_MERIT event for all raid members */
	//			if(br->BotRaidGroups[0])
	//			{
	//				for(int j=0; j<MAX_GROUP_MEMBERS; j++)
	//				{
	//					if(br->BotRaidGroups[0]->members[j] && br->BotRaidGroups[0]->members[j]->IsClient())
	//					{
	//						parse->Event(EVENT_KILLED_MERIT, GetNPCTypeID(), "killed", this, br->BotRaidGroups[0]->members[j]);
	//						if(RuleB(TaskSystem, EnableTaskSystem))
	//						{
	//							br->BotRaidGroups[0]->members[j]->CastToClient()->UpdateTasksOnKill(GetNPCTypeID());
	//						}
	//					}
	//				}
	//			}
	//		}
	//	}
	//}

	//corpse->Depop();

	//Group *g = entity_list.GetGroupByMob(this);
	//if(g) {
	//	for(int i=0; i<MAX_GROUP_MEMBERS; i++) {
	//		if(g->members[i]) {
	//			if(g->members[i] == this) {
	//				// If the leader dies, make the next bot the leader
	//				// and reset all bots followid
	//				if(g->IsLeader(g->members[i])) {
	//					if(g->members[i+1]) {
	//						g->SetLeader(g->members[i+1]);
	//						g->members[i+1]->SetFollowID(g->members[i]->GetFollowID());
	//						for(int j=0; j<MAX_GROUP_MEMBERS; j++) {
	//							if(g->members[j] && (g->members[j] != g->members[i+1])) {
	//								g->members[j]->SetFollowID(g->members[i+1]->GetID());
	//							}
	//						}
	//					}
	//				}

	//				// delete from group data
	//				g->membername[i][0] = '\0';
	//				memset(g->membername[i], 0, 64);
	//				g->members[i]->BotOwner = NULL;
	//				g->members[i] = NULL;

	//				// if group members exist below this one, move
	//				// them all up one slot in the group list
	//				int j = i+1;
	//				for(; j<MAX_GROUP_MEMBERS; j++) {
	//					if(g->members[j]) {
	//						g->members[j-1] = g->members[j];
	//						strcpy(g->membername[j-1], g->members[j]->GetName());
	//						g->membername[j][0] = '\0';
	//						memset(g->membername[j], 0, 64);
	//						g->members[j] = NULL;
	//					}
	//				}

	//				// update the client group
	//				EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
	//				GroupJoin_Struct* gu = (GroupJoin_Struct*)outapp->pBuffer;
	//				gu->action = groupActLeave;
	//				strcpy(gu->membername, GetName());
	//				if(g) {
	//					for(int k=0; k<MAX_GROUP_MEMBERS; k++) {
	//						if(g->members[k] && g->members[k]->IsClient())
	//							g->members[k]->CastToClient()->QueuePacket(outapp);
	//					}
	//				}
	//				safe_delete(outapp);

	//				// now that's done, lets see if all we have left is the client
	//				// and we can clean up the clients raid group and group
	//				if(IsBotRaiding()) {
	//					BotRaids* br = entity_list.GetBotRaidByMob(this);
	//					if(br) {
	//						if(this == br->botmaintank) {
	//							br->botmaintank = NULL;
	//						}
	//						if(this == br->botsecondtank) {
	//							br->botsecondtank = NULL;
	//						}
	//					}
	//					if(g->BotGroupCount() == 0) {
	//						int32 gid = g->GetID();
	//						if(br) {
	//							br->RemoveEmptyBotGroup();
	//						}
	//						entity_list.RemoveGroup(gid);
	//					}
	//					if(br && (br->RaidBotGroupsCount() == 1)) {
	//						br->RemoveClientGroup(br->GetRaidBotLeader());
	//					}
	//					if(br && (br->RaidBotGroupsCount() == 0)) {
	//						br->DisbandBotRaid();
	//					}
	//				}
	//			}
	//		}
	//	}
	//}

	//// Delete from database
	//database.CleanBotLeaderEntries(this->GetBotID());
	entity_list.RemoveNPC(this->GetID());
}

void Bot::Damage(Mob *from, sint32 damage, int16 spell_id, SkillType attack_skill, bool avoidable, sint8 buffslot, bool iBuffTic) {
	NPC::Damage(from, damage, spell_id, attack_skill, avoidable, buffslot, iBuffTic);

	// franck-add: when a bot takes some dmg, its leader must see it in the group HP bar
	if(IsGrouped() && GetHP() > 0) {
		Group *g = entity_list.GetGroupByMob(this);
		if(g) {
			EQApplicationPacket hp_app;
			CreateHPPacket(&hp_app);
			for(int i=0; i<MAX_GROUP_MEMBERS; i++) {
				if(g->members[i] && g->members[i]->IsClient()) {
					g->members[i]->CastToClient()->QueuePacket(&hp_app);
				}
			}
		}
	}
}

bool Bot::Attack(Mob* other, int Hand, bool FromRiposte) {
	return this->BotAttackMelee(other, Hand, FromRiposte);
}

void Bot::AddToHateList(Mob* other, sint32 hate, sint32 damage, bool iYellForHelp, bool bFrenzy, bool iBuffTic) {
	if(other && other != this && other != this->GetBotOwner() && other != GetOwner() && !SpecAttacks[IMMUNE_AGGRO] && !other->SpecAttacks[IMMUNE_TARGET]) {
		// TODO: Evaluate "GetBotOwner()->CastToClient()->IsOrderBotAttack()"
		/*if(GetBotOwner() && !GetBotOwner()->CastToClient()->IsOrderBotAttack()) {
				return;
		}*/

		CommonAddToHateList(other, hate, damage, iYellForHelp, bFrenzy, iBuffTic);

		if(GetBotOwner() && GetBotOwner()->CastToClient()->GetFeigned()) {
			AddFeignMemory(GetBotOwner()->CastToClient());
		}
		else {
			if(!hate_list.IsOnHateList(GetBotOwner()))
				hate_list.Add(GetBotOwner(), 0, 0, false, true);
		}
	}
}

void Bot::ProcessBotCommands(Client *c, const Seperator *sep) {
	// TODO: All bot command processing occurs here now instead of in command.cpp

	// TODO: Log any possible error messages as most of these will be MySQL error messages.
	std::string TempErrorMessage;

	if(sep->arg[1][0] == '\0') {
		c->Message(13, "Bad argument, type #bot help");
		return;
	}
	if(!strcasecmp( sep->arg[1], "help") && !strcasecmp( sep->arg[2], "\0")){
		c->Message(0, "List of commands availables for bots :");
		c->Message(0, "#bot help - show this");
		c->Message(0, "#bot create [name] [class (id)] [race (id)] [model (male/female)] - create a permanent bot. See #bot help create.");
		c->Message(0, "#bot help create - show all the race/class id. (make it easier to create bots)");
		c->Message(0, "#bot delete - completely destroy forever the targeted bot and all its items.");
		c->Message(0, "#bot list [all/class(1-16)] - list your bots all or by class. Classes: 1(Warrior), 2(Cleric), 3(Paladin), 4(Ranger), 5(Sk), 6(Druid), 7(Monk), 8(Bard), 9(Rogue), 10(Shaman), 11(Necro), 12(Wiz), 13(Mag), 14(Ench), 15(Beast), 16(Bersek)");
		c->Message(0, "#bot spawn [botid] - spawn a bot from its ID (use list to see all the bots). ");
		c->Message(0, "#bot group add - make the targetted bot joigning your group.");
		c->Message(0, "#bot group remove [target} - kick the targetted bot from your group (it will die also).");
		c->Message(0, "#bot group order [follow/guard/attack (target)] - Give orders [follow/guard/attack (target)] to your grouped bots.");
		c->Message(0, "#bot inventory list - show the inventory (and the slots IDs) of the targetted bot.");
		c->Message(0, "#bot inventory remove [slotid] - remove the item at the given slot in the inventory of the targetted bot.");
		c->Message(0, "#bot update - you must type that command once you gain a level.");
		c->Message(0, "#bot group summon - It will summon all your grouped bots to you.");
		c->Message(0, "#bot summon - It will summon your targeted bot to you.");
		c->Message(0, "#bot ai mez - If you're grouped with an enchanter, he will mez your target.");
		c->Message(0, "#bot picklock - You must have a targeted rogue bot in your group and be right on the door.");
		c->Message(0, "#bot cure [poison|disease|curse|blindness] Cleric has most options");
		c->Message(0, "#bot bindme - You must have a Cleric in your group to get Bind Affinity cast on you.");
		c->Message(0, "#bot raid [commands] (#bot raid help will show some help).");
		c->Message(0, "#bot track - look at mobs in the zone (ranger has options)");
		c->Message(0, "#bot target calm - attempts to pacify your target mob.");
		c->Message(0, "#bot evac - transports your pc group to safe location in the current zone. bots are lost");
		c->Message(0, "#bot resurrectme - Your bot Cleric will rez you.");
		c->Message(0, "#bot corpse summon - Necromancers summon corpse.");
		c->Message(0, "#bot lore - cast Identify on the item on your mouse pointer.");
		c->Message(0, "#bot sow - Bot sow on you (Druid has options)");
		c->Message(0, "#bot invis - Bot invisiblity (must have proper class in group)");
		c->Message(0, "#bot levitate - Bot levitation (must have proper class in group)");
		c->Message(0, "#bot resist - Bot resist buffs (must have proper class in group)");
		c->Message(0, "#bot runeme - Enchanter Bot cast Rune spell on you");
		c->Message(0, "#bot shrinkme - Shaman Bot will shrink you");
		c->Message(0, "#bot endureb - Bot enduring breath (must have proper class in group)");
		c->Message(0, "#bot charm - (must have proper class in group)");
		c->Message(0, "#bot dire charm - (must have proper class in group)");
		c->Message(0, "#bot pet remove - (remove pet before charm)");
		c->Message(0, "#bot gate - you need a Druid or Wizard in your group)");
		c->Message(0, "#bot saveraid - Save your current group(s) of bots.");
		c->Message(0, "#bot spawnraid - Spawns your saved bots.");
		c->Message(0, "#bot groupraid - Groups your spawned bots.");
		c->Message(0, "#bot archery - Toggle Archery Skilled bots between using a Bow or using Melee weapons.");
		c->Message(0, "#bot magepet [earth|water|air|fire|monster] - Select the pet type you want your Mage bot to use.");
		return;
	}

	if(!strcasecmp(sep->arg[1], "create")) {
		if(sep->arg[2][0] == '\0' || sep->arg[3][0] == '\0' || sep->arg[4][0] == '\0' || sep->arg[5][0] == '\0' || sep->arg[6][0] != '\0') {
			c->Message(0, "Usage: #bot create [name] [class(id)] [race(id)] [gender (male/female)]");
			return;
		}
		else if(strcasecmp(sep->arg[3],"1") && strcasecmp(sep->arg[3],"2") && strcasecmp(sep->arg[3],"3") && strcasecmp(sep->arg[3],"4") && strcasecmp(sep->arg[3],"5") && strcasecmp(sep->arg[3],"6") && strcasecmp(sep->arg[3],"7") && strcasecmp(sep->arg[3],"8") && strcasecmp(sep->arg[3],"9") && strcasecmp(sep->arg[3],"10") && strcasecmp(sep->arg[3],"11") && strcasecmp(sep->arg[3],"12") && strcasecmp(sep->arg[3],"13") && strcasecmp(sep->arg[3],"14") && strcasecmp(sep->arg[3],"15") && strcasecmp(sep->arg[3],"16")) {
			c->Message(0, "Usage: #bot create [name] [class(id)] [race(id)] [gender (male/female)]");
			return;
		}		
		else if(strcasecmp(sep->arg[4],"1") && strcasecmp(sep->arg[4],"2") && strcasecmp(sep->arg[4],"3") && strcasecmp(sep->arg[4],"4") && strcasecmp(sep->arg[4],"5") && strcasecmp(sep->arg[4],"6") && strcasecmp(sep->arg[4],"7") && strcasecmp(sep->arg[4],"8") && strcasecmp(sep->arg[4],"9") && strcasecmp(sep->arg[4],"10") && strcasecmp(sep->arg[4],"11") && strcasecmp(sep->arg[4],"12") && strcasecmp(sep->arg[4],"330") && strcasecmp(sep->arg[4],"128") && strcasecmp(sep->arg[4],"130")) {
			c->Message(0, "Usage: #bot create [name] [class(1-16)] [race(1-12,128,130,330)] [gender (male/female)]");
			return;
		}
		else if(strcasecmp(sep->arg[5],"male") && strcasecmp(sep->arg[5],"female")) {
			c->Message(0, "Usage: #bot create [name] [class(1-16)] [race(1-12,128,130,330)] [gender (male/female)]");
			return;
		}

		int32 MaxBotCreate = RuleI(Bots, CreateBotCount);
		if(SpawnedBotCount(c->CharacterID(), &TempErrorMessage) >= MaxBotCreate) {
			c->Message(0, "You cannot create more than %i bots.", MaxBotCreate);
			return;
		}

		if(!TempErrorMessage.empty()) {
			c->Message(13, "Database Error: %s", TempErrorMessage.c_str());
			return;
		}

		int gender = 0;
		if(!strcasecmp(sep->arg[5], "female"))
			gender = 1;

		NPCType DefaultNPCTypeStruct = CreateDefaultNPCTypeStructForBot(std::string(sep->arg[2]), std::string(), c->GetLevel(), atoi(sep->arg[4]), atoi(sep->arg[3]), gender);
		Bot* NewBot = new Bot(DefaultNPCTypeStruct, c);

		if(NewBot) {
			if(!NewBot->IsValidRaceClassCombo()) {
				c->Message(0, "That Race/Class combination cannot be created.");
				return;
			}

			if(!NewBot->IsValidName()) {
				c->Message(0, "%s has invalid characters. You can use only the A-Z, a-z and _ characters in a bot name.", NewBot->GetCleanName());
				return;
			}

			if(!NewBot->IsBotNameAvailable(&TempErrorMessage)) {
				c->Message(0, "The name %s is already being used. Please choose a different name.", NewBot->GetCleanName());
				return;
			}

			if(!TempErrorMessage.empty()) {
				c->Message(13, "Database Error: %s", TempErrorMessage.c_str());
				return;
			}

			// Now that all validation is complete, we can save our newly created bot
			if(!NewBot->Save())
				c->Message(0, "Unable to save %s as a bot.", NewBot->GetCleanName());
			else
				c->Message(0, "%s saved as bot %u.", NewBot->GetCleanName(), NewBot->GetBotID());
		}
		else {
			// TODO: Log error message here
		}

		// Bot creation is complete
		return;
	}

	if(!strcasecmp(sep->arg[1], "help") && !strcasecmp(sep->arg[2], "create") ){
		c->Message(0, "Classes:  1(Warrior), 2(Cleric), 3(Paladin), 4(Ranger), 5(Sk), 6(Druid), 7(Monk), 8(Bard), 9(Rogue), 10(Shaman), 11(Necro), 12(Wiz), 13(Mag), 14(Ench), 15(Beast), 16(Bersek)");
		c->Message(0, "------------------------------------------------------------------");
		c->Message(0, "Races: 1(Human), 2(Barb), 3(Erudit), 4(Wood elf), 5(High elf), 6(Dark elf), 7(Half elf), 8(Dwarf), 9(Troll), 10(Ogre), 11(Halfling), 12(Gnome), 330(Froglok), 128(Iksar), 130(Vah shir)");
		c->Message(0, "------------------------------------------------------------------");
		c->Message(0, "Usage: #bot create [name] [class(1-16)] [race(1-12,128,130,330)] [gender(male/female)]");
		c->Message(0, "Example: #bot create Jesuschrist 9 6 male");
		return;
	}

	if(!strcasecmp(sep->arg[1], "delete") ) {
		if((c->GetTarget() == NULL) || !c->GetTarget()->IsBot())
		{
			c->Message(15, "You must target a bot!");
			return;
		}
		else if(c->GetTarget()->CastToBot()->GetBotOwnerCharacterID() != c->CharacterID())
		{
			c->Message(15, "You can't delete a bot that you don't own.");
			return;
		}

		if(c->GetTarget()->IsBot()) {
			Bot* BotTargeted = c->GetTarget()->CastToBot();

			if(BotTargeted) {
				if(BotTargeted->DeleteBot(&TempErrorMessage)) {
					BotTargeted->Say("...but why?!! We had such good adventures together! gaahhh...glrrrk...");
					// TODO: decide on BotOwner = NULL
					//c->GetTarget()->BotOwner = NULL;

					if(BotTargeted->IsGrouped()) {
						Group *g = entity_list.GetGroupByMob(c->GetTarget());
						if(g) {
							g->DelMember(BotTargeted);
						}
					}
					else {
						Group *g = entity_list.GetGroupByMob(c->GetTarget());
						if(g && g->GroupCount() == 2) {
							g->DisbandGroup();
							// TODO: Client group window now shows a no members but the "Disband" button is still enabled.
						}
					}

					BotTargeted->Kill();
				}
				else {
					// TODO: log error message here
					c->Message(15, "Error deleting Bot!");
				}

				if(!TempErrorMessage.empty()) {
					c->Message(13, "Database Error: %s", TempErrorMessage.c_str());
					return;
				}
			}
		}

		return;
	}

	if(!strcasecmp(sep->arg[1], "list")) {
		bool listAll = true;
		int iClass = atoi(sep->arg[2]);

		if(iClass > 0 && iClass < 17)
			listAll = false;

		std::list<BotsAvailableList> AvailableBots = GetBotList(c->CharacterID(), &TempErrorMessage);

		if(!TempErrorMessage.empty()) {
			c->Message(13, "Database Error: %s", TempErrorMessage.c_str());
			return;
		}

		if(!AvailableBots.empty()) {
			for(std::list<BotsAvailableList>::iterator TempAvailableBotsList = AvailableBots.begin(); TempAvailableBotsList != AvailableBots.end(); TempAvailableBotsList++) {
				if(listAll) {
					c->Message(0, "ID: %u -- Class: %s -- Name: %s -- Race: %s", TempAvailableBotsList->BotID, ClassIdToString(TempAvailableBotsList->BotClass).c_str(), TempAvailableBotsList->BotName, RaceIdToString(TempAvailableBotsList->BotRace).c_str());
				}
				else {
					if(TempAvailableBotsList->BotClass == iClass)
						c->Message(0, "ID: %u -- Class: %s -- Name: %s -- Race: %s", TempAvailableBotsList->BotID, ClassIdToString(TempAvailableBotsList->BotClass).c_str(), TempAvailableBotsList->BotName, RaceIdToString(TempAvailableBotsList->BotRace).c_str());
				}
			}
		}
		else {
			c->Message(0, "You have no bots created. Use the #bot create command to create a bot.");
		}
	}

	if(!strcasecmp(sep->arg[1], "spawn") ){

		if(GetBotOwnerCharacterID(atoi(sep->arg[2]), &TempErrorMessage) != c->CharacterID()) {
			c->Message(0, "You can't spawn a bot that you don't own.");
			return;
		}

		if(!TempErrorMessage.empty()) {
			c->Message(13, "Database Error: %s", TempErrorMessage.c_str());
			return;
		}

		if(c->GetFeigned()) {
			c->Message(0, "You can't summon bots while you are feigned.");
			return;
		}

		// TODO:
		//if(c->IsBotRaiding()) {
		//	BotRaids *br = entity_list.GetBotRaidByMob(c->CastToMob());
		//	if(br) {
		//		if(br->GetBotRaidAggro()) {
		//			c->Message(15, "You can't summon bots while you are engaged.");
		//			return;
		//		}
		//	}
		//}

		if(c->IsGrouped()) {
			Group *g = entity_list.GetGroupByClient(c);
			for (int i=0; i<MAX_GROUP_MEMBERS; i++) {
				if(g && g->members[i] && !g->members[i]->qglobal && (g->members[i]->GetAppearance() != eaDead) && g->members[i]->IsEngaged()) {
					c->Message(0, "You can't summon bots while you are engaged.");
					return;
				}
				if(g && g->members[i] && g->members[i]->qglobal) {
					return;
				}
			}
		}

		Mob* TempBotMob = entity_list.GetMobByBotID(atoi(sep->arg[2]));
		if(TempBotMob) {
			c->Message(0, "This bot is already in the zone.");
			return;
		}

		const int spawnedBotCount = SpawnedBotCount(c->CharacterID(), &TempErrorMessage);

		if(!TempErrorMessage.empty()) {
			c->Message(13, "Database Error: %s", TempErrorMessage.c_str());
			return;
		}

		if(RuleB(Bots, BotQuest)) {
			const int allowedBots = AllowedBotSpawns(c->CharacterID(), &TempErrorMessage);

			if(!TempErrorMessage.empty()) {
				c->Message(13, "Database Error: %s", TempErrorMessage.c_str());
				return;
			}

			if(allowedBots == 0) {
				c->Message(0, "You cannot spawn any bots.");
				return;
			}

			if(spawnedBotCount >= allowedBots) {
				c->Message(0, "You cannot spawn more than %i bots.", spawnedBotCount);
				return;
			}

		}

		if(spawnedBotCount >= RuleI(Bots, SpawnBotCount)) {
			c->Message(0, "You cannot spawn more than %i bots.", spawnedBotCount);
			return;
		}

		Bot* TempBot = LoadBot(atoi(sep->arg[2]), &TempErrorMessage);

		if(!TempErrorMessage.empty()) {
			c->Message(13, "Database Error: %s", TempErrorMessage.c_str());
			return;
		}

		if(TempBot) {
			// We have a bot loaded from the database
			TempBot->Spawn(c, &TempErrorMessage);

			if(!TempErrorMessage.empty()) {
				c->Message(13, "Database Error: %s", TempErrorMessage.c_str());
				return;
			}
			
			TempBot->CastToMob()->Say("I am ready for battle.");
		}
		else {
			// We did not find a bot for the specified bot id from the database
			c->Message(0, "BotID: %i not found", atoi(sep->arg[2]));
		}

		return;
	}

	if(!strcasecmp(sep->arg[1], "archery")) {
		if((c->GetTarget() == NULL) || (c->GetTarget() == c) || !c->GetTarget()->IsBot()) {
			c->Message(15, "You must target a bot!");
			return;
		}

		Mob *archerbot = c->GetTarget();
		if((archerbot->GetClass()==WARRIOR)||(archerbot->GetClass()==PALADIN)||(archerbot->GetClass()==RANGER)||(archerbot->GetClass()==SHADOWKNIGHT)||(archerbot->GetClass()==ROGUE)) {
			//const Item_Struct* botweapon = database.GetItem(archerbot->CastToBot()->GetItem(SLOT_RANGE));
			const Item_Struct* botweapon = database.GetItem(archerbot->CastToBot()->GetItem(SLOT_RANGE)->item_id);
			uint32 archeryMaterial;
			uint32 archeryColor;
			uint32 archeryBowID;
			uint32 archeryAmmoID;
			uint32 range = 0;
			if(botweapon && (botweapon->ItemType == ItemTypeBow)) {
				archeryMaterial = atoi(botweapon->IDFile+2);
				archeryBowID = botweapon->ID;
				archeryColor = botweapon->Color;
				range =+ botweapon->Range;
				botweapon = database.GetItem(archerbot->CastToNPC()->GetItem(SLOT_AMMO)->item_id);
				if(!botweapon || (botweapon->ItemType != ItemTypeArrow)) {
					archerbot->Say("I don't have any arrows.");
					archerbot->CastToBot()->SetBotArcheryRange(0);
					return;
				}
				range += botweapon->Range;
				archeryAmmoID = botweapon->ID;
			}
			else {
				archerbot->Say("I don't have a bow.");
				archerbot->CastToBot()->SetBotArcheryRange(0);
				return;
			}
			if(archerbot->CastToBot()->IsBotArcher()) {
				archerbot->CastToBot()->SetBotArcher(false);
				archerbot->Say("Using melee skills.");
				archerbot->CastToBot()->BotAddEquipItem(MATERIAL_PRIMARY, archerbot->CastToBot()->GetBotItemBySlot(SLOT_PRIMARY, &TempErrorMessage));

				if(!TempErrorMessage.empty()) {
					c->Message(13, "Database Error: %s", TempErrorMessage.c_str());
					return;
				}
				archerbot->SendWearChange(MATERIAL_PRIMARY);
				archerbot->CastToBot()->BotAddEquipItem(MATERIAL_SECONDARY, archerbot->CastToBot()->GetBotItemBySlot(SLOT_SECONDARY, &TempErrorMessage));

				if(!TempErrorMessage.empty()) {
					c->Message(13, "Database Error: %s", TempErrorMessage.c_str());
					return;
				}
				archerbot->SendWearChange(MATERIAL_SECONDARY);
				archerbot->CastToBot()->SetBotArcheryRange(0);
			}
			else {
				archerbot->CastToBot()->SetBotArcher(true);
				archerbot->Say("Using archery skills.");
				archerbot->CastToBot()->BotRemoveEquipItem(MATERIAL_PRIMARY);
				archerbot->SendWearChange(MATERIAL_PRIMARY);
				archerbot->CastToBot()->BotRemoveEquipItem(MATERIAL_SECONDARY);
				archerbot->SendWearChange(MATERIAL_SECONDARY);
				archerbot->CastToBot()->BotAddEquipItem(MATERIAL_SECONDARY, archeryBowID);
				archerbot->CastToBot()->SendBotArcheryWearChange(MATERIAL_SECONDARY, archeryMaterial, archeryColor);
				archerbot->CastToBot()->BotAddEquipItem(MATERIAL_PRIMARY, archeryAmmoID);
				archerbot->CastToBot()->SetBotArcheryRange(range);
			}
		}
		else {
			archerbot->Say("I don't know how to use a bow.");
		}
		return;
	}

	if(!strcasecmp(sep->arg[1], "picklock")) {
		if((c->GetTarget() == NULL) || (c->GetTarget() == c) || !c->GetTarget()->IsBot() || (c->GetTarget()->GetClass() != ROGUE)) {
			c->Message(15, "You must target a rogue bot!");
		}
		else {
			entity_list.OpenDoorsNear(c->GetTarget()->CastToNPC());
		}

		return;
	}

	if(!strcasecmp(sep->arg[1], "summon")) {
		if((c->GetTarget() == NULL) || (c->GetTarget() == c) || !c->GetTarget()->IsBot() || c->GetTarget()->IsPet())
		{
			c->Message(15, "You must target a bot!");
		}
		else if(c->GetTarget()->IsMob() && !c->GetTarget()->IsPet())
		{
			Mob *b = c->GetTarget();

			// Is our target "botable" ?
			if(b && !b->IsBot()){
				c->Message(15, "You must target a bot!");
			}
			else if(b && (b->CastToBot()->GetBotOwnerCharacterID() != c->CharacterID()))
			{
				b->Say("You can only summon your own bots.");
			}
			else {
				b->SetTarget(c->CastToMob());
				b->Warp(c->GetX(), c->GetY(), c->GetZ());
			}
		}

		return;
	}

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
//				database.SetBotLeader(npc->GetNPCTypeID(), c->CharacterID(), npc->GetCleanName(), zone->GetShortName());
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
	if(!strcasecmp(sep->arg[1], "group") && !strcasecmp(sep->arg[2], "add")) {
		if(c->GetFeigned()) {
			c->Message(15, "You can't create bot groups while feigned!");
			return;
		}

		// TODO: Uncomment this block of code after bot raids has been integrated
		//if(c->IsBotRaiding()) {
		//	BotRaids *br = entity_list.GetBotRaidByMob(c->CastToMob());
		//	if(br) {
		//		if(br->GetBotRaidAggro()) {
		//			c->Message(15, "You can't create bot groups while you are engaged.");
		//			return;
		//		}
		//	}
		//}

		if(c->IsGrouped()) {
			Group *g = entity_list.GetGroupByClient(c);
			for (int i=0; i<MAX_GROUP_MEMBERS; i++) {
				if(g && g->members[i] && g->members[i]->IsEngaged())
				{
					c->Message(15, "You can't create bot groups while you are engaged.");
					return;
				}
			}
		}

		if((c->GetTarget() == NULL) || !c->GetTarget()->IsBot()) {
			c->Message(15, "You must target a bot!");
			return;
		}

		if(c->GetTarget()->IsClient()) {
			c->Message(15, "You can't invite clients this way.");
			return;
		}

		if(c->IsGrouped()) {
			Group *g = entity_list.GetGroupByClient(c);
			if(g && (c->CastToMob() != g->members[0])) {
				c->Message(15, "Only the group leader can invite bots.");
				Mob* kmob = c->GetTarget();
				if(kmob != NULL) {
					kmob->CastToBot()->SetBotOwner(0);
					// kmob->BotOwner = NULL;
					kmob->Kill();
				}
				return;
			}
		}

		if (c->IsGrouped()) {
			Group *g = entity_list.GetGroupByClient(c);
			if(g && (GetCountBotsInGroup(g) > 5)) {
				c->Message(15, "There is no more room in your group.");
				Mob* kmob = c->GetTarget();
				if(kmob != NULL) {
					kmob->CastToBot()->SetBotOwner(0);
					//kmob->BotOwner = NULL;
					kmob->Kill();
				}
				return;
			}
		}

		if(c->GetTarget()->IsBot())
		{
			Bot* b = c->GetTarget()->CastToBot();

			if(b->GetBotOwnerCharacterID() != c->CharacterID()) {
				b->Say("I can't be your bot, you are not my owner.");
				return;
			}

			// Is he already grouped ?
			if(b->IsGrouped()) {
				b->Say("I'm already grouped!");
				return;
			}

			// else, we do:
			//1: Set its leader
			b->Say("I'm becoming %s\'s bot!", c->GetCleanName());

			//2: Set the follow ID so he's following its leader
			b->SetFollowID(c->GetID());
			b->SetBotOwner(c->CastToMob());
			b->SetOwnerID(0);
			c->SetOwnerID(0);

			//3:  invite it to the group
			if(!c->IsGrouped()) {
				Group *g = new Group(c->CastToMob());
				if(AddBotToGroup(b, g)) {
					entity_list.AddGroup(g);
				}
			}
			else {
				AddBotToGroup(b, c->GetGroup());
			}

			// TODO: Uncomment this block of code after bot raids has been integrated
			/*if(c->IsBotRaiding()) {
				b->SetBotRaiding(true);
				b->SetBotRaidID(c->CastToMob()->GetBotRaidID());
			}*/

			uint32 itemID = 0;
			const Item_Struct* item2 = NULL;

			for(int i=0; i<22; i++) {
				itemID = b->GetBotItemBySlot(i, &TempErrorMessage);

				if(!TempErrorMessage.empty()) {
					c->Message(13, "Database Error: %s", TempErrorMessage.c_str());
					return;
				}
				// itemID = database.GetBotItemBySlot(b->GetNPCTypeID(), i);
				if(itemID != 0) {
					item2 = database.GetItem(itemID);
					b->BotTradeAddItem(itemID, item2->MaxCharges, item2->Slots, i, c, &TempErrorMessage, false);

					if(!TempErrorMessage.empty()) {
						c->Message(13, "Database Error: %s", TempErrorMessage.c_str());
						return;
					}
				}
			}

			b->CalcBotStats();
		}

		return;
	}

	if(!strcasecmp(sep->arg[1], "group") && !strcasecmp(sep->arg[2], "remove")) {
		if(c->GetTarget() != NULL) {
			if(c->GetTarget()->IsBot() && c->GetTarget()->CastToBot()->GetBotOwnerCharacterID() == c->CharacterID()) {
				if(c->GetTarget()->IsGrouped()) {
					Group *g = entity_list.GetGroupByMob(c->GetTarget());
					if(g && g->members[0]) {
						if(g->members[0] == c->GetTarget()) {
							for(int i=5; i>=0; i--) {
								if(g->members[i]) {
									g->members[i]->CastToBot()->SetBotOwner(0);
									g->members[i]->Kill();
								}
							}
						}
						else {
							c->GetTarget()->CastToBot()->SetBotOwner(0);;
							c->GetTarget()->Kill();
						}
						
						if(GetCountBotsInGroup(g) < 2) {
							g->DisbandGroup();
						}
					}
				}
				else {
					c->GetTarget()->CastToBot()->SetBotOwner(0);
					c->GetTarget()->Kill();
				}
				// TODO: Uncomment this block of code after bot raids has been integrated
				/*if(c->GetTarget()->CastToBot()->IsBotRaiding()) {
					
					if(database.SpawnedBotCount(c->CharacterID()) < 6) {
						entity_list.RemoveBotRaid(c->CastToMob()->GetBotRaidID());
						Group *g = c->GetGroup();
						if(g) {
							for(int i=0; i<MAX_GROUP_MEMBERS; i++) {
								if(g->members[i]) {
									g->members[i]->SetBotRaidID(0);
									g->members[i]->SetBotRaiding(false);
								}
							}
						}
					}
				}*/
			}
			else {
				c->Message(15, "You must target a bot first.");
			}
		}
		else {
			c->Message(15, "You must target a bot first.");
		}
		return;
	}

	if(!strcasecmp(sep->arg[1], "group") && !strcasecmp(sep->arg[2], "order"))
	{
		if(!strcasecmp(sep->arg[3], "follow"))
		{
			/*if(c->IsBotRaiding()) {
				BotRaids *br = entity_list.GetBotRaidByMob(c->CastToMob());
				br->FollowGuardCmd(c, false);
			}
			else*/ if(c->IsGrouped())
			{
				Group *g = c->GetGroup();
				if(g) {
					int32 botfollowid = 0;
					const char* botfollowname;
					for(int i=0; i<MAX_GROUP_MEMBERS; i++) {
						if(g->members[i] && g->members[i]->IsBot()) {
							if(botfollowid == 0) {
								botfollowid = g->members[i]->GetID();
								botfollowname = g->members[i]->GetCleanName();
								g->members[i]->SetFollowID(c->GetID());
								g->members[i]->Say("Following %s.", c->GetName());
							}
							else {
								g->members[i]->SetFollowID(botfollowid);
								g->members[i]->Say("Following %s.", botfollowname);
							}
							g->members[i]->WipeHateList();
						}
					}
				}
			}
		}
		else if(!strcasecmp(sep->arg[3], "guard"))
		{
			/*if(c->IsBotRaiding()) {
				BotRaids *br = entity_list.GetBotRaidByMob(c->CastToMob());
				br->FollowGuardCmd(c, true);
			}
			else*/ if(c->IsGrouped())
			{
				Group *g = c->GetGroup();
				if(g) {
					for(int i=0; i<MAX_GROUP_MEMBERS; i++)
					{
						if(g->members[i] && g->members[i]->IsBot()) {
							g->members[i]->SetFollowID(0);
							g->members[i]->WipeHateList();
							g->members[i]->Say("Guarding here.");
						}
					}
				}
			}
		}
		else if(!strcasecmp(sep->arg[3], "attack"))
		{
			/*if(c->IsGrouped() && (c->GetTarget() != NULL) && c->IsAttackAllowed(c->GetTarget())) {
				c->SetOrderBotAttack(true);
				if(c->IsBotRaiding()) {
					BotRaids *br = entity_list.GetBotRaidByMob(c->CastToMob());
					if(br) {
						c->SetOrderBotAttack(true);
						br->AddBotRaidAggro(c->GetTarget());
						c->SetOrderBotAttack(false);
					}
				}
				else {
					Group *g = entity_list.GetGroupByMob(c->CastToMob());
					if(g) {
						for(int i=0; i<MAX_GROUP_MEMBERS; i++) {
							if(g->members[i] && g->members[i]->IsBot()) {
								c->SetOrderBotAttack(true);
								g->members[i]->AddToHateList(c->GetTarget(), 1);
								c->SetOrderBotAttack(false);
							}
						}
					}
				}
				c->SetOrderBotAttack(false);
			}*/

			// This function was added from the part of the code block that is commented out above.
			// TODO: evaluate SetOrderBotAttack and the bot commend toggles in general.
			if(c->IsGrouped() && (c->GetTarget() != NULL) && c->IsAttackAllowed(c->GetTarget())) {
				Group *g = entity_list.GetGroupByMob(c->CastToMob());
				if(g) {
					for(int i=0; i<MAX_GROUP_MEMBERS; i++) {
						if(g->members[i] && g->members[i]->IsBot()) {
							//c->SetOrderBotAttack(true);
							g->members[i]->AddToHateList(c->GetTarget(), 1);
							//c->SetOrderBotAttack(false);
						}
					}
				}
			}
			else {
				c->Message(15, "You must target a monster.");
			}
		}
		return;
	}

	if(!strcasecmp(sep->arg[1], "inventory") && !strcasecmp(sep->arg[2], "list"))
	{
		if(c->GetTarget() != NULL)
		{
			if(c->GetTarget()->IsBot() && c->GetTarget()->CastToBot()->GetBotOwnerCharacterID() == c->CharacterID())
			{
				Mob* b = c->GetTarget();	
				int x = c->GetTarget()->CastToBot()->GetBotItemsCount(&TempErrorMessage);

				if(!TempErrorMessage.empty()) {
					c->Message(13, "Database Error: %s", TempErrorMessage.c_str());
					return;
				}

				const char* equipped[22] = {"Charm", "Left Ear", "Head", "Face", "Right Ear", "Neck", "Shoulders", "Arms", "Back",
					"Left Wrist", "Right Wrist", "Range", "Hands", "Primary Hand", "Secondary Hand",
					"Left Finger", "Right Finger", "Chest", "Legs", "Feet", "Waist", "Ammo" };
				const Item_Struct* item2 = NULL;
				bool is2Hweapon = false;
				for(int i=0; i<22 ; i++)
				{
					if((i == 14) && is2Hweapon) {
						continue;
					}

					item2 = database.GetItem(c->GetTarget()->CastToBot()->GetBotItemBySlot(i, &TempErrorMessage));

					if(!TempErrorMessage.empty()) {
						c->Message(13, "Database Error: %s", TempErrorMessage.c_str());
						return;
					}
					if(item2 == 0) {
						c->Message(15, "I need something for my %s (Item %i)", equipped[i], i);
						continue;
					}
					if((i == 13) && ((item2->ItemType == ItemType2HS) || (item2->ItemType == ItemType2HB) || (item2->ItemType == ItemType2HPierce))) {
						is2Hweapon = true;
					}
					if((i == 0) || (i == 11) || (i == 13) || (i == 14) || (i == 21)) {
						if (c->GetClientVersion() == EQClientSoF)
						{
							c->Message(15, "Using %c%06X00000000000000000000000000000000000000000000%s%c in my %s (Item %i)", 0x12, item2->ID, item2->Name, 0x12, equipped[i], i);
						}
						else
						{
							c->Message(15, "Using %c%06X000000000000000000000000000000000000000%s%c in my %s (Item %i)", 0x12, item2->ID, item2->Name, 0x12, equipped[i], i);
						}
					}
					else {
						if (c->GetClientVersion() == EQClientSoF)
						{
							c->Message(15, "Using %c%06X00000000000000000000000000000000000000000000%s%c on my %s (Item %i)", 0x12, item2->ID, item2->Name, 0x12, equipped[i], i);
						}
						else
						{
							c->Message(15, "Using %c%06X000000000000000000000000000000000000000%s%c on my %s (Item %i)", 0x12, item2->ID, item2->Name, 0x12, equipped[i], i);
						}
					}
				}
			}
			else {
				c->Message(15, "You must group your bot first.");
			}
		}
		else {
			c->Message(15, "You must target a bot first.");
		}
		return;
	}

	if(!strcasecmp(sep->arg[1], "inventory") && !strcasecmp(sep->arg[2], "remove")) {
		if((c->GetTarget() == NULL) || (sep->arg[3] == '\0') || !c->GetTarget()->IsBot())
		{
			c->Message(15, "Usage: #bot inventory remove [slotid] (You must have a bot targetted) ");
			return;
		}		
		else if(c->GetTarget()->IsBot() && c->GetTarget()->CastToBot()->GetBotOwnerCharacterID() == c->CharacterID())
		{
			int slotId = atoi(sep->arg[3]);
			if(slotId > 21 || slotId < 0) {
				c->Message(15, "A bot has 21 slots in its inventory, please choose a slot between 0 and 21.");
				return;
			}
			const char* equipped[22] = {"Charm", "Left Ear", "Head", "Face", "Right Ear", "Neck", "Shoulders", "Arms", "Back",
				"Left Wrist", "Right Wrist", "Range", "Hands", "Primary Hand", "Secondary Hand",
				"Left Finger", "Right Finger", "Chest", "Legs", "Feet", "Waist", "Ammo" };
			const Item_Struct *itm = database.GetItem(c->GetTarget()->CastToBot()->GetBotItemBySlot(slotId, &TempErrorMessage));

			if(!TempErrorMessage.empty()) {
				c->Message(13, "Database Error: %s", TempErrorMessage.c_str());
				return;
			}

			// Don't allow the player to remove a lore item they already possess and cause a crash
			if(!c->CheckLoreConflict(itm)) {
				if(itm) {
					const ItemInst* itminst = new ItemInst(itm, itm->MaxCharges);
					c->PushItemOnCursor(*itminst, true);
					safe_delete(itminst);
					Bot *gearbot = c->GetTarget()->CastToBot();
					if((slotId == SLOT_RANGE)||(slotId == SLOT_AMMO)||(slotId == SLOT_PRIMARY)||(slotId == SLOT_SECONDARY)) {
						gearbot->SetBotArcher(false);
					}
					gearbot->RemoveBotItemBySlot(slotId, &TempErrorMessage);

					if(!TempErrorMessage.empty()) {
						c->Message(13, "Database Error: %s", TempErrorMessage.c_str());
						return;
					}

					gearbot->RemoveItem(itm->ID);
					int8 materialFromSlot = Inventory::CalcMaterialFromSlot(slotId);
					if(materialFromSlot != 0xFF) {
						gearbot->BotRemoveEquipItem(materialFromSlot);
						gearbot->SendWearChange(materialFromSlot);
					}
					gearbot->CalcBotStats();
					switch(slotId) {
						case 0:
						case 1:
						case 2:
						case 3:
						case 4:
						case 5:
						case 8:
						case 9:
						case 10:
						case 11:
						case 13:
						case 14:
						case 15:
						case 16:
						case 17:
						case 20:
						case 21:
							gearbot->Say("My %s is now unequipped.", equipped[slotId]);
							break;
						case 6:
						case 7:
						case 12:
						case 18:
						case 19:
							gearbot->Say("My %s are now unequipped.", equipped[slotId]);
							break;
						default:
							break;
					}
				}
				else {
					switch(slotId) {
						case 0:
						case 1:
						case 2:
						case 3:
						case 4:
						case 5:
						case 8:
						case 9:
						case 10:
						case 11:
						case 13:
						case 14:
						case 15:
						case 16:
						case 17:
						case 20:
						case 21:
							c->GetTarget()->Say("My %s is already unequipped.", equipped[slotId]);
							break;
						case 6:
						case 7:
						case 12:
						case 18:
						case 19:
							c->GetTarget()->Say("My %s are already unequipped.", equipped[slotId]);
							break;
						default:
							break;
					}
				}
			}
			else {
				c->Message(15, "Duplicate Lore item.");
			}
		}
		return;
	}

	if(!strcasecmp(sep->arg[1], "update")) {
		// Congdar: add IsEngaged check for exploit to keep bots alive by repeatedly using #bot update.
		if((c->GetTarget() != NULL) && c->GetTarget()->IsBot()) {
			if(c->GetLevel() <= c->GetTarget()->GetLevel()) {
				c->Message(15, "This bot has already been updated.");
				return;
			}

			// TODO: Uncomment this block of code after bot raiding has been integrated
			/*if(c->IsBotRaiding()) {
				BotRaids *br = entity_list.GetBotRaidByMob(c->CastToMob());
				if(br) {
					if(br->GetBotRaidAggro()) {
						c->Message(15, "You can't update bots while you are engaged.");
						return;
					}
				}
			}*/

			if(c->IsGrouped())
			{
				Group *g = entity_list.GetGroupByClient(c);
				for (int i=0; i<MAX_GROUP_MEMBERS; i++)
				{
					if(g && g->members[i] && g->members[i]->IsEngaged())
					{
						c->Message(15, "You can't update bots while you are engaged.");
						return;
					}
				}
			}

			if((c->GetTarget()->CastToBot()->GetBotOwner() == c->CastToMob()) && !c->GetFeigned()) {
				Bot* bot = c->GetTarget()->CastToBot();
				bot->SetLevel(c->GetLevel());
				bot->SetPetChooser(false);
				bot->CalcBotStats();
			}
			else {
				if(c->GetFeigned()) {
					c->Message(15, "You cannot update bots while feigned.");
				}
				else {
					c->Message(15, "You must target your bot first");
				}
			}
		}
		else {
			c->Message(15, "You must target a bot first");
		}
		return;
	}

	if(!strcasecmp(sep->arg[1], "group") && !strcasecmp(sep->arg[2], "summon") ) {
		// TODO: Uncomment this block of code after bot raids has been integrated.
		/*if(c->IsBotRaiding()) {
			BotRaids *brsummon = entity_list.GetBotRaidByMob(c->CastToMob());
			if(brsummon) {
				brsummon->SummonRaidBots(c->CastToMob(), false);
			}
		}
		else*/ if(c->IsGrouped())
		{
			Group *g = c->GetGroup();
			if(g) {
				for(int i=0; i<MAX_GROUP_MEMBERS; i++)
				{
					if(g->members[i] && g->members[i]->IsBot()) {
						Bot* botMember = g->members[i]->CastToBot();
						botMember->SetTarget(botMember->GetBotOwner());
						botMember->Warp(c->GetX(), c->GetY(), c->GetZ());
						if(botMember->GetPetID()) {
							botMember->GetPet()->SetTarget(botMember);
							botMember->GetPet()->Warp(c->GetX(), c->GetY(), c->GetZ());
						}
					}
				}
			}
		}
		return;
	}

	//Bind
	if(!strcasecmp(sep->arg[1], "bindme")) {
		Mob *binder = NULL;
		bool hasbinder = false;
		if(c->IsGrouped())
		{
			Group *g = c->GetGroup();
			if(g) {
				for(int i=0; i<MAX_GROUP_MEMBERS; i++)
				{
					if(g->members[i] && g->members[i]->IsBot() && (g->members[i]->GetClass() == CLERIC))
					{
						hasbinder = true;
						binder = g->members[i];
					}
				}
				if(!hasbinder) {
					c->Message(15, "You must have a Cleric in your group.");
				}
			}
		}
		if(hasbinder) {
			binder->Say("Attempting to bind you %s.", c->GetName());
			binder->CastToNPC()->CastSpell(35, c->GetID(), 1, -1, -1);
		}
		return;
	}

	// Rune
	if(!strcasecmp(sep->arg[1], "runeme")) {
		Mob *runeer = NULL;
		bool hasruneer = false;
		if(c->IsGrouped())
		{
			Group *g = c->GetGroup();
			if(g) {
				for(int i=0; i<MAX_GROUP_MEMBERS; i++)
				{
					if(g->members[i] && g->members[i]->IsBot() && (g->members[i]->GetClass() == ENCHANTER))
					{
						hasruneer = true;
						runeer = g->members[i];
					}
				}
				if(!hasruneer) {
					c->Message(15, "You must have an Enchanter in your group.");
				}
			}
		}
		if(hasruneer) {
			if      (c->GetLevel() <= 12) {
				runeer->Say("I need to be level 13 or higher for this...");
			}
			else if ((c->GetLevel() >= 13) && (c->GetLevel() <= 21)) {
				runeer->Say("Casting Rune I...");
				runeer->CastSpell(481, c->GetID(), 1, -1, -1);
			}
			else if ((c->GetLevel() >= 22) && (c->GetLevel() <= 32)) {
				runeer->Say("Casting Rune II...");
				runeer->CastSpell(482, c->GetID(), 1, -1, -1);
			}
			else if ((c->GetLevel() >= 33) && (c->GetLevel() <= 39)) { 
				runeer->Say("Casting Rune III...");
				runeer->CastSpell(483, c->GetID(), 1, -1, -1);
			}
			else if ((c->GetLevel() >= 40) && (c->GetLevel() <= 51)) { 
				runeer->Say("Casting Rune IV...");
				runeer->CastSpell(484, c->GetID(), 1, -1, -1);
			}
			else if ((c->GetLevel() >= 52) && (c->GetLevel() <= 60)) { 
				runeer->Say("Casting Rune V...");
				runeer->CastSpell(1689, c->GetID(), 1, -1, -1);
			}
			else if (c->GetLevel() >= 61){ 
				runeer->Say("Casting Rune of Zebuxoruk...");
				runeer->CastSpell(3343, c->GetID(), 1, -1, -1);
			}
		}
		return;
	}

	//Tracking
	if(!strcasecmp(sep->arg[1], "track") && c->IsGrouped()) {
		Mob *Tracker;
		int32 TrackerClass = 0;

		Group *g = c->GetGroup();
		if(g) {
			for(int i=0; i<MAX_GROUP_MEMBERS; i++) {
				if(g->members[i] && g->members[i]->IsBot()) {
					switch(g->members[i]->GetClass()) {
						case RANGER:
							Tracker = g->members[i];
							TrackerClass = RANGER;
							break;
						case BARD:
							// If we haven't found a tracker yet, use bard.
							if(TrackerClass == 0) {
								Tracker = g->members[i];
								TrackerClass = BARD;
							}
							break;
						case DRUID:
							// Unless we have a ranger, druid is next best.
							if(TrackerClass != RANGER) {
								Tracker = g->members[i];
								TrackerClass = DRUID;
							}
							break;
						default:
							break;
					}
				}
			}

			int Level = (c->GetLevel());
			int RangeR = (Level*80); //Ranger
			int RangeD = (Level*30); //Druid
			int RangeB = (Level*20); //Bard
			switch(TrackerClass) {
				case RANGER:
					if(!strcasecmp(sep->arg[2], "all")) {
						Tracker->Say("Tracking everything", c->GetName());
						entity_list.ShowSpawnWindow(c, RangeR, false);
					}
					else if(!strcasecmp(sep->arg[2], "rare")) { 
						Tracker->Say("Selective tracking", c->GetName());
						entity_list.ShowSpawnWindow(c, RangeR, true);
					}
					else if(!strcasecmp(sep->arg[2], "near")) { 
						Tracker->Say("Tracking mobs nearby", c->GetName());
						entity_list.ShowSpawnWindow(c, RangeD, false);
					}
					else 
						Tracker->Say("You want to [track all], [track near], or [track rare]?", c->GetName());

					break;

				case BARD:

					if(TrackerClass != RANGER)
						Tracker->Say("Tracking up", c->GetName());
					entity_list.ShowSpawnWindow(c, RangeB, false);
					break;

				case DRUID:

					if(TrackerClass = BARD)
						Tracker->Say("Tracking up", c->GetName());
					entity_list.ShowSpawnWindow(c, RangeD, false);
					break;

				default:
					c->Message(15, "You must have a Ranger, Druid, or Bard in your group.");
					break;
			}
		}
	}

	//Cure
	if ((!strcasecmp(sep->arg[1], "cure")) && (c->IsGrouped())) {
		Mob *Curer;
		int32 CurerClass = 0;
		Group *g = c->GetGroup();
		if(g) {
			for(int i=0; i<MAX_GROUP_MEMBERS; i++){
				if(g->members[i] && g->members[i]->IsBot()) {
					switch(g->members[i]->GetClass()) {
						case CLERIC:
							Curer = g->members[i];
							CurerClass = CLERIC;
							break;
						case SHAMAN:
							if(CurerClass != CLERIC){
								Curer = g->members[i];
								CurerClass = SHAMAN;
							}
						case DRUID:
							if (CurerClass == 0){
								Curer = g->members[i];
								CurerClass = DRUID;
							}
							break;
							break;
						default:
							break;
					}
				}
			}
			switch(CurerClass) {
				case CLERIC:
					if	(!strcasecmp(sep->arg[2], "poison") && (c->GetLevel() >= 1))  {
						Curer->Say("Trying to cure us of %s.", sep->arg[2]);
						Curer->CastToBot()->Bot_Command_Cure(1, Curer->GetLevel());
					}
					else if (!strcasecmp(sep->arg[2], "disease") && (c->GetLevel() >= 4)) {
						Curer->Say("Trying to cure us of %s.", sep->arg[2]);
						Curer->CastToBot()->Bot_Command_Cure(2, Curer->GetLevel());
					}
					else if(!strcasecmp(sep->arg[2], "curse") && (c->GetLevel() >= 8)) {
						Curer->Say("Trying to cure us of %s.", sep->arg[2]);
						Curer->CastToBot()->Bot_Command_Cure(3, Curer->GetLevel());
					}
					else if(!strcasecmp(sep->arg[2], "blindness") && (c->GetLevel() >= 3)) {
						Curer->Say("Trying to cure us of %s.", sep->arg[2]);
						Curer->CastToBot()->Bot_Command_Cure(4, Curer->GetLevel());
					}
					else if (!strcasecmp(sep->arg[2], "curse") && (c->GetLevel() <= 8)
						|| !strcasecmp(sep->arg[2], "blindness") && (c->GetLevel() <= 3) 
						|| !strcasecmp(sep->arg[2], "disease") && (c->GetLevel() <= 4)
						|| !strcasecmp(sep->arg[2], "poison") && (c->GetLevel() <= 1)) {
							Curer->Say("I don't have the needed level yet", sep->arg[2]);
					}
					else
						Curer->Say("Do you want [cure poison], [cure disease], [cure curse], or [cure blindness]?", c->GetName());

					break;

				case SHAMAN:
					if	(!strcasecmp(sep->arg[2], "poison") && (c->GetLevel() >= 2))  {
						Curer->Say("Trying to cure us of %s.", sep->arg[2]);
						Curer->CastToBot()->Bot_Command_Cure(1, Curer->GetLevel());
					}
					else if (!strcasecmp(sep->arg[2], "disease") && (c->GetLevel() >= 1)) {
						Curer->Say("Trying to cure us of %s.", sep->arg[2]);
						Curer->CastToBot()->Bot_Command_Cure(2, Curer->GetLevel());
					}
					else if(!strcasecmp(sep->arg[2], "curse")) {
						Curer->Say("I don't have that spell", sep->arg[2]);
					}
					else if(!strcasecmp(sep->arg[2], "blindness") && (c->GetLevel() >= 7)) {
						Curer->Say("Trying to cure us of %s.", sep->arg[2]);
						Curer->CastToBot()->Bot_Command_Cure(4, Curer->GetLevel());
					}
					else if (!strcasecmp(sep->arg[2], "blindness") && (c->GetLevel() <= 7) 
						|| !strcasecmp(sep->arg[2], "disease") && (c->GetLevel() <= 1)
						|| !strcasecmp(sep->arg[2], "poison") && (c->GetLevel() <= 2)) {
							Curer->Say("I don't have the needed level yet", sep->arg[2]);
					}
					else 
						Curer->Say("Do you want [cure poison], [cure disease], or [cure blindness]?", c->GetName());

					break;

				case DRUID:

					if	(!strcasecmp(sep->arg[2], "poison") && (c->GetLevel() >= 5)) {
						Curer->Say("Trying to cure us of %s.", sep->arg[2]);
						Curer->CastToBot()->Bot_Command_Cure(1, Curer->GetLevel());
					}
					else if (!strcasecmp(sep->arg[2], "disease") && (c->GetLevel() >= 4)) {
						Curer->Say("Trying to cure us of %s.", sep->arg[2]);
						Curer->CastToBot()->Bot_Command_Cure(2, Curer->GetLevel());
					}
					else if(!strcasecmp(sep->arg[2], "curse")) { // Fire level 1
						Curer->Say("I don't have that spell", sep->arg[2]);
					}
					else if(!strcasecmp(sep->arg[2], "blindness") && (c->GetLevel() >= 13)) {
						Curer->Say("I don't have that spell", sep->arg[2]);
					}
					else if (!strcasecmp(sep->arg[2], "disease") && (c->GetLevel() <= 4)
						|| !strcasecmp(sep->arg[2], "poison") && (c->GetLevel() <= 5)) {
							Curer->Say("I don't have the needed level yet", sep->arg[2]) ;
					}
					else 
						Curer->Say("Do you want [cure poison], or [cure disease]?", c->GetName());

					break;

				default:
					c->Message(15, "You must have a Cleric, Shaman, or Druid in your group.");
					break;
			}
		}
	}

	//Mez
	if(!strcasecmp(sep->arg[1], "ai") && !strcasecmp(sep->arg[2], "mez"))
	{
		Mob *target = c->GetTarget();
		if(target == NULL || target == c || target->IsBot() || target->IsPet() && target->GetOwner()->IsBot())
		{
			c->Message(15, "You must select a monster");
			return;
		}

		if(c->IsGrouped())
		{
			bool hasmezzer = false;
			Group *g = c->GetGroup();
			for(int i=0; i<MAX_GROUP_MEMBERS; i++)
			{
				if(g && g->members[i] && g->members[i]->IsBot() && (g->members[i]->GetClass() == ENCHANTER))
				{
					hasmezzer = true;
					Mob *mezzer = g->members[i];
					mezzer->Say("Trying to mez %s \n", target->GetCleanName());
					mezzer->CastToBot()->MesmerizeTarget(target);
				}
			}
			if(!hasmezzer) {
				c->Message(15, "You must have an Enchanter in your group.");
			}
		}
		return;
	}

	//Lore (Identify item)
	if(!strcasecmp(sep->arg[1], "lore")) {
		if(c->IsGrouped())
		{
			bool hascaster = false;
			Group *g = c->GetGroup();
			for(int i=0; i<MAX_GROUP_MEMBERS; i++)
			{
				if(g && g->members[i] && g->members[i]->IsBot()) {
					uint8 casterlevel = g->members[i]->GetLevel();
					switch(g->members[i]->GetClass()) {
						case ENCHANTER:
							if(casterlevel >= 15) {
								hascaster = true;
							}
							break;
						case WIZARD:
							if(casterlevel >= 14) {
								hascaster = true;
							}
							break;
						case NECROMANCER:
							if(casterlevel >= 17) {
								hascaster = true;
							}
							break;
						case MAGICIAN:
							if(casterlevel >= 13) {
								hascaster = true;
							}
							break;
						default:
							break;
					}
					if(hascaster) {
						g->members[i]->Say("Trying to Identify your item...");
						g->members[i]->CastSpell(305, c->GetID(), 1, -1, -1);
						break;
					}
				}
			}
			if(!hascaster) {
				c->Message(15, "You don't see anyone in your group that can cast Identify.");
			}
		}
		else {
			c->Message(15, "You don't see anyone in your group that can cast Identify.");
		}
		return;
	}

	//Resurrect
	if(!strcasecmp(sep->arg[1], "resurrectme"))
	{
		Mob *target = c->GetTarget();
		if(target == NULL || !target->IsCorpse())
		{
			c->Message(15, "You must select a corpse");
			return;
		}

		if(c->IsGrouped())
		{
			bool hasrezzer = false;
			Group *g = c->GetGroup();
			for(int i=0; i<MAX_GROUP_MEMBERS; i++)
			{
				if(g && g->members[i] && g->members[i]->IsBot() && (g->members[i]->GetClass() == CLERIC))
				{
					hasrezzer = true;
					Mob *rezzer = g->members[i];
					rezzer->Say("Trying to rez %s", target->GetCleanName());
					rezzer->CastToBot()->Bot_Command_RezzTarget(target);
					break;
				}
			}
			if(!hasrezzer) {
				c->Message(15, "You must have a Cleric in your group.");
			}
		}
		else {
			c->Message(15, "You must have a Cleric in your group.");
		}
		return;
	}

	if(!strcasecmp(sep->arg[1], "magepet"))
	{
		if(c->GetTarget() && c->GetTarget()->IsBot() && (c->GetTarget()->GetClass() == MAGICIAN))
		{
			if(c->GetTarget()->CastToBot()->GetBotOwnerCharacterID() == c->CharacterID())
			{
				int botlevel = c->GetTarget()->GetLevel();
				c->GetTarget()->CastToBot()->SetPetChooser(true);
				if(botlevel == 1)
				{
					c->GetTarget()->Say("I don't have any pets yet.");
					return;
				}
				if(!strcasecmp(sep->arg[2], "water"))
				{
					c->GetTarget()->CastToBot()->SetPetChooserID(0);
				}
				else if(!strcasecmp(sep->arg[2], "fire"))
				{
					if(botlevel < 3)
					{
						c->GetTarget()->Say("I don't have that pet yet.");
						return;
					}
					else
					{
						c->GetTarget()->CastToBot()->SetPetChooserID(1);
					}
				}
				else if(!strcasecmp(sep->arg[2], "air"))
				{
					if(botlevel < 4)
					{
						c->GetTarget()->Say("I don't have that pet yet.");
						return;
					}
					else
					{
						c->GetTarget()->CastToBot()->SetPetChooserID(2);
					}
				}
				else if(!strcasecmp(sep->arg[2], "earth"))
				{
					if(botlevel < 5)
					{
						c->GetTarget()->Say("I don't have that pet yet.");
						return;
					}
					else
					{
						c->GetTarget()->CastToBot()->SetPetChooserID(3);
					}
				}
				else if(!strcasecmp(sep->arg[2], "monster"))
				{
					if(botlevel < 30)
					{
						c->GetTarget()->Say("I don't have that pet yet.");
						return;
					}
					else
					{
						c->GetTarget()->CastToBot()->SetPetChooserID(4);
					}
				}
				if(c->GetTarget()->GetPet())
				{
					// cast reclaim energy
					int16 id = c->GetTarget()->GetPetID();
					c->GetTarget()->SetPetID(0);
					c->GetTarget()->CastSpell(331, id);
				}
			}
		}
		else
		{
			c->Message(15, "You must target your Magician bot.");
		}
		return;
	}

	//Summon Corpse
	if(!strcasecmp(sep->arg[1], "corpse") && !strcasecmp(sep->arg[2], "summon")) {
		if(c->GetTarget() == NULL) {
			c->Message(15, "You must select player with his corpse in the zone.");
			return;
		}
		if(c->IsGrouped()) {
			bool hassummoner = false;
			Mob *t = c->GetTarget();
			Group *g = c->GetGroup();
			int summonerlevel = 0;
			for(int i=0; i<MAX_GROUP_MEMBERS; i++) {
				if(g && g->members[i] && g->members[i]->IsBot() && ((g->members[i]->GetClass() == NECROMANCER)||(g->members[i]->GetClass() == SHADOWKNIGHT))) {
					hassummoner = true;
					summonerlevel = g->members[i]->GetLevel();
					if(!t->IsClient()) {
						g->members[i]->Say("You have to target a player with a corpse in the zone");
						return;
					}
					else if(summonerlevel < 12) {
						g->members[i]->Say("I don't have that spell yet.");
					}
					else if((summonerlevel > 11) && (summonerlevel < 35)) {
						g->members[i]->Say("Attempting to summon %s\'s corpse.", t->GetCleanName());
						g->members[i]->CastSpell(2213, t->GetID(), 1, -1, -1);
						return;
					}
					else if((summonerlevel > 34) && (summonerlevel < 71)) {
						g->members[i]->Say("Attempting to summon %s\'s corpse.", t->GetCleanName());
						g->members[i]->CastSpell(3, t->GetID(), 1, -1, -1);
						return;
					}
					else if((summonerlevel > 70) && (summonerlevel < 76)) {
						g->members[i]->Say("Attempting to summon %s\'s corpse.", t->GetCleanName());
						g->members[i]->CastSpell(10042, t->GetID(), 1, -1, -1);
						return;
					}
					else if((summonerlevel > 75) && (summonerlevel < 81)) {
						g->members[i]->Say("Attempting to summon %s\'s corpse.", t->GetCleanName());
						g->members[i]->CastSpell(14823, t->GetID(), 1, -1, -1);
						return;
					}
				}
			}
			if (!hassummoner) {
				c->Message(15, "You must have a Necromancer or Shadowknight in your group.");
			}
			return;
		}
	}

	//Pacify
	if(!strcasecmp(sep->arg[1], "target") && !strcasecmp(sep->arg[2], "calm"))
	{
		Mob *target = c->GetTarget();

		if(target == NULL || target->IsClient() || target->IsBot() || target->IsPet() && target->GetOwner()->IsBot())
			c->Message(15, "You must select a monster");
		else {
			if(c->IsGrouped()) {
				Group *g = c->GetGroup();

				for(int i=0; i<MAX_GROUP_MEMBERS; i++) {
					if(g && g->members[i] && g->members[i]->IsBot() && ((g->members[i]->GetClass() == ENCHANTER) || g->members[i]->GetClass() == CLERIC)) {
						Bot *pacer = g->members[i]->CastToBot();
						pacer->Say("Trying to pacify %s \n", target->GetCleanName());

						if(pacer->Bot_Command_CalmTarget(target)) {
							if(target->FindType(SE_Lull) || target->FindType(SE_Harmony) || target->FindType(SE_Calm))
							//if(pacer->IsPacified(target))
								c->Message(0, "I have successfully pacified %s.", target->GetCleanName());
							else
								c->Message(0, "I failed to pacify %s.", target->GetCleanName());
						}
						else
							c->Message(0, "I failed to pacify %s.", target->GetCleanName());
					}
					else
						c->Message(15, "You must have an Enchanter or Cleric in your group.");
				}
			}
		}

		return;
	}

	//Charm
	if(!strcasecmp(sep->arg[1], "charm"))
	{
		Mob *target = c->GetTarget();
		if(target == NULL || target->IsClient() || target->IsBot() || (target->IsPet() && target->GetOwner()->IsBot()))
		{
			c->Message(15, "You must select a monster");
			return;
		}
		int32 DBtype = c->GetTarget()->GetBodyType();
		Mob *Charmer;
		int32 CharmerClass = 0;
		Group *g = c->GetGroup();
		if(g) {
			for(int i=0; i<MAX_GROUP_MEMBERS; i++){
				if(g->members[i] && g->members[i]->IsBot()) {
					switch(g->members[i]->GetClass()) {
						case ENCHANTER:
							Charmer = g->members[i];
							CharmerClass = ENCHANTER;
							break;
						case NECROMANCER:
							if(CharmerClass != ENCHANTER){
								Charmer = g->members[i];
								CharmerClass = NECROMANCER;
							}
						case DRUID:
							if (CharmerClass == 0){
								Charmer = g->members[i];
								CharmerClass = DRUID;
							}
							break;
							break;
						default:
							break;
					}
				}
			}
			switch(CharmerClass) {
				case ENCHANTER:
					if	(c->GetLevel() >= 11) {
						Charmer->Say("Trying to charm %s \n", target->GetCleanName(), sep->arg[2]);
						Charmer->CastToBot()->Bot_Command_CharmTarget (1,target);
					}
					else if (c->GetLevel() <= 10){
						Charmer->Say("I don't have the needed level yet", sep->arg[2]);
					}
					else
						Charmer->Say("Mob level is too high or can't be charmed", c->GetName());
					break;

				case NECROMANCER:
					if	((c->GetLevel() >= 18) && (DBtype == 3)) {
						Charmer->Say("Trying to Charm %s \n", target->GetCleanName(), sep->arg[2]);
						Charmer->CastToBot()->Bot_Command_CharmTarget (2,target);
					}
					else if (c->GetLevel() <= 17){
						Charmer->Say("I don't have the needed level yet", sep->arg[2]);
					}
					else
						Charmer->Say("Mob Is not undead...", c->GetName());
					break;

				case DRUID:
					if	((c->GetLevel() >= 13) && (DBtype == 21)) {
						Charmer->Say("Trying to charm %s \n", target->GetCleanName(), sep->arg[2]);
						Charmer->CastToBot()->Bot_Command_CharmTarget (3,target);
					}
					else if (c->GetLevel() <= 12){
						Charmer->Say("I don't have the needed level yet", sep->arg[2]);
					}
					else
						Charmer->Say("Mob is not an animal...", c->GetName());
					break;

				default:
					c->Message(15, "You must have an Enchanter, Necromancer or Druid in your group.");
					break;
			}
		}
	}

	// Remove Bot's Pet
	if(!strcasecmp(sep->arg[1], "pet") && !strcasecmp(sep->arg[2], "remove")) {
		if(c->GetTarget() != NULL) {
			if (c->IsGrouped() && c->GetTarget()->IsBot() && (c->GetTarget()->CastToBot()->GetBotOwnerCharacterID() != c->CharacterID()) &&
				((c->GetTarget()->GetClass() == NECROMANCER) || (c->GetTarget()->GetClass() == ENCHANTER) || (c->GetTarget()->GetClass() == DRUID))) {
					if(c->GetTarget()->CastToBot()->IsBotCharmer()) {
						c->GetTarget()->CastToBot()->SetBotCharmer(false);
						c->GetTarget()->Say("Using a summoned pet.");
					}
					else {
						if(c->GetTarget()->GetPet())
						{
							c->GetTarget()->GetPet()->Say_StringID(PET_GETLOST_STRING);
							c->GetTarget()->GetPet()->Kill();
							c->GetTarget()->SetPetID(0);
						}
						c->GetTarget()->CastToBot()->SetBotCharmer(true);
						c->GetTarget()->Say("Available for Dire Charm command.");
					}
			}
			else {
				c->Message(15, "You must target your Enchanter, Necromancer, or Druid bot.");
			}
		}
		else {
			c->Message(15, "You must target an Enchanter, Necromancer, or Druid bot.");
		}
		return;
	}

	//Dire Charm
	if(!strcasecmp(sep->arg[1], "Dire") && !strcasecmp(sep->arg[2], "Charm"))
	{
		Mob *target = c->GetTarget();
		if(target == NULL || target->IsClient() || target->IsBot() || (target->IsPet() && target->GetOwner()->IsBot()))
		{
			c->Message(15, "You must select a monster");
			return;
		}
		int32 DBtype = c->GetTarget()->GetBodyType();
		Mob *Direr;
		int32 DirerClass = 0;
		Group *g = c->GetGroup();
		if(g) {
			for(int i=0; i<MAX_GROUP_MEMBERS; i++){
				if(g->members[i] && g->members[i]->IsBot()) {
					switch(g->members[i]->GetClass()) {
						case ENCHANTER:
							Direr = g->members[i];
							DirerClass = ENCHANTER;
							break;
						case NECROMANCER:
							if(DirerClass != ENCHANTER){
								Direr = g->members[i];
								DirerClass = NECROMANCER;
							}
						case DRUID:
							if (DirerClass == 0){
								Direr = g->members[i];
								DirerClass = DRUID;
							}
							break;
							break;
						default:
							break;
					}
				}
			}
			switch(DirerClass) {
				case ENCHANTER:
					if	(c->GetLevel() >= 55) {
						Direr->Say("Trying to dire charm %s \n", target->GetCleanName(), sep->arg[2]);
						Direr->CastToBot()->Bot_Command_DireTarget (1,target);
					}
					else if (c->GetLevel() <= 55){
						Direr->Say("I don't have the needed level yet", sep->arg[2]);
					}
					else
						Direr->Say("Mob level is too high or can't be charmed", c->GetName());
					break;

				case NECROMANCER:
					if	((c->GetLevel() >= 55) && (DBtype == 3)) {
						Direr->Say("Trying to dire charm %s \n", target->GetCleanName(), sep->arg[2]);
						Direr->CastToBot()->Bot_Command_DireTarget (2,target);
					}
					else if (c->GetLevel() <= 55){
						Direr->Say("I don't have the needed level yet", sep->arg[2]);
					}
					else
						Direr->Say("Mob Is not undead...", c->GetName());
					break;

				case DRUID:
					if	((c->GetLevel() >= 55) && (DBtype == 21)) {
						Direr->Say("Trying to dire charm %s \n", target->GetCleanName(), sep->arg[2]);
						Direr->CastToBot()->Bot_Command_DireTarget (3,target);
					}
					else if (c->GetLevel() <= 55){
						Direr->Say("I don't have the needed level yet", sep->arg[2]);
					}
					else
						Direr->Say("Mob is not an animal...", c->GetName());
					break;

				default:
					c->Message(15, "You must have an Enchanter, Necromancer or Druid in your group.");
					break;
			}
		}
	}

	// Evacuate
	if(!strcasecmp(sep->arg[1], "evac")) {
		Mob *evac = NULL;
		bool hasevac = false;
		if(c->IsGrouped())
		{
			Group *g = c->GetGroup();
			if(g) {
				for(int i=0; i<MAX_GROUP_MEMBERS; i++)
				{
					if((g->members[i] && g->members[i]->IsBot() && (g->members[i]->GetClass() == DRUID)) 
						|| (g->members[i] && g->members[i]->IsBot() && (g->members[i]->GetClass() == WIZARD)))
					{
						hasevac = true;
						evac = g->members[i];
					}
				}
				if(!hasevac) {
					c->Message(15, "You must have a Druid in your group.");
				}
			}
		}
		if((hasevac)  && (c->GetLevel() >= 18)) {
			evac->Say("Attempting to Evac you %s.", c->GetName());
			evac->CastToClient()->CastSpell(2183, c->GetID(), 1, -1, -1);
		}
		else if((hasevac)  && (c->GetLevel() <= 17)) {
			evac->Say("I'm not level 18 yet.", c->GetName());
		}
		return;
	}

	// Sow
	if ((!strcasecmp(sep->arg[1], "sow")) && (c->IsGrouped())) {
		Mob *Sower;
		int32 SowerClass = 0;
		Group *g = c->GetGroup();
		if(g) {
			for(int i=0; i<MAX_GROUP_MEMBERS; i++){
				if(g->members[i] && g->members[i]->IsBot()) {
					switch(g->members[i]->GetClass()) {
						case DRUID:
							Sower = g->members[i];
							SowerClass = DRUID;
							break;
						case SHAMAN:
							if (SowerClass != DRUID){
								Sower = g->members[i];
								SowerClass = SHAMAN;
							}
							break;
						case RANGER:
							if (SowerClass == 0){
								Sower = g->members[i];
								SowerClass = RANGER;
							}
							break;
						case BEASTLORD:
							if (SowerClass == 0){
								Sower = g->members[i];
								SowerClass = BEASTLORD;
							}
							break;
						default:
							break;
					}
				}
			}
			switch(SowerClass) {
				case DRUID:
					if      ((!strcasecmp(sep->arg[2], "regular")) && (zone->CanCastOutdoor())  && (c->GetLevel() >= 10) ) {
						Sower->Say("Casting sow...");
						Sower->CastSpell(278, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "regular")) && (zone->CanCastOutdoor())  && (c->GetLevel() <= 10) ) {
						Sower->Say("I'm not level 10 yet.");
					}
					else if ((!strcasecmp(sep->arg[2], "wolf")) && zone->CanCastOutdoor() && (c->GetLevel() >= 20)) {
						Sower->Say("Casting group wolf...");
						Sower->CastSpell(428, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "wolf")) && (c->GetLevel() <= 20)) {
						Sower->Say("I'm not level 20 yet.");
					}
					else if ((!strcasecmp(sep->arg[2], "feral")) && (c->GetLevel() >= 50)) { 
						Sower->Say("Casting Feral Pack...");
						Sower->CastSpell(4058, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "feral")) && (c->GetLevel() <= 50)) {
						Sower->Say("I'm not level 50 yet.");
					}
					else if ((!strcasecmp(sep->arg[2], "shrew")) && (c->GetLevel() >= 35)) { 
						Sower->Say("Casting Pack Shrew...");
						Sower->CastSpell(4055, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "wolf")) && (c->GetLevel() <= 35)) {
						Sower->Say("I'm not level 35 yet.");
					}
					else if ((!zone->CanCastOutdoor()) && (!strcasecmp(sep->arg[2], "regular")) ||
						(!zone->CanCastOutdoor()) && (!strcasecmp(sep->arg[2], "wolf"))) {
							Sower->Say("I can't cast this spell indoors, try [sow shrew] if you're 35 or higher, or [sow feral] if you're 50 or higher,", c->GetName());
					}
					else if (!zone->CanCastOutdoor()) {
						Sower->Say("I can't cast this spell indoors, try [sow shrew] if you're 35 or higher, or [sow feral] if you're 50 or higher,", c->GetName());
					}
					else if (zone->CanCastOutdoor()) {
						Sower->Say("Do you want [sow regular] or [sow wolf]?", c->GetName());
					}
					else if (!zone->CanCastOutdoor()) {
						Sower->Say("I can't cast this spell indoors, try [sow shrew] if you're 35 or higher, or [sow feral] if you're 50 or higher,", c->GetName());
					}
					break;

				case SHAMAN:

					if ((zone->CanCastOutdoor()) && (c->GetLevel() >= 9)) { 
						Sower->Say("Casting SoW...");
						Sower->CastToClient()->CastSpell(278, c->GetID(), 1, -1, -1);
					}
					else if (!zone->CanCastOutdoor()) {
						Sower->Say("I can't cast this spell indoors", c->GetName());
					}
					else if (c->GetLevel() <= 9) {
						Sower->Say("I'm not level 9 yet.");
					}
					break;

				case RANGER:

					if ((zone->CanCastOutdoor()) && (c->GetLevel() >= 28)){
						Sower->Say("Casting SoW...");
						Sower->CastToClient()->CastSpell(278, c->GetID(), 1, -1, -1);
					}
					else if (!zone->CanCastOutdoor()) {
						Sower->Say("I can't cast this spell indoors", c->GetName());
					}
					else if (c->GetLevel() <= 28) {
						Sower->Say("I'm not level 28 yet.");
					}
					break;

				case BEASTLORD:

					if((zone->CanCastOutdoor()) && (c->GetLevel() >= 24)) {
						Sower->Say("Casting SoW...");
						Sower->CastToClient()->CastSpell(278, c->GetID(), 1, -1, -1);
					}
					else if (!zone->CanCastOutdoor()) {
						Sower->Say("I can't cast this spell indoors", c->GetName());
					}
					else if (c->GetLevel() <= 24) {
						Sower->Say("I'm not level 24 yet.");
					}
					break;


				default:
					c->Message(15, "You must have a Druid, Shaman, Ranger,  or Beastlord in your group.");
					break;
			}
		}
	}

	// Shrink
	if ((!strcasecmp(sep->arg[1], "shrinkme")) && (c->IsGrouped())) {
		Mob *Shrinker;
		int32 ShrinkerClass = 0;
		Group *g = c->GetGroup();

		if(g) {
			for(int i=0; i<MAX_GROUP_MEMBERS; i++){
				if(g->members[i] && g->members[i]->IsBot()) {
					switch(g->members[i]->GetClass()) {
						case SHAMAN:
							Shrinker = g->members[i];
							ShrinkerClass = SHAMAN;
							break;
						case BEASTLORD:
							if (ShrinkerClass != SHAMAN){
								Shrinker = g->members[i];
								ShrinkerClass = BEASTLORD;
							}
							break;
						default:
							break;
					}
				}
			}
			switch(ShrinkerClass) {
				case SHAMAN:

					if (c->GetLevel() >= 15) { 
						Shrinker->Say("Casting Shrink...");
						//Shrinker->CastToBot()->BotRaidSpell(345);
						Shrinker->CastSpell(345, c->GetID(), 1);
					}
					else if (c->GetLevel() <= 14) {
						Shrinker->Say("I'm not level 15 yet.");
					}
					break;

				case BEASTLORD:

					if (c->GetLevel() >= 23) {
						Shrinker->Say("Casting Shrink...");
						//Shrinker->CastToBot()->BotRaidSpell(345);
						Shrinker->CastSpell(345, c->GetID(), 1);
					}
					else if (c->GetLevel() <= 22) {
						Shrinker->Say("I'm not level 23 yet.");
					}
					break;

				default:
					c->Message(15, "You must have a Shaman or Beastlord in your group.");
					break;
			}
		}
	}

	// Gate
	if ((!strcasecmp(sep->arg[1], "gate")) && (c->IsGrouped())) {
		Mob *Gater;
		int32 GaterClass = 0;
		Group *g = c->GetGroup();
		if(g) {
			for(int i=0; i<MAX_GROUP_MEMBERS; i++){
				if(g->members[i] && g->members[i]->IsBot()) {
					switch(g->members[i]->GetClass()) {
						case DRUID:
							Gater = g->members[i];
							GaterClass = DRUID;
							break;
						case WIZARD:
							if (GaterClass == 0){
								Gater = g->members[i];
								GaterClass = WIZARD;
							}
							break;
						default:
							break;
					}
				}
			}
			switch(GaterClass) {
				case DRUID:
					if      ((!strcasecmp(sep->arg[2], "karana")) && (c->GetLevel() >= 25) ) {
						Gater->Say("Casting Circle of Karana...");
						Gater->CastSpell(550, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "commons")) && (c->GetLevel() >= 27)) {
						Gater->Say("Casting Circle of Commons...");
						Gater->CastSpell(551, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "tox")) && (c->GetLevel() >= 25)) { 
						Gater->Say("Casting Circle of Toxxulia...");
						Gater->CastSpell(552, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "butcher")) && (c->GetLevel() >= 25)) { 
						Gater->Say("Casting Circle of Butcherblock...");
						Gater->CastSpell(553, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "lava")) && (c->GetLevel() >= 30)) { 
						Gater->Say("Casting Circle of Lavastorm...");
						Gater->CastSpell(554, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "ro")) && (c->GetLevel() >= 32)) { 
						Gater->Say("Casting Circle of Ro...");
						Gater->CastSpell(555, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "feerrott")) && (c->GetLevel() >= 32)) { 
						Gater->Say("Casting Circle of feerrott...");
						Gater->CastSpell(556, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "steamfont")) && (c->GetLevel() >= 31)) { 
						Gater->Say("Casting Circle of Steamfont...");
						Gater->CastSpell(557, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "misty")) && (c->GetLevel() >= 36)) { 
						Gater->Say("Casting Circle of Misty...");
						Gater->CastSpell(558, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "wakening")) && (c->GetLevel() >= 40)) { 
						Gater->Say("Casting Circle of Wakening Lands...");
						Gater->CastSpell(1398, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "iceclad")) && (c->GetLevel() >= 32)) { 
						Gater->Say("Casting Circle of Iceclad Ocean...");
						Gater->CastSpell(1434, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "divide")) && (c->GetLevel() >= 36)) { 
						Gater->Say("Casting Circle of The Great Divide...");
						Gater->CastSpell(1438, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "cobalt")) && (c->GetLevel() >= 42)) { 
						Gater->Say("Casting Circle of Cobalt Scar...");
						Gater->CastSpell(1440, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "combines")) && (c->GetLevel() >= 33)) { 
						Gater->Say("Casting Circle of The Combines...");
						Gater->CastSpell(1517, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "surefall")) && (c->GetLevel() >= 26)) { 
						Gater->Say("Casting Circle of Surefall Glade...");
						Gater->CastSpell(2020, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "grimling")) && (c->GetLevel() >= 29)) { 
						Gater->Say("Casting Circle of Grimling Forest...");
						Gater->CastSpell(2419, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "twilight")) && (c->GetLevel() >= 33)) { 
						Gater->Say("Casting Circle of Twilight...");
						Gater->CastSpell(2424, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "dawnshroud")) && (c->GetLevel() >= 37)) { 
						Gater->Say("Casting Circle of Dawnshroud...");
						Gater->CastSpell(2429, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "nexus")) && (c->GetLevel() >= 26)) { 
						Gater->Say("Casting Circle of The Nexus...");
						Gater->CastSpell(2432, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "pok")) && (c->GetLevel() >= 38)) { 
						Gater->Say("Casting Circle of Knowledge...");
						Gater->CastSpell(3184, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "stonebrunt")) && (c->GetLevel() >= 28)) { 
						Gater->Say("Casting Circle of Stonebrunt Mountains...");
						Gater->CastSpell(3792, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "bloodfields")) && (c->GetLevel() >= 55)) { 
						Gater->Say("Casting Circle of Bloodfields...");
						Gater->CastSpell(6184, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "emerald")) && (c->GetLevel() >= 39)) { 
						Gater->Say("Casting Wind of the South...");
						Gater->CastSpell(1737, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "skyfire")) && (c->GetLevel() >= 44)) { 
						Gater->Say("Casting Wind of the North...");
						Gater->CastSpell(1736, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "slaughter")) && (c->GetLevel() >= 64)) { 
						Gater->Say("Casting Circle of Slaughter...");
						Gater->CastSpell(6179, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "karana") 
						|| !strcasecmp(sep->arg[2], "tox") 
						|| !strcasecmp(sep->arg[2], "butcher") && (c->GetLevel() <= 25))
						|| !strcasecmp(sep->arg[2], "commons") && (c->GetLevel() <= 27)
						|| (!strcasecmp(sep->arg[2], "ro") 
						|| !strcasecmp(sep->arg[2], "feerrott") && (c->GetLevel() <= 32))
						|| !strcasecmp(sep->arg[2], "steamfont") && (c->GetLevel() <= 31)
						|| !strcasecmp(sep->arg[2], "misty") && (c->GetLevel() <= 36)
						|| !strcasecmp(sep->arg[2], "lava") && (c->GetLevel() <= 30)
						|| !strcasecmp(sep->arg[2], "wakening") && (c->GetLevel() <= 40)
						|| !strcasecmp(sep->arg[2], "iceclad") && (c->GetLevel() <= 32)
						|| !strcasecmp(sep->arg[2], "divide") && (c->GetLevel() <= 38)
						|| !strcasecmp(sep->arg[2], "cobalt") && (c->GetLevel() <= 42)
						|| !strcasecmp(sep->arg[2], "combines") && (c->GetLevel() <= 33)
						|| !strcasecmp(sep->arg[2], "surefall") && (c->GetLevel() <= 26)
						|| !strcasecmp(sep->arg[2], "grimling") && (c->GetLevel() <= 29)
						|| !strcasecmp(sep->arg[2], "twilight") && (c->GetLevel() <= 33)
						|| !strcasecmp(sep->arg[2], "dawnshroud") && (c->GetLevel() <= 37)
						|| !strcasecmp(sep->arg[2], "nexus") && (c->GetLevel() <= 26)
						|| !strcasecmp(sep->arg[2], "pok") && (c->GetLevel() <= 38)
						|| !strcasecmp(sep->arg[2], "stonebrunt") && (c->GetLevel() <= 28)
						|| !strcasecmp(sep->arg[2], "bloodfields") && (c->GetLevel() <= 55)
						|| !strcasecmp(sep->arg[2], "emerald") && (c->GetLevel() <= 38)
						|| !strcasecmp(sep->arg[2], "skyfire") && (c->GetLevel() <= 43)
						|| !strcasecmp(sep->arg[2], "wos") && (c->GetLevel() <= 64)) {
							Gater->Say("I don't have the needed level yet", sep->arg[2]);
					}
					else {
						Gater->Say("With the proper level I can [gate] to [karana],[commons],[tox],[butcher],[lava],[ro],[feerrott],[steamfont],[misty],[wakening],[iceclad],[divide],[cobalt],[combines],[surefall],[grimling],[twilight],[dawnshroud],[nexus],[pok],[stonebrunt],[bloodfields],[emerald],[skyfire] or [wos].", c->GetName());
					}
					break;

				case WIZARD:

					if      ((!strcasecmp(sep->arg[2], "commons")) && (c->GetLevel() >= 35) ) {
						Gater->Say("Casting Common Portal...");
						Gater->CastSpell(566, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "fay")) && (c->GetLevel() >= 27)) {
						Gater->Say("Casting Fay Portal...");
						Gater->CastSpell(563, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "ro")) && (c->GetLevel() >= 37)) {
						Gater->Say("Casting Ro Portal...");
						Gater->CastSpell(567, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "tox")) && (c->GetLevel() >= 25)) {
						Gater->Say("Casting Toxxula Portal...");
						Gater->CastSpell(561, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "nk")) && (c->GetLevel() >= 27)) {
						Gater->Say("Casting North Karana Portal...");
						Gater->CastSpell(562, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "nek")) && (c->GetLevel() >= 32)) {
						Gater->Say("Casting Nektulos Portal...");
						Gater->CastSpell(564, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "wakening")) && (c->GetLevel() >= 43)) {
						Gater->Say("Casting Wakening Lands Portal...");
						Gater->CastSpell(1399, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "iceclad")) && (c->GetLevel() >= 33)) {
						Gater->Say("Casting Iceclad Ocean Portal...");
						Gater->CastSpell(1418, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "divide")) && (c->GetLevel() >= 36)) {
						Gater->Say("Casting Great Divide Portal...");
						Gater->CastSpell(1423, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "cobalt")) && (c->GetLevel() >= 43)) {
						Gater->Say("Casting Cobalt Scar Portal...");
						Gater->CastSpell(1425, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "combines")) && (c->GetLevel() >= 34)) {
						Gater->Say("Casting Combines Portal...");
						Gater->CastSpell(1516, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "wk")) && (c->GetLevel() >= 27)) {
						Gater->Say("Casting West Karana Portal...");
						Gater->CastSpell(568, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "twilight")) && (c->GetLevel() >= 27)) {
						Gater->Say("Casting Twilight Portal...");
						Gater->CastSpell(2425, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "dawnshroud")) && (c->GetLevel() >= 27)) {
						Gater->Say("Casting Dawnshroud Portal...");
						Gater->CastSpell(2430, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "nexus")) && (c->GetLevel() >= 29)) {
						Gater->Say("Casting Nexus Portal...");
						Gater->CastSpell(2944, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "pok")) && (c->GetLevel() >= 27)) {
						Gater->Say("Casting Plane of Knowledge Portal...");
						Gater->CastSpell(3180, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "wos")) && (c->GetLevel() >= 27)) {
						Gater->Say("Casting Wall of Slaughter Portal...");
						Gater->CastSpell(6178, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "grimling")) && (c->GetLevel() >= 29)) {
						Gater->Say("Casting Fay Portal...");
						Gater->CastSpell(2420, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "emerald")) && (c->GetLevel() >= 37)) {
						Gater->Say("Porting to Emerald Jungle...");
						Gater->CastSpell(1739, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "hateplane")) && (c->GetLevel() >= 39)) {
						Gater->Say("Porting to Hate Plane...");
						Gater->CastSpell(666, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "airplane")) && (c->GetLevel() >= 39)) {
						Gater->Say("Porting to airplane...");
						Gater->CastSpell(674, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "skyfire")) && (c->GetLevel() >= 36)) {
						Gater->Say("Porting to Skyfire...");
						Gater->CastSpell(1738, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "bloodfields")) && (c->GetLevel() >= 55)) {
						Gater->Say("Casting Bloodfields Portal...");
						Gater->CastSpell(6183, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "stonebrunt")) && (c->GetLevel() >= 27)) {
						Gater->Say("Casting Stonebrunt Portal...");
						Gater->CastSpell(3793, c->GetID(), 1, -1, -1);
					}
					else if ((!strcasecmp(sep->arg[2], "commons") && (c->GetLevel() <= 35))
						|| !strcasecmp(sep->arg[2], "fay") && (c->GetLevel() <= 27)
						|| (!strcasecmp(sep->arg[2], "ro") && (c->GetLevel() <= 37))
						|| !strcasecmp(sep->arg[2], "tox") && (c->GetLevel() <= 25)
						|| !strcasecmp(sep->arg[2], "nk") && (c->GetLevel() <= 25)
						|| !strcasecmp(sep->arg[2], "nek") && (c->GetLevel() <= 32)
						|| !strcasecmp(sep->arg[2], "wakening") && (c->GetLevel() <= 43)
						|| !strcasecmp(sep->arg[2], "iceclad") && (c->GetLevel() <= 33)
						|| !strcasecmp(sep->arg[2], "divide") && (c->GetLevel() <= 36)
						|| !strcasecmp(sep->arg[2], "cobalt") && (c->GetLevel() <= 43)
						|| !strcasecmp(sep->arg[2], "combines") && (c->GetLevel() <= 34)
						|| !strcasecmp(sep->arg[2], "wk") && (c->GetLevel() <= 37)
						|| !strcasecmp(sep->arg[2], "twilight") && (c->GetLevel() <= 33)
						|| !strcasecmp(sep->arg[2], "dawnshroud") && (c->GetLevel() <= 39)
						|| !strcasecmp(sep->arg[2], "nexus") && (c->GetLevel() <= 29)
						|| (!strcasecmp(sep->arg[2], "pok")
						|| !strcasecmp(sep->arg[2], "hateplane")
						|| !strcasecmp(sep->arg[2], "airplane") && (c->GetLevel() <= 38))
						|| !strcasecmp(sep->arg[2], "grimling") && (c->GetLevel() <= 29)
						|| !strcasecmp(sep->arg[2], "bloodfields") && (c->GetLevel() <= 55)
						|| !strcasecmp(sep->arg[2], "stonebrunt") && (c->GetLevel() <= 27)
						|| !strcasecmp(sep->arg[2], "emerald") && (c->GetLevel() <= 36)
						|| !strcasecmp(sep->arg[2], "skyfire") && (c->GetLevel() <= 36)
						|| !strcasecmp(sep->arg[2], "wos") && (c->GetLevel() <= 64)) {
							Gater->Say("I don't have the needed level yet", sep->arg[2]);
					}
					else {
						Gater->Say("With the proper level I can [gate] to [commons],[fay],[ro],[tox],[nk],[wakening],[iceclad],[divide],[cobalt],[combines],[wk],[grimling],[twilight],[dawnshroud],[nexus],[pok],[stonebrunt],[bloodfields],[emerald],[skyfire],[hateplane],[airplane] or [wos].", c->GetName());
					}
					break;
				default:
					c->Message(15, "You must have a Druid or Wizard in your group.");
					break;
			}
		}
	}

	//Endure Breath
	if ((!strcasecmp(sep->arg[1], "endureb")) && (c->IsGrouped())) {
		Mob *Endurer;
		int32 EndurerClass = 0;
		Group *g = c->GetGroup();
		if(g) {
			for(int i=0; i<MAX_GROUP_MEMBERS; i++){
				if(g->members[i] && g->members[i]->IsBot()) {
					switch(g->members[i]->GetClass()) {
						case DRUID:
							Endurer = g->members[i];
							EndurerClass = DRUID;
							break;
						case SHAMAN:
							if (EndurerClass != DRUID){
								Endurer = g->members[i];
								EndurerClass = SHAMAN;
							}
							break;
						case ENCHANTER:
							if(EndurerClass == 0){
								Endurer = g->members[i];
								EndurerClass = ENCHANTER;
							}
							break;
						case RANGER:
							if(EndurerClass == 0) {
								Endurer = g->members[i];
								EndurerClass = RANGER;
							}
							break;
						default:
							break;
					}
				}
			}
			switch(EndurerClass) {
				case DRUID:

					if  (c->GetLevel() <= 6) {
						Endurer->Say("I'm not level 6 yet.");
					}
					else if (zone->CanCastOutdoor()) {
						Endurer->Say("Casting Enduring Breath...");
						Endurer->CastSpell(86, c->GetID(), 1, -1, -1);
						break;
					}
					break;
				case SHAMAN:

					if ((zone->CanCastOutdoor()) && (c->GetLevel() >= 12)) { 
						Endurer->Say("Casting Enduring Breath...");
						Endurer->CastToClient()->CastSpell(86, c->GetID(), 1, -1, -1);
					}
					else if (c->GetLevel() <= 12) {
						Endurer->Say("I'm not level 12 yet.");
					}
					break;
				case RANGER:

					if((zone->CanCastOutdoor()) && (c->GetLevel() >= 20)){
						Endurer->Say("Casting Enduring Breath...");
						Endurer->CastToClient()->CastSpell(86, c->GetID(), 1, -1, -1);
					}
					else if (c->GetLevel() <= 20) {
						Endurer->Say("I'm not level 20 yet.");
					}
					break;
				case ENCHANTER:

					if((zone->CanCastOutdoor()) && (c->GetLevel() >= 12)) {
						Endurer->Say("Casting Enduring Breath...");
						Endurer->CastToClient()->CastSpell(86, c->GetID(), 1, -1, -1);
					}
					else if (c->GetLevel() <= 12) {
						Endurer->Say("I'm not level 12 yet.");
					}
					break;
				default:
					c->Message(15, "You must have a Druid, Shaman, Ranger, or Enchanter in your group.");
					break;
			}
		}
	}

	//Invisible
	if ((!strcasecmp(sep->arg[1], "invis")) && (c->IsGrouped())) {
		Mob *Inviser;
		int32 InviserClass = 0;
		Group *g = c->GetGroup();
		if(g) {
			for(int i=0; i<MAX_GROUP_MEMBERS; i++){
				if(g->members[i] && g->members[i]->IsBot()) {
					switch(g->members[i]->GetClass()) {
						case ENCHANTER:
							Inviser = g->members[i];
							InviserClass = ENCHANTER;
							break;
						case MAGICIAN:
							if (InviserClass != ENCHANTER){
								Inviser = g->members[i];
								InviserClass = MAGICIAN;
							}
							break;
						case WIZARD:
							if((InviserClass != ENCHANTER) || (InviserClass != MAGICIAN)){
								Inviser = g->members[i];
								InviserClass = WIZARD;
							}
							break;
						case NECROMANCER:
							if(InviserClass == 0){
								Inviser = g->members[i];
								InviserClass = NECROMANCER;
							}
							break;
						case DRUID:
							if((InviserClass != ENCHANTER) || (InviserClass != WIZARD)
								|| (InviserClass != MAGICIAN)){
									Inviser = g->members[i];
									InviserClass = DRUID;
							}
							break;
						default:
							break;
					}
				}
			}
			switch(InviserClass) {
				case ENCHANTER:
					if  ((c->GetLevel() <= 14) && (!strcasecmp(sep->arg[2], "undead"))) {
						Inviser->Say("I'm not level 14 yet.");
					}
					else if ((!c->IsInvisible(c)) && (!c->invisible_undead) && (c->GetLevel() >= 14) && (!strcasecmp(sep->arg[2], "undead"))) {
						Inviser->Say("Casting invis undead...");
						Inviser->CastSpell(235, c->GetID(), 1, -1, -1);
					}
					else if  ((c->GetLevel() <= 4) && (!strcasecmp(sep->arg[2], "live"))) {
						Inviser->Say("I'm not level 4 yet.");
					}
					else if ((!c->IsInvisible(c))&& (!c->invisible_undead) && (c->GetLevel() >= 4) && (!strcasecmp(sep->arg[2], "live"))) { 
						Inviser->Say("Casting invisibilty...");
						Inviser->CastSpell(42, c->GetID(), 1, -1, -1);
					}
					else if  ((c->GetLevel() <= 6) && (!strcasecmp(sep->arg[2], "see"))) {
						Inviser->Say("I'm not level 6 yet.");
					}
					else if ((c->GetLevel() >= 6) && (!strcasecmp(sep->arg[2], "see"))) { 
						Inviser->Say("Casting see invisible...");
						Inviser->CastSpell(80, c->GetID(), 1, -1, -1);
					}
					else if ((c->IsInvisible(c)) || (c->invisible_undead)) { 
						Inviser->Say("I can't cast this if you're already invis-buffed...");
					}
					else {
						Inviser->Say("Do you want [invis undead], [invis live] or [invis see] ?", c->GetName());
					}
					break;
				case MAGICIAN:
					if  (!strcasecmp(sep->arg[2], "undead")) {
						Inviser->Say("I don't have that spell.");
					}
					else if  ((c->GetLevel() <= 8) && (!strcasecmp(sep->arg[2], "live"))) {
						Inviser->Say("I'm not level 8 yet.");
					}
					else if ((!c->IsInvisible(c))&& (!c->invisible_undead) && (c->GetLevel() >= 8) && (!strcasecmp(sep->arg[2], "live"))) { 
						Inviser->Say("Casting invisibilty...");
						Inviser->CastSpell(42, c->GetID(), 1, -1, -1);
					}
					else if  ((c->GetLevel() <= 16) && (!strcasecmp(sep->arg[2], "see"))) {
						Inviser->Say("I'm not level 16 yet.");
					}
					else if ((c->GetLevel() >= 16) && (!strcasecmp(sep->arg[2], "see"))) { 
						Inviser->Say("Casting see invisible...");
						Inviser->CastSpell(80, c->GetID(), 1, -1, -1);
					}
					else if ((c->IsInvisible(c)) || (c->invisible_undead)) { 
						Inviser->Say("I can't cast this if you're already invis-buffed...");
					}
					else {
						Inviser->Say("Do you want [invis live] or [invis see] ?", c->GetName());
					}
					break;
				case WIZARD:
					if  ((c->GetLevel() <= 39) && (!strcasecmp(sep->arg[2], "undead"))) {
						Inviser->Say("I'm not level 39 yet.");
					}
					else if ((!c->IsInvisible(c))&& (!c->invisible_undead) && (c->GetLevel() >= 39) && (!strcasecmp(sep->arg[2], "undead"))) {
						Inviser->Say("Casting invis undead...");
						Inviser->CastSpell(235, c->GetID(), 1, -1, -1);
					}
					else if  ((c->GetLevel() <= 16) && (!strcasecmp(sep->arg[2], "live"))) {
						Inviser->Say("I'm not level 16 yet.");
					}
					else if ((!c->IsInvisible(c))&& (!c->invisible_undead) && (c->GetLevel() >= 16) && (!strcasecmp(sep->arg[2], "live"))) { 
						Inviser->Say("Casting invisibilty...");
						Inviser->CastSpell(42, c->GetID(), 1, -1, -1);
					}
					else if  ((c->GetLevel() <= 4) && (!strcasecmp(sep->arg[2], "see"))) {
						Inviser->Say("I'm not level 6 yet.");
					}
					else if ((c->GetLevel() >= 4) && (!strcasecmp(sep->arg[2], "see"))) { 
						Inviser->Say("Casting see invisible...");
						Inviser->CastSpell(80, c->GetID(), 1, -1, -1);
					}
					else if ((c->IsInvisible(c)) || (c->invisible_undead)) { 
						Inviser->Say("I can't cast this if you're already invis-buffed...");
					}
					else {
						Inviser->Say("Do you want [invis undead], [invis live] or [invis see] ?", c->GetName());
					}
					break;
				case NECROMANCER:
					if ((!c->IsInvisible(c))&& (!c->invisible_undead) && (!strcasecmp(sep->arg[2], "undead"))) {
						Inviser->Say("Casting invis undead...");
						Inviser->CastSpell(235, c->GetID(), 1, -1, -1);
					}
					else if (!strcasecmp(sep->arg[2], "see")) { 
						Inviser->Say("I don't have that spell...");
					}
					else if (!strcasecmp(sep->arg[2], "live")) { 
						Inviser->Say("I don't have that spell...");
					}
					else if ((c->IsInvisible(c))|| (c->invisible_undead)) { 
						Inviser->Say("I can't cast this if you're already invis-buffed...");
					}
					else {
						Inviser->Say("I only have [invis undead]", c->GetName());
					}
					break;
				case DRUID:
					if  (!strcasecmp(sep->arg[2], "undead")) {
						Inviser->Say("I don't have that spell...");
					}
					else if  ((c->GetLevel() <= 4) && (!strcasecmp(sep->arg[2], "live"))) {
						Inviser->Say("I'm not level 4 yet.");
					}
					else if ((!c->IsInvisible(c))&& (!c->invisible_undead) && (c->GetLevel() >= 18) && (!strcasecmp(sep->arg[2], "live"))) { 
						Inviser->Say("Casting Superior Camouflage...");
						Inviser->CastSpell(34, c->GetID(), 1, -1, -1);
					}
					else if ((!c->IsInvisible(c))&& (!c->invisible_undead) && (c->GetLevel() >= 4) && (!strcasecmp(sep->arg[2], "live")) && (zone->CanCastOutdoor())) { 
						Inviser->Say("Casting Camouflage...");
						Inviser->CastSpell(247, c->GetID(), 1, -1, -1);
					}
					else if ((c->GetLevel() >= 4) && (!strcasecmp(sep->arg[2], "live")) && (!zone->CanCastOutdoor())) { 
						Inviser->Say("I can't cast this spell indoors...");
					}
					else if  ((c->GetLevel() <= 13) && (!strcasecmp(sep->arg[2], "see"))) {
						Inviser->Say("I'm not level 13 yet.");
					}
					else if ((c->GetLevel() >= 13) && (!strcasecmp(sep->arg[2], "see"))) { 
						Inviser->Say("Casting see invisible...");
						Inviser->CastSpell(80, c->GetID(), 1, -1, -1);
					}
					else if ((c->IsInvisible(c)) || (c->invisible_undead)) { 
						Inviser->Say("I can't cast this if you're already invis-buffed...");
					}
					else {
						Inviser->Say("Do you want [invis live] or [invis see] ?", c->GetName());
					}
					break;
				default:
					c->Message(15, "You must have a Enchanter, Magician, Wizard, Druid, or Necromancer in your group.");
					break;
			}
		}
	}

	//Levitate
	if ((!strcasecmp(sep->arg[1], "levitate")) && (c->IsGrouped())) {
		Mob *Lever;
		int32 LeverClass = 0;
		Group *g = c->GetGroup();
		if(g) {
			for(int i=0; i<MAX_GROUP_MEMBERS; i++){
				if(g->members[i] && g->members[i]->IsBot()) {
					switch(g->members[i]->GetClass()) {
						case DRUID:
							Lever = g->members[i];
							LeverClass = DRUID;
							break;
						case SHAMAN:
							if (LeverClass != DRUID){
								Lever = g->members[i];
								LeverClass = SHAMAN;
							}
							break;
						case WIZARD:
							if(LeverClass == 0){
								Lever = g->members[i];
								LeverClass = WIZARD;
							}
							break;
						case ENCHANTER:
							if (LeverClass == 0) {
								Lever = g->members[i];
								LeverClass = ENCHANTER;
							}
							break;
						default:
							break;
					}
				}
			}
			switch(LeverClass) {
				case DRUID:
					if  (c->GetLevel() <= 14) {
						Lever->Say("I'm not level 14 yet.");
					}
					else if (zone->CanCastOutdoor()) {
						Lever->Say("Casting Levitate...");
						Lever->CastSpell(261, c->GetID(), 1, -1, -1);
						break;
					}
					else if (!zone->CanCastOutdoor()) {
						Lever->Say("I can't cast this spell indoors", c->GetName());
					}
					break;

				case SHAMAN:

					if ((zone->CanCastOutdoor()) && (c->GetLevel() >= 10)) { 
						Lever->Say("Casting Levitate...");
						Lever->CastToClient()->CastSpell(261, c->GetID(), 1, -1, -1);
					}
					else if (!zone->CanCastOutdoor()) {
						Lever->Say("I can't cast this spell indoors", c->GetName());
					}
					else if (c->GetLevel() <= 10) {
						Lever->Say("I'm not level 10 yet.");
					}
					break;

				case WIZARD:

					if((zone->CanCastOutdoor()) && (c->GetLevel() >= 22)){
						Lever->Say("Casting Levitate...");
						Lever->CastToClient()->CastSpell(261, c->GetID(), 1, -1, -1);
					}
					else if (!zone->CanCastOutdoor()) {
						Lever->Say("I can't cast this spell indoors", c->GetName());
					}
					else if (c->GetLevel() <= 22) {
						Lever->Say("I'm not level 22 yet.");
					}
					break;

				case ENCHANTER:

					if((zone->CanCastOutdoor()) && (c->GetLevel() >= 15)) {
						Lever->Say("Casting Levitate...");
						Lever->CastToClient()->CastSpell(261, c->GetID(), 1, -1, -1);
					}
					else if (!zone->CanCastOutdoor()) {
						Lever->Say("I can't cast this spell indoors", c->GetName());
					}
					else if (c->GetLevel() <= 15) {
						Lever->Say("I'm not level 15 yet.");
					}
					break;


				default:
					c->Message(15, "You must have a Druid, Shaman, Wizard, or Enchanter in your group.");
					break;
			}
		}
	}

	//Resists
	if ((!strcasecmp(sep->arg[1], "resist")) && (c->IsGrouped())) {
		Mob *Resister;
		int32 ResisterClass = 0;
		Group *g = c->GetGroup();
		if(g) {
			for(int i=0; i<MAX_GROUP_MEMBERS; i++){
				if(g->members[i] && g->members[i]->IsBot()) {
					switch(g->members[i]->GetClass()) {
						case CLERIC:
							Resister = g->members[i];
							ResisterClass = CLERIC;
							break;
						case SHAMAN:
							if(ResisterClass != CLERIC){
								Resister = g->members[i];
								ResisterClass = SHAMAN;
							}
						case DRUID:
							if (ResisterClass == 0){
								Resister = g->members[i];
								ResisterClass = DRUID;
							}
							break;
							break;
						default:
							break;
					}
				}
			}
			switch(ResisterClass) {
				case CLERIC:
					if	(!strcasecmp(sep->arg[2], "poison") && (c->GetLevel() >= 6))  {
						Resister->Say("Casting poison protection...", sep->arg[2]);
						Resister->CastToBot()->Bot_Command_Resist(1, Resister->GetLevel());
					}
					else if (!strcasecmp(sep->arg[2], "disease") && (c->GetLevel() >= 11)) {
						Resister->Say("Casting disease protection...", sep->arg[2]);
						Resister->CastToBot()->Bot_Command_Resist(2, Resister->GetLevel());
					}
					else if(!strcasecmp(sep->arg[2], "fire") && (c->GetLevel() >= 8)) {
						Resister->Say("Casting fire protection...", sep->arg[2]);
						Resister->CastToBot()->Bot_Command_Resist(3, Resister->GetLevel());
					}
					else if(!strcasecmp(sep->arg[2], "cold") && (c->GetLevel() >= 13)) {
						Resister->Say("Casting cold protection...", sep->arg[2]);
						Resister->CastToBot()->Bot_Command_Resist(4, Resister->GetLevel());
					}
					else if(!strcasecmp(sep->arg[2], "magic") && (c->GetLevel() >= 16)) {
						Resister->Say("Casting magic protection...", sep->arg[2]);
						Resister->CastToBot()->Bot_Command_Resist(5, Resister->GetLevel());
					}
					else if (!strcasecmp(sep->arg[2], "magic") && (c->GetLevel() <= 16)
						|| !strcasecmp(sep->arg[2], "cold") && (c->GetLevel() <= 13)
						|| !strcasecmp(sep->arg[2], "fire") && (c->GetLevel() <= 8) 
						|| !strcasecmp(sep->arg[2], "disease") && (c->GetLevel() <= 11)
						|| !strcasecmp(sep->arg[2], "poison") && (c->GetLevel() <= 6)) {
							Resister->Say("I don't have the needed level yet", sep->arg[2]);
					}
					else 
						Resister->Say("Do you want [resist poison], [resist disease], [resist fire], [resist cold], or [resist magic]?", c->GetName());

					break;

				case SHAMAN:
					if	(!strcasecmp(sep->arg[2], "poison") && (c->GetLevel() >= 20))  {
						Resister->Say("Casting poison protection...", sep->arg[2]);
						Resister->CastToBot()->Bot_Command_Resist(12, Resister->GetLevel());
					}
					else if (!strcasecmp(sep->arg[2], "disease") && (c->GetLevel() >= 8)) {
						Resister->Say("Casting disease protection...", sep->arg[2]);
						Resister->CastToBot()->Bot_Command_Resist(13, Resister->GetLevel());
					}
					else if(!strcasecmp(sep->arg[2], "fire") && (c->GetLevel() >= 5)) {
						Resister->Say("Casting fire protection...", sep->arg[2]);
						Resister->CastToBot()->Bot_Command_Resist(14, Resister->GetLevel());
					}
					else if(!strcasecmp(sep->arg[2], "cold") && (c->GetLevel() >= 1)) {
						Resister->Say("Casting cold protection...", sep->arg[2]);
						Resister->CastToBot()->Bot_Command_Resist(15, Resister->GetLevel());
					}
					else if(!strcasecmp(sep->arg[2], "magic") && (c->GetLevel() >= 19)) {
						Resister->Say("Casting magic protection...", sep->arg[2]);
						Resister->CastToBot()->Bot_Command_Resist(16, Resister->GetLevel());
					}
					else if (!strcasecmp(sep->arg[2], "magic") && (c->GetLevel() <= 19)
						|| !strcasecmp(sep->arg[2], "cold") && (c->GetLevel() <= 1)
						|| !strcasecmp(sep->arg[2], "fire") && (c->GetLevel() <= 5) 
						|| !strcasecmp(sep->arg[2], "disease") && (c->GetLevel() <= 8)
						|| !strcasecmp(sep->arg[2], "poison") && (c->GetLevel() <= 20)) {
							Resister->Say("I don't have the needed level yet", sep->arg[2]);
					}
					else 
						Resister->Say("Do you want [resist poison], [resist disease], [resist fire], [resist cold], or [resist magic]?", c->GetName());

					break;

				case DRUID:

					if	(!strcasecmp(sep->arg[2], "poison") && (c->GetLevel() >= 19)) {
						Resister->Say("Casting poison protection...", sep->arg[2]);
						Resister->CastToBot()->Bot_Command_Resist(7, Resister->GetLevel());
					}
					else if (!strcasecmp(sep->arg[2], "disease") && (c->GetLevel() >= 19)) {
						Resister->Say("Casting disease protection...", sep->arg[2]);
						Resister->CastToBot()->Bot_Command_Resist(8, Resister->GetLevel());
					}
					else if(!strcasecmp(sep->arg[2], "fire")) { // Fire level 1
						Resister->Say("Casting fire protection...", sep->arg[2]);
						Resister->CastToBot()->Bot_Command_Resist(9, Resister->GetLevel());
					}
					else if(!strcasecmp(sep->arg[2], "cold") && (c->GetLevel() >= 13)) {
						Resister->Say("Casting cold protection...", sep->arg[2]);
						Resister->CastToBot()->Bot_Command_Resist(10, Resister->GetLevel());
					}
					else if(!strcasecmp(sep->arg[2], "magic") && (c->GetLevel() >= 16)) {
						Resister->Say("Casting magic protection...", sep->arg[2]);
						Resister->CastToBot()->Bot_Command_Resist(11, Resister->GetLevel());
					}
					else if (!strcasecmp(sep->arg[2], "magic") && (c->GetLevel() <= 16)
						|| !strcasecmp(sep->arg[2], "cold") && (c->GetLevel() <= 9)
						|| !strcasecmp(sep->arg[2], "disease") && (c->GetLevel() <= 19)
						|| !strcasecmp(sep->arg[2], "poison") && (c->GetLevel() <= 19)) {
							Resister->Say("I don't have the needed level yet", sep->arg[2]) ;
					}
					else 
						Resister->Say("Do you want [resist poison], [resist disease], [resist fire], [resist cold], or [resist magic]?", c->GetName());

					break;

				default:
					c->Message(15, "You must have a Cleric, Shaman, or Druid in your group.");
					break;
			}
		}
	}

	// debug commands
	if(!strcasecmp(sep->arg[1], "debug") && !strcasecmp(sep->arg[2], "inventory")) {
		Mob *target = c->GetTarget();

		if(target && target->IsBot()) {
			for(int i=0; i<9; i++) {
				c->Message(15,"Equiped slot: %i , item: %i \n", i, target->CastToBot()->GetEquipment(i));
			}
			if(target->CastToBot()->GetEquipment(8) > 0)
				c->Message(15,"This bot has an item in off-hand.");
		}
		return;
	}

	if(!strcasecmp(sep->arg[1], "debug") && !strcasecmp(sep->arg[2], "botcaracs"))
	{
		Mob *target = c->GetTarget();
		if(target && target->IsBot())
		{
			if(target->CanThisClassDualWield())
				c->Message(15, "This class can dual wield.");
			if(target->CanThisClassDoubleAttack())
				c->Message(15, "This class can double attack.");
		}
		if(target->GetPetID())
			c->Message(15, "I've a pet and its name is %s", target->GetPet()->GetCleanName() );
		return;
	}

	if(!strcasecmp(sep->arg[1], "debug") && !strcasecmp(sep->arg[2], "spells"))
	{
		Mob *target = c->GetTarget();
		if(target && target->IsBot())
		{
			for(int i=0; i<16; i++)
			{
				if(target->CastToBot()->BotGetSpells(i) != 0)
				{
					SPDat_Spell_Struct botspell = spells[target->CastToBot()->BotGetSpells(i)];
					c->Message(15, "(DEBUG) %s , Slot(%i), Spell (%s) Priority (%i) \n", target->GetCleanName(), i, botspell.name, target->CastToBot()->BotGetSpellPriority(i));
				}
			}
		}
		return;
	}



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

// franck: EQoffline
// This function has been reworked for the caster bots, when engaged.
// Healers bots must heal thoses who loose HP.
bool EntityList::Bot_AICheckCloseBeneficialSpells(Bot* caster, int8 iChance, float iRange, int16 iSpellTypes) {
	_ZP(EntityList_Bot_AICheckCloseBeneficialSpells);

	if((iSpellTypes&SpellTypes_Detrimental) != 0) {
		//according to live, you can buff and heal through walls...
		//now with PCs, this only applies if you can TARGET the target, but
		// according to Rogean, Live NPCs will just cast through walls/floors, no problem..
		//
		// This check was put in to address an idle-mob CPU issue
		_log(AI__ERROR, "Error: detrimental spells requested from AICheckCloseBeneficialSpells!!");
		return(false);
	}
	
	if(!caster)
		return false;

	if(!caster->AI_HasSpells())
		return false;

	if (iChance < 100) {
		int8 tmp = MakeRandomInt(1, 100);
		if (tmp > iChance)
			return false;
	}

	// Franck: EQoffline.
	// Ok, Beneficial spells depend of the class of the caster also..
	int8 botCasterClass = caster->GetClass();;

	// Heal and buffs spells might have a different chance, that's why I separe them .
	if( botCasterClass == CLERIC || botCasterClass == DRUID || botCasterClass == SHAMAN || botCasterClass == PALADIN || botCasterClass == BEASTLORD || botCasterClass == RANGER) {
		//If AI_EngagedCastCheck() said to the healer that he had to heal
		if( iSpellTypes == SpellType_Heal )	// 
		{
			// check raids
			if( caster->CastToMob()->IsGrouped() && caster->CastToBot()->IsBotRaiding() 
				// TODO: Uncomment this line after bot raids are integrated
				//&& (entity_list.GetBotRaidByMob(caster) != NULL)
				) {
				// TODO: Uncomment this block of code after bot raids are integrated
				//BotRaids *br = entity_list.GetBotRaidByMob(caster);
				//// boolean trying to ai the heal rotation, prolly not working well.
				//if(br) {
				//	if(br->GetBotMainTank() && (br->GetBotMainTank()->GetHPRatio() < 80))
				//	{
				//		if(caster->Bot_AICastSpell(br->GetBotMainTank(), 80, SpellType_Heal)) {
				//			return true;
				//		}
				//	}
				//	else if(br->GetBotSecondTank() && (br->GetBotSecondTank()->GetHPRatio() < 80))
				//	{
				//		if(caster->Bot_AICastSpell(br->GetBotSecondTank(), 80, SpellType_Heal)) {
				//			return true;
				//		}
				//	}
				//}
			}

			// check in group
			if(caster->IsGrouped()) {
				Group *g = entity_list.GetGroupByMob(caster);
				
				if(g) {
					for( int i = 0; i<MAX_GROUP_MEMBERS; i++) {
						if(g->members[i] && !g->members[i]->qglobal && (g->members[i]->GetHPRatio() < 80)) {
							if(caster->Bot_AICastSpell(g->members[i], 100, SpellType_Heal))
								return true;
						}
						if(g->members[i] && !g->members[i]->qglobal && g->members[i]->GetPetID()) {
							if(caster->Bot_AICastSpell(g->members[i]->GetPet(), 60, SpellType_Heal))
								return true;
						}
					}
				}
			}
		}
	}

	//Ok for the buffs..
	if( iSpellTypes == SpellType_Buff) {
		// Let's try to make Bard working...
		if(botCasterClass == BARD) {
			if(caster->Bot_AICastSpell(caster, 100, SpellType_Buff))
				return true;
			else
				return false;
		}

		if(caster->IsGrouped() )
		{
			Group *g = entity_list.GetGroupByMob(caster);
			
			if(g) {
				for( int i = 0; i<MAX_GROUP_MEMBERS; i++) {
					if(g->members[i]) {
						if(caster->Bot_AICastSpell(g->members[i], 100, SpellType_Buff)) {
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

// TODO: Refactor this method.
// This method contains a large amount of redundant code that can be replaced with private methods
void Bot::CalcBotStats(bool showtext) {
	Client* BotOwner = this->GetBotOwner()->CastToClient();

	if(!BotOwner)
		return;

	if(showtext) {
		BotOwner->Message(15, "Bot updating...");
	}
	// base stats
	int brace = GetBaseRace(); // Angelox
	int bclass = GetClass();
	int blevel = GetLevel();
	
	// Check Race/Class combos
	bool isComboAllowed = false;
	switch(brace) {
		case 1: // Human
			switch(bclass) {
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
					isComboAllowed = true;
					break;
			}
			break;
		case 2: // Barbarian
			switch(bclass) {
				case 1: // Warrior
				case 9: // Rogue
				case 10: // Shaman
				case 15: // Beastlord
				case 16: // Berserker
					isComboAllowed = true;
					break;
			}
			break;
		case 3: // Erudite
			switch(bclass) {
				case 2: // Cleric
				case 3: // Paladin
				case 5: // Shadowknight
				case 11: // Necromancer
				case 12: // Wizard
				case 13: // Magician
				case 14: // Enchanter
					isComboAllowed = true;
					break;
			}
			break;
		case 4: // Wood Elf
			switch(bclass) {
				case 1: // Warrior
				case 4: // Ranger
				case 6: // Druid
				case 8: // Bard
				case 9: // Rogue
					isComboAllowed = true;
					break;
			}
			break;
		case 5: // High Elf
			switch(bclass) {
				case 2: // Cleric
				case 3: // Paladin
				case 12: // Wizard
				case 13: // Magician
				case 14: // Enchanter
					isComboAllowed = true;
					break;
			}
			break;
		case 6: // Dark Elf
			switch(bclass) {
				case 1: // Warrior
				case 2: // Cleric
				case 5: // Shadowknight
				case 9: // Rogue
				case 11: // Necromancer
				case 12: // Wizard
				case 13: // Magician
				case 14: // Enchanter
					isComboAllowed = true;
					break;
			}
			break;
		case 7: // Half Elf
			switch(bclass) {
				case 1: // Warrior
				case 3: // Paladin
				case 4: // Ranger
				case 6: // Druid
				case 8: // Bard
				case 9: // Rogue
					isComboAllowed = true;
					break;
			}
			break;
		case 8: // Dwarf
			switch(bclass) {
				case 1: // Warrior
				case 2: // Cleric
				case 3: // Paladin
				case 9: // Rogue
				case 16: // Berserker
					isComboAllowed = true;
					break;
			}
			break;
		case 9: // Troll
			switch(bclass) {
				case 1: // Warrior
				case 5: // Shadowknight
				case 10: // Shaman
				case 15: // Beastlord
				case 16: // Berserker
					isComboAllowed = true;
					break;
			}
			break;
		case 10: // Ogre
			switch(bclass) {
				case 1: // Warrior
				case 5: // Shadowknight
				case 10: // Shaman
				case 15: // Beastlord
				case 16: // Berserker
					isComboAllowed = true;
					break;
			}
			break;
		case 11: // Halfling
			switch(bclass) {
				case 1: // Warrior
				case 2: // Cleric
				case 3: // Paladin
				case 4: // Ranger
				case 6: // Druid
				case 9: // Rogue
					isComboAllowed = true;
					break;
			}
			break;
		case 12: // Gnome
			switch(bclass) {
				case 1: // Warrior
				case 2: // Cleric
				case 3: // Paladin
				case 5: // Shadowknight
				case 9: // Rogue
				case 11: // Necromancer
				case 12: // Wizard
				case 13: // Magician
				case 14: // Enchanter
					isComboAllowed = true;
					break;
			}
			break;
		case 128: // Iksar
			switch(bclass) {
				case 1: // Warrior
				case 5: // Shadowknight
				case 7: // Monk
				case 10: // Shaman
				case 11: // Necromancer
				case 15: // Beastlord
					isComboAllowed = true;
					break;
			}
			break;
		case 130: // Vah Shir
			switch(bclass) {
				case 1: // Warrior
				case 8: // Bard
				case 9: // Rogue
				case 10: // Shaman
				case 15: // Beastlord
				case 16: // Berserker
					isComboAllowed = true;
					break;
			}
			break;
		case 330: // Froglok
			switch(bclass) {
				case 1: // Warrior
				case 2: // Cleric
				case 3: // Paladin
				case 5: // Shadowknight
				case 9: // Rogue
				case 10: // Shaman
				case 11: // Necromancer
				case 12: // Wizard
					isComboAllowed = true;
					break;
			}
			break;
	}
	if(!isComboAllowed) {
		BotOwner->Message(15, "A %s - %s bot was detected. Is this Race/Class combination allowed?.", GetRaceName(brace), GetEQClassName(bclass, blevel));
		BotOwner->Message(15, "Previous Bots Code releases did not check Race/Class combinations during create.");
		BotOwner->Message(15, "Unless you are experiencing heavy lag, you should delete and remake this bot.");
	}
	isComboAllowed = false;

	int spellid = 0;
	// base stats
	sint16 bstr = 75;
	sint16 bsta = 75;
	sint16 bdex = 75;
	sint16 bagi = 75;
	sint16 bwis = 75;
	sint16 bint = 75;
	sint16 bcha = 75;
	sint16 bATK = 5;
	sint16 bMR = 25;
	sint16 bFR = 25;
	sint16 bDR = 15;
	sint16 bPR = 15;
	sint16 bCR = 25;

	switch(bclass) {
		case 1: // Warrior
			bstr += 10;
			bsta += 20;
			bagi += 10;
			bdex += 10;
			bATK += 12;
			bMR += (blevel / 2 + 1);
			break;
		case 2: // Cleric
			spellid = 701;
			bstr += 5;
			bsta += 5;
			bagi += 10;
			bwis += 30;
			bATK += 8;
			break;
		case 3: // Paladin
			spellid = 708;
			bstr += 15;
			bsta += 5;
			bwis += 15;
			bcha += 10;
			bdex += 5;
			bATK += 17;
			bDR += 8;
			if(blevel > 50) {
				bDR += (blevel - 50);
			}
			break;
		case 4: // Ranger
			spellid = 710;
			bstr += 15;
			bsta += 10;
			bagi += 10;
			bwis += 15;
			bATK += 17;
			bFR += 4;
			if(blevel > 50) {
				bFR += (blevel - 50);
			}
			bCR += 4;
			if(blevel > 50) {
				bCR += (blevel - 50);
			}
			break;
		case 5: // Shadowknight
			spellid = 709;
			bstr += 10;
			bsta += 15;
			bint += 20;
			bcha += 5;
			bATK += 17;
			bPR += 4;
			if(blevel > 50) {
				bPR += (blevel - 50);
			}
			bDR += 4;
			if(blevel > 50) {
				bDR += (blevel - 50);
			}
			break;
		case 6: // Druid
			spellid = 707;
			bsta += 15;
			bwis += 35;
			bATK += 5;
			break;
		case 7: // Monk
			bstr += 5;
			bsta += 15;
			bagi += 15;
			bdex += 15;
			bATK += 17;
			break;
		case 8: // Bard
			spellid = 711;
			bstr += 15;
			bdex += 10;
			bcha += 15;
			bint += 10;
			bATK += 17;
			break;
		case 9: // Rogue
			bstr += 10;
			bsta += 20;
			bagi += 10;
			bdex += 10;
			bATK += 12;
			bPR += 8;
			if(blevel > 50) {
				bPR += (blevel - 50);
			}
			break;
		case 10: // Shaman
			spellid = 706;
			bsta += 10;
			bwis += 30;
			bcha += 10;
			bATK += 28;
			break;
		case 11: // Necromancer
			spellid = 703;
			bdex += 10;
			bagi += 10;
			bint += 30;
			bATK += 5;
			break;
		case 12: // Wizard
			spellid = 702;
			bsta += 20;
			bint += 30;
			bATK += 5;
			break;
		case 13: // Magician
			spellid = 704;
			bsta += 20;
			bint += 30;
			bATK += 5;
			break;
		case 14: // Enchanter
			spellid = 705;
			bint += 25;
			bcha += 25;
			bATK += 5;
			break;
		case 15: // Beastlord
			spellid = 712;
			bsta += 10;
			bagi += 10;
			bdex += 5;
			bwis += 20;
			bcha += 5;
			bATK += 31;
			break;
		case 16: // Berserker
			bstr += 10;
			bsta += 15;
			bdex += 15;
			bagi += 10;
			bATK += 25;
			break;
	}

	float bsize = 6.0;
	switch(brace) {
		case 1: // Humans have no race bonus
			break;
		case 2: // Barbarian
			bstr += 28;
			bsta += 20;
			bagi += 7;
			bdex -= 5;
			bwis -= 5;
			bint -= 10;
			bcha -= 20;
			bsize = 7;
			bCR += 10;
			break;
		case 3: // Erudite
			bstr -= 15;
			bsta -= 5;
			bagi -= 5;
			bdex -= 5;
			bwis += 8;
			bint += 32;
			bcha -= 5;
			bMR += 5;
			bDR -= 5;
			break;
		case 4: // Wood Elf
			bstr -= 10;
			bsta -= 10;
			bagi += 20;
			bdex += 5;
			bwis += 5;
			bsize = 5;
			break;
		case 5: // High Elf
			bstr -= 20;
			bsta -= 10;
			bagi += 10;
			bdex -= 5;
			bwis += 20;
			bint += 12;
			bcha += 5;
			break;
		case 6: // Dark Elf
			bstr -= 15;
			bsta -= 10;
			bagi += 15;
			bwis += 8;
			bint += 24;
			bcha -= 15;
			bsize = 5;
			break;
		case 7: // Half Elf
			bstr -= 5;
			bsta -= 5;
			bagi += 15;
			bdex += 10;
			bwis -= 15;
			bsize = 5.5;
			break;
		case 8: // Dwarf
			bstr += 15;
			bsta += 15;
			bagi -= 5;
			bdex += 15;
			bwis += 8;
			bint -= 15;
			bcha -= 30;
			bsize = 4;
			bMR -= 5;
			bPR += 5;
			break;
		case 9: // Troll
			bstr += 33;
			bsta += 34;
			bagi += 8;
			bwis -= 15;
			bint -= 23;
			bcha -= 35;
			bsize = 8;
			bFR -= 20;
			break;
		case 10: // Ogre
			bstr += 55;
			bsta += 77;
			bagi -= 5;
			bdex -= 5;
			bwis -= 8;
			bint -= 15;
			bcha -= 38;
			bsize = 9;
			break;
		case 11: // Halfling
			bstr -= 5;
			bagi += 20;
			bdex += 15;
			bwis += 5;
			bint -= 8;
			bcha -= 25;
			bsize = 3.5;
			bPR += 5;
			bDR += 5;
			break;
		case 12: // Gnome
			bstr -= 15;
			bsta -= 5;
			bagi += 10;
			bdex += 10;
			bwis -= 8;
			bint += 23;
			bcha -= 15;
			bsize = 3;
			break;
		case 128: // Iksar
			bstr -= 5;
			bsta -= 5;
			bagi += 15;
			bdex += 10;
			bwis += 5;
			bcha -= 20;
			bMR -= 5;
			bFR -= 5;
			break;
		case 130: // Vah Shir
			bstr += 15;
			bagi += 15;
			bdex -= 5;
			bwis -= 5;
			bint -= 10;
			bcha -= 10;
			bsize = 7;
			bMR -= 5;
			bFR -= 5;
			break;
		case 330: // Froglok
			bstr -= 5;
			bsta += 5;
			bagi += 25;
			bdex += 25;
			bcha -= 25;
			bsize = 5;
			bMR -= 5;
			bFR -= 5;
			break;
	}

	// General AA bonus'
	if(blevel >= 51 ) {
		bstr += 2;	// Innate Strength AA 1
		bsta += 2;	// Innate Stamina AA 1
		bagi += 2;	// Innate Agility AA 1
		bdex += 2;	// Innate Dexterity AA 1
		bint += 2;	// Innate Intelligence AA 1
		bwis += 2;	// Innate Wisdom AA 1
		bcha += 2;	// Innate Charisma AA 1
		bFR += 2;	// Innate Fire Protection AA 1
		bCR += 2;	// Innate Cold Protection AA 1
		bMR += 2;	// Innate Magic Protection AA 1
		bPR += 2;	// Innate Poison Protection AA 1
		bDR += 2;	// Innate Disease AA 1
	}
	if(blevel >= 52 ) {
		bstr += 2;	// Innate Strength AA 2
		bsta += 2;	// Innate Stamina AA 2
		bagi += 2;	// Innate Agility AA 2
		bdex += 2;	// Innate Dexterity AA 2
		bint += 2;	// Innate Intelligence AA 2
		bwis += 2;	// Innate Wisdom AA 2
		bcha += 2;	// Innate Charisma AA 2
		bFR += 2;	// Innate Fire Protection AA 2
		bCR += 2;	// Innate Cold Protection AA 2
		bMR += 2;	// Innate Magic Protection AA 2
		bPR += 2;	// Innate Poison Protection AA 2
		bDR += 2;	// Innate Disease AA 2
	}
	if(blevel >= 53 ) {
		bstr += 2;	// Innate Strength AA 3
		bsta += 2;	// Innate Stamina AA 3
		bagi += 2;	// Innate Agility AA 3
		bdex += 2;	// Innate Dexterity AA 3
		bint += 2;	// Innate Intelligence AA 3
		bwis += 2;	// Innate Wisdom AA 3
		bcha += 2;	// Innate Charisma AA 3
		bFR += 2;	// Innate Fire Protection AA 3
		bCR += 2;	// Innate Cold Protection AA 3
		bMR += 2;	// Innate Magic Protection AA 3
		bPR += 2;	// Innate Poison Protection AA 3
		bDR += 2;	// Innate Disease AA 3
	}
	if(blevel >= 54 ) {
		bstr += 2;	// Innate Strength AA 4
		bsta += 2;	// Innate Stamina AA 4
		bagi += 2;	// Innate Agility AA 4
		bdex += 2;	// Innate Dexterity AA 4
		bint += 2;	// Innate Intelligence AA 4
		bwis += 2;	// Innate Wisdom AA 4
		bcha += 2;	// Innate Charisma AA 4
		bFR += 2;	// Innate Fire Protection AA 4
		bCR += 2;	// Innate Cold Protection AA 4
		bMR += 2;	// Innate Magic Protection AA 4
		bPR += 2;	// Innate Poison Protection AA 4
		bDR += 2;	// Innate Disease AA 4
	}
	if(blevel >= 55 ) {
		bstr += 2;	// Innate Strength AA 5
		bsta += 2;	// Innate Stamina AA 5
		bagi += 2;	// Innate Agility AA 5
		bdex += 2;	// Innate Dexterity AA 5
		bint += 2;	// Innate Intelligence AA 5
		bwis += 2;	// Innate Wisdom AA 5
		bcha += 2;	// Innate Charisma AA 5
		bFR += 2;	// Innate Fire Protection AA 5
		bCR += 2;	// Innate Cold Protection AA 5
		bMR += 2;	// Innate Magic Protection AA 5
		bPR += 2;	// Innate Poison Protection AA 5
		bDR += 2;	// Innate Disease AA 5
	}
	if(blevel >= 61 ) {
		bstr += 2;	// Advanced Innate Strength AA 1
		bsta += 2;	// Advanced Innate Stamina AA 1
		bagi += 2;	// Advanced Innate Agility AA 1
		bdex += 2;	// Advanced Innate Dexterity AA 1
		bint += 2;	// Advanced Innate Intelligence AA 1
		bwis += 2;	// Advanced Innate Wisdom AA 1
		bcha += 2;	// Advanced Innate Charisma AA 1
		bFR += 2;	// Warding of Solusek AA 1
		bCR += 2;	// Blessing of E'ci AA 1
		bMR += 2;	// Marr's Protection AA 1
		bPR += 2;	// Shroud of the Faceless AA 1
		bDR += 2;	// Bertoxxulous' Gift AA 1
	}
	if(blevel >= 62 ) {
		bstr += 2;	// Advanced Innate Strength AA 2
		bsta += 2;	// Advanced Innate Stamina AA 2
		bagi += 2;	// Advanced Innate Agility AA 2
		bdex += 2;	// Advanced Innate Dexterity AA 2
		bint += 2;	// Advanced Innate Intelligence AA 2
		bwis += 2;	// Advanced Innate Wisdom AA 2
		bcha += 2;	// Advanced Innate Charisma AA 2
		bFR += 2;	// Warding of Solusek AA 2
		bCR += 2;	// Blessing of E'ci AA 2
		bMR += 2;	// Marr's Protection AA 2
		bPR += 2;	// Shroud of the Faceless AA 2
		bDR += 2;	// Bertoxxulous' Gift AA 2
	}
	if(blevel >= 63 ) {
		bstr += 2;	// Advanced Innate Strength AA 3
		bsta += 2;	// Advanced Innate Stamina AA 3
		bagi += 2;	// Advanced Innate Agility AA 3
		bdex += 2;	// Advanced Innate Dexterity AA 3
		bint += 2;	// Advanced Innate Intelligence AA 3
		bwis += 2;	// Advanced Innate Wisdom AA 3
		bcha += 2;	// Advanced Innate Charisma AA 3
		bFR += 2;	// Warding of Solusek AA 3
		bCR += 2;	// Blessing of E'ci AA 3
		bMR += 2;	// Marr's Protection AA 3
		bPR += 2;	// Shroud of the Faceless AA 3
		bDR += 2;	// Bertoxxulous' Gift AA 3
	}
	if(blevel >= 64 ) {
		bstr += 2;	// Advanced Innate Strength AA 4
		bsta += 2;	// Advanced Innate Stamina AA 4
		bagi += 2;	// Advanced Innate Agility AA 4
		bdex += 2;	// Advanced Innate Dexterity AA 4
		bint += 2;	// Advanced Innate Intelligence AA 4
		bwis += 2;	// Advanced Innate Wisdom AA 4
		bcha += 2;	// Advanced Innate Charisma AA 4
		bFR += 2;	// Warding of Solusek AA 4
		bCR += 2;	// Blessing of E'ci AA 4
		bMR += 2;	// Marr's Protection AA 4
		bPR += 2;	// Shroud of the Faceless AA 4
		bDR += 2;	// Bertoxxulous' Gift AA 4
	}
	if(blevel >= 65 ) {
		bstr += 2;	// Advanced Innate Strength AA 5
		bsta += 2;	// Advanced Innate Stamina AA 5
		bagi += 2;	// Advanced Innate Agility AA 5
		bdex += 2;	// Advanced Innate Dexterity AA 5
		bint += 2;	// Advanced Innate Intelligence AA 5
		bwis += 2;	// Advanced Innate Wisdom AA 5
		bcha += 2;	// Advanced Innate Charisma AA 5
		bFR += 2;	// Warding of Solusek AA 5
		bCR += 2;	// Blessing of E'ci AA 5
		bMR += 2;	// Marr's Protection AA 5
		bPR += 2;	// Shroud of the Faceless AA 5
		bDR += 2;	// Bertoxxulous' Gift AA 5
	}
	if(blevel >= 66 ) {
		bstr += 2;	// Advanced Innate Strength AA 6
		bsta += 2;	// Advanced Innate Stamina AA 6
		bagi += 2;	// Advanced Innate Agility AA 6
		bdex += 2;	// Advanced Innate Dexterity AA 6
		bint += 2;	// Advanced Innate Intelligence AA 6
		bwis += 2;	// Advanced Innate Wisdom AA 6
		bcha += 2;	// Advanced Innate Charisma AA 6
		bFR += 2;	// Warding of Solusek AA 6
		bCR += 2;	// Blessing of E'ci AA 6
		bMR += 2;	// Marr's Protection AA 6
		bPR += 2;	// Shroud of the Faceless AA 6
		bDR += 2;	// Bertoxxulous' Gift AA 6
	}
	if(blevel >= 67 ) {
		bstr += 2;	// Advanced Innate Strength AA 7
		bsta += 2;	// Advanced Innate Stamina AA 7
		bagi += 2;	// Advanced Innate Agility AA 7
		bdex += 2;	// Advanced Innate Dexterity AA 7
		bint += 2;	// Advanced Innate Intelligence AA 7
		bwis += 2;	// Advanced Innate Wisdom AA 7
		bcha += 2;	// Advanced Innate Charisma AA 7
		bFR += 2;	// Warding of Solusek AA 7
		bCR += 2;	// Blessing of E'ci AA 7
		bMR += 2;	// Marr's Protection AA 7
		bPR += 2;	// Shroud of the Faceless AA 7
		bDR += 2;	// Bertoxxulous' Gift AA 7
	}
	if(blevel >= 68 ) {
		bstr += 2;	// Advanced Innate Strength AA 8
		bsta += 2;	// Advanced Innate Stamina AA 8
		bagi += 2;	// Advanced Innate Agility AA 8
		bdex += 2;	// Advanced Innate Dexterity AA 8
		bint += 2;	// Advanced Innate Intelligence AA 8
		bwis += 2;	// Advanced Innate Wisdom AA 8
		bcha += 2;	// Advanced Innate Charisma AA 8
		bFR += 2;	// Warding of Solusek AA 8
		bCR += 2;	// Blessing of E'ci AA 8
		bMR += 2;	// Marr's Protection AA 8
		bPR += 2;	// Shroud of the Faceless AA 8
		bDR += 2;	// Bertoxxulous' Gift AA 8
	}
	if(blevel >= 69 ) {
		bstr += 2;	// Advanced Innate Strength AA 9
		bsta += 2;	// Advanced Innate Stamina AA 9
		bagi += 2;	// Advanced Innate Agility AA 9
		bdex += 2;	// Advanced Innate Dexterity AA 9
		bint += 2;	// Advanced Innate Intelligence AA 9
		bwis += 2;	// Advanced Innate Wisdom AA 9
		bcha += 2;	// Advanced Innate Charisma AA 9
		bFR += 2;	// Warding of Solusek AA 9
		bCR += 2;	// Blessing of E'ci AA 9
		bMR += 2;	// Marr's Protection AA 9
		bPR += 2;	// Shroud of the Faceless AA 9
		bDR += 2;	// Bertoxxulous' Gift AA 9
	}
	if(blevel >= 70 ) {
		bstr += 2;	// Advanced Innate Strength AA 10
		bsta += 2;	// Advanced Innate Stamina AA 10
		bagi += 2;	// Advanced Innate Agility AA 10
		bdex += 2;	// Advanced Innate Dexterity AA 10
		bint += 2;	// Advanced Innate Intelligence AA 10
		bwis += 2;	// Advanced Innate Wisdom AA 10
		bcha += 2;	// Advanced Innate Charisma AA 10
		bFR += 2;	// Warding of Solusek AA 10
		bCR += 2;	// Blessing of E'ci AA 10
		bMR += 2;	// Marr's Protection AA 10
		bPR += 2;	// Shroud of the Faceless AA 10
		bDR += 2;	// Bertoxxulous' Gift AA 10
	}


	// Base AC
	int bac = (blevel * 3) * 4;
	switch(bclass) {
		case WARRIOR:
		case SHADOWKNIGHT:
		case PALADIN: bac = bac*1.5;
	}

	// Calc Base Hit Points
	int16 lm = GetClassLevelFactor();
	int16 Post255;
	if((bsta-255)/2 > 0)
		Post255 = (bsta-255)/2;
	else
		Post255 = 0;
	sint32 bot_hp = (5)+(blevel*lm/10) + (((bsta-Post255)*blevel*lm/3000)) + ((Post255*blevel)*lm/6000);


	// Now, we need to calc the base mana.
	sint32 bot_mana = 0;
	sint32 WisInt = 0;
	sint32 MindLesserFactor, MindFactor;
	switch (GetCasterClass()) {
		case 'I':
			WisInt = bint;
			if((( WisInt - 199 ) / 2) > 0) {
				MindLesserFactor = ( WisInt - 199 ) / 2;
			}
			else {
				MindLesserFactor = 0;
			}
			MindFactor = WisInt - MindLesserFactor;
			if(WisInt > 100) {
				bot_mana = (((5 * (MindFactor + 20)) / 2) * 3 * blevel / 40);
			}
			else {
				bot_mana = (((5 * (MindFactor + 200)) / 2) * 3 * blevel / 100);
			}
			bot_mana += (itembonuses.Mana + spellbonuses.Mana);
			break;

		case 'W':
			WisInt = bwis;
			if((( WisInt - 199 ) / 2) > 0) {
				MindLesserFactor = ( WisInt - 199 ) / 2;
			}
			else {
				MindLesserFactor = 0;
			}
			MindFactor = WisInt - MindLesserFactor;
			if(WisInt > 100) {
				bot_mana = (((5 * (MindFactor + 20)) / 2) * 3 * blevel / 40);
			}
			else {
				bot_mana = (((5 * (MindFactor + 200)) / 2) * 3 * blevel / 100);
			}
			bot_mana += (itembonuses.Mana + spellbonuses.Mana);
			break;

		default:
			bot_mana = 0;
			break;
	}

	base_hp = bot_hp;
	max_mana = cur_mana = bot_mana;
	AC = bac;
	AGI = bagi;
	ATK = bATK;
	CHA = bcha;
	CR = bCR;
	DEX = bdex;
	DR = bDR;
	FR = bFR;
	INT = bint;
	MR = bMR;
	PR = bPR;
	STA = bsta;
	STR = bstr;
	WIS = bwis;

	// Special Attacks
	if(((bclass == MONK) || (bclass == WARRIOR) || (bclass == RANGER) || (bclass == BERSERKER))	&& (blevel >= 60)) {
		SpecAttacks[SPECATK_TRIPLE] = true;
	}

	if(showtext) {
		BotOwner->Message(15, "Base stats:");
		BotOwner->Message(15, "Level: %i HP: %i AC: %i Mana: %i STR: %i STA: %i DEX: %i AGI: %i INT: %i WIS: %i CHA: %i", blevel, base_hp, AC, max_mana, STR, STA, DEX, AGI, INT, WIS, CHA);
		BotOwner->Message(15, "Resists-- Magic: %i, Poison: %i, Fire: %i, Cold: %i, Disease: %i.",MR,PR,FR,CR,DR);
	}

	// Let's find the items in the bot inventory
	sint32 items_hp = 0;
	sint32 items_mana = 0;

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES* result;
	MYSQL_ROW row;
	bool ret = false;			

	/* Update to the DB with base stats*/
	if(database.RunQuery(query, MakeAnyLenString(&query, "update npc_types set level=%i, hp=%i, size=%f, npc_spells_id=%i, runspeed=%f, MR=%i, CR=%i, DR=%i, FR=%i, PR=%i, AC=%i, STR=%i, STA=%i, DEX=%i, AGI=%i, _INT=%i, WIS=%i, CHA=%i, ATK=%i where id=%i",blevel,base_hp,bsize,spellid,2.501f,MR,CR,DR,FR,PR,AC,STR,STA,DEX,AGI,INT,WIS,CHA,ATK,GetNPCTypeID()), errbuf)) {
		safe_delete_array(query);
	}
	else {
		Say("My database update failed!!!");
	}

	query = 0;
	memset(&itembonuses, 0, sizeof(StatBonuses));
	for(int i=0; i<22; i++) {
		if(database.RunQuery(query, MakeAnyLenString(&query, "SELECT itemid FROM botinventory WHERE npctypeid=%i AND botslotid=%i", GetNPCTypeID(), i), errbuf, &result)) {
			safe_delete_array(query);
			if(mysql_num_rows(result) == 1) {
				row = mysql_fetch_row(result);
				int iteminslot = atoi(row[0]);
				mysql_free_result(result);

				if(iteminslot > 0) {
					const Item_Struct *itemtmp = database.GetItem(iteminslot);
					if(itemtmp->AC != 0)
						itembonuses.AC += itemtmp->AC;
					if(itemtmp->HP != 0)
						itembonuses.HP += itemtmp->HP;
					if(itemtmp->Mana != 0)
						itembonuses.Mana += itemtmp->Mana;
					if(itemtmp->Endur != 0)
						itembonuses.Endurance += itemtmp->Endur;
					if(itemtmp->AStr != 0)
						itembonuses.STR += itemtmp->AStr;
					if(itemtmp->ASta != 0)
						itembonuses.STA += itemtmp->ASta;
					if(itemtmp->ADex != 0)
						itembonuses.DEX += itemtmp->ADex;
					if(itemtmp->AAgi != 0)
						itembonuses.AGI += itemtmp->AAgi;
					if(itemtmp->AInt != 0)
						itembonuses.INT += itemtmp->AInt;
					if(itemtmp->AWis != 0)
						itembonuses.WIS += itemtmp->AWis;
					if(itemtmp->ACha != 0)
						itembonuses.CHA += itemtmp->ACha;
					if(itemtmp->MR != 0)
						itembonuses.MR += itemtmp->MR;
					if(itemtmp->FR != 0)
						itembonuses.FR += itemtmp->FR;
					if(itemtmp->CR != 0)
						itembonuses.CR += itemtmp->CR;
					if(itemtmp->PR != 0)
						itembonuses.PR += itemtmp->PR;
					if(itemtmp->DR != 0)
						itembonuses.DR += itemtmp->DR;
					if(itemtmp->Regen != 0)
						itembonuses.HPRegen += itemtmp->Regen;
					if(itemtmp->ManaRegen != 0)
						itembonuses.ManaRegen += itemtmp->ManaRegen;
					if(itemtmp->Attack != 0)
						itembonuses.ATK += itemtmp->Attack;
					if(itemtmp->DamageShield != 0)
						itembonuses.DamageShield += itemtmp->DamageShield;
					if(itemtmp->SpellShield != 0)
						itembonuses.SpellDamageShield += itemtmp->SpellShield;
					if(itemtmp->Shielding != 0)
						itembonuses.MeleeMitigation += itemtmp->Shielding;
					if(itemtmp->StunResist != 0)
						itembonuses.StunResist += itemtmp->StunResist;
					if(itemtmp->StrikeThrough != 0)
						itembonuses.StrikeThrough += itemtmp->StrikeThrough;
					if(itemtmp->Avoidance != 0)
						itembonuses.AvoidMeleeChance += itemtmp->Avoidance;
					if(itemtmp->Accuracy != 0)
						itembonuses.HitChance += itemtmp->Accuracy;
					if(itemtmp->CombatEffects != 0)
						itembonuses.ProcChance += itemtmp->CombatEffects;
					if ((itemtmp->Worn.Effect != 0) && (itemtmp->Worn.Type == ET_WornEffect)) { // latent effects
						ApplySpellsBonuses(itemtmp->Worn.Effect, itemtmp->Worn.Level, &itembonuses);
					}
				}
			}
		}
	}

	bMR += itembonuses.MR;
	bCR += itembonuses.CR;
	bDR += itembonuses.DR;
	bFR += itembonuses.FR;
	bPR += itembonuses.PR;
	bac += itembonuses.AC;
	bstr += itembonuses.STR;
	bsta += itembonuses.STA;
	bdex += itembonuses.DEX;
	bagi += itembonuses.AGI;
	bint += itembonuses.INT;
	bwis += itembonuses.WIS;
	bcha += itembonuses.CHA;
	bATK += itembonuses.ATK;

	bMR += spellbonuses.MR;
	bCR += spellbonuses.CR;
	bDR += spellbonuses.DR;
	bFR += spellbonuses.FR;
	bPR += spellbonuses.PR;
	bac += spellbonuses.AC;
	bstr += spellbonuses.STR;
	bsta += spellbonuses.STA;
	bdex += spellbonuses.DEX;
	bagi += spellbonuses.AGI;
	bint += spellbonuses.INT;
	bwis += spellbonuses.WIS;
	bcha += spellbonuses.CHA;
	bATK += spellbonuses.ATK;

	if((bsta-255)/2 > 0)
		Post255 = (bsta-255)/2;
	else
		Post255 = 0;
	bot_hp = (5)+(blevel*lm/10) + (((bsta-Post255)*blevel*lm/3000));
	bot_hp += itembonuses.HP;

	// Hitpoint AA's
	int32 nd = 10000;
	if(blevel >= 69) {
		nd += 1650;	// Planar Durablility AA 3
		if(bclass == WARRIOR) { // Sturdiness AA 5
			nd += 500;
		}
	}
	else if(blevel >= 68) {
		nd += 1650;	// Planar Durablility AA 3
		if(bclass == WARRIOR) { // Sturdiness AA 4
			nd += 400;
		}
	}
	else if(blevel >= 67) {
		nd += 1650;	// Planar Durablility AA 3
		if(bclass == WARRIOR) { // Sturdiness AA 3
			nd += 300;
		}
	}
	else if(blevel >= 66) {
		nd += 1650;	// Planar Durablility AA 3
		if(bclass == WARRIOR) { // Sturdiness AA 2
			nd += 200;
		}
	}
	else if(blevel >= 65) {
		nd += 1650;	// Planar Durablility AA 3
		if(bclass == WARRIOR) { // Sturdiness AA 1
			nd += 100;
		}
	}
	else if(blevel >= 63) {
		nd += 1500;	// Planar Durablility AA 2
	}
	else if(blevel >= 61) {
		nd += 1350;	// Planar Durablility AA 1
	}
	else if(blevel >= 59) {
		nd += 1200;	// Physical Enhancememt AA 1
	}
	else if(blevel >= 57) {
		nd += 1000;	// Natural Durablility AA 3
	}
	else if(blevel >= 56) {
		nd += 500;	// Natural Durablility AA 2
	}
	else if(blevel >= 55) {
		nd += 200;	// Natural Durablility AA 1
	}
	bot_hp = bot_hp * nd / 10000;
	bot_hp += spellbonuses.HP;
	max_hp = cur_hp = bot_hp;

	switch (GetCasterClass()) {
		case 'I':
			WisInt = bint;
			if((( WisInt - 199 ) / 2) > 0) {
				MindLesserFactor = ( WisInt - 199 ) / 2;
			}
			else {
				MindLesserFactor = 0;
			}
			MindFactor = WisInt - MindLesserFactor;
			if(WisInt > 100) {
				bot_mana = (((5 * (MindFactor + 20)) / 2) * 3 * blevel / 40);
			}
			else {
				bot_mana = (((5 * (MindFactor + 200)) / 2) * 3 * blevel / 100);
			}
			bot_mana += (itembonuses.Mana + spellbonuses.Mana);
			break;

		case 'W':
			WisInt = bwis;
			if((( WisInt - 199 ) / 2) > 0) {
				MindLesserFactor = ( WisInt - 199 ) / 2;
			}
			else {
				MindLesserFactor = 0;
			}
			MindFactor = WisInt - MindLesserFactor;
			if(WisInt > 100) {
				bot_mana = (((5 * (MindFactor + 20)) / 2) * 3 * blevel / 40);
			}
			else {
				bot_mana = (((5 * (MindFactor + 200)) / 2) * 3 * blevel / 100);
			}
			bot_mana += (itembonuses.Mana + spellbonuses.Mana);
			break;

		default:
			bot_mana = 0;
			break;
	}
	max_mana = cur_mana = bot_mana;
	CastToNPC()->AI_AddNPCSpells(spellid);

	if(showtext) {
		BotOwner->Message(15, "I'm updated.");
		BotOwner->Message(15, "Level: %i HP: %i AC: %i Mana: %i STR: %i STA: %i DEX: %i AGI: %i INT: %i WIS: %i CHA: %i", blevel, max_hp, bac, max_mana, bstr, bsta, bdex, bagi, bint, bwis, bcha);
		BotOwner->Message(15, "Resists-- Magic: %i, Poison: %i, Fire: %i, Cold: %i, Disease: %i.",bMR,bPR,bFR,bCR,bDR);
	}
}

void EntityList::ShowSpawnWindow(Client* client, int Distance, bool NamedOnly) {

	const char *WindowTitle = "Bot Tracking Window";

	string WindowText;
	int LastCon = -1;
	int CurrentCon = 0;
	
	int32 array_counter = 0;
	
	LinkedListIterator<Mob*> iterator(mob_list);
	iterator.Reset();

	while(iterator.MoreElements())
	{
		if (iterator.GetData() && (iterator.GetData()->DistNoZ(*client)<=Distance))
		{
			if(iterator.GetData()->IsTrackable()) {
				Mob* cur_entity = iterator.GetData();
				int  Extras = (cur_entity->IsBot() || cur_entity->IsPet() || cur_entity->IsFamiliar() || cur_entity->IsClient());
				const char *const MyArray[] = {
					"a_","an_","Innkeep_","Barkeep_",
					"Guard_","Merchant_","Lieutenant_",
					"Banker_","Centaur_","Aviak_","Baker_",
					"Sir_","Armorer_","Deathfist_","Deputy_",
					"Sentry_","Sentinel_","Leatherfoot_",
					"Corporal_","goblin_","Bouncer_","Captain_",
					"orc_","fire_","inferno_","young_","cinder_",
					"flame_","gnomish_","CWG_","sonic_","greater_",
					"ice_","dry_","Priest_","dark-boned_",
					"Tentacle_","Basher_","Dar_","Greenblood_",
					"clockwork_","guide_","rogue_","minotaur_",
					"brownie_","Teir'","dark_","tormented_",
					"mortuary_","lesser_","giant_","infected_",
					"wharf_","Apprentice_","Scout_","Recruit_",
					"Spiritist_","Pit_","Royal_","scalebone_",
					"carrion_","Crusader_","Trooper_","hunter_",
					"decaying_","iksar_","klok_","templar_","lord_",
					"froglok_","war_","large_","charbone_","icebone_",
					"Vicar_","Cavalier_","Heretic_","Reaver_","venomous_",
					"Sheildbearer_","pond_","mountain_","plaguebone_","Brother_",
					"great_","strathbone_","briarweb_","strathbone_","skeletal_",
					"minion_","spectral_","myconid_","spurbone_","sabretooth_",
					"Tin_","Iron_","Erollisi_","Petrifier_","Burynai_",
					"undead_","decayed_","You_","smoldering_","gyrating_",
					"lumpy_","Marshal_","Sheriff_","Chief_","Risen_",
					"lascar_","tribal_","fungi_","Xi_","Legionnaire_",
					"Centurion_","Zun_","Diabo_","Scribe_","Defender_","Capt_",
					"blazing_","Solusek_","imp_","hexbone_","elementalbone_",
					"stone_","lava_","_",""
				};
				unsigned int MyArraySize;
				 for ( MyArraySize = 0; true; MyArraySize++) {   //Find empty string & get size
				   if (!(*(MyArray[MyArraySize]))) break;   //Checks for null char in 1st pos
				};
				if (NamedOnly) {
				   bool ContinueFlag = false;
				   const char *CurEntityName = cur_entity->GetName();  //Call function once
				   for (int Index = 0; Index < MyArraySize; Index++) {
				      if (!strncasecmp(CurEntityName, MyArray[Index], strlen(MyArray[Index])) || (Extras)) {
				         iterator.Advance();
				         ContinueFlag = true;
				         break;   //From Index for
				       };
				   };
				  if (ContinueFlag) continue; //Moved here or would apply to Index for
				};

				CurrentCon = client->GetLevelCon(cur_entity->GetLevel());
				if(CurrentCon != LastCon) {

					if(LastCon != -1)
						WindowText += "</c>";

					LastCon = CurrentCon;

					switch(CurrentCon) {

						case CON_GREEN: {
							WindowText += "<c \"#00FF00\">";
							break;
						}

						case CON_LIGHTBLUE: {
							WindowText += "<c \"#8080FF\">";
							break;
						}
						case CON_BLUE: {
							WindowText += "<c \"#2020FF\">";
							break;
						}

						case CON_YELLOW: {
							WindowText += "<c \"#FFFF00\">";
							break;
						}
						case CON_RED: {
							WindowText += "<c \"#FF0000\">";
							break;
						}
						default: {
							WindowText += "<c \"#FFFFFF\">";
							break;
						}
					}
				}

				WindowText += cur_entity->GetCleanName();
				WindowText += "<br>";

				if(strlen(WindowText.c_str()) > 4000) {
					// Popup window is limited to 4096 characters.
					WindowText += "</c><br><br>List truncated ... too many mobs to display";
					break;
				}
			}
		}

		iterator.Advance();
	}
	WindowText += "</c>";

	client->SendPopupToClient(WindowTitle, WindowText.c_str());

	return; 
}

#endif