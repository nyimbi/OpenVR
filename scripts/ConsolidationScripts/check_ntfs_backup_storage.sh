#!/bin/bash

cd `dirname $0`
SCRIPTS_DIR=$PWD
OPENVR_DIR=${SCRIPTS_DIR/scripts/}

LOCAL_BACKUP="$OPENVR_DIR/share/info"
SUBSTR_CMD="$OPENVR_DIR/scripts/substring"




echo "$PASSWD" | sudo -S fdisk -l | grep NTFS | cut -d " " -f1 |
while read curdisk; do
	#curdisk=`$SUBSTR_CMD "$disk" "on " " type"`
	if [ "$curdisk" != "/" ]; then
		if [ "$curdisk" != "/backups" ]; then
			echo "$curdisk";
		fi
	fi
done


