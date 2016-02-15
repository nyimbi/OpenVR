#!/bin/bash
cd `dirname $0`
SCRIPTS_DIR=$PWD
OPENVR_DIR=${SCRIPTS_DIR/scripts/}

CHECK_REMOVABLE_SCRIPT="check_removable.sh"
MOUNT_ENC_SCRIPT="mount_backup.sh"
UNMOUNT_ENC_SCRIPT="unmount_backup.sh"

LOCAL_BACKUP="$OPENVR_DIR/share/info"
SUBSTR_CMD="$OPENVR_DIR/scripts/substring"
TEMP_DIR="$OPENVR_DIR/tmp"

EXT_BK_DIR="OpenVR-BK"

DB_BK_DIR="$OPENVR_DIR/database"
BK_DATABASES="openvr"
BK_TABLES="registrations fp_fingerprints experiment_results"
BK_LOGFILE="backup-log.txt"

macAddr=`ifconfig eth0 | grep HWaddr | cut -d " " -f 11 | sed -e 's/^[ \t]*//'`
macAddr=${macAddr//:/}
dateTime=`date +%Y%m%d_%H.%M.%S`
prefix="INEC-BK"

# Mount Encrypted Storage Box
$SCRIPTS_DIR/$MOUNT_ENC_SCRIPT


#echo "System MAC: $macAddr"
mysqldump -u openvr --password=9p98qysYZRE2hu4K --databases "$BK_DATABASES" --tables $BK_TABLES | gzip > "$DB_BK_DIR/$prefix-$macAddr-$dateTime.sql.gz"

# Sync With External Storage


#mkdir -p $TEMP_DIR;
mount | grep sd |
while read disk; do
	curdisk=`$SUBSTR_CMD "$disk" "on " " type"`		
	if [ "$curdisk" = "/" ]; then
		echo "Not Using $curdisk";
		
	else
		echo "Backuping $curdisk";		
		cd "$curdisk";
		mkdir -p "$curdisk/$EXT_BK_DIR/$macAddr"
		rsync -azvu $DB_BK_DIR "$curdisk/$EXT_BK_DIR/$macAddr/" 

		#mkdir -p $LOCAL_BACKUP
		#find -name "$BACKUP_EXTENSION" |
		#while read bkfile; do
			# Backup All 
		#	echo "Importing $bkfile"
			#importfile "$bkfile"
		#done
	fi
done


# Unmount Encrypted Storage Box
$SCRIPTS_DIR/$UNMOUNT_ENC_SCRIPT
