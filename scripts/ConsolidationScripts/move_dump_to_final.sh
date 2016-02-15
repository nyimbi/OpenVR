#!/bin/bash

dumpDir="/openvr-station/processed-dumps/"

finishedDumpPath="/openvr-station/finished-dumps"

targetDump=$1

if [ ! -n "$targetDump" ];then
echo "No directory specified"
exit;
fi

if [ ! -e "${dumpDir}${targetDump}" ];then

echo "Directory not found"
exit;
fi


hostName=`hostname`
	PASSWORD="password"

mysqladmin -u root -P 30311 -h 127.0.0.1 --password=0yVcBpu.Ce3g\$ refresh
#LAZY MANS WAY!!!!
echo $PASSWORD | sudo -S umount /opt/libd/var/lib/mysql/dmp_${targetDump}


mkdir -p $finishedDumpPath

echo "Moving $dumpDir ..."

cd $dumpDir;

rm -rf "${finishedDumpPath}/${targetDump}/"

mv "$targetDump" $finishedDumpPath;
echo "Dump $targetDump Moved to Finished Dump"
