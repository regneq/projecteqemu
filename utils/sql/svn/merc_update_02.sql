ALTER TABLE `group_id` ADD COLUMN `ismerc`  tinyint(3) NOT NULL DEFAULT 0 AFTER `name`, DROP PRIMARY KEY, ADD PRIMARY KEY (`groupid`, `charid`, `ismerc`);
ALTER TABLE `merc_types` ADD COLUMN `merc_npc_type_id`  int(11) NOT NULL AFTER `merc_type_id`;

