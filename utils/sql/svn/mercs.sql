DROP TABLE IF EXISTS `mercs`;

CREATE TABLE IF NOT EXISTS `mercs` (
  `MercId` int(11) NOT NULL AUTO_INCREMENT,
  `MercOwnerCharacterId` int(11) NOT NULL DEFAULT '0',
  `Name` varchar(64) NOT NULL DEFAULT '',
  `Class` tinyint(4) NOT NULL DEFAULT '0',
  `Level` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `Profile` blob,
  `Extprofile` blob,
  `BotCreateDate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `LastSpawnDate` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `TotalPlayTime` int(10) unsigned NOT NULL DEFAULT '0',
  `LastZoneId` smallint(6) NOT NULL DEFAULT '0',
  PRIMARY KEY (`MercId`),
  UNIQUE KEY `Name` (`Name`)
) ENGINE=InnoDB;


