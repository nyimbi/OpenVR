
REPLACE INTO openvr.registrations SELECT * FROM openvr_merging.registrations;
REPLACE INTO openvr.fp_fingerprints SELECT * FROM openvr_merging.fp_fingerprints;

INSERT INTO openvr.logs (title, created) VALUES ("Importing Registrations And Fingerprint Database", NOW());
