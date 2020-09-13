#!/bin/bash

usage()
{
    echo "usage: flash_fpga_over_spi <inputfile>.jic"
}


IN_FILE=$1

if [[ -z $IN_FILE ]]; then
    echo "Please provide an input file!"
    usage
    exit 1
fi


echo "Setting SPI MUX"
echo 477 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio477/direction
echo "1" > /sys/class/gpio/gpio477/value

echo "Checking NVCR on MT25QU01G"
flash-chk-nvcr -D /dev/spidev0.1 -v "0xAFEE"

echo "Converting JIC file"
IN_FILE_BASE=$(basename ${IN_FILE})
TMP_FILE=$(mktemp /tmp/${IN_FILE_BASE%.*}.XXXXX)
jic2bin.sh -i ${IN_FILE} -o ${TMP_FILE}


echo "Flashing..."
flashrom -c MT25QU01G -p linux_spi:spispeed=33333,dev=/dev/spidev0.1 -w ${TMP_FILE}

echo "Reverting SPI MUX"
echo "0" > /sys/class/gpio/gpio477/value
echo 477 > /sys/class/gpio/unexport


