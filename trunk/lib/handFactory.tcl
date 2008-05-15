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

#
# This source file defines a handFactory class.
# It is primarily used by the "stackhand" input method,
# but it can be used in other ways as well.
#
# When the handFactory generates a hand, it does *not*
# change the status of the dealer - the current deal is not
# altered.
#
namespace eval handFactory {
    variable data
    set data(count) 0

    # dumb lookups
    set data(spades) 0
    set data(hearts) 1
    set data(diamonds) 2
    set data(clubs) 3
    set data(0) 0
    set data(1) 1
    set data(2) 2
    set data(3) 3

    #
    # Says that the data table is not up to date
    #
    proc invalidate {id} {
	variable data
	set data($id:invalid) 1
    }

    #
    # The constructor.  Very similar to the "stackhands" initializer,
    # only a handFactory does not know which seat it is going to.
    #
    proc create {{shapeclass AllShapes} {evaluator zerovector} {range {0 0}}} {
	variable data
	set id $data(count)

	incr data(count)
	proc pov$id {cmd args} "docmd \$cmd $id \$args"
	init $id $shapeclass $evaluator $range
	return "::handFactory::pov$id"
    }

    #
    # Utility procedure for dispatching object methods
    #
    proc docmd {command id arglist} {
	set cmd [list $command $id]
	foreach arg $arglist {
	    lappend cmd $arg
	}
	eval $cmd
    }

    proc init {id shapeclass evaluator range} {
	variable data
	set myshape [list]
	set value(list:0) [list]
	set value(list:1) [list]
	set value(list:2) [list]
	set value(list:3) [list]

	set data($id:evaluator) $evaluator
	set data($id:evalmin) [lindex $range 0]
	set data($id:evalmax) [lindex $range 1]
	set data($id:shapeclass) $shapeclass

	foreach suitnum {0 1 2 3} {
	    set data($id:$suitnum:requirements) [list]
	}
	
	# make invalid - means attempts to generate new deal will
	# construct new data table
	invalidate $id
    }

    proc restrictSuit {id suitname cardset {compare disjoint}} {
       variable data
       set len [holding length $cardset]
       if {$len>0} {
           set suitnum $data($suitname)
           addHoldingCond $id "holding $compare \$h $cardset" $suitnum
       }
    }

    proc restrictHand {id hand} {
       foreach cards $hand suitname {spades hearts diamonds clubs} {
          restrictSuit $id $suitname $cards
       }
    }

    proc addHoldingCond {id code args} {
	variable data
	if {[llength $args]==0} {
	    set suitnums [list 0 1 2 3]
	} else {
	    set suitnums [list]
	    foreach arg $args {
		lappend suitnums $data($arg)
	    }
	}

	foreach suitnum $suitnums {
	    lappend data($id:$suitnum:requirements) $code
	}
	invalidate $id
    }

    proc setShapeclass {id class} {
	variable data
	set pov($id:shapeclass) $class
	invalidate $id
    }

    proc setEvaluator {id hproc min max} {
	variable data
	set data($id:evaluator) $hproc
	set data($id:evalmin) $min
	set data($id:evalmax) $max
	invalidate $id
    }

