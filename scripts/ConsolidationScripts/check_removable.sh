#!/bin/bash

cd `dirname $0`
SCRIPTS_DIR=$PWD
OPENVR_DIR=${SCRIPTS_DIR/scripts/}

LOCAL_BACKUP="$OPENVR_DIR/share/info"
SUBSTR_CMD="$OPENVR_DIR/scripts/substring"





mount | grep -e 's[dr]' |
while read disk; do
	curdisk=`$SUBSTR_CMD "$disk" "on " " type"`
	if [ "$curdisk" != "/" ]; then
#		if [ "$curdisk" != "/home" ]; then
			echo "$curdisk";
#		fi
	fi
done

