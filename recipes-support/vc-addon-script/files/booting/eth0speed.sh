#!/bin/bash

# Configure R-CAR debug port for 100Mbps or 1Gbps (= 1000Mbps)
speedmode=$1   # set variable mode to desired Mbps rate

phyId=`phytool read eth0/6/3`
#echo @6 $phyId

#check if this is PHY addressing we have on VC2
if [ $phyId == "0x1622" ]; then
    echo "Detected VC2"
    PHY=eth0/6
    GBPS=0x0140
    MBPS=0x2100

    CLKSKEW=0x03e0


#check if this is PHY addressing we have on VC3
else 
    if [ $phyId == "0xffff" ]; then
        #GP0.1   gpio497
        if [ ! -e /sys/class/gpio/gpio497/direction ]; then
            echo 497 > /sys/class/gpio/export
        fi
        echo out > /sys/class/gpio/gpio497/direction
        echo 1   > /sys/class/gpio/gpio497/value  
    fi
    phyId=`phytool read eth0/5/3`
    #echo @5 $phyId
    if [ $phyId == "0x1622" ]; then
        echo "Detected VC3"
    else
        echo "No valid PHY for eth0 detected"
        exit 1
    fi
    PHY=eth0/5
    GBPS=0x1140
    MBPS=0x2100

    CLKSKEW=0x03ef
fi
#echo $PHY


## Adapt RGMII clock pad skew
phytool write ${PHY}/0x0D 0x0002
phytool write ${PHY}/0x0E 0x0008
phytool write ${PHY}/0x0D 0x4002
phytool write ${PHY}/0x0E ${CLKSKEW}


if [ "$speedmode" == "100" ]; then
        echo Configuring R-CAR AVB-MAC for 100 Mbps
        ethtool -s eth0 speed 100 duplex full autoneg off
        ## Disable auto negotiation of PHY and fix to 100Mbps
        phytool write $PHY/0 $MBPS
elif [ "$speedmode" == "1000" ]; then
        echo Configuring R-CAR AVB-MAC for 1 Gbps
        ethtool -s eth0 speed 1000 duplex full autoneg off
        ## Disable auto negotiation of PHY and fix to 1000Mbps
        phytool write $PHY/0 $GBPS
else 
    echo No valid speed given. Use 100 or 1000
    echo $0 speed
fi
