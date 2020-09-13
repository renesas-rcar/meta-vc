#!/usr/bin/tclsh

#of Realtec 9010AA
set ADDR {0 1 2 3 9 10 13 14 0x10 0x18 0x1f}
set PHYS {0 1 2 3 4 5}
set PHYNAME  "PHY0  PHY1  PHY2  PHY3  PHY4  PHY5"
set LINKNAME "T1-4  T1-5  T1-6  T1-7  ETH1  ETH2"

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
parray regs

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

