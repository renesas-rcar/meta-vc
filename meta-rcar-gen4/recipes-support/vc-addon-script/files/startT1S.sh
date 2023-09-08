#!/bin/bash

VC_NUM=`dd if=/sys/bus/nvmem/devices/board-data/nvmem bs=1 count=2 skip=1812 status=none | hexdump -e '"%d"'`

#T1S_OVERLAYS=~/examples/t1s0.dtbo

if [ -d /sys/module/lan865x_mod ]; then
  echo "lan865x driver will be removed"
  rmmod lan865x_mod
fi

#apply overlaays
for dtbo in $T1S_OVERLAYS; do
  ovl=$(basename "$dtbo" .dtbo)
  ovld=/sys/kernel/config/device-tree/overlays/$ovl
  if [ -d $ovld ]; then
    echo "removing existing $ovl overlay"
    rmdir $ovld
  fi
  echo "loading $dtbo"
  mkdir $ovld
  cat $dtbo > $ovld/dtbo
done

#load the driver
if [ -f /home/root/lan865x_mod.ko ]; then
  echo "loading custom module from /home/root/lan865x_mod.ko"
  insmod /home/root/lan865x_mod.ko || exit 1
else
  echo "loading system lan865x_mod.ko"
  modprobe lan865x_mod || exit 1
fi
echo "lan865x driver loaded"
sleep 1

# Detect eth device names
spidevs=$(cd /sys/bus/spi/drivers/LAN865x && echo spi*)
ethdevs=$(for d in $spidevs; do ls /sys/bus/spi/devices/$d/net; done)

# Disable IPv6
for eth in $ethdevs; do
  sysctl -w net.ipv6.conf.$eth.disable_ipv6=1
done

# Add some ip addresses
n=0
for eth in $ethdevs; do
  ip addr add 6.6.$n.$VC_NUM/16 dev $eth
  n=$[$n+1]
done
