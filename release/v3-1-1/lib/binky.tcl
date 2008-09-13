#
# Copyright (C) 1996-2001, Thomas Andrews
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

source lib/handProc.tcl

namespace eval binky {
  variable binkyData
  set binkyData(offense/nt) 0
  set binkyData(defense/nt) 1
  set binkyData(offense/suit) 2
  set binkyData(defense/suit) 3

  proc add {key args} {
     variable binkyData
     set binkyData($key) $args
  } 

  proc ddlookup {key column} {
     variable binkyData
     if {[info exists binkyData($key)]} {
         set result [lindex $binkyData($key) $column]
         return $result
     }
     return -13.00
  }

  proc ddvalue {column holding len} {
     ddlookup $holding $column
  } 

  holdingProc -string binkyconvert {A K Q J T x9 len string} {

     regsub -all {[2-8]} $string x string
     if {$len==0} { return "-" }

     if {$len>=1 && $len<=5} { 
        return $string
     }

     return $string
     regsub 9 $string x string

     if {$len>=7} {
       regsub T $string x string
     }

     if {$len>=8} {
	 #regsub J $string x string
     }

     return $string
  }  

  proc initialize {} {
    variable binkyData
    set hbody {
        set converted [::binkyconvert holding $string]
        ::binky::ddvalue %d $converted $len
    }

    set sbody {
       ::binky::ddlookup [join [lsort -decreasing -integer [list $s $h $d $c]] "-"] %d
    }

    
    set body {
       expr {[binkys.%s.%s %%h]+[binkyh.%s.%s %%h]}
    }

    foreach val {offense defense} {
       foreach contract {nt suit} {
          set column $binkyData($val/$contract)
          holdingProc -double binkyh.$val.$contract {x2 string len} [format $hbody $column]
          shapefunc binkys.$val.$contract [format $sbody $column]
          handProc $val.$contract [format $body $val $contract $val $contract]
          namespace export $val.$contract
       }
    } 
}
add               4-3-3-3  3.12   3.38   3.60   2.29 
add               4-4-3-2  3.07   3.34   3.89   2.39
add               5-3-3-2  3.04   3.44   3.94   2.24
add               5-4-2-2  2.98   3.40   4.19   2.28 
add               6-3-2-2  2.97   3.68   4.26   2.03 
add               4-4-4-1  3.06   3.32   4.41   2.57
add               5-4-3-1  3.00   3.40   4.46   2.42
add               6-3-3-1  3.01   3.69   4.52   2.18
add               7-2-2-2  2.93   4.05   4.60   1.77
add               6-4-2-1  2.95   3.64   4.75   2.17 
add               5-5-2-1  2.92   3.46   4.78   2.28 
add               7-3-2-1  2.92   4.03   4.81   1.85 
add               5-4-4-0  3.17   3.49   5.12   2.57 
add               8-2-2-1  2.85   4.44   5.15   1.52 
add               6-4-3-0  3.16   3.74   5.22   2.37
add               5-5-3-0  3.10   3.57   5.24   2.45
add               7-3-3-0  3.14   4.12   5.29   2.09
add               6-5-1-1  2.81   3.64   5.30   2.06
add               7-4-1-1  2.96   4.10   5.31   1.93 
add               8-3-1-1  2.94   4.58   5.39   1.61
add               7-4-2-0  3.12   4.17   5.52   2.04 
add               8-3-2-0  3.06   4.53   5.55   1.66
add               6-5-2-0  3.01   3.75   5.56   2.17 
add               9-2-1-1  2.59   4.79   5.64   1.08 
add               9-3-1-0  2.61   4.74   5.98   1.17 
add               9-2-2-0  2.97   5.17   5.99   1.33
add               8-4-1-0  3.05   4.61   6.03   1.67
add               7-5-1-0  2.99   4.21   6.10   1.90 
add               6-6-1-0  2.82   3.86   6.13   1.90 
add              10-1-1-1  1.86   5.53   6.19   1.27 
add              10-2-1-0  2.93   5.65   6.26   0.96
add               9-4-0-0  3.16   5.03   6.64   1.16 
add               7-6-0-0  3.14   4.02   6.71   1.64
add               8-5-0-0  3.01   4.40   6.75   1.48
add              10-3-0-0  1.98   0.39   6.77   1.73 
add              11-2-0-0  2.96   2.36   7.11   1.71 
add              11-1-1-0  2.96   2.37   7.11   1.70 
add              12-1-0-0  2.93   2.32   7.41   1.69 
add              13-0-0-0  0.89   0.26   7.68   1.68

add                    -   0.00   0.00   0.00   0.00

add                   x  -0.29  -0.29  -0.18  -0.21
add                   9  -0.26  -0.27  -0.16  -0.17
add                   T  -0.21  -0.20  -0.13  -0.15
add                   J  -0.07  -0.04  -0.04  -0.04
add                   Q   0.16   0.19   0.08   0.13
add                   K   0.61   0.61   0.31   0.44
add                   A   1.85   1.76   1.16   1.21
add                  xx  -0.76  -0.72  -0.48  -0.51
add                  9x  -0.73  -0.69  -0.46  -0.47
add                  Tx  -0.65  -0.61  -0.42  -0.42
add                  T9  -0.60  -0.58  -0.40  -0.39
add                  Jx  -0.44  -0.41  -0.31  -0.29
add                  J9  -0.39  -0.37  -0.28  -0.26
add                  JT  -0.33  -0.29  -0.22  -0.20
add                  Qx  -0.08  -0.08  -0.10  -0.06
add                  Q9  -0.08  -0.07  -0.09  -0.04
add                  QT   0.03   0.07  -0.02   0.03
add                  QJ   0.17   0.21   0.07   0.14
add                  Kx   0.73   0.66   0.45   0.49
add                  K9   0.77   0.71   0.49   0.52
add                  KT   0.86   0.79   0.53   0.58
add                  KJ   1.09   1.03   0.66   0.74
add                  KQ   1.37   1.34   0.87   0.95
add                  Ax   1.54   1.48   1.04   1.05
add                  A9   1.56   1.50   1.05   1.09
add                  AT   1.64   1.58   1.10   1.14
add                  AJ   1.84   1.79   1.23   1.27
add                  AQ   2.37   2.23   1.54   1.59
add                  AK   2.79   2.65   1.94   2.02

add                 xxx  -1.26  -1.18  -0.84  -0.87
add                 9xx  -1.23  -1.15  -0.82  -0.84
add                 Txx  -1.11  -1.04  -0.74  -0.76
add                 T9x  -1.06  -1.00  -0.71  -0.73
add                 Jxx  -0.84  -0.80  -0.59  -0.59
add                 J9x  -0.78  -0.74  -0.55  -0.54
add                 JTx  -0.68  -0.64  -0.47  -0.48
add                 JT9  -0.65  -0.59  -0.41  -0.43
add                 Qxx  -0.33  -0.34  -0.29  -0.29
add                 Q9x  -0.25  -0.27  -0.24  -0.23
add                 QTx  -0.08  -0.11  -0.15  -0.12
add                 QT9  -0.04  -0.07  -0.14  -0.06
add                 QJx   0.18   0.13  -0.01   0.04
add                 QJ9   0.19   0.15  -0.01   0.06
add                 QJT   0.30   0.25   0.07   0.13
add                 Kxx   0.30   0.27   0.20   0.20
add                 K9x   0.38   0.35   0.26   0.27
add                 KTx   0.53   0.49   0.35   0.36
add                 KT9   0.64   0.59   0.41   0.45
add                 KJx   0.83   0.78   0.54   0.57
add                 KJ9   0.89   0.82   0.57   0.64
add                 KJT   0.99   0.92   0.63   0.70
add                 Axx   1.05   1.03   0.76   0.72
add                 KQx   1.17   1.12   0.78   0.83
add                 KQ9   1.21   1.14   0.80   0.88
add                 A9x   1.13   1.11   0.82   0.80
add                 ATx   1.27   1.23   0.90   0.91
add                 KQT   1.35   1.28   0.91   0.95
add                 AT9   1.31   1.27   0.94   0.95
add                 KQJ   1.46   1.39   0.97   1.07
add                 AJx   1.59   1.51   1.10   1.10
add                 AJ9   1.65   1.59   1.17   1.15
add                 AJT   1.78   1.71   1.23   1.25
add                 AQx   1.97   1.90   1.40   1.39
add                 AQ9   2.04   1.98   1.44   1.45
add                 AQT   2.16   2.08   1.52   1.54
add                 AQJ   2.33   2.24   1.64   1.70
add                 AKx   2.42   2.37   1.76   1.79
add                 AK9   2.50   2.46   1.83   1.87
add                 AKT   2.64   2.57   1.91   1.98
add                 AKJ   2.83   2.77   2.04   2.12
add                 AKQ   3.03   3.02   2.24   2.35

add                xxxx  -1.69  -1.63  -1.16  -1.18
add                9xxx  -1.60  -1.55  -1.11  -1.12
add                Txxx  -1.44  -1.42  -1.02  -1.04
add                T9xx  -1.36  -1.34  -0.98  -0.98
add                Jxxx  -1.14  -1.15  -0.85  -0.87
add                J9xx  -1.04  -1.06  -0.80  -0.80
add                JTxx  -0.91  -0.95  -0.74  -0.73
add                JT9x  -0.86  -0.89  -0.71  -0.70
add                Qxxx  -0.75  -0.77  -0.59  -0.60
add                Q9xx  -0.62  -0.65  -0.50  -0.51
add                QTxx  -0.50  -0.54  -0.44  -0.43
add                QT9x  -0.38  -0.42  -0.36  -0.35
add                QJxx  -0.25  -0.29  -0.28  -0.27
add                QJ9x  -0.18  -0.22  -0.25  -0.21
add                QJTx  -0.10  -0.12  -0.17  -0.15
add                QJT9  -0.01   0.00  -0.12  -0.07
add                Kxxx  -0.16  -0.18  -0.11  -0.12
add                K9xx  -0.05  -0.08  -0.04  -0.05
add                KTxx   0.13   0.10   0.08   0.07
add                KT9x   0.23   0.18   0.12   0.14
add                KJxx   0.38   0.35   0.26   0.24
add                KJ9x   0.47   0.45   0.32   0.32
add                KJT9   0.53   0.49   0.37   0.34
add                KJTx   0.54   0.54   0.39   0.37
add                Axxx   0.56   0.55   0.45   0.42
add                KQxx   0.68   0.66   0.48   0.48
add                A9xx   0.68   0.67   0.53   0.51
add                KQ9x   0.75   0.76   0.53   0.54
add                KQT9   0.89   0.88   0.60   0.66
add                ATxx   0.84   0.81   0.63   0.61
add                KQTx   0.88   0.89   0.63   0.63
add                AT9x   0.95   0.91   0.69   0.69
add                KQJx   1.03   1.05   0.72   0.74
add                KQJT   1.18   1.23   0.78   0.87
add                AJxx   1.10   1.08   0.82   0.80
add                KQJ9   1.11   1.16   0.83   0.84
add                AJ9x   1.25   1.22   0.91   0.90
add                AJTx   1.35   1.35   0.98   0.97
add                AJT9   1.42   1.38   1.01   1.04
add                AQxx   1.48   1.47   1.11   1.08
add                AQ9x   1.56   1.54   1.17   1.13
add                AQTx   1.70   1.69   1.25   1.24
add                AQT9   1.77   1.79   1.34   1.32
add                AQJx   1.86   1.89   1.39   1.39
add                AQJ9   1.89   1.91   1.40   1.41
add                AKxx   1.92   1.93   1.47   1.50
add                AQJT   2.00   2.02   1.50   1.51
add                AK9x   2.02   2.03   1.54   1.57
add                AKTx   2.12   2.16   1.62   1.67
add                AKT9   2.17   2.21   1.66   1.64
add                AKJx   2.36   2.41   1.77   1.80
add                AKJT   2.43   2.49   1.83   1.90
add                AKJ9   2.46   2.52   1.85   1.89
add                AKQx   2.64   2.74   1.97   2.00
add                AKQT   2.78   2.88   2.06   2.10
add                AKQ9   2.75   2.90   2.10   2.07
add                AKQJ   2.86   3.02   2.15   2.13

add               xxxxx  -2.01  -2.07  -1.42  -1.29
add               9xxxx  -1.95  -2.02  -1.38  -1.27
add               Txxxx  -1.80  -1.86  -1.27  -1.20
add               T9xxx  -1.77  -1.84  -1.25  -1.17
add               Jxxxx  -1.54  -1.63  -1.15  -1.07
add               J9xxx  -1.44  -1.53  -1.05  -1.01
add               JTxxx  -1.40  -1.47  -1.01  -0.99
add               JT9xx  -1.32  -1.41  -0.96  -0.95
add               Qxxxx  -1.16  -1.24  -0.87  -0.85
add               Q9xxx  -1.05  -1.11  -0.78  -0.78
add               QTxxx  -0.96  -1.03  -0.74  -0.74
add               QT9xx  -0.88  -0.93  -0.66  -0.69
add               QJxxx  -0.71  -0.75  -0.56  -0.58
add               QJ9xx  -0.69  -0.72  -0.53  -0.58
add               QJTxx  -0.57  -0.59  -0.46  -0.51
add               Kxxxx  -0.56  -0.61  -0.40  -0.38
add               QJT9x  -0.51  -0.52  -0.39  -0.45
add               K9xxx  -0.46  -0.51  -0.31  -0.33
add               KTxxx  -0.30  -0.36  -0.23  -0.24
add               KT9xx  -0.22  -0.27  -0.15  -0.19
add               KJxxx  -0.07  -0.09  -0.06  -0.10
add               KJ9xx   0.00   0.01   0.01  -0.06
add               KJTxx   0.10   0.12   0.07   0.00
add               KJT9x   0.17   0.20   0.13   0.03
add               Axxxx   0.17   0.12   0.18   0.25
add               KQxxx   0.27   0.29   0.18   0.12
add               A9xxx   0.26   0.20   0.23   0.29
add               KQ9xx   0.33   0.36   0.24   0.17
add               KQTxx   0.48   0.53   0.32   0.24
add               ATxxx   0.42   0.38   0.34   0.37
add               KQT9x   0.48   0.57   0.35   0.27
add               AT9xx   0.54   0.49   0.40   0.47
add               KQJxx   0.62   0.73   0.42   0.33
add               KQJ9x   0.67   0.82   0.48   0.35
add               KQJTx   0.76   0.93   0.52   0.41
add               KQJT9   0.75   0.98   0.54   0.39
add               AJxxx   0.72   0.70   0.55   0.54
add               AJ9xx   0.83   0.83   0.60   0.59
add               AJTxx   0.96   0.99   0.69   0.65
add               AJT9x   0.99   1.04   0.75   0.68
add               AQxxx   1.09   1.10   0.81   0.80
add               AQ9xx   1.23   1.25   0.89   0.86
add               AQTxx   1.36   1.41   0.99   0.93
add               AQT9x   1.37   1.43   1.01   0.95
add               AQJxx   1.52   1.62   1.12   1.02
add               AQJ9x   1.60   1.72   1.16   1.08
add               AKxxx   1.57   1.61   1.19   1.23
add               AQJTx   1.71   1.83   1.23   1.11
add               AQJT9   1.71   1.83   1.24   1.15
add               AK9xx   1.67   1.73   1.26   1.26
add               AKTxx   1.81   1.89   1.34   1.33
add               AKT9x   1.88   1.97   1.39   1.35
add               AKJ9x   2.06   2.22   1.50   1.43
add               AKJxx   2.09   2.21   1.51   1.47
add               AKJT9   2.13   2.24   1.54   1.44
add               AKJTx   2.18   2.37   1.60   1.52
add               AKQxx   2.38   2.57   1.69   1.61
add               AKQ9x   2.44   2.67   1.75   1.64
add               AKQTx   2.58   2.83   1.82   1.69
add               AKQT9   2.65   2.84   1.83   1.71
add               AKQJx   2.79   3.07   1.92   1.77
add               AKQJT   2.89   3.14   1.96   1.79
add               AKQJ9   2.98   3.18   1.99   1.89

add              xxxxxx  -2.37  -2.62  -1.60  -1.36
add              9xxxxx  -2.35  -2.60  -1.59  -1.34
add              Txxxxx  -2.19  -2.44  -1.48  -1.28
add              T9xxxx  -2.17  -2.42  -1.46  -1.26
add              Jxxxxx  -1.98  -2.20  -1.34  -1.19
add              J9xxxx  -1.96  -2.18  -1.33  -1.17
add              JTxxxx  -1.83  -2.04  -1.21  -1.11
add              JT9xxx  -1.81  -2.02  -1.19  -1.09
add              Qxxxxx  -1.56  -1.76  -1.10  -0.97
add              Q9xxxx  -1.54  -1.74  -1.09  -0.96
add              QTxxxx  -1.42  -1.57  -0.96  -0.90
add              QT9xxx  -1.40  -1.55  -0.94  -0.88
add              QJxxxx  -1.18  -1.31  -0.78  -0.79
add              QJ9xxx  -1.16  -1.29  -0.76  -0.78
add              QJTxxx  -1.14  -1.24  -0.69  -0.76
add              QJT9xx  -1.12  -1.22  -0.67  -0.74
add              Kxxxxx  -0.94  -1.10  -0.63  -0.58
add              K9xxxx  -0.93  -1.08  -0.61  -0.56
add              KTxxxx  -0.73  -0.85  -0.49  -0.49
add              KT9xxx  -0.71  -0.83  -0.48  -0.47
add              KJxxxx  -0.48  -0.55  -0.34  -0.39
add              KJ9xxx  -0.46  -0.53  -0.33  -0.37
add              KJTxxx  -0.34  -0.34  -0.20  -0.30
add              KJT9xx  -0.33  -0.32  -0.18  -0.29
add              KQxxxx  -0.15  -0.14  -0.10  -0.17
add              KQ9xxx  -0.13  -0.12  -0.08  -0.15
add              Axxxxx  -0.20  -0.30  -0.07   0.13
add              A9xxxx  -0.18  -0.29  -0.06   0.15
add              KQTxxx   0.01   0.10   0.03  -0.08
add              KQT9xx   0.03   0.11   0.04  -0.06
add              ATxxxx   0.08  -0.02   0.09   0.23
add              AT9xxx   0.10   0.00   0.11   0.24
add              KQJxxx   0.22   0.39   0.19  -0.01
add              KQJ9xx   0.24   0.40   0.20   0.00
add              AJxxxx   0.36   0.34   0.28   0.35
add              KQJTxx   0.30   0.48   0.28   0.07
add              AJ9xxx   0.38   0.36   0.29   0.36
add              KQJT9x   0.32   0.50   0.30   0.08
add              AJTxxx   0.55   0.57   0.40   0.43
add              AJT9xx   0.57   0.59   0.41   0.45
add              AQxxxx   0.73   0.76   0.49   0.55
add              AQ9xxx   0.75   0.78   0.50   0.56
add              AQTxxx   0.98   1.05   0.67   0.63
add              AQT9xx   1.00   1.07   0.68   0.65
add              AQJxxx   1.21   1.34   0.80   0.76
add              AQJ9xx   1.23   1.36   0.82   0.77
add              AQJTxx   1.26   1.43   0.85   0.75
add              AQJT9x   1.28   1.45   0.86   0.76
add              AKxxxx   1.37   1.47   0.96   0.98
add              AK9xxx   1.39   1.48   0.97   1.00
add              AKTxxx   1.55   1.70   1.06   1.02
add              AKT9xx   1.57   1.72   1.07   1.03
add              AKJxxx   1.85   2.06   1.23   1.15
add              AKJ9xx   1.87   2.08   1.24   1.16
add              AKJTxx   1.97   2.24   1.30   1.15
add              AKJT9x   1.98   2.25   1.31   1.16
add              AKQxxx   2.21   2.57   1.42   1.26
add              AKQ9xx   2.23   2.58   1.44   1.27
add              AKQTxx   2.41   2.81   1.53   1.33
add              AKQT9x   2.43   2.82   1.54   1.35
add              AKQJxx   2.55   3.01   1.60   1.36
add              AKQJ9x   2.56   3.03   1.61   1.37
add              AKQJTx   2.69   3.12   1.65   1.40
add              AKQJT9   2.71   3.14   1.66   1.41

add             xxxxxxx  -2.65  -3.21  -1.72  -1.34
add             9xxxxxx  -2.63  -3.19  -1.71  -1.32
add             Txxxxxx  -2.60  -3.16  -1.69  -1.30
add             T9xxxxx  -2.58  -3.15  -1.68  -1.28
add             Jxxxxxx  -2.36  -2.83  -1.47  -1.24
add             J9xxxxx  -2.35  -2.81  -1.45  -1.23
add             JTxxxxx  -2.31  -2.78  -1.43  -1.20
add             JT9xxxx  -2.29  -2.76  -1.42  -1.19
add             Qxxxxxx  -2.00  -2.38  -1.21  -1.05
add             Q9xxxxx  -1.98  -2.37  -1.20  -1.03
add             QTxxxxx  -1.95  -2.33  -1.18  -1.01
add             QT9xxxx  -1.93  -2.32  -1.16  -0.99
add             QJxxxxx  -1.70  -2.04  -0.98  -0.90
add             QJ9xxxx  -1.69  -2.02  -0.96  -0.89
add             QJTxxxx  -1.65  -1.98  -0.94  -0.86
add             QJT9xxx  -1.63  -1.97  -0.93  -0.84
add             Kxxxxxx  -1.31  -1.61  -0.86  -0.77
add             K9xxxxx  -1.30  -1.60  -0.84  -0.76
add             KTxxxxx  -1.27  -1.56  -0.82  -0.73
add             KT9xxxx  -1.25  -1.55  -0.81  -0.72
add             KJxxxxx  -0.94  -1.10  -0.55  -0.60
add             KJ9xxxx  -0.93  -1.09  -0.54  -0.58
add             KJTxxxx  -0.89  -1.05  -0.52  -0.56
add             KJT9xxx  -0.87  -1.03  -0.50  -0.54
add             KQxxxxx  -0.60  -0.63  -0.28  -0.40
add             KQ9xxxx  -0.58  -0.61  -0.26  -0.38
add             KQTxxxx  -0.55  -0.58  -0.24  -0.36
add             Axxxxxx  -0.45  -0.61  -0.23   0.03
add             KQT9xxx  -0.53  -0.56  -0.23  -0.34
add             A9xxxxx  -0.43  -0.60  -0.22   0.04
add             ATxxxxx  -0.40  -0.56  -0.20   0.06
add             AT9xxxx  -0.38  -0.55  -0.19   0.08
add             KQJxxxx  -0.33  -0.24  -0.07  -0.29
add             KQJ9xxx  -0.31  -0.23  -0.05  -0.28
add             KQJTxxx  -0.28  -0.19  -0.03  -0.25
add             KQJT9xx  -0.26  -0.18  -0.02  -0.24
add             AJxxxxx   0.09   0.02   0.05   0.18
add             AJ9xxxx   0.11   0.03   0.06   0.20
add             AJTxxxx   0.15   0.07   0.08   0.22
add             AJT9xxx   0.17   0.09   0.10   0.23
add             AQxxxxx   0.49   0.53   0.29   0.37
add             AQ9xxxx   0.51   0.54   0.30   0.38
add             AQTxxxx   0.54   0.58   0.32   0.40
add             AQT9xxx   0.56   0.59   0.33   0.42
add             AQJxxxx   0.95   1.08   0.56   0.51
add             AQJ9xxx   0.96   1.10   0.58   0.53
add             AQJTxxx   1.00   1.14   0.60   0.55
add             AQJT9xx   1.02   1.15   0.61   0.56
add             AKxxxxx   1.15   1.36   0.70   0.71
add             AK9xxxx   1.16   1.38   0.71   0.73
add             AKTxxxx   1.20   1.41   0.73   0.75
add             AKT9xxx   1.21   1.43   0.74   0.76
add             AKJxxxx   1.58   1.95   0.93   0.80
add             AKJ9xxx   1.60   1.97   0.94   0.81
add             AKJTxxx   1.63   2.00   0.96   0.83
add             AKJT9xx   1.65   2.02   0.97   0.85
add             AKQxxxx   2.05   2.58   1.17   0.93
add             AKQ9xxx   2.06   2.60   1.18   0.95
add             AKQTxxx   2.10   2.63   1.20   0.97
add             AKQT9xx   2.11   2.64   1.21   0.98
add             AKQJxxx   2.33   2.89   1.30   1.03
add             AKQJ9xx   2.34   2.91   1.31   1.04
add             AKQJTxx   2.37   2.94   1.33   1.06
add             AKQJT9x   2.39   2.95   1.34   1.07

add            9xxxxxxx  -3.02  -3.87  -1.76  -1.34
add            Txxxxxxx  -2.99  -3.84  -1.75  -1.32
add            T9xxxxxx  -2.98  -3.82  -1.74  -1.31
add            Jxxxxxxx  -2.93  -3.77  -1.71  -1.28
add            J9xxxxxx  -2.91  -3.76  -1.70  -1.26
add            JTxxxxxx  -2.88  -3.73  -1.68  -1.24
add            JT9xxxxx  -2.87  -3.71  -1.67  -1.23
add            Qxxxxxxx  -2.38  -3.05  -1.29  -1.12
add            Q9xxxxxx  -2.37  -3.04  -1.28  -1.10
add            QTxxxxxx  -2.34  -3.01  -1.27  -1.08
add            QT9xxxxx  -2.32  -2.99  -1.25  -1.07
add            QJxxxxxx  -2.27  -2.93  -1.22  -1.03
add            QJ9xxxxx  -2.25  -2.92  -1.21  -1.02
add            QJTxxxxx  -2.22  -2.88  -1.19  -1.00
add            QJT9xxxx  -2.20  -2.87  -1.18  -0.98
add            Kxxxxxxx  -1.44  -1.88  -0.81  -0.77
add            K9xxxxxx  -1.42  -1.87  -0.80  -0.76
add            KTxxxxxx  -1.39  -1.84  -0.78  -0.74
add            KT9xxxxx  -1.38  -1.83  -0.77  -0.72
add            KJxxxxxx  -1.33  -1.77  -0.74  -0.69
add            KJ9xxxxx  -1.31  -1.76  -0.73  -0.68
add            KJTxxxxx  -1.28  -1.72  -0.71  -0.66
add            KJT9xxxx  -1.26  -1.71  -0.70  -0.64
add            KQxxxxxx  -1.15  -1.36  -0.44  -0.63
add            KQ9xxxxx  -1.13  -1.35  -0.43  -0.61
add            KQTxxxxx  -1.10  -1.32  -0.41  -0.59
add            KQT9xxxx  -1.09  -1.30  -0.40  -0.58
add            KQJxxxxx  -1.03  -1.24  -0.37  -0.54
add            KQJ9xxxx  -1.01  -1.23  -0.35  -0.53
add            KQJTxxxx  -0.98  -1.20  -0.34  -0.51
add            KQJT9xxx  -0.97  -1.19  -0.33  -0.49
add            Axxxxxxx  -0.52  -0.71  -0.26   0.04
add            A9xxxxxx  -0.50  -0.70  -0.25   0.06
add            ATxxxxxx  -0.47  -0.67  -0.23   0.08
add            AT9xxxxx  -0.46  -0.65  -0.22   0.09
add            AJxxxxxx  -0.41  -0.60  -0.19   0.12
add            AJ9xxxxx  -0.39  -0.59  -0.18   0.14
add            AJTxxxxx  -0.36  -0.55  -0.16   0.16
add            AJT9xxxx  -0.34  -0.54  -0.15   0.17
add            AQxxxxxx   0.42   0.41   0.20   0.27
add            AQ9xxxxx   0.44   0.43   0.21   0.29
add            AQTxxxxx   0.47   0.46   0.23   0.30
add            AQT9xxxx   0.48   0.47   0.24   0.32
add            AQJxxxxx   0.54   0.53   0.27   0.35
add            AQJ9xxxx   0.56   0.55   0.28   0.37
add            AQJTxxxx   0.59   0.58   0.30   0.39
add            AQJT9xxx   0.60   0.59   0.31   0.40
add            AKxxxxxx   0.98   1.38   0.50   0.45
add            AK9xxxxx   0.99   1.39   0.51   0.46
add            AKTxxxxx   1.03   1.42   0.53   0.48
add            AKT9xxxx   1.04   1.44   0.54   0.49
add            AKJxxxxx   1.09   1.49   0.57   0.52
add            AKJ9xxxx   1.11   1.50   0.58   0.54
add            AKJTxxxx   1.14   1.54   0.60   0.56
add            AKJT9xxx   1.16   1.55   0.61   0.57
add            AKQxxxxx   1.82   2.46   0.86   0.63
add            AKQ9xxxx   1.83   2.48   0.88   0.64
add            AKQTxxxx   1.86   2.51   0.89   0.66
add            AKQT9xxx   1.87   2.52   0.90   0.67
add            AKQJxxxx   1.93   2.58   0.93   0.71
add            AKQJ9xxx   1.95   2.59   0.94   0.72
add            AKQJTxxx   1.98   2.62   0.96   0.74
add            AKQJT9xx   1.99   2.63   0.97   0.75

add           T9xxxxxxx  -4.22  -6.58  -2.55  -1.65
add           J9xxxxxxx  -4.17  -6.51  -2.52  -1.60
add           JTxxxxxxx  -4.13  -6.49  -2.50  -1.59
add           JT9xxxxxx  -4.12  -6.47  -2.49  -1.57
add           Q9xxxxxxx  -2.98  -4.33  -1.64  -1.18
add           QTxxxxxxx  -2.95  -4.30  -1.62  -1.16
add           QT9xxxxxx  -2.94  -4.29  -1.61  -1.15
add           QJxxxxxxx  -2.88  -4.23  -1.58  -1.11
add           QJ9xxxxxx  -2.87  -4.22  -1.57  -1.10
add           QJTxxxxxx  -2.84  -4.19  -1.56  -1.08
add           QJT9xxxxx  -2.83  -4.18  -1.54  -1.07
add           K9xxxxxxx  -1.57  -2.79  -0.97  -0.72
add           KTxxxxxxx  -1.54  -2.76  -0.96  -0.70
add           KT9xxxxxx  -1.53  -2.75  -0.95  -0.69
add           KJxxxxxxx  -1.48  -2.70  -0.92  -0.66
add           KJ9xxxxxx  -1.47  -2.69  -0.91  -0.65
add           KJTxxxxxx  -1.44  -2.66  -0.89  -0.63
add           KJT9xxxxx  -1.43  -2.65  -0.88  -0.61
add           KQxxxxxxx  -1.80  -2.59  -0.62  -0.70
add           KQ9xxxxxx  -1.79  -2.58  -0.61  -0.69
add           KQTxxxxxx  -1.76  -2.55  -0.59  -0.67
add           A9xxxxxxx  -1.30  -1.45  -0.58  -0.36
add           KQT9xxxxx  -1.75  -2.54  -0.58  -0.65
add           ATxxxxxxx  -1.27  -1.42  -0.57  -0.34
add           AT9xxxxxx  -1.26  -1.41  -0.56  -0.33
add           KQJxxxxxx  -1.69  -2.48  -0.55  -0.62
add           KQJ9xxxxx  -1.68  -2.47  -0.54  -0.61
add           AJxxxxxxx  -1.21  -1.35  -0.53  -0.30
add           KQJTxxxxx  -1.65  -2.44  -0.53  -0.59
add           AJ9xxxxxx  -1.20  -1.34  -0.52  -0.28
add           KQJT9xxxx  -1.64  -2.43  -0.51  -0.58
add           AJTxxxxxx  -1.17  -1.31  -0.51  -0.27
add           AJT9xxxxx  -1.16  -1.30  -0.49  -0.25
add           AQxxxxxxx   0.22   0.03  -0.09   0.14
add           AQ9xxxxxx   0.24   0.04  -0.08   0.15
add           AQTxxxxxx   0.27   0.07  -0.06   0.17
add           AQT9xxxxx   0.28   0.08  -0.05   0.18
add           AQJxxxxxx   0.33   0.14  -0.02   0.21
add           AQJ9xxxxx   0.35   0.15  -0.01   0.23
add           AQJTxxxxx   0.38   0.18   0.01   0.24
add           AQJT9xxxx   0.39   0.20   0.02   0.26
add           AKxxxxxxx   1.06   1.41   0.35   0.20
add           AK9xxxxxx   1.08   1.42   0.35   0.21
add           AKTxxxxxx   1.10   1.45   0.37   0.23
add           AKT9xxxxx   1.11   1.46   0.38   0.24
add           AKJxxxxxx   1.17   1.51   0.41   0.27
add           AKJ9xxxxx   1.18   1.52   0.42   0.28
add           AKJTxxxxx   1.21   1.56   0.43   0.30
add           AKJT9xxxx   1.22   1.57   0.44   0.31
add           AKQxxxxxx   1.15   2.04   0.69   0.43
add           AKQ9xxxxx   1.16   2.05   0.70   0.44
add           AKQTxxxxx   1.19   2.08   0.72   0.46
add           AKQT9xxxx   1.20   2.09   0.73   0.47
add           AKQJxxxxx   1.25   2.14   0.76   0.50
add           AKQJ9xxxx   1.26   2.15   0.77   0.51
add           AKQJTxxxx   1.29   2.18   0.78   0.53
add           AKQJT9xxx   1.30   2.19   0.79   0.54

add          KT9xxxxxxx  -3.10  -3.97  -1.95  -1.32
add          KJ9xxxxxxx  -3.04  -3.91  -1.91  -1.29
add          KJTxxxxxxx  -3.02  -3.88  -1.90  -1.27
add          KJT9xxxxxx  -3.01  -3.88  -1.89  -1.26
add          AT9xxxxxxx  -3.25  -0.64  -1.20  -1.10
add          AJ9xxxxxxx  -3.20  -0.58  -1.17  -1.06
add          AJTxxxxxxx  -3.17  -0.56  -1.15  -1.05
add          AJT9xxxxxx  -3.16  -0.55  -1.15  -1.03
add          KQ9xxxxxxx  -2.63  -4.31  -0.75  -0.51
add          KQTxxxxxxx  -2.61  -4.28  -0.73  -0.49
add          KQT9xxxxxx  -2.60  -4.27  -0.72  -0.48
add          KQJxxxxxxx  -2.54  -4.21  -0.69  -0.45
add          KQJ9xxxxxx  -2.53  -4.21  -0.69  -0.44
add          KQJTxxxxxx  -2.51  -4.18  -0.67  -0.43
add          KQJT9xxxxx  -2.50  -4.17  -0.66  -0.41
add          QT9xxxxxxx  -1.78  -5.14  -0.57   0.04
add          QJ9xxxxxxx  -1.72  -5.08  -0.53   0.08
add          QJTxxxxxxx  -1.70  -5.05  -0.52   0.09
add          QJT9xxxxxx  -1.68  -5.04  -0.51   0.11
add          AK9xxxxxxx   1.95   1.04   0.02   0.33
add          AKTxxxxxxx   1.97   1.07   0.04   0.34
add          AKT9xxxxxx   1.98   1.08   0.05   0.35
add          AKJxxxxxxx   2.03   1.13   0.07   0.38
add          AKJ9xxxxxx   2.04   1.14   0.08   0.39
add          AKJTxxxxxx   2.06   1.17   0.10   0.41
add          AKJT9xxxxx   2.07   1.17   0.10   0.42
add          AQ9xxxxxxx   0.90   2.22   0.18   0.65
add          AQTxxxxxxx   0.92   2.25   0.19   0.66
add          AQT9xxxxxx   0.93   2.26   0.20   0.67
add          AQJxxxxxxx   0.98   2.31   0.23   0.70
add          AQJ9xxxxxx   1.00   2.32   0.24   0.72
add          AQJTxxxxxx   1.02   2.35   0.25   0.73
add          AQJT9xxxxx   1.04   2.36   0.26   0.74
add          JT9xxxxxxx  -0.88  -4.24   0.37  -0.84
add          AKQxxxxxxx   0.85   1.50   0.48  -0.02
add          AKQ9xxxxxx   0.86   1.51   0.49  -0.01
add          AKQTxxxxxx   0.89   1.53   0.50   0.01
add          AKQT9xxxxx   0.90   1.54   0.51   0.02
add          AKQJxxxxxx   0.95   1.60   0.54   0.05
add          AKQJ9xxxxx   0.96   1.61   0.55   0.06
add          AKQJTxxxxx   0.98   1.63   0.56   0.07
add          AKQJT9xxxx   0.99   1.64   0.57   0.09

add         QJT9xxxxxxx  -0.85  -0.82  -0.54  -1.39
add         KJT9xxxxxxx  -0.71  -0.68  -0.44  -1.27
add         KQT9xxxxxxx  -0.60  -0.58  -0.39  -1.20
add         KQJ9xxxxxxx  -0.55  -0.52  -0.35  -1.16
add         KQJTxxxxxxx  -0.53  -0.50  -0.34  -1.15
add         KQJT9xxxxxx  -0.52  -0.49  -0.33  -1.14
add         AJT9xxxxxxx  -0.32  -0.32  -0.18  -1.03
add         AQT9xxxxxxx  -0.24  -0.24  -0.13  -0.97
add         AQJ9xxxxxxx  -0.18  -0.18  -0.10  -0.93
add         AQJTxxxxxxx  -0.16  -0.15  -0.09  -0.92
add         AQJT9xxxxxx  -0.15  -0.14  -0.08  -0.91
add         AKT9xxxxxxx  -0.04  -0.06  -0.02  -0.83
add         AKJ9xxxxxxx   0.01  -0.01   0.01  -0.80
add         AKJTxxxxxxx   0.03   0.02   0.03  -0.79
add         AKJT9xxxxxx   0.04   0.03   0.04  -0.78
add         AKQ9xxxxxxx   0.10   0.09   0.06   0.06
add         AKQTxxxxxxx   0.12   0.11   0.07   0.08
add         AKQT9xxxxxx   0.13   0.12   0.08   0.09
add         AKQJxxxxxxx   0.17   0.17   0.11   0.11
add         AKQJ9xxxxxx   0.18   0.18   0.12   0.12
add         AKQJTxxxxxx   0.20   0.20   0.13   0.14
add         AKQJT9xxxxx   0.21   0.21   0.14   0.15

add        KQJT9xxxxxxx  -0.62  -0.59  -0.39  -0.40
add        AQJT9xxxxxxx  -0.20  -0.20  -0.10  -0.15
add        AKJT9xxxxxxx  -0.05  -0.07  -0.03  -0.04
add        AKQT9xxxxxxx   0.02   0.01   0.01   0.01
add        AKQJ9xxxxxxx   0.07   0.07   0.04   0.05
add        AKQJTxxxxxxx   0.09   0.09   0.05   0.06
add        AKQJT9xxxxxx   0.10   0.10   0.06   0.07

add       AKQJT9xxxxxxx   0.00   0.00   0.00   0.00

initialize
}
