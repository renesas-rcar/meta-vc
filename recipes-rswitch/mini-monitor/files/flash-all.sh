#/bin/sh
#
#
DEF_TTY=/dev/ttyUSB0
FLASH_CMD_DELAY=${FLASH_CMD_DELAY:-0.5}

if [ -z "$1" ]; then
    TTY=${DEF_TTY}
else
    TTY=$1
fi
echo "Using TTY '${TTY}'"

if [ -z "$2" ]; then
    DEF_PATH=.
else
    DEF_PATH=$2
fi
echo "SREC path is '${DEF_PATH}'"

if [ -z "$3" ]; then
    TG=h3vc3
else
    TG=$3
fi
echo "Board model '${TG}'"


MINIMON_SREC=${MINIMON_SREC:-${DEF_PATH}/$(ls -1 AArch64_Gen3_H3_M3_Scif_MiniMon_V*.mot)}
BOOTPARAM_SREC=${BOOTPARAM_SREC:-${DEF_PATH}/bootparam_sa0.srec}
BL2_SREC=${BL2_SREC:-${DEF_PATH}/bl2-h3vc.srec}
CERT_SREC=${CERT_SREC:-${DEF_PATH}/cert_header_sa6.srec}
BL31_SREC=${BL31_SREC:-${DEF_PATH}/bl31-h3vc.srec}
TEE_SREC=${TEE_SREC:-${DEF_PATH}/tee-h3vc.srec}
UBOOT_SREC=${UBOOT_SREC:-${DEF_PATH}/u-boot-elf-${TG}.srec}

echo "Using Mini Monitor '${MINIMON_SREC}'"
echo "Using U-Boot '${UBOOT_SREC}'"


if [[ ! -c ${TTY} ]]; then
    echo "${TTY} is not a valid TTY"
    echo ""
    echo "Usage $0 [tty port (def.: /dev/ttyUSB0)]"
    echo ""
    echo "The following environ variable can be used to override default values:"
    echo "FLASH_CMD_DELAY       - Delay the is used between single commands"
    echo "BOOTPARAM_SREC        - Lcoation of the boot parameter srec file (default: ./bootparam_sa0.srec)"
    echo "BL2_SREC              - Location of the stage 2 bootloader srec file (default: ./bl2-h3vc2.srec]"
    echo "CERT_SREC             - Location of the certificate srec file (default: ./cert_header_sa6.srec)"
    echo "BL31_SREC             - Location of the stage 3 bootloader srec file (default: ./bl31-h3vc2.srec)"
    echo "TEE_SREC              - Location of the Trusted Execution Environment srec file (default:./tee-h3vc2.srec)"
    echo "UBOOT_SREC            - Location of the U-Boot bootloader srec file (default: ./u-boot-elf.srec)"
    echo "MINIMON_SREC          - Location of the Mini Monitor srec file (default: ./AArch64_Gen3_H3_M3_Scif_MiniMon_V*.mot)"
    exit -1
fi



if [[ ${FLASH_CMD_DELAY} =~ ^-?[0-9]\.?[0-9]+$ ]]; then
    DELAY=${FLASH_CMD_DELAY}
else
    echo "${FLASH_CMD_DELAY} is an invalid delay value"
    exit -1
fi

stty -F ${TTY} 115200
sleep ${DELAY}

echo "Downloading Mini Monitor..."

dd if=${MINIMON_SREC} status=progress bs=1 of=${TTY}


sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}


echo -e "\r\n" > ${TTY}
sleep ${DELAY}


echo -e "sup\r\n" > ${TTY}
sleep ${DELAY}
stty -F ${TTY} 921600


echo "Flashing bootparam..."

echo -e "xls2\r\n" > ${TTY}
sleep ${DELAY}

echo -e "3\r\n" > ${TTY}
sleep ${DELAY}

echo -n "y" > ${TTY}
sleep ${DELAY}

echo -n "y" > ${TTY}
sleep ${DELAY}

echo -e "E6320000\r\n" > ${TTY}
sleep ${DELAY}

echo -e "000000\r\n" > ${TTY}
sleep ${DELAY}
sleep ${DELAY}

dd if=$BOOTPARAM_SREC of=${TTY} bs=1 status=progress

echo "done!"
echo ""
sleep ${DELAY}
sleep ${DELAY}


