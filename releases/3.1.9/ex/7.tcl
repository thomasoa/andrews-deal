# 
# A Certain poster to rec.games.bridge has been posting
# a computer generated bidding system that, to most bridge
# players eyes, looks somewhat deranged.
#

# bid.lib is a library of tcl routines which allow you
# to define opening bids and later check to see if a hand
# fits the conditions

source lib/bid.tcl

defvector AKQ 4 3 2
defvector Top4 1 1 1 1
defvector Aces 1
defvector None 0
proc top4spades {hand} {
	return [Top4 $hand spades]
}

# Usage:
#     describebid <bid> <shape condition> <function> <min> <max>
#
# Don Quixote's system:
# 
describebid 5D {$d>=8} AKQ 0 3
describebid 5C {$c>=8} controls 0 1
describebid 4S {$s==8} controls 0 3
describebid 4H {$h>=7} controls 0 1
describebid 4D {$d>=8} AKQ 5 7
describebid 4C {$h==8 && $d*$c*$s==3} Aces 1 1
describebid 3NT {$s==9 && ($h==2 || $d==2 || $c==2)} top4spades 2 4
describebid 3S {$s==7 && $d*$c*$h==6} AKQ 0 7
describebid 3H {$h==7} losers 16 17
describebid 3D {$d==7 && ($h==3 || $c==3 || $d==3)} hcp 3 3
describebid 3C {$c==7 && $d*$h*$s==4} AKQ 0 3
describebid 2NT {$s==7 && $c*$h*$d==8} hcp 4 5
describebid 2S {$s==7 && $c*$d*$h==4} hcp 3 5
describebid 2H {$h==7} Aces 0 0
describebid 2D {$d==7 && $h*$s*$c==6} hcp 2 5
describebid 2C {$c>=3 && $d>=3 && $h>=3 && $s>=3} hcp 22 23

#######################################
#
# This uses the proceducre "getbidlist", which is really
# a "shapefunc" defined in bid.lib .  It returns a list
# of all the possible described bids which can be made, given
# only the shape of the hand.  This turns out to be a very
# good optimization - rather than looping through all the
# bids, we only have to loop through a small subset.
#
# "checkcondition" then actually checks whether the hand
# is fits the other condition (which does a computation
# using the function provided and checks whether the hand is
# in the correct range.)
#
# This search looks for (south) hands which would open
# 2C under the above system.  What kind of system
# opens at the 2 level only 1/100 times, anyway?
# Bizarre.
#
#######################################

foreach bid $bidlist {
	set count($bid) 0
}
set accepted 0
set tried 0

main {
	incr tried
	foreach bid [getbidlist south] {
		if {[checkcondition $bid south]} {
			puts "$bid with [south]"
			incr count($bid)
			incr accepted
			accept
		}
	}
	reject
}

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
