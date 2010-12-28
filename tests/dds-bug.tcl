set diagram {
        {432 QJT9 32 AQJT}
        {A765 K32 87654 5}
        {KQ A8765 AQ K432}
        {JT98 4 KJT9 9876}
}

#
# First time I get the result of zero
#
test equal {dds -leader west -diagram $diagram -trick JS south hearts} 11
test equal {dds -leader west -diagram $diagram -trick JS south hearts} 11

