#!/bin/bash
cd `dirname $0`
SCRIPTS_DIR=$PWD
#OPENVR_DIR=${SCRIPTS_DIR/scripts/}
OPENVR_DIR="/opt/openvr"
FP_DIR="$OPENVR_DIR/stored_minutiae"

CHECK_REMOVABLE_SCRIPT="check_removable.sh"
MOUNT_ENC_SCRIPT="mount_backup.sh"
UNMOUNT_ENC_SCRIPT="unmount_backup.sh"

LOCAL_BACKUP="$OPENVR_DIR/share/info"
SUBSTR_CMD="$SCRIPTS_DIR/substring"
TEMP_DIR="$OPENVR_DIR/tmp"
EXT_BK_DIR="OpenVR-BK"

macAddr=`ifconfig -a | grep eth | cut -d " " -f 11 | sed -e 's/^[ \t]*//'`
macAddr=${macAddr//:/}
dateTime=`date +%Y%m%d_%H.%M.%S`







# Mount Encrypted Storage Box
$SCRIPTS_DIR/$MOUNT_ENC_SCRIPT

# Sync With External Storage  -- Backup to all external storage
$SCRIPTS_DIR/$CHECK_REMOVABLE_SCRIPT |
while read curdisk; do
	echo "$curdisk";
	mkdir -p "$curdisk/$EXT_BK_DIR"
	rsync -au "$FP_DIR" "$curdisk/$EXT_BK_DIR/$macAddr"
done


# Unmount Encrypted Storage Box
$SCRIPTS_DIR/$UNMOUNT_ENC_SCRIPT

