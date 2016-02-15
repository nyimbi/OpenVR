
REPLACE INTO openvr.registrations SELECT * FROM openvr_merging.registrations;
REPLACE INTO openvr.fp_fingerprints SELECT * FROM openvr_merging.fp_fingerprints;
REPLACE INTO openvr.auto_scans SELECT * FROM openvr_merging.auto_scans;
REPLACE INTO openvr.experiment_results SELECT * FROM openvr_merging.experiment_results;
REPLACE INTO openvr.registered_operators SELECT * FROM openvr_merging.registered_operators;
REPLACE INTO openvr.administrators SELECT * FROM openvr_merging.administrators;
REPLACE INTO openvr.duplicate_maps SELECT * FROM openvr_merging.duplicate_maps;


INSERT INTO openvr.logs (title, created) VALUES ("Importing Registrations And Fingerprint Database", NOW());
