#############################
# When I first wrote this I was somewhat surprised.
#
# This deals hands where south opens a strong blue club and
# north has a 3+ controls (that is, he has a strong positive
# response.)
#
# I was surprised to find that slam was making 1/2 the time on these
# hands.  In other words, for Blue-clubbers, if the auction starts:
#
#	1C	1S/1NT/2C/2D
#
# there is a 50% chance that a slam should be bid...
#############################

main {
	reject unless {[controls north]>=3}
	set w [hcp south]
	reject unless {$w >=17}
	if {[balanced south]} {
		reject if {$w==17} {$w>=22 && $w<=24}
	}
	accept
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
