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

rename testDominated {}
