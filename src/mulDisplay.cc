
#include "mulGlobal.h"
#include "mulStruct.h"
#include "zbufGlobal.h"
#include "mulDisplay.h"
#include "mulMulti.h"
#include "mulDo.h"
#include "input.h"
#include "quickif.h"
#include "calcp.h"
#include "savemat_mod.h"

#include <cmath>
#include <cstring>

static void dismat(ssystem *sys, double **mat, int rows, int cols);
static void dischg(ssystem *sys, charge *pq);
static void dissimpcube(ssystem *sys, cube *pc);
static void dumpChgsWDummy(ssystem *sys, charge **chgs, int numchgs, int *is_dummy, double x, double y, double z);

void disExtrasimpcube(ssystem *sys, cube *pc)
{
  sys->msg("cubes[%d][%d][%d][%d]\n", pc->level, pc->j, pc->k, pc->l);
}

void disExParsimpcube(ssystem *sys, cube *pc)
{
  cube *pa = pc->parent;
  sys->msg("cubes[%d][%d][%d][%d], ", pc->level, pc->j, pc->k, pc->l);
  sys->msg("parent = cubes[%d][%d][%d][%d]\n", pa->level, pa->j, pa->k, pa->l);
}

void dissimpcube(ssystem *sys, cube *pc)
{
  sys->msg("cube center: x=%g y=%g z=%g\n", pc->x, pc->y, pc->z);
  sys->msg("index=%d dindex=%d level=%d loc_exact=%d mul_exact=%d numkids=%d\n",
         pc->index, pc->dindex, pc->level,
         pc->loc_exact, pc->mul_exact, pc->numkids);
  sys->msg("numnbrs=%d upnumvects=%d directnumvects=%d downnumvects=%d\n",
         pc->numnbrs, pc->upnumvects, pc->directnumvects, pc->downnumvects);
}

void discube(ssystem *sys, cube *pc)
{
  int i;
  sys->msg("cube center: x=%g y=%g z=%g\n", pc->x, pc->y, pc->z);
  sys->msg("index=%d dindex=%d level=%d loc_exact=%d mul_exact=%d numkids=%d\n",
         pc->index, pc->dindex, pc->level,
         pc->loc_exact, pc->mul_exact, pc->numkids);
  sys->msg("numnbrs=%d upnumvects=%d directnumvects=%d downnumvects=%d\n",
         pc->numnbrs, pc->upnumvects, pc->directnumvects, pc->downnumvects);
  if(pc->directnumvects > 0) {
    sys->msg("num of elements in ");
    for(i=0; i < pc->directnumvects; i++) {
      sys->msg("v%d = %d ", i, pc->directnumeles[i]);
    }
    sys->msg("\nchgs\n");
    for(i=0; i < pc->directnumeles[0]; i++) {
      dischg(sys, pc->chgs[i]);
    }
  }
  if(pc->downnumvects > 0) {
    sys->msg("num of down elements in ");
    for(i=0; i < pc->downnumvects; i++) {
      sys->msg("v%d = %d ", i, pc->downnumeles[i]);
    }
  }
}

void disdirectcube(ssystem *sys, cube *pc)
{
int i;
  for(i=0; i < pc->directnumvects; i++) {
    sys->msg("matrix %d\n", i);
    dismat(sys, pc->directmats[i], pc->directnumeles[0], pc->directnumeles[i]);
    if(i==0) {
      sys->msg("lu factored matrix\n");
      dismat(sys, pc->directlu, pc->directnumeles[0], pc->directnumeles[i]);
    }
  }
}


void dissys(ssystem *sys)
{
int i, j, k, l, side;
  sys->msg("side=%d depth=%d order=%d\n",
         sys->side, sys->depth, sys->order);
  sys->msg("Cube corner is x=%g y=%g z=%g\n", sys->minx, sys->miny, sys->minz);
  sys->msg("Cube side length= %g\n", sys->length);
  sys->msg("Printing all the cubes\n");
  for(i = 0, side = 1; i <= sys->depth; i++, side *= 2) {
    for(j=0; j < side; j++) {
      for(k=0; k < side; k++) {
        for(l=0; l < side; l++) {
          if (sys->cubes[i][j][k][l]) {
            sys->msg("\ncubes[%d][%d][%d][%d]\n", i, j, k, l);
            dissimpcube(sys, sys->cubes[i][j][k][l]);
/*          disdirectcube(sys, &(sys->cubes[i][j][k][l])); */
          }
        }
      }
    }
  }
}



static void dismat(ssystem *sys, double **mat, int rows, int cols)
{
int i,j;
  if(cols != 0) {
    for(i=0; i < rows; i++) {
      sys->msg("\n i=%d\n", i);
      for(j=0; j < cols; j++) {
        sys->msg("%d %g  ", j, mat[i][j]);
        if(((j+1) % 5) == 0) sys->msg("\n");
      }
    }
    sys->msg("\n");
  }
}


void disvect(ssystem *sys, double *v, int size)
{
int i;
  for(i=0; i < size; i++) {
    sys->msg("i=%d %g ", i, v[i]);
    if(((i+1) % 5) == 0) sys->msg("\n");
  }
  sys->msg("\n");
}

static void dischg(ssystem *sys, charge *pq)
{
  sys->msg("cond=%d index=%d\n", pq->cond, pq->index);
}

/*
  dumps a rows x cols matrix of doubles; assumes indices from zero 
*/
void dumpMat(ssystem *sys, double **mat, int rows, int cols)
{
  int i, j;
  for(i = 0; i < rows; i++) {
    sys->msg("    row%d ", i);
    for(j = 0; j < cols; j++) {
      if(mat[i][j] < 0.0) sys->msg("%.5e ", mat[i][j]);
      else sys->msg(" %.5e ", mat[i][j]);
    }
    sys->msg("\n");
  }
}

/*
  dumps a rows x cols matrix of doubles; assumes indices from zero 
*/
void dumpCorners(ssystem *sys, double **mat, int rows, int cols)
{
  int i, j;
  for(i = 0; i < rows; i++) {
    sys->msg("  corner%d ", i);
    for(j = 0; j < cols; j++) {
      if(mat[i][j] < 0.0) sys->msg("%.5e ", mat[i][j]);
      else sys->msg(" %.5e ", mat[i][j]);
    }
    sys->msg("\n");
  }
}

/*
  dumps a vector of itegers along side a vector of doubles, index from zero
*/
void dumpVecs(ssystem *sys, double *dblvec, int *intvec, int size)
{
  int i;

  for(i = 0; i < size; i++) {
    sys->msg("%d %d %g\n", i, intvec[i], dblvec[i]);
  }
}

