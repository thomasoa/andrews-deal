##############################################
# Look for hand where north has 44 in the majors, a short minor,
# and 11-15 HCP.
#
# To execute:
#	deal -i ex/1-stack.tcl [num]
#
# This example uses the smart stacking algorithm to generate the
# same sorts of hands as example1 and example1-shapeclass. 
# The very first deal is slow - when the deal is requested a
# large "factory" object is built in memory - but every other
# deal generated after that first one is generated extremely quickly.
#
# In this example, you'd have to want about 1500 matches to the condition
# for the investment to break even.  But after 1500, the advantage of
# the smart stacking is huge.
#
# The payoff is even greater for rarer hand conditions.
##############################################

shapeclass roman_short_minor {expr $h==4 && $s==4 && ($c<=1 || $d<=1)}

deal::input smartstack north roman_short_minor HCP 11 15


set start [clock seconds]
set count 0
defvector HCP 4 3 2 1

proc flush_deal {} {
    global start
    global count
    set time [expr {[clock seconds]-$start}]
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
