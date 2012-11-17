SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `merc_npc_types`
-- ----------------------------
DROP TABLE IF EXISTS `merc_npc_types`;
CREATE TABLE `merc_npc_types` (
  `id` int(11) NOT NULL,
  `clientlevel` int(11) NOT NULL DEFAULT '1',
  `name` text NOT NULL,
  `lastname` varchar(32) DEFAULT NULL,
  `level` tinyint(2) unsigned NOT NULL DEFAULT '0',
  `race` smallint(5) unsigned NOT NULL DEFAULT '0',
  `class` tinyint(2) unsigned NOT NULL DEFAULT '0',
  `bodytype` int(11) DEFAULT NULL,
  `hp` int(11) NOT NULL DEFAULT '0',
  `mana` int(11) NOT NULL DEFAULT '0',
  `gender` tinyint(2) unsigned NOT NULL DEFAULT '0',
  `texture` tinyint(2) unsigned NOT NULL DEFAULT '0',
  `helmtexture` tinyint(2) unsigned NOT NULL DEFAULT '0',
  `size` float NOT NULL DEFAULT '0',
  `hp_regen_rate` int(11) unsigned NOT NULL DEFAULT '0',
  `mana_regen_rate` int(11) unsigned NOT NULL DEFAULT '0',
  `loottable_id` int(11) unsigned NOT NULL DEFAULT '0',
  `merchant_id` int(11) unsigned NOT NULL DEFAULT '0',
  `alt_currency_id` int(11) unsigned NOT NULL DEFAULT '0',
  `npc_spells_id` int(11) unsigned NOT NULL DEFAULT '0',
  `npc_faction_id` int(11) NOT NULL DEFAULT '0',
  `adventure_template_id` int(10) unsigned NOT NULL DEFAULT '0',
  `trap_template` int(10) unsigned DEFAULT '0',
  `mindmg` int(10) unsigned NOT NULL DEFAULT '0',
  `maxdmg` int(10) unsigned NOT NULL DEFAULT '0',
  `attack_count` smallint(6) NOT NULL DEFAULT '-1',
  `npcspecialattks` varchar(36) NOT NULL DEFAULT '',
  `aggroradius` int(10) unsigned NOT NULL DEFAULT '0',
  `face` int(10) unsigned NOT NULL DEFAULT '1',
  `luclin_hairstyle` int(10) unsigned NOT NULL DEFAULT '1',
  `luclin_haircolor` int(10) unsigned NOT NULL DEFAULT '1',
  `luclin_eyecolor` int(10) unsigned NOT NULL DEFAULT '1',
  `luclin_eyecolor2` int(10) unsigned NOT NULL DEFAULT '1',
  `luclin_beardcolor` int(10) unsigned NOT NULL DEFAULT '1',
  `luclin_beard` int(10) unsigned NOT NULL DEFAULT '0',
  `drakkin_heritage` int(10) NOT NULL DEFAULT '0',
  `drakkin_tattoo` int(10) NOT NULL DEFAULT '0',
  `drakkin_details` int(10) NOT NULL DEFAULT '0',
  `armortint_id` int(10) unsigned NOT NULL DEFAULT '0',
  `armortint_red` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `armortint_green` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `armortint_blue` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `d_meele_texture1` int(10) unsigned NOT NULL DEFAULT '0',
  `d_meele_texture2` int(10) unsigned NOT NULL DEFAULT '0',
  `prim_melee_type` tinyint(4) unsigned NOT NULL DEFAULT '28',
  `sec_melee_type` tinyint(4) unsigned NOT NULL DEFAULT '28',
  `runspeed` float NOT NULL DEFAULT '0',
  `MR` smallint(5) NOT NULL DEFAULT '0',
  `CR` smallint(5) NOT NULL DEFAULT '0',
  `DR` smallint(5) NOT NULL DEFAULT '0',
  `FR` smallint(5) NOT NULL DEFAULT '0',
  `PR` smallint(5) NOT NULL DEFAULT '0',
  `Corrup` smallint(5) NOT NULL DEFAULT '0',
  `see_invis` smallint(4) NOT NULL DEFAULT '0',
  `see_invis_undead` smallint(4) NOT NULL DEFAULT '0',
  `qglobal` int(2) unsigned NOT NULL DEFAULT '0',
  `AC` smallint(5) NOT NULL DEFAULT '0',
  `npc_aggro` tinyint(4) NOT NULL DEFAULT '0',
  `spawn_limit` tinyint(4) NOT NULL DEFAULT '0',
  `attack_speed` float NOT NULL DEFAULT '0',
  `findable` tinyint(4) NOT NULL DEFAULT '0',
  `STR` mediumint(8) unsigned NOT NULL DEFAULT '75',
  `STA` mediumint(8) unsigned NOT NULL DEFAULT '75',
  `DEX` mediumint(8) unsigned NOT NULL DEFAULT '75',
  `AGI` mediumint(8) unsigned NOT NULL DEFAULT '75',
  `_INT` mediumint(8) unsigned NOT NULL DEFAULT '80',
  `WIS` mediumint(8) unsigned NOT NULL DEFAULT '75',
  `CHA` mediumint(8) unsigned NOT NULL DEFAULT '75',
  `see_hide` tinyint(4) NOT NULL DEFAULT '0',
  `see_improved_hide` tinyint(4) NOT NULL DEFAULT '0',
  `trackable` tinyint(4) NOT NULL DEFAULT '1',
  `isbot` tinyint(4) NOT NULL DEFAULT '0',
  `exclude` tinyint(4) NOT NULL DEFAULT '1',
  `ATK` mediumint(9) NOT NULL DEFAULT '0',
  `Accuracy` mediumint(9) NOT NULL DEFAULT '0',
  `slow_mitigation` float unsigned NOT NULL DEFAULT '0',
  `version` smallint(5) unsigned NOT NULL DEFAULT '0',
  `maxlevel` tinyint(3) NOT NULL DEFAULT '0',
  `scalerate` int(11) NOT NULL DEFAULT '100',
  `private_corpse` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `unique_spawn_by_name` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `underwater` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `isquest` tinyint(3) NOT NULL DEFAULT '0',
  `emoteid` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`clientlevel`,`id`)
) ENGINE=MyISAM AUTO_INCREMENT=999214 DEFAULT CHARSET=latin1 PACK_KEYS=0;

