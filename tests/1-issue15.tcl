
holdingProc -boolean AK {A K} {
    expr {$A&&$K}
}

set hand {AJxx AK K32 AKxx}

test 1 {AK hand $hand} {hearts clubs}
test 2 {AK hand $hand spades diamonds} {}
test 3 {AK hand $hand spades diamonds clubs hearts} {clubs hearts}
test 4 {AK hand $hand clubs} 1
test 5 {AK hand $hand diamonds} 0
test 6 {AK hand $hand hearts} 1
test 7 {AK hand $hand spades} 0

