#!/bin/bash

echo 399 > /sys/class/gpio/export
echo out> /sys/class/gpio/gpio399/direction 
echo 1 > /sys/class/gpio/gpio399/value

devmem 0xc9004cc0 32 1
