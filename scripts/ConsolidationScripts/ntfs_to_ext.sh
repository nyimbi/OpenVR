#!/bin/bash
logFile="/tmp/openvr_dump_copy.log"
echo "" >$logFile
logMessage(){

echo $1 >> $logFile
echo $1

}


hostName=`hostname`
	PASSWORD="password"

aCount=0
echo "$PASSWORD" | sudo -S fdisk -l | grep [NTFS,FAT]|grep -v "sda"|grep "dev"|grep -v "Disk" |awk '{ print $1 }'|
while read ntfsDrive; do
echo "-----------------------------------"
	#Double check if it's NTFS
	isNTFS=`echo "$PASSWORD" | sudo -S parted $ntfsDrive print | grep -i [NTFS,FAT]`
	echo $ntfsDrive
	#echo $isNTFS
	#PASSWORD="sad"
	#continue;
	if [ "$isNTFS" != "" ]; then
		logMessage "$ntfsDrive is an unsupported PARTITION"
		# Check if disk is mounted.
		isMounted=`mount | grep $ntfsDrive`
		if [ "$isMounted" != "" ]; then
			logMessage "$ntfsDrive is mounted. Will attempt to unmount"
			echo "$PASSWORD" | sudo -S umount "$ntfsDrive"
			isStillMounted=`mount | grep $ntfsDrive`
			if [ "$isStillMounted" != "" ]; then
				logMessage "Still Mounted! Unable to unmount $ntfsDrive. Exiting Process"
				exit;
			fi
		fi

		timeNow=`date "+%d%m%Y_%H%M%S"`

		newLabel="DUMP${aCount}_${timeNow}"

		logMessage "Attempting to format $ntfsDrive as ext4 with label $newLabel"

		aCount=$((aCount +1))

		formatResult=`echo "$PASSWORD" | sudo -S mkfs.ext4 -L "$newLabel" -j "$ntfsDrive"`

		logMessage $formatResult

		logMessage "$ntfsDrive has been formatted as ext4 and labelled as $newLabel"

		logMessage "Now attempting to remount as ext4"

		MOUNT_DIR="/media/$newLabel";
		echo "$PASSWORD" | sudo -S mkdir -p $MOUNT_DIR
		if [ ! -e "$MOUNT_DIR" ];then

			logMessage "There was an error creating the mountpoint $MOUNT_DIR. Please eject device and try running the process again"

		fi
		echo "$PASSWORD" | sudo -S mount "$ntfsDrive" "$MOUNT_DIR"

		logMessage "$ntfsDrive Mounted!"

		logMessage "Done with $ntfsDrive. Looking for another drive"
	fi

done
echo "-----------------------------------"
