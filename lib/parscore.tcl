#
# Copyright (C) 1996-2001, Thomas Andrews
#
# $Id$
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

source lib/score.tcl

namespace eval parscore {
    variable parscore
    proc rlist {A B C D} {list $D $C $B $A}

    set parscore(order:south) [rlist south west north east]
    set parscore(order:west) [rlist west north east south]
    set parscore(order:north) [rlist north east south west]
    set parscore(order:east) [rlist east south west north]
    set parscore(mult:east) -1
    set parscore(mult:west) -1
    set parscore(mult:north) 1
    set parscore(mult:south) 1
    set parscore(pair:south) NS
    set parscore(pair:north) NS
    set parscore(pair:east)  EW
    set parscore(pair:west)  EW

    proc par_first_upper {word} {
        string toupper [string range $word 0 0]
    }

    proc par_upcase {word} {
        set first par_first_upper
        set rest [string range $word 1 end]
        append first $rest
    }

    proc parscore {dealer whovul} {
        ::deal::metadata parscore.$dealer.$whovul [list parscore_uncached $dealer $whovul]
    }

    proc parscore_uncached {dealer whovul} {
        if {"$whovul"=="EW"} {
	    set vul(EW) vul
            set vul(NS) nonvul
        } elseif {"$whovul"=="NS"} {
            set vul(EW) nonvul
	    set vul(NS) vul
        } elseif {"$whovul"=="All"} {
	    set vul(EW) vul
	    set vul(NS) vul
        } else {
	    set vul(EW) nonvul
	    set vul(NS) nonvul
        }	
    
        global ::parscore::parscore

        # Quick call to precompute tricks
        foreach denom {notrump spades hearts diamonds clubs} {
            foreach hand {north east south west} {
	        set tricks($hand:$denom) [deal::tricks $hand $denom]
	    }
        }
    
        set hands $parscore(order:$dealer)
    
        set bestcontract {Pass}
        set bestdeclarer {}
        set bestscore 0
        set besttricks ""
        set bestauction "Pass   Pass    Pass    Pass"
        set passes(3) "Pass Pass Pass"
        set passes(2) "Pass Pass"
        set passes(1) "Pass"
        set passes(0) ""
        set biggestfit 0
    
        for {set level 1} {$level<=7} {incr level} {
	    set anymake 0
	    foreach denom {clubs diamonds hearts spades notrump} {
                set passcount 4
	        foreach declarer $hands {
                    if {$denom == "notrump"} {
                      set fit 14
                    } else {
                      set fit [expr {[$denom $declarer]+[$denom [partner $declarer]]}]
                    }
		    incr passcount -1
		    set pair $parscore(pair:$declarer)
		    if {$tricks($declarer:$denom)<6+$level} { 
		        set makes 0
		        set contract [list $level $denom doubled]
		    } else { 
		        set makes 1
		        set anymake 1
		        set contract [list $level $denom]
		    }
    
		    set mult $parscore(mult:$declarer)
		    set newscore [score $contract $vul($pair) $tricks($declarer:$denom)]
		    #puts "Comparing [expr {$mult*$newscore}] in $contract by $declarer to $bestscore in $bestcontract by $bestdeclarer"
		    if {$newscore>$mult*$bestscore || (($newscore==$mult*$bestscore) && $fit>$biggestfit) } {
                        set biggestfit $fit
		        set bestcontract $contract
		        set bestdeclarer $declarer
		        set bestscore [expr {$mult*$newscore}]
		        set besttricks $tricks($declarer:$denom)
		        set level [lindex $contract 0]
		        set suit [par_first_upper [lindex $contract 1]]
		        set auction $passes($passcount)
		        lappend auction "$level$suit"
                        if {!$makes} {
			    lappend auction "X"
		        }
		        lappend auction 
		        set bestauction $auction
		    }
	        }
	    }
        }
        list $bestcontract $bestdeclarer $bestscore $besttricks $bestauction
    }
} 

proc parscore {dealer vul} {
    parscore::parscore $dealer $vul
}
