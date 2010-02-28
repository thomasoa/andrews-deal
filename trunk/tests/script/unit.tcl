set unit_test(count) 0
set unit_test(fails) 0
set unit_test(totalcount) 0
set unit_test(totalfails) 0
set unit_test(verbosity) 1

# Borrowed from http://wiki.tcl.tk/918
 #==========================================================
 # NAME    : lshift
 # PURPOSE : shift list and return first element
 # AUTHOR  : Richard Booth
 #           http://www.lehigh.edu/~rvb2
 #           rvb2@lehigh.edu rvbooth@agere.com
 # ---------------------------------------------------------
proc lshift {inputlist} {
    upvar $inputlist argv
    set arg  [lindex $argv 0]
    set argv [lreplace $argv[set argv {}] 0 0]
    return $arg
}

proc test_notify {string} {
    global unit_test
    if {$unit_test(verbosity)>0} {
        puts stderr $string
    }
}
   
proc startContext {context} {
    global unit_test
    set unit_test(count) 0
    set unit_test(fails) 0
    set unit_test(context) $context
}

proc finishContext {} {
    global unit_test
    incr unit_test(totalcount) $unit_test(count)
    incr unit_test(totalfails) $unit_test(fails)
    if {$unit_test(fails)>0 || $unit_test(verbosity)>0} {
        puts stderr "$unit_test(context): $unit_test(fails) failures out of $unit_test(count) tests"
    }
}

proc fail {id note} {
    global unit_test
    puts "FAIL $unit_test(context) $id $note"
    incr unit_test(count)
    incr unit_test(fails)
}

proc pass {id} {
    global unit_test
    incr unit_test(count)
    if {$unit_test(verbosity)>1} {
	puts "PASS $unit_test(context) $id"
    }
}

proc test {id cmd expected} {
    global unit_test
    set result [uplevel "$cmd"]
   
    if {$expected != $result} {
	fail $id "{$cmd} returned {$result}, expected {$expected}"
    } else {
	pass $id
    }
}

proc test-moe {id expectedP sampleSize sampleCount} {
    set sampleP [expr {1.0*$sampleCount/$sampleSize}]
    set moe [expr {1.96*sqrt($expectedP*(1-$expectedP)/($sampleSize-1))}]
    set min [expr {$expectedP-$moe}]
    set max [expr {$expectedP+$moe}]
    if {$sampleP<$min || $sampleP>$max} {
	fail $id [format "Expected sample probability between %.3f and %.3f, got %.3f" $min $max $sampleP]
    } else {
	pass $id
    }
}

set testFiles [list]
while {[llength $argv] > 0} {
  set arg [lshift argv]
  switch -- $arg {
      -silent { set unit_test(verbosity) 0 }
      -verbose { set unit_test(verbosity) 2 }
      default { lappend testFiles $arg }
   }
}

foreach testFile $testFiles {
    startContext $testFile
    source $testFile
    finishContext 
}

test_notify "AllTests: $unit_test(totalfails) failures out of $unit_test(totalcount) tests"

if {$unit_test(totalfails)>0} {
    exit 1
} else {
    exit 0
}
