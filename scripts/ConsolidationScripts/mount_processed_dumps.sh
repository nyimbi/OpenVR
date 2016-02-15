#!/bin/bash

logFile="/tmp/openvr_merge_log.log"
echo "" >$logFile
logMessage() {

echo $1 >> $logFile
echo $1

}

processedDir="/openvr-station/processed-dumps"
targetRoot="/opt/libd/var/lib/mysql"
mountPrefix="dmp_"
#mountPrefix=""

hostName=`hostname`
SYSTEM_PASSWORD="password"

#I kinda need to umount all mounted dumps....

#procMounted=`mount |grep "$processedDir"`

#if [ -n "$procMounted" ];then #some stuff are mounted... so unmount all!#
	#echo $SYSTEM_PASSWORD|umount $processedDir/*
#fi;


for processedDump in `ls -p $processedDir |grep "/"`
do

	dumpName=`dirname ${processedDump}/../`
	logMessage "Checking if $dumpName is already mounted";
	proposedMountPoint="${targetRoot}/${mountPrefix}${dumpName}"
	dumpAlreadyMounted=`mount |grep "$proposedMountPoint"`
	if [ ! -n "$dumpAlreadyMounted" ];then # No it is not!
		logMessage "Nope!"
		#Then try to mount it abi ?
		if [ ! -e "$proposedMountPoint" ];then
			logMessage "Creating Mount Point"
			mkdir "$proposedMountPoint"
		fi

		echo $SYSTEM_PASSWORD|sudo -S mount --bind "${processedDir}/${processedDump}/openvr" "${proposedMountPoint}"

		echo $SYSTEM_PASSWORD|sudo -S chown 1000:1000 "$proposedMountPoint" -R

	else #Yes it is :(
		logMessage "Yes. So skipping $dumpName"

	fi;

	logMessage "$dumpName mounted on $proposedMountPoint"
done;



