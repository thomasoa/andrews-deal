#!./deal -x
proc test {cmd expected} {
   set result [uplevel "$cmd"]
   if {$expected != $result} {
      puts "FAIL {$cmd} returned {$result}, expected {$expected}"
   } else {
      puts "PASS {$cmd} $expected"
   }
}

#
# Test "holding contains ..."
#
test {holding contains AKJ2 KJ} 1
test {holding contains AKJ2 KQ} 0

#
# Test "holding containedIn ..."
#
test {holding containedIn AKJ2 AKQJ32} 1
test {holding containedIn AKJ2 AKQJ3} 0

#
# Test "holding matches ..."
#
test {holding matches Axxx A876} 1
test {holding matches KJxx KT32} 0
test {holding matches KJxx KJT3} 0
test {holding matches KJxx KJ92} 0
test {holding matches KJxx KJ82} 1
test {holding matches T KJxx KJ97} 1
test {holding matches T KJxx KJT7} 0
test {holding matches Axxx Axxx} 1
test {holding matches Axxx A8xx} 1
test {holding matches Axxx A85x} 1
test {holding matches {} {}} 1

test {holding decode 0} {}
set number 1
foreach card {2 3 4 5 6 7 8 9 T J Q K A} {
  test "holding decode $number" $card
  set number [expr {$number*2}]
}

test {holding disjoint} 1
test {holding disjoint AJ2} 1
test {holding disjoint AJ2 K43} 1
test {holding disjoint AJ2 K42} 0
