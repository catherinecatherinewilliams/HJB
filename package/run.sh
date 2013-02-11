#! /bin/sh
export SERVER_ADDRESS=127.0.0.1
export LOGFILE=/test/btscan/btscan.out
export INQUIRY_LEN=5
export LOG_MAX_LINES=20000
hciconfig hci0 up
/test/btscan/bin/btscan.arm
echo "programme launched, check btscanlog.out for result"
