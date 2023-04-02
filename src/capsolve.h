
#if !defined(capsolve_H)
#define capsolve_H

int capsolve(double ***capmat, ssystem *sys, charge *chglist, int size, int real_size, double *trimat, double *sqrmat, int *real_index, int numconds, Name *name_list);

#endif
