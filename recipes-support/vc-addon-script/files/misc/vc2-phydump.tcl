#!/usr/bin/tclsh
set ADRMAX 0x10
set PHYS 7

set gpio497 [open /sys/class/gpio/gpio497/value w+]
set gpio498 [open /sys/class/gpio/gpio498/value w+]

set ADDR {0 1 2 3 9 0x16 0x17 0x18 0x1F 0x31}

for {set phy 0} {$phy<$PHYS} {incr phy} {
        if {$phy == 0} {
                puts $gpio497 0
                puts $gpio498 1
                puts "[read $gpio497] - [read $gpio498]"
                #exit
                set n 0
        } elseif {$phy==4} {
                puts $gpio497 1
                puts $gpio498 0
                puts "[read $gpio497] - [read $gpio498]"
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
        #puts $regs($phy)
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
