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
# In prior releases, this file was called the rather useless name
# "library"
#

#
# Determine if a hand is balanced and within a certain hcp range.
#
proc nt {seat min max} {
	if {[balanced $seat]} {
		set hcp [hcp $seat]
		return [expr {$hcp>=$min && $hcp<=$max}]
	}
	return 0
}

#
# In Roth's picture bidding, he allows 5-card majors to open
# 1NT. Roth_nt_shape needs to be defined
#
proc Roth_nt {seat min max} {
   if {[Roth_nt_shape $seat]}  {
	set hcp [hcp $seat]
        return [expr {$hcp>=$min && $hcp<=$max}]
   } else {
	return 0
   }
}

proc 5CM_nt {seat min max} {
    set s [spades $seat]
    set h [hearts $seat]
    set d [diamonds $seat]
    set c [clubs $seat]
    set count(0) 0
    set count(1) 0
    set count(4) 0
    set count(5) 0
    set count(6) 0
    set count(7) 0

    set count($s) 1
    set count($h) 1
    set count($d) 1
    set count($c) 1
    if {
	($count(1)==0) && 
	($count(0)==0) && 
	($count(6)==0) &&
	($count(7)==0)
    } {
	if {$count(5)==1 && $count(4)==1} { return 0 }

	set hcp [hcp $seat]
	return [expr {$hcp>=$min && $hcp<=$max}]
    }
    return 0
}

defvector Top0 0
defvector Top1 1
defvector Aces 1
defvector Top2 1 1
defvector Top3 1 1 1
defvector Top4 1 1 1 1
defvector Honors 1 1 1 1
defvector Top5 1 1 1 1 1
defvector Top6 1 1 1 1 1 1
defvector Top7 1 1 1 1 1 1 1
defvector Top5Q 2 2 2 1 1

defvector Ace 1
defvector King 0 1
defvector AceKing 1 1
defvector Queen 0 0 1
defvector JT 0 0 0 1 1

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
holdingProc defense {A K Q len} {
    set defense 0
    if {$A} { incr defense 2 }
    if {$K&&$len<7} {
        incr defense 2
    }
    if {$Q&&$len<6} {
        if {$A||$K} {
            incr defense 2
        } else {
            incr defense 1
        }
    }
    return $defense
}

proc defense {seat suit} {
	set len [$suit $seat]
	set defense 0
	if {[Ace $seat $suit]} {
	    incr defense 2
        }
	if {[King $seat $suit] && $len < 7} {
	    incr defense 2
	}
	if {[Queen $seat $suit] && $len < 6} {
	    if {[AceKing $seat $suit]>0} {
	        incr defense 2
	    } else {
		incr defense 1
	    }
	}
	return $defense
}

#
# This routine needs some work
#

defvector losers0_1 100
defvector losers0_2 50 50
defvector losers0_3 50 25 25
defvector losers0_4 40 30 20 10 10

defvector losers1_2 100 50 50
defvector losers1_3 50 50 30 20
defvector losers1_4 30 30 30 30 10 10

defvector losers2_3 100 50 50 30 20 20
defvector losers2_4 40 40 40 40 10 10 10

defvector losers3_4 100 60 60 30 10 5 5

proc offense {seat suit} {
	global Losers
	set len [$suit $seat]
	set baselose $Losers($len)
	if {$baselose == 0} { return $len }
	if {$baselose == 1} {
		return [expr $len-1+[Aces $seat $suit]];
	}
	if {$baselose == 2} {
		if {[losers0_2 $seat $suit]>=100} { return $len }
		if {[losers1_2 $seat $suit]>=100} { return [expr $len-1] }
		return [expr $len-2]
	}
	if {$baselose == 3} {
		if {[losers0_3 $seat $suit]>=100} { return $len }
		if {[losers1_3 $seat $suit]>=100} { return [expr $len-1] }
		if {[losers2_3 $seat $suit]>=100} { return [expr $len-2] }
		return [expr $len-3]
	}
	if {[losers0_4 $seat $suit]>=100} { return $len }
	if {[losers1_4 $seat $suit]>=100} { return [expr $len-1] }
	if {[losers2_4 $seat $suit]>=100} { return [expr $len-2] }
	if {[losers3_4 $seat $suit]>=100} { return [expr $len-3] }
	return [expr $len-4]
}

proc sOP {seat suit} {
	set offense [offense $seat $suit]
	set defense [defense $seat $suit]
	return [expr $offense-$defense]
}

