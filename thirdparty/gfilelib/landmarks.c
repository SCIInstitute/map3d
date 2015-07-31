#include "cutil.h"
#include "landmarks.h"
char *clmark[NUMLMARKTYPES] = {"Artery", "Occlusion", "Closure",
			       "Stimulus", "Lead", "Plane", 
			       "Rod", "PaceNeedle", "Cath",
			       "Fiber", "RecNeedle", "Cannula" };
long lmarktypes[NUMLMARKTYPES] = {LM_COR, LM_OCCLUS, LM_STITCH, 
				    LM_STIM, LM_LEAD, LM_PLANE, LM_ROD,
				    LM_PACENEEDLE, LM_CATH, LM_FIBER,
				    LM_RECNEEDLE, LM_CANNULA};
