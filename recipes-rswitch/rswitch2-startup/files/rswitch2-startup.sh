#!/bin/bash

## Observe errors by this scripts with
##   systemctl status rswitch2-startup.service

# Get the board number from the hostname. It will be used to derive IP
# and MAC addresses
ecu=`echo $HOSTNAME | egrep -o '[[:digit:]]+$'`
echo "Run on ECU $ecu"


# Default config
TSN4_SPEED=1000
TSN5_SPEED=1000
TSN7_SPEED=1000

TSN0_SPEED=100
TSN6_SPEED=100
ETH1_SPEED=100

TSN4_ROLE=master
TSN5_ROLE=master
TSN6_ROLE=master
TSN7_ROLE=master

TSN_IP_AND_MAC_FROM_HOTSTNAME=yes
FWD_ENGINE_CONFIG_FILE=/etc/rswitch2/fwd-default.xml


# Load user config
DEFAULT_CONF_FILE=/etc/rswitch2/rswitch2.conf
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
    ETH1_MAC=${TSN_MAC_BASE:0:14}:01
    TSN4_MAC=${TSN_MAC_BASE:0:14}:04
    TSN5_MAC=${TSN_MAC_BASE:0:14}:05
    TSN6_MAC=${TSN_MAC_BASE:0:14}:06
    TSN7_MAC=${TSN_MAC_BASE:0:14}:07
    TSNGW_MAC=${TSN_MAC_BASE:0:14}:CC
else
    # Set IP and MAC addresses according to the values specified in
    # /etc/default/rswitch2-conf. If some of them are not set in the
    # config file, use the default values below
    : "${TSN0_MAC:=74:90:50:00:00:10}"
    : "${ETH1_MAC:=74:90:50:00:00:11}"
    : "${TSN4_MAC:=74:90:50:00:00:14}"
    : "${TSN5_MAC:=74:90:50:00:00:15}"
    : "${TSN6_MAC:=74:90:50:00:00:16}"
    : "${TSN7_MAC:=74:90:50:00:00:17}"
    : "${TSNGW_MAC:=74:90:50:00:00:CC}"
    : "${TSNGW_IP:=192.168.0.1}"
fi


function phyRoleToOnOff {
    if [ "x$1" == "xmaster" ]; then
	echo "on"
    else
	echo "off"
    fi
}


case "$1" in
start)
    ##----- Section 1: Setup RSwitch -----

    ## load kernel module for the RSwitch
    modprobe rswitch2 || exit 1

    ## Initialise the CPU side switch interface
    ifconfig tsngw hw ether $TSNGW_MAC
    ifconfig tsngw up $TSNGW_IP/24 || exit 1

    ## Initialise all interfaces
    ##   no IP address allowed
    ##   to change in Slave mode, set master-phy off
    ##   Interface tsn7 is only allowed with 100 Mbps, can be switched to Eth1 and MCU
    ifconfig tsn4 hw ether $TSN4_MAC
    ifconfig tsn4 up || exit 1
    ethtool --set-priv-flags tsn4 master-phy $(phyRoleToOnOff $TSN4_ROLE)
    ethtool -s tsn4 speed $TSN4_SPEED

    ifconfig tsn5 hw ether $TSN5_MAC
    ifconfig tsn5 up || exit 1
    ethtool --set-priv-flags tsn5 master-phy $(phyRoleToOnOff $TSN5_ROLE)
    ethtool -s tsn5 speed $TSN5_SPEED

    if [ -d /sys/class/net/tsn6 ]; then
        ifconfig tsn6 hw ether $TSN6_MAC
        ifconfig tsn6 up || exit 1
        ethtool --set-priv-flags tsn6 master-phy $(phyRoleToOnOff $TSN6_ROLE)
        ethtool -s tsn6 speed $TSN6_SPEED
    elif [ -d /sys/class/net/eth1 ]; then
        ifconfig eth1 hw ether $ETH1_MAC
        ifconfig eth1 up || exit 1
        ethtool -s eth1 speed $ETH1_SPEED duplex full autoneg on
    else
        ifconfig tsn0 hw ether $TSN0_MAC
        ifconfig tsn0 up || exit 1
        ethtool -s tsn0 speed $TSN0_SPEED
    fi

    ifconfig tsn7 hw ether $TSN7_MAC
    ifconfig tsn7 up || exit 1
    ethtool --set-priv-flags tsn7 master-phy $(phyRoleToOnOff $TSN7_ROLE)
    ethtool -s tsn7 speed $TSN7_SPEED


    ##----- Section 2: Load the switch configuration  -----

    ## Example: L2 switch broadcast routing and learning
    rswitch2tool -c fwd -f "$FWD_ENGINE_CONFIG_FILE" || exit 1


    ##----- Section 3: Update the LED  -----

    ## switch status led (LED8) to green
    echo "0" > /sys/class/leds/led8\:orange/brightness
    echo "255" > /sys/class/leds/led8\:green/brightness

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

    rmmod rswitch2

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