-- ----------------------------
-- Records of merc_npc_types
-- ----------------------------
INSERT INTO `merc_npc_types` VALUES ('644', '1', 'SwarmPetPaladin', '', '40', '127', '3', '63', '400', '0', '0', '0', '0', '6', '60', '0', '0', '0', '0', '8', '0', '0', '0', '32', '70', '-1', 'fL', '70', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '160', '0', '28', '28', '1.5', '48', '48', '48', '48', '48', '75', '0', '0', '0', '324', '0', '0', '-15', '0', '250', '250', '250', '250', '250', '250', '250', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '100', '0', '0', '0', '0', '0');

-- ----------------------------
-- Table structure for `merc_types`
-- ----------------------------
DROP TABLE IF EXISTS `merc_types`;
CREATE TABLE `merc_types` (
  `merc_type_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `merc_npc_type_id` int(11) NOT NULL,
  `race_id` int(11) NOT NULL DEFAULT '1',
  `proficiency_id` tinyint(3) unsigned NOT NULL,
  `dbstring` varchar(12) NOT NULL,
  PRIMARY KEY (`merc_type_id`)
) ENGINE=InnoDB AUTO_INCREMENT=49 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of merc_types
-- ----------------------------
INSERT INTO `merc_types` VALUES ('1', '644', '1', '1', '1000100');
INSERT INTO `merc_types` VALUES ('2', '644', '1', '2', '1000200');
INSERT INTO `merc_types` VALUES ('3', '644', '2', '1', '2000100');
INSERT INTO `merc_types` VALUES ('4', '644', '2', '2', '2000200');
INSERT INTO `merc_types` VALUES ('5', '644', '3', '1', '3000100');
INSERT INTO `merc_types` VALUES ('6', '644', '3', '2', '3000200');
INSERT INTO `merc_types` VALUES ('7', '644', '4', '1', '4000100');
INSERT INTO `merc_types` VALUES ('8', '644', '4', '2', '4000200');
INSERT INTO `merc_types` VALUES ('9', '644', '5', '1', '5000100');
INSERT INTO `merc_types` VALUES ('10', '644', '5', '2', '5000200');
INSERT INTO `merc_types` VALUES ('11', '644', '6', '1', '6000100');
INSERT INTO `merc_types` VALUES ('12', '644', '6', '2', '6000200');
INSERT INTO `merc_types` VALUES ('13', '644', '7', '1', '7000100');
INSERT INTO `merc_types` VALUES ('14', '644', '7', '2', '7000200');
INSERT INTO `merc_types` VALUES ('15', '644', '8', '1', '8000100');
INSERT INTO `merc_types` VALUES ('16', '644', '8', '2', '8000200');
INSERT INTO `merc_types` VALUES ('17', '644', '9', '1', '9000100');
INSERT INTO `merc_types` VALUES ('18', '644', '9', '2', '9000200');
INSERT INTO `merc_types` VALUES ('19', '644', '10', '1', '10000100');
INSERT INTO `merc_types` VALUES ('20', '644', '10', '2', '10000200');
INSERT INTO `merc_types` VALUES ('21', '644', '11', '1', '11000100');
INSERT INTO `merc_types` VALUES ('22', '644', '11', '2', '11000200');
INSERT INTO `merc_types` VALUES ('23', '644', '12', '1', '12000100');
INSERT INTO `merc_types` VALUES ('24', '644', '12', '2', '12000200');
INSERT INTO `merc_types` VALUES ('25', '644', '128', '1', '128000100');
INSERT INTO `merc_types` VALUES ('26', '644', '128', '2', '128000200');
INSERT INTO `merc_types` VALUES ('27', '644', '130', '1', '130000100');
INSERT INTO `merc_types` VALUES ('28', '644', '130', '2', '130000200');
INSERT INTO `merc_types` VALUES ('29', '644', '330', '1', '330000100');
INSERT INTO `merc_types` VALUES ('30', '644', '330', '2', '330000200');
INSERT INTO `merc_types` VALUES ('31', '644', '522', '1', '522000100');
INSERT INTO `merc_types` VALUES ('32', '644', '522', '2', '522000200');
INSERT INTO `merc_types` VALUES ('33', '644', '26', '1', '26000100');
INSERT INTO `merc_types` VALUES ('34', '644', '26', '2', '26000200');
INSERT INTO `merc_types` VALUES ('35', '644', '48', '1', '48000100');
INSERT INTO `merc_types` VALUES ('36', '644', '48', '2', '48000200');
INSERT INTO `merc_types` VALUES ('37', '644', '51', '1', '51000100');
INSERT INTO `merc_types` VALUES ('38', '644', '51', '2', '51000200');
INSERT INTO `merc_types` VALUES ('39', '644', '137', '1', '137000100');
INSERT INTO `merc_types` VALUES ('40', '644', '137', '2', '137000200');
INSERT INTO `merc_types` VALUES ('41', '644', '433', '1', '433000100');
INSERT INTO `merc_types` VALUES ('42', '644', '433', '2', '433000200');
INSERT INTO `merc_types` VALUES ('43', '644', '456', '1', '456000100');
INSERT INTO `merc_types` VALUES ('44', '644', '456', '2', '456000200');
INSERT INTO `merc_types` VALUES ('45', '644', '458', '1', '458000100');
INSERT INTO `merc_types` VALUES ('46', '644', '458', '2', '458000200');
INSERT INTO `merc_types` VALUES ('47', '644', '568', '1', '568000100');
INSERT INTO `merc_types` VALUES ('48', '644', '568', '2', '568000200');
