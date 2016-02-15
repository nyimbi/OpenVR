#!/bin/sh

whoAmI=`whoami`
if [ $whoAmI != "openvr" ]; then
echo "Looks like you're a dev user! So aborting!"
exit;
fi

rm -rf /opt/openvr/stored_minutiae/* /opt/openvr/stored_minutiae_sym/* /opt/openvr/stored_minutiae_symx/* /opt/openvr/templates/* /backups/OpenVR-BK/* /backups/.OpenVR-ENC/* /backups/tmp/*
MYSQL_CMD="mysql --force -u root -P 30311 -h 127.0.0.1 openvr --password=0yVcBpu.Ce3g$" 
`$MYSQL_CMD -e "TRUNCATE administrators"`
`$MYSQL_CMD -e "TRUNCATE audit_logs "`
`$MYSQL_CMD -e "TRUNCATE authorized_operators "`
`$MYSQL_CMD -e "TRUNCATE auto_scans "`
`$MYSQL_CMD -e "TRUNCATE duplicate_maps "`
`$MYSQL_CMD -e "TRUNCATE experiment_results "`
`$MYSQL_CMD -e "TRUNCATE fp_fingerprints "`
`$MYSQL_CMD -e "TRUNCATE fp_slim "`
`$MYSQL_CMD -e "TRUNCATE import_dump_logs "`
`$MYSQL_CMD -e "TRUNCATE import_logs "`
`$MYSQL_CMD -e "TRUNCATE logs "`
`$MYSQL_CMD -e "TRUNCATE merged_dumps "`
`$MYSQL_CMD -e "TRUNCATE registered_operators "`
`$MYSQL_CMD -e "TRUNCATE registrations "`
`$MYSQL_CMD -e "TRUNCATE server_consolidation_logs "`
`$MYSQL_CMD -e "TRUNCATE server_dump_logs "`
`$MYSQL_CMD -e "TRUNCATE settings "`
`$MYSQL_CMD -e "TRUNCATE temp_available_pu "`
`$MYSQL_CMD -e "TRUNCATE temp_duplicates "`
`$MYSQL_CMD -e "TRUNCATE temp_duplicate_exceptions "`
`$MYSQL_CMD -e "TRUNCATE temp_duplicate_invalids "`
`$MYSQL_CMD -e "TRUNCATE temp_duplicate_maps "`
`$MYSQL_CMD -e "TRUNCATE temp_duplicate_maps_exceptions "`
`$MYSQL_CMD -e "TRUNCATE temp_invalids "`
`$MYSQL_CMD -e "TRUNCATE temp_pu_voters "`
`$MYSQL_CMD -e "TRUNCATE temp_regs "`
`$MYSQL_CMD -e "TRUNCATE temp_valids "`
`$MYSQL_CMD -e "TRUNCATE tmp_afis_scans "`
`$MYSQL_CMD -e "TRUNCATE tmp_cmds "`
`$MYSQL_CMD -e "TRUNCATE tmp_registrations "`
`$MYSQL_CMD -e "TRUNCATE tmp_search "`
