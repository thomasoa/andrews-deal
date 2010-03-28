set value1 [expr {1*4+1*2.8+1*1.8+1*1.0}]
set value2 [expr {1*4+1*2.8+1*1.8}]
set value [expr {$value1 + $value1 + $value1 + $value2}]

sdev stat
for {set i 0} {$i<100} {incr i} {
   stat add $value
}

#
# This used to return -NaN due to very slight floating point inaccuracies
#
test-float regression-zero-sdev {stat sdev}  0.00
test-float average-1  {stat average} $value
