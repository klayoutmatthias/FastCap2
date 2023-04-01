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

#include "mulGlobal.h"
#include "mulDisplay.h"
#include "mulDo.h"
#include "capsolve.h"
#include "input.h"
#include "zbufInOut.h"
#include "zbuf2fastcap.h"
#include "electric.h"
#include "quickif.h"
#include "blkDirect.h"
#include "direct.h"
#include "resusage.h"

#include <cmath>

static int gmres(ssystem *sys, double *q, double *p, double *r, double *ap, double **bv, double **bh, int size, int real_size, int maxiter, double tol, charge *chglist);
static void computePsi(ssystem *sys, double *q, double *p, int size, int real_size, charge *chglist);
static int gcr(ssystem *sys, double *q, double *p, double *r, double *ap, double **bp, double **bap, int size, int real_size, int maxiter, double tol, charge *chglist);

/* This routine takes the cube data struct and computes capacitances. */
int capsolve(double ***capmat, ssystem *sys, charge *chglist, int size, int real_size, int numconds, Name *name_list)
/* double ***capmat: pointer to capacitance matrix */
/* real_size: real_size = total #panels, incl dummies */
{
  int i, cond, iter, maxiter = MAXITER, ttliter = 0;
  charge *nq;
  double *q, *p, *r, *ap;
  double **bp, **bap;
  extern double fullsoltime;
  surface *surf;
  extern ITER *kill_num_list, *kinp_num_list;
  extern double iter_tol;

  extern ITER *qpic_num_list;
  extern int q_;
  extern int dd_;

  extern double *trimat, *sqrmat; /* globals in blkDirect.c */

  /* Allocate space for the capacitance matrix. */
  *capmat = sys->heap.alloc<double *>(numconds+1, AMSC);
  for(i=1; i <= numconds; i++)  {
    (*capmat)[i] = sys->heap.alloc<double>(numconds+1, AMSC);
  }

  /* Allocate space for cg vectors , r=residual and p=projection, ap = Ap. */
  q = sys->heap.alloc<double>(size+1, AMSC);
  r = sys->heap.alloc<double>(size+1, AMSC);

  if (DIRSOL != ON) {		/* too much to allocate if not used */

    /* allocate for gcr accumulated basis vectors (moved out of loop 30Apr90) */
    fflush(stdout);		/* so header will be saved if crash occurs */

    bp = sys->heap.alloc<double *>(maxiter+1, AMSC);
    bap = sys->heap.alloc<double *>(maxiter+1, AMSC);

  }

  /* ualloc_verify(); */

  /* P is the "psuedo-charge" for multipole. Ap is the "psuedo-potential". */
  p = sys->q;
  ap = sys->p;

  /* Loop through all the conductors. */
  for(cond=1; cond <= numconds; cond++) {
    
    /* skip conductors in the -rs and the -ri kill list */
    if(want_this_iter(kill_num_list, cond)
       || want_this_iter(kinp_num_list, cond)) continue;

    fprintf(stdout, "\nStarting on column %d (%s)\n", cond, 
	    getConductorName(cond, &name_list));
    fflush(stdout);

    /* Set up the initial residue vector and charge guess. */
    for(i=1; i <= size; i++) r[i] = q[i] = 0.0;
    i = 0;
    for(nq = chglist; nq != NULL; nq = nq->next) {
      if(nq->cond == cond && !nq->dummy 
	 && (nq->surf->type == CONDTR || nq->surf->type == BOTH)) 
	  r[nq->index] = 1.0;
    }

    if (DIRSOL == ON) {

      /* do a direct forward elimination/back solve for the charge vector */
      if(size > MAXSIZ) {		/* index from 1 here, from 0 in solvers */
        blkCompressVector(r+1, size, real_size, sys->is_dummy+1);
        blkSolve(q+1, r+1, real_size, trimat, sqrmat);
        blkExpandVector(q+1, size, real_size);
      }
      else {
        starttimer;
        solve(sys->directlist->directmats[0], q+1, r+1, size);
        stoptimer;
        fullsoltime += dtime;
      }
      iter = 0;

    } else {

      /* Do gcr. First allocate space for back vectors. */
      /* allocation moved out of loop 30Apr90 */
      if (ITRTYP == GMRES) {
        if((iter = gmres(sys,q,p,r,ap,bp,bap,size,real_size,maxiter,iter_tol,chglist))
           > maxiter) {
          fprintf(stderr, "NONCONVERGENCE AFTER %d ITERATIONS\n", maxiter);
          exit(0);
        }
      } else {
        if((iter = gcr(sys,q,p,r,ap,bp,bap,size,real_size,maxiter,iter_tol,chglist))
           > maxiter) {
          fprintf(stderr, "NONCONVERGENCE AFTER %d ITERATIONS\n", maxiter);
          exit(0);
        }
      }

      ttliter += iter;

    }

    if (sys->dmpchg == DMPCHG_LAST) {
      fprintf(stdout, "\nPanel charges, iteration %d\n", iter);
      dumpChgDen(stdout, q, chglist);
      fprintf(stdout, "End panel charges\n");
    }

    if (sys->capvew) {
      /* dump shaded geometry file if only if this column picture wanted */
      /* (variable names are messed up - iter list now is list of columns) */
      if(want_this_iter(qpic_num_list, cond) || (q_ && qpic_num_list == NULL)) {
        dump_ps_geometry(sys, chglist, q, cond, dd_);
      }
    }

    /* Calc cap matrix entries by summing up charges over each conductor. */
    /* use the permittivity ratio to get the real surface charge */
    /* NOT IMPLEMENTED: fancy stuff for infinitessimally thin conductors */
    /* (once again, permittivity data is poorly organized, lots of pointing) */
    for(i=1; i <= numconds; i++) (*capmat)[i][cond] = 0.0;
    for(nq = chglist; nq != NULL; nq = nq->next) {
      if(nq->dummy || (surf = nq->surf)->type != CONDTR) continue;
      (*capmat)[nq->cond][cond] += surf->outer_perm * q[nq->index];
    }

    if (sys->rawdat) {
      if(!sys->itrdat) fprintf(stdout, "\n");
      fprintf(stdout, "cond=%d iters=%d\n", cond, iter);

      for(i=1; i <= numconds; i++) {
        fprintf(stdout, "c%d%d=%g  ", i, cond, (*capmat)[i][cond]);
        if(i % 4 == 0) fprintf(stdout, "\n");
      }
      fprintf(stdout, "\n\n");
    }

    if (sys->itrdat && sys->rawdat) {
      fprintf(stdout, "%d iterations\n", iter);
    }

  }
  fflush(stdout);
  return(ttliter);
}


