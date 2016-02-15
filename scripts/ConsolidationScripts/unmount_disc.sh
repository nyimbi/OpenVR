#!/bin/bash

cd `dirname $0`
SCRIPTS_DIR=$PWD

ENC_KEY_SCRIPT="key.sh"
OPENVR_DIR=${SCRIPTS_DIR/scripts/}
EXT_BK_DIR="OpenVR-BK"
EXT_ENCFS_RAW_DIR=".OpenVR-RAW"
LOCAL_BACKUP="$OPENVR_DIR/share/info"
SUBSTR_CMD="$OPENVR_DIR/scripts/substring"
CHECK_REMOVABLE_SCRIPT="check_disc.sh"






echo
echo "==============================================================="
echo "Unmounting All Encryted Backup Folder on External Storage"
echo "==============================================================="
echo
#echo "Encryption Key: $SCRIPTS_DIR/$ENC_KEY_SCRIPT"

$SCRIPTS_DIR/$CHECK_REMOVABLE_SCRIPT |
while read curdisk; do
	echo "RAW Folder: $curdisk/$EXT_ENCFS_RAW_DIR"
	echo "Mount Point: $curdisk/$EXT_BK_DIR"

	encMounted=`mount | grep "$curdisk/$EXT_BK_DIR" | wc -l`
	if [ "$encMounted" -gt "0" ]; then
		echo "Unmounting Encrypted Device"
		echo "$curdisk/$EXT_BK_DIR"
		fusermount -u "$curdisk/$EXT_BK_DIR"
	else
		echo "Already Mounted!"
	fi
done


