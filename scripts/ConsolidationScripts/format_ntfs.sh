#!/bin/bash

cd `dirname $0`
SCRIPTS_DIR=$PWD
OPENVR_DIR=${SCRIPTS_DIR/scripts/}

BK_FP="backup_fingerprints.sh"
BK_DB="backup_database.sh"
IMPORT_BK="import_backups.sh"
PASSWORD="password"
SUBSTR_CMD="$OPENVR_DIR/scripts/substring"

dateTime=`date +%Y%m%d_%H.%M.%S`

# GET MAC ADDRESS
macAddr=`ifconfig -a | grep eth | cut -d " " -f 11 | sed -e 's/^[ \t]*//'`
macAddr=${macAddr//:/}




# Find NTFS Disks
echo "$PASSWORD" | sudo -S fdisk -l | grep NTFS | cut -d " " -f1 |
while read ntfsDrive; do
	#Double check if it's NTFS
	isNTFS=`echo "$PASSWORD" | sudo -S parted $ntfsDrive print | grep ntfs`

	if [ "$isNTFS" != "" ]; then
		# Check if disk is mounted.
		isMounted=`mount | grep $ntfsDrive`
		echo "===================="
		echo " Formatting $ntfsDrive"
		echo
	#	echo "$isMounted"

		# Check if it's mounted
		if [ "$isMounted" != "" ]; then
			# Get Mount Point
			mountPoint=`$SUBSTR_CMD "$isMounted" "on " " type"`


		else
			# Fix ntfs drive issues
			echo "$PASSWORD" | sudo -S ntfsfix "$ntfsDrive"

			# Mount
			mountPoint="/media/ntfsdrive"
			echo "$PASSWORD" | sudo -S mkdir -p $mountPoint
			echo "$PASSWORD" | sudo -S mount "$ntfsDrive" "$mountPoint"

		fi

		echo "Mount Point: $mountPoint"


		# Backup Disk [tar]

		tmp="/media/tmp"
		echo "$PASSWORD" | sudo -S rm -rf $tmp
		echo "$PASSWORD" | sudo -S mkdir -p "$tmp"
		cd /
		echo "$PASSWORD" | sudo -S tar cf "$tmp/$macAddr-$dateTime.tar" "$mountPoint"

		# Unmount Disk
		echo "$PASSWORD" | sudo -S umount "$ntfsDrive" -f
		sleep 5s

		# Format as FAT32
		echo "$PASSWORD" | sudo -S mkfs.vfat -F 32 -n INEC $ntfsDrive
		sleep 5s

		echo "$PASSWORD" | sudo -S mkdir -p "$mountPoint"
		echo "$PASSWORD" | sudo -S mount "$ntfsDrive" "$mountPoint"


		# Restore data [tar]
		cd /
		echo "$PASSWORD" | sudo -S tar xf "$tmp/$macAddr-$dateTime.tar"
		echo "$PASSWORD" | sudo -S rm -rf $tmp



	fi
done

