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

shapeclass spade_shape {expr $s>=5 && $s>=$h && $d<=$s && $c<=$s}

shapeclass heart_shape {expr $h>=5 && $s<$h && $d<=$h && $c<=$h}

shapeclass diamond_shape {expr ($s<5 || $d>$s) && ($h<5 || $d>$h) && ($d>$c || ($d==$c && $d>=5))}

shapeclass club_shape {expr ($s<5 || $c>$s) && ($h<5 || $c>$h) && ($d<$c || ($d==$c && $c<5))}

