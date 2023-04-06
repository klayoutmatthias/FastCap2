
#if !defined(mulMulti_H)
#define mulMulti_H

struct ssystem;
struct charge;

void xyz2sphere(double x, double y, double z, double x0, double y0, double z0, double *rho, double *cosA, double *beta);
void evalLegendre(double cosA, double *vector, int order);

void mulMultiAlloc(ssystem *sys, int maxchgs, int order, int depth);
double **mulMulti2P(ssystem *sys, double x, double y, double z, charge **chgs, int numchgs, int order);
double **mulQ2Multi(ssystem *sys, charge **chgs, int *is_dummy, int numchgs, double x, double y, double z, int order);
double **mulMulti2Multi(ssystem *sys, double x, double y, double z, double xp, double yp, double zp, int order);

int multerms(int order);
int costerms(int order);
int sinterms(int order);

double iPwr(ssystem *sys, int e);
double fact(ssystem *sys, int x);

#endif
