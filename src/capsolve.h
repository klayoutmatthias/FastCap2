
#if !defined(capsolve_H)
#define capsolve_H

struct ssystem;
struct charge;

int capsolve(double ***capmat, ssystem *sys, charge *chglist, int size, int real_size, double *trimat, double *sqrmat, int *real_index);

#endif
