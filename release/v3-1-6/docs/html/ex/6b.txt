#
# File: ex/6b.tcl
#
# This implements the same search as ex/6a.tcl, using a simple
# shape class.
# 
# Basically, we use a shapeclass to immediately reject deals where
# south doesn't have a void.  This keeps us from doing unnecessary
# work, and practically doubles the speed of the search.
#
# The shape class "hasvoid" returns one (True) if there is a void
# somewhere in the hand and zero (False) otherwise.  It would
# seem like this is doing *more* work than example A, but the
# lookup in a shapeclass is much better optimized, and we break
# out of the loop if south doesn't have void, and move on to
# the next.
#
# example6c is about the same speed but uses an idiom worth
# seeing for other sorts of problems.
#

shapecond hasvoid {$s==0 || $h==0 || $d==0 || $c==0}

main {
	reject unless [hasvoid south]
	accept if {[spades south]==0 && [spades north]==0}
	accept if {[hearts south]==0 && [hearts north]==0}
	accept if {[diamonds south]==0 && [diamonds north]==0}
	accept if {[clubs south]==0 && [clubs north]==0}
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
