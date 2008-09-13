#
#
# To run:
#
#     deal -i ex/3nt-nostack.tcl
#
# Find deals where south is "AK K52 98765 962" and north has a gambling
# 3nt hand.
#
# This uses the definition: Solid 7-9 card minor suit, no 4-card major or
# 5+ card in the other minor, no controls outside the solid suit.
#
# This is slower than the smart-stacking version by an order of magnitude
# when requesting 1000 deals.
# 
source format/none

south is "AK K52 98765 962"

source ex/3nt-common.tcl

set diamonds 0
set clubs 0

main {
   reject unless {[gam3NT.shape north]}
   reject unless {0==[gam3NT.suit north]}
   if {[diamonds north]>=7} { incr diamonds } { incr clubs }
   puts "-- [north shape]"
   accept
}

deal_finished {
    #puts "Solid diamonds: $diamonds"
    #puts "Solid clubs   : $clubs"
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
