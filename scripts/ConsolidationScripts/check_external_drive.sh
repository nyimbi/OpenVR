#!/bin/bash

cd `dirname $0`
SCRIPTS_DIR=$PWD
OPENVR_DIR=${SCRIPTS_DIR/scripts/}
RETURN_PARTITION_FORMAT=$1
LOCAL_BACKUP="$OPENVR_DIR/share/info"
SUBSTR_CMD="$OPENVR_DIR/scripts/substring"

#REMEMBER TO ENABLE THIS BACK!
#
#

#discoveredPartition="c"
mount | grep -e 'sd' | grep -v 'sda' |
while read disk; do
	curdisk=`$SUBSTR_CMD "$disk" "on " " type"`
	if [ "$curdisk" != "/"  ] && [ "$curdisk" != "/home" ] &&  [ "$curdisk" != "/backups" ]; then
			discoveredPartition=$curdisk;
			if [ ! -n "$RETURN_PARTITION_FORMAT" ];then
			echo $curdisk
			exit;
			fi
			partitionFormat=`mount|grep "$curdisk" | awk '{ print $5 }'`

			echo "${curdisk}_____${partitionFormat}";
			exit;
			break;	#to force to first external drive
	fi
done
exit;


#mkfs.ntfs /dev/sdc1
