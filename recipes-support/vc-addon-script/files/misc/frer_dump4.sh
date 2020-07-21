#!/bin/sh
COUNTER=0
# Change the Hash entry based on the driver input
HashEntry=2
# Change the CSID Based on the Configuration from User //XML
CSID=1
# Ingress Port Number
PortNum=4
OUTPUT=0
NULLSIBASE="0x20330000"
SeqRecBase="0x20333000"
IndRecBase="0x20332000"
IngNullSICnt="0x20334000"

RES=`slimswitchtool -x 0x2030FA00`
echo -e "Common 0x2030FA00: $RES"

RES=`slimswitchtool -x0x2030FA04`
echo -e "Common 0x2030FA04 : $RES"

RES=`slimswitchtool -x0x2030FA10`
echo -e "Common 0x2030FA10 : $RES"

RES=`slimswitchtool -x0x2030FA14`
echo -e "Common 0x2030FA14 : $RES"

RES=`slimswitchtool -x0x2030FA18`
echo -e "Common 0x2030FA18 : $RES"

#Null SI Register Dump
while [  $COUNTER -lt 4 ]; do 
    OUTPUT=$(($NULLSIBASE + $((HashEntry*16)) + $((COUNTER*4))))
    LS=`slimswitchtool -x0x$(echo "ibase=10;obase=16;$OUTPUT"|bc)`
    echo "---- Null Stream address 0x$(echo "ibase=10;obase=16;$OUTPUT"|bc) : $LS ----------"
    let COUNTER=COUNTER+1
done

COUNTER=0
#Sequence Recovery Dump
while [  $COUNTER -lt 4 ]; do 
    OUTPUT=$(($SeqRecBase + $((CSID*16)) + $((COUNTER*4))))
    LS=`slimswitchtool -x0x$(echo "ibase=10;obase=16;$OUTPUT"|bc)`
    echo "---- Sequence Reocovery address 0x$(echo "ibase=10;obase=16;$OUTPUT"|bc) : $LS ----------"
    let COUNTER=COUNTER+1
done


COUNTER=0
#Individual Recovery Register Dump
while [  $COUNTER -lt 2 ]; do 
    OUTPUT=$(($IndRecBase + $((HashEntry*8)) + $((COUNTER*4))))
    LS=`slimswitchtool -x0x$(echo "ibase=10;obase=16;$OUTPUT"|bc)`
    echo "---- Individual Recovery address 0x$(echo "ibase=10;obase=16;$OUTPUT"|bc) : $LS ----------"
    let COUNTER=COUNTER+1
done

#IngressNullFrameCounter
COUNTER=0
#Null SI Register Dump
#while [  $COUNTER -lt 4 ]; do
    OUTPUT=$(($IngNullSICnt + $((PortNum*0x800)) + $((HashEntry*4))))
    LS=`slimswitchtool -x0x$(echo "ibase=10;obase=16;$OUTPUT"|bc)`
    echo "---- Null Stream Ingress Counter 0x$(echo "ibase=10;obase=16;$OUTPUT"|bc) : $LS ----------"
    let COUNTER=COUNTER+1
#done


#RES=`slimswitchtool -x0x20334820`
#echo -e "\nIngress Null Counter 0x2033487C : $RES"

