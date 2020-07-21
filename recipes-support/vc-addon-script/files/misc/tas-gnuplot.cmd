set datafile separator ","
set autoscale
set grid
set xlabel "TAS cycle in {/Symbol m}s"
set ylabel "TxQueue Number"
set yrange [ -0.5 : 7.9 ]
set ytics out
set size  ratio 0.5
plot "tas.csv" u 1:2 w steps t "", "tas.csv" u 1:3 w steps t "", "tas.csv" u 1:4 w steps t "", "tas.csv" u 1:5 w steps t "", "tas.csv" u 1:6 w steps t "", "tas.csv" u 1:7 w steps t "", "tas.csv" u 1:8 w steps t "", "tas.csv" u 1:9 w steps t ""

set terminal "png" size 1000,500
set output "tas.png"
replot

set terminal "emf" size 1000,500
set output "tas.emf"
replot
