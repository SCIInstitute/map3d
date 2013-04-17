# This script shows how to read from ASCII, graphicsio, and MATLAB files
# 

MAP3D=map3d
GEOM=../geom/tank
DATA=../data/tank

$MAP3D -nw  -f ${GEOM}/25feb97_sock.fac \
	-as 400 800 400 800 \
	-p ${DATA}/cool1-atdr-run1.mat -s 130 400 \
	-at 400 800 200 420 -t 126 \
	-ch ${GEOM}/sock128.channels \
	-f ${GEOM}/25feb97_sock.mat \
	-as 800 1200 400 800 \
	-p ${DATA}/cool1-atdr-run1.mat -s 130 400 \
	-at 800 1200 200 420 -t 126 \
	-ch ${GEOM}/sock128.channels

