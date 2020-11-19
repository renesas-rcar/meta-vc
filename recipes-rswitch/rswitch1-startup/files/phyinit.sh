#!/bin/bash

phyId=`phytool read eth0/6/3`
#echo @6 $phyId

#check if this is PHY addressing we have on VC2
if [ $phyId == "0x1622" ]; then
    echo "Detected VC2 for PHY initialisation"

    #number of phys
    PHYNR=6

    #GP2.10  gpio462   RESET
    #GP0.1   gpio497
    #GP0.2   gpio498
    for a in 497 498; do
        if [ ! -e /sys/class/gpio/gpio$a ]; then
            echo $a > /sys/class/gpio/export
            echo out > /sys/class/gpio/gpio$a/direction
        fi
    done



    function setPhy {
        if [ $1 == 0 ]; then
            echo 0 > /sys/class/gpio/gpio497/value  #GP0.1
            echo 1 > /sys/class/gpio/gpio498/value  #GP0.2
            n=0
        elif [ $1 == 4 ]; then
            echo 1 > /sys/class/gpio/gpio497/value  #GP0.1
            echo 0 > /sys/class/gpio/gpio498/value  #GP0.2
            n=0
        else
            n=$1
        fi
    }


    #Select 0
    echo 0 > /sys/class/gpio/gpio497/value  #GP0.1
    echo 1 > /sys/class/gpio/gpio498/value  #GP0.2


    #KSZ8031 have no broadcast mode

    #disable the broadcast on KSZ9091 (bit9)
    n=5
    CFG2=`phytool read eth0/$n/0x16`
    val=`echo "puts [format 0x%04x [expr $CFG2 | 0x0200]]" | tclsh`
    #echo $CFG2 $val
    phytool write eth0/$n/0x16 $val
    
    #disable the broadcast on RTL9000 (bit13)
    for p in 1 2 3 0 4; do
        setPhy $p
        phytool write eth0/$n/0x31 0x0a43
        CFG2=`phytool read eth0/$n/0x18`
        val=`echo "puts [format 0x%04x [expr $CFG2 & ~0x2000]]" | tclsh`
        #echo $CFG2 $val
        phytool write eth0/$n/0x18 $val
    done
    #exit


    #check all PHY IDs
    declare -A cfgregs
    for p in `seq 0 $PHYNR`; do
        setPhy $p
        CFG=`phytool read eth0/$n/0x00`
        ID1=`phytool read eth0/$n/0x02`
        ID2=`phytool read eth0/$n/0x03`
        cfgregs[$p]=$CFG
        echo "PHY[$p]  ${ID1#0x} ${ID2#0x}  ${CFG#0x}  ($n)"
    done
    #echo ${cfgregs[@]}
    #exit


    #check PHY write and independence
    for p in 0 1 2 3 4 5; do
        #for mask in "| 0x0800" "& 0xf7ff"; do
        for val in "0x0800" "0x2100"; do
            if [ $p == 6 -a $val == 0x0800 ]; then
                continue
            fi
            setPhy $p
            #echo "Write val=$val to phy$p ($n)"
            echo "phytool write eth0/$n/0x00 $val"
            phytool write eth0/$n/0x00 $val

            s=""
            for pp in `seq 0 $PHYNR`; do
                setPhy $pp
                CFG=`phytool read eth0/$n/0x00`
                s="$s $CFG"
                if [ $p != $pp ]; then
                    if [ $CFG != ${cfgregs[$pp]} ]; then
                        echo "Error: After write PHY[$p]=$val read PHY[$pp] is $CFG but ${cfgregs[$pp]} expected"
                    fi
                else
                    case $p in
                    [0-4])
                        if [ $val == 0x0800 ]; then
                            expected=0x2900
                        else
                            expected=0x2100
                        fi
                        ;;
                    *)
                        expected=$val
                        cfgregs[$pp]=$val
                        ;;
                    esac
                    if [ $CFG != $expected ]; then
                        echo "Error: After write $val to PHY[$pp] is $CFG but $expected expected"
                    fi
                fi
            done
            #echo $s
        done
        #exit
    done

#    #debug eth0 (Eth2/1GB connector) fixed to 100 Mb as per PHY trimming
#    ethtool -s eth0 speed 100 duplex full autoneg off


#---------------------------------------------------------------------------------
#VC3
#---------------------------------------------------------------------------------
else
    echo "Detected VC3 for PHY initialisation"

    #GP0.1   gpio497
    if [ ! -e /sys/class/gpio/gpio497 ]; then
        echo 497 > /sys/class/gpio/export
        echo out > /sys/class/gpio/gpio497/direction
    fi
    echo 1 > /sys/class/gpio/gpio497/value  #GP0.1
    if [ "`cat /sys/class/gpio/gpio497/value`" != "1" ]; then
        echo "Cannot set gpio497"
        exit 1
    fi


    #KSZ8031 have no broadcast mode

    #disable the broadcast on RTL9010 (24.13) can be done in 1-shot as no other phys with broadcast available
    phytool write eth0/0/0x18 0x0090

    #the default speed of RTL9010 is 100 Mbps as per RSW1 limitation
    /etc/rswitch1/writeRTL9010.sh 0 || exit 1
    /etc/rswitch1/writeRTL9010.sh 1 || exit 1
    /etc/rswitch1/writeRTL9010.sh 2 || exit 1
    /etc/rswitch1/writeRTL9010.sh 3 || exit 1

    # connect RSW1 with 1G-T1-7
    devmem 0xc9004cc0 32 0x2
fi

exit 0
