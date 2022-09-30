#!/bin/bash

echo "Current switch port configuration:"
echo -n " etha0: "; cat /proc/device-tree/soc/ethernet\@e68c0000/ports/port\@0/phy-mode ;echo
echo -n " etha1: "; cat /proc/device-tree/soc/ethernet\@e68c0000/ports/port\@1/phy-mode ;echo
echo -n " etha2: "; cat /proc/device-tree/soc/ethernet\@e68c0000/ports/port\@2/phy-mode ;echo


lsmod | grep rswitch2
if [ $? == "0" ]; then
  echo "rswitch2 driver still loaded. Unload the driver module and try agian."
  exit 1
fi

load=no
load0=no
load1=no
load2=no

for var in "$@"
do
  if [ $var == "all" ]; then
    echo "Automatic loading of rswitch2 driver and make all ports up"
    load0=yes
    load1=yes
    load2=yes
    load=yes
  elif [ $var == "up0" ]; then
    echo "Automatic loading of rswitch2 driver and make port 0 up"
    load0=yes
    load=yes
  elif [ $var == "up1" ]; then
    echo "Automatic loading of rswitch2 driver and make port 1 up"
    load1=yes
    load=yes
  elif [ $var == "up2" ]; then
    echo "Automatic loading of rswitch2 driver and make port 2 up"
    load2=yes
    load=yes

  else
    echo "Unknown parameter $var"
    echo " Use 'all' to load rswitch2 driver after configuration and make all ports up"
    echo " Use 'up0, up1, up2' to load rswitch2 driver and make the selected port up"
    exit 1
  fi
done


if [ $load == "yes" ]; then
  #Start the RSW2 driver
  if [ -f /home/root/rswitch2.ko ]; then
    insmod /home/root/rswitch2.ko || exit 1
  else
    modprobe rswitch2 || exit 1
  fi

  ifconfig sw0 192.168.0.7 up
  sleep 1

  if [ $load0 == "yes" ]; then
    echo "------- ETHA 0 --------"
    sysctl -w net.ipv6.conf.sw0p0.disable_ipv6=1
    ifconfig sw0p0 up
  fi
  if [ $load1 == "yes" ]; then
    echo "------- ETHA 1 --------"
    sysctl -w net.ipv6.conf.sw0p1.disable_ipv6=1
    ifconfig sw0p1 up
  fi
  if [ $load2 == "yes" ]; then
    echo "------- ETHA 2 --------"
    sysctl -w net.ipv6.conf.sw0p2.disable_ipv6=1
    ifconfig sw0p2 up
  fi
else
  echo "No action requested"
fi
