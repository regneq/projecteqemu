DROP TABLE IF EXISTS botgroups;
DROP TABLE IF EXISTS botinventory;
DROP TABLE IF EXISTS bots;

CREATE TABLE  bots (
  `BotID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `BotOwnerCharacterID` int(10) unsigned NOT NULL,
  `BotSpellsID` int(10) unsigned NOT NULL DEFAULT '0',
  `Name` varchar(64) NOT NULL,
  `LastName` varchar(32) DEFAULT NULL,
  `BotLevel` tinyint(2) unsigned NOT NULL DEFAULT '0',
  `Race` smallint(5) NOT NULL DEFAULT '0',
  `Class` tinyint(2) NOT NULL DEFAULT '0',
  `Gender` tinyint(2) NOT NULL DEFAULT '0',
  `Size` float NOT NULL DEFAULT '0',
  `Face` int(10) NOT NULL DEFAULT '1',
  `LuclinHairStyle` int(10) NOT NULL DEFAULT '1',
  `LuclinHairColor` int(10) NOT NULL DEFAULT '1',
  `LuclinEyeColor` int(10) NOT NULL DEFAULT '1',
  `LuclinEyeColor2` int(10) NOT NULL DEFAULT '1',
  `LuclinBeardColor` int(10) NOT NULL DEFAULT '1',
  `LuclinBeard` int(10) NOT NULL DEFAULT '0',
  `DrakkinHeritage` int(10) NOT NULL DEFAULT '0',
  `DrakkinTattoo` int(10) NOT NULL DEFAULT '0',
  `DrakkinDetails` int(10) NOT NULL DEFAULT '0',
  `MR` smallint(5) NOT NULL DEFAULT '0',
  `CR` smallint(5) NOT NULL DEFAULT '0',
  `DR` smallint(5) NOT NULL DEFAULT '0',
  `FR` smallint(5) NOT NULL DEFAULT '0',
  `PR` smallint(5) NOT NULL DEFAULT '0',
  `AC` smallint(5) NOT NULL DEFAULT '0',
  `STR` mediumint(8) NOT NULL DEFAULT '75',
  `STA` mediumint(8) NOT NULL DEFAULT '75',
  `DEX` mediumint(8) NOT NULL DEFAULT '75',
  `AGI` mediumint(8) NOT NULL DEFAULT '75',
  `_INT` mediumint(8) NOT NULL DEFAULT '80',
  `WIS` mediumint(8) NOT NULL DEFAULT '75',
  `CHA` mediumint(8) NOT NULL DEFAULT '75',
  `ATK` mediumint(9) NOT NULL DEFAULT '0',
  `BotCreateDate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `LastSpawnDate` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `TotalPlayTime` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`BotID`)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS botinventory (
  BotInventoryID integer unsigned NOT NULL auto_increment,
  BotID integer unsigned NOT NULL DEFAULT '0',
  SlotID integer signed NOT NULL DEFAULT '0',
  ItemID integer unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (BotInventoryID),
  KEY FK_botinventory_1 (BotID),
  CONSTRAINT FK_botinventory_1 FOREIGN KEY (BotID) REFERENCES bots (BotID)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS botgroups (
  GroupID integer unsigned NOT NULL default 0,
  CharID integer unsigned NOT NULL default 0,
  BotID integer unsigned NOT NULL default 0,
  Slot integer unsigned NOT NULL default 0,
  PRIMARY KEY (BotID),
  CONSTRAINT FK_botgroups_1 FOREIGN KEY (BotID) REFERENCES bots (BotID)
) ENGINE=InnoDB;

delete from rule_values where rule_name like 'Bots%' and ruleset_id = 1;

INSERT INTO rule_values VALUES ('1', 'Bots:BotManaRegen', '3.0', 'Adjust mana regen for bots, 1 is fast and higher numbers slow it down 3 is about the same as players.');
INSERT INTO rule_values VALUES ('1', 'Bots:BotFinishBuffing', 'false', 'Allow for buffs to complete even if the bot caster is out of mana.  Only affects buffing out of combat.');
INSERT INTO rule_values VALUES ('1', 'Bots:CreateBotCount', '150', 'Number of bots that each account can create');
INSERT INTO rule_values VALUES ('1', 'Bots:SpawnBotCount', '71', 'Number of bots a character can have spawned at one time, You + 71 bots is a 12 group raid');
INSERT INTO rule_values VALUES ('1', 'Bots:BotQuest', 'false', 'Optional quest method to manage bot spawn limits using the quest_globals name bot_spawn_limit, see: /bazaar/Aediles_Thrall.pl');
INSERT INTO rule_values VALUES ('1', 'Bots:BotGroupBuffing', 'false', 'Bots will cast single target buffs as group buffs, default is false for single. Does not make single target buffs work for MGB.');
INSERT INTO rule_values VALUES ('1', 'Bots:BotSpellQuest', 'false', 'Anita Thrall\'s (Anita_Thrall.pl) Bot Spell Scriber quests.');

DELIMITER $$

DROP FUNCTION IF EXISTS `GetMobType` $$
CREATE FUNCTION `GetMobType` (mobname VARCHAR(64)) RETURNS CHAR(1)
BEGIN
    DECLARE Result CHAR(1);

    SET Result = NULL;

    IF (select count(*) from character_ where name = mobname) > 0 THEN
      SET Result = 'C';
    ELSEIF (select count(*) from bots where Name = mobname) > 0 THEN
      SET Result = 'B';
    END IF;

    RETURN Result;
END $$

DELIMITER ;

DROP VIEW IF EXISTS `vwGroups`;
CREATE VIEW `vwGroups` AS
  select g.groupid as groupid,
GetMobType(g.name) as mobtype,
g.name as name,
g.charid as mobid,
ifnull(c.level, b.BotLevel) as level
from group_id as g
left join character_ as c on g.name = c.name
left join bots as b on g.name = b.Name;

DROP TABLE IF EXISTS `botactives`;
CREATE TABLE `botactives` (
  `ActiveBotId` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`ActiveBotId`),
  KEY `FK_botactives_1` (`ActiveBotId`),
  CONSTRAINT `FK_botactives_1` FOREIGN KEY (`ActiveBotId`) REFERENCES `bots` (`BotID`)
) ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=latin1;