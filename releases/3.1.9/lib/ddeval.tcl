#
# Copyright (C) 1996-2001, Thomas Andrews
#
# $Id$
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

namespace eval ddeval {
  variable ddData
  set ddData(offense/nt) 2
  set ddData(defense/nt) 3
  set ddData(offense/suit) 4
  set ddData(defense/suit) 5
  set ddData(offense/best) 0
  set ddData(defense/best) 1

  proc add {key count args} {
     variable ddData
     set ddData($key) $args
  } 

  proc ddlookup {key column} {
     variable ddData
     if {[info exists ddData($key)]} {
         set result [lindex $ddData($key) $column]
         return $result
     }
     return -13.00
  }

  proc ddvalue {column holding len} {
     set holdingval [ddlookup $holding $column]
     set lengthval [ddlookup length$len $column]
     expr {$holdingval-$lengthval}
  } 

  holdingProc -string convert {A K Q J T x9 len string} {

     regsub -all {[2-8]} $string x string
     if {$len==0} { return "-" }

     if {$len>=1 && $len<=5} { 
        return $string
     }

     regsub 9 $string x string

     if {$len>=7} {
       regsub T $string x string
     }

     if {$len>=8} {
       regsub J $string x string
     }

     return $string

  }  
  proc initialize {} {
    variable ddData
    set hbody {
        set converted [::convert holding $string]
        ::ddeval::ddvalue %d $converted $len
    }

    set sbody {
       ::ddeval::ddlookup [join [lsort -decreasing -integer [list $s $h $d $c]] "-"] %d
    }

    
    set body {
       expr {[dds.%s.%s %%h]+[ddh.%s.%s %%h]}
    }

    foreach val {offense defense} {
       foreach contract {nt suit best} {
          set column $ddData($val/$contract)
          holdingProc -double ddh.$val.$contract {x2 string len} [format $hbody $column]
          shapefunc dds.$val.$contract [format $sbody $column]
          handProc $val.$contract [format $body $val $contract $val $contract]
          namespace export $val.$contract
       }
    } 
  }

# Raw data used
#    Average for lengths
add             length0  146816   9.62  4.54   6.14  7.25   9.62  4.58
add             length1  919147   8.90  4.53   6.00  7.04   8.89  4.56
add             length2 2360473   8.40  4.51   6.04  6.96   8.39  4.52
add             length3 3284394   8.21  4.56   6.09  6.91   8.19  4.58
add             length4 2740694   8.34  4.65   6.08  6.86   8.32  4.67
add             length5 1430496   8.58  4.58   6.03  6.92   8.57  4.60
add             length6  476133   8.91  4.38   6.01  7.17   8.90  4.41
add             length7  100949   9.33  4.12   5.99  7.57   9.32  4.16
add             length8   13488   9.84  3.82   5.96  8.04   9.84  3.86
add             length9     993  10.40  3.39   5.72  8.43  10.40  3.44
add            length10      48  11.00  3.29   5.90  9.29  11.00  3.35
add            length11       1  12.00  4.00   6.00  6.00  12.00  4.00
#    Average for shapes
add             4-3-3-3  302661   7.80  4.59   6.15  6.83   7.76  4.59
add             4-4-3-2  619409   8.09  4.67   6.10  6.80   8.07  4.68
add             4-4-4-1   85946   8.62  4.83   6.09  6.79   8.61  4.85
add             5-3-3-2  443513   8.14  4.52   6.07  6.90   8.13  4.53
add             5-4-2-2  303105   8.41  4.56   6.01  6.87   8.40  4.57
add             5-4-3-1  371483   8.69  4.68   6.03  6.88   8.68  4.70
add             5-4-4-0   35658   9.38  4.80   6.19  6.99   9.37  4.84
add             5-5-2-1   91467   9.03  4.53   5.94  6.95   9.03  4.56
add             5-5-3-0   25826   9.51  4.66   6.12  7.07   9.51  4.71
add             6-3-2-2  161401   8.51  4.30   6.00  7.16   8.50  4.32
add             6-3-3-1   98854   8.78  4.43   6.03  7.17   8.77  4.46
add             6-4-2-1  134700   9.02  4.41   5.97  7.13   9.02  4.45
add             6-4-3-0   37917   9.51  4.59   6.18  7.25   9.51  4.63
add             6-5-1-1   20206   9.61  4.28   5.83  7.15   9.61  4.32
add             6-5-2-0   18685   9.88  4.38   6.03  7.27   9.88  4.43
add             6-6-1-0    2098  10.51  4.08   5.83  7.40  10.51  4.14
add             7-2-2-2   14761   8.91  4.03   5.95  7.54   8.91  4.05
add             7-3-2-1   53849   9.14  4.09   5.94  7.53   9.13  4.13
add             7-3-3-0    7522   9.65  4.29   6.16  7.64   9.64  4.35
add             7-4-1-1   11107   9.67  4.16   5.98  7.61   9.66  4.20
add             7-4-2-0   10387   9.89  4.25   6.14  7.69   9.88  4.30
add             7-5-1-0    3149  10.50  4.11   6.00  7.75  10.50  4.15
add             7-6-0-0     174  11.18  3.80   6.14  7.59  11.18  3.86
add             8-2-2-1    5565   9.57  3.76   5.87  7.96   9.57  3.80
add             8-3-1-1    3403   9.83  3.84   5.96  8.11   9.82  3.88
add             8-3-2-0    3076  10.00  3.87   6.08  8.07   9.99  3.92
add             8-4-1-0    1333  10.49  3.88   6.06  8.16  10.49  3.92
add             8-5-0-0     111  11.26  3.70   6.02  7.98  11.26  3.71
add             9-2-1-1     468  10.19  3.31   5.61  8.34  10.19  3.36
add             9-2-2-0     228  10.56  3.54   5.99  8.73  10.55  3.60
add             9-3-1-0     268  10.56  3.39   5.63  8.31  10.55  3.44
add             9-4-0-0      29  11.24  3.41   6.17  8.62  11.24  3.41
add            10-1-1-1       9  10.89  3.33   4.89  9.11  10.89  3.56
add            10-2-1-0      38  10.97  3.21   5.95  9.24  10.97  3.24
add            10-3-0-0       1  13.00  6.00  13.00 13.00  13.00  6.00
add            11-1-1-0       1  12.00  4.00   6.00  6.00  12.00  4.00
#    Average for holdings
add                   -  146816   9.62  4.54   6.14  7.25   9.62  4.58
add                   x  495231   8.81  4.41   5.84  6.89   8.80  4.44
add                   9   70427   8.82  4.42   5.85  6.89   8.81  4.46
add                   T   70675   8.82  4.43   5.86  6.91   8.81  4.46
add                   J   71048   8.85  4.49   5.92  6.99   8.85  4.52
add                   Q   70924   8.89  4.58   6.02  7.08   8.88  4.61
add                   K   70241   8.97  4.75   6.26  7.28   8.96  4.77
add                   A   70601   9.62  5.31   7.23  8.16   9.61  5.33
add                  xx  635907   8.10  4.17   5.55  6.51   8.09  4.19
add                  9x  212118   8.11  4.19   5.56  6.52   8.10  4.21
add                  Tx  212445   8.13  4.22   5.60  6.56   8.11  4.24
add                  T9   30256   8.13  4.23   5.62  6.57   8.12  4.25
add                  Jx  211292   8.18  4.30   5.72  6.67   8.17  4.32
add                  J9   29947   8.19  4.32   5.75  6.69   8.18  4.33
add                  JT   30091   8.23  4.35   5.76  6.72   8.22  4.37
add                  Q9   30585   8.31  4.46   5.93  6.86   8.29  4.47
add                  Qx  211693   8.31  4.45   5.95  6.87   8.29  4.47
add                  QT   29998   8.35  4.51   6.00  6.95   8.33  4.52
add                  QJ   30423   8.39  4.56   6.05  7.00   8.37  4.57
add                  Kx  211718   8.71  4.84   6.56  7.40   8.69  4.86
add                  K9   30151   8.73  4.86   6.58  7.43   8.71  4.88
add                  KT   30110   8.75  4.90   6.63  7.47   8.73  4.91
add                  KJ   30345   8.83  5.01   6.77  7.62   8.80  5.02
add                  KQ   30192   8.94  5.14   6.91  7.79   8.92  5.15
add                  Ax  211106   9.09  5.20   7.10  7.95   9.08  5.22
add                  A9   30596   9.10  5.22   7.10  7.95   9.08  5.24
add                  AT   30558   9.12  5.26   7.14  7.99   9.10  5.27
add                  AJ   30125   9.20  5.34   7.26  8.11   9.18  5.35
add                  AQ   30399   9.43  5.58   7.65  8.42   9.40  5.59
add                  AK   30418   9.65  5.85   7.85  8.62   9.63  5.86
add                 xxx  401886   7.64  3.97   5.23  6.13   7.62  3.98
add                 9xx  241434   7.65  3.99   5.24  6.14   7.63  4.00
add                 Txx  241139   7.70  4.04   5.32  6.21   7.68  4.05
add                 T9x   80254   7.72  4.06   5.34  6.23   7.70  4.07
add                 Jxx  241111   7.80  4.16   5.51  6.37   7.78  4.17
add                 J9x   80790   7.83  4.20   5.54  6.40   7.81  4.21
add                 JTx   80365   7.88  4.23   5.60  6.46   7.86  4.24
add                 JT9   11637   7.92  4.26   5.61  6.48   7.90  4.27
add                 Qxx  240850   8.02  4.38   5.89  6.69   8.00  4.39
add                 Q9x   80391   8.06  4.42   5.94  6.74   8.03  4.43
add                 QT9   11313   8.12  4.56   6.09  6.88   8.09  4.56
add                 QTx   80633   8.12  4.52   6.07  6.86   8.10  4.52
add                 QJ9   11575   8.20  4.62   6.23  7.01   8.17  4.63
add                 QJx   80296   8.21  4.62   6.24  7.01   8.18  4.62
add                 QJT   11524   8.26  4.67   6.29  7.06   8.22  4.67
add                 Kxx  241066   8.36  4.71   6.31  7.10   8.33  4.72
add                 K9x   80122   8.40  4.76   6.37  7.16   8.38  4.77
add                 KTx   80536   8.47  4.84   6.48  7.26   8.44  4.84
add                 KT9   11304   8.52  4.90   6.57  7.34   8.49  4.91
add                 KJx   80827   8.62  4.99   6.70  7.46   8.58  5.00
add                 KJ9   11500   8.62  5.04   6.73  7.48   8.59  5.05
add                 KJT   11448   8.66  5.08   6.79  7.54   8.63  5.09
add                 Axx  240375   8.71  5.03   6.79  7.59   8.69  5.04
add                 A9x   80825   8.76  5.09   6.85  7.65   8.74  5.10
add                 KQx   80907   8.76  5.17   6.90  7.66   8.73  5.17
add                 KQ9   11528   8.76  5.19   6.91  7.66   8.73  5.20
add                 ATx   79893   8.82  5.18   6.95  7.73   8.79  5.19
add                 AT9   11583   8.85  5.21   6.97  7.75   8.82  5.22
add                 KQT   11434   8.85  5.25   7.01  7.76   8.82  5.25
add                 KQJ   11431   8.85  5.32   7.03  7.78   8.82  5.32
add                 AJx   80441   8.97  5.32   7.19  7.93   8.94  5.33
add                 AJ9   11585   9.03  5.36   7.23  7.99   9.00  5.37
add                 AJT   11452   9.05  5.44   7.32  8.06   9.03  5.44
add                 AQx   79757   9.18  5.52   7.44  8.18   9.15  5.53
add                 AQ9   11600   9.20  5.57   7.49  8.24   9.18  5.58
add                 AQT   11377   9.27  5.64   7.57  8.30   9.23  5.65
add                 AQJ   11643   9.34  5.74   7.65  8.38   9.30  5.75
add                 AKx   80396   9.38  5.76   7.67  8.44   9.35  5.77
add                 AK9   11370   9.43  5.84   7.73  8.51   9.40  5.84
add                 AKT   11542   9.49  5.91   7.83  8.58   9.46  5.92
add                 AKJ   11595   9.57  6.02   7.93  8.70   9.54  6.02
add                 AKQ   11659   9.68  6.15   7.99  8.81   9.65  6.16
add                xxxx  134325   7.53  3.84   4.92  5.76   7.52  3.86
add                9xxx  133916   7.57  3.88   4.99  5.82   7.56  3.90
add                Txxx  134132   7.63  3.94   5.11  5.91   7.62  3.96
add                T9xx   80515   7.66  3.98   5.17  5.96   7.65  4.00
add                Jxxx  134518   7.75  4.07   5.33  6.10   7.74  4.08
add                J9xx   80574   7.80  4.11   5.41  6.16   7.78  4.13
add                JTxx   80373   7.83  4.16   5.49  6.23   7.81  4.17
add                JT9x   26964   7.85  4.18   5.52  6.27   7.83  4.19
add                Qxxx  134360   7.94  4.24   5.59  6.35   7.92  4.26
add                Q9xx   80247   8.00  4.31   5.70  6.45   7.99  4.33
add                QTxx   80282   8.05  4.38   5.78  6.52   8.03  4.39
add                QT9x   26750   8.11  4.45   5.88  6.61   8.09  4.45
add                QJxx   80871   8.16  4.49   5.94  6.68   8.13  4.50
add                QJ9x   26680   8.18  4.53   5.99  6.73   8.15  4.54
add                QJTx   26374   8.23  4.56   6.02  6.78   8.20  4.57
add                Kxxx  133668   8.26  4.56   5.98  6.73   8.24  4.58
add                QJT9    3851   8.27  4.63   6.09  6.88   8.24  4.64
add                K9xx   80561   8.32  4.63   6.07  6.81   8.30  4.64
add                KTxx   80481   8.41  4.72   6.21  6.95   8.39  4.73
add                KT9x   26815   8.45  4.78   6.28  7.01   8.42  4.79
add                KJxx   80533   8.54  4.84   6.37  7.12   8.52  4.85
add                KJ9x   26653   8.59  4.90   6.44  7.20   8.57  4.91
add                Axxx  133756   8.61  4.92   6.43  7.20   8.60  4.93
add                KJT9    3793   8.61  4.88   6.43  7.18   8.58  4.89
add                KJTx   26936   8.63  4.93   6.47  7.25   8.61  4.94
add                KQxx   80503   8.67  4.99   6.53  7.30   8.65  5.00
add                A9xx   80552   8.68  4.99   6.53  7.29   8.67  5.00
add                KQ9x   26862   8.72  5.03   6.58  7.38   8.69  5.04
add                KQT9    3864   8.75  5.12   6.66  7.43   8.72  5.12
add                ATxx   80823   8.76  5.07   6.65  7.40   8.74  5.08
add                KQTx   26799   8.79  5.11   6.67  7.47   8.76  5.11
add                AT9x   26668   8.81  5.13   6.74  7.47   8.79  5.14
add                KQJx   26727   8.83  5.17   6.73  7.54   8.80  5.17
add                KQJT    3726   8.86  5.25   6.81  7.66   8.82  5.25
add                AJxx   80271   8.90  5.20   6.83  7.59   8.88  5.21
add                KQJ9    3816   8.91  5.25   6.79  7.63   8.89  5.25
add                AJ9x   26980   8.98  5.29   6.96  7.71   8.96  5.30
add                AJTx   27168   9.03  5.33   7.01  7.79   9.00  5.34
add                AJT9    3879   9.05  5.39   7.06  7.80   9.02  5.40
add                AQxx   80642   9.10  5.40   7.07  7.84   9.09  5.41
add                AQ9x   26929   9.15  5.43   7.13  7.90   9.13  5.44
add                AQTx   26813   9.21  5.52   7.23  8.01   9.19  5.53
add                AQT9    3866   9.28  5.59   7.28  8.09   9.26  5.59
add                AQJ9    3843   9.29  5.62   7.32  8.12   9.27  5.63
add                AKxx   80322   9.30  5.65   7.29  8.09   9.28  5.66
add                AQJx   26978   9.30  5.61   7.31  8.12   9.28  5.62
add                AK9x   27156   9.36  5.71   7.37  8.17   9.34  5.72
add                AQJT    3810   9.38  5.69   7.38  8.19   9.35  5.70
add                AKTx   26814   9.41  5.77   7.44  8.26   9.39  5.79
add                AKT9    3914   9.45  5.74   7.47  8.29   9.42  5.75
add                AKJx   26984   9.52  5.86   7.59  8.43   9.49  5.87
add                AKJT    3777   9.55  5.93   7.60  8.45   9.52  5.93
add                AKJ9    3819   9.59  5.94   7.67  8.52   9.56  5.95
add                AKQx   27008   9.63  5.98   7.73  8.62   9.60  5.98
add                AKQT    3889   9.69  6.05   7.81  8.71   9.66  6.05
add                AKQJ    3734   9.74  6.02   7.80  8.77   9.70  6.02
add                AKQ9    3830   9.74  6.04   7.82  8.77   9.72  6.04
add               xxxxx   23435   7.61  3.73   4.67  5.50   7.60  3.77
add               9xxxx   38712   7.62  3.73   4.71  5.53   7.62  3.77
add               Txxxx   38924   7.71  3.78   4.83  5.65   7.71  3.82
add               T9xxx   39308   7.72  3.80   4.84  5.65   7.71  3.83
add               Jxxxx   39174   7.79  3.88   5.01  5.80   7.78  3.90
add               J9xxx   38607   7.87  3.91   5.09  5.88   7.87  3.94
add               JTxxx   38886   7.89  3.91   5.09  5.90   7.88  3.94
add               JT9xx   23432   7.93  3.93   5.14  5.94   7.92  3.96
add               Qxxxx   39018   7.99  4.01   5.26  6.07   7.98  4.03
add               Q9xxx   38439   8.06  4.06   5.35  6.17   8.05  4.09
add               QTxxx   39015   8.08  4.08   5.40  6.22   8.07  4.10
add               QT9xx   23360   8.14  4.13   5.46  6.30   8.13  4.14
add               QJxxx   38915   8.21  4.20   5.57  6.41   8.20  4.21
add               QJ9xx   23083   8.23  4.18   5.57  6.42   8.21  4.19
add               QJTxx   23391   8.27  4.23   5.64  6.51   8.26  4.24
add               Kxxxx   38921   8.30  4.33   5.66  6.49   8.29  4.35
add               QJT9x    7792   8.32  4.26   5.68  6.56   8.31  4.28
add               K9xxx   38740   8.37  4.36   5.74  6.57   8.37  4.38
add               KTxxx   39207   8.45  4.42   5.86  6.69   8.43  4.44
add               KT9xx   23135   8.50  4.46   5.92  6.76   8.49  4.48
add               KJxxx   39145   8.56  4.52   6.01  6.88   8.55  4.53
add               KJ9xx   23249   8.62  4.55   6.06  6.96   8.60  4.56
add               KJTxx   23266   8.65  4.58   6.12  7.03   8.64  4.59
add               Axxxx   39445   8.67  4.74   6.11  6.95   8.67  4.78
add               KJT9x    7718   8.69  4.60   6.17  7.09   8.68  4.60
add               A9xxx   38952   8.71  4.77   6.18  7.01   8.70  4.80
add               KQxxx   38979   8.71  4.65   6.21  7.12   8.70  4.66
add               KQ9xx   23514   8.76  4.68   6.25  7.18   8.74  4.69
add               ATxxx   38910   8.80  4.84   6.31  7.16   8.79  4.86
add               KQTxx   23299   8.82  4.73   6.36  7.31   8.80  4.74
add               KQT9x    7934   8.83  4.74   6.34  7.33   8.81  4.75
add               AT9xx   23167   8.85  4.91   6.41  7.25   8.84  4.94
add               KQJxx   23100   8.87  4.77   6.41  7.42   8.85  4.78
add               KQJ9x    7677   8.91  4.78   6.44  7.50   8.89  4.78
add               KQJTx    7650   8.93  4.81   6.49  7.56   8.91  4.82
add               KQJT9    1137   8.94  4.78   6.46  7.60   8.91  4.78
add               AJxxx   38828   8.96  4.96   6.53  7.40   8.95  4.98
add               AJ9xx   23184   9.01  4.99   6.62  7.51   8.99  5.01
add               AJTxx   23230   9.07  5.03   6.71  7.63   9.06  5.05
add               AJT9x    7762   9.11  5.05   6.72  7.66   9.10  5.06
add               AQxxx   39193   9.15  5.13   6.77  7.67   9.13  5.15
add               AQ9xx   23417   9.22  5.19   6.89  7.80   9.20  5.20
add               AQTxx   23262   9.29  5.22   6.98  7.92   9.27  5.24
add               AQT9x    7794   9.29  5.24   6.97  7.92   9.28  5.25
add               AKxxx   38890   9.35  5.39   7.02  7.96   9.34  5.41
add               AQJxx   23241   9.37  5.27   7.05  8.05   9.35  5.28
add               AQJ9x    7730   9.40  5.31   7.12  8.13   9.38  5.32
add               AK9xx   23241   9.41  5.41   7.10  8.06   9.40  5.43
add               AQJTx    7670   9.44  5.33   7.18  8.20   9.42  5.33
add               AQJT9    1099   9.44  5.34   7.16  8.19   9.42  5.35
add               AKTxx   23269   9.47  5.46   7.21  8.19   9.46  5.48
add               AKT9x    7676   9.51  5.46   7.26  8.25   9.49  5.48
add               AKJT9    1100   9.58  5.47   7.37  8.39   9.56  5.48
add               AKJ9x    7934   9.58  5.50   7.36  8.42   9.56  5.51
add               AKJxx   23406   9.60  5.56   7.41  8.43   9.58  5.57
add               AKJTx    7876   9.65  5.57   7.44  8.54   9.63  5.58
add               AKQxx   23248   9.69  5.60   7.55  8.65   9.67  5.61
add               AKQ9x    7710   9.73  5.63   7.59  8.74   9.72  5.63
add               AKQT9    1156   9.79  5.66   7.75  8.86   9.76  5.66
add               AKQTx    7814   9.79  5.65   7.70  8.86   9.77  5.66
add               AKQJx    7910   9.84  5.68   7.82  9.02   9.81  5.69
add               AKQJT    1142   9.85  5.67   7.86  9.04   9.82  5.67
add               AKQJ9    1078   9.90  5.79   7.99  9.12   9.87  5.79
add              xxxxxx    7826   7.82  3.54   4.41  5.31   7.82  3.60
add              Txxxxx   15523   7.92  3.58   4.54  5.44   7.91  3.64
add              Jxxxxx   15505   8.00  3.63   4.68  5.60   8.00  3.68
add              JTxxxx   19384   8.10  3.67   4.77  5.70   8.10  3.72
add              Qxxxxx   15529   8.16  3.77   4.97  5.92   8.16  3.81
add              QTxxxx   19569   8.28  3.81   5.06  6.05   8.27  3.85
add              QJxxxx   19400   8.40  3.87   5.21  6.23   8.40  3.90
add              QJTxxx   15478   8.45  3.86   5.19  6.25   8.45  3.89
add              Kxxxxx   15645   8.49  4.02   5.38  6.38   8.48  4.05
add              KTxxxx   19433   8.59  4.07   5.54  6.57   8.58  4.10
add              KJxxxx   19269   8.69  4.13   5.71  6.80   8.68  4.15
add              KJTxxx   15640   8.80  4.17   5.79  6.95   8.79  4.19
add              Axxxxx   15508   8.83  4.49   5.85  6.90   8.83  4.56
add              KQxxxx   19400   8.85  4.26   5.91  7.08   8.84  4.28
add              KQTxxx   15623   8.94  4.32   6.01  7.26   8.93  4.33
add              ATxxxx   19345   8.96  4.57   6.07  7.13   8.96  4.62
add              KQJxxx   15574   9.06  4.33   6.14  7.47   9.04  4.34
add              AJxxxx   19209   9.10  4.64   6.28  7.42   9.10  4.69
add              KQJTxx    7917   9.11  4.37   6.16  7.51   9.10  4.38
add              AJTxxx   15500   9.19  4.69   6.41  7.59   9.18  4.73
add              AQxxxx   19364   9.24  4.77   6.52  7.71   9.23  4.80
add              AQTxxx   15428   9.38  4.82   6.71  7.95   9.37  4.85
add              AQJxxx   15605   9.47  4.90   6.86  8.16   9.46  4.92
add              AQJTxx    7712   9.48  4.86   6.85  8.19   9.47  4.87
add              AKxxxx   19384   9.54  5.05   6.93  8.20   9.53  5.07
add              AKTxxx   15645   9.61  5.05   7.06  8.38   9.60  5.07
add              AKJxxx   15651   9.73  5.14   7.28  8.67   9.72  5.15
add              AKJTxx    7758   9.77  5.10   7.34  8.79   9.76  5.11
add              AKQxxx   15573   9.85  5.16   7.50  9.04   9.83  5.17
add              AKQTxx    7698   9.92  5.21   7.65  9.23   9.90  5.21
add              AKQJxx    7773   9.95  5.17   7.70  9.36   9.93  5.18
add              AKQJTx    2265   9.96  5.18   7.79  9.42   9.94  5.19
add             xxxxxxx    2129   8.19  3.35   4.22  5.22   8.19  3.45
add             Jxxxxxx    4858   8.38  3.38   4.39  5.48   8.37  3.46
add             Qxxxxxx    4977   8.55  3.51   4.63  5.81   8.55  3.57
add             QJxxxxx    7351   8.70  3.57   4.80  6.03   8.70  3.63
add             Kxxxxxx    4986   8.75  3.65   5.11  6.38   8.75  3.69
add             KJxxxxx    7326   8.98  3.75   5.36  6.77   8.98  3.78
add             Axxxxxx    5010   9.16  4.21   5.69  7.10   9.16  4.29
add             KQxxxxx    7467   9.17  3.87   5.57  7.12   9.17  3.89
add             KQJxxxx    7402   9.31  3.89   5.71  7.38   9.30  3.91
add             AJxxxxx    7563   9.38  4.31   6.11  7.61   9.37  4.36
add             AQxxxxx    7440   9.54  4.42   6.38  8.00   9.53  4.46
add             AQJxxxx    7422   9.74  4.50   6.71  8.43   9.73  4.52
add             AKxxxxx    7277   9.79  4.61   6.81  8.62   9.78  4.64
add             AKJxxxx    7284   9.94  4.63   7.12  9.09   9.93  4.64
add             AKQxxxx    7443  10.10  4.69   7.45  9.59  10.09  4.69
add             AKQJxxx    5014  10.17  4.70   7.60  9.78  10.15  4.70
add            xxxxxxxx     460   8.71  3.09   3.90  5.11   8.71  3.20
add            Qxxxxxxx    1275   9.05  3.18   4.33  5.72   9.05  3.28
add            Kxxxxxxx    1257   9.39  3.42   5.07  6.69   9.39  3.47
add            KQxxxxxx    2165   9.61  3.42   5.12  6.98   9.61  3.45
add            Axxxxxxx    1258   9.72  3.99   5.69  7.57   9.72  4.08
add            AQxxxxxx    2209  10.05  4.10   6.40  8.47  10.04  4.15
add            AKxxxxxx    2207  10.18  4.14   6.73  9.22  10.18  4.16
add            AKQxxxxx    2657  10.41  4.17   7.32 10.07  10.40  4.18
add           xxxxxxxxx       6   8.50  1.83   2.50  2.83   8.50  2.50
add           Qxxxxxxxx      55   9.31  2.71   3.58  4.91   9.31  2.85
add           Kxxxxxxxx      58   9.84  3.07   4.79  6.26   9.84  3.16
add           Axxxxxxxx      70  10.00  3.17   4.74  7.30  10.00  3.31
add           KQxxxxxxx     159  10.07  2.97   4.35  6.26  10.07  3.04
add           AQxxxxxxx     171  10.39  3.64   6.07  8.58  10.38  3.67
add           AKxxxxxxx     173  10.66  3.56   6.68  9.75  10.66  3.57
add           AKQxxxxxx     301  10.88  3.62   6.54 10.16  10.88  3.65
add          Kxxxxxxxxx       2   9.50  2.50   3.50  6.00   9.50  2.50
add          Axxxxxxxxx       2  10.00  2.50   3.00  9.00  10.00  2.50
add          KQxxxxxxxx      10  10.60  2.90   3.80  5.50  10.60  3.20
add          Qxxxxxxxxx       1  11.00  4.00   5.00  5.00  11.00  4.00
add          AKxxxxxxxx       6  11.00  3.67   7.83 10.33  11.00  3.67
add          AQxxxxxxxx       7  11.29  4.14   7.00 11.71  11.29  4.14
add          AKQxxxxxxx      20  11.35  3.20   6.55 10.60  11.35  3.20
add         AKQxxxxxxxx       1  12.00  4.00   6.00  6.00  12.00  4.00
initialize

   handProc ::ddeval::all {
      set result [list]
      foreach evaluator {offense.suit offense.nt defense.suit defense.nt} {
         lappend list [list $evaluator [$evaluator %h]]
      }
      set list
   }
}

package provide DDEval 0.5

