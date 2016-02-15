#!/bin/bash
cd `dirname $0`
SCRIPTS_DIR=$PWD
OPENVR_DIR="/opt/openvr"
DB_USER="root"
DB_PASS="0yVcBpu.Ce3g$"
DB_BK_FOLDER="$OPENVR_DIR/backup"
DB_BK_DIR="$DB_BK_FOLDER/$2"
BK_DATABASES="openvr"
#BK_TABLES="registrations fp_fingerprints experiment_results duplicate_maps auto_scans registered_operators administrators"
BK_TABLES="registrations"
DB_DUMP_OPTIONS=" --insert-ignore -ntc "
WHERE_OPTION="polling_unit_id='$1'"
DB_DUMP_OPTIONS=" --complete-insert -h 127.0.0.1   -P 30311"


macAddr=`ifconfig -a | grep eth | cut -d " " -f 11 | sed -e 's/^[ \t]*//'`
macAddr=${macAddr//:/}
dateTime=`date +%Y%m%d_%H.%M.%S`
prefix="INEC-BK"
echo "System MAC: $macAddr"

#mysqldump -u "$DB_USER" --password="$DB_PASS" -h 127.0.0.1   --complete-insert --databases "$BK_DATABASES" --tables $BK_TABLES | gzip > "$DB_BK_DIR/

mkdir -p $DB_BK_DIR

if [ $1 ]; then
	mysqldump -u "$DB_USER" --password="$DB_PASS" $DB_DUMP_OPTIONS --where="polling_unit_id='$1'" --databases "$BK_DATABASES" --tables registrations | gzip > "$DB_BK_DIR/$prefix-$macAddr-$1-$dateTime.sql.gz"
	mysqldump -u "$DB_USER" --password="$DB_PASS" $DB_DUMP_OPTIONS --where="registration_id IN (SELECT id FROM registrations WHERE polling_unit_id='$1')" --databases "$BK_DATABASES" --tables fp_fingerprints | gzip >> "$DB_BK_DIR/$prefix-$macAddr-$1-$dateTime.sql.gz"
else
	mysqldump -u "$DB_USER" --password="$DB_PASS" $DB_DUMP_OPTIONS --databases "$BK_DATABASES" --tables $BK_TABLES | gzip > "$DB_BK_DIR/$prefix-$macAddr-$dateTime.sql.gz"
fi



