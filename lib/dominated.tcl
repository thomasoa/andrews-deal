proc holdingReplace {holding cardOut cardIn} {
   string map [list $cardOut $cardIn] $holding
   #set out [holding encode cardOut]
   #set in [holding encode cardIn]
   #set h [holding encode holding]
   #expr {($h^$out)|$in}
}

holdingProc -string dominated {A K Q J T s len} {
    set dominated [list]
    if {$A && !$K} {
        lappend dominated [holdingReplace $s A K]
    }

    if {$K && !$Q} {
        lappend dominated [holdingReplace $s K Q]
    }

    if {$Q && !$J} {
        lappend dominated [holdingReplace $s Q J]
    }

    if {$J && !$T} {
        lappend dominated [holdingReplace $s J T]
    }

    if {$T && ![holding contains $s 9]} {
        lappend dominated [holdingReplace $s T x]
    }
    return $dominated
}
