#!/bin/bash

dumpDir="/openvr-station/processed-dumps/"
finishedDumpPath="/openvr-station/finished-dumps"




hostName=`hostname`
	PASSWORD="password"

mysqladmin -u root -P 30311 -h 127.0.0.1 --password=0yVcBpu.Ce3g\$ refresh
#LAZY MANS WAY!!!!
echo $PASSWORD | sudo -S umount /opt/libd/var/lib/mysql/dmp_*


mkdir -p $finishedDumpPath

echo "Moving $dumpDir ..."

cd $dumpDir;
for i in `ls`
do
echo $i
mv "$i" $finishedDumpPath;
done
echo "Dumps Moved to Finished Dumps"
