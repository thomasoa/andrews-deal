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
# Package gib::*
#
#   Provide an interface for the GIB double-dummy solver.
#
#   gib::tricks <declarer> <denom>
# 
#      Returns the double-dummy number of tricks in the given denomination
#
#   gib::directory <path>
#
#      Tells the package where to find the GIB binaries. If this
#      isn't called before calling gib::tricks, it defaults to
#      "C:\Program Files\GIB"
#
namespace eval gib {

    variable executable {c:\Program Files\GIB\bridge.exe}
    variable infile {c:\temp\gibinfoo.txt}
    variable outfile {c:\temp\giboutfoo.txt}
    variable tmpfile {c:\temp\gibtempfoo.txt}

    proc infile {} { variable infile ; return $infile }

    proc outfile {} { variable outfile ; return $outfile }

    proc tempfile {} { variable tmpfile ; return $tmpfile }

    set leader(south) w
    set leader(west) n
    set leader(north) e
    set leader(east) s

    proc format_deal_gib {} {
	set result [list]
	foreach hand {north east south west} {
	    set h [join [$hand] "."]
	    lappend result "[string index $hand 0] $h"
	}
	join $result "\r\n"
    }

    proc directory {path} {
	variable executable
	set executable "$path/bridge.exe"
    }

    proc head_gib {declarer denom} {
	global gib::leader
	set lead $leader($declarer)
	set denom [string index $denom 0]
	return "[format_deal_gib]\n$lead $denom"
    }

    # gib always returns the count of north's tricks.  This returns declarers
    # tricks, given
    #
    # This routine is its own inverse, so we can also use it to return
    # N/S tricks by passing in declarer and declarers tricks.
    #
    proc rectify_tricks {declarer nstricks} {
	if {"$declarer"=="north" || "$declarer"=="south"} {
	    return $nstricks
	}
	expr {13-$nstricks}
    }

    proc execute {arguments input cards} {
	variable executable

	if {![file exists $executable]} {
	    error "Could not find executable $executable"
	}

	set tmpfile [tempfile]
	set tmp [open $tmpfile w]
	puts $tmp $input
	foreach card $cards { puts $tmp $card }
	close $tmp
	
	#set infile [infile]
	#set in [open $infile w]
	lappend arguments -q $tmpfile

	set outfile [outfile]

	if {[file exists $outfile]} { file delete $outfile }

	#
	# If you have GIB version 3.3.x, you need to uncomment these line
	# Not require for previous version or later versions
	#
	#if {![file exists eval.dat]} {
	#   file copy {c:\Games\GIB\eval.dat} eval.dat
	#}

        set exec [linsert $arguments 0 exec $executable]
	set output [eval $exec]
	return $output
    }

    #
    # This uses the public domain gib.exe available on Matt Ginsberg's site.
    #
    proc nstricks {declarer denom {cards {}}} {
	global gib::executable

	set arguments [list -d -v]
	set input [head_gib $declarer $denom]
	    
	set output [execute $arguments $input $cards]
	set tricks ""
	if {![regexp {South can take ([0-9]+) trick} $output dummy tricks]} {
	    error "Bad GIB output:\n\n$output"
	}
	return $tricks
    }


    #
    # "tricks" - Determine the number of tricks declarer can
    # make in the denomination given.
    #
    proc tricks {declarer denom} {
	::deal::metadata "gib.$declarer.$denom" {
	    rectify_tricks $declarer [nstricks $declarer $denom]
	}
    }

    proc tricksCmd {declarer denom} {
      rectify_tricks $declarer [nstricks $declarer $denom]
    }
      

    #
    # determine which leads hold declarer to his tricks
    #
    proc holdingLeads {declarer denom} {
	global gib::executable
	
	set tricks [tricks $declarer $denom]
	if {$tricks==13} { return "No lead gives up a 14th trick" }
	set nstricks [rectify_tricks $declarer $tricks]
	if {$nstricks==$tricks} {
	    incr nstricks
	    set search "-"
	} else {
	    set search "+"
	}

	set hndl [open gibtest.txt w]
	puts $hndl "-d -w"
	puts $hndl [head_gib $declarer $denom]
	puts $hndl ""
	puts $hndl "."
	puts $hndl "g $nstricks"
	puts $hndl "p"
	puts $hndl "quit"
	close $hndl
	catch {set output [exec $executable "<gibtest.txt" ">giblead.txt"]}
	set leads [list]
	set regexp ""
	set fh [open "giblead.txt" "r"]
	append regexp {([SHDC][AKQJ2-9])} "\\" $search
	while {![eof $fh]} {
	    set line [gets $fh]
	    if {[regexp $regexp $line dummy card]} {
		lappend leads $card
	    }
	}

	return $leads    
    }
}

#
# The gib::library implements the ability to read files of the
# format of Matt Ginsberg's 'library.dat' file.  The file contains
# deals along with the double dummy results of all possible contracts
# played from all directions.
#
namespace eval gib::library {

    variable file "c:/Program Files/GIB/library.dat"
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
	    $hand is $holding($hand-spades) $holding($hand-hearts) \
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
	close $filehandle
	set filehandle {}
    }

    proc stackhand {name hand} {
	error "Can't stack hands when reading deals from files"
    }

    proc stackcards {name args} {
	error "Can't stack cards when reading deals from files"
    }

    namespace export get_next_deal initialize finalize
}

set deal::tricksCmd ::gib::tricksCmd
set deal::tricksCache gib
#gib::directory "c:/games/gib"
