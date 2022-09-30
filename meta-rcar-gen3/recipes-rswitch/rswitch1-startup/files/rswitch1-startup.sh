#!/bin/bash

## Observe errors by this scripts with
##   systemctl status vcstartup.service

# Get the board number from the hostname. It will be used to derive IP
# and MAC addresses
ecu=`echo $HOSTNAME | egrep -o '[[:digit:]]+$'`
echo "Run on ECU $ecu"

#Do not execute this script when there is no RSwitch1 in FPGA
if [ "`grep 1172e001 /proc/bus/pci/devices`" == "" ]; then
   echo "No RSwitch1 found"
   exit 0
fi

board=vc3
if [ "`grep vc2 /sys/firmware/devicetree/base/compatible`" != "" ]; then
    board=vc2
fi


# Default config
#TSN1_ROLE=master
#TSN2_ROLE=master
#TSN3_ROLE=master
#TSN4_ROLE=master
#TSN5_ROLE=master

TSN_IP_AND_MAC_FROM_HOTSTNAME=yes

FWD_ENGINE_CONFIG_FILE=/etc/rswitch1/l2_gptp_default.xml
FWD_ENGINE_TEMPLATE_FILE=/etc/rswitch1/tsngw_static_entry.template


# Load user config
DEFAULT_CONF_FILE=/etc/rswitch1/rswitch1-conf
if [ -f "$DEFAULT_CONF_FILE" ]; then
    . "$DEFAULT_CONF_FILE"
fi

if [ "$TSN_IP_AND_MAC_FROM_HOTSTNAME" == "yes" ]; then
    # Derive IP and MAC addresses from hostname
    TSNGW_IP=192.168.0.$ecu

    _ecu=000$ecu
    mac1=${_ecu: -4:2}
    mac2=${_ecu: -2}
    TSN_MAC_BASE=74:90:50:$mac1:$mac2:xx

    TSN0_MAC=${TSN_MAC_BASE:0:14}:00
    TSN1_MAC=${TSN_MAC_BASE:0:14}:01
    TSN2_MAC=${TSN_MAC_BASE:0:14}:02
    TSN3_MAC=${TSN_MAC_BASE:0:14}:03
    TSN4_MAC=${TSN_MAC_BASE:0:14}:04
    TSN5_MAC=${TSN_MAC_BASE:0:14}:05

    TSNGW_MAC=${TSN_MAC_BASE:0:14}:CC
else
    # Set IP and MAC addresses according to the values specified in
    # /etc/default/rswitch1-conf. If some of them are not set in the
    # config file, use the default values below
    : "${TSN0_MAC:=74:90:50:00:00:00}"
    : "${TSN1_MAC:=74:90:50:00:00:01}"
    : "${TSN2_MAC:=74:90:50:00:00:02}"
    : "${TSN3_MAC:=74:90:50:00:00:03}"
    : "${TSN4_MAC:=74:90:50:00:00:04}"
    : "${TSN5_MAC:=74:90:50:00:00:05}"

    : "${TSNGW_MAC:=74:90:50:00:00:CC}"

    : "${TSNGW_IP:=192.168.0.99}"
fi


case "$1" in
start)
    ##----- Section 1: Setup RSwitch -----

    ## Configure PHYs and R-Car GPIO pins for PHY switching
    ## to allow seting PHY Master or Slave mode by rswitchtool
    /etc/rswitch1/phyinit.sh || exit 1

    ## load kernel module for the RSwitch
    modprobe rswitch_eth || exit 1

    ## Initialise the CPU side switch interface
    ifconfig tsngw hw ether $TSNGW_MAC
    ifconfig tsngw up $TSNGW_IP/24 || exit 1

    ## Initialise all interfaces
    ##   no IP address allowed
    ##   TSN0 is to internal MCU boards, use only if F1K board is present in box
    ifconfig tsn0 hw ether $TSN0_MAC || exit 1
    ifconfig tsn0 up

    ifconfig tsn1 hw ether $TSN1_MAC || exit 1
    ifconfig tsn1 up

    ifconfig tsn2 hw ether $TSN2_MAC || exit 1
    ifconfig tsn2 up

    ifconfig tsn3 hw ether $TSN3_MAC || exit 1
    ifconfig tsn3 up

    ## The additional 2 interfaces existing only on VC2 main board
    if [ "$board" == "vc2" ]; then
        ifconfig tsn4 hw ether $TSN4_MAC || exit 1
        ifconfig tsn4 up

        ifconfig tsn5 hw ether $TSN5_MAC || exit 1
        ifconfig tsn5 up
    fi

    ## switch status led (LED8) to green
    echo "0" > /sys/class/leds/led8\:orange/brightness
    echo "255" > /sys/class/leds/led8\:green/brightness


    ##----- Section 2: Load the switch configuration  -----

    ## Basic configuration - includes Layer 2 routing and gPTP function
    rswitch1tool --configure=fwd -f /etc/rswitch1/l2_gptp_default.xml || exit 1

    ## Entry for R-Car CPU via tsngw
    sed s/vcXX/$ecu/g $FWD_ENGINE_TEMPLATE_FILE > /tmp/tsngw_static_entry.xml || exit 1
    rswitch1tool --configure=fwd -f /tmp/tsngw_static_entry.xml || exit 1
    rm -f /tmp/tsngw_static_entry.xml
    ;;

stop)
    ifconfig tsn4 down
    ifconfig tsn5 down
    if [ -d /sys/class/net/tsn6 ]; then
        ifconfig tsn6 down
    fi
    if [ -d /sys/class/net/eth1 ]; then
        ifconfig eth1 down
    fi
    if [ -d /sys/class/net/tsn0 ]; then
        ifconfig tsn0 down
    fi
    ifconfig tsn7 down
    ifconfig tsngw down

    modprobe -r rswitch_eth

    ## switch status led (LED8) to yellow
    echo "0" > /sys/class/leds/led8\:green/brightness
    echo "255" > /sys/class/leds/led8\:orange/brightness
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
