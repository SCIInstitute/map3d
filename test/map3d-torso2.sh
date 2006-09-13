# A script with a torso and heart surfaces from the same geometry file
# With data for each from a patient undergoing angioplasty
#

MAP3D=map3d
GEOM=../geom/torso
DATA=../data/torso

$MAP3D  -f ${GEOM}/daltorsoepi.geom@1 \
	-p ${DATA}/p2_3200_77_torso.tsdf -s 1 200 \
	-as 350 750 200 600 \
	-f ${GEOM}/daltorsoepi.geom@2 \
	-p ${DATA}/p2_3200_77_epi.tsdf -s 1 200
