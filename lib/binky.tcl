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

source lib/handProc.tcl

namespace eval binky {
  variable binkyData
  set binkyData(nt) 0
  set binkyData(suit) 1

  proc add {key args} {
     variable binkyData
     set binkyData($key) $args
  } 

  proc ddlookup {key column} {
     variable binkyData
     if {[info exists binkyData($key)]} {
         set result [lindex $binkyData($key) $column]
         return $result
     }
     puts stderr "Got unknown key $key"
     return -1000.00
  }

  proc ddvalue {column holding len} {
     ddlookup $holding $column
  } 

  holdingProc -string binkyconvert {A K Q J T x9 len string} {

     regsub -all {[2-8]} $string x string
     if {$len==0} { return "-" }

     if {$len>=1 && $len<=5} { 
        return $string
     }

     return $string
     regsub 9 $string x string

     if {$len>=7} {
       regsub T $string x string
     }

     if {$len>=8} {
	 #regsub J $string x string
     }

     return $string
  }  

  proc initialize {} {
    variable binkyData
    set hbody {
        set converted [::binkyconvert holding $string]
        ::binky::ddvalue %d $converted $len
    }

    set sbody {
       ::binky::ddlookup [join [lsort -decreasing -integer [list $s $h $d $c]] "-"] %d
    }

    
    set body {
       expr {[binkys.%s %%h]+[binkyh.%s %%h]}
    }

    foreach contract {nt suit} {
       set column $binkyData($contract)
       holdingProc -double binkyh.$contract {x2 string len} [format $hbody $column]
       shapefunc binkys.$contract [format $sbody $column]
       handProc $contract [format $body $contract $contract]
       namespace export $contract
    }
  }
  source lib/binky-data.tcl
  initialize
}
