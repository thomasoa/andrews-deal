#
# File: ex/6c.tcl
#
# This file uses a "shape function" to optimize a search for hands
# where north and south have voids in the same suit.
#
# The shape function "voids" returns a list of the suits in which
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
# We then run through this list and check if north has any void in
# common.
#
# This doesn't seem like it would be faster, but it is, because
# the shape function is computed up front as a table, and each call
# after the first is a quick lookup.  In fact, this is twice
# as fast as the script in ex/6a.tcl .
#
# It is only slightly faster than ex/6b.tcl, though.  The main
# reason I include it is to show a technique that can be used elsewhere.
# Another example which uses this is example7, a rather complicated routine...
#

shapefunc voids {
	set res ""
	if {$s==0} { lappend res spades }
	if {$h==0} { lappend res hearts }
	if {$d==0} { lappend res diamonds }
	if {$c==0} { lappend res clubs }
	return $res
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