    proc updateData {id} {
	variable data
	if {!$data($id:invalid)} { return }
	set evaluator $data($id:evaluator)
        #deal::debug starting to update data for handFactory $id

	set shapeclass $data($id:shapeclass)

        foreach shape [$shapeclass list] {
            foreach suitnum {0 1 2 3} len $shape {
                set lengths($suitnum:$len) 1
            }
        }

	foreach suitnum {0 1 2 3} {
	    set max 0
	    set min 13
	    set rqs $data($id:$suitnum:requirements)
	    foreachHolding h {
                #deal::debug holding $h in suit $suitnum
             
		set l [holding length $h]
                if {![info exists lengths($suitnum:$l)]} {
                    continue
                }
		set passed 1
		foreach rq $data($id:$suitnum:requirements) {
		    if {![eval $rq]} { set passed 0 ; break }
		}
		if {!$passed} {continue}

		# Okay, we've decided this holding is compatible
		
		if {$l>$max} { set max $l }
		if {$l<$min} { set min $l }

		set ev [$evaluator holding $h]

		if {![info exists value($suitnum:$ev)]} {
		    set value($suitnum:$ev) 1
		    lappend value(list:$suitnum) $ev
		}

		if {![info exists data($id:$suitnum:$l:$ev)]} {
		    set data($id:$suitnum:$l:$ev) [list]
		}

		lappend data($id:$suitnum:$l:$ev) $h

	    }
	    set data($id:$suitnum:max) $max
	    set data($id:$suitnum:min) $min
	}

	set emin $data($id:evalmin)
	set emax $data($id:evalmax)

	set data($id:values) [list]

	foreach s1 $value(list:0) {
	    foreach s2 $value(list:1) {
		foreach s3 $value(list:2) {
		    foreach s4 $value(list:3) {
			set tot [expr {$s1+$s2+$s3+$s4}]
			if {$tot>=$emin&&$tot<=$emax} {
			    lappend data($id:values) [list $s1 $s2 $s3 $s4]
			}
		    }
		}
	    }
	}

	set total 0.0
	set data($id:eshapes) [list]
	set countlist [list]
	set shapeclass $data($id:shapeclass)
	set code [list reject if] 
	foreach suitnum {0 1 2 3} v {s h d c} {
	    lappend code "\$$v>$data($id:$suitnum:max)" "\$$v<$data($id:$suitnum:min)"
	}
	lappend code "!(\[$shapeclass eval \$s \$h \$d \$c\])"

	shapeclass hfactory.shapeclass$id $code
	

	foreach shape [hfactory.shapeclass$id list] {

	    foreach valueset $data($id:values) {
		set count 1.0

		foreach suitnum {0 1 2 3} len $shape v $valueset {
		    if {[info exists data($id:$suitnum:$len:$v)]} {
			set count [expr {$count*[llength $data($id:$suitnum:$len:$v)]}]
		    } else {
			set count 0.0
		    }
		}

		if {$count>=1.0} {
		    lappend data($id:eshapes) [list $shape $valueset]
		    lappend countlist $count
		    set total [expr {$total+$count}]
		}
	    }
	}
    
	set sumprob 0.0

	set data($id:problist) [list]

        if ($total<0.5) {
            error "No such hands exist"
        }

	foreach shape $data($id:eshapes) count $countlist {
	    lappend data($id:sums) $sumprob
            set prob [expr {$count*1.0/$total}]
            lappend data($id:problist) $prob
	    set sumprob [expr {$sumprob+$prob}]
	}
	set data($id:invalid) 0
    }

    proc getdata {id {mult 1.0}} {
	updateData $id
	variable data
	set result [list]
	foreach eshape $data($id:eshapes) prob $data($id:problist) {
            lappend eshape [expr {$prob*$mult}]
	    lappend result $eshape
	}
	return $result
    }

    proc chooseshape {id} {
	variable data

	set shapes $data($id:eshapes)
	set probs $data($id:sums)
	set i 0
	set max [llength $probs]
	set j [expr {$max-1}]
	set rand [rand]

	# binary search code...
	while {$j>$i&&$i<$max} {
	    set mid [expr {int(($j+$i+1)/2)}]
	    if {[lindex $probs $mid]>$rand} {
		set j [expr {$mid-1}]
	    } else {
		set i $mid
	    }
	}
	return [lindex $shapes $i]
    }

    proc sample {id} {
	updateData $id
	variable data
	set pattern [chooseshape $id]
	set shape [lindex $pattern 0]
	set values [lindex $pattern 1]

	set hand [list]
	foreach suitnum {0 1 2 3} len $shape val $values {
	    set l [llength $data($id:$suitnum:$len:$val)]
	    lappend hand [lindex $data($id:$suitnum:$len:$val) [rand $l]]
	}
	return $hand
    }

}

defvector zerovector 0
