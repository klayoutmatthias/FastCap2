
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
#include "counters.h"

#include <cmath>
#include <cassert>
#include <string>
#include <sstream>

static int gmres(ssystem *sys, double *q, double *p, double *r, double *ap, double **bv, double **bh, int size, int real_size, double *sqrmat, int *real_index, int maxiter, double tol, charge *chglist);
static void computePsi(ssystem *sys, double *q, double *p, int size, int real_size, double *sqrmat, int *real_index, charge *chglist);
static int gcr(ssystem *sys, double *q, double *p, double *r, double *ap, double **bp, double **bap, int size, int real_size, double *sqrmat, int *real_index, int maxiter, double tol, charge *chglist);

/* This routine takes the cube data struct and computes capacitances. */
int capsolve(double ***capmat, ssystem *sys, charge *chglist, int size, int real_size, double *trimat, double *sqrmat, int *real_index)
/* double ***capmat: pointer to capacitance matrix */
/* real_size: real_size = total #panels, incl dummies */
{
  int i, cond, iter, maxiter = MAXITER, ttliter = 0;
  charge *nq;
  double *q, *p, *r, *ap;
  double **bp = 0, **bap = 0;
  Surface *surf;

  /* Allocate space for the capacitance matrix. */
  *capmat = sys->heap.mat(sys->num_cond+1, sys->num_cond+1);

  /* Allocate space for cg vectors , r=residual and p=projection, ap = Ap. */
  q = sys->heap.alloc<double>(size+1, AMSC);
  r = sys->heap.alloc<double>(size+1, AMSC);

  if (! sys->dirsol) {          /* too much to allocate if not used */

    /* allocate for gcr accumulated basis vectors (moved out of loop 30Apr90) */
    sys->flush();             /* so header will be saved if crash occurs */

    bp = sys->heap.alloc<double *>(maxiter+1, AMSC);
    bap = sys->heap.alloc<double *>(maxiter+1, AMSC);

  }

  /* P is the "psuedo-charge" for multipole. Ap is the "psuedo-potential". */
  p = sys->q;
  ap = sys->p;

  /* Loop through all the conductors. */
  for (cond = 1; cond <= sys->num_cond; cond++) {
    
    /* skip conductors in the -rs and the -ri kill list */
    if (sys->kill_num_list.find(cond) != sys->kill_num_list.end() || sys->kinp_num_list.find(cond) != sys->kinp_num_list.end()) {
      continue;
    }

    sys->msg("\nStarting on column %d (%s)\n", cond, sys->conductor_name_str(cond));
    sys->flush();

    /* Set up the initial residue vector and charge guess. */
    for(i=1; i <= size; i++) r[i] = q[i] = 0.0;
    i = 0;
    for(nq = chglist; nq != NULL; nq = nq->next) {
      if(nq->cond == cond && !nq->dummy 
         && (nq->surf->type == CONDTR || nq->surf->type == BOTH)) 
          r[nq->index] = 1.0;
    }

    if (sys->dirsol) {

      /* do a direct forward elimination/back solve for the charge vector */
      if(size > MAXSIZ) {               /* index from 1 here, from 0 in solvers */
        blkCompressVector(sys, r+1, size, real_size, sys->is_dummy+1);
        blkSolve(sys, q+1, r+1, real_size, trimat, sqrmat);
        blkExpandVector(q+1, size, real_size, real_index);
      }
      else {
        starttimer;
        solve(sys->directlist->directmats[0], q+1, r+1, size);
        stoptimer;
        counters.fullsoltime += dtime;
      }
      iter = 0;

    } else {

      /* Do gcr. First allocate space for back vectors. */
      /* allocation moved out of loop 30Apr90 */
      if (ITRTYP == GMRES) {
        if((iter = gmres(sys,q,p,r,ap,bp,bap,size,real_size,sqrmat,real_index,maxiter,sys->iter_tol,chglist))
           > maxiter) {
          sys->error("NONCONVERGENCE AFTER %d ITERATIONS", maxiter);
        }
      } else {
        if((iter = gcr(sys,q,p,r,ap,bp,bap,size,real_size,sqrmat,real_index,maxiter,sys->iter_tol,chglist))
           > maxiter) {
          sys->error("NONCONVERGENCE AFTER %d ITERATIONS", maxiter);
        }
      }

      ttliter += iter;

    }

    if (sys->dmpchg == DMPCHG_LAST) {
      sys->msg("\nPanel charges, iteration %d\n", iter);
      dumpChgDen(sys, q, chglist);
      sys->msg("End panel charges\n");
    }

    if (sys->capvew && sys->ps_file_base) {
      /* dump shaded geometry file if only if this column picture wanted */
      /* (variable names are messed up - iter list now is list of columns) */
      if (sys->qpic_num_list.find(cond) != sys->qpic_num_list.end() || (sys->q_ && sys->qpic_name_list == NULL)) {
        /* set up ps file name */
        std::ostringstream os;
        os << sys->ps_file_base << cond << ".ps";
        dump_ps_geometry(sys, os.str().c_str(), chglist, q, sys->dd_);
      }
    }

    /* Calc cap matrix entries by summing up charges over each conductor. */
    /* use the permittivity ratio to get the real surface charge */
    /* NOT IMPLEMENTED: fancy stuff for infinitessimally thin conductors */
    /* (once again, permittivity data is poorly organized, lots of pointing) */
    for(i=1; i <= sys->num_cond; i++) (*capmat)[i][cond] = 0.0;
    for(nq = chglist; nq != NULL; nq = nq->next) {
      if(nq->dummy || (surf = nq->surf)->type != CONDTR) continue;
      (*capmat)[nq->cond][cond] += surf->outer_perm * q[nq->index];
    }

    if (sys->rawdat) {
      if(!sys->itrdat) sys->msg("\n");
      sys->msg("cond=%d iters=%d\n", cond, iter);

      for(i=1; i <= sys->num_cond; i++) {
        sys->msg("c%d%d=%g  ", i, cond, (*capmat)[i][cond]);
        if(i % 4 == 0) sys->msg("\n");
      }
      sys->msg("\n\n");
    }

    if (sys->itrdat && sys->rawdat) {
      sys->msg("%d iterations\n", iter);
    }

  }
  sys->flush();
  return(ttliter);
}


