-- --------------------------------------------------------
-- Host:                         127.0.0.1
-- Server version:               5.6.23 - MySQL Community Server (GPL)
-- Server OS:                    Win64
-- HeidiSQL Version:             12.8.0.6908
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;


-- Dumping database structure for characters
CREATE DATABASE IF NOT EXISTS `characters` /*!40100 DEFAULT CHARACTER SET armscii8 COLLATE armscii8_bin */;
USE `characters`;

-- Dumping structure for table characters.levelcraft_unit_experience
CREATE TABLE IF NOT EXISTS `levelcraft_unit_experience` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `account` int(11) DEFAULT NULL,
  `character` int(11) DEFAULT NULL,
  `unit` int(11) DEFAULT NULL,
  `zone` int(11) DEFAULT NULL,
  `damage_dealt` bigint(20) DEFAULT '0',
  `damage_received` bigint(20) DEFAULT '0',
  `crowd_controls` int(11) DEFAULT '0',
  `kills` int(11) DEFAULT '0',
  `deaths` int(11) DEFAULT '0',
  PRIMARY KEY (`id`) USING BTREE,
  KEY `idx_account_unit` (`account`,`unit`),
  KEY `idx_account_zone` (`account`,`zone`),
  KEY `idx_character_unit` (`character`,`unit`),
  KEY `idx_character_zone` (`character`,`zone`)
) ENGINE=InnoDB DEFAULT CHARSET=armscii8 COLLATE=armscii8_bin;

-- Dumping data for table characters.levelcraft_unit_experience: ~0 rows (approximately)

/*!40103 SET TIME_ZONE=IFNULL(@OLD_TIME_ZONE, 'system') */;
/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IFNULL(@OLD_FOREIGN_KEY_CHECKS, 1) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40111 SET SQL_NOTES=IFNULL(@OLD_SQL_NOTES, 1) */;
