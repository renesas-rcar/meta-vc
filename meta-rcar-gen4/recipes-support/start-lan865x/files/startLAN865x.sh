#!/bin/bash

lsmod | grep lan865x
if [ $? == "0" ]; then
  echo "LAN865x driver still loaded. Reload the driver module and try agian."
  echo "rmmod lan865x_mod"
  exit 1
fi

lsmod | grep lan865x_mod
if [ $? != "0" ]; then
  insmod /usr/lib/lan865x_mod.ko
fi


exit 0

