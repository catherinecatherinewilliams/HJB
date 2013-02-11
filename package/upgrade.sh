#!/bin/sh

#set env
BACKUP_DIR=/test/backup
RUN_BIN=btscan.arm
FIFONAME=/test/btscan/bin/fifo
BIN_DIR=/test/btscan/bin
SCRIPT=run.sh
PATCH_DIR=$1
PrepareBackupPath()
{
	echo Creating backup dir...
	if [ -d "${BACKUP_DIR}.bak" ]; then
	rm -rf ${BACKUP_DIR}.bak
	echo rmbackupdir
	fi
	if [ -d "${BACKUP_DIR}" ]; then
	mv -f "${BACKUP_DIR}" "${BACKUP_DIR}.bak"
	echo mvbackup
	fi
	mkdir -p ${BACKUP_DIR}
	echo mkdirbackupdir
	return 0
}

backup()
{
	echo backupstart
	cp -rf $BIN_DIR/* $BACKUP_DIR
	echo copy
	if [ -f "$BACKUP_DIR/$RUN_BIN" ]; then
	echo backup successfully
	return 1
	else
	echo backup failed
	return 0
	fi 
}
upgrade()
{
	echo upgrade start
	cp -rf $PATCH_DIR/* $BIN_DIR/
	chmod +x $BIN_DIR/*
	$BIN_DIR/$SCRIPT
	echo upgrade successful
        
}
killprocess()
{
	echo killprocessstart
	ID=`ps  | grep "$RUN_BIN" | grep -v "$0" | grep -v "grep" | awk '{print $2}'`
	IDNAME=`ps | grep "$RUN_BIN" | grep -v "$0" | grep -v "grep" | awk '{print $8}'`
	echo process ID   list: $ID
	echo process NAME list: $IDNAME
	echo "--start kill-------------"
	#for id in $ID
	#do
	kill -9 $ID
	echo "killed $id"
	#done
	echo "--kill_process finished!-"
	return 0
}
checkFIFOexist()
{
	echo "Properties for $FIFONAME:"

	if [ -f $FIFONAME ]; then

	echo "Size is $(ls -lh $FIFONAME | awk '{ print $5 }')"

	echo "Type is $(file $FIFONAME | cut -d":" -f2 -)"
	return 1
	else
	echo "File does not exist."
	return 0

	fi
}
rollback()
{
	echo rollback start
	cp -rf $BACKUP_DIR/* $BIN_DIR/
	chmod +x $BIN_DIR/*
	$BIN_DIR/$SCRIPT
        echo rollback successful

	
}
if [ ! -n "$1" ] ;then  
echo "You need specify the upgrade package directory"  
exit
else  
echo "the upgrade pcakge directory is $1"  
fi 
PrepareBackupPath
backup
killprocess
upgrade
echo sleep 
sleep 1
if [ ! -n "$INQUIRY_LEN" ] ;then  
sleep 5
else
sleep $INQUIRY_LEN
checkFIFOexist
if [ -f $FIFONAME ]; then
echo check finished
rm -f $FIFONAME
else
killprocess
rollback
fi

