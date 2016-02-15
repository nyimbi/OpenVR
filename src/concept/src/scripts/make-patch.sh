#!/bin/bash

cd `dirname $0`
SCRIPTS_SRC_DIR=$PWD
SCRIPTS_DIR=${SCRIPTS_SRC_DIR/src\//}
OPENVR_DIR=${SCRIPTS_DIR/scripts/} 	
MANIFEST="src/scripts/manifest.txt"
EXT_BK_DIR="OpenVR-BK"
CHECK_REMOVABLE_SCRIPT="check_removable.sh"
MOUNT_ENC_SCRIPT="mount_backup.sh"
UNMOUNT_ENC_SCRIPT="unmount_backup.sh"
SUBSTR_CMD="$OPENVR_DIR/scripts/substring"
TEMP_DIR="$OPENVR_DIR/tmp"

RELEASE_DIR="release"
TEMP_DIR="tmp"
PREFIX="INEC-OPENVR"
dateTime=`date +%Y%m%d_%H.%M.%S`


echo "SCRIPTS DIR: $SCRIPTS_DIR"
echo "OPENVR_DIR: $OPENVR_DIR"
echo "==============================================================="
echo "Creating Release"
echo "==============================================================="
echo
echo "Compiling Scripts ..."
$SCRIPTS_SRC_DIR/compile_scripts.sh
echo
echo "Compiling Application ..."
cd $OPENVR_DIR
#make -w
#make

mkdir "$OPENVR_DIR/$RELEASE_DIR/$PREFIX-$dateTime" 

cat "$OPENVR_DIR/$MANIFEST" |
while read appfile; do
	cp -r $OPENVR_DIR/$appfile "$OPENVR_DIR/$RELEASE_DIR/$PREFIX-$dateTime"
done
echo
echo "Cleaning up new release ..."
rm -rf $OPENVR_DIR/$TEMP_DIR/*
rm -rf $OPENVR_DIR/photos/*
rm -rf $OPENVR_DIR/templates/*
rm -rf $OPENVR_DIR/database/*
#rm -rf $OPENVR_DIR/stored_minutiae/*

echo
echo "Copying new release to removable ..."


# Mount Encrypted Storage Box
$SCRIPTS_DIR/$MOUNT_ENC_SCRIPT

# Removable Script Box
mount | grep sd |
while read disk; do
	curdisk=`$SUBSTR_CMD "$disk" "on " " type"`		
	if [ "$curdisk" = "/" ]; then
		echo "Not Using $curdisk";
	else
		echo "Copying releases to $curdisk";		
		cd "$curdisk";
		mkdir -p "$curdisk/$EXT_BK_DIR/$RELEASE_DIR"
		rsync -azvu "$OPENVR_DIR/$RELEASE_DIR" "$curdisk/$EXT_BK_DIR" 
	fi
done

# Unmount Encrypted Storage Box
$SCRIPTS_DIR/$UNMOUNT_ENC_SCRIPT