/*
  dumps the relative coordinates of an array of charges or evaluation pnts
*/
static void dumpChgs(ssystem *sys, charge **chgs, int numchgs, double x, double y, double z)
{
  int i;
  double rho, cosA, beta;
  for(i = 0; i < numchgs; i++) {
    xyz2sphere(chgs[i]->x, chgs[i]->y, chgs[i]->z,
               x, y, z, &rho, &cosA, &beta);
    sys->msg("    %d %d ", chgs[i]->index, chgs[i]->cond);
    if(rho < 0) sys->msg("(%.5e ", rho);
    else sys->msg("( %.5e ", rho);
    if(cosA < 0) sys->msg("%.5e ", cosA);
    else sys->msg(" %.5e ", cosA);
    if(beta < 0) sys->msg("%.5e) ", beta);
    else sys->msg(" %.5e) ", beta);
    if(x < 0) sys->msg("(%.5e ", chgs[i]->x);
    else sys->msg("( %.5e ", chgs[i]->x);
    if(y < 0) sys->msg("%.5e ", chgs[i]->y);
    else sys->msg(" %.5e ", chgs[i]->y);
    if(z < 0) sys->msg("%.5e)\n", chgs[i]->z);
    else sys->msg(" %.5e)\n", chgs[i]->z);
  }
}

/*
  dumps the relative coordinates of an array of charges or evaluation pnts
  - also dumps dummy bit
*/
void dumpChgsWDummy(ssystem *sys, charge **chgs, int numchgs, int *is_dummy, double x, double y, double z)
{
  int i;
  double rho, cosA, beta;
  for(i = 0; i < numchgs; i++) {
    xyz2sphere(chgs[i]->x, chgs[i]->y, chgs[i]->z,
               x, y, z, &rho, &cosA, &beta);
    sys->msg("    %d %d(%d) %d ", chgs[i]->index, is_dummy[i], 
            chgs[i]->dummy, chgs[i]->cond);
    if(rho < 0) sys->msg("(%.5e ", rho);
    else sys->msg("( %.5e ", rho);
    if(cosA < 0) sys->msg("%.5e ", cosA);
    else sys->msg(" %.5e ", cosA);
    if(beta < 0) sys->msg("%.5e) ", beta);
    else sys->msg(" %.5e) ", beta);
    if(x < 0) sys->msg("(%.5e ", chgs[i]->x);
    else sys->msg("( %.5e ", chgs[i]->x);
    if(y < 0) sys->msg("%.5e ", chgs[i]->y);
    else sys->msg(" %.5e ", chgs[i]->y);
    if(z < 0) sys->msg("%.5e)\n", chgs[i]->z);
    else sys->msg(" %.5e)\n", chgs[i]->z);
  }
}

/*
  display the matrix built for a given charge to multipole transformation
*/
void dispQ2M(ssystem *sys, double **mat, charge **chgs, int numchgs, double x, double y, double z, int order)
{
  sys->msg("\nQ2M MATRIX: cube at (%.5e %.5e %.5e)\n", x, y, z);
  dumpMat(sys, mat, multerms(order), numchgs);
  sys->msg(
          "    CHARGES IN CUBE # cond (rho_i cos(alpha_i) beta_i) (x y z):\n");
  dumpChgs(sys, chgs, numchgs, x, y, z);
}

/*
  display the matrix built for a given multipole to local transformation
*/
void dispM2L(ssystem *sys, double **mat, double x, double y, double z, double xp, double yp, double zp, int order)
{
  sys->msg(
   "\nM2L MATRIX: multi at (%.5e %.5e %.5e) -> local at (%.5e %.5e %.5e)\n",
          x, y, z, xp, yp, zp);
  dumpMat(sys, mat, multerms(order), multerms(order));
}

/*
  display the matrix built for a given charge to local transformation
*/
void dispQ2L(ssystem *sys, double **mat, charge **chgs, int numchgs, double x, double y, double z, int order)
{
  sys->msg("\nQ2L MATRIX: cube at (%.5e %.5e %.5e)\n", x, y, z);
  dumpMat(sys, mat, multerms(order), numchgs);
  sys->msg(
          "    CHARGES IN CUBE # cond (rho_i cos(alpha_i) beta_i) (x y z):\n");
  dumpChgs(sys, chgs, numchgs, x, y, z);
}

/*
  display the matrix built for a given charge to potential transformation
*/
void dispQ2P(ssystem *sys, double **mat, charge **chgs, int numchgs, int *is_dummy, charge **pchgs, int numpchgs)
{
  sys->msg("\nQ2P MATRIX:\n");
  dumpMat(sys, mat, numpchgs, numchgs);
  sys->msg(
          "    PANELS IN CUBE # dummy(real) cond (rho_i cos(alpha_i) beta_i) (x y z):\n");
  dumpChgsWDummy(sys, chgs, numchgs, is_dummy, 0.0, 0.0, 0.0);
  sys->msg(
          "    EVALS IN CUBE # cond (rho_i cos(alpha_i) beta_i) (x y z):\n");
  dumpChgs(sys, pchgs, numpchgs, 0.0, 0.0, 0.0);
}

/*
  display the matrix built for a given charge to potential transformation
*/
void dispQ2PDiag(ssystem *sys, double **mat, charge **chgs, int numchgs, int *is_dummy)
{
  sys->msg("\nQ2PDiag MATRIX:\n");
  dumpMat(sys, mat, numchgs, numchgs);
  sys->msg(
          "    PANELS IN CUBE # dummy(real) cond (rho_i cos(alpha_i) beta_i) (x y z):\n");
  dumpChgsWDummy(sys, chgs, numchgs, is_dummy, 0.0, 0.0, 0.0);
}

/*
  display the matrix built for a given multipole to multipole transformation
*/
void dispM2M(ssystem *sys, double **mat, double x, double y, double z, double xp, double yp, double zp, int order)
{
  sys->msg(
      "\nM2M MATRIX: cube at (%.5e %.5e %.5e) shifted to (%.5e %.5e %.5e)\n", 
          x, y, z, xp, yp, zp);
  dumpMat(sys, mat, multerms(order), multerms(order));
}

/*
  display the matrix built for a given local to local transformation
*/
void dispL2L(ssystem *sys, double **mat, double x, double y, double z, double xp, double yp, double zp, int order)
{
  sys->msg(
      "\nL2L MATRIX: cube at (%.5e %.5e %.5e) shifted to (%.5e %.5e %.5e)\n", 
          x, y, z, xp, yp, zp);
  dumpMat(sys, mat, multerms(order), multerms(order));
}

/*
  display the matrix built for a given multipole to potential transformation
*/
void dispM2P(ssystem *sys, double **mat, double x, double y, double z, charge **chgs, int numchgs, int order)
{
  sys->msg("\nM2P MATRIX: cube at (%.5e %.5e %.5e)\n", x, y, z);
  dumpMat(sys, mat, numchgs, multerms(order));
  sys->msg(
          "    EVAL PNTS IN CUBE # cond (rho_i, cos(alpha_i), beta_i):\n");
  dumpChgs(sys, chgs, numchgs, x, y, z);
}

