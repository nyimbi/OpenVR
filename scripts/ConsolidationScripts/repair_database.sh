#!/bin/bash

cd `dirname $0`
SCRIPTS_DIR=$PWD
#OPENVR_DIR=${SCRIPTS_DIR/scripts/}
OPENVR_DIR="/opt/openvr"
DB_USER="openvr"
DB_PASSWORD="I\$0gSowLYazb4"
#DB_PASSWORD="9p98qysYZRE2hu4K"
DB_NAME="openvr"


BACKUP_SCRIPT="backup.sh"

MOUNT_ENC_SCRIPT="mount_backup.sh"
UNMOUNT_ENC_SCRIPT="unmount_backup.sh"
CHECK_REMOVABLE_SCRIPT="check_removable.sh"
QUERIES_DIR="$OPENVR_DIR/scripts/queries"

DB_BACKUP_EXTENSION="INEC-BK-*.gz"
BACKUP_SQL_EXTENSION="*.sql"
LOCAL_BACKUP="$OPENVR_DIR/share/info"
TEMP_DIR="$OPENVR_DIR/tmp"
OPENVR_DB="openvr"

EXT_BK_DIR="OpenVR-BK"

MYSQL_REPAIR_CMD="mysqlrepair $OPENVR_DB -u $DB_USER --password=$DB_PASSWORD  -h 127.0.0.1 -P 30311 "
#MYSQL_REPAIR_CMD="mysqlrepair $OPENVR_DB -u $DB_USER --password=$DB_PASSWORD  -h 127.0.0.1  "
SUBSTR_CMD="$SCRIPTS_DIR/substring"
REPAIR_DB="$QUERIES_DIR/repair_db.sql"





# Backup
#$SCRIPTS_DIR/$BACKUP_SCRIPT

echo "Repairing DB"
$MYSQL_REPAIR_CMD



