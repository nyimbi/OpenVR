#!/bin/bash

#rsync -avuL --rsh='sshpass -p "password" ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no  ' `pwd` openvr@localhost:~/
macAddress=$1
dbFilesToCopy=$2
serverLogin=$3

localDir="/tmp/OpenVR-DATA/dumps/$macAddress/"

rm -rf $localDir

mkdir -p $localDir

mkdir -p $localDir/openvr

destDir="/openvr-station/dumps/"

dbDestinationDir="$destDir/$macAddress/openvr"

tarFile="$localDir/stored_minutiae.tar"

cd /opt/openvr
echo "TARING"
tar -zcf $tarFile stored_minutiae

result=`rsync -auL --rsh='sshpass -p "password" ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no  ' $localDir/../  $serverLogin:$destDir `

filesToCopy=""

for i in $(echo $dbFilesToCopy | tr "," "\n")
do
  # process
  filesToCopy+="$i "
done

cd /opt/libd/var/lib/mysql/openvr

result2=`rsync -avuL --rsh='sshpass -p "password" ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no ' $filesToCopy  $serverLogin:$dbDestinationDir`

totalRegs=`mysql -u root --password="0yVcBpu.Ce3g\$" -h 127.0.0.1 -P 30311 openvr -e "SELECT COUNT(*) total_units from registrations" |tr -d "total_units"|tr -d "\n"`

outputString="<?xml version=\"1.0\"?><DUMP RECORDS='$totalRegs' MAC='$macAddress' />"

echo -e $outputString > $localDir/finished.xml

result2=`rsync -avuL --rsh='sshpass -p "password" ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no ' $localDir/finished.xml  $serverLogin:$destDir/$macAddress`