/*
  display the matrix built for a given local to potential transformation
*/
void dispL2P(ssystem *sys, double **mat, double x, double y, double z, charge **chgs, int numchgs, int order)
{
  sys->msg("\nL2P MATRIX: cube at (%.5e %.5e %.5e)\n", x, y, z);
  dumpMat(sys, mat, numchgs, multerms(order));
  sys->msg(
          "    EVAL PNTS IN CUBE # cond (rho_i, cos(alpha_i), beta_i):\n");
  dumpChgs(sys, chgs, numchgs, x, y, z);
}

/*
  displays upward pass and moment vectors associated with a cube - debug only
*/
static void dumpUpVecs(ssystem *sys, cube *pc)
{
  int i, j;
  sys->msg(
    "\nUPWARD PASS/MOMENT VECTORS, LEVEL %d CUBE AT (%.5e %.5e %.5e):\n",
          pc->level, pc->x, pc->y, pc->z);
  for(i = 0; i < pc->upnumvects; i++) {
    sys->msg("%d", i);
    for(j = 0; j < pc->upnumeles[i]; j++) {
      if(pc->upvects[i][j] < 0.0) 
          sys->msg(" %.5e", pc->upvects[i][j]);
      else sys->msg("  %.5e", pc->upvects[i][j]);
    }
    sys->msg("\n");
  }
  sys->msg("M");
  for(j = 0; j < pc->multisize; j++) {
    if(pc->multi[j] < 0.0) sys->msg(" %.5e", pc->multi[j]);
    else sys->msg("  %.5e", pc->multi[j]);
  }
  sys->msg("\n");
}

/*
  displays the upward pass vectors for the eight level 1 cubes - debug only
*/
void dumpLevOneUpVecs(ssystem *sys)
{
  int i, j, k;
  cube *****cubes = sys->cubes;
  for(i = 0; i < 2; i++) {
    for(j = 0; j < 2; j++) {
      for(k = 0; k < 2; k++) {
        if(cubes[1][i][j][k] != NULL) dumpUpVecs(sys, cubes[1][i][j][k]);
      }
    }
  }
}

/*
  checks a cube (direct, local or eval) list for bad cube structs - debug only
  -- doesn't quite do this - always uses direct list for one thing
*/
void chkList(ssystem *sys, int listtype)
/* int listtype: DIRECT, LOCAL or EVAL */
{
  int cnt[BUFSIZ];              /* # of cubes processed by level */
  int depth = sys->depth;
  int lev, nn;
  int i, j;
  cube *nc;
  for(i = 0; i <= depth; i++) cnt[i] = 0;
  nc = sys->directlist;
  while(nc != NULL) {
    /* check number and level of neighbors */
    lev = nc->level;
    nn = nc->numnbrs;
    for(i = 0; i < nn; i++) {
      if(lev != ((nc->nbrs)[i])->level) {
        sys->info("chkList: level %d cube has a level %d nbr\n", lev,
                ((nc->nbrs)[i])->level);
        sys->info(" ok cubes ");
        for(j = 0; j <= depth; j++) sys->info("lev%d: %d ", j, cnt[j]);
        sys->info("\n");
        sys->error("List check error - see previous messages for details");
      }
    }
    /* check number of kids */
    if(lev == depth && nc->numkids != 0) {
      sys->info("chkList: level %d cube has children\n", lev);
      sys->info(" ok cubes ");
      for(j = 0; j <= depth; j++) sys->info("lev%d: %d ", j, cnt[j]);
      sys->info("\n");
      sys->error("List check error - see previous messages for details");
    }
    /* if lowest level, check status of eval and direct vects */
    if(lev == depth) {
      if(nc->dindex == 0 || nc->directnumeles == NULL) {
        sys->info("chkList: level %d cube has bad direct info\n", lev);
        sys->info(" ok cubes ");
        for(j = 0; j <= depth; j++) sys->info("lev%d: %d ", j, cnt[j]);
        sys->info("\n");
        sys->error("List check error - see previous messages for details");
      }
      if(nc->evalnumvects == 0 && listtype == EVAL) {
        sys->info("chkList: level %d cube has no eval info\n", lev);
        sys->info(" ok cubes ");
        for(j = 0; j <= depth; j++) sys->info("lev%d: %d ", j, cnt[j]);
        sys->info("\n");
        sys->error("List check error - see previous messages for details");
      }
    }
    cnt[lev]++;
    if(listtype == DIRECT) nc = nc->dnext;
    else if(listtype == LOCAL) nc = nc->lnext;
    else if(listtype == EVAL) nc = nc->enext;
    else {
      sys->error("chkList: bad flag");
    }
  }
  if(listtype == DIRECT) sys->msg("\nDirect ");
  else if(listtype == LOCAL) sys->msg("\nLocal ");
  else sys->msg("\nEval ");
  sys->msg("list ok: ");
  for(j = 0; j <= depth; j++) sys->msg("lev%d: %d ", j, cnt[j]);
  sys->msg("\n\n");
}

/*
  chks a cube for bad cube struct (direct, local or eval) entries - debug only
*/
static void chkCube(ssystem *sys, cube *nc, int listtype)
/* int listtype: DIRECT, LOCAL or EVAL */
{
  int depth = sys->depth;
  int lev, nn;
  int i;
  if(nc != NULL) {
    /* check number and level of neighbors */
    lev = nc->level;
    nn = nc->numnbrs;
    for(i = 0; i < nn; i++) {
      if(lev != ((nc->nbrs)[i])->level) {
        sys->msg("chkCube: level %d cube has a level %d nbr\n", lev,
                ((nc->nbrs)[i])->level);
/*      exit(0);*/
      }
    }
    /* check number of kids */
    if(lev == depth && nc->numkids != 0) {
      sys->msg("chkCube: level %d cube has children\n", lev);
    }
    /* if lowest level, check status of eval and direct vects */
    if(lev == depth) {
      if(nc->dindex == 0) {
        sys->msg("chkCube: level %d cube has zero direct index\n", lev);
      }
      if(nc->directnumeles == NULL) {
        sys->msg(
                "chkCube: level %d cube has NULL directnumeles\n", lev);
      }
      if(nc->evalnumvects == 0 && listtype == EVAL) {
        sys->msg("chkCube: level %d cube has no eval info\n", lev);
      }
      if(nc->eval == NULL && listtype == EVAL) {
        sys->msg("chkCube: level %d cube has no eval pntr\n", lev);
      }
    }
  }
}

