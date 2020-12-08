#!/bin/bash

# Default config
PTP4L_INTERFACES=
PTP4L_CONFIG_FILE=/etc/ptp4l/ptp4l_slave.cfg


# Load user config
DEFAULT_CONF_FILE=/etc/ptp4l/ptp4l.conf
if [ -f "$DEFAULT_CONF_FILE" ]; then
	. "$DEFAULT_CONF_FILE"
fi

#sanity check
if [ "$PTP4L_INTERFACES" == "" ]; then
	echo "No ptp4l interfaces configured. ptp4l service not started."
	exit 0
fi

if [ ! -f "$PTP4L_CONFIG_FILE" ]; then
	echo "No ptp4l \"$PTP4L_CONFIG_FILE\" configuration file found"
	exit 1
fi



case "$1" in
start)
	interfaces=
	for i in $PTP4L_INTERFACES; do
		interfaces+="-i$i "
	done

	## start ptp4l application
	echo "Start 'ptp4l $interfaces -f $PTP4L_CONFIG_FILE -m'"
	ptp4l $interfaces -f $PTP4L_CONFIG_FILE -m &

	## switch status led (LED7) to yellow (started)
	echo "0" > /sys/class/leds/led7\:green/brightness
	echo "255" > /sys/class/leds/led7\:orange/brightness
	;;

stop)
	## stop ptp4l deamon
	killall ptp4l

	## switch status led (LED7) to off
	echo "0" > /sys/class/leds/led7\:green/brightness
	echo "0" > /sys/class/leds/led7\:orange/brightness
	;;

force-reload|restart)
	$0 stop
	$0 start
	;;

*)
    echo "Usage: $0 {start|stop}"
    exit 1
    ;;
esac

exit 0
