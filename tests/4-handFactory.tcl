#./deal -x
source lib/handFactory.tcl

defvector honors 1 1 1 1 1

# Any 4333, 4432, or 5332
shapecond bal {$s*$s+$h*$h+$d*$d+$c*$c<=47}

set yarborough [handFactory::create bal honors {0 0}]

set sampleSize 100000
test_prep "Generating $sampleSize hands, might take a moment ... " {
    set hands [list]
    set failures [list]
    for {set i 0} {$i<$sampleSize} {incr i} {
        set hand [$yarborough sample]
        lappend hands $hand
        if {![bal hand $hand] || [honors hand $hand]!=0} {
            lappend failures $hand
        }  
    }
    unset i
}

test checkFactorySamples { set failures } {}
unset failures

set spots {2 3 4 5 6 7 8 9}

foreach spot $spots {
    sdev spot

    set count 0
    foreach hand $hands {
        foreach suit $hand {
             if {[holding contains $suit $spot]} {
                 incr count
             }
         }
    }

    # Expected value is 13/8/4
    set expected [expr {13.0/8/4}]
    test-moe yarborough-$spot-moe $expected [expr {4*$sampleSize}] $count
}

patternfunc pattern { return "$l1$l2$l3$l4" }

set pattern(4333) 0
set pattern(4432) 0
set pattern(5332) 0

foreach hand $hands {
    incr pattern([pattern hand $hand])
}


# Hand-computed probabilities
set p(4333) [expr {40.0/163}]
set p(5332) [expr {48.0/163}]
set p(4432) [expr {75.0/163}]

foreach pat {4333 5332 4432} {
    test-moe yarborough-$pat-moe $p($pat) $sampleSize $pattern($pat)
}

unset yarborough
unset p
unset spots
unset pattern
unset count
unset expected
rename bal {}
rename spot {}
rename pattern {}
