#!/bin/bash

case "$1" in
	1)
		;;
	2)
		;;
	3)
		;;
	4)
		;;
	5)
		;;
	*)	echo "Invalid TSN port number"
		exit 1
		;;
esac

#As the PHY0 switches changed by monitor_link.sh, we cannot use the pytool here
#This tool monitors the LED state instead

declare -a led=( 0 0x80 0x20 0x08 0x02 0x80)
declare -a bank=(0  0    0    0    0    1)

BANK=${bank[$1]}
MASK=${led[$1]}


while true; do
	link=`i2cget -y 5 0x4d $BANK`
	led_on=`echo "puts [expr ($link & $MASK) == 0]" | tclsh`
	#echo "link=$link  MASK=$MASK  BANK=$BANK  led_on=$led_on"

	if [ "$led_on" == "1" ]; then
		echo "Link $1 is up"
		exit 0
	fi
	sleep 1
done
