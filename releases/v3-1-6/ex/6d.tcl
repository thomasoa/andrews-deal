#
# File: ex/6d.tcl
#
# This file uses a "holding function" to optimize a search for hands
# where north and south have voids in the same suit.
#
# The holding function "isVoid" returns a list of the suits in which
# the hand given has a void.  So if south is:
#
#       S ---                    S AK5                S KJ8
#       H 5432                   H A982               H Q932
#       D ---                    D 543                D AT9863
#       C AQT986543              C T92                C ----
#
# [voids south] will return: 
#
#      "spades diamonds"         ""                   "clubs"
#
# This is the same output as in the "voids" procedure for ex/6c.tcl,
# but it is implemented as a holding procedure rather than a shape
# procedure.
#
# We then run through this list and check if north has any void in
# common.
#
# This is considerably slower the 6b.tcl and 6c.tcl, though.

holdingProc -boolean voids {len} {
  expr {$len==0}
}

main {
	foreach suit [voids south] {
		accept if {[$suit north]==0}
	}
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
