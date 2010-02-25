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


proc handProc {name body} {
    set part(1) $body
    set part(2) $body
    set triples {
                %h $hand          {hand $hand}
                %S {$hand spades} {hand $hand spades]}
                %H {$hand hearts} {hand $hand hearts]}
                %D {$hand diamonds} {hand $hand diamonds]}
                %C {$hand clubs} {hand $hand clubs]}
	        %% % %
    }

    set procBody {
           if {0==[string compare $hand hand]} {
                set hand [lindex $args 0]
                set args [lrange $args 1 end]
                %2
           } else {
                %1
           }
    }
    foreach var {1 2} {
        foreach {code replace(1) replace(2)} $triples {
           regsub -all $code $part($var) $replace($var) part($var)
        }
        regsub -all "%$var" $procBody $part($var) procBody
    }

    uplevel [list proc $name {hand args} $procBody]
}
