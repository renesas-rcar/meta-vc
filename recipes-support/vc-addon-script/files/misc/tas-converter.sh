#!/bin/bash


./tas-schedule-to-csv.tcl $1 > tas.csv
gnuplot -f tas-gnuplot.cmd &
echo Done

