#!/bin/bash

cd `dirname $0`
SCRIPTS_SRC_DIR=$PWD
SCRIPTS_DIR=${SCRIPTS_SRC_DIR/src\//}
OPENVR_DIR=${SCRIPTS_DIR/scripts/}
NON_COMPILE_SCRIPTS="$SCRIPTS_SRC_DIR/non_compile" 	

./compile_scripts.sh
cp $NON_COMPILE_SCRIPTS/* "$SCRIPTS_DIR"

