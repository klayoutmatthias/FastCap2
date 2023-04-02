
#if !defined(blkDirect_H)
#define blkDirect_H

struct ssystem;
struct cube;

void blkQ2Pfull(ssystem *sys, cube *directlist, int numchgs, int numchgs_wdummy,
                double **triArray, double **sqrArray, int **real_index, int *is_dummy);
void blkCompressVector(ssystem *sys, double *vec, int num_panels, int real_size, int *is_dummy);
void blkAqprod(ssystem *sys, double *p, double *q, int size, double *sqmat);
void blkExpandVector(double *vec, int num_panels, int real_size, int *real_index);
void blkLUdecomp(ssystem *sys, double *sqrArray, double *triArray, int numchgs);
void blkSolve(ssystem *sys, double *x, double *b, int siz, double *matri, double *matsq);

#endif