/* 
Preconditioned(possibly) Generalized Conjugate Residuals.
*/
static int gcr(ssystem *sys, double *q, double *p, double *r, double *ap, double **bp, double **bap, int size, int real_size, int maxiter, double tol, charge *chglist)
{
  int iter, i, j;
  double norm, beta, alpha, maxnorm;
  extern double conjtime;

  /* NOTE ON EFFICIENCY: all the loops of length "size" could have */
  /*   if(sys->is_dummy[i]) continue; as their first line to save some ops */
  /* currently the entries corresponding to dummy panels are set to zero */

  for(iter = 0; iter < maxiter; iter++) {

    /* allocate the back vectors if they haven't been already (22OCT90) */
    if(bp[iter] == NULL) {
      bp[iter] = sys->heap.alloc<double>(size+1, AMSC);
      bap[iter] = sys->heap.alloc<double>(size+1, AMSC);
    }

    for(i=1; i <= size; i++) {
      bp[iter][i] = p[i] = r[i];
    }

    computePsi(sys, p, ap, size, real_size, chglist);
    
    starttimer;
    for(i=1; i <= size; i++) {
      bap[iter][i] = ap[i];
    }
    
    /* Subtract the backward projections from p and ap. */
    for(j= 0; j < iter; j++) {
      INNER(beta, ap, bap[j], size);
      for(i=1; i <= size; i++) {
	bp[iter][i] -= beta * bp[j][i];
	bap[iter][i] -= beta * bap[j][i];
      }
    }
    
    /* Normalize the p and ap vectors so that ap*ap = 1. */
    INNER(norm, bap[iter], bap[iter], size);
    norm = sqrt(norm);
    for(i=1; i <= size; i++) {
      bap[iter][i] /= norm;
      bp[iter][i] /= norm;
    }
    
    /* Compute the projection in the p direction and get the next p. */
    INNER(alpha, r, bap[iter], size);
    for(i=1; i <= size; i++) {
      q[i] += alpha * bp[iter][i];
      r[i] -= alpha * bap[iter][i];
    }

    /* Check convergence. */
    for(maxnorm = 0.0, i=1; i <= size; i++) maxnorm = MAX(ABS(r[i]),maxnorm);
    if (sys->itrdat) {
      INNER(norm, r, r, size);
      fprintf(stdout, "max res = %g ||res|| = %g\n", maxnorm, sqrt(norm));
    } else {
      fprintf(stdout, "%d ", iter+1);
      if((iter+1) % 15 == 0) fprintf(stdout, "\n");
    }
    fflush(stdout);
    stoptimer;
    conjtime += dtime;
    if(maxnorm < tol) break;
  }
  
  if (PRECOND != NONE) {
    /* Undo the preconditioning to get the real q. */
    for(i=1; i <= size; i++) {
      p[i] = q[i];
      ap[i] = 0.0;
    }
    mulPrecond(sys, PRECOND);
    for(i=1; i <= size; i++) {
      q[i] = p[i];
    }
  }
  
  if(maxnorm >= tol) {
    fprintf(stdout, "\ngcr: WARNING exiting without converging\n");
  }
  return(iter+1);
}


