# A simple script with one torso geometry and a data file with some 
# body surface potentials generated from a dipole 

MAP3D=map3d
GEOM=../geom/torso
DATA=../data/torso

$MAP3D -f ${GEOM}/daltorso.geom -p ${DATA}/dipole2.tsdf -s 1 6
