#!./deal -x
#
# Test "holding contains ..."
#
test contains-1 {holding contains AKJ2 KJ} 1
test contains-2 {holding contains AKJ2 KQ} 0

#
# Test "holding containedIn ..."
#
test containedIn-1 {holding containedIn AKJ2 AKQJ32} 1
test containedIn-2 {holding containedIn AKJ2 AKQJ3} 0

#
# Test "holding matches ..."
#
test matches-1 {holding matches Axxx A876} 1
test matches-2 {holding matches KJxx KT32} 0
test matches-3 {holding matches KJxx KJT3} 0
test matches-4 {holding matches KJxx KJ92} 0
test matches-5 {holding matches KJxx KJ82} 1
test matches-6 {holding matches T KJxx KJ97} 1
test matches-7 {holding matches T KJxx KJT7} 0
test matches-8 {holding matches Axxx Axxx} 1
test matches-9 {holding matches Axxx A8xx} 1
test matches-10 {holding matches Axxx A85x} 1
test matches-11 {holding matches {} {}} 1

test decode-void {holding decode 0} {}
set number 1
foreach card {2 3 4 5 6 7 8 9 T J Q K A} {
  test decoding-$card "holding decode $number" $card
  set number [expr {$number*2}]
}

test disjoint-empty {holding disjoint} 1
test disjoint-1arg {holding disjoint AJ2} 1
test disjoint-true {holding disjoint AJ2 K43} 1
test disjoint-false {holding disjoint AJ2 K42} 0
