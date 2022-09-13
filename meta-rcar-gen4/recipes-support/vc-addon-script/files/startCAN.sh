#!/bin/sh

from=${1:-0}
to=${2:-15}
rate=${3:-250}

echo "Initialise CAN $from ~ $to with $rate kbps"

for c in `seq $from $to`; do
	ip l set can$c type can fd on bitrate ${rate}000 dbitrate ${rate}000
	ip l set can$c up
done

echo "Use 'candump any' to observe can communication"
echo "Use 'cansend can0 123#1122334455667788' to send a frame"
