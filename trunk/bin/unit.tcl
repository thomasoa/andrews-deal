set unit_test(count) 0
set unit_test(fails) 0
set unit_test(totalcount) 0
set unit_test(totalfails) 0
set unit_test(verbosity) 1

# Borrowed lshift code from http://wiki.tcl.tk/918
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

proc test_prep {message code} {
    global unit_test
    if {$unit_test(verbosity)>0} {
        puts -nonewline stderr "$unit_test(context) $message"
    }

    uplevel $code

    if {$unit_test(verbosity)>0} {
        puts stderr ""
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
    # 99% confidence interval
    set moe [expr {1.29*sqrt(1.0/$sampleSize)}]
    set diff [expr {abs($sampleP-$expectedP)}]
    if {$diff>$moe} {
        set error [expr {100.0*($diff/$moe-1)}]
        #set min [expr {$expectedP-$moe}]
        #set max [expr {$expectedP+$moe}]
	fail $id [format {Sample out of 99%% confidence range: Prob: %.4f outside expected sample range of %.4f +/- %.4f ; outside range by %4.2f%%} $sampleP $expectedP $moe $error]
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

if {[llength $testFiles]==0} {
   set testFiles [glob -type f {tests/*.tcl}]
}

foreach testFile $testFiles {
    startContext $testFile
    source $testFile
    finishContext 
}

test_notify "Total: $unit_test(totalfails) failures out of $unit_test(totalcount) tests"

if {$unit_test(totalfails)>0} {
    exit 1
} else {
    exit 0
}
