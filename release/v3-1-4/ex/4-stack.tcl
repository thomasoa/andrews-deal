#
# By agreement, you open a weak 2 with exactly 6 cards in your major,
# fewer than 4 cards in the other major, 5-10 HCP and either 2 of the
# top 3 or three of the top 5 in your suit.
#

holdingProc -boolean Weak2Quality {length A K Q J T} {
   accept if {$length<3}
   reject unless {$length==6}
   accept if {(($A+$K+$Q)>=2 || ($A+$K+$Q+$J+$T)>=3)}
}

shapecond Weak2Shape {$d<=3&&$c<=3&&(($h<=3&&$s==6)||($h==6&&$s<=3))}

deal::input smartstack south Weak2Shape hcp 5 10

smartstack::restrictHolding spades Weak2Quality 1 1
smartstack::restrictHolding hearts Weak2Quality 1 1

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