/*
  checks the lowest level cubes for trouble using chkCube - debug only
*/
void chkLowLev(ssystem *sys, int listtype)
/* int listtype: DIRECT, LOCAL or EVAL */
{
  int i, j, k, l, side, depth = sys->depth, cnt = 0;
  cube *nc, *****cubes = sys->cubes;
  for(i = 1, side = 1; i <= depth; i++, side *= 2);
  for(j=0; j < side; j++) {     /* loop through all cubes at level depth */
    for(k=0; k < side; k++) {
      for(l=0; l < side; l++) {
        nc = cubes[depth][j][k][l];
        if(nc != NULL) {
          chkCube(sys, nc, listtype);
          cnt++;
        }
      }
    }
  }
  sys->msg("Total lowest level (level %d) cubes checked = %d\n", 
          depth, cnt);
}

/*
  dump the contents of a face struct
*/
void dump_face(ssystem *sys, face *fac)
{
  int i;
  face **behind = fac->behind;

  sys->msg("Face %d, %d sides, depth %d, mark %d, greylev %g\n",
          fac->index, fac->numsides, fac->depth, fac->mark, fac->greylev);
  sys->msg("  plane: n = (%g %g %g) rhs = %g\n",
          fac->normal[0], fac->normal[1], fac->normal[2], fac->rhs);
  sys->msg("  behind [depth(index)]:");
  for(i = 0; i < fac->numbehind; i++) {
    sys->msg(" %d(%d)", behind[i]->depth, behind[i]->index);
    if(i % 10 == 0 && i != 0) sys->msg("\n");
  }
  i--;
  if(!(i % 10 && i != 0)) sys->msg("\n");
  sys->msg(" Corners\n");
  dumpCorners(sys, fac->c, fac->numsides, 3);
}  

/*
  core display routine used below
*/
static void dumpSynCore1(ssystem *sys, char *str, int depth, int *fcnt, int *excnt, int *emcnt, int *tcnt)
{
  int i;
  sys->msg("%-13s", str);
  for(i = 0; i <= depth; i++) {
    sprintf(str, "%d/%d/%d/%d ", fcnt[i], excnt[i], emcnt[i], tcnt[i]);
    if(i < 2) sys->msg("%8s", str);
    else if(i == 2) sys->msg("%12s", str);
    else if(i == 3) sys->msg("%16s", str);
    else if(i == 4) sys->msg("%20s", str);
    else if(i == 5) sys->msg("%24s", str);
    else sys->msg("%28s", str);
  }
  sys->msg("\n");
}
/*
  core display rtn used below
*/
static void dumpSynCore2(ssystem *sys, char *str, int depth, int *cnt)
{
  int i;

  sys->msg("%-13s", str);
  for(i = 0; i <= depth; i++) {
    sprintf(str, "%d    ", cnt[i]);
    if(i < 2) sys->msg("%8s", str);
    else if(i == 2) sys->msg("%12s", str);
    else if(i == 3) sys->msg("%16s", str);
    else if(i == 4) sys->msg("%20s", str);
    else if(i == 5) sys->msg("%24s", str);
    else sys->msg("%28s", str);
  }
  sys->msg("\n");
}

/*
  displays number of exact, full, empty and total cubes per level in
  all cubes, and eval, direct, multi and local lists
*/
void dumpSynop(ssystem *sys)
{
  int i, j, k, l, side, depth = sys->depth, lev;
  int excnt[BUFSIZ], fcnt[BUFSIZ], emcnt[BUFSIZ], tcnt[BUFSIZ];
  char str[BUFSIZ];
  cube *****cubes = sys->cubes, *nc;

  for(i = 0; i <= depth; i++) excnt[i] = fcnt[i] = emcnt[i] = tcnt[i] = 0;

  sys->msg(
          "\nCUBE AND EXPANSION SYNOPSIS (full/mul_exact/empty/ttl):\n");
  sys->msg("             ");
  for(i = 0; i <= depth; i++) {
    sprintf(str, "level%d ", i);
    if(i < 2) sys->msg("%8s", str);
    else if(i == 2) sys->msg("%12s", str);
    else if(i == 3) sys->msg("%16s", str);
    else if(i == 4) sys->msg("%20s", str);
    else if(i == 5) sys->msg("%24s", str);
    else sys->msg("%28s", str);
  }
  sys->msg("\n");
  /* dump cube usage by level */
  for(i = 0, side = 1; i <= depth; i++, side *= 2) {
    for(j=0; j < side; j++) {   /* loop through all cubes at levels >= 0 */
      for(k=0; k < side; k++) {
        for(l=0; l < side; l++) {
          nc = cubes[i][j][k][l];
          tcnt[i]++;
          if(nc != NULL) {
            lev = nc->level;
            fcnt[i]++;
            if(nc->mul_exact == TRUE) excnt[i]++;
          }
          else emcnt[i]++;
        }
      }
    }
  }
  sprintf(str, "All cubes");
  dumpSynCore1(sys, str, depth, fcnt, excnt, emcnt, tcnt);
  
  for(i = 0; i <= depth; i++) excnt[i] = fcnt[i] = emcnt[i] = tcnt[i] = 0;
  /* dump cube direct list by level */
  for(nc = sys->directlist; nc != NULL; nc = nc->dnext) {
    lev = nc->level;
    tcnt[lev]++;
    if(nc->upnumvects > 0) fcnt[lev]++;
    else emcnt[lev]++;
    if(nc->mul_exact == TRUE) excnt[lev]++;
  }
  sprintf(str, "Direct list");
  dumpSynCore1(sys, str, depth, fcnt, excnt, emcnt, tcnt);

  for(i = 0; i <= depth; i++) excnt[i] = fcnt[i] = emcnt[i] = tcnt[i] = 0;
  /* dump cube local list by level */
  for(i = 0; i <= depth; i++) {
    for(nc = sys->locallist[i]; nc != NULL; nc = nc->lnext) {
      lev = nc->level;
      tcnt[lev]++;
      if(nc->upnumvects > 0) fcnt[lev]++;
      else emcnt[lev]++;
      if(nc->mul_exact == TRUE) excnt[lev]++;
    }
  }
  sprintf(str, "Local list");
  dumpSynCore1(sys, str, depth, fcnt, excnt, emcnt, tcnt);
    
  for(i = 0; i <= depth; i++) excnt[i] = fcnt[i] = emcnt[i] = tcnt[i] = 0;
  /* dump cube multipole list by level */
  for(i = 0; i <= depth; i++) {
    for(nc = sys->multilist[i]; nc != NULL; nc = nc->mnext) {
      lev = nc->level;
      tcnt[lev]++;
      if(nc->upnumvects > 0) fcnt[lev]++;
      else emcnt[lev]++;
      if(nc->mul_exact == TRUE) excnt[lev]++;
    }
  }
  sprintf(str, "Multi list");
  dumpSynCore1(sys, str, depth, fcnt, excnt, emcnt, tcnt);


  sprintf(str, "Multis built");
  dumpSynCore2(sys, str, depth, sys->mm.multicnt);

  sprintf(str, "Locals built");
  dumpSynCore2(sys, str, depth, sys->mm.localcnt);

}

