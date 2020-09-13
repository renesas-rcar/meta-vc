#!/bin/bash

##Observe errors by this scripts with
## systemctl status vc2startup.service

ecu=`echo $HOSTNAME | egrep -o '[[:digit:]]+'`
echo "Run on ECU $ecu"

WAITLINK=/home/root/vc2/booting/wait_link_up.sh
SYNCSERV=/home/root/vc2/booting/gptp_status_server.py

case "$1" in
start)
    ## switch off the 8 LEDs on front of box
    i2cset -y 5 0x4d 0 0xff  #leds 0..3 off
    i2cset -y 5 0x4d 1 0xff  #leds 4..7 off

    ## Map the local RAM for PCIe access
    devmem 0xC8001000 32 0x00
    devmem 0xC8001008 32 0x40000000

    ## Enable AVBES in R-Car as debug port
    ifconfig eth0 hw ether 74:90:50:00:$ecu:EE
    ifconfig eth0 up
    ifconfig eth0 192.168.0.$ecu/24
#    ifconfig eth0 192.168.178.$ecu/24

    ## Configure default router of the R-Car debug port
    route add default gw 192.168.0.10
#    route add default gw 192.168.178.61

    ## Configure PHYs and R-Car GPIO pins for PHY switching
    ## to allow seting PHY Master or Slave mode by rswitchtool
    /home/root/vc2/booting/phyinit.sh


    ## load all kernel modules for the switch and enable PCIe connection to FPGA
    modprobe rswitch_eth || exit 1

    ## Set correct clock frequency for gPTP timer_domain
    ## devmem 0xC9054810 32 0xE38E39

    ## enable PPS output (RAVBES) based on corrected timer
    ## devmem 0xe6800390 32 0x00010000
    devmem 0xe6060000 32 0xffff8412; devmem 0xe6060108 32 0x00007BeD  #GPSR2[6]
    devmem 0xe6060000 32 0x9ffecccc; devmem 0xe6060204 32 0x60013333  #IP1[19:16]

    ## Map the local RAM for PCIe access
    devmem 0xC8001000 32 0x00
    devmem 0xC8001008 32 0x40000000


    ##############################################
    ## Configure Switch and Ethernet TSN functions
    ##############################################

    ## Basic configuration - includes Layer 2 routing and gPTP function
    rswitchtool --configure=fwd -f /home/root/vc2/configuration/l2_gptp_default.xml

    ## [Renesas_VC_AN] Sect. 1 - CBS example
    #rswitchtool --configure=fwd -f /home/root/vc2/configuration/l2_gptp_default.xml
    #rswitchtool --configure=eth -f /home/root/vc2/configuration/eth_cbs.xml

    ## [Renesas_VC_AN] Sect. 2 - TAS example
    #rswitchtool --configure=fwd -f /home/root/vc2/configuration/l2_gptp_default.xml
    #rswitchtool --configure=eth -f /home/root/vc2/configuration/eth_tas_port3.xml

    ## [Renesas_VC_AN] Sect. 3 -  Timing synchronization with gPTP example
    #rswitchtool --configure=fwd -f /home/root/vc2/configuration/l2_gptp_default.xml
    #
    ### gPTP: Run as Bridge (ptp4linux application) -- between interface TSN3 and TSN4
    ##($WAITLINK 3; $WAITLINK 4; ptp4l -i tsn3 -i tsn4 -f /home/root/vc2/configuration/ptp4l.cfg -m | $SYNCSERV) &
    #
    ### gPTP: Run as Endpoint (ptp4linux application) -- on interface TSN3
    #(#$WAITLINK 3; ptp4l -i tsn3 -f /home/root/vc2/configuration/ptp4l.cfg -m | $SYNCSERV) &
    #
    ### gPTP: run as grand master (ptp4linux application) -- on interface TSN3 and TSN4
    #($WAITLINK 3; $WAITLINK 4; ptp4l -i tsn3 -i tsn4 -f /home/root/vc2/configuration/ptp4l_master.cfg -m | $SYNCSERV) &
    #
    ###gPTP: Run as Bridge (avnu application) -- TSN3 connected to GM port, TSN4 to Slave Port
    ##($WAITLINK 3; daemon_cl tsn3 -S -V -E) &
    ##($WAITLINK 4; daemon_cl tsn4  -V -GM) &

    ## [Renesas_VC_AN] Sect. 4 - VLAN-unaware routing example
    #rswitchtool --configure=fwd -f /home/root/vc2/configuration/l2_gptp_default.xml
    #rswitchtool --configure=fwd -f /home/root/vc2/configuration/VLAN-unaware_Routing.xml

    ## [Renesas_VC_AN] Sect. 6 - Preemption example
    #rswitchtool --configure=fwd -f /home/root/vc2/configuration/l2_gptp_default.xml
    #if [ $ecu == 22 ]; then
    #    ##DA 46:46:46:46:46:46 is only routed to CPU - needed to see frames in tcpdump
    #    rswitchtool --configure=fwd -f /home/root/vc2/configuration/Preemption_fwd_rxbox.xml
    #else
    #    ##DA 46:46:46:46:46:46 needs also be routed to port 2 and 3
    #    rswitchtool --configure=fwd -f /home/root/vc2/configuration/Preemption_fwd_txbox.xml
    #fi
    #rswitchtool --configure=eth -f /home/root/vc2/configuration/Preemption_eth.xml

    ## [Renesas_VC_AN] Sect. 7 - Quality of Service (QoS) - Overload Management example
    #rswitchtool --configure=fwd -f /home/root/vc2/configuration/l2_gptp_default.xml
    #rswitchtool --configure=fwd -f /home/root/vc2/configuration/QoS_OverloadManagement.xml

    ## VLAN routing function
    #rswitchtool --configure=fwd -f /home/root/vc2/configuration/l2_gptp_vlan.xml

    ## Preemption with TAS functions
    #rswitchtool --configure=fwd -f /home/root/vc2/configuration/l2_gptp_vlan5.xml
    #rswitchtool --configure=eth -f /home/root/vc2/configuration/preemption_TAS.xml


    ## Configure box specific items like Ethernet T1 master/slave
