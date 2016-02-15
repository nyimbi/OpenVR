#!/bin/bash

if [ ! -n "$1" ]; then
	echo "Empty Directory Provided";
	exit;
fi
ROOT_DIR=$1
ENC_DIR="$ROOT_DIR/.OpenVR-ENC"
UNENC_DIR="$ROOT_DIR/OpenVR-DATA"

fusermount -u $UNENC_DIR 
exit;
