/*!\page LICENSE LICENSE
 
Copyright (C) 2003 by the Board of Trustees of Massachusetts Institute of Technology, hereafter designated as the Copyright Owners.
 
License to use, copy, modify, sell and/or distribute this software and
its documentation for any purpose is hereby granted without royalty,
subject to the following terms and conditions:
 
1.  The above copyright notice and this permission notice must
appear in all copies of the software and related documentation.
 
2.  The names of the Copyright Owners may not be used in advertising or
publicity pertaining to distribution of the software without the specific,
prior written permission of the Copyright Owners.
 
3.  THE SOFTWARE IS PROVIDED "AS-IS" AND THE COPYRIGHT OWNERS MAKE NO
REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED, BY WAY OF EXAMPLE, BUT NOT
LIMITATION.  THE COPYRIGHT OWNERS MAKE NO REPRESENTATIONS OR WARRANTIES OF
MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE OR THAT THE USE OF THE
SOFTWARE WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS TRADEMARKS OR OTHER
RIGHTS. THE COPYRIGHT OWNERS SHALL NOT BE LIABLE FOR ANY LIABILITY OR DAMAGES
WITH RESPECT TO ANY CLAIM BY LICENSEE OR ANY THIRD PARTY ON ACCOUNT OF, OR
ARISING FROM THE LICENSE, OR ANY SUBLICENSE OR USE OF THE SOFTWARE OR ANY
SERVICE OR SUPPORT.
 
LICENSEE shall indemnify, hold harmless and defend the Copyright Owners and
their trustees, officers, employees, students and agents against any and all
claims arising out of the exercise of any rights under this Agreement,
including, without limiting the generality of the foregoing, against any
damages, losses or liabilities whatsoever with respect to death or injury to
person or damage to property arising from or out of the possession, use, or
operation of Software or Licensed Program(s) by LICENSEE or its customers.
 
*/

#if !defined(mulDisplay_H)
#define mulDisplay_H

#include "zbufStruct.h"

void mksCapDump(double **capmat, int numconds, double relperm, Name **name_list);
void dumpConfig(FILE *fp, char *name);
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
void dumpMat(double **mat, int rows, int cols);
void dumpQ2PDiag(cube *nextc);
void dispQ2M(double **mat, charge **chgs, int numchgs, double x, double y, double z, int order);
void dispM2L(double **mat, double x, double y, double z, double xp, double yp, double zp, int order);
void dispQ2L(double **mat, charge **chgs, int numchgs, double x, double y, double z, int order);
void dispQ2P(double **mat, charge **chgs, int numchgs, int *is_dummy, charge **pchgs, int numpchgs);
void dispQ2PDiag(double **mat, charge **chgs, int numchgs, int *is_dummy);
void dispM2M(double **mat, double x, double y, double z, double xp, double yp, double zp, int order);
void dispL2L(double **mat, double x, double y, double z, double xp, double yp, double zp, int order);
void dispM2P(double **mat, double x, double y, double z, charge **chgs, int numchgs, int order);
void dispL2P(double **mat, double x, double y, double z, charge **chgs, int numchgs, int order);
void dumpLevOneUpVecs(ssystem *sys);
int dumpNameList(Name *name_list);
void dumpChgDen(FILE *fp, double *q, charge *chglist);
void dumpVecs(double *dblvec, int *intvec, int size);
void chkDummyList(charge **panels, int *is_dummy, int n_chgs);
void chkDummy(double *vector, int *is_dummy, int size);
void disExParsimpcube(cube *pc);
void disExtrasimpcube(cube *pc);
void discube(cube *pc);
void disdirectcube(cube *pc);
void disvect(double *v, int size);

#endif