#    if [ $ecu == 22 ]; then
#        ## bring all 100base-T1 ports into master mode
#        rswitchtool -c phy -f /home/root/vc2/configuration/phy_master_12345.xml
#    else
#        ## bring all 100base-T1 ports into slave mode
#        rswitchtool -c phy -f /home/root/vc2/configuration/phy_slave_12345.xml
#    fi

    ## Activate the switch ports
    ## tsn0 to tsn5 have no IP address all communication is done by tsngw

    ## TSN0 is to internal MCU boards, use only if F1K board is present in box
    ifconfig tsn0 hw ether 74:90:50:00:$ecu:08
    ifconfig tsn0 up

    ifconfig tsn1 hw ether 74:90:50:00:$ecu:10
    ifconfig tsn1 up

    ifconfig tsn2 hw ether 74:90:50:00:$ecu:20
    ifconfig tsn2 up

    ifconfig tsn3 hw ether 74:90:50:00:$ecu:30
    ifconfig tsn3 up

#    ifconfig tsn4 hw ether 74:90:50:00:$ecu:40
#    ifconfig tsn4 up
#
#    ifconfig tsn5 hw ether 74:90:50:00:$ecu:50
#    ifconfig tsn5 up

    ## tsngw is the internal port of R-Car CPU for IP communication
    ifconfig tsngw hw ether 74:90:50:00:$ecu:CC
    ifconfig tsngw up
    ifconfig tsngw 192.168.1.$ecu/24
    sed s/vcXX/$ecu/g /home/root/vc2/configuration/tsngw_static_entry.template > /home/root/vc2/configuration/tsngw_static_entry.xml
    rswitchtool --configure=fwd -f /home/root/vc2/configuration/tsngw_static_entry.xml
    rm -f /home/root/vc2/configuration/tsngw_static_entry.xml

    ## Configure R-CAR debug port for 100Mbps or 1Gbps (= 1000Mbps)
    /home/root/vc2/booting/eth0speed.sh 100

    ## small daemon to monitor the link status of tsn ports
#    /home/root/vc2/booting/monitor_link.sh &
    ;;

stop)

    ifconfig tsn0 down
    ifconfig tsn1 down
    ifconfig tsn2 down
    ifconfig tsn3 down
#    ifconfig tsn4 down
#    ifconfig tsn5 down
    ifconfig tsngw down

    ifconfig eth0 down

    #rmmod rswitch_fwd
    #rmmod rswitch_eth
    #rmmod rswitch_ptp
    #rmmod rswitch_debug

    ## switch off the 7 LEDs and set 5 to orange on front of box
    i2cset -y 5 0x4d 0 0xff  #leds 0..3 off
    i2cset -y 5 0x4d 1 0xbf  #leds 4..7 off
    ;;

force-reload|restart)
    $0 stop
    $0 start
    ;;

*)
    echo "Usage: $0 {start|stop}"
    exit 1
    ;;
esac

exit 0
