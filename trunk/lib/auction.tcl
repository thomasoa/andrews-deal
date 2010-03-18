#

namespace eval auction {
    
    variable data
    variable statics
    set statics(count) 0
    set statics(call:p) -1
    set statics(call:pass) -1
    set statics(call:double) -2
    set statics(call:d) -2
    set statics(call:x) -2
    set statics(call:dbl) -2
    set statics(call:redouble) -3
    set statics(call:r) -3
    set statics(call:xx) -3
    foreach level {1 2 3 4 5 6 7} {
       foreach denomination {clubs diamonds hearts spades notrump} letter {c d h s n} code {0 1 2 3 4} {
           set statics(call:$level$letter) [expr {($level-1)*5+$code}]
       }
    }
    set statics(pair:east) "EW"
    set statics(pair:west) "EW"
    set statics(pair:north) "NS"
    set statics(pair:south) "NS"

    proc setMember {id member value} {
        variable data
        set "[memberRef $id $member]" $value
    }

    proc getMember {id member} {
        variable data
        set "[memberRef $id $member]"
    }

    proc hasMember {id member} {
       variable data
       return [info exists "[memberRef $id $member]"]
    }

    proc unsetMember {id member} {
       variable data
       unset "[memberRef $id $member]"
    }

    proc memberRef {id member} {
        variable data
        return "data($id:$member)"
    }

    proc call {id call} {
        variable data
        variable statics
        set call [string tolower $call]

        if {![info exists statics(call:$call)]} {
            error "Invalid call"
        }

        set caller [getMember $id nextCaller]
        set lastBid [getMember $id lastBid]
        set lastBidder [getMember $id lastBidder]
        set callNumber $statics(call:$call)

        if {$callNumber>=0} {
            if {$callNumber<=$lastBid} {
              error "Insufficient bid: $call"
            }
            setMember $id lastBid $callNumber
            setMember $id lastBidder $caller
            setMember $id doubled 0
            setMember $id passes 0
        } elseif {$callNumber==-1} {
            # Pass
            set passes [incr [memberRef $id passes]]
            
            if {$passes==4 || ($passes==3 && [llength [getMember $id calls]]>=3)} {
               setMember $id completed 1
            }
        } elseif {$callNumber==-2} {
            # Double
            if {$lastBid<0} {
                error "Can't double when only passes"
            }

            if {[getMember $id doubled]>0} {
                error "Can't double a contract already doubled"
            }

            set myPair $statics(pair:$caller)
            set bidPair $statics(pair:$lastBidder)

            if {$myPair==$bidPair} {
                error "Can't double your sides own bid"
            }
            setMember $id passes 0
            setMember $id doubled 1
        } elseif {$callNumber==-3} {
            # Redouble
            set doubled [getMember $id doubled]
            if {$doubled==0} {
                   error "Can't redouble a contract not doubled"
            }

            if {$doubled==2} {
                   error "Contract already redoubled"
            }

            set myPair $statics(pair:$caller)
            set bidPair $statics(pair:$lastBidder)

            if {$myPair!=$bidPair} {
                error "Can't redouble when you doubled"
            }
            setMember $id passes 0
            setMember $id doubled 2
        } else {
            # Error
            error "Invalid call $call"
        }

        lappend  [memberRef $id calls] $call
        setMember $id nextCaller [rho $caller]
    }

    proc create {dealer {calls {}}} {

        variable statics
        set id $statics(count)
        incr statics(count)
        proc auction$id {cmd args} [format {
            set command [linsert $args 0 auction::$cmd %s]
            uplevel $command
        } $id]

        setMember $id calls [list]
        setMember $id passes 0
        setMember $id contractLevel -1
        setMember $id contractDenomination ""
        setMember $id contractDeclarer ""
        setMember $id doubled 0
        setMember $id lastBid -1
        setMember $id lastBidder ""
        setMember $id nextCaller $dealer
        setMember $id dealer $dealer
        setMember $id completed 0

        foreach call $calls {
            call $id $call
        }
        return "::auction::auction$id"
    }

    proc completed {id} {
        getMember $id completed
    }

    proc delete {id} {
        rename ::auction::auction$id ""
        unsetMember $id calls
        unsetMember $id passes
        unsetMember $id contractLevel
        unsetMember $id contractDenomination
        unsetMember $id contractDeclarer
        unsetMember $id doubled
        unsetMember $id lastBid
        unsetMember $id lastBidder
        unsetMember $id nextCaller
        unsetMember $id dealer
    }
}

