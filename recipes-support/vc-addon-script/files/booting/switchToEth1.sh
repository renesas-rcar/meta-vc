#!/bin/bash

# Configure Eth2 PHY (connected to switch in FPGA) for 100Mbps 

PHY=eth0/4

#check if this is PHY addressing KSZ9031 (on VC3)
phyId=`phytool read ${PHY}/3`
if [ $phyId == "0x1622" ]; then
    echo "Detected VC3"
    GBPS=0x0140
    MBPS=0x2100

    CLKSKEW=0x03e0


#check if this is PHY addressing we have on VC3
else 
    echo "No valid PHY for eth1 detected (PHYID=${phyId})"
    exit 1
fi


## Adapt RGMII clock pad skew
phytool write ${PHY}/0x0D 0x0002
phytool write ${PHY}/0x0E 0x0008
phytool write ${PHY}/0x0D 0x4002
phytool write ${PHY}/0x0E ${CLKSKEW}

##MII mux to Eth1
## 2 T1-7
## 6 Eth1
devmem 0xc9004cc0 32 6


#echo Configuring Switch Port Eth1 for 100 Mbps
#ethtool -s eth0 speed 100 duplex full autoneg off

## Disable auto negotiation of PHY and fix to 100Mbps
phytool write $PHY/0 $MBPS