/*
  dumps the Gaussian unit (statcoulombs/meter^2) charge densities on panels
*/
void dumpChgDen(ssystem *sys, double *q, charge *chglist)
{
  charge *panel;

  for(panel = chglist; panel != NULL; panel = panel->next) {
    if(panel->dummy) continue;
    sys->msg("%d\tq/A = %.5e %g", panel->index,
            q[panel->index]/panel->area, panel->area);
    if(panel->surf->type == CONDTR) sys->msg(" CONDTR");
    if(panel->surf->type == DIELEC) sys->msg(" DIELEC");
    if(panel->surf->type == BOTH) sys->msg(" BOTH");
    sys->msg(" (%.3g %.3g %.3g)", panel->x, panel->y, panel->z);
    sys->msg(" cond = %d\n", panel->cond);
  }
  sys->flush();
}

/*
  like dumpMat but different formating and row labels (for dumpMatBldCnts)
*/
static void dumpMatCnts(ssystem *sys, int **mat, int depth, char *type)
{
  int i, j;
  char str[BUFSIZ];

  sys->msg(
          "\n%s MATRIX BUILD TOTALS (row = from cube, col = to cube):\n", 
          type);

  for(i = 0; i <= depth; i++) {
    sprintf(str, " to %d ", i);
    if(i == 0) sys->msg("%13s", str);
    else if(i < 10) sys->msg("%6s", str);
    else sys->msg("%7s", str);
  }
  sys->msg("\n");

  for(i = 0; i <= depth; i++) {
    sprintf(str, "from %d ", i);
    sys->msg("%-7s", str); /* print row label */
    for(j = 0; j <= depth; j++) {
      sprintf(str, "%d ", mat[i][j]);
      if(j < 10) sys->msg("%6s", str);
      else sys->msg("%7s", str);
    }
    sys->msg("\n");
  }

}

/*
  display matrix build count totals
*/
void dumpMatBldCnts(ssystem *sys)
{
  char type[BUFSIZ];

  sprintf(type, "Q2M");
  dumpMatCnts(sys, sys->mm.Q2Mcnt, sys->depth, type);

  sprintf(type, "Q2L");
  dumpMatCnts(sys, sys->mm.Q2Lcnt, sys->depth, type);

  sprintf(type, "Q2P");
  dumpMatCnts(sys, sys->mm.Q2Pcnt, sys->depth, type);

  sprintf(type, "M2M");
  dumpMatCnts(sys, sys->mm.M2Mcnt, sys->depth, type);

  sprintf(type, "M2L");
  dumpMatCnts(sys, sys->mm.M2Lcnt, sys->depth, type);

  sprintf(type, "M2P");
  dumpMatCnts(sys, sys->mm.M2Pcnt, sys->depth, type);

  sprintf(type, "L2L");
  dumpMatCnts(sys, sys->mm.L2Lcnt, sys->depth, type);

  sprintf(type, "L2P");
  dumpMatCnts(sys, sys->mm.L2Pcnt, sys->depth, type);

  sprintf(type, "Q2PDiag");
  dumpMatCnts(sys, sys->mm.Q2PDcnt, sys->depth, type);

}

/* 
  dumps state of important compile flags
*/
void dumpConfig(ssystem *sys, const char *name)
{
  int size = -1;

  sys->msg("\n%s CONFIGURATION FLAGS:\n", name);

  sys->msg(" DISCRETIZATION CONFIGURATION\n");

  sys->msg("   WRMETH");
  if(WRMETH == COLLOC)
      sys->msg(" == COLLOC (point collocation)\n");
  else if(WRMETH == SUBDOM)
      sys->msg(" == SUBDOM (not implemented - do collocation)\n");
  else if(WRMETH == GALKIN)
      sys->msg(" == GALKIN (not implemented - do collocation)\n");
  sys->msg("   ELTYPE");
  if(ELTYPE == CONST)
      sys->msg(" == CONST (constant panel densities)\n");
  else if(ELTYPE == AFFINE)
      sys->msg(" == AFFINE (not implemented - use constant)\n");
  else if(ELTYPE == QUADRA)
      sys->msg(" == QUADRA (not implemented - use constant)\n");

  sys->msg(" MULTIPOLE CONFIGURATION\n");

  sys->msg("   DNTYPE");
  if(DNTYPE == NOLOCL) 
      sys->msg(" == NOLOCL (no locals in dwnwd pass)\n");
  else if(DNTYPE == NOSHFT) 
      sys->msg(" == NOSHFT (no local2local shift dwnwd pass)\n");
  else if(DNTYPE == GRENGD) 
      sys->msg(" == GRENGD (full Greengard dwnwd pass)\n");
  sys->msg("   MULTI");
  if(MULTI == ON) sys->msg(" == ON (include multipole part of P*q)\n");
  else sys->msg(" == OFF (don't use multipole part of P*q)\n");
  sys->msg("   RADINTER");
  if(RADINTER == ON) 
      sys->msg(" == ON (allow parent level interaction list entries)\n");
  else 
   sys->msg(" == OFF (use only cube level interaction list entries)\n");
  sys->msg("   NNBRS == %d (max distance to a nrst neighbor)\n", NNBRS);
  sys->msg("   ADAPT");
  if(ADAPT == ON) 
      sys->msg(" == ON (adaptive - no expansions in exact cubes)\n");
  else sys->msg(" == OFF (not adaptive - expansions in all cubes)\n");
  sys->msg("   OPCNT");
  if(OPCNT == ON) 
      sys->msg(" == ON (count P*q ops - exit after mat build)\n");
  else sys->msg(" == OFF (no P*q op count - iterate to convergence)\n");

  sys->msg("   MAXDEP");
  sys->msg(
          " == %d (assume no more than %d partitioning levels are needed)\n",
          MAXDEP, MAXDEP);

  sys->msg("   NUMDPT");
  sys->msg(
          " == %d (do %d potential evaluations for each dielectric panel)\n",
          NUMDPT, NUMDPT);

  sys->msg(" LINEAR SYSTEM SOLUTION CONFIGURATION\n");

  sys->msg("   ITRTYP");
  if(ITRTYP == GCR)
      sys->msg(" == GCR (generalized conjugate residuals)\n");
  else if(ITRTYP == GMRES)
      sys->msg(" == GMRES (generalized minimum residuals)\n");
  else sys->msg(" == %d (not implemented - use GCR)\n", ITRTYP);

  sys->msg("   PRECOND");
  if(PRECOND == BD) {
    sys->msg(
            " == BD (use block diagonal preconditioner)\n");
  }
  else if(PRECOND == OL) {
    sys->msg(
            " == OL (use overlap preconditioner)\n");
  }
  else if(PRECOND == NONE) {
    sys->msg(
            " == NONE (no preconditioner)\n");
  }
  else sys->msg(" == %d (not implemented - use BD, OL or NONE)\n", PRECOND);

  sys->msg("   DIRSOL");
  if (sys->dirsol)
      sys->msg(" == ON (do the whole calculation directly)\n");
  else sys->msg(" == OFF (do the calculation iteratively)\n");

  sys->msg("   EXPGCR");
  if (sys->expgcr)
      sys->msg(" == ON (do all P*q's explicitly w/full matrix)\n");
  else sys->msg(" == OFF (do P*q's with multipole)\n");

  sys->msg("   MAXITER");
  if(MAXITER < 0) {
    sys->msg(" == size (for n panel system, do at most n iterations)\n");
  }
  else sys->msg(" == %d (stop after %d iterations if not converged)\n",
          MAXITER, MAXITER);

  sys->msg("   EXRTSH");
  sys->msg(
          " == %g (exact/ttl cubes > %g on lowest level => stop refinement)\n",
          EXRTSH, EXRTSH);
}


