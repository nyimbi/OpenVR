#!/bin/bash

cd `dirname $0`
SCRIPTS_SRC_DIR=$PWD
SCRIPTS_DIR=${SCRIPTS_SRC_DIR/src\//}
OPENVR_DIR=${SCRIPTS_DIR/scripts/} 	


echo "SCRIPTS DIR: $SCRIPTS_DIR"
echo "OPENVR_DIR: $OPENVR_DIR"

cd $SCRIPTS_SRC_DIR
rm *.c
ls $SCRIPTS_SRC_DIR |
while read script_file; do
	shc -r -f $script_file
	mv "$script_file.x" "$SCRIPTS_DIR/$script_file"
done

rm *.c
