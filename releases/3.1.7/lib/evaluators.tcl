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

#
# A set of routines to compute (quickly) the "offensive potential"
# of a hand.
#
# uses new 'holdingProc' declaration
#

set Losers(0) 0
set Losers(1) 1
set Losers(2) 2
set Losers(3) 3
set Losers(4) 4
set Losers(5) 4
set Losers(6) 3
set Losers(7) 3
set Losers(8) 2
set Losers(9) 2
set Losers(10) 2
set Losers(11) 1
set Losers(12) 1
set Losers(13) 0


#
# This returns 2X the number of expected defensive tricks
# from a suit.  This is a crude estimate.
#
holdingProc -double  defense {len A K Q J} {
	set defense 0
	if {$A} { incr defense 2 }
	if {$K && $len < 7} {
	    incr defense 2
	}
	if {$K && ($len==7)} { incr defense 1 }
	if {$Q && $len < 6} {
	    if {$A||$K} {
	        incr defense 2
	    } else {
		incr defense 1
	    }
	}
	if {$Q && $len==6} {
	    if {$A||$K} { incr defense 1 }
        }
	if {$len<=4 && $A && $K && !$Q && $J} { incr defense 1; }

	return [expr $defense/2.0]
}

holdingProc offense {len A K Q J T x9 x8} {
	global Losers
	set baselose $Losers($len)
	if {$baselose == 0} { return $len }
	if {$baselose == 1} {
		return [expr $len+$A]
	}
	if {$baselose == 2} {
	        if {$A && $K} { return $len; }
		if {$A || ($K && $Q)} { return [expr $len-1] }
		return [expr $len-2]
	}
	if {$baselose == 3} {
		if {$A&&$K&&$Q} { return $len }
		if {($A&&$K)||(($A||$K)&&$Q&&$J)} { return [expr $len-1] }
		if {$A||($K&&$Q)} { return [expr $len-2] }
		if {($Q||$K)&&$J&&($T||$x9)} { return [expr $len-2] }
		return [expr $len-3]
	}

	if {$A&&$K&&$Q&&($J||$T)} { return $len }
	if {3*($A+$K+$Q+$J)+$T+$x9>=10} { return [expr $len-1] }
	if {4*($A+$K+$Q+$J)+$T+$x9+$x8>=10} { return [expr $len-2]}
	if {20*$A+12*$K+12*$Q+6*$J+2*$T+$x9+$x8>20} { return [expr $len-3]}
	return [expr $len-4]
}

holdingProc -double OP {len A K Q J T x9 x8} {
	set defense [defense $len $A $K $Q $J]
	set offense [offense $len $A $K $Q $J $T $x9 $x8]
	return [expr $offense-2*$defense]
}

holdingProc Controls {A K} {expr {$A*2+$K}}

holdingProc HCP {A K Q J} {expr {$A*4+$K*3+$Q*2+$J}}
holdingProc HC321 {A K Q} {expr {$A*3+$K*2+$Q}}

#
# I've modeled this Tcl code against Danil Suits implementation
# of 'quality' and 'CCCC' from 'dealer.'
#
shapefunc CCCC.shapepoints {
   set pts 0
   # Add one point per singleton, two per doubleton, three per void
   foreach len [list $s $h $d $c] {
      if {$len<=2} { incr pts [expr {(3-$len)*100}] }
   }

   if {$pts==0} {
      # If flat, return -1/2.
      return -50
   }

   # Ignore first doubleton.
   expr {$pts-100}
}

holdingProc Quality {len A K Q J T x9 x8} {
   set SuitFactor [expr {10*$len}]
   set Quality [expr {$SuitFactor*[HCP $A $K $Q $J]}]

   set HigherHonors [expr {$A+$K+$Q+$J}]

   if {$len>6} {
      set ReplaceCount 3
      if {$Q} { incr ReplaceCount -2 }
      if {$J} { incr ReplaceCount -1 }
      if {$ReplaceCount > ($len-6)} { set ReplaceCount [expr {$len-6}] }

      incr Quality [expr {$ReplaceCount*$SuitFactor}]
   } else {
      if {$T} {
        if {$HigherHonors>1 || $J} {
	   incr Quality $SuitFactor
        } else {
           incr Quality [expr {$SuitFactor/2}]
        }
      }
      if {$x9} {
        if {$HigherHonors==2 || $T || $x8} {
           incr Quality [expr {$SuitFactor/2}]
        }
      }
   }

   return $Quality
}
   
holdingProc CCCC.holding {len A K Q J T x9 x8} {
   set HigherHonors 0
   set eval 0

   if {$A} {
      incr eval 300
      incr HigherHonors
   }

   if {$K} {
      if {$len==1} { incr eval 50 } else { incr eval 200 }
      incr HigherHonors
   }

   if {$Q} {
       if {$len<=2} {
	   if {$HigherHonors==1} {
	       incr eval 50
           }
       } else {
	   if {$HigherHonors==0} { incr eval 75 } { incr eval 100 }
       }
       incr HigherHonors
   }

   if {$J} {
       if {$len>2} {
	   if {$HigherHonors == 2} {
	       incr eval 50
	   }
	   if {$HigherHonors==1} {
	       incr eval 25
	   }
       }
       incr HigherHonors
   }

   if {$T} {
      if {$HigherHonors==2 || ($HigherHonors==1 && $x9)} {
	 incr eval 25
      }
   }
   incr eval [Quality $len $A $K $Q $J $T $x9 $x8]
   return $eval
}
proc CCCC {hand} {
    set p [CCCC.holding $hand]
    set dist [CCCC.shapepoints $hand] 
    expr {$p+$dist}
}