echo -n "y" > ${TTY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
echo -e "\r\n" > ${TTY}
sleep ${DELAY}
sleep ${DELAY}




echo "Flashing BL2..."

echo -e "xls2\r\n" > ${TTY}
sleep ${DELAY}

echo -e "3\r\n" > ${TTY}
sleep ${DELAY}

echo -n "y" > ${TTY}
sleep ${DELAY}

echo -n "y" > ${TTY}
sleep ${DELAY}

echo -e "E6304000\r\n" > ${TTY}
sleep ${DELAY}

echo -e "040000\r\n" > ${TTY}
sleep ${DELAY}
sleep ${DELAY}

dd if=$BL2_SREC of=${TTY} bs=1 status=progress


echo "done!"
echo ""
sleep ${DELAY}
sleep ${DELAY}


echo -n "y" > ${TTY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
echo -e "\r\n" > ${TTY}
sleep ${DELAY}
sleep ${DELAY}




echo "Flashing cert header..."

echo -e "xls2\r\n" > ${TTY}
sleep ${DELAY}

echo -e "3\r\n" > ${TTY}
sleep ${DELAY}

echo -n "y" > ${TTY}
sleep ${DELAY}

echo -n "y" > ${TTY}
sleep ${DELAY}

echo -e "E6320000\r\n" > ${TTY}
sleep ${DELAY}

echo -e "180000\r\n" > ${TTY}
sleep ${DELAY}
sleep ${DELAY}

dd if=$CERT_SREC of=${TTY} bs=1 status=progress


echo "done!"
echo ""
sleep ${DELAY}
sleep ${DELAY}



echo -n "y" > ${TTY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
echo -e "\r\n" > ${TTY}
sleep ${DELAY}
sleep ${DELAY}



echo "Flashing BL31..."

echo -e "xls2\r\n" > ${TTY}
sleep ${DELAY}

echo -e "3\r\n" > ${TTY}
sleep ${DELAY}

echo -n "y" > ${TTY}
sleep ${DELAY}

echo -n "y" > ${TTY}
sleep ${DELAY}

echo -e "44000000\r\n" > ${TTY}
sleep ${DELAY}

echo -e "1C0000\r\n" > ${TTY}
sleep ${DELAY}
sleep ${DELAY}

dd if=$BL31_SREC of=${TTY} bs=1 status=progress


echo "done!"
echo ""
sleep ${DELAY}
sleep ${DELAY}


echo -n "y" > ${TTY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
echo -e "\r\n" > ${TTY}
sleep ${DELAY}
sleep ${DELAY}



echo "Flashing TEE..."

echo -e "xls2\r\n" > ${TTY}
sleep ${DELAY}

echo -e "3\r\n" > ${TTY}
sleep ${DELAY}

echo -n "y" > ${TTY}
sleep ${DELAY}

echo -n "y" > ${TTY}
sleep ${DELAY}

echo -e "44100000\r\n" > ${TTY}
sleep ${DELAY}

echo -e "200000\r\n" > ${TTY}
sleep ${DELAY}
sleep ${DELAY}

dd if=$TEE_SREC of=${TTY} bs=1 status=progress


echo "done!"
echo ""
sleep ${DELAY}
sleep ${DELAY}


echo -n "y" > ${TTY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
echo -e "\r\n" > ${TTY}
sleep ${DELAY}
sleep ${DELAY}












echo "Flashing u-Boot..."


echo -e "xls2\r\n" > ${TTY}
sleep ${DELAY}

echo -e "3\r\n" > ${TTY}
sleep ${DELAY}

echo -n "y" > ${TTY}
sleep ${DELAY}

echo -n "y" > ${TTY}
sleep ${DELAY}

echo -e "50000000\r\n" > ${TTY}
sleep ${DELAY}

echo -e "640000\r\n" > ${TTY}
sleep ${DELAY}
sleep ${DELAY}

dd if=${UBOOT_SREC} of=${TTY} bs=1 status=progress

echo "done!"
echo ""
sleep ${DELAY}
sleep ${DELAY}





echo -n "y" > ${TTY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
sleep ${DELAY}
echo -e "\r\n" > ${TTY}
sleep ${DELAY}
sleep ${DELAY}


#echo -e "ml e6160058\r\n" > ${TTY}
#sleep ${DELAY}
#echo -e "0\r\n" > ${TTY}
#sleep ${DELAY}
#echo -n "." > ${TTY}
#sleep ${DELAY}


#echo -e "ml e6160110\r\n" > ${TTY}
#sleep ${DELAY}
#echo -e "5aa58000\r\n" > ${TTY}

stty -F ${TTY} 115200
