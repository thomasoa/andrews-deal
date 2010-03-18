source lib/dominated.tcl

#
# The "dominated" routine returns the next smaller holding values
#
proc testDominated {holding expected} {
    if {$holding == ""} {
        set id "void"
    } else {
        set id $holding
    }
    test $id "dominated holding {$holding}" $expected
}

proc testDominatedHands {hand expected} {
   set id hand-[join hand ","]
   set output [lsort [dominatedHands $hand]]
   set expected [lsort $expected]
   if {$output==$expected} {
       pass $id
   } else {
       fail $id "Got sorted result: {$output} ; expected {$expected}"
   }
}

testDominated AKJx {AQJx AKTx}
testDominated AKJx {AQJx AKTx}
testDominated AQxxxx {KQxxxx AJxxxx}
testDominated AQJ {KQJ AQT}
testDominated AQTxx {KQTxx AJTxx AQxxx}
testDominated xxxxx {}
testDominated Txxxxxxxx {}
testDominated AKQJT {AKQJx}
testDominated KQJT {KQJx}
testDominated QJT {QJx}
testDominated JT {Jx}
testDominated AKQ {AKJ}
testDominated KQ {KJ}
testDominated {} {}

testDominatedHands {xxx xxx xxx xxxx} {}
testDominatedHands {xxx xxx Axx xxxx} [list {xxx xxx Kxx xxxx}]
testDominatedHands {xxx xxx Txx xxxx} [list {xxx xxx xxx xxxx}]
testDominatedHands {Txx xxx Txx xxxx} [list {Txx xxx xxx xxxx} {xxx xxx Txx xxxx}]
testDominatedHands {xxx QJx Axx xxxx} [list {xxx QTx Axx xxxx} {xxx QJx Kxx xxxx}]
testDominatedHands {xxx KJx Axx xxxx} [list {xxx QJx Axx xxxx} {xxx KTx Axx xxxx} {xxx KJx Kxx xxxx}]

rename testDominatedHands {}
rename testDominated {}
