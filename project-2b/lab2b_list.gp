#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#
# output:
#	lab2_list-1.png ... cost per operation vs threads and iterations
#	lab2_list-2.png ... threads and iterations that run (un-protected) w/o failure
#	lab2_list-3.png ... threads and iterations that run (protected) w/o failure
#	lab2_list-4.png ... cost per operation vs number of threads
#
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#
#	Early in your implementation, you will not have data for all of the
#	tests, and the later sections may generate errors for missing data.
#

# general plot parameters
set terminal png
set datafile separator ","

# unset the kinky x axis
unset xtics
set xtics

set title "List-1: Total Number of Operations Per Second"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Operations per second"
set logscale y
set output 'lab2b_1.png'
set key left top
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'ops w/mutex' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-s,[0-9]*,1000,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'ops w/spin-lock' with linespoints lc rgb 'green'

# how many threads/iterations we can run without failure (w/o yielding)
set title "List-2: Wait-for-lock Time vs. Threads"
set xlabel "Threads"
set logscale x 10
set ylabel "Time(ns)"
set logscale y 10
set output 'lab2b_2.png'

# grep out only single threaded, un-protected, non-yield results
plot \
     "< grep 'list-none-m,[0-9]*,1000,' lab2b_list.csv" using ($2):($8) \
	title 'wait-for-lock' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,[0-9]*,1000,' lab2b_list.csv" using ($2):($7) \
	title 'average-op-time' with linespoints lc rgb 'green'


set title "List-3: Unprotected Threads and Iterations that run without failure"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_3.png'
# note that unsuccessful runs should have produced no output
plot \
     "< grep list-id-none lab2b_list.csv" using ($2):($3) \
	title 'w/o sync' with points lc rgb 'green', \
     "< grep list-id-s lab2b_list.csv" using ($2):($3) \
	title 'sync=s' with points lc rgb 'red', \
     "< grep list-id-m lab2b_list.csv" using ($2):($3) \
	title 'sync=m' with points lc rgb 'violet', \

set title "List-4: Total Number of Operations Per Second w/ Mutex and Lists"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Operations per second"
set logscale y
set output 'lab2b_4.png'
set key left top
plot \
     "< grep -e 'list-id-m,[0-9]*,1000,1' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'ops w/mutex lists=1' with linespoints lc rgb 'red', \
	"< grep -e 'list-id-m,[0-9]*,1000,4' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'ops w/mutex lists=4' with linespoints lc rgb 'blue', \
	"< grep -e 'list-id-m,[0-9]*,1000,8' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'ops w/mutex lists=8' with linespoints lc rgb 'green', \
	"< grep -e 'list-id-m,[0-9]*,1000,16' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'ops w/mutex lists=16' with linespoints lc rgb 'violet', \

set title "List-5: Total Number of Operations Per Second w/ Spin-lock and Lists"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Operations per second"
set logscale y
set output 'lab2b_5.png'
set key left top
plot \
     "< grep -e 'list-id-s,[0-9]*,1000,1' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'ops w/spin-lock lists=1' with linespoints lc rgb 'red', \
	"< grep -e 'list-id-s,[0-9]*,1000,4' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'ops w/spin-lock lists=4' with linespoints lc rgb 'blue', \
	"< grep -e 'list-id-s,[0-9]*,1000,8' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'ops w/spin-lock lists=8' with linespoints lc rgb 'green', \
	"< grep -e 'list-id-s,[0-9]*,1000,16' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'ops w/spin-lock lists=16' with linespoints lc rgb 'violet'	

