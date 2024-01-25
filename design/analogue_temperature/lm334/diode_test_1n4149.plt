set encoding utf8
set termoption noenhanced
set title "1n4149 temp coef simulation"
set xlabel "Celsius"
set ylabel "V"
set grid
unset logscale x 
set xrange [-5.000000e+01:1.000000e+02]
unset logscale y 
set yrange [4.159475e-01:7.505172e-01]
#set xtics 1
#set x2tics 1
#set ytics 1
#set y2tics 1
set format y "%g"
set format x "%g"
plot 'diode_test_1n4149.data' using 1:2 with lines lw 1 title "v(ip)"
