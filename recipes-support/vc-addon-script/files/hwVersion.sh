#!/bin/bash

echo "R-Car Wakeup ID=3c : " `i2cget -y 5 0x3c 15` `i2cget -y 5 0x3c 14` "   (expected 0x57 0x15)"

# Force is need, because LED kernel also accesses it
echo "LED control  ID=4d : " `i2cget -f -y 5 0x4d 0x15` `i2cget -f -y 5 0x4d 0x14` "   (expected 0x54 0x11)"

echo "Mode control ID=5e : " `i2cget -y 5 0x5e 15` `i2cget -y 5 0x5e 14` "   (expected 0x67 0x10)"
echo "FPGA sysid rtlid   : " `devmem 0xc9004d40` `devmem 0xc9004d48`
echo `uname -a`