/* 
Preconditioned(possibly) Generalized Conjugate Residuals.
*/
static int gcr(ssystem *sys, double *q, double *p, double *r, double *ap, double **bp, double **bap, int size, int real_size, double *sqrmat, int *real_index, int maxiter, double tol, charge *chglist)
{
  int iter, i, j;
  double norm, beta, alpha, maxnorm;

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

    computePsi(sys, p, ap, size, real_size, sqrmat, real_index, chglist);
    
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
      sys->msg("max res = %g ||res|| = %g\n", maxnorm, sqrt(norm));
    } else {
      sys->msg("%d ", iter+1);
      if((iter+1) % 15 == 0) sys->msg("\n");
    }
    sys->flush();
    stoptimer;
    counters.conjtime += dtime;
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
    sys->msg("\ngcr: WARNING exiting without converging\n");
  }
  return(iter+1);
}


/* 
  Preconditioned(possibly) Generalized Minimum Residual. 
  */
static int gmres(ssystem *sys, double *q, double *p, double *r, double *ap, double **bv, double **bh, int size, int real_size, double *sqrmat, int *real_index, int maxiter, double tol, charge *chglist)
{
  int iter, i, j;
  double rnorm, norm;
  double hi, hip1, length;

  static Heap local_heap;
  static double *c=NULL, *s=NULL, *g=NULL, *y=NULL;
  static int alloc_size = 0;
  
  starttimer;

  /* Allocation or reallocation */
  if (size+1 > alloc_size) {
    alloc_size = size+1;
    c = local_heap.alloc<double>(size+1, AMSC);
    s = local_heap.alloc<double>(size+1, AMSC);
    g = local_heap.alloc<double>(size+1, AMSC);
    y = local_heap.alloc<double>(size+1, AMSC);
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
  counters.conjtime += dtime;

  if (sys->itrdat) {
    sys->msg("||res|| = %g\n", rnorm); /* initial guess residual norm */
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
    counters.conjtime += dtime;

    /* Form Av{iter}. */
    computePsi(sys, p, ap, size, real_size, sqrmat, real_index, chglist);

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
    /* assert(g[iter+1] == 0); WHY IS THIS HERE ??? */
    hi = g[iter];
    g[iter] = c[iter] * hi;
    g[iter+1] = s[iter] * hi;    
    
    rnorm = ABS(g[iter+1]);

    stoptimer;
    counters.conjtime += dtime;

    if (sys->itrdat) {
      sys->msg("||res|| = %g\n", rnorm);
    } else {
      sys->msg("%d ", iter);
      if((iter) % 15 == 0 && iter != 0) sys->msg("\n");
    }
    sys->flush();
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
  counters.conjtime += dtime;
  
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
    counters.prectime += dtime;
  }

  if(rnorm > tol) {
    sys->msg("\ngmres: WARNING exiting without converging\n");
  }
  return(iter);
}

/* 
ComputePsi computes the potential from the charge vector, or may
include a preconditioner.  It is assumed that the vectors for the
charge and potential have already been set up and that the potential
vector has been zeroed.  ARBITRARY VECTORS CAN NOT BE USED.
*/

static void computePsi(ssystem *sys, double *q, double *p, int size, int real_size, double *sqrmat, int *real_index, charge *chglist)
{
  int i;

  assert(p == sys->p);
  assert(q == sys->q);

  for(i=1; i <= size; i++) p[i] = 0;

  if (PRECOND != NONE) {
    starttimer;
    mulPrecond(sys, PRECOND);
    stoptimer;
    counters.prectime += dtime;
  }

  if (sys->expgcr) {

    blkCompressVector(sys, q+1, size, real_size, sys->is_dummy+1);
    blkAqprod(sys, p+1, q+1, real_size, sqrmat);        /* offset since index from 1 */
    blkExpandVector(p+1, size, real_size, real_index); /* ap changed to p, r chged to q */
    blkExpandVector(q+1, size, real_size, real_index); /*    7 Oct 91 */

  } else {

    starttimer;
    mulDirect(sys);
    stoptimer;
    counters.dirtime += dtime;

    starttimer;
    mulUp(sys);
    stoptimer;
    counters.uptime += dtime;

    if (sys->dupvec) {
      dumpLevOneUpVecs(sys);
    }

    if (DNTYPE == NOSHFT) {
      mulDown(sys);             /* do downward pass without local exp shifts */
    }

    if (DNTYPE == GRENGD) {
      mulDown(sys);             /* do hierarchical local shift dwnwd pass */
    }

    stoptimer;
    counters.downtime += dtime;

    starttimer;

    if (MULTI == ON) {
      mulEval(sys);             /* evaluate either locals or multis or both */
    }

    stoptimer;
    counters.evaltime += dtime;

    if (sys->dmpchg == DMPCHG_LAST) {
      sys->msg("\nPanel potentials divided by areas\n");
      dumpChgDen(sys, p, chglist);
      sys->msg("End panel potentials\n");
    }

    /* convert the voltage vec entries on dielectric i/f's into eps1E1-eps2E2 */
    compute_electric_fields(sys, chglist);

    if (OPCNT == ON) {
      printops(sys);
      //  TODO: remove?
      //  exit(0);
    }

  }
}
