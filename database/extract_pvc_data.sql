
CREATE TABLE IF NOT EXISTS `pvc_registrations` (
  `id` char(40) NOT NULL,
  `polling_unit_id` int(11) DEFAULT NULL,
  `delimitation` varchar(255) DEFAULT NULL,
  `last_name` varchar(100) DEFAULT NULL,
  `first_name` varchar(100) DEFAULT NULL,
  `other_names` varchar(100) DEFAULT NULL,
  `dob_day` varchar(2) DEFAULT NULL,
  `dob_month` varchar(4) DEFAULT NULL,
  `dob_year` varchar(4) DEFAULT NULL,
  `gender` varchar(6) DEFAULT NULL,
  `home_address` varchar(255) DEFAULT NULL,
  `occupation` varchar(255) DEFAULT NULL,
  `photo_front` blob,
  `int_created` datetime DEFAULT NULL,
  `is_special` tinyint(1) NOT NULL DEFAULT '0',
  `state` varchar(255) NOT NULL,
  `lga` varchar(255) NOT NULL,
  `ward` varchar(255) NOT NULL,
  `pu` varchar(255) NOT NULL,
  `state_code` char(2) NOT NULL,
  `lga_code` char(2) NOT NULL,
  `ward_code` varchar(3) NOT NULL,
  `pu_code` varchar(5) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `polling_unit_id` (`polling_unit_id`),
  KEY `deli` (`delimitation`),
  KEY `l_name` (`last_name`),
  KEY `f_name` (`first_name`),
  KEY `o_name` (`other_names`),
  KEY `dob_d` (`dob_day`),
  KEY `dob_m` (`dob_month`),
  KEY `dob_y` (`dob_year`),
  KEY `genda` (`gender`),
  KEY `occup` (`occupation`),
  KEY `d_created` (`int_created`),
  KEY `is_spe` (`is_special`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;


INSERT IGNORE INTO pvc_registrations (SELECT reg.id,reg.polling_unit_id,reg.delimitation,last_name,first_name,other_names,
	dob_day,dob_month,dob_year,gender,home_address,occupation,photo_front,int_created,is_special,s.name,l.name,
	r.name,p.name,s.abbreviation,l.abbreviation,r.abbreviation,p.abbreviation FROM registrations reg 
	LEFT JOIN polling_units p ON p.id=reg.polling_unit_id LEFT JOIN registration_areas r ON r.id=p.registration_area_id
	LEFT JOIN local_governments l ON l.id=r.local_government_id LEFT JOIN states s ON s.id=l.state_id);



CREATE TABLE IF NOT EXISTS `pvc_fingerprints` (
  `registration_id` char(30) NOT NULL DEFAULT '',
  `finger_number` tinyint(2) NOT NULL,
  `finger_abbrv` char(2) NOT NULL,
  `template` longblob NOT NULL,
  PRIMARY KEY (`registration_id`,`finger_number`),
  KEY `registration_id` (`registration_id`),
  KEY `finger_number` (`finger_number`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;


INSERT IGNORE INTO pvc_fingerprints(registration_id,finger_number,template) (SELECT registration_id,fingerprint_finger,fingerprint_minutia FROM fp_fingerprints);

DELETE from pvc_fingerprints WHERE registration_id IN (SELECT vin FROM invalid_voters WHERE status=0);

DELETE from pvc_registrations WHERE id IN (SELECT vin FROM invalid_voters WHERE status=0);

CREATE TABLE rul_bs (SELECT registration_id FROM (SELECT registration_id,count(*) tt from pvc_fingerprints GROUP BY registration_id) as o WHERE tt<2);

DELETE FROM pvc_registrations WHERE id IN ( select registration_id from rul_bs);

UPDATE pvc_registrations SET last_name=UPPER(last_name),
first_name=UPPER(first_name),
other_names=UPPER(other_names),
home_address=UPPER(home_address),
occupation=UPPER(occupation);

UPDATE pvc_registrations SET occupation='OTHER' WHERE occupation='' OR occupation IS NULL;


SELECT COUNT(*) as 'Total to Print' FROM pvc_registrations;

