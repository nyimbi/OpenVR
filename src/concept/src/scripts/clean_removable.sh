#!/bin/bash
cd `dirname $0`
SCRIPTS_DIR=$PWD
OPENVR_DIR=${SCRIPTS_DIR/scripts/}
FP_DIR="$OPENVR_DIR/stored_minutiae"

CHECK_REMOVABLE_SCRIPT="check_removable.sh"
MOUNT_ENC_SCRIPT="mount_backup.sh"
UNMOUNT_ENC_SCRIPT="unmount_backup.sh"

LOCAL_BACKUP="$OPENVR_DIR/share/info"
SUBSTR_CMD="$OPENVR_DIR/scripts/substring"
TEMP_DIR="$OPENVR_DIR/tmp"
EXT_BK_DIR="OpenVR-BK"
BK_DIR="$OPENVR_DIR/database"
BK_DATABASES="openvr"
BK_TABLES="registrations fp_fingerprints"
BK_LOGFILE="backup-log.txt"

# Mount Encrypted Storage Box
$SCRIPTS_DIR/$MOUNT_ENC_SCRIPT


macAddr=`ifconfig eth0 | grep HWaddr | cut -d " " -f 11 | sed -e 's/^[ \t]*//'`
macAddr=${macAddr//:/}
dateTime=`date +%Y%m%d_%H.%M.%S`
prefix="INEC-BK"


# Clean all removable drives of backup data
$SCRIPTS_DIR/check_removable.sh | 
while read disk; do
	curdisk=`$SUBSTR_CMD "$disk" "on " " type"`		
	if [ "$curdisk" != "/" ]; then
		echo "$curdisk";		
		rm -rf "$curdisk/$EXT_BK_DIR/$macAddr"
	fi
done



# @TODO Allow cleaning of all backups or this machine's backup file.



# Unmount Encrypted Storage Box
$SCRIPTS_DIR/$UNMOUNT_ENC_SCRIPT



