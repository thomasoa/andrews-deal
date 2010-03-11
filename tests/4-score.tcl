source lib/score.tcl

test imps-1 {IMPs 110 110} 0
test imps-2 {IMPs 110 100} 0
test imps-3 {IMPs 100 110} 0

test imps-4 {IMPs 90 130} 1
test imps-5 {IMPs 130 90} -1
test imps-6 {IMPs 100 120} 1
test imps-7 {IMPs 120 100} -1

test imps-8 {IMPs 50 130} 2
test imps-9 {IMPs 130 50} -2
test imps-10 {IMPs 50 100} 2
test imps-11 {IMPs 100 50} -2

test imps-12 {IMPs 110 200} 3
test imps-13 {IMPs 200 110} -3
test imps-14 {IMPs 80 200} 3
test imps-15 {IMPs 200 80} -3