/* 
  Preconditioned(possibly) Generalized Minimum Residual. 
  */
static int gmres(ssystem *sys, double *q, double *p, double *r, double *ap, double **bv, double **bh, int size, int real_size, int maxiter, double tol, charge *chglist)
{
  int iter, i, j;
  double rnorm, norm;
  double hi, hip1, length;
  extern double conjtime, prectime;

  static double *c=NULL, *s=NULL, *g=NULL, *y=NULL;
  
  starttimer;

  /* Allocation first time through. */
  if(c == NULL) {
    c = sys->heap.alloc<double>(size+1, AMSC);
    s = sys->heap.alloc<double>(size+1, AMSC);
    g = sys->heap.alloc<double>(size+1, AMSC);
    y = sys->heap.alloc<double>(size+1, AMSC);
  }
  
  /* Set up v^1 and g^0. */
  INNER(rnorm, r, r, size);
  rnorm = sqrt(rnorm);
  for(i=1; i <= size; i++) {
    p[i] = r[i] / rnorm;
    g[i] = 0.0;
  }
  g[1] = rnorm;

  stoptimer;
  conjtime += dtime;

  if (sys->itrdat) {
    fprintf(stdout, "||res|| = %g\n", rnorm); /* initial guess residual norm */
  }
  
  for(iter = 1; (iter <= maxiter) && (rnorm > tol); iter++) {
    
    starttimer;
    /* allocate the back vectors if they haven't been already */
    if(bv[iter] == NULL) {
      bv[iter] = sys->heap.alloc<double>(size+1, AMSC);
      bh[iter] = sys->heap.alloc<double>(size+2, AMSC);
    }
    
    /* Save p as the v{iter}. */
    for(i=1; i <= size; i++) bv[iter][i] = p[i];
    
    stoptimer;
    conjtime += dtime;

    /* Form Av{iter}. */
    computePsi(sys, p, ap, size, real_size, chglist);

    starttimer;
    
    /* Initialize v^{iter+1} to Av^{iter}. */
    for(i=1; i <= size; i++) p[i] = ap[i];
    
    /* Make v^{iter+1} orthogonal to v^{i}, i <= iter. */
    for(j=1; j <= iter; j++) {
      INNER(hi, ap, bv[j], size);
      for(i=1; i <= size; i++) p[i] -= hi * bv[j][i];
      bh[iter][j] = hi;
    }
    
    /* Normalize v^{iter+1}. */
    INNER(norm, p, p, size);    
    norm = sqrt(norm);
    for(i=1; i <= size; i++) p[i] /= norm;
    bh[iter][iter+1] = norm;
    
    /* Apply rotations to new h column. */
    for(i=1; i < iter; i++) {
      hi = bh[iter][i];
      hip1 = bh[iter][i+1];
      bh[iter][i] = c[i] * hi - s[i] * hip1;
      bh[iter][i+1] = c[i] * hip1 + s[i] * hi;
    }
    
    /* Compute new rotations. */
    hi = bh[iter][iter];
    hip1 = bh[iter][iter+1];
    length = sqrt(hi * hi + hip1 * hip1);
    c[iter] = hi/length;
    s[iter] = -hip1/length;
    
    /* Apply new rotations. */
    bh[iter][iter] = c[iter] * hi - s[iter] * hip1;
    bh[iter][iter+1] = c[iter] * hip1 + s[iter] * hi;
    /* ASSERT(g[iter+1] == 0); WHY IS THIS HERE ??? */
    hi = g[iter];
    g[iter] = c[iter] * hi;
    g[iter+1] = s[iter] * hi;    
    
    rnorm = ABS(g[iter+1]);

    stoptimer;
    conjtime += dtime;

    if (sys->itrdat) {
      fprintf(stdout, "||res|| = %g\n", rnorm);
    } else {
      fprintf(stdout, "%d ", iter);
      if((iter) % 15 == 0 && iter != 0) fprintf(stdout, "\n");
    }
    fflush(stdout);
  }
  /* Decrement from the last increment. */
  iter--;

  starttimer;
  
  /* Compute solution, note, bh is bh[col][row]. */
  for(i=1; i <= iter; i++) y[i] = g[i];
  for(i = iter; i > 0; i--) {
    y[i] /=  bh[i][i];
    for(j = i-1; j > 0; j--) {
      y[j] -= bh[i][j]*y[i];
    }
  }
  for(i=1; i <= size; i++) {
    q[i] = 0.0;
    for(j=1; j <= iter; j++) {
      q[i] += y[j] * bv[j][i];
    }
  }

  stoptimer;
  conjtime += dtime;
  
  if (PRECOND != NONE) {
    /* Undo the preconditioning to get the real q. */
    starttimer;
    for(i=1; i <= size; i++) {
      p[i] = q[i];
      ap[i] = 0.0;
    }
    mulPrecond(sys, PRECOND);
    for(i=1; i <= size; i++) {
      q[i] = p[i];
    }
    stoptimer;
    prectime += dtime;
  }

  if(rnorm > tol) {
    fprintf(stdout, "\ngmres: WARNING exiting without converging\n");
  }
  return(iter);
}

