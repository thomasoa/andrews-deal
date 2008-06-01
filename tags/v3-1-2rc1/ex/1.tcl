##############################################
# Look for hand where north has 44 in the majors, a short minor,
# and 11-15 HCP.
#
# Parts of this could be done more efficiently with a "shapeclass"
# command.
#
# To execute:
#	deal -i ex/1.tcl [num]
##############################################
main {
					  # Pitch deals
					  # where north does
					  # not have four spades
	reject if {[spades north]!=4}

					  # Pitch deals
					  # where north does
					  # not have four hearts
	reject if {[hearts north]!=4}

					  # Pitch deals
					  # where north has
					  # 2 or 3 diamonds
	set d [diamonds north]
	reject if {$d==2} {$d==3}

					  # Accept deals
					  # where north has
					  # 11-15 HCP.
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
