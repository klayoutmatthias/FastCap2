
#if !defined(mulDisplay_H)
#define mulDisplay_H

#include "zbufStruct.h"
#include <cstdio>

void mksCapDump(ssystem *sys, double **capmat, int numconds, double relperm, Name **name_list);
void dumpConfig(ssystem *sys, FILE *fp, const char *name);
void dump_face(FILE *fp, face *fac);
void dumpCorners(FILE *fp, double **mat, int rows, int cols);
void dumpCondNames(FILE *fp, Name *name_list);
int has_duplicate_panels(FILE *fp, charge *chglst);
void dumpMulSet(ssystem *sy, int numLev, int order);
void dump_preconditioner(ssystem *sys, charge *chglist, int type);
void dissys(ssystem *sys);
void chkList(ssystem *sys, int listtype);
void chkLowLev(ssystem *sys, int listtype);
void dumpSynop(ssystem *sys);
void dumpMatBldCnts(ssystem *sys);
void dumpMat(ssystem *sys, double **mat, int rows, int cols);
void dumpQ2PDiag(ssystem *sys, cube *nextc);
void dispQ2M(ssystem *sys, double **mat, charge **chgs, int numchgs, double x, double y, double z, int order);
void dispM2L(ssystem *sys, double **mat, double x, double y, double z, double xp, double yp, double zp, int order);
void dispQ2L(ssystem *sys, double **mat, charge **chgs, int numchgs, double x, double y, double z, int order);
void dispQ2P(ssystem *sys, double **mat, charge **chgs, int numchgs, int *is_dummy, charge **pchgs, int numpchgs);
void dispQ2PDiag(ssystem *sys, double **mat, charge **chgs, int numchgs, int *is_dummy);
void dispM2M(ssystem *sys, double **mat, double x, double y, double z, double xp, double yp, double zp, int order);
void dispL2L(ssystem *sys, double **mat, double x, double y, double z, double xp, double yp, double zp, int order);
void dispM2P(ssystem *sys, double **mat, double x, double y, double z, charge **chgs, int numchgs, int order);
void dispL2P(ssystem *sys, double **mat, double x, double y, double z, charge **chgs, int numchgs, int order);
void dumpLevOneUpVecs(ssystem *sys);
int dumpNameList(ssystem *sys, Name *name_list);
void dumpChgDen(FILE *fp, double *q, charge *chglist);
void dumpVecs(ssystem *sys, double *dblvec, int *intvec, int size);
void chkDummyList(ssystem *sys, charge **panels, int *is_dummy, int n_chgs);
void chkDummy(ssystem *sys, double *vector, int *is_dummy, int size);
void disExParsimpcube(ssystem *sys, cube *pc);
void disExtrasimpcube(ssystem *sys, cube *pc);
void discube(ssystem *sys, cube *pc);
void disdirectcube(ssystem *sys, cube *pc);
void disvect(ssystem *sys, double *v, int size);

#endif
