#!/bin/bash

VC_NUM=`dd if=/sys/bus/nvmem/devices/board-data/nvmem bs=1 count=2 skip=1812 status=none | hexdump -e '"%d"'`

#T1S_OVERLAY=~/examples/t1s0.dtbo


lsmod | grep lan865x_mod
if [ $? == "0" ]; then
  echo "lan865x driver will be removed"
  rmmod lan865x_mod
  #remove a potential overlay
  if [ -d /sys/kernel/config/device-tree/overlays/t1s ]; then
    rmdir /sys/kernel/config/device-tree/overlays/t1s
  fi
fi


#apply overlay
if [ -f "${T1S_OVERLAY}" ]; then
  mkdir /sys/kernel/config/device-tree/overlays/t1s
  cat ~/examples/t1s0.dtbo > /sys/kernel/config/device-tree/overlays/t1s/dtbo
fi

#temporary fix to configure the input filter for low level detection 
#eth0, intp35
devmem 0xDFED_4b0c 4
#eth1, intp34
devmem 0xDFED_4b08 4
#eth2, intp33
devmem 0xDFED_4b04 4


#load the driver
if [ -f /home/root/lan865x_mod.ko ]; then
  insmod /home/root/lan865x_mod.ko || exit 1
else
  modprobe lan865x_mod || exit 1
fi
echo "lan865x driver loaded"
sleep 1


#disable IPv6 (the ethX numbers may need update)
sysctl -w net.ipv6.conf.eth0.disable_ipv6=1
sysctl -w net.ipv6.conf.eth1.disable_ipv6=1
sysctl -w net.ipv6.conf.eth2.disable_ipv6=1


#Add some IP addresses
ifconfig eth0 6.6.0.$VC_NUM/16
ifconfig eth1 6.6.1.$VC_NUM/16
ifconfig eth2 6.6.2.$VC_NUM/16
