#!/bin/bash
cd `dirname $0`
SCRIPTS_DIR=$PWD
#OPENVR_DIR=${SCRIPTS_DIR/scripts/}
OPENVR_DIR="/opt/openvr"

CHECK_REMOVABLE_SCRIPT="check_removable.sh"
MOUNT_ENC_SCRIPT="mount_backup.sh"
UNMOUNT_ENC_SCRIPT="unmount_backup.sh"

LOCAL_BACKUP="$OPENVR_DIR/share/info"
SUBSTR_CMD="$SCRIPTS_DIR/scripts/substring"
TEMP_DIR="$OPENVR_DIR/tmp"

EXT_BK_DIR="OpenVR-BK"

DB_BK_DIR="$OPENVR_DIR/database"
BK_DATABASES="openvr"
BK_TABLES="registrations fp_fingerprints experiment_results duplicate_maps auto_scans registered_operators administrators"
BK_LOGFILE="backup-log.txt"






macAddr=`ifconfig -a | grep eth | cut -d " " -f 11 | sed -e 's/^[ \t]*//'`
macAddr=${macAddr//:/}
dateTime=`date +%Y%m%d_%H.%M.%S`
prefix="INEC-BK"

# Mount Encrypted Storage Box
$SCRIPTS_DIR/$MOUNT_ENC_SCRIPT


#echo "System MAC: $macAddr"
mysqldump -u openvr --password=I\$0gSowLYazb4 -h 127.0.0.1 -P 30311 --complete-insert --databases "$BK_DATABASES" --tables $BK_TABLES | gzip > "$DB_BK_DIR/$prefix-$macAddr-$dateTime.sql.gz"

# Sync With External Storage


#mkdir -p $TEMP_DIR;
$SCRIPTS_DIR/$CHECK_REMOVABLE_SCRIPT |
while read curdisk; do
	echo "Backuping $curdisk";
	cd "$curdisk";
	mkdir -p "$curdisk/$EXT_BK_DIR/$macAddr"
	rsync -avu $DB_BK_DIR "$curdisk/$EXT_BK_DIR/$macAddr/"
done


# Unmount Encrypted Storage Box
$SCRIPTS_DIR/$UNMOUNT_ENC_SCRIPT
