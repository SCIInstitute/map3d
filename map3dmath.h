/* Map3D math routines */

// macros
#define cjmax( a, b ) ( ( a ) < ( b ) ? ( b ) : ( a ) )
#define cjmin( a, b ) ( ( a ) < ( b ) ? ( a ) : ( b ) )
#define VECMAG3(x1,y1,z1) ( sqrt( (double) \
( (x1)*(x1) + (y1)*(y1) + (z1)*(z1) ) ) )
#define DOTPROD3(vec1,vec2) ( vec1[0] * vec2[0] + vec1[1] * vec2[1] + \
vec1[2] * vec2[2] )


//vector operations
float vectorLength(float a[]);
void normalizeVector(float a[]);
void crossProduct(float a[], float b[], float c[]);
float dotProduct(float a[], float b[]);
void AxisAndAngleOfRotation(float orig[], float transformed[], float& angle, float axis[]);

//matrix operations
void clearMatrix16(float a[]);
void MultMatrix16x4(float *a, float *b, float *product);
void MultMatrix9x3(float *a, float *b, float *product);
void MultMatrix16x16(float *a, float *b, float *product);
void InvertMatrix16(float *a, float *inverse);
void TransposeMatrix16(float *a, float *transpose);
void MultMatrix16x4(float *a, float *b, float *product);
void MultMatrix16x4(float *a, float *b, float *product);

// quaternion operations
struct Quat;
void normalizeQuat(Quat * q);