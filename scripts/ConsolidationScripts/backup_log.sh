#!/bin/bash
cd `dirname $0`
SCRIPTS_DIR=$PWD
OPENVR_DIR="/opt/openvr"
DB_BK_FOLDER="$OPENVR_DIR/backup"
DB_BK_DIR="$DB_BK_FOLDER/$1"
BK_DATABASES="openvr_cvr"
DB_USER="root"
DB_PASS="12345678"
BK_LOG_FILE="$DB_BK_DIR/backup.log"
XML_LOG_FILE="$DB_BK_DIR/backup.xml"

macAddr=`ifconfig -a | grep eth | cut -d " " -f 11 | sed -e 's/^[ \t]*//'`
macAddr=${macAddr//:/}
dateTime=`date +%Y%m%d_%H.%M.%S`
prefix="INEC-BK"
#echo "System MAC: $macAddr"

mkdir -p "$DB_BK_DIR"

mysqldump -ntc --insert-ignore -X -u $DB_USER --password=$DB_PASS --where="id='$1'"  --databases $BK_DATABASES --tables backups > $XML_LOG_FILE
mysqldump -ntc --insert-ignore -X -u $DB_USER --password=$DB_PASS --where="session_id='$1'"  --databases $BK_DATABASES --tables backup_sessions >> $XML_LOG_FILE
mysqldump -ntc --insert-ignore -u $DB_USER --password=$DB_PASS --where="id='$1'"  --databases $BK_DATABASES --tables backups > $BK_LOG_FILE
mysqldump -ntc --insert-ignore -u $DB_USER --password=$DB_PASS --where="session_id='$1'"  --databases $BK_DATABASES --tables backup_sessions >> $BK_LOG_FILE


