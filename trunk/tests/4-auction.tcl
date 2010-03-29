source lib/auction.tcl

set auction [auction::create south]

test completed-init {$auction completed} 0
$auction call Pass
test completed-P {$auction completed} 0
$auction call Pass
test completed-PP {$auction completed} 0
$auction call Pass
test completed-PPP {$auction completed} 0
$auction call Pass
test completed-PPPP {$auction completed} 1
$auction delete

set auction [auction::create south]
$auction call 1S
test completed-1S {$auction completed} 0
$auction call Pass
test completed-1SP {$auction completed} 0
$auction call Pass
test completed-1SPP {$auction completed} 0
$auction call Pass
test completed-1SPPP {$auction completed} 1
$auction delete

#
# Error cases
#
test-error error-first-double {auction::create south {X}} {Can't double when only passes}
test-error error-pass-double {auction::create south {P X}} {Can't double when only passes}
test-error error-2pass-double {auction::create south {P P X}} {Can't double when only passes}
test-error error-3pass-double {auction::create south {P P P X}} {Can't double when only passes}

test-error error-first-redouble {auction::create south {XX}} {Can't redouble a contract not doubled}
test-error error-pass-redouble {auction::create south {P XX}} {Can't redouble a contract not doubled}
test-error error-2pass-redouble {auction::create south {P P XX}} {Can't redouble a contract not doubled}
test-error error-3pass-redouble {auction::create south {P P P XX}} {Can't redouble a contract not doubled}

test-error error-double-self {auction::create south {1S P X}} {Can't double your sides own bid}
test-error error-redouble-self {auction::create south {1S X P XX}} {Can't redouble when you doubled}

test-error error-double-again {auction::create south {1S X P X}} {Can't double a contract already doubled}
test-error error-double-again {auction::create south {1S X X}} {Can't double a contract already doubled}

test-error error-insufficient {auction::create south {1S 1H}} {Insufficient bid: 1h}
