#!/bin/bash
# 1. Detect ExHD [DONE]
# 2. Format if NOT Formatted
# 3. Mount if not mounted.
# 4. Backup External HDD. [DONE]
# 5. Import Files in Backup to Staging DB
# 6. Merge Changes
# 7. Remove Backup Files

cd `dirname $0`
SCRIPTS_DIR=$PWD
#OPENVR_DIR=${SCRIPTS_DIR/scripts/}
OPENVR_DIR="/opt/openvr"

DB_USER="openvr"
DB_PASSWORD="I\$0gSowLYazb4"
DB_NAME="openvr"



MOUNT_ENC_SCRIPT="mount_backup.sh"
UNMOUNT_ENC_SCRIPT="unmount_backup.sh"
CHECK_REMOVABLE_SCRIPT="check_removable.sh"
QUERIES_DIR="$OPENVR_DIR/scripts/queries"
#SCRIPTS_DIR="$OPENVR_DIR/scripts"
DB_BACKUP_EXTENSION="INEC-BK-*.gz"
BACKUP_SQL_EXTENSION="*.sql"
LOCAL_BACKUP="$OPENVR_DIR/share/info"
TEMP_DIR="$OPENVR_DIR/tmp"
TEMP_DB="openvr_merging"
ENC_KEY_SCRIPT="key.sh"
EXT_BK_DIR="OpenVR-BK"
EXT_ENCFS_RAW_DIR=".OpenVR-RAW"

CREATE_TEMP_DB_CMD="mysqladmin -u $DB_USER --password=$DB_PASSWORD create $TEMP_DB  -h 127.0.0.1 -P 30311 "
DROP_TEMP_DB_CMD="mysqladmin -u $DB_USER --password=$DB_PASSWORD drop $TEMP_DB  -h 127.0.0.1 -P 30311 "

MYSQL_INPUT_CMD="mysql -u $DB_USER --password=$DB_PASSWORD  -h 127.0.0.1 -P 30311  "
IMPORT_SQL="mysql -u $DB_USER --password=$DB_PASSWORD  -h 127.0.0.1 -P 30311  $TEMP_DB"
SUBSTR_CMD="$OPENVR_DIR/scripts/substring"
BACKUP_WORKING_DB="mysqldump -u $DB_USER --password=$DB_PASSWORD  -h 127.0.0.1 -P 30311  $DB_NAME  | gzip >  $TEMP_DIR/openvr_working_db.sql.gz"

MERGE_WORKING_WITH_TEMP_DB="$QUERIES_DIR/merge_dbs.sql"
CREATE_TEMP_DB="$QUERIES_DIR/create_merging_db.sql"
REMOVE_TEMP_DB="$QUERIES_DIR/remove_merging_db.sql"



# GET MAC ADDRESS
macAddr=`ifconfig -a | grep eth | cut -d " " -f 11 | sed -e 's/^[ \t]*//'`
macAddr=${macAddr//:/}
echo $macAddr

# IMPORT BACKUP FILE
function importfile {

	fileName=$1;
	echo "Processing $fileName"
	echo "Creating Staging Database"
	`$MYSQL_INPUT_CMD  < $CREATE_TEMP_DB`
	echo "$MYSQL_INPUT_CMD $TEMP_DB < $CREATE_TEMP_DB"

	echo "Importing Backup Info to TEMP DB"
	`gunzip < $fileName | $MYSQL_INPUT_CMD $TEMP_DB`

	# Backup Original Table
	#`$BACKUP_WORKING_DB`

	# Merge Database with using transaction if possible (else restore bk on fail)
	# If Merge Unsuccessful, Restore Backup
	echo "Merging TEMP DB With Working DB"
	`$MYSQL_INPUT_CMD $TEMP_DB < $MERGE_WORKING_WITH_TEMP_DB`

	# Remove TEMP_DB
	echo "Droping Staging Database"
	`$MYSQL_INPUT_CMD $TEMP_DB < $REMOVE_TEMP_DB`
}


# All External Hardware
#mount | grep sd | cut -d " " -f 1
#while read devName; do
#	echo "Found $devName"
#done;


mkdir -p $LOCAL_BACKUP
mkdir -p $TEMP_DIR;

curdisk="/backups"

mkdir -p "$curdisk/tmp"
mv "$curdisk/$EXT_BK_DIR/*" "$curdisk/tmp"

# Mount Encrypted Storage Box (/backups)
encMounted=`mount | grep "$curdisk/$EXT_BK_DIR" | wc -l`
if [ "$encMounted" == "0" ]; then
	echo "Mounting Encrypted Device"
	encfs --extpass="$SCRIPTS_DIR/$ENC_KEY_SCRIPT" "$curdisk/$EXT_ENCFS_RAW_DIR" "$curdisk/$EXT_BK_DIR"
else
	echo "Already Mounted!"
fi

mv "$curdisk/tmp/*" "$curdisk/$EXT_BK_DIR/"

echo
echo "=============================================="
echo "<< Using $curdisk >>";
cd "$curdisk";
echo
# Sync Local Backup
echo "<< Synchronizing Local Backup >>"
#rsync -azvu "$curdisk/$EXT_BK_DIR/$macAddr" "$LOCAL_BACKUP"
echo "<< Synchronizing Fingerprint Templates >>"
#rsync -azvu "$curdisk/$EXT_BK_DIR/$macAddr/stored_minutiae" "$OPENVR_DIR/stored_minutiae"
find -name stored_minutiae |
while read fpdir; do
	rsync -azvu "$fpdir" "$OPENVR_DIR/"
	#rsync -azvu "$curdisk/$EXT_BK_DIR/stored_minutiae" "$OPENVR_DIR/stored_minutiae"
done
echo
echo
echo "<< Restoring Latest Database Backups >>"

#	find -name "$DB_BACKUP_EXTENSION" |
bkfile=`ls $curdisk/$EXT_BK_DIR/$macAddr/database -t | head -n 1`

# Backup All
echo
cp "$bkfile" "$TEMP_DIR"
filename=`basename $bkfile`
echo "Backup File: $bkfile"
echo "Backup Filename: $filename"
importfile "$curdisk/$EXT_BK_DIR/$macAddr/database/$filename"


# Unmount Encrypted Storage Box
encMounted=`mount | grep "$curdisk/$EXT_BK_DIR" | wc -l`
if [ "$encMounted" -gt "0" ]; then
	echo "Unmounting Encrypted Device"
	echo "$curdisk/$EXT_BK_DIR"
	fusermount -u "$curdisk/$EXT_BK_DIR"
else
	echo "Already Mounted!"
fi

rm -rf "$TEMP_DIR/*"
