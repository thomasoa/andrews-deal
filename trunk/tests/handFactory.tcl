#./deal -x
source lib/handFactory.tcl

defvector honors 1 1 1 1 1

# Any 4333, 4432, or 5332
shapecond bal {$s*$s+$h*$h+$d*$d+$c*$c<=47}

set yarborough [handFactory::create bal honors {0 0}]

test checkOutput {
    set failures 0
    set hands [list]
    for {set i 0} {$i<100000} {incr i} {
        set hand [$yarborough sample]
        lappend hands $hand
        if {![bal hand $hand] || [honors hand $hand]!=0} {
            incr failures
        }  
    }
    set failures
} 0

set spots {2 3 4 5 6 7 8 9}

foreach spot $spots {
    sdev spot
    foreach hand $hands {

        set count 0
        foreach suit $hand {
             if {[holding contains $suit $spot]} {
                 incr count
             }
         }
         spot add $count
    }
    set average [spot average]
    # Expected value is 13/8 = 1.625
    set error "0.010"
    set expected "1.625"
    test spot-$spot {expr {$average>=$expected-$error&&$average<=$expected+$error}} 1
}

patternfunc pattern { return "$l1$l2$l3$l4" }
set "pattern(4333)" 0
set "pattern(4432)" 0
set "pattern(5332)" 0

foreach hand $hands {
    incr pattern([pattern hand  $hand])
}

set p(4333) [expr {40.0/163}]
set p(5332) [expr {48.0/163}]
set p(4432) [expr {75.0/163}]
foreach pat {4333 5332 4432} {
    test-moe yarborough-$pat-moe $p($pat) 100000 $pattern($pat)
}

