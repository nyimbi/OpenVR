#!/bin/bash

#Specify directories which contain the database
dirsToBackup="/opt/libd/var/lib/mysql/openvr /opt/libd/var/lib/mysql/afisdb"

#Check disk spaced used by files to back up
du -hs $dirsToBackup

#Generate a filename for the backup based on the system name and time.
#this ensures that this backup does not overwrite any other
outputFile=`hostname`"_"`date +"%m-%d-%Y_%H.%M.%S"`"_backup.tar"

echo "Press Enter to Begin the Backup Process. Or CTRL+C to Cancel"

read

#backup the files
tar -cvf $outputFile $dirsToBackup

#check  the backup file created
ls -lh $outputFile

#alert user
echo "Backup Completed - $outputFile"

zenity --info --text "Backup Completed - $outputFile"