
#if !defined(mulDisplay_H)
#define mulDisplay_H

#include <cstdio>

struct ssystem;
struct charge;
struct Name;
struct face;
struct cube;
struct line;

void mksCapDump(ssystem *sys, double **capmat);
void dumpConfig(ssystem *sys, const char *name);
void dump_face(ssystem *sys, face *fac);
void dumpCorners(ssystem *sys, double **mat, int rows, int cols);
void dumpCondNames(ssystem *sys);
int has_duplicate_panels(ssystem *sys, charge *chglst);
void dumpMulSet(ssystem *sys);
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
void dumpChgDen(ssystem *sys, double *q, charge *chglist);
void dumpVecs(ssystem *sys, double *dblvec, int *intvec, int size);
void chkDummyList(ssystem *sys, charge **panels, int *is_dummy, int n_chgs);
void chkDummy(ssystem *sys, double *vector, int *is_dummy, int size);
void disExParsimpcube(ssystem *sys, cube *pc);
void disExtrasimpcube(ssystem *sys, cube *pc);
void discube(ssystem *sys, cube *pc);
void disdirectcube(ssystem *sys, cube *pc);
void disvect(ssystem *sys, double *v, int size);

#endif
