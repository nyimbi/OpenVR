#!/bin/bash

cd `dirname $0`
SCRIPTS_DIR=$PWD
ENC_KEY_SCRIPT="key.sh"
OPENVR_DIR=${SCRIPTS_DIR/scripts/}
EXT_BK_DIR="OpenVR-BK"
EXT_ENCFS_RAW_DIR=".OpenVR-RAW"
LOCAL_BACKUP="$OPENVR_DIR/share/info"
SUBSTR_CMD="$OPENVR_DIR/scripts/substring"

echo
echo "==============================================================="
echo "Mounting Encryted Backup Folder on External Storage"
echo "==============================================================="
echo 
echo "Encryption Key: $SCRIPTS_DIR/$ENC_KEY_SCRIPT"


mount | grep sd |
while read disk; do
	curdisk=`$SUBSTR_CMD "$disk" "on " " type"`		
	if [ "$curdisk" != "/" ]; then
		echo "RAW Folder: $curdisk/$EXT_ENCFS_RAW_DIR"
		echo "Mount Point: $curdisk/$EXT_BK_DIR"
		mkdir -p "$curdisk/$EXT_BK_DIR"
		mkdir -p "$curdisk/$EXT_ENCFS_RAW_DIR"

		encMounted=`mount | grep "$curdisk/$EXT_BK_DIR" | wc -l` 
		if [ "$encMounted" == "0" ]; then		
			echo "Mounting Encrypted Device"			
			encfs --extpass="$SCRIPTS_DIR/$ENC_KEY_SCRIPT" "$curdisk/$EXT_ENCFS_RAW_DIR" "$curdisk/$EXT_BK_DIR"
		else
			echo "Already Mounted!"
		fi
	fi
done


