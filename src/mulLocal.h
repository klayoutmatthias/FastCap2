
#if !defined(mulLocal_H)
#define mulLocal_H

struct ssystem;
struct charge;

void evalFacFra(ssystem *sys, double **array, int order);
void evalSqrtFac(ssystem *sys, double **arrayout, double **arrayin, int order);
double **mulLocal2P(ssystem *sys, double x, double y, double z, charge **chgs, int numchgs, int order);
double **mulQ2Local(ssystem *sys, charge **chgs, int numchgs, int *is_dummy, double x, double y, double z, int order);
double **mulLocal2Local(ssystem *sys, double x, double y, double z, double xc, double yc, double zc, int order);
double **mulMulti2Local(ssystem *sys, double x, double y, double z, double xp, double yp, double zp, int order);

#endif
