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
namespace eval line {

    ::deal::nostacking
    variable handle stdin
 
    proc finalize {} {
	variable handle
	close $handle
    }

    proc initialize {args} {
        variable handle

        if {"$handle"!="stdin"} {
	   finalize
        }

	if {[llength $args]>0} {
	    set file [lindex $args 0]
	    set handle [open $file "r"]
	}
    }

    proc next {} {
	variable handle
	set length -1
	catch { set length [gets $handle line] }
	reset_deck
	if {$length<=0} { 
	    catch { finalize  }
	    # Return "return" code, indicating we're done - no more dealing.
	    return -code return
	    
	}
        set hands [split $line "|"]
	foreach hand {north east south west} val [split $line "|"] {
	    deck_stack_hand $hand [split $val " "] 
	}
	deal_reset_cmds {::line::next}
    }

    proc set_input {{file {}}} {
	if {$file==""} {
	    initialize
	} else {
	    initialize $file
	}
	deal_reset_cmds {::line::next}
    }
}


