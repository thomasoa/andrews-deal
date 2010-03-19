#
# Tests sdev subcommands, "serialize" and "merge"
#
sdev sample
test serialize-empty {sample serialize} {0 0.0 0.0 0.0 0.0 0.0}
sample add 10
test serialize-1 {sample serialize} {1 1.0 10.0 100.0 10.0 10.0}
sample add 20
test serialize-2 {sample serialize} {2 2.0 30.0 500.0 10.0 20.0}

sdev sample2
sample2 merge [sample serialize]
test merge-1 {sample2 serialize} [sample serialize]
sample2 merge [sample serialize]
test merge-2 {sample2 serialize} {4 4.0 60.0 1000.0 10.0 20.0}

rename sample {}
rename sample2 {}

sdev sample3
sdev sample4
sdev total
for {set i 0} {$i<10000} {incr i} {
   set rand1 [rand]
   set rand2 [rand]
   sample3 add $rand1
   sample4 add $rand2
   total add $rand1 $rand2
}

sdev result
result merge [sample3 serialize]
result merge [sample4 serialize]

#
# We don't expect 100% accuracy due to the vaguries of floating point arithmetic
#
set error 0.00000000000001
test       merge-3-count {result count} [total count]
test-float merge-3-average {result average} [total average] $error
test-float merge-3-sdev {result sdev} [total sdev] $error
