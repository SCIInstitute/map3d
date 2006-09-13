MAP3D=/usr/local/bin/map3dgl
GEOM=${ROBHOME}/maprodxn/andy3/25feb97/geom
DATA=${ROBHOME}/maprodxn/andy3/25feb97/data
GEOM=../geom/tank
DATA=../data/tank

$MAP3D -nw -f ${GEOM}/25feb97_sock.fac \
	-as 200 600 400 800 \
	-p ${DATA}/cool1-atdr_new.tsdf@1 -s 1 476 \
	-at 200 600 200 420 -t 9\
	-ch ${GEOM}/sock128.channels \
	-lm ${GEOM}/25feb97_sock.lmarks \
	-f ${GEOM}/25feb97_sock_closed.geom \
	-as 590 990 400 800 \
	-p ${DATA}/cool1-atdr_new.tsdf@2 -s 1 476 \
	-at 590 990 200 420 -t 12 \
	-ch ${GEOM}/sock128.channels \
	-lm ${GEOM}/25feb97_sock.lmarks 

	$MAP3D -nw -b -f ${GEOM}/25feb97_sock.fac \
	-as 200 800 300 900 \
        -lm  ${GEOM}/25feb97_sock.lmarks \
        -p ${DATA}/cool1-atdr_new.tsdf@1 -s 1 1000 \
	-ch ${GEOM}/sock128.channels \
	-f ${GEOM}/25feb97_sock.fac \
	-p ${DATA}/cool1-atdr_new.tsdf@1 -s 1 1000 \
	-as 600 601 600 601 

