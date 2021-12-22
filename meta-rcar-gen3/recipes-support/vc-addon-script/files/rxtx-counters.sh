#!/bin/bash
echo "            tsn4       tsn5       tsn6       tsn7"
echo RxCnt... `devmem 0xc9051408` `devmem 0xc904f408` `devmem 0xc904b408` `devmem 0xc904d408`
echo RxMACErr `devmem 0xc9051424` `devmem 0xc904f424` `devmem 0xc904b424` `devmem 0xc904d424`
echo RxCRCErr `devmem 0xc9051424` `devmem 0xc904f424` `devmem 0xc904b424` `devmem 0xc904d424`
echo RxPhyErr `devmem 0xc905141c` `devmem 0xc904f41c` `devmem 0xc904b41c` `devmem 0xc904d41c`
echo TxCnt... `devmem 0xc9051508` `devmem 0xc904f508` `devmem 0xc904b508` `devmem 0xc904d508`
echo --                                              
echo MACspeed `devmem 0xc9051004` `devmem 0xc904f004` `devmem 0xc904b004` `devmem 0xc904d004`
echo tsn6-mux `devmem 0xc9004cc0` --- 0 SW+MCU 2 SW+T1-7 6 SW+Eth1 1 MCU+T1-7 5 MCU+Eth1

if [ "$1" == "rx" ]; then
    v=0x5a02
elif [ "$1" == "tx" ]; then
    v=0x5a06
else 
    exit 0
fi
for i in 0 1 2 3; do
    #enable the rx/tx mode
    phytool write eth0/${i}/0x1b 0xc800
    phytool write eth0/${i}/0x1c $v

    #read the counter
    phytool write eth0/${i}/0x1b 0xc812
    phystr=" $phystr 0x"`phytool read eth0/${i}/0x1c`
    phytool write eth0/${i}/0x1b 0xc810
    s=`phytool read eth0/${i}/0x1c`
    if [ "${s:0:2}" = "0x" ]; then
        s=${s:2}
    fi
    phystr=$phystr$s

    #reset the counter
    phytool write eth0/${i}/0x1b 0xc802
    phytool write eth0/${i}/0x1c 0x0060
done

echo --   
echo PHYcnt.. $phystr
