#
# File: ex/6a.tcl
#
# This is a test which shows why you'd want to use a "shape class"
# or "shape function", which are fast lookup tables...
# 
# This is the slow version, which doesn't use the shape function
#
# It seeks hands where north and south have voids in the same
# suit.  (Alternatively, it finds hands where east and west have
# a 13-card fit.)
#
# See ex/6b.tcl and ex/6c.tcl for faster versions of the same search
#

main {
	accept unless {[spades south]!=0} {[spades north]!=0}
	accept unless {[hearts south]!=0} {[hearts north]!=0}
	accept unless {[diamonds south]!=0} {[diamonds north]!=0}
	accept unless {[clubs south]!=0} {[clubs north]!=0}
}

# It should be noted that this script is written so that
# the north hand is never even dealt unless the south hand
# has a void.  That's because, internally, 'deal' doesn't deal
# a hand unless information is requested about that hand.
#
# If the lines were:
#
#       accept if {[spades south]==0 && [spades north]==0}
#
# 'deal' would request information for *both* hands, and that would
# double the amount of work.  Hence the use of the slightly less clear
# 'accept unless' idiom.
#
# How much of an optimization is it to not deal the north hand?
# Profiling has shown me that a vast amount of time is spent in
# the randomnization routines, so cutting down on these calls
# is a significant time savings.
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
