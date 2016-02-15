#!/bin/bash

cd `dirname $0`
SCRIPTS_DIR=$PWD
#OPENVR_DIR=${SCRIPTS_DIR/scripts/}
OPENVR_DIR="/opt/openvr"
DB_USER="openvr"
DB_PASSWORD="I\$0gSowLYazb4"
DB_NAME="openvr"


ENC_KEY_SCRIPT="key.sh"
MOUNT_ENC_SCRIPT="mount_backup.sh"
UNMOUNT_ENC_SCRIPT="unmount_backup.sh"
CHECK_REMOVABLE_SCRIPT="check_removable.sh"
QUERIES_DIR="$OPENVR_DIR/scripts/queries"

DB_BACKUP_EXTENSION="INEC-BK-*.gz"
BACKUP_SQL_EXTENSION="*.sql"
LOCAL_BACKUP="$OPENVR_DIR/share/info"
TEMP_DIR="$OPENVR_DIR/tmp"
TEMP_DB="openvr_merging"

EXT_BK_DIR="OpenVR-BK"
IMPORT_LOGS="$OPENVR_DIR/logs.txt"

DB_BACKUP="$OPENVR_DIR/database"




cd $DB_BACKUP

bkCount=`ls $DB_BACKUP -t | wc -l`
removeCount=`expr $bkCount - 1 `

# The following dumps will be removed
ls $DB_BACKUP -t | tail -n $removeCount |
while read dump; do
	# echo $dump
	rm $dump
done


# Mount Encrypted Storage Box
$SCRIPTS_DIR/$MOUNT_ENC_SCRIPT

# CLEAN UP /backups
rm -rf /backups/$EXT_BK_DIR/*
rm -rf $OPENVR_DIR/tmp/*

# Unmount Encrypted Storage Box
$SCRIPTS_DIR/$UNMOUNT_ENC_SCRIPT

