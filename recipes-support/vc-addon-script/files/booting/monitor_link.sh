#TSN             0  1    2    3    4    5
declare -a phy=(-1  0    0    1    2    3)
declare -a  br=( 0  1    2    3    4    5)
declare -a led=( 0 0x80 0x20 0x08 0x02 0x80)
declare -a bank=(0  0    0    0    0    1)

i2cset -y 5 0x4d 0 0xff  #leds 0..3 off
i2cset -y 5 0x4d 1 0xff  #leds 4..7 off

#GP0_1 is GPIO 497 in linux.
#GP0_2 is GPIO 498 in linux.
#mapping already done by phyinit.sh

while true; do
    #check the real PHY
    for (( i=1; i<=5; i++ )); do
        #echo "tsn$i / BR${br[$i]} / phy=${phy[$i]} / led=${led[$i]}"
        if [ $i == 1 ]; then
            #if you want to have setup MDIO access to Eth1(IC9)
            echo 0 > /sys/class/gpio/gpio497/value
            echo 1 > /sys/class/gpio/gpio498/value
        elif [ $i == 2 ]; then
            #if you want to have setup MDIO access to Eth2(IC10)
            echo 1 > /sys/class/gpio/gpio497/value
            echo 0 > /sys/class/gpio/gpio498/value
        fi
        link=`phytool eth0/${phy[$i]} | grep "+link"`
        #echo "  link=$link"
        led_is=`i2cget -y 5 0x4d ${bank[$i]}`
        if [ "$link" != "" ]; then
            #echo "  LINK"
            link=1
            led_new=`echo "puts [format 0x%02x [expr $led_is & ~${led[$i]}]]" | tclsh`
        else
            link=0
            led_new=`echo "puts [format 0x%02x [expr $led_is | ${led[$i]}]]" | tclsh`
        fi
        led_changed=`echo "puts [expr $led_is != $led_new]" | tclsh`
        #echo $led_is $i ${led[$i]} $led_new $led_changed
        if [ $led_changed != 0 ]; then
            echo "Link tsn$i/BR${br[$i]}/phy=${phy[$i]} is now $link  (bank=${bank[$i]} is=$led_is new=$led_new)"
            l=`i2cset -y 5 0x4d ${bank[$i]} $led_new`
        fi
    done
    #exit

    #reuse LED8 for gPTP sync information
    gptp=`nc localhost 5555`
    if [ "$gptp" == "" ]; then
            gptp=0
    elif [ "$gptp" != "0" ]; then
            gptp=1
    fi
    led_is=`i2cget -y 5 0x4d 1`
    led_changed=`echo "puts [expr ($led_is&0x01) == $gptp]" | tclsh`
    #echo gptp=$gptp led_is=$led_is led_changed=$led_changed
    if [ $led_changed != 0 ]; then
            if [ $gptp == 1 ]; then
                    led_new=`echo "puts [format 0x%02x [expr $led_is & ~0x01]]" | tclsh`
            else
                    led_new=`echo "puts [format 0x%02x [expr $led_is | 0x01]]" | tclsh`
            fi
            echo "gPTP sync state changed to $gptp  (bank=1 is=$led_is new=$led_new)"
            l=`i2cset -y 5 0x4d 1 $led_new`
    fi

    #poll interval is 1 sec
    sleep 1
done
