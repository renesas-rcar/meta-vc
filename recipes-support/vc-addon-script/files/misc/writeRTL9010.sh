#!/bin/sh

if [ "$1" != "" ]; then
    PHY=$1
else
    echo "$0 PhyNr"
    exit 1
fi
#echo $PHY

#------------------------------------------------------
#Step1 Check PHY is operational
v=`phytool read eth0/${PHY}/0x02`
if [ "$v" != "0x001c" ]; then
	echo Incorrect PHYID1 $v expected is 0x001c
	exit 1
fi
v=`phytool read eth0/${PHY}/0x03`
if [ "$v" != "0xc800" ]; then
	echo Incorrect PHYID2 $v expected is 0xc800
	exit 1
fi

#echo 0x0a42/16=3
phytool write eth0/${PHY}/0x1f 0x0a42
v=`phytool read eth0/${PHY}/0x10`
let "vv = $v & 3"
if [ $vv != 3 ]; then
	echo Phy is not active [0x10] = $v
	exit 1
fi


#------------------------------------------------------
#Step2 Write init patch

phytool write eth0/${PHY}/0x1b 0xBC40
phytool write eth0/${PHY}/0x1c 0x0FDA

phytool write eth0/${PHY}/0x1b 0xBCC4
phytool write eth0/${PHY}/0x1c 0x8309
phytool write eth0/${PHY}/0x1b 0xBCC6
phytool write eth0/${PHY}/0x1c 0x120B
phytool write eth0/${PHY}/0x1b 0xBCC8
phytool write eth0/${PHY}/0x1c 0x0005
phytool write eth0/${PHY}/0x1b 0xBC40
phytool write eth0/${PHY}/0x1c 0x0FDA
phytool write eth0/${PHY}/0x1b 0xAC16
phytool write eth0/${PHY}/0x1c 0x0000
phytool write eth0/${PHY}/0x1b 0xAC1A
phytool write eth0/${PHY}/0x1c 0x0284
phytool write eth0/${PHY}/0x1b 0x823C
phytool write eth0/${PHY}/0x1c 0x4022
phytool write eth0/${PHY}/0x1b 0x823E
phytool write eth0/${PHY}/0x1c 0x007C
phytool write eth0/${PHY}/0x1b 0x8266

phytool write eth0/${PHY}/0x1c 0x4022
phytool write eth0/${PHY}/0x1b 0x8268
phytool write eth0/${PHY}/0x1c 0x007C
phytool write eth0/${PHY}/0x1b 0x8295
phytool write eth0/${PHY}/0x1c 0x3406
phytool write eth0/${PHY}/0x1b 0x82BF
phytool write eth0/${PHY}/0x1c 0x3406
phytool write eth0/${PHY}/0x1b 0x8205
phytool write eth0/${PHY}/0x1c 0x0110

phytool write eth0/${PHY}/0x1b 0x8207
phytool write eth0/${PHY}/0x1c 0x0125
phytool write eth0/${PHY}/0x1b 0x8209
phytool write eth0/${PHY}/0x1c 0x0125
phytool write eth0/${PHY}/0x1b 0x820B
phytool write eth0/${PHY}/0x1c 0x0125
phytool write eth0/${PHY}/0x1b 0x820D
phytool write eth0/${PHY}/0x1c 0x0110
phytool write eth0/${PHY}/0x1b 0x820F
phytool write eth0/${PHY}/0x1c 0x0120
phytool write eth0/${PHY}/0x1b 0x8211
phytool write eth0/${PHY}/0x1c 0x0125
phytool write eth0/${PHY}/0x1b 0x8213
phytool write eth0/${PHY}/0x1c 0x0130
phytool write eth0/${PHY}/0x1b 0x8296
phytool write eth0/${PHY}/0x1c 0x02A5
phytool write eth0/${PHY}/0x1b 0x82C0
phytool write eth0/${PHY}/0x1c 0x0200
phytool write eth0/${PHY}/0x1b 0xC018
phytool write eth0/${PHY}/0x1c 0x0108

phytool write eth0/${PHY}/0x1b 0xC014
phytool write eth0/${PHY}/0x1c 0x0109
phytool write eth0/${PHY}/0x1b 0xC026
phytool write eth0/${PHY}/0x1c 0x075E
phytool write eth0/${PHY}/0x1b 0xC028
phytool write eth0/${PHY}/0x1c 0x0534


#------------------------------------------------------
#Step3 Optional application specific configurations

#RGMII 2.5V 5-25cm line
phytool write eth0/${PHY}/0x1b 0xd414
phytool write eth0/${PHY}/0x1c 0x0201
phytool write eth0/${PHY}/0x1b 0xd416
phytool write eth0/${PHY}/0x1c 0x0101
phytool write eth0/${PHY}/0x1b 0xd418
phytool write eth0/${PHY}/0x1c 0x0200
phytool write eth0/${PHY}/0x1b 0xd41a
phytool write eth0/${PHY}/0x1c 0x0100
phytool write eth0/${PHY}/0x1b 0xd42e
phytool write eth0/${PHY}/0x1c 0xffff

#RGMII timing correction
phytool write eth0/${PHY}/0x1b 0xd42a
#phytool write eth0/${PHY}/0x1c 0x7007
phytool write eth0/${PHY}/0x1c 0x0007



#------------------------------------------------------
#Step4 SW reset

phytool write eth0/${PHY}/0 0x8000
usleep 20000

v=`phytool read eth0/${PHY}/0`
if [ "$v" != "0x0140" ]; then
	echo Incorrect BMCR $v expected 0x0140
	exit 1
fi

#bring PHY to 100 Mbps mode
phytool write eth0/${PHY}/0 0x2100

echo Done.
