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
    set statics(call:rdbl) -3
    set statics(call:r) -3
    set statics(call:xx) -3
    set statics(norm:-3) "Rdbl"
    set statics(norm:-2) "Dbl"
    set statics(norm:-1) "Pass"
    foreach level {1 2 3 4 5 6 7} {
       foreach denomination {clubs diamonds hearts spades notrump} letter {c d h s n} code {0 1 2 3 4} norm {C D H S NT} {
           set callNo  [expr {($level-1)*5+$code}]
           set statics(norm:$callNo) "$level$norm"
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

    proc parseBid {callNo} {
        set denomNo [expr {$callNo%5}]
        set level [expr {int($callNo/5)+1}]
        return [list $level $denomNo [lindex {clubs diamonds hearts spades notrump} $denomNo]]
    }

    proc call {id call} {
        variable data
        variable statics
        if {[getMember $id completed]} {
             error "Auction already complete"
        }

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
            set bid [parseBid $callNumber]
            set level [lindex $bid 0]
            set denom [lindex $bid 1]
            set denomString [lindex $bid 2]
            set pair $statics(pair:$caller)
            set firstCallers [getMember $id firstCaller::$pair]
            set firstCaller [lindex $firstCallers $denom]
            if {$firstCaller == ""} {
               setMember $id firstCaller::$pair [lreplace $firstCallers $denom $denom $caller]
               setMember $id contractDeclarer $caller
            } else {
               setMember $id contractDeclarer $firstCaller
            }
            setMember $id contractLevel $level
            setMember $id contractDenomination $denomString
             
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

        lappend  [memberRef $id calls] $statics(norm:$callNumber)
        setMember $id nextCaller [lho $caller]
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
        setMember $id firstCaller::EW [list {} {} {} {} {}]
        setMember $id firstCaller::NS [list {} {} {} {} {}]

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

    proc contract {id} {
        set level [getMember $id contractLevel]
        if {$level<0} { 
             return [list "Passed Out"] 
        }
        set denom [getMember $id contractDenomination]
        set declarer [getMember $id contractDeclarer]
        set doubled [getMember $id doubled]
        set contract [list $level $denom $declarer]
        if {$doubled==1} {
            lappend contract doubled
        } elseif {$doubled==2} {
            lappend contract redoubled
        }
        return $contract
    }

}

