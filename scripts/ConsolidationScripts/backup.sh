#!/bin/bash

cd `dirname $0`
SCRIPTS_DIR=$PWD
OPENVR_DIR=${SCRIPTS_DIR/scripts/}
BK_FP="backup_fingerprints.sh"
BK_DB="backup_database.sh"
IMPORT_BK="import_backups.sh"
FORMAT_SCRIPT="format_ntfs.sh"


# FORMAT NTFS
$SCRIPTS_DIR/$FORMAT_SCRIPT

# Backup Database
$SCRIPTS_DIR/$BK_DB


# Backup Fingerprints
$SCRIPTS_DIR/$BK_FP


# Import Backups!
#$SCRIPTS_DIR/$IMPORT_BK
