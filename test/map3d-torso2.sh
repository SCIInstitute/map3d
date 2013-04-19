# A script with a torso and heart surfaces from the same geometry file
# With data for each from a patient undergoing angioplasty
#

MAP3D=map3d
GEOM=../geom/torso
DATA=../data/torso

$MAP3D  -f ${GEOM}/daltorsoepi.mat \
	-p ${DATA}/dipole2_002.pot -s 1 200 \
	-as 350 750 200 600 \
	-f ${GEOM}/daltorsoepi.fac \
	-p ${DATA}/dipole2_003.pot -s 1 200
