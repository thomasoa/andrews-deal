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
source lib/gib.tcl

#
# The giblib implements the ability to read files of the
# format of Matt Ginsberg's 'library.dat' file.  The file contains
# deals along with the double dummy results of all possible contracts
# played from all directions.
#
# The date file is the "binary" file described at the bottom 
# http://www.cirl.uoregon.edu/ginsberg/gibresearch.html .
#
namespace eval giblib {

    ::deal::nostacking

    variable file "library.dat"
    variable trial 0
    variable filehandle {}

    proc readNextData {} {
	variable filehandle
	set haveRead 0
	set data ""
	while {$haveRead<26} {
	    if {[eof $filehandle]} { return "" }
	    append data [read $filehandle [expr 26-$haveRead]]
	    set haveRead [string length $data]
	}
	return $data
    }

    set who(00) west
    set who(01) north
    set who(10) east
    set who(11) south
    set binary(0000) 0
    set binary(0001) 1
    set binary(0010) 2
    set binary(0011) 3
    set binary(0100) 4
    set binary(0101) 5
    set binary(0110) 6
    set binary(0111) 7
    set binary(1000) 8
    set binary(1001) 9
    set binary(1010) 10
    set binary(1011) 11
    set binary(1100) 12
    set binary(1101) 13
    set binary(1110) 14
    set binary(1111) 15

    variable cardNames [list A K Q J T 9 8 7 6 5 4 3 2]
    variable trial 1
    variable deal

    variable suitOrder [list spades hearts diamonds clubs]
    proc parseData {data} {
	set count [binary scan $data "B32B32B32B32SSSSS" \
		cards(spades) cards(hearts) cards(diamonds) cards(clubs) \
		tricks(notrump) tricks(spades) tricks(hearts) tricks(diamonds) tricks(clubs) ]
	set suitno 0
	
	variable cardNames
	variable who
	variable suitOrder

	set decoded [list]
	foreach hand {north east south west} {
	    foreach suit $suitOrder {
		set holding($hand-$suit) ""
	    }
	}

	foreach suit $suitOrder {
	    binary scan $cards($suit) a6a2a2a2a2a2a2a2a2a2a2a2a2a2 dummy c(2) c(3) c(4) c(5) \
		    c(6) c(7) c(8) c(9) c(T) c(J) c(Q) c(K) c(A)

	    foreach card $cardNames {
		set whom $who($c($card))
		append holding($whom-$suit) $card
	    }
	}
	reset_deck
	foreach hand {north east south west} {
	    deck_stack_hand $hand $holding($hand-spades) $holding($hand-hearts) \
		    $holding($hand-diamonds) $holding($hand-clubs)
	}

	set ddresults [list]
	foreach contract "notrump $suitOrder" {
	    # trick to turning small into unsigned int
	    set trickvalue [expr {( $tricks($contract) + 0x10000 ) % 0x10000} ]
	    set results [list $contract]
	    foreach hand {south west north east} {
		set tr [expr {15&$trickvalue}]
		::deal::metadata gib.$hand.$contract {
		    gib::rectify_tricks $hand $tr
		}
		set trickvalue [expr {$trickvalue/16}]
	    }
	}
    }

    proc openlib {} {
	variable file
	set fh [open $file r]
	fconfigure $fh -translation binary
	return $fh
    }

    proc initialize {{filename unset}} {
	
	variable filehandle
	variable trial

	if {"$filehandle"!=""} { finalize }

	if {"$filename"!="unset"} {
	    variable file
	    set file $filename
	}

	set filehandle [openlib]
	set trial 0
    }

    proc get_next_deal {} {
	variable trial
	set data [readNextData]
	if {[string length $data]!=26} {
	    return 0
	}
	parseData $data
	incr trial
    }

    proc finalize {} {
	variable filehandle
	if {"$filehandle"!=""} {
	    close $filehandle	
	    set filehandle {}
	}
    }

    proc stackhand {name hand} {
	error "Can't stack hands when reading deals from files"
    }

    proc stackcards {name args} {
	error "Can't stack cards when reading deals from files"
    }

    namespace export get_next_deal initialize finalize

    proc set_input {{filename {}}} {
        variable file
	if {$filename==""} {
	    set filename $file
	}
	initialize $filename
	deal_reset_cmds ::giblib::next
    }

    proc next {} {
	deal_reset_cmds ::giblib::next
	if {[catch {giblib::get_next_deal}]} {
	    # All done
	    return -code return
	}
    }
}
