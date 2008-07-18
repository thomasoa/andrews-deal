# Great 88 problem 1 - Avoidance
set diagram {
           {A987 - A -} 
           {Q5 987 - -}
           {64 K2 2 -}
           {K2 AQ3 - -}
}

puts [dds -leader south -diagram $diagram south notrump]
# Should return 3
puts [dds -leader south -diagram $diagram -trick 4S south notrump]
# Should return 3

puts [dds -leader west -diagram $diagram -trick {4S KS 7S 5S} south notrump]
# Should return 3

puts [dds -leader north -diagram $diagram -trick {4S 2S AS 5S} -trick 7S south notrump]
# Should return 2

exit 0
