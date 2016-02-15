#!/bin/bash
cd `dirname $0`
SCRIPTS_DIR=$PWD
#OPENVR_DIR=${SCRIPTS_DIR/scripts/}
OPENVR_DIR="/opt/openvr"

CHECK_REMOVABLE_SCRIPT="check_removable.sh"
MOUNT_ENC_SCRIPT="mount_backup.sh"
UNMOUNT_ENC_SCRIPT="unmount_backup.sh"
FP_BACKUP_SCRIPT="backup_fingerprints.sh"
FORMAT_SCRIPT="format_ntfs.sh"

LOCAL_BACKUP="$OPENVR_DIR/share/info"
SUBSTR_CMD="$SCRIPTS_DIR/scripts/substring"
TEMP_DIR="$OPENVR_DIR/tmp"

EXT_BK_DIR="OpenVR-BK"

DB_BK_DIR="$OPENVR_DIR/database"
BK_DATABASES="openvr"
BK_TABLES="registrations fp_fingerprints experiment_results duplicate_maps auto_scans registered_operators administrators"
BK_LOGFILE="backup-log.txt"








# FORMAT NTFS
#$SCRIPTS_DIR/$FORMAT_SCRIPT



macAddr=`ifconfig -a | grep 'eth' |awk -F: '/HWaddr/{print $3 $4 $5 $6 $7}' |tr -d \\n | tr -d [:blank:]| tr [:cntrl:] '_'`
#macAddr=${macAddr//:/}

dateTime=`date +%Y%m%d_%H.%M.%S`
prefix="INEC-INC-BK"
outFile="$DB_BK_DIR/$prefix-$macAddr-$dateTime.sql"
#echo $outFile;
#exit;
# Mount Encrypted Storage Box
$SCRIPTS_DIR/$MOUNT_ENC_SCRIPT

#echo "System MAC: $macAddr"
#mysqldump -u openvr --password=I\$0gSowLYazb4 -h 127.0.0.1 -P 30311 --complete-insert --databases "$BK_DATABASES" --tables $BK_TABLES | gzip > "$DB_BK_DIR/$prefix-$macAddr-$dateTime.sql.gz"

# mysqldump -u openvr --password=I\$0gSowLYazb4 -h 127.0.0.1 -P 30311 --complete-insert --databases "$BK_DATABASES" --tables $BK_TABLES | gzip > "$DB_BK_DIR/$prefix-$macAddr-$dateTime.sql.gz"
# registrations fp_fingerprints experiment_results duplicate_maps auto_scans registered_operators administrators

dbPassword="I\$0gSowLYazb4"
dbUser=openvr
dbHost=127.0.0.1
dbPort=30311
dbName=openvr

dumpPrefix="mysqldump -u $dbUser --password=$dbPassword -h $dbHost -P $dbPort $dbName --skip-opt --complete-insert --tables "

mqExePrefix="mysql -u $dbUser --password=$dbPassword -h $dbHost -P $dbPort $dbName  "

vinExtSql="SELECT id from bk_vins";

expExtSql="SELECT id from bk_exps";

createBkSql="CREATE TABLE IF NOT EXISTS bk_vins (  id varchar(40) NOT NULL,  PRIMARY KEY (id)) ENGINE=MyISAM DEFAULT CHARSET=latin1;";

createExpSql="CREATE TABLE IF NOT EXISTS bk_exps (  id varchar(40) NOT NULL,  PRIMARY KEY (id)) ENGINE=MyISAM DEFAULT CHARSET=latin1;";

fpExtSql="SELECT id from bk_vins";

$mqExePrefix -e "$createBkSql";

$mqExePrefix -e "$createExpSql";

$dumpPrefix registrations --where="id NOT IN($vinExtSql)" >> $outFile

$dumpPrefix fp_fingerprints --where="registration_id NOT IN($vinExtSql)" >> $outFile

$dumpPrefix experiment_results --where="id NOT IN($expExtSql)" >> $outFile

$dumpPrefix duplicate_maps --where="vin NOT IN($vinExtSql)" >> $outFile

$dumpPrefix auto_scans --where="vin NOT IN($vinExtSql)" >> $outFile

$dumpPrefix registered_operators administrators >> $outFile

#Run this last
$mqExePrefix -e " REPLACE INTO bk_vins ( SELECT id from registrations WHERE id NOT IN ($vinExtSql) )"

$mqExePrefix -e " REPLACE INTO bk_exps ( SELECT id from experiment_results WHERE id NOT IN ($expExtSql) )"

#experiment_results duplicate_maps auto_scans registered_operators administrators

gzip $outFile

# Sync With External Storage

#mkdir -p $TEMP_DIR;
$SCRIPTS_DIR/$CHECK_REMOVABLE_SCRIPT |

while read curdisk; do

curdisk=`echo $curdisk |tr [:blank:] '\\ '`
	echo "Backuping $curdisk";
	cd "$curdisk";
	mkdir -p "$curdisk/$EXT_BK_DIR/$macAddr"
	rsync -azvuL $DB_BK_DIR "$curdisk/$EXT_BK_DIR/$macAddr/"
done

$SCRIPTS_DIR/$FP_BACKUP_SCRIPT

# Unmount Encrypted Storage Box
$SCRIPTS_DIR/$UNMOUNT_ENC_SCRIPT