/* 
ComputePsi computes the potential from the charge vector, or may
include a preconditioner.  It is assumed that the vectors for the
charge and potential have already been set up and that the potential
vector has been zeroed.  ARBITRARY VECTORS CAN NOT BE USED.
*/

static void computePsi(ssystem *sys, double *q, double *p, int size, int real_size, charge *chglist)
{
  extern double dirtime, uptime, downtime, evaltime, prectime;
  int i;

  ASSERT(p == sys->p);
  ASSERT(q == sys->q);

  for(i=1; i <= size; i++) p[i] = 0;

  if (PRECOND != NONE) {
    starttimer;
    mulPrecond(sys, PRECOND);
    stoptimer;
    prectime += dtime;
  }

  if (EXPGCR == ON) {

    blkCompressVector(q+1, size, real_size, sys->is_dummy+1);
    blkAqprod(p+1, q+1, real_size, sqrmat);	/* offset since index from 1 */
    blkExpandVector(p+1, size, real_size); /* ap changed to p, r chged to q */
    blkExpandVector(q+1, size, real_size); /*    7 Oct 91 */

  } else {

    starttimer;
    mulDirect(sys);
    stoptimer;
    dirtime += dtime;

    starttimer;
    mulUp(sys);
    stoptimer;
    uptime += dtime;

    if (sys->dupvec) {
      dumpLevOneUpVecs(sys);
    }

#if DNTYPE == NOSHFT
    mulDown(sys);		/* do downward pass without local exp shifts */
#endif

#if DNTYPE == GRENGD
    mulDown(sys);	       	/* do heirarchical local shift dwnwd pass */
#endif
    stoptimer;
    downtime += dtime;

    starttimer;
#if MULTI == ON
    mulEval(sys);		/* evaluate either locals or multis or both */
#endif
    stoptimer;
    evaltime += dtime;

    if (sys->dmpchg == DMPCHG_LAST) {
      fprintf(stdout, "\nPanel potentials divided by areas\n");
      dumpChgDen(stdout, p, chglist);
      fprintf(stdout, "End panel potentials\n");
    }

    /* convert the voltage vec entries on dielectric i/f's into eps1E1-eps2E2 */
    compute_electric_fields(sys, chglist);

#if OPCNT == ON
    printops();
    exit(0);
#endif				/* OPCNT == ON */

  }
}
