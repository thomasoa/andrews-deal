source lib/auction.tcl

set auction [auction::create south]

test completed-init {$auction completed} 0
test contract-init {$auction contract} {{Passed Out}}
$auction call Pass
test completed-P {$auction completed} 0
test contract-P {$auction contract} {{Passed Out}}

$auction call Pass
test completed-PP {$auction completed} 0
test contract-PP {$auction contract} {{Passed Out}}

$auction call Pass
test completed-PPP {$auction completed} 0
test contract-PPP {$auction contract} {{Passed Out}}

$auction call Pass
test completed-PPPP {$auction completed} 1
test contract-PPPP {$auction contract} {{Passed Out}}

$auction delete

set auction [auction::create south]
$auction call 1S
test completed-1S {$auction completed} 0
test contract-1S {$auction contract} {1 spades south}
$auction call Pass
test completed-1SP {$auction completed} 0
test contract-1SP {$auction contract} {1 spades south}
$auction call Pass
test completed-1SPP {$auction completed} 0
test contract-1SPP {$auction contract} {1 spades south}
$auction call Pass
test completed-1SPPP {$auction completed} 1
test contract-1SPPP {$auction contract} {1 spades south}
$auction delete

set auction [auction::create south {1S Pass 2S}]
test contract-1SP2S {$auction contract} {2 spades south}
$auction call X
test contract-1SP2SX {$auction contract} {2 spades south doubled}
$auction call 3S
test contract-1SP2SX3S {$auction contract} {3 spades south}
$auction call X
test contract-1SP2SX3SX {$auction contract} {3 spades south doubled}
$auction call XX
test contract-1SP2SX3SXR {$auction contract} {3 spades south redoubled}
$auction call Pass
$auction call 4S
test contract-1SP2SX3SXR4S {$auction contract} {4 spades south}

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

test-error error-insufficient-1 {auction::create south {1D 1C}} {Insufficient bid: 1c}
test-error error-insufficient-2 {auction::create south {2H 2D}} {Insufficient bid: 2d}
test-error error-insufficient-2 {auction::create south {3S 3H}} {Insufficient bid: 3h}
test-error error-insufficient-2 {auction::create south {4N 4S}} {Insufficient bid: 4s}
test-error error-insufficient-5 {auction::create south {6C 5N}} {Insufficient bid: 5n}

test-error error-insufficient-6 {auction::create south {1D Pass 1C}} {Insufficient bid: 1c}
test-error error-insufficient-7 {auction::create south {2H Pass 2D}} {Insufficient bid: 2d}
test-error error-insufficient-8 {auction::create south {3S Pass 3H}} {Insufficient bid: 3h}
test-error error-insufficient-9 {auction::create south {4N Pass 4S}} {Insufficient bid: 4s}
test-error error-insufficient-10 {auction::create south {6C Pass 5N}} {Insufficient bid: 5n}

test-error error-insufficient-11 {auction::create south {1D Double 1C}} {Insufficient bid: 1c}
test-error error-insufficient-12 {auction::create south {2H Double 2D}} {Insufficient bid: 2d}
test-error error-insufficient-13 {auction::create south {3S Double 3H}} {Insufficient bid: 3h}
test-error error-insufficient-14 {auction::create south {4N Double 4S}} {Insufficient bid: 4s}
test-error error-insufficient-15 {auction::create south {6C Double 5N}} {Insufficient bid: 5n}

test-error error-insufficient-16 {auction::create south {1D Double Pass 1C}} {Insufficient bid: 1c}
test-error error-insufficient-17 {auction::create south {2H Double Pass 2D}} {Insufficient bid: 2d}
test-error error-insufficient-18 {auction::create south {3S Double Pass 3H}} {Insufficient bid: 3h}
test-error error-insufficient-19 {auction::create south {4N Double Pass 4S}} {Insufficient bid: 4s}
test-error error-insufficient-20 {auction::create south {6C Double Pass 5N}} {Insufficient bid: 5n}

test-error error-insufficient-21 {auction::create south {1D 1D}} {Insufficient bid: 1d}
test-error error-insufficient-22 {auction::create south {2H 2H}} {Insufficient bid: 2h}
test-error error-insufficient-23 {auction::create south {3S 3S}} {Insufficient bid: 3s}
test-error error-insufficient-24 {auction::create south {4N 4N}} {Insufficient bid: 4n}
test-error error-insufficient-25 {auction::create south {6C 6C}} {Insufficient bid: 6c}

test-error error-toomanycalls-1 {auction::create south {P P P P 1S}} {Auction already complete}
test-error error-toomanycalls-2 {auction::create south {1S P P P 2S}} {Auction already complete}
test-error error-toomanycalls-3 {auction::create south {1S X P P P 2S}} {Auction already complete}
test-error error-toomanycalls-4 {auction::create south {1S X XX P P P 2S}} {Auction already complete}
