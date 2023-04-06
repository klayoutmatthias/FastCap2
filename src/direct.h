
#include "mulStruct.h"

#if !defined(direct_H)
#define direct_H

struct ssystem;
struct charge;

int compressMat(ssystem *sys, double **mat, int size, int *is_dummy, int comp_rows);
void expandMat(double **mat, int size, int comp_size, int *is_dummy, int exp_rows);
void invert(double **mat, int size, int *reorder);
void solve(double **mat, double *x, double *b, int size);
double **ludecomp(ssystem *sys, double **matin, int size, int allocate);

double **Q2PDiag(ssystem *sys, charge **chgs, int numchgs, int *is_dummy, int calc);
double **Q2P(ssystem *sys, charge **qchgs, int numqchgs, int *is_dummy, charge **pchgs, int numpchgs, int calc);
double **Q2Pfull(ssystem *sys, cube *directlist, int numchgs);

#endif
