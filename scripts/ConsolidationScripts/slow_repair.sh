#!/bin/sh


OPT_DIR="/opt/libd/var/lib/mysql/openvr"
FP_FILE_MYI="$OPT_DIR/fp_fingerprints.MYI"
FP_FILE_MYD="$OPT_DIR/fp_fingerprints.MYD"
FP_FILE_FRM="$OPT_DIR/fp_fingerprints.frm"
MYISAMCHK_CMD="myisamchk --force --safe-recover -vvv"


showMessage(){

zenity --info --text "$1"
  
}

showError(){
	
	zenity --error --title "Error Detected" --text "$1"
	
}

if [ ! -e $FP_FILE_MYI ];then

	showError "Unable to find $FP_FILE_MYI";
	return;

fi


if [ ! -e $FP_FILE_MYD ];then

	showError "Unable to find $FP_FILE_MYD";
	
	return;

fi


if [ ! -e $FP_FILE_FRM ];then

	showError "Unable to find $FP_FILE_FRM";
	
	return;

fi




showMessage "The database tables have been found.\n In order to repair corrupted tables successfully, \na large amount of hard disk space must be available on an external disk.\n"
showMessage "Connect the External Hard Drive you will be using as the temporary place to store the files"
sleep 0;
showMessage "Select the External Hard Drive you will be using as the temporary place to store the files"
a=0

DIR_TEMP_HDD=`zenity --file-selection --title="Select the Root Directory of the External Hard Drive to Use" --directory --filename=/media/`



case $? in
			 0)
					showMessage "\"$DIR_TEMP_HDD\" selected.";;
			 1)
					showError "No directory selected.";
					exit;;
			-1)
					showError "No directory selected.";
					exit;;
esac

#AVAILABLE_SPACE=`df "$DIR_TEMP_HDD"|awk '{ print $4 }'|tail -n 1`

#showMessage "$AVAILABLE_SPACE"

REPAIR_DIR="${DIR_TEMP_HDD}/repairs/"

rm -rf $REPAIR_DIR/*

currentUser=`whoami`

if [ "$currentUser" = "openvr" ];then

	SYSTEM_PASSWORD="password"

else

	SYSTEM_PASSWORD="';/.['"
fi


echo $SYSTEM_PASSWORD| sudo -S mkdir "$REPAIR_DIR"

echo $SYSTEM_PASSWORD | sudo -S chown `whoami`:`whoami` "$REPAIR_DIR"

 
(
        echo "10" ; cp -v $FP_FILE_MYI "$REPAIR_DIR";
        echo "# Copying 1 of 3 Files" ; sleep 1
        echo "20" ; sleep 1
        echo "# Copying 2 of 3 Files" ; sleep 1
        echo "30" ; cp -v $FP_FILE_MYD "$REPAIR_DIR";
        echo "Copying 3 of 3 Files" ; sleep 1
        echo "60" ; cp -v $FP_FILE_FRM "$REPAIR_DIR";
        echo "# Done Copying" ; sleep 1
        echo "100" ; sleep 0
        echo "Corrupt tables have been copied. Will now try to repair each one";sleep 1
        ) |
        zenity --progress \
          --title="Database Repair" \
          --text="Copying Out Corrupt Tables..." \
          --percentage=0 --auto-close

        if [ "$?" = -1 ] ; then
                zenity --error \
                  --text="Process Cancelled."
                  exit;
        fi


 
CORR_FP_FILE_MYI="$REPAIR_DIR/fp_fingerprints.MYI"
CORR_FP_FILE_MYD="$REPAIR_DIR/fp_fingerprints.MYD"
CORR_FP_FILE_FRM="$REPAIR_DIR/fp_fingerprints.frm"


if [ ! -e $CORR_FP_FILE_MYI ];then

	showError "Unable to find $CORR_FP_FILE_MYD";
	return;

fi


if [ ! -e $CORR_FP_FILE_MYD ];then

	showError "Unable to find $CORR_FP_FILE_MYD";
	
	return;

fi


if [ ! -e $CORR_FP_FILE_FRM ];then

	showError "Unable to find $CORR_FP_FILE_FRM";
	
	return;

fi




cd "$REPAIR_DIR";

$MYISAMCHK_CMD $CORR_FP_FILE_MYI | tee /tmp/repair.log| zenity --progress --title "Repairing Database" --text "This process will take a long time" --pulsate --auto-close


zenity --info --text "Repair Finished. Will now Copy back" &


(
        echo "10" ; cp -v $CORR_FP_FILE_MYI "$OPT_DIR";
        echo "# Copying 1 of 3 Files" ; sleep 1
        echo "20" ; sleep 1
        echo "# Copying 2 of 3 Files" ; sleep 1
        echo "30" ; cp -v $CORR_FP_FILE_MYD "$OPT_DIR";
        echo "Copying 3 of 3 Files" ; sleep 1
        echo "60" ; cp -v $CORR_FP_FILE_FRM "$OPT_DIR";
        echo "# Done Copying" ; sleep 1
        echo "100" ; sleep 1
        ) |
        zenity --progress \
          --title="Database Repair" \
          --text="Restoring Corrupt Tables..." \
          --percentage=0

        if [ "$?" = -1 ] ; then
                zenity --error \
                  --text="Process Cancelled."
                  exit;
        fi

showMessage "Success! Please restart the machine."
