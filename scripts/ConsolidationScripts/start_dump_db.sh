#!/bin/bash

if [ "$1" == "" ]; then
	echo "Invalid Dump Dir";
	exit 2;
fi

OPENVR_DUMPS="/openvr-station/dumps"
MYSQL_CONFIG="/openvr-station/opt/libd/var/lib/mysql"
dumpDir=$1

echo "Linking MySQL configuration files ..."
cd "$dumpDir"
ln -sf "$MYSQL_CONFIG"/* ./

$dumpDir/stop_db.sh

echo "Starting MySQL ..."
$dumpDir/start_db.sh



