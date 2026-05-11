CREATE TABLE IF NOT EXISTS `character_resurrection` (
    `guid`       INT UNSIGNED NOT NULL,
    `rez_count`  TINYINT UNSIGNED NOT NULL DEFAULT 3,
    `last_reset` INT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;