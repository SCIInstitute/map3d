# The most basic of scripts to show a single surface geometry
# and one run of data with a channels mapping

MAP3D=map3d
GEOM=../geom/tank
DATA=../data/tank

$MAP3D -nw -f ${GEOM}/25feb97_sock.fac \
        -p ${DATA}/cool1-atdr-run1.mat -s 1 1000 \
	-ch ${GEOM}/sock128.channels 
