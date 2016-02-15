#!/bin/bash

cd `dirname $0`
SCRIPTS_DIR=$PWD
OPENVR_DIR=${SCRIPTS_DIR/scripts/}

LOCAL_BACKUP="$OPENVR_DIR/share/info"
SUBSTR_CMD="$OPENVR_DIR/scripts/substring"

lpinfo --timeout 1 -v | grep usb
