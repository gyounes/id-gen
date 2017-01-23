# Note you need gnuplot 4.4 for the pdfcairo terminal.

# set terminal pdfcairo font "Gill Sans,9" linewidth 4 rounded fontscale 1.0
set terminal pdfcairo font "Gill Sans,7" linewidth 1 rounded fontscale 1.0

# Select histogram data
set style data histogram

# Give the bars a plain fill pattern, and draw a solid line around them.
set style fill solid border

set output outputname
set xlabel "Number of Bytes"
set ylabel "Number of Ids"

set key right top

set datafile missing '0'

# Titles with spaces are not allowed
# These titles should be separated by "_" and here we replace by " "
# pretty(titles) = system("echo ".titles." | sed 's/_/ /g'")

# Input file contains comma-separated values fields
set datafile separator ","

set style histogram clustered

# read from one file multiple columns
#stats inputfiles skip 1
#max_col = STATS_columns
#plot for [COL=2:max_col] inputfiles using COL:xticlabels(1) title columnheader

# read from multiple files generate multiple graphs
# do for [i=1:words(inputfiles)] {
#  inputfile = word(inputfiles, i)
#  stats inputfile skip 1
#  max_col = STATS_columns
#  plot for [COL=2:max_col] inputfile using COL:xticlabels(1) title columnheader w histogram ls i
# }

# read from multiple files generate single graph
plot for [i=1:words(inputfiles)] word(inputfiles, i) using 2:xticlabels(1) title columnheader w histogram ls i