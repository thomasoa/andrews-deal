
sdev randomInt

for {set i 0} {$i<10000} {incr i} {
   set value [rand 1000]
   if {$value<0 || $value>=1000} { puts stderr "Got value $value" }
   randomInt add $value
}

test-moe-sample rand-1000 [expr 999.0/2] [expr sqrt(999.0*1000.0/12)] 10000 [randomInt average]

sdev randomReal
for {set i 0} {$i<10000} {incr i} {
    set  value [rand]
    #if {$value<0 || $value>=1} { puts stderr "Got real value $value out of range" }
    randomReal add $value
}

test-moe-sample rand-real 0.5 [expr sqrt(1.0/12)] 10000 [randomReal average]
