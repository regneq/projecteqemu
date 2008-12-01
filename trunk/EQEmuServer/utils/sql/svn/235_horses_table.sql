DROP TABLE IF EXISTS `horses`;
CREATE TABLE `horses` (
  `id` int(11) NOT NULL auto_increment,
  `filename` varchar(32) NOT NULL,
  `race` smallint(3) NOT NULL default '216',
  `gender` tinyint(1) NOT NULL default '0',
  `texture` tinyint(2) NOT NULL default '0',
  `mountspeed` float(4,2) NOT NULL default '0.75',
  `notes` varchar(64) default 'Notes',
  PRIMARY KEY  (`id`,`filename`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;


INSERT INTO `horses` VALUES ('1', 'SumChimeraFast', '216', '0', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('2', 'SumCragslither1Fast', '436', '2', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('3', 'SumCragslither2Fast', '436', '2', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('4', 'SumCragslither3Fast', '436', '2', '1', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('5', 'SumHorseBlFast', '216', '0', '2', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('6', 'SumHorseBlRun1', '216', '0', '2', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('7', 'SumHorseBlRun2', '216', '0', '2', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('8', 'SumHorseBlSlow1', '216', '0', '2', '0.75', 'Notes');
INSERT INTO `horses` VALUES ('9', 'SumHorseBlSlow2', '216', '0', '0', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('10', 'SumHorseBrFast', '216', '0', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('11', 'SumHorseBrRun1', '216', '0', '0', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('12', 'SumHorseBrRun2', '216', '0', '0', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('13', 'SumHorseBrSlow1', '216', '0', '0', '0.75', 'Notes');
INSERT INTO `horses` VALUES ('14', 'SumHorseBrSlow2', '216', '0', '0', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('15', 'SumHorseTaFast', '216', '0', '3', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('16', 'SumHorseTaRun1', '216', '0', '3', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('17', 'SumHorseTaRun2', '216', '0', '3', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('18', 'SumHorseTaSlow1', '216', '0', '3', '0.75', 'Notes');
INSERT INTO `horses` VALUES ('19', 'SumHorseTaSlow2', '216', '0', '3', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('20', 'SumHorseWhFast', '216', '0', '1', '1.75', '2871 - Summon Horse SumHorseWhFast');
INSERT INTO `horses` VALUES ('21', 'SumHorseWhRun1', '216', '0', '1', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('22', 'SumHorseWhRun2', '216', '0', '1', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('23', 'SumHorseWhSlow1', '216', '0', '1', '0.75', 'Notes');
INSERT INTO `horses` VALUES ('24', 'SumHorseWhSlow2', '216', '0', '1', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('25', 'SumKirin0Fast', '356', '2', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('26', 'SumKirin2Fast', '356', '2', '1', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('27', 'SumLizardBlkFast', '216', '1', '1', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('28', 'SumLizardBlkRun1', '216', '1', '1', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('29', 'SumLizardBlkRun2', '216', '1', '1', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('30', 'SumLizardBlkSlow1', '216', '1', '1', '0.75', 'Notes');
INSERT INTO `horses` VALUES ('31', 'SumLizardBlkSlow2', '216', '1', '1', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('32', 'SumLizardGrnFast', '216', '1', '2', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('33', 'SumLizardGrnRun1', '216', '1', '2', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('34', 'SumLizardGrnRun2', '216', '1', '2', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('35', 'SumLizardGrnSlow1', '216', '1', '2', '0.75', 'Notes');
INSERT INTO `horses` VALUES ('36', 'SumLizardGrnSlow2', '216', '1', '2', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('37', 'SumLizardRedFast', '216', '1', '3', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('38', 'SumLizardRedRun1', '216', '1', '3', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('39', 'SumLizardRedRun2', '216', '1', '3', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('40', 'SumLizardRedSlow1', '216', '1', '3', '0.75', 'Notes');
INSERT INTO `horses` VALUES ('41', 'SumLizardRedSlow2', '216', '1', '3', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('42', 'SumLizardWhtFast', '216', '1', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('43', 'SumLizardWhtRun1', '216', '1', '0', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('44', 'SumLizardWhtRun2', '216', '1', '0', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('45', 'SumLizardWhtSlow1', '216', '1', '0', '0.75', 'Notes');
INSERT INTO `horses` VALUES ('46', 'SumLizardWhtSlow2', '216', '1', '0', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('47', 'SumNightmareFast', '42', '2', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('48', 'SumPuma1Fast', '63', '2', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('49', 'SumPuma3Fast', '63', '2', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('50', 'SumRoboboar', '472', '2', '0', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('51', 'SumRoboboarFast', '472', '2', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('52', 'SumRoboboarRun1', '472', '2', '0', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('53', 'SumRoboboarRun2', '472', '2', '0', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('54', 'SumRoboboarSlow2', '472', '2', '0', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('55', 'SumUnicornFast', '216', '0', '1', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('56', 'SumWarHorseBlFast', '216', '0', '2', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('57', 'SumWarHorseBlRun1', '216', '0', '2', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('58', 'SumWarHorseBlRun2', '216', '0', '2', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('59', 'SumWarHorseBlSlow1', '216', '0', '2', '0.75', 'Notes');
INSERT INTO `horses` VALUES ('60', 'SumWarHorseBlSlow2', '216', '0', '2', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('61', 'SumWarHorseBrFast', '216', '0', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('62', 'SumWarHorseBrRun1', '216', '0', '0', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('63', 'SumWarHorseBrRun2', '216', '0', '0', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('64', 'SumWarHorseBrSlow1', '216', '0', '0', '0.75', 'Notes');
INSERT INTO `horses` VALUES ('65', 'SumWarHorseBrSlow2', '216', '0', '0', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('66', 'SumWarHorseTaFast', '216', '0', '3', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('67', 'SumWarHorseTaRun1', '216', '0', '3', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('68', 'SumWarHorseTaRun2', '216', '0', '3', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('69', 'SumWarHorseTaSlow1', '216', '0', '3', '0.75', 'Notes');
INSERT INTO `horses` VALUES ('70', 'SumWarHorseTaSlow2', '216', '0', '3', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('71', 'SumWarHorseWhFast', '216', '0', '1', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('72', 'SumWarHorseWhRun1', '216', '0', '1', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('73', 'SumWarHorseWhRun2', '216', '0', '1', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('74', 'SumWarHorseWhSlow1', '216', '0', '1', '0.75', 'Notes');
INSERT INTO `horses` VALUES ('75', 'SumWarHorseWhSlow2', '216', '0', '1', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('76', 'SumWorgFastClaimDigital', '42', '2', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('77', 'SumWorgFastClaimRetailBox', '42', '2', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('78', 'SumWorgRun1ClaimDigital', '42', '2', '0', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('79', 'SumWorgRun1ClaimRetailBox', '42', '2', '0', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('80', 'SumWorgRun2ClaimDigital', '42', '2', '0', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('81', 'SumWorgRun2ClaimRetailBox', '42', '2', '0', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('82', 'SumWorgSlow2ClaimDigital', '42', '2', '0', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('83', 'SumWorgSlow2ClaimRetailBox', '42', '2', '0', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('84', 'TestHorseA', '216', '0', '0', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('85', 'TestWarHorseA\r\nTestHorseA\r\nTestH', '216', '0', '0', '1.00', 'Notes');