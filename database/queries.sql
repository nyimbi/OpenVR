-- DROP THE TABLE IF IT EXISTS
DROP TABLE IF EXISTS vin_length;
-- CREATE TABLE
CREATE TABLE vin_length (id varchar(40), lid INTEGER,id_tr VARCHAR(40),PRIMARY KEY (id));
-- ADD INDEXES
ALTER TABLE vin_length ADD INDEX(lid);

-- POPULATE TABLE WITH DATA

INSERT INTO vin_length(id) (SELECT id FROM pvc_registrations);

--SET the length column
UPDATE vin_length SET lid=LENGTH(id),id_tr=id;
-- REMOVE NORMAL VINS
DELETE FROM vin_length WHERE lid=19;

-- REVIEW WORK REQUIRED
SELECT lid,COUNT(*) FROM vin_length GROUP BY lid;

-- DEAL WITH SHORT VINS
UPDATE vin_length SET id_tr=CONCAT('AAAAAAAAAA',id) WHERE lid=9;


-- DEAL WITH VINS WITH INVALID INVALID COMMAND
UPDATE vin_length SET id_tr=CONCAT('BBBBBBBBBB',REPLACE(id,'INVALID COMMAND','')) WHERE lid=24;

-- DEAL WITH VINS WITH TWO PARTS
UPDATE vin_length SET id_tr=SUBSTRING(id,12,30) WHERE lid=30;

--UPDATE THE LENFTHS COLUMN AGAIN
UPDATE vin_length SET lid=LENGTH(id_tr);

-- REVIEW WORK DONE
SELECT lid,COUNT(*) FROM vin_length GROUP BY lid;

-- UPDATE BASE TABLE
UPDATE pvc_registrations pvc LEFT JOIN vin_length v ON pvc.id=v.id SET pvc.id=v.id_tr WHERE pvc.id=v.id;
-- UPDATE LINKED TABLE

UPDATE pvc_fingerprints pvcf LEFT JOIN vin_length v ON pvcf.registrations_id=v.id SET pvcf.registrations_id=v.id_tr, pvcf.id=CONCAT(v.id_tr,'_',fingerprint_finger) WHERE pvcf.registrations_id=v.id;

