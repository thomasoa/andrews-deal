#
# deal.tcl - this file is sourced at startup by Deal 3.0 or later
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
source format/default

namespace eval deal {

    variable metadata
    #
    # Put data in the cache, to be unset at next call
    # to deal_deck
    #
    proc metadata {name code} {
	variable metadata
	if {![info exists metadata($name)]} {
	    if {[catch {set metadata($name) [uplevel $code]}]} {
		global errorInfo
		puts stderr "Error: $errorInfo"
	    } else {
		deal_reset_cmds [list unset ::deal::metadata($name)]
	    }
	}
	return $metadata($name)
    }

    proc loop {} {
	next
	write
    }

    proc input {format args} {
	uplevel #0 [list source "input/$format.tcl" ]
	set command [list "${format}::set_input"]
	foreach arg $args {
	    lappend command $arg
	}
	uplevel #0 $command
    }

    proc debug {args} {
       puts stderr $args
    }

    # Cause an error if any hand stacking has occured
    proc nostacking {} {
        set format [uplevel {namespace current}]
        proc ::stack_hand {args} \
		"error \"No hand stacking with input format $format\""
        proc ::stack_cards {args} \
		"error \"No card stacking with input format $format\""
        foreach hand {south north east west} {
          foreach holding [stacked $hand] {
             if {[holding length $holding]!=0} {
                error "Stacking cards is not consistent with input format $format"
             }
          }
        }
    }
}

# These two routines used to be defined in C, but it's better for them
# to fit the pattern of shape functions.

shapecond balanced {($h<5)&&($s<5)&&($s*$s+$h*$h+$d*$d+$c*$c)<=47}
shapecond semibalanced {$h<=5&&$s<=5&&$d<=6&&$c<=6&&$c>=2&&$d>=2&&$h>=2&&$s>=2}
shapecond AnyShape {1}

#
#  The three routines, joinclass, negateclass, intersectclass, used to be
#  implemented in C, but were never documented and recently crashed Deal 3.0.x
#  when called.  I've reimplemented them here in pure Tcl.
#
proc joinclass {newclass args} {
    set values [list 0]
    foreach class $args {
	lappend values "\[$class eval \$s \$h \$d \$c\]"
    }

    shapecond ___tempclass [join $values "||"]

    # make sure it is compiled first - use temporary name
    # in case we are re-using an old name for a class
    ___tempclass eval 13 0 0 0
    rename ___tempclass $newclass
}

proc negateclass {newclass class} {
    shapecond ___tempclass "!\[$class eval \$s \$h \$d \$c\]"
    ___tempclass eval 13 0 0 0
    rename ___tempclass $newclass
}

proc intersectclass {newclass args} {

    set values [list 1]
    foreach class $args {
	lappend values "\[$class eval \$s \$h \$d \$c\]"
    }
    shapecond ___tempclass [join $values "&&"]
    ___tempclass eval 13 0 0 0
    rename ___tempclass $newclass
}

