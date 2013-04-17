# This one shows borderless mode window layout, two surfaces with the same
# data, and some landmarks.

MAP3D=map3d
GEOM=../geom/tank
DATA=../data/tank

$MAP3D -nw -b -f ${GEOM}/25feb97_sock.fac \
	-as 200 600 400 800 \
	-p ${DATA}/cool1-atdr.mat -s 1 476 \
	-at 200 600 200 400 -t 9\
	-al 10 200 500 800 \
	-ch ${GEOM}/sock128.channels \
	-lm ${GEOM}/25feb97_sock.lmarks \
	-f ${GEOM}/25feb97_sock_closed.fac \
	-as 590 990 400 800 \
	-sl 1 \
	-p ${DATA}/cool1-atdr.mat -s 1 476 \
	-at 590 990 200 400 -t 126 \
	-al 10 200 200 500 \
	-ch ${GEOM}/sock128.channels \
	-lm ${GEOM}/25feb97_sock.lmarks 

