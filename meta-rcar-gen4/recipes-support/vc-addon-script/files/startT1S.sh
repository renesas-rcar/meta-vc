#!/bin/bash

VC_NUM=`dd if=/sys/bus/nvmem/devices/board-data/nvmem bs=1 count=2 skip=1812 status=none | hexdump -e '"%d"'`

if [ -f /home/root/lan865x_mod.ko ]; then
  insmod /home/root/lan865x_mod.ko || exit 1
else
  modprobe lan865x_mod || exit 1
fi

sleep 1


sysctl -w net.ipv6.conf.eth0.disable_ipv6=1
sysctl -w net.ipv6.conf.eth1.disable_ipv6=1
sysctl -w net.ipv6.conf.eth2.disable_ipv6=1


ifconfig eth0 6.6.0.$VC_NUM/16 
ifconfig eth1 6.6.1.$VC_NUM/16
ifconfig eth2 6.6.2.$VC_NUM/16 
