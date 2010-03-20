sdev trickStats

north is {763 972 8753 A42} 
south is {AK94 AJ KQ4 KT76}

proc write_deal {} {
    trickStats add [deal::tricks south notrump]
}

proc flush_deal {} {
    puts [format {On average, partner makes %.2f tricks in notrump with standard deviation %.2f} [trickStats average] [trickStats sdev]]
}

