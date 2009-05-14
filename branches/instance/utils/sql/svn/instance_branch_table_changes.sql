CREATE TABLE `instance_lockout` (
  `id` int(11) NOT NULL auto_increment,
  `zone` int(11) unsigned NOT NULL default '0',
  `version` tinyint(4) unsigned NOT NULL default '0',
  `start_time` int(11) unsigned NOT NULL default '0',
  `duration` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `id` (`id`),
  KEY `id_2` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `instance_lockout_player` (
  `id` int(11) unsigned NOT NULL default '0',
  `charid` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`charid`,`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

ALTER TABLE `respawn_times` ADD `instance_id` SMALLINT DEFAULT '0' NOT NULL AFTER `duration`;
ALTER TABLE `respawn_times` DROP PRIMARY KEY, ADD PRIMARY KEY (`id`, `instance_id`);

ALTER TABLE `character_` ADD `instanceid` SMALLINT UNSIGNED DEFAULT '0' NOT NULL AFTER `zoneid`;
ALTER TABLE `character_` DROP `instZflagNum`;
ALTER TABLE `character_` DROP `instZOrgID`;

ALTER TABLE `spawn2` ADD `version` SMALLINT UNSIGNED DEFAULT '0' NOT NULL AFTER `zone`;

ALTER TABLE `player_corpses` ADD `instanceid` SMALLINT UNSIGNED DEFAULT '0' NOT NULL AFTER `zoneid`;
ALTER TABLE `player_corpses` ADD INDEX `instanceid` (`instanceid`);

ALTER TABLE `traps` ADD INDEX `zone` (`zone`);
ALTER TABLE `traps` ADD `version` SMALLINT UNSIGNED DEFAULT '0' NOT NULL AFTER `zone`;

ALTER TABLE `ground_spawns` ADD INDEX `zone` (`zoneid`);
ALTER TABLE `ground_spawns` ADD `version` SMALLINT UNSIGNED DEFAULT '0' NOT NULL AFTER `zoneid`;

ALTER TABLE `object` ADD INDEX `zone` (`zoneid`);
ALTER TABLE `object` ADD `version` SMALLINT UNSIGNED DEFAULT '0' NOT NULL AFTER `zoneid`;
ALTER TABLE `object` DROP `linked_list_addr_01`;
ALTER TABLE `object` DROP `linked_list_addr_02`;
ALTER TABLE `object` DROP `unknown88`;
DELETE FROM object WHERE object.type=1 AND object.itemid!=0;

ALTER TABLE `doors` ADD `version` SMALLINT UNSIGNED DEFAULT '0' NOT NULL AFTER `zone`;