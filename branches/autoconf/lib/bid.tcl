#
# Copyright (C) 1996-2001, Thomas Andrews
#
# $Id$
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

set bidlist ""

proc describebid {bid shapecond evaluator rangemin rangemax} {
	global handevaluator
	global brmin
	global brmax
	global bidlist
	lappend bidlist $bid

	shapecond bidshape$bid $shapecond
	
	set handevaluator($bid) $evaluator
	set brmin($bid) $rangemin
	set brmax($bid) $rangemax
}

proc checkcondition {bid hand} {
	global handevaluator
	global brmin
	global brmax
	if {![bidshape$bid $hand]} {
		return 0
	}
	set val [eval $handevaluator($bid) $hand]
	if {$val>=$brmin($bid) && $val<=$brmax($bid)} {
		return 1
	}
	return 0
}

shapefunc getbidlist {
	global bidlist
	set res ""
	foreach bid $bidlist {
		if {[bidshape$bid eval $s $h $d $c]} {
			lappend res $bid
		}
	}
	return $res
}
