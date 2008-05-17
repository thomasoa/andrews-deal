#
# Copyright (C) 1996-2001, Thomas Andrews
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
source lib/handFactory.tcl

namespace eval smartstack {

    variable info

    proc set_input {hand shapeclass {valuation zero} {min 0} args} {
	variable info

        proc ::stack_hand {handname hand} {

           foreach suit {spades hearts diamonds clubs} holding $hand {
              ::smartstack::stack $handname $suit $holding
           }

        }

        proc ::stack_cards {handname args} {
           foreach {suit holding} $args {
              ::smartstack::stack $handname $suit $holding
           }
        }

	if {[llength $args]>0} {
	    set range [list $min [lindex $args 0]]
	} else {
	    set range [list $min $min]
	}

	set info(hand) $hand
	set info(stackcmds) [list]
	set info(factory) [handFactory::create $shapeclass $valuation $range]
        foreach suit {spades hearts diamonds clubs} placed [stacked $hand] {
           $info(factory) restrictSuit $suit $placed contains
           set restrict($suit) {}
        }

        foreach other [list [lho $hand] [partner $hand] [rho $hand]] {
          set stackcmd [list deck_stack_cards $other]
	  set count 0

          foreach suit {spades hearts diamonds clubs} \
		  sletter {S H D C} \
		  cardset [stacked $other] {
            incr count [holding length $cardset]
            set restrict($suit) [::holding union $restrict($suit) $cardset]
            lappend stackcmd $suit $cardset
	  }

          if ($count>0) {
            lappend info(stackcmds) $stackcmd
          }

        }

        foreach suit {spades hearts diamonds clubs} {
           $info(factory) restrictSuit $suit $restrict($suit)
        }

        reset_deck

	deal_reset_cmds {::smartstack::next}
    }

    proc next {} {
	variable info
	reset_deck
	

	foreach cmd $info(stackcmds) {
          set v [catch $cmd]
	  if {$v} {
	    global errorInfo
	    puts stderr "Problem running smartstack: $errorInfo"
	    return -code return
          }
        }

	set v [catch {$info(factory) sample} hand]
	if {$v} {
	    global errorInfo
	    puts stderr "Problem running smartstack: $errorInfo"
	    return -code return
	}
	set v [catch {deck_stack_hand $info(hand) $hand}]
	if {$v} {
	    global errorInfo
	    puts stderr "Problem running smartstack: $errorInfo"
	    return -code return
        }
	deal_reset_cmds {::smartstack::next}
    }

    proc restrictHolding {suit valuation {min 0} args}  {
	if {[llength $args]>0} {
	    set max [lindex $args 0]
	} else {
	    set max $min
	}
	variable info
	$info(factory) addHoldingCond "set v \[$valuation holding \$h\] ; expr {\$v>=$min&&\$v>=$max}" $suit
    }

    proc stack {hand suit holding} {
        variable info
        if {[holding length $holding]==0} { return }

        if {0==[string compare $info(hand) $hand]} {
           set rule contains
        } else {
           set rule disjoint
           lappend info(stackcmds) [list deck_stack_cards $hand $suit $holding]
        }

        $info(factory) addHoldingCond "holding $rule \$h $holding" $suit
    }

    proc getdata {{mult 1.0}} {
	variable info
	$info(factory) getdata $mult
    }
}

