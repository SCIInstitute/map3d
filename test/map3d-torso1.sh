# A simple script with one torso geometry and a data file with some 
# body surface potentials generated from a dipole 

MAP3D=map3d
GEOM=../geom/torso
DATA=../data/torso

$MAP3D -f ${GEOM}/daltorso.fac -p ${DATA}/dipole2_001.pot -s 1 6
