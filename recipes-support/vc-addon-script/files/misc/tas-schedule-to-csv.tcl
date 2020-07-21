#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" ${1+"$@"}

set file [lindex $argv 0]
if {$file == ""} {
	puts stderr "Usage:\n$argv0 tas.sch\nto generate gnuplot files" 
	exit 1
}

set ff [open $file]
set tas [read $ff]
close $ff
#puts -nonewline $tas
#puts [llength $tas]

if {[llength $tas] != 8} {
	puts stderr "ERROR: Expect definition of 8 queues in $file"
	exit 1
}



#Calculate the cycle time 
set cycletime 0
for {set q 0} {$q < 8} {incr q} {
	set t [lindex $tas $q]
	set qnr [lindex $t 0]
	set rapport($qnr) 0
	set gatelist($qnr) [lrange $t 1 end]
	foreach e $gatelist($qnr) {
		incr rapport($qnr) [lindex $e 1]
	}
	#puts "Q$qnr $rapport"
	if {$cycletime < $rapport($qnr)} {
		set cycletime $rapport($qnr)
	}
}
#parray rapport
puts stderr "Tas Cycle Time = $cycletime"



#expand the shorten gate lists
for {set q 0} {$q < 8} {incr q} {
	if {$cycletime > $rapport($q)} {
		set n [expr $cycletime / $rapport($q)]
		#puts "n=$n"
		if {$n * $rapport($q) != $cycletime} {
			puts stderr "ERROR: Queue $q with rapport of $rapport($q) doesn't fit in $cycletime"
			exit 1
		}
		set g $gatelist($q)
		if {[llength $g] == 1} {
			set gatelist($q) [list [list [lindex [lindex $g 0] 0] $cycletime]]
		} else {
			while {$n>1} {
				append gatelist($q) " $g"
				incr n -1
			}
		}
	}	
}
#parray gatelist



#convert to continious times
for {set q 0} {$q < 8} {incr q} {
	set t 0
	set gl $gatelist($q)
	set gatelist($q) {}
	set state [lindex [lindex $gl 0] 0]
	foreach g $gl {
		lappend gatelist($q) [list $state $t]
		incr t [lindex $g 1]
		set state [lindex $g 0]
	}	
	lappend gatelist($q) [list [lindex $g 0] $t]
}
#parray gatelist



#extract the toggles
set t 0
while {$t != ""} {
	#puts "t=$t"
	#parray gatelist
	set csv($t) [format "%9d," $t] 
	for {set q 0} {$q < 8} {incr q} {
		set g [lindex $gatelist($q) 0]
		set state [lindex $g 0]
		if {[lindex $g 1] <= $t} {
			set gatelist($q) [lrange $gatelist($q) 1 end]
			set g [lindex $gatelist($q) 0]
			if {$g != ""} {
				set state [lindex $g 0]
			}
		} 
		append csv($t) "  $q.[expr {$state*5}],"
		
	}
	set csv($t) [string range $csv($t) 0 "end-1"]
	#puts $csv($t)

	#look for the next toggle time
	set t -1
	for {set q 0} {$q < 8} {incr q} {
		set gt [lindex [lindex $gatelist($q) 0] 1]
		
		if {$t == -1 || $t > $gt} {
			set t $gt
		} 
	}
}
#parray csv


foreach t [lsort -integer [array names csv]] {
	puts $csv($t)
}
