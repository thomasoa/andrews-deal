# You hold the south hand: 764 J4 J753 AQJ2
# and the auction has gone: 1S(W)-1NT-2NT-3NT by the opponents,
# who are playing 2/1.
# Choose a lead.

# In this example, I've assumed west has no side 4-card suit,
# and that he holds exactly 5 spades and 16-19 HCP.

# I've also assumed that East had some way to show a 5-card heart
# suit over 2NT, and hence, that he doesn't hold one, and also that
# east does not have spade support.

south is 764 J4 J753 AQJ2

shapecond balanced5S {$s==5&&($h*$d*$c==18)}

main {
	reject unless {[balanced5S west]}
	set w [hcp west]
	reject if {$w<16} {$w>19} 
	reject if {[hearts east]>4} {[spades east]>2}
	set e [hcp east]
	reject if {$e<6} {$e>11} {[losers east]<6}
	accept
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