/*
  pads a string on the right up to a given length, truncates if too long
*/
static char *padName(char *tostr, const char *frstr, int len)
{
  int i;
  for(i = 0; frstr && frstr[i] != '\0'; i++) tostr[i] = frstr[i];
  if(i > len) tostr[len] = '\0';                /* truncate */
  else {                        /* pad */
    for(; i < len; i++) tostr[i] = ' ';
    tostr[len] = '\0';
  }
  return(tostr);
}

/*
  returns a string of spaces (doesn't stdio have this somewhere?)
*/
static char *spaces(char *str, int num)
{
  int i;

  for(i = 0; i < num; i++) str[i] = ' ';
  str[num] = '\0';
  return(str);
}
    
/*
  prints the capacitance matrix with symmetrized (averaged) off-diagonals
  - mks units (Farads) are used
  - some attempt to scale (eg pF, nF, uF etc) is made
  - also checks for M-matrix sign pattern, diag dominance
*/
void mksCapDump(ssystem *sys, double **sym_mat)
{
  int i, j, ii, jj, maxlen, sigfig, colwidth, i_killed, j_killed;
  int first_offd = TRUE;
  double maxdiag = 0.0, minoffd = 0.0, scale = 1.0;
  int scale_mag = 0;
  char unit[BUFSIZ], name[BUFSIZ];

  /* get the smallest and largest (absolute value) symmetrized elements */
  /* check for non-M-matrix symmetrized capacitance matrix */
  for (i = 1, ii = 1; i <= sys->num_cond; i++) {

    /* skip conductors removed from input */
    if (sys->kinp_num_list.find(i) != sys->kinp_num_list.end()) {
      continue;
    }

    i_killed = (sys->kill_num_list.find(i) != sys->kill_num_list.end());

    maxdiag = MAX(maxdiag, fabs(sym_mat[ii][ii]));

    for (j = 1, jj = 1; j <= sys->num_cond; j++) {

      /* skip conductors removed from input */
      if (sys->kinp_num_list.find(j) != sys->kinp_num_list.end()) {
        continue;
      }

      j_killed = (sys->kill_num_list.find(j) != sys->kill_num_list.end());
      if (j != i && ! (i_killed && j_killed)) {

        double mat_entry = sym_mat[ii][jj];
        if (mat_entry != 0.0) {
          if (first_offd) {
            minoffd = fabs(mat_entry);
            first_offd = FALSE;
          } else {
            minoffd = MIN(minoffd, fabs(mat_entry));
          }
        }

      }

      ++jj;

    }

    ++ii;

  }

  /* figure the units to use for the matrix entries 
     - set up so smallest element is between 0.1 and 10 */
  double min_value = 1e-20;
  double scale_value = minoffd < min_value ? maxdiag : minoffd;
  if (scale_value > 1e-20) {
    while(scale_value*FPIEPS*sys->perm_factor*scale > 10.0) {
      scale *= 1e-3;
      scale_mag += 3;
    }
    while(scale_value*FPIEPS*sys->perm_factor*scale < 0.1) {
      scale *= 1e+3;
      scale_mag -= 3;
    }
  }

  /* get the appropriate unit string */
  switch (scale_mag) {
  case 18:
    strcpy(unit, "exa");
    break;
  case 15:
    strcpy(unit, "peta");
    break;
  case 12:
    strcpy(unit, "tera");
    break;
  case 9:
    strcpy(unit, "giga");
    break;
  case 6:
    strcpy(unit, "mega");
    break;
  case 3:
    strcpy(unit, "kilo");
    break;
  case 0:
    strcpy(unit, "");
    break;
  case -3:
    strcpy(unit, "milli");
    break;
  case -6:
    strcpy(unit, "micro");
    break;
  case -9:
    strcpy(unit, "nano");
    break;
  case -12:
    strcpy(unit, "pico");
    break;
  case -15:
    strcpy(unit, "femto");
    break;
  case -18:
    strcpy(unit, "atto");
    break;
  default:
    sprintf(unit, "every unit is %g ", 1 / scale);
    break;
  }

  /* get the length of the longest name */
  maxlen = 0;
  for(i = 1; i <= sys->num_cond; i++) {
    if (sys->kinp_num_list.find(i) == sys->kinp_num_list.end()) {
      maxlen = MAX(strlen(sys->conductor_name_str(i)), (size_t)maxlen);
    }
  }

  /* print the matrix */
  sigfig = 2+log10(1.0/sys->iter_tol);  /* get no. significant figs to prnt */
  colwidth = sigfig+6;          /* field width for cap mat columns */

  if (!sys->itrdat) {
    sys->msg("\n");
  }

  if (sys->kill_name_list != NULL) {
    sys->msg("\nPARTIAL CAPACITANCE MATRIX, %sfarads\n", unit);
  } else {
    sys->msg("\nCAPACITANCE MATRIX, %sfarads\n", unit);
  }

  if (sys->num_cond < 10) {
    sys->msg("%s", spaces(unit, maxlen+2));
  } else if(sys->num_cond < 100) {
    sys->msg("%s", spaces(unit, maxlen+3));
  } else {
    sys->msg("%s", spaces(unit, maxlen+4));
  }

  for (j = 1; j <= sys->num_cond; j++) { /* column headings */
    if (sys->kinp_num_list.find(j) == sys->kinp_num_list.end()) {
      sprintf(name, "%d ", j);
      sprintf(unit, "%%%ds", colwidth+1);
      sys->msg(unit, name);
    }
  }

  sys->msg("\n");

  for (i = 1, ii = 1; i <= sys->num_cond; i++) { /* rows */

    /* skip conductors removed from input */
    if (sys->kinp_num_list.find(i) != sys->kinp_num_list.end()) {
      continue;
    }

    sprintf(unit, "%d", i);
    const char *cond_name = sys->conductor_name_str(i);

    if (sys->num_cond < 10) {
      sys->msg("%s %1s", padName(name, cond_name, maxlen), unit);
    } else if (sys->num_cond < 100) {
      sys->msg("%s %2s", padName(name, cond_name, maxlen), unit);
    } else {
      sys->msg("%s %3s", padName(name, cond_name, maxlen), unit);
    }

    for(j = 1, jj = 1; j <= sys->num_cond; j++) {

      /* skip conductors removed from input */
      if (sys->kinp_num_list.find(j) != sys->kinp_num_list.end()) {
        continue;
      }

      if (sys->kill_num_list.find(i) != sys->kill_num_list.end() && sys->kill_num_list.find(j) != sys->kill_num_list.end()) {
        /* print a blank if capacitance was not calculated */
        sys->msg("%s", spaces(unit, colwidth+1));
      } else {
        sprintf(unit, " %%%d.%dg", colwidth, sigfig);
        sys->msg(unit, scale*FPIEPS*sys->perm_factor*sym_mat[jj][ii]);
      }

      ++jj;

    }

    sys->msg("\n");

    ++ii;

  }
}

