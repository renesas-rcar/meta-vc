#!/bin/sh
#
#Generic Boot script for the VC2
#This file shall be copied to /etc/init.d and is not intended to be edited.
#Use "update-rc.d -f vc2startup.sh defaults" to register this file for startup
#
#Calls specific startup scripts in /home/root folder
#  startup_all.sh {start|stop|restart|...}
#  startup_$HOST.sh {start|stop|restart|...}
#
#Use these startup scripts for the box specific startup

echo "VC2 environment start"
path=/home/root/vc2/booting
script1=$path/startup_all.sh
script2=$path/startup_$HOSTNAME.sh

if [ -f $script1 ]; then
	#echo "Found $script1"
	$script1 $1 || exit 1
else
	echo "No generic startup script $script1"
fi

if [ -f $script2 ]; then
        #echo "Found $script2"
	$script2 $1 || exit 1
else
    	echo "No box specific startup script $script2"
fi

exit 0

