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
APP="concept"

RELEASE_DIR="release"
PATCH_DIR="/opt/.openvr-patches"
TEMP_DIR="tmp"
PREFIX="INEC-OPENVR"
dateTime=`date +%Y%m%d_%H.%M.%S`

DB_USER="openvr"
DB_PASSWORD="9p98qysYZRE2hu4K"
DB_NAME="openvr"
MYSQL_INPUT_CMD="mysql -u $DB_USER --password=$DB_PASSWORD "

echo "SCRIPTS DIR: $SCRIPTS_DIR"
echo "OPENVR_DIR: $OPENVR_DIR"
echo "==============================================================="
echo "Importing Releases"
echo "==============================================================="
echo

mkdir -p "$PATCH_DIR"

# Mount Encrypted Storage Box
$SCRIPTS_DIR/$MOUNT_ENC_SCRIPT

# Removable Script Box
mount | grep sd |
while read disk; do
	curdisk=`$SUBSTR_CMD "$disk" "on " " type"`		
	if [ "$curdisk" = "/" ]; then
		echo "Not Using $curdisk";
	else
		echo "Copying releases from $curdisk ...";		
		cd "$curdisk";
		rsync -azvu "$curdisk/$EXT_BK_DIR/$RELEASE_DIR" "$PATCH_DIR"
	fi
done


# Unmount Encrypted Storage Box
$SCRIPTS_DIR/$UNMOUNT_ENC_SCRIPT

# Link Latest to OpenVR folder
OpenVRLatestDir=`ls $PATCH_DIR/$RELEASE_DIR -t | head -n 1`


#rm $OPENVR_DIR/concept
rm "$OPENVR_DIR$APP"
ln -s "$PATCH_DIR/$RELEASE_DIR/$OpenVRLatestDir/concept" "$OPENVR_DIR/concept"

if [ -e "$PATCH_DIR/$RELEASE_DIR/$OpenVRLatestDir/sql/" ]; 
	ls $PATCH_DIR/$RELEASE_DIR/$OpenVRLatestDir/sql/ | 
	while read dbpatch; do
		`$MYSQL_INPUT_CMD $DB_NAME < "$PATCH_DIR/$RELEASE_DIR/$OpenVRLatestDir/sql/$dbpatch"
	done
fi

