namespace eval dominated {

    proc replaceCard {holding cardOut cardIn} {
       string map [list $cardOut $cardIn] $holding
       #set out [holding encode cardOut]
       #set in [holding encode cardIn]
       #set h [holding encode holding]
       #expr {($h^$out)|$in}
    }

    holdingProc -string dominated {A K Q J T s len} {
        set dominated [list]
        if {$A && !$K} {
            lappend dominated [dominated::replaceCard $s A K]
        }

        if {$K && !$Q} {
            lappend dominated [dominated::replaceCard $s K Q]
        }
    
        if {$Q && !$J} {
            lappend dominated [dominated::replaceCard $s Q J]
        }

        if {$J && !$T} {
            lappend dominated [dominated::replaceCard $s J T]
        }
    
        if {$T && ![holding contains $s 9]} {
            lappend dominated [dominated::replaceCard $s T x]
        }
        return $dominated
    }

    proc dominatedHands {hand} {
         set results [list]
         foreach suitNo {0 1 2 3} dominatedHoldings [dominated hand $hand] {
             foreach holding $dominatedHoldings {
                 set dHand [lreplace $hand $suitNo $suitNo $holding]
                 lappend results $dHand
             }
         }
         return $results
    }

    namespace export dominatedHands
}
     
namespace import  dominated::dominatedHands
