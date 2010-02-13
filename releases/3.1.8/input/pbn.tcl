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
namespace eval pbn {

    ::deal::nostacking

    variable handle stdin
 
    proc finalize {} {
	variable handle
	close $handle
    }

    proc initialize {file} {
        variable handle

        if {"$handle"!="stdin"} {
	   finalize
        }
	
	set handle [open $file "r"]
	puts stderr "file opened for $file"
	deal_reset_cmds {::pbn::next}
    }

    proc next {} {
	variable handle
	set length [gets $handle line]
	reset_deck
	if {$length==0} { 
	    finalize 
	    exit 0
	}
	puts stderr "Input: $line"
	foreach hand {north east south west} val [split $line "|"] {
	    deck_stack_hand $hand [split $val " "]
	}
	deal_reset_cmds {::pbn::next}
    }

    proc set_input {file} {
	initialize $file
	deal_reset_cmds {::pbn::next}
    }
}