/*
  dumps brief information about multipole set up
*/
void dumpMulSet(ssystem *sys)
{
  int numcubes, numsides, i;

  for(numcubes = 1, i = 0; i < sys->depth; numcubes *= 8, i++);
  for(numsides = 1, i = 0; i < sys->depth; numsides *= 2, i++);

  sys->msg("\nMULTIPOLE SETUP SUMMARY\n");
  sys->msg("Level 0 cube extremal coordinates\n");
  sys->msg("  x: %g to %g\n", 
          sys->minx, sys->minx + numsides * (sys->length));
  sys->msg("  y: %g to %g\n", 
          sys->miny, sys->miny + numsides * (sys->length));
  sys->msg("  z: %g to %g\n", 
          sys->minz, sys->minz + numsides * (sys->length));
  sys->msg("Level %d (lowest level) cubes\n  %d total\n", 
          sys->depth, numcubes);
  sys->msg(
          "  side length = %g\n  maximum number of panels in each = %d\n",
          sys->length, sys->mul_maxlq);
  sys->msg("  maximum number of evaluation points in each = %d\n",
          sys->loc_maxlq);
  sys->msg(
          "Maximum number of panels treated with a multipole expansion = %d\n",
          sys->max_panel);
  sys->msg(
  "Maximum number of evaluation points treated with a local expansion = %d\n",
          sys->max_eval_pnt);
  sys->msg(
          "Maximum number of panels treated exactly = %d (limit = %d)\n",
          sys->mul_maxq, multerms(sys->order));
  sys->msg(
   "Maximum number of evaluation points treated exactly = %d (limit = %d)\n",
          sys->loc_maxq, multerms(sys->order));
}
/*
  dump all the preconditioner matrices in the direct list cubes as one
  big matrix for matlab read in "Ctil"
  - figures preconditioner by multiplying it by columns of the inverse
    and dumping results one column at a time
  - figures the unpreconditioned matrix (using calcp) and dumps it to "P"
  - type determines which of the matrices to dump
*/
void dump_preconditioner(ssystem *sys, charge *chglist, int type)
/* int type: 1=>dump P and C; 2=>P only; 3=>C only */
{
  int num_panels, i, j;
  charge *pp, *pi;
  FILE *fp;

  /* find the number of panels */
  for(num_panels = 0, pp = chglist; pp != NULL; pp = pp->next, num_panels++);

  /* open the output file */
  if((fp = fopen("prec.mat","w")) == NULL) {
    sys->error("dump_preconditioner: can't open `prec.mat'");
  }

  if(type == 1 || type == 3) {
    sys->msg("\nDumping preconditioner to `prec.mat' as `Ctil'\n");
    /* dump the preconditioner one column at a time */
    /* - savemat arg "type" can be used to make rowwise dumps
         type = x0xx  =>  columnwise dumps
         type = x1xx  =>  rowwise dumps (see matlab manual) */
    for(j = 1; j < num_panels+1; j++) {
      /* load I col into q and zero p */
      for(i = 0; i < num_panels+1; i++) {
        if(i == j) sys->q[i] = 1.0;
        else sys->q[i] = 0.0;
      }
      /* figure the column of C in p (xfered to q after calculation) */
      mulPrecond(sys, PRECOND);
      /* dump the preconditioner column */
      if(j == 1) savemat_mod(fp, 1000, "Ctil", num_panels, num_panels, 0,
                             &(sys->q[1]), (double *)NULL, 0, num_panels);
      else savemat_mod(fp, 1000, "Ctil", num_panels, num_panels, 0,
                       &(sys->q[1]), (double *)NULL, 1, num_panels);

    }
  }

  if(type == 1 || type == 2) {
    sys->msg("\nDumping pot. coeff. mat. to `prec.mat' as `P'\n");
    /* dump the P matrix - DANGER: does n^2 calcp() calls,
       but storage is only n */
    /* are q indices from 1?? */
    for(j = 1; j < num_panels+1; j++) {
      /* find the panel with the current index (select column) */
      for(pp = chglist; pp != NULL; pp = pp->next) {
        if(pp->index == j) break;
      }
      if(pp == NULL) {
        sys->error("dump_preconditioner: no charge with index %d", j);
      }
      for(i = 0; i < num_panels+1; i++) sys->p[i] = 0.0;
      /* find the column---influence of q_j on potentials at each q_i */
      for(i = 1, pi = chglist; pi != NULL; i++, pi = pi->next) {
        sys->p[pi->index] = calcp(sys, pp, pi->x, pi->y, pi->z, NULL);
      }
      /* dump the column */
      if(j == 1) savemat_mod(fp, 1000, "P", num_panels, num_panels, 0,
                             &(sys->p[1]), (double *)NULL, 0, num_panels);
      else savemat_mod(fp, 1000, "P", num_panels, num_panels, 0,
                       &(sys->p[1]), (double *)NULL, 1, num_panels);

    }
  }

  fclose(fp);
}

