shapecond gam3NT.shape {$s<=3&&$h<=3&&(($d>=7&&$c<=4)||($c>=7&&$d<=4))}

# return '1' if not compatible with a gambling notrump hand,
# '0' otherwise
holdingProc gam3NT.suit {A K Q J len} {

   if {$len>=7} {
     if {$len<=9&&$A&&$K&&$Q} { return 0}
     return 1
   }

   if {$len<=4} {
     if {$A||$K} { return 1 }
     return 0
   }

   return 1

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
