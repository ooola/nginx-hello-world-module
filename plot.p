#output as png image
set terminal png

# save file to "basic.png"
set output "basic.png"

# graph title
set title "ab -n 50000 -c 500"

# nicer aspect ratio for image size
set size 1,0.7

# y-axis grid
set grid y

# x-axis label
set xlabel "request"

# y-axis label
set ylabel "response time (ms)"

# plot data from "helloworld.tsv" and "withsdk.tsv" using column 9 with smooth sbezier lines
# and titles of "Hellow World" and "Feature Flag" for the given data
plot "helloworld.tsv" using 9 smooth sbezier with lines title "Hello World", \
 "withsdk.tsv" using 9 smooth sbezier with lines title "Feature Flag"
