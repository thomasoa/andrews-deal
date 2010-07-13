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
namespace eval ddline {
    variable glob

    set glob(suitorder) {spades hearts diamonds clubs notrump}

    ::deal::nostacking

    proc set_input {{fname {}} {skip 0} {suitorder {spades hearts diamonds clubs}}} {
	variable glob
	if {$fname!=""} {
	    set glob(filehandle) [open $fname r]
	} else {
	    set glob(filehandle) stdin
	}

        while {$skip>0} {
          incr skip -1
          gets $glob(filehandle) line
        }

	set glob(suitorder) $suitorder
	lappend glob(suitorder) notrump
	deal_reset_cmds ::ddline::next
    }

    proc next {} {
	deal_reset_cmds ::ddline::next
	if {[catch {::ddline::nextline}]} {
	    return -code return
	}
    }

    proc parseline {line} {
        variable glob

	foreach pname {north east south west ddnorth ddeast ddsouth ddwest} val [split $line "|"] {
	    set part($pname) $val
	}

	foreach hand {north east south west} {
	    set h [split $part($hand) "."]
	    foreach suit  $glob(suitorder) \
		    tricks $part(dd$hand) \
		    holding $h {
		set holdings($suit) $holding
		::deal::metadata ddline.$hand.$suit {expr $tricks}
	    }
	    deck_stack_hand $hand [list $holdings(spades) $holdings(hearts) $holdings(diamonds) $holdings(clubs)]
	}
    }

    proc nextline {} {
	variable glob
	set length -1
	catch { set length [gets $glob(filehandle) line] }
	reset_deck
	if {$length<=0} {
	    return -code return
	}
        parseline $line
    }
}

set deal::tricksCache ddline
