# This one shows two different data sets on the same geometry
# Note the use of MATLAB files that contain multiple time series.
# There is also explicit control of the time instants to read
# and display; this aligns the two signals for better comparison.
# Also the use of locked scaling and the supression of one of the legend
# windows.
# See if you can spot the difference in the two time sequences.
# Hint: look in the late ST segment and especially the T wave.
#######################################################################

#MAP3D=../map3d
MAP3D=map3d
GEOM=../geom/tank
DATA=../data/tank

$MAP3D -nw -b -f ${GEOM}/25feb97_sock_closed.fac \
	-as 200 600 400 800 \
	-p ${DATA}/cool1-atdr.mat -s 82 420 \
	-sl 2 \
	-at 200 600 250 400 -t 126\
	-al 1 200 500 750 \
	-ch ${GEOM}/sock128.channels \
	-lm ${GEOM}/25feb97_sock.lmarks \
	-f ${GEOM}/25feb97_sock_closed.fac \
	-as 590 990 400 800 \
	-p ${DATA}/cool1-atdr.mat -s 74 412 \
	-slw 0 \
	-at 590 990 250 400 -t 126 \
        -al 1 200 250 500  \
	-ch ${GEOM}/sock128.channels \
	-lm ${GEOM}/25feb97_sock.lmarks 

