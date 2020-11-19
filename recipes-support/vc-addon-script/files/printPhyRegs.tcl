#!/usr/bin/tclsh

#check VC version
set ff [open /sys/firmware/devicetree/base/compatible]
set vcVersion [read $ff]
close $ff

if {[string match "*vc2*" $vcVersion]} {
#VC2 PHYs
    set ADRMAX 0x10
    set PHYS 7


    set ADDR {0 1 2 3 9 0x16 0x17 0x18 0x1F 0x31}

    for {set phy 0} {$phy<$PHYS} {incr phy} {
            if {$phy == 0} {
                    set gpio497 [open /sys/class/gpio/gpio497/value w]
                    set gpio498 [open /sys/class/gpio/gpio498/value w]
                    puts $gpio497 0
                    puts $gpio498 1
                    close $gpio497
                    close $gpio498
                    #exit
                    set n 0
            } elseif {$phy==4} {
                    set gpio497 [open /sys/class/gpio/gpio497/value w]
                    set gpio498 [open /sys/class/gpio/gpio498/value w]
                    puts $gpio497 1
                    puts $gpio498 0
                    close $gpio497
                    close $gpio498
                    #exit
                    set n 0
            } else {
                set n $phy
            }
            foreach a $ADDR {
                    set s [format "phytool read eth0/%d/0x%02x" $n $a]
                    #puts $s
                    set v [eval exec $s]
                    lappend regs($phy) $v
                    #break

            }
            #puts "$phy $regs($phy)"
            #break
    }
    #parray regs


    puts "     PHY0a PHY0b PHY1  PHY2  PHY3  PHY5  PHY6"
    puts "Addr BR1   BR2   BR3   BR4   BR5   100M  1G"
    set idx 0
    foreach a $ADDR {
            set s ""
            foreach phy {0 4 1 2 3 5 6} {
                    set v [lindex $regs($phy) $idx]
                    if {$a == 0x01 && ($v&0x0004)!=0} {
                            append s [format "  \033\[32m%04x\033\[m" $v]
                    } else {
                            append s [format "  %04x" $v]
                    }
            }
            switch [format "0x%02x" $a]  {
                0x00 { set c "BMCR    14:Loopback  13:Speed0  12:AutoNeg  8:Duplex  6:Speed1" }
                0x01 { set c "BMSR    2:Link  1:Jabber  0:Extended" }
                0x02 { set c "PHYID1" }
                0x03 { set c "PHYID2" }
                default {set c "" }
            }
            puts "[format "%02x" $a] $s  $c"
            incr idx
    }

} else {
#VC3 phys

    #of Realtec 9010AA
    set ADDR {0 1 2 3 9 10 13 14 0x10 0x18 0x1a 0x1f}
    set PHYS {0 1 3 2 4 5}
    set DEVNAME  "tsn4  tsn5  tsn7  ---tsn6---  eth0"
    set PHYNAME  "PHY0  PHY1  PHY3  PHY2  PHY4  PHY5"
    set LINKNAME "T1-4  T1-5  T1-7  T1-6  ETH1  ETH2"

    foreach phy $PHYS {
             foreach a $ADDR {
                    set s [format "phytool read eth0/%d/0x%02x" $phy $a]
                    #puts $s
                    set v [eval exec $s]
                    lappend regs($phy) $v
                    #break

            }
            #puts $regs($phy)
            #break
    }
    #parray regs

    puts "     $DEVNAME"
    puts "     $PHYNAME"
    puts "Addr $LINKNAME"
    set idx 0
    foreach a $ADDR {
            set s ""
            foreach phy $PHYS {
                    set v [lindex $regs($phy) $idx]
                    if {$a == 0x01 && ($v&0x0004)!=0} {  #link
                            append s [format "  \033\[32m%04x\033\[m" $v]
                    } elseif {$a == 0x09 && ($v&0x0800)==0x0800} {  #master
                            append s [format "  \033\[36m%04x\033\[m" $v]
                    } elseif {$a == 0x0A && ($v&0x3000)==0x3000} {  #receiver
                            append s [format "  \033\[32m%04x\033\[m" $v]
                    } else {
                            append s [format "  %04x" $v]
                    }
            }
            switch [format "0x%02x" $a]  {
                0x00 { set c "BMCR    14:Loopback  13:Speed0  12:AutoNeg  8:Duplex  6:Speed1" }
                0x01 { set c "BMSR    2:Link  1:Jabber  0:Extended" }
                0x02 { set c "PHYID1" }
                0x03 { set c "PHYID2" }
                0x09 { set c [format "Mode    15:13:Test  11:MasterSlave=%d" [expr ($v&0x800)!=0]] }
                default {set c "" }
            }
            puts "[format "%02x" $a] $s  $c"
            incr idx
    }
}

