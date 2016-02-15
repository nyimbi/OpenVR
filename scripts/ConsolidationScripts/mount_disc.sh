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
TMP_DIR="/tmp"






echo
echo "==============================================================="
echo "Mounting Encryted Backup Folder on External Storage"
echo "==============================================================="
echo

$SCRIPTS_DIR/$CHECK_REMOVABLE_SCRIPT |
while read curdisk; do
	echo "RAW Folder: $curdisk/$EXT_ENCFS_RAW_DIR"
	echo "Mount Point: $TMP_DIR/$curdisk/$EXT_BK_DIR"

	mkdir -p "$TMP_DIR/$curdisk/$EXT_BK_DIR"

	encMounted=`mount | grep "$TMP_DIR/$curdisk/$EXT_BK_DIR" | wc -l`
	if [ "$encMounted" == "0" ]; then
		echo "Mounting Encrypted Device"
		encfs --extpass="$SCRIPTS_DIR/$ENC_KEY_SCRIPT" "$curdisk/$EXT_ENCFS_RAW_DIR" "$TMP_DIR/$curdisk/$EXT_BK_DIR"
	else
		echo "Already Mounted!"
	fi

done


