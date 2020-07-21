#!/bin/bash

echo "R-Car Wakeup ID=3c (0x57, 0x12) : " `i2cget -y 5 0x3c   15` `i2cget -y 5 0x3c 14`
echo "LED control  ID=4d (0x54, 0x10) : " `i2cget -y 5 0x4d 0x15` `i2cget -y 5 0x4d 0x14`
echo "Mode control ID=5e (0x67, 0x10) : " `i2cget -y 5 0x5e   15` `i2cget -y 5 0x5e 14`
echo "FPGA sysid rtlid                : " `devmem 0xc9004d40` `devmem 0xc9004d48`
echo `uname -a`