/*
  do an n^2/2 check to see if any panels have the same center points
  (debug only)
*/
int has_duplicate_panels(ssystem *sys, charge *chglst)
{
  int no_duplicates;
  charge *cp, *cpinner;

  no_duplicates = TRUE;
  for(cp = chglst; cp != NULL; cp = cp->next) {
    for(cpinner = cp->next; cpinner != NULL; cpinner = cpinner->next) {
      if(cp->x == cpinner->x && cp->y == cpinner->y && cp->z == cpinner->z) {
        no_duplicates = FALSE;

        if(cp->surf->type == CONDTR) sys->msg("Panels %d(CONDTR)", cp->index);
        if(cp->surf->type == DIELEC) sys->msg("Panels %d(DIELEC)", cp->index);
        if(cp->surf->type == BOTH) sys->msg("Panels %d(BOTH)", cp->index);

        if(cpinner->surf->type == CONDTR) sys->msg(" and %d(CONDTR)", cpinner->index);
        if(cpinner->surf->type == DIELEC) sys->msg(" and %d(DIELEC)", cpinner->index);
        if(cpinner->surf->type == BOTH) sys->msg(" and %d(BOTH)", cpinner->index);

        sys->msg(" both have center (%.3g %.3g %.3g)\n", cp->x, cp->y, cp->z);
      }
    }
  }

  if(no_duplicates) return(FALSE);
  else return(TRUE);
}

/*
  dump the condensed matrix for matlab use
*/
void dumpQ2PDiag(ssystem *sys, cube *nextc)
{
  int i, j, ind, pos_d, neg_d;
  double temp[BUFSIZ], temp_mat[100][100], **rmat;
  double pos_fact, neg_fact, panel_fact;
  FILE *fp;

  /* dump the diag matrix to a matlab file along with its dummy vector */
  /*   must complie on sobolev with /usr/local/matlab/loadsave/savemat.o */
  if((fp = fopen("Q2PDiag.mat", "w")) == NULL) {
    sys->error("dumpQ2PDiag: can't open `Q2PDiag.mat' to write");
  }
  if(int(sizeof(temp)) < nextc->upnumeles[0]*nextc->upnumeles[0]) {
    sys->error("dumpQ2PDiag: temporary arrays not big enough");
  }

  /* incorporate the electric field evaluation into the matrix */
  /* if a row corresponds to a flux density difference, alter it */
  rmat = nextc->directmats[0];
  for(i = 0; i < nextc->upnumeles[0]; i++) {
    if(nextc->chgs[i]->dummy) {
      for(j = 0; j < nextc->upnumeles[0]; j++) temp_mat[i][j] = 0.0;
      continue;
    }

    if(nextc->chgs[i]->surf->type == CONDTR) {
      for(j = 0; j < nextc->upnumeles[0]; j++) {
        temp_mat[i][j] = rmat[i][j];
      }
    }
    else {

      pos_fact 
          = nextc->chgs[i]->surf->outer_perm/nextc->chgs[i]->pos_dummy->area;
      pos_d = nextc->chgs[i]->pos_dummy->index - 1;
      neg_fact 
          = nextc->chgs[i]->surf->inner_perm/nextc->chgs[i]->neg_dummy->area;
      neg_d = nextc->chgs[i]->neg_dummy->index - 1;
      panel_fact = pos_fact + neg_fact;

      for(j = 0; j < nextc->upnumeles[0]; j++) {
        temp_mat[i][j] = pos_fact*rmat[pos_d][j] - panel_fact*rmat[i][j]
            + neg_fact*rmat[neg_d][j];
      }
    }
  }
  
  /* flatten the Q2PDiag matrix */
  for(j = ind = 0; j < nextc->upnumeles[0]; j++) {
    for(i = 0; i < nextc->upnumeles[0]; i++) {
      temp[ind++] = temp_mat[i][j];
    }
  }
  savemat(fp, 1000, "A", nextc->upnumeles[0], nextc->upnumeles[0],
          0, temp, (double *)NULL);

  /* make the is_dummy vector a vector of doubles */
  for(i = 0; i < nextc->upnumeles[0]; i++) 
      temp[i] = (double)(nextc->nbr_is_dummy[0][i]);
  savemat(fp, 1000, "is_dummy", nextc->upnumeles[0], 1,
          0, temp, (double *)NULL);

  /* make a vector with 0 => CONDTR 1 => DIELEC 2 => BOTH -1 => dummy */
  for(i = 0; i < nextc->upnumeles[0]; i++) {
    if(nextc->chgs[i]->dummy) temp[i] = -1.0;
    else temp[i] = (double)(nextc->chgs[i]->surf->type);
  }
  savemat(fp, 1000, "surf_type", nextc->upnumeles[0], 1,
          0, temp, (double *)NULL);

  fclose(fp);
  sys->msg("Dumped Q2PDiag matrix to `Q2PDiag.mat'\n");
}

/*
  debug only - check a vector to make sure it has zeros in dummy entries
*/
void chkDummy(ssystem *sys, double *vector, int *is_dummy, int size)
{
  int i, first = TRUE;

  for(i = 0; i < size; i++) {
    if(is_dummy[i] && vector[i] != 0.0) {
      if(first) {
        first = FALSE;
        sys->info("\nchkDummy: entries should be 0.0: %d", i);
      }
      else sys->info(" %d", i);
    }
  }
  if(!first) sys->info("\n");
}

/*
  debug only - print message if dummy list doesn't match panel list
*/
void chkDummyList(ssystem *sys, charge **panels, int *is_dummy, int n_chgs)
{
  int i;
  int first = TRUE;
  
  for(i = 0; i < n_chgs; i++) {
    if((is_dummy[i] && !panels[i]->dummy) || (!is_dummy[i] && panels[i]->dummy)) {
      if(first) {
        first = FALSE;
        sys->info("chkDummyList: inconsistent dummy list entries:\n");
      }
      sys->info(" %d is_dummy = %d, panel->dummy = %d\n", i,
              is_dummy[i], panels[i]->dummy);
    }
  }

}

/*
  print the conductor names
*/
void dumpCondNames(ssystem *sys)
{ 
  int i;
  Name *cur_name;

  sys->msg("CONDUCTOR NAMES\n");
  for(cur_name = sys->cond_names, i = 0; cur_name != NULL;
      cur_name = cur_name->next, i++) {
    sys->msg("  %d `%s'\n", i+1, cur_name->last_alias());
  }
}

/*
  debug only: dump state of internal conductor name list
*/
int dumpNameList(ssystem *sys, Name *name_list)
{
  Name *cur_name, *cur_alias;

  for(cur_name = name_list; cur_name != NULL; cur_name = cur_name->next) {
    sys->msg("`%s'\n", cur_name->name);
    for(cur_alias = cur_name->alias_list; cur_alias != NULL; 
        cur_alias = cur_alias->next) {
      sys->msg("  `%s'\n", cur_alias->name);
    }
  }
  return(TRUE);
}

