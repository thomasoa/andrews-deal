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
#
# read the output from a line input
#
# This reads input in the same format as the -l output from deal.
#
namespace eval symmetric {

    ::deal::nostacking
    variable number 0
    variable current 0
    variable increment 1
    variable endnumber
    variable cards {K Q J T 9 8 7 6 5 4 3 2}
 
    proc finalize {} {
    }

    proc initialize {start incr end} {
        variable number
        variable increment
        variable endnumber

        set number $start
        set increment $incr
        set endnumber $end

	deal_reset_cmds {::symmetric::next}
    }

    proc generate {num} {
        variable current
        variable cards
	reset_deck
        set holding(0) "A"
        set holding(1) ""
        set holding(2) ""
        set holding(3) ""

        set current $num
        foreach card $cards {
          set suit [expr {$num%4}]
          append holding($suit) $card
          set num [expr {($num-$suit)/4}]
        }

        deck_stack_hand south $holding(0) $holding(1) $holding(2) $holding(3)
        deck_stack_hand west  $holding(1) $holding(2) $holding(3) $holding(0)
        deck_stack_hand north $holding(2) $holding(3) $holding(0) $holding(1)
        deck_stack_hand east  $holding(3) $holding(0) $holding(1) $holding(2)
        deal_deck
    }

    proc next {} {
        variable number
        variable increment
        variable endnumber

        if {$number>=$endnumber} {
          return -code return
        }

        generate $number
        incr number $increment
        
	deal_reset_cmds {::symmetric::next}
    }

    proc set_input {{start 0} {increment 1} {end 16777216}} {
        initialize $start $increment $end
    }
}


