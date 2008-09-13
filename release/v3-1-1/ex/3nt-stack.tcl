#
# Find deals where south is "AK K52 98765 962" and north has a gambling
# 3nt hand.
#
# This was one of the first examples I gave of how "smart stacking" could
# be used.
#
# Compare output from this with output from ex/3nt-nostack.tcl to make
# sure that the relative odds are being obeyed, eg:
#
#   deal -i ex/3nt-stack.tcl 1000 | sort | uniq -c | sort -nr > stack.txt
#   deal -i ex/3nt-nostack.tcl 1000 | sort | uniq -c | sort -nr > nostack.txt
#
# Obviously, they shouldn't be exactly the same, but they should be similar.

source format/none

source ex/3nt-common.tcl

deal::input smartstack north gam3NT.shape gam3NT.suit 0 0
south is "AK K52 98765 962"

# Dump the data table from the smart stacking routine

#foreach line [smartstack::getdata] {
# puts $line
#}

main {
   reject unless {[gam3NT.shape north]}
   reject unless {0==[gam3NT.suit north]}
   puts "-- [north shape]"
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
