Generate a plot of the configured TAS cycle
===========================================

1. Generating the scheduling file (e.g. tas-example.sch)
--------------------------------------------------------
  In this file the schedule for every TX queue needs to be define,
  one queue per line in ascending order.
    {6 {0 25000} {1 200} {0 24800} ...}
     ┬ └───┬───┘
     │     └───── Gate configuration
     │            {0 25000}
     │             ┬ └─┬─┘
     │             │   └──── duration time in ns
     │             └──────── gate state: 0-close, 1-open
     │
     └─────────── TX Queue number

2. Generating the input file for gnuplot execution
--------------------------------------------------
  use the script tas-schedule-to-csv.tcl with [input_file].sch to generate
  the input file for gnuplot:

  ~> tas-schedule-to-csv.tcl tas-example.sch > tas-example.csv

3. Generate plot
----------------
  use gnuplot to generate a plot and picture files .emf and .png
  by using the command input file tas-gnuplot.cmd
  before using tas-gnuplot.cmd adapt in line the name of the csv input file
    or rename your csv input file in tas.csv

  ~> gnuplot tas-gnuplot.cmd

  Output is: tas.emf
             tas.png file

