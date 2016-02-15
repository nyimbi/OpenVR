ALTER TABLE  `registrations` ADD  `is_special` TINYINT NOT NULL DEFAULT  '0',
ADD  `special_group` VARCHAR( 40 ) NULL ,
ADD  `special_notes` VARCHAR( 255 ) NULL ,
ADD  `special_photo` BLOB NULL
