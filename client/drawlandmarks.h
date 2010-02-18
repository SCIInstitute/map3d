/* drawlandmarks.h */


#define MAXNPANELS 10
#define NPANELS 6

struct Land_Mark;
struct LandMarkSeg;
class Landmark_Draw;
class GeomWindow;

void DrawLandMarks(Land_Mark * onelandmark, Landmark_Draw * onelandmarkdraw, GeomWindow* geom);
void DrawArrowhead(float tip[3], float arrowpts[4][3]);
void DrawCorSeg(LandMarkSeg * onelandmarkseg, Landmark_Draw * draw, long type, GeomWindow* geom);
void DrawLMarkPick(LandMarkSeg * onelandmarkseg, long pointnum);
void DrawLMarkPoint(LandMarkSeg * onelandmarkseg, Landmark_Draw * draw, long type, GeomWindow* geom);
void DrawLMarkPlane(LandMarkSeg * onelandmarkseg, Landmark_Draw * draw, GeomWindow* geom);
void DrawLMarkRod(LandMarkSeg * onelandmarkseg, Landmark_Draw * draw, long type, long radius, GeomWindow* geom);
void GetArrow3D(float basept[3], float endpt[3],
                float arrowpt1[3], float arrowpt2[3], float arrowpt3[3], float arrowpt4[3], float tipradius);
void MakeCircPoints(float radius, long npanels, float circpts[][3]);
