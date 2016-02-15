#!/bin/bash

cd `dirname $0`
SCRIPTS_DIR=$PWD
OPENVR_DIR=${SCRIPTS_DIR/scripts/}

LOCAL_BACKUP="$OPENVR_DIR/share/info"
SUBSTR_CMD="$OPENVR_DIR/scripts/substring"

#
#
	#exit;
#fi

mount | grep sd |
while read disk; do
	curdisk=`$SUBSTR_CMD "$disk" "on " " type"`
	if [ "$curdisk" != "/"  ] && [ "$curdisk" != "/home" ] &&  [ "$curdisk" != "/backups" ]; then
		if [ "$curdisk" != "/backups" ]; then
			echo "$curdisk";
		fi
	fi
done