proc OP {seat} {
	return [expr {
		[sOP $seat hearts] + [sOP $seat spades] 
		+ [sOP $seat diamonds] + [sOP $seat clubs]
	}
	]
}
	

proc thomas_opener {seat} {
	set hcp [hcp $seat]
	if {$hcp>12} {return 1}
	if {[balanced $seat] && $hcp==12} {return 1}
	if {[losers $seat]<=2+$hcp} {
		if {[Aces $seat]>1} {return 1}
		if {[OP $seat]<4} {return 1}
	}
	return 0
}

proc sound_opener {seat} {
	return [expr {
		([losers $seat]+38<=4*[hcp $seat]) ||
		[nt $seat 13 15]
	} ]
}

proc roth_opener {seat} {
	set h [dhcp $seat]
	if {$h>14} { return 1 }
	if {$h<12} { return 0 }
	set l [losers $seat]
	if {$l>8} { return 0 }
	if {$h>13} { return 1 }

	set count 0
	if {[Aces $seat]<1} { incr count }
	if {$l>6} {incr count }
	if {[spades $seat]<=1} {incr count }
	if {[hearts $seat]+[spades $seat]<4} {incr count}
	if {$h-$count>=12} { return 1 }
	return 0
}

proc standard_opening_suit {seat} {
    set s [spades $seat]
    set h [hearts $seat]
    set d [diamonds $seat]
    set c [clubs $seat]
    if {($s >= 5) && ($s >= $h) && ($s >= $d) && ($s >= $c)} {
	return "spades"
    }
    if {($h >=5) && ($h >= $d) && ($h >= $c)} {	
	return "hearts"
    }
    if {$d>$c} { return diamonds }
    if {$d==$c && $d>=5} { return diamonds }
    return "clubs"
}

proc solid_suit {seat suit} {
	expr "[Top3 $seat $suit]==3 && [$suit $seat]>=7"
}

proc gambling_nt {seat} {
	if {[hearts $seat]>=4 || [spades $seat]>=4} {return 0}
	return [expr {
		(([clubs $seat]<=3 && [solid_suit $seat diamonds]) ||
		 ([diamonds $seat]<=3 && [solid_suit $seat clubs])) &&
		[controls $seat] <= 4 &&
		[hcp $seat] <= 13
	}]
}

proc r1_control_suit {seat suit} {
	if {[$suit seat] = 0} {return 1}
	if {[Ace $seat] = 1} {return 1}
	return 0
}

proc r2_control_suit {seat suit} {
	if {[$suit seat] <= 1} {return 1}
	if {[King $seat] = 1} {return 1}
	return 0
}

proc weak_two_quality {seat suit} {
	if {[weak_two_qualvec $seat $suit] < 4}  {
		return 0
	}
	return 1
}

proc partner {seat} {
	if {$seat >= "south"} {
		if {$seat == "south"} { return "north" }
		return "east"
	}
	if {$seat == "north"} {return "south"}
	return "west"
}

holdingProc dhcp {A K Q J len} {
   if {$len==0} { return 0 }
   if {$len==1} { return [expr {$A*4+$K*2}]}
   if {$len==2} { return [expr {$A*4+$K*3+$Q}]}
   return [expr {$A*4+$K*3+$Q*2+$J}]
}

defvector newhcpvector 11 7 4 2 1
proc newhcp {seat args} {
	set total 0
	if {$args==""} {
		set suits "spades hearts diamonds clubs"
	} else {
		set suits $args
	}
	foreach suit "$suits" {
		set val [newhcpvector $seat $suit]
		set count [$suit $seat]
		if {$count==1 && $val<=7 && $val>=2} { incr val -2 }
		if {[Top4 $seat $suit]>=2} { incr val 2 }
		if {$count==1} {
			incr val 2
		}
		if {$count==0} {
			incr val 3
		}
		if {$count>=5} { incr val [expr 2*($count-4)] }
		incr total $val
	}
	return $total
}

holdingProc -string distinct_cards {A K Q J T x9 x8 x7 x6 x5 x4 x3 x2} {
  set L [list]
  set last 0
  foreach card {A K Q J T 9 8 7 6 5 4 3 2} \
	hasit [list $A $K $Q $J $T $x9 $x8 $x7 $x6 $x5 $x4 $x3 $x2] {
      if {$hasit & !$last} {
          lappend L $card
      }
      set last $hasit
  }
  set L
}
