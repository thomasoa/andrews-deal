#
# A simple 'holding procedure' to determine high card points
#
holdingProc HCP {A K Q J} {
    expr {$A*4+$K*3+$Q*2+$J}
}

#
# The 'smartPoints' procedure incorporates two adjustments to
# the standard high card points.
#
# 1) A point is added for every card over 4 in a single suit
# 2) Honors in short suits are somewhat discounted
#
holdingProc smartPoints {length A K Q J} {
    if {$length>=5} {
	return [expr {$A*4+$K*3+$Q*2+$J+$length-4}]
    }
    if {$length==0} { return 0 }
    if {$length==1} {
	return [expr {$A*4+$K*2}]
    }
    if {$length==2} {
	return [expr {$A*4+$K*3+$Q}]
    }
    expr {$A*4+$K*3+$Q*2+$J}
}

#
# Find deals where north has less than 13 HCP but are compensated over
# 13 points by shape
#
main {
    reject if {[HCP north]>12}
    accept if {[smartPoints north]>12}
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
