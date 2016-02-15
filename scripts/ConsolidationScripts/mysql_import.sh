#!/bin/bash
cd `dirname $0`
SCRIPTS_DIR=$PWD
#OPENVR_DIR=${SCRIPTS_DIR/scripts/}
OPENVR_DIR="/opt/openvr/"
DB_USER="root"
DB_PASSWORD="12345678"
DB_NAME="openvr_cvr"



MOUNT_ENC_SCRIPT="mount_backup.sh"
UNMOUNT_ENC_SCRIPT="unmount_backup.sh"
CHECK_REMOVABLE_SCRIPT="check_removable.sh"
QUERIES_DIR="$OPENVR_DIR/scripts/queries"

DB_BACKUP_EXTENSION="INEC-*.gz"
BACKUP_SQL_EXTENSION="*.sql"
LOCAL_BACKUP="$OPENVR_DIR/share/info"
TEMP_DIR="$OPENVR_DIR/tmp"
TEMP_DB="openvr_merging"

EXT_BK_DIR="OpenVR-BK"
IMPORT_LOGS="$OPENVR_DIR/logs.txt"
MYSQL_INPUT_CMD="mysql -u $DB_USER --password=$DB_PASSWORD openvr_cvr "

if [ "$2" == "unzipped" ]; then
	$MYSQL_INPUT_CMD < $1
else :
	gunzip < $1 |  $MYSQL_INPUT_CMD
fi;
