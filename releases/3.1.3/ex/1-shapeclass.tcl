##############################################
# Look for hand where north has 44 in the majors, a short minor,
# and 11-15 HCP.
#
# To execute:
#	deal -i ex/1-shapeclass.tcl [num]
##############################################

shapeclass roman_short_minor {expr {$h==4 && $s==4 && ($c<=1 || $d<=1)}}

main {
	reject unless {[roman_short_minor north]}
	set hcp_n [hcp north]
	accept if {$hcp_n>=11 && $hcp_n<=15}
}
##############################################



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
