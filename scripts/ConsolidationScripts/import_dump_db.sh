#!/bin/bash
hostName=`hostname`

PASS="0yVcBpu.Ce3g\$"
HOST=" -h 127.0.0.1 "
	PORT="30311"
	VERBOSE=""
	PASSWORD=$PASS

echo "Consolidating data ..."
mysqldump -u root --password=$PASS -P 30312 -h 127.0.0.1 --no-create-info --skip-comment $VERBOSE --insert-ignore --skip-add-drop-table --add-locks --complete-insert --quick --extended-insert openvr | mysql -u root --password=$PASSWORD $HOST -P $PORT openvr

