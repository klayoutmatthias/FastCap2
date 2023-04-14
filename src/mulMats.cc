
#include "mulGlobal.h"
#include "mulMats.h"
#include "mulMulti.h"
#include "mulLocal.h"
#include "mulDisplay.h"
#include "direct.h"
#include "calcp.h"
#include "blkDirect.h"
#include "resusage.h"
#include "counters.h"

#include <cassert>

/*
MulMatDirect creates the matrices for the piece of the problem that is done
directly exactly.
*/
void mulMatDirect(ssystem *sys, double **trimat, double **sqrmat, int **real_index, int up_size, int eval_size)
  /* double *trimat, *sqrmat: flattened triangular, square matrices */
  /* int *real_index: for map btwn condensed/expanded vectors */
{
  cube *nextc, *nextnbr;
  int i, nummats, **temp = 0;

  /* First count the number of matrices to be done directly. */
  for(nextc=sys->directlist; nextc != NULL; nextc = nextc->dnext) {
    for(nummats=1, i=0; i < nextc->numnbrs; i++) {
      nextnbr = nextc->nbrs[i];
      assert(nextnbr->upnumvects > 0);
      nummats++;
    }

  /* Allocate space for the vects and mats. */
    nextc->directnumvects = nummats;
    if(nummats > 0) {
      nextc->directq = sys->heap.alloc<double *>(nummats, AMSC);
      temp = sys->heap.alloc<int *>(nummats, AMSC);
      nextc->directnumeles = sys->heap.alloc<int>(nummats, AMSC);
      nextc->directmats = sys->heap.alloc<double**>(nummats, AMSC);
      nextc->precondmats = sys->heap.alloc<double**>(nummats, AMSC);
    }

    /* initialize the pointer from this cube to its part of dummy vector
       - save the self part found in indexkid() */
    temp[0] = nextc->nbr_is_dummy[0];
    nextc->nbr_is_dummy = temp;
  }

/* Now place in the matrices. */
  for(nextc=sys->directlist; nextc != NULL; nextc = nextc->dnext) {
    nextc->directq[0] = nextc->upvects[0];
    nextc->directnumeles[0] = nextc->upnumeles[0];

    starttimer;
    if (sys->dirsol || sys->expgcr) {
      if(nextc == sys->directlist) {
        if(eval_size < MAXSIZ) {
          sys->error("mulMatDirect: non-block direct methods not supported");
        }
        else blkQ2Pfull(sys, sys->directlist, up_size, eval_size,
                        trimat, sqrmat, real_index, sys->is_dummy);
      }
      else nextc->directmats[0]
          = Q2PDiag(sys, nextc->chgs, nextc->upnumeles[0], nextc->nbr_is_dummy[0],
                    TRUE);
    } else {
      nextc->directmats[0]
          = Q2PDiag(sys, nextc->chgs, nextc->upnumeles[0], nextc->nbr_is_dummy[0],
                    TRUE);
      nextc->precondmats[0]
          = Q2PDiag(sys, nextc->chgs, nextc->upnumeles[0], nextc->nbr_is_dummy[0],
                    FALSE);
    }

    stoptimer;
    counters.dirtime += dtime;

    if (sys->dsq2pd) {
      dumpQ2PDiag(sys, nextc);
    }

    if (sys->dmtcnt) {
      sys->mm.Q2PDcnt[nextc->level][nextc->level]++;
    }

    if (sys->dirsol) {
      /* transform A into LU */
      if(eval_size > MAXSIZ) {
        blkLUdecomp(sys, *sqrmat, *trimat, up_size);
      }
      else if(nextc == sys->directlist) {
        starttimer;
        nextc->directlu = ludecomp(sys, nextc->directmats[0], eval_size, TRUE);
        stoptimer;
        counters.lutime += dtime;
      }
    }
    
    starttimer;
    for(nummats=1, i=0; i < nextc->numnbrs; i++) {
      nextnbr = nextc->nbrs[i];
      assert(nextnbr->upnumvects > 0);
      nextc->directq[nummats] = nextnbr->upvects[0];
      nextc->nbr_is_dummy[nummats] = nextnbr->nbr_is_dummy[0];
      nextc->directnumeles[nummats] = nextnbr->upnumeles[0];
      nextc->directmats[nummats] = Q2P(sys,
                                       nextnbr->chgs,
                                       nextnbr->upnumeles[0], 
                                       nextnbr->nbr_is_dummy[0],
                                       nextc->chgs, nextc->upnumeles[0],
                                       TRUE);
      nextc->precondmats[nummats++] = Q2P(sys,
                                          nextnbr->chgs,
                                          nextnbr->upnumeles[0], 
                                          nextnbr->nbr_is_dummy[0],
                                          nextc->chgs, nextc->upnumeles[0],
                                          FALSE);
      if (sys->dmtcnt) {
        sys->mm.Q2Pcnt[nextc->level][nextnbr->level]++;
      }
    }
    stoptimer;
    counters.dirtime += dtime;
  }
}


/*
MulMatPrecond creates the preconditioner matrix
*/
void bdmulMatPrecond(ssystem *sys)
{
  cube *nc, *kid, *kidnbr;
  double **mat, **nbrmat;
  int i, j, k, l, kidi;
  int kidsize, nbrsize, size, row, col, first;

  for(nc=sys->precondlist; nc != NULL; nc = nc->pnext) {

    /* find total number of charges in cube to dimension P. */
    for(size=0, i=0; i < nc->numkids; i++) {
      kid = nc->kids[i];
      if(kid != NULL) {
        assert(kid->level == sys->depth);
        size += kid->directnumeles[0];  /* Equals number of charges. */
      }
    }

    /* allocate and zero a preconditioner matrix. */
    mat = sys->heap.alloc<double *>(size, AMSC);
    for(i = 0; i < size; i++) {
      mat[i] = sys->heap.alloc<double>(size, AMSC);
    }
    for(i = 0; i < size; i++) {
      for(j = 0; j < size; j++) {
        mat[i][j] = 0.0;
      }
    }

    /* Chase through the kids to place in potential coeffs. */
    for(first = TRUE, row=0, kidi=0; kidi < nc->numkids; kidi++) {
      kid = nc->kids[kidi];
      if(kid != NULL) {
        /* Exploit the hierarchical charge numbering to get precond vector. */
        if(first == TRUE) {
          first = FALSE;
          nc->prevectq = kid->directq[0];
          nc->prevectp = kid->eval;
        }
        /* Get the diagonal block of P^{-1}. */
        kidsize = kid->directnumeles[0];
        for(k = kidsize - 1; k >= 0; k--) {
          for(l = kidsize - 1; l >= 0; l--) {
            mat[row + k][row + l] = kid->directmats[0][k][l];
          }
        }
        /* Get the off-diagonals of P^{-1}. */
        for(col = 0, i = 0; i < nc->numkids; i++) {
          kidnbr = nc->kids[i];
          if(kidnbr != NULL) {
            if(kidnbr != kid) {
              /* Chase thru list of nbrs to get matrix associated with this 
                 kidnbr.  Note, this is because the kid list and nbr matrix
                 list are in different orders, could be fixed. */
              for(j = kid->numnbrs-1; j >= 0; j--) {
                if(kidnbr == kid->nbrs[j]) {
                  nbrmat = kid->directmats[j+1];
                  nbrsize = kidnbr->directnumeles[0];
                  for(k = kidsize - 1; k >= 0; k--) {
                    for(l = nbrsize - 1; l >= 0; l--) {
                      mat[row + k][col + l] = nbrmat[k][l];
                    }
                  }
                  break;
                }
              }
            }
            col += kidnbr->directnumeles[0];
          }
        }
        assert(col == size);
        row += kidsize;
      }
    }    
    assert(row == size);

    nc->precond = ludecomp(sys, mat, size, FALSE);
    nc->presize = size;
  }
}
      

/* This near picks up only the hamming distance one cubes. */    
#define HNEAR(nbr, nj, nk, nl) \
((ABS((nbr)->j - (nj)) + ABS((nbr)->k - (nk)) + ABS((nbr)->l - (nl))) <= 1)

/* This near picks up all 27 neighboring cubes. */
#define NEAR(nbr, nj, nk, nl) \
((ABS((nbr)->j - (nj)) <= 1) && \
 (ABS((nbr)->k - (nk)) <= 1) && \
 (ABS((nbr)->l - (nl)) <= 1))

/* This near picks only the diagonal, for testing. */
#define DNEAR(nbr, nj, nk, nl) \
(((nbr)->j == (nj)) && \
 ((nbr)->k == (nk)) && \
 ((nbr)->l == (nl)) )
        
void olmulMatPrecond(ssystem *sys)
{
  cube *nc, *nnbr, *nnnbr;
  double **mat, **nmat;
  int i, j, k, l, m;
  int maxsize, nsize, nnsize, nnnsize;
  int nj, nk, nl, offset, noffset;
  int *nc_dummy, *nnbr_dummy, *nnnbr_dummy;
  static Heap local_heap;
  static int *is_dummy;         /* local dummy flag vector, stays around */
  static int big_mat_size = 0;  /* size of previous mat */
  charge **nnnbr_pc, **nnbr_pc, **nc_pc;

/* Figure out the max number of elements in any set of near cubes. */
  for(maxsize=0, nc=sys->directlist; nc != NULL; nc = nc->dnext) {
    nsize = nc->directnumeles[0];
    nj = nc->j;
    nk = nc->k;
    nl = nc->l;
    for(i=0; i < nc->numnbrs; i++) {
      nnbr = nc->nbrs[i];
      if(NEAR(nnbr, nj, nk, nl)) nsize += nnbr->directnumeles[0];
    }
    maxsize = MAX(nsize, maxsize);
  }

  /* Allocate a matrix big enough for any set of 7. */
  if (sys->jacdbg) {
    sys->msg("max direct size =%d\n", maxsize);
  }
  mat = sys->heap.alloc<double*>(maxsize, AMSC);
  for(i=0; i < maxsize; i++) {
    mat[i] = sys->heap.alloc<double>(maxsize, AMSC);
  }

  /* Now go fill-in a matrix. */
  for(maxsize=0, nc=sys->directlist; nc != NULL; nc = nc->dnext) {
    nsize = nc->directnumeles[0];
    nc_dummy = nc->nbr_is_dummy[0];
    nc_pc = nc->chgs;
    if (sys->chkdum) {
      chkDummyList(sys, nc_pc, nc_dummy, nsize);
    }
    nj = nc->j;
    nk = nc->k;
    nl = nc->l;
    for(i = nsize - 1; i >= 0; i--) {
      if(nc_dummy[i]) continue; /* dummy rows copied only in divided diff */
      if(nc_pc[i]->surf->type != DIELEC) {
        for(j = nsize - 1; j >= 0; j--) {
          mat[i][j] = nc->directmats[0][i][j];
        }
      }
      else {
        if (sys->dpcomp) {
          sys->msg("Source mat, nc to nc\n");
          dumpMat(sys, nc->directmats[0], nsize, nsize);
        }
        find_flux_density_row(sys, mat, nc->directmats[0], i, nsize, nsize, 0, 0,
                              nc_pc, nc_pc, nc_dummy, nc_dummy);
      } 
    }
    offset = nsize;
    for(k=0; k < nc->numnbrs; k++) { /* loop on neighbors of nc */
      nnbr = nc->nbrs[k];
      if(NEAR(nnbr, nj, nk, nl)) {
        nnsize = nc->directnumeles[k+1];
        nmat = nc->directmats[k+1];
        assert(nc->directnumeles[k+1] == nnbr->directnumeles[0]);
        nnbr_dummy = nnbr->nbr_is_dummy[0];
        nnbr_pc = nnbr->chgs;
        if (sys->chkdum) {
          chkDummyList(sys, nnbr_pc, nnbr_dummy, nnsize);
        }
        for(i = nsize - 1; i >= 0; i--) {
          if(nc_dummy[i]) continue;
          if(nc_pc[i]->surf->type != DIELEC) {
            for(j = nnsize - 1; j >= 0; j--) {
              mat[i][offset + j] = nmat[i][j];
            }
          }
          else {
            if (sys->dpcomp) {
              sys->msg("Source mat, nnbr to nc\n");
              dumpMat(sys, nmat, nsize, nnsize);
            }
            find_flux_density_row(sys, mat, nmat, i, nnsize, nsize, 0, offset,
                                  nc_pc, nnbr_pc, nc_dummy, nnbr_dummy);
          }
        }
        /* Get the row of the big matrix associated with this nnbr. */
        for(noffset = 0, l = -1; l < nc->numnbrs; l++) { /* lp on nc's nbrs */
          if(l < 0) nnnbr = nc;
          else nnnbr = nc->nbrs[l];
          if(NEAR(nnnbr, nj, nk, nl)) {  /* Note, near to nc!! */
            if(nnbr == nnnbr) m = -1;
            else { /* Find this nnnbr's position in nnbr's list */
              for(m=0; m < nnbr->numnbrs; m++) {
                if(nnbr->nbrs[m] == nnnbr) break;
              }
              assert(m < nnbr->numnbrs);
            }
            nnnsize = nnbr->directnumeles[m+1];
            nmat = nnbr->directmats[m+1];
            assert(nnbr->directnumeles[m+1] == nnnbr->directnumeles[0]);
            nnnbr_pc = nnnbr->chgs; /* panels in nnnbr */
            nnnbr_dummy = nnnbr->nbr_is_dummy[0];
            if (sys->chkdum) {
              chkDummyList(sys, nnnbr_pc, nnnbr_dummy, nnnsize);
            }
            for(i = nnsize - 1; i >= 0; i--) { /* loop on panels in nnbr */
              if(nnbr_dummy[i]) continue;
              if(nnbr_pc[i]->surf->type != DIELEC) {
                for(j = nnnsize - 1; j >= 0; j--) {
                  mat[offset + i][noffset+j] = nmat[i][j];
                }
              }
              else {
                if (sys->dpcomp) {
                  sys->msg("Source mat, nnnbr to nnbr\n");
                  dumpMat(sys, nmat, nnsize, nnnsize);
                }
                find_flux_density_row(sys, mat, nmat, i, nnnsize, nnsize, offset,
                                      noffset, nnbr_pc, nnnbr_pc, nnbr_dummy,
                                      nnnbr_dummy);
              }
            }
            noffset += nnnsize;
          }
        }
        offset += nnsize;
      }
    }

    /* set up the local is_dummy vector for the rows/cols of mat */
    /* THIS COULD BE AVOIDED BY USING CUBE is_dummy's INSIDE invert() */
    if(big_mat_size < offset) { /* allocate only if larger array needed */
      is_dummy = local_heap.alloc<int>(offset, AMSC);
    }
    /* dump sections of the dummy vector in order cubes appear in nbr lst */
    /* (use fragment of Jacob's loop above) */
    nnnsize = noffset = nc->directnumeles[0];
    nc_dummy = nc->nbr_is_dummy[0];
    for(i = nnnsize - 1; i >= 0; i--) {
      is_dummy[i] = nc_dummy[i];
    }
    for(l = 0; l < nc->numnbrs; l++) {
      nnnbr = nc->nbrs[l];
      if(NEAR(nnnbr, nj, nk, nl)) {
        nnnsize = nnnbr->directnumeles[0];
        nc_dummy = nnnbr->nbr_is_dummy[0];
        for(i = nnnsize - 1; i >= 0; i--) {
          is_dummy[i + noffset] = nc_dummy[i];
        }
        noffset += nnnsize;
      }
    }

    /* The big Matrix is filled in, invert it and get the preconditioner. */
    if (sys->dpcomp) {
      sys->msg("Before compression\n");
      dumpMat(sys, mat, offset, offset);
    }
    nnnsize = compressMat(sys, mat, offset, is_dummy, BOTH);
    if (sys->dpcomp) {
      sys->msg("After compression\n");
      dumpMat(sys, mat, nnnsize, nnnsize);
    }
    invert(mat, nnnsize, NULL);
    expandMat(mat, offset, nnnsize, is_dummy, BOTH);
    if (sys->dpcomp) {
      sys->msg("After expansion\n");
      dumpMat(sys, mat, offset, offset);
    }

    /* Copy out the preconditioner to the saved matrices. */
    for(i = nsize - 1; i >= 0; i--) {
      for(j = nsize - 1; j >= 0; j--) {
        nc->precondmats[0][i][j] = mat[i][j];
      }
    }
    offset = nsize;
    for(k=0; k < nc->numnbrs; k++) {
      nnbr = nc->nbrs[k];
      if(NEAR(nnbr, nj, nk, nl)) {
        nnsize = nc->directnumeles[k+1];
        nmat = nc->precondmats[k+1];
        for(i = nsize - 1; i >= 0; i--) {
          for(j = nnsize - 1; j >= 0; j--) {
            nmat[i][j] = mat[i][offset + j];
          }
        }
        offset += nnsize;
      }
      else nc->precondmats[k+1] = NULL;
    }
  }
}

/*
  finds a row of flux density coeffs from three potential coeff rows
  - to_mat[eval_row][] is the destination row; from_mat[eval_row][]
    initially contains the potential coefficients for evals at the 
    center of eval_panels[eval_row] (unless NUMDPT == 2, is garbage then)
  - the eval panels are scaned until eval_panels[eval_row]'s
    dummies are found and the corresponding two rows are identified
  - the divided differences built with entries in the same columns in
    these three rows replace the to_mat[eval_row][] entries:
    to_mat[eval_row][j] = a1*from_mat[eval_row][j]
              + a2*from_mat[pos_dum_row][j] + a3*from_mat[neg_dum_row][j]
  - if a dummy panel is not found in the panel list, its row is generated
    using explicit calcp() calls (shouldn't happen much)
  - global flags used here
    NUMDPT = number of divided diff points, 2 or 3
    SKIPDQ = ON=>don't do cancellation-prone add-subtract of identical
      influence of DIELEC/BOTH panels' charges on dummy panel pot. evals
*/
void find_flux_density_row(ssystem *sys, double **to_mat, double **from_mat, int eval_row, int n_chg, int n_eval, int row_offset,
                      int col_offset, charge **eval_panels, charge **chg_panels, int *eval_is_dummy, 
                      int *chg_is_dummy)
{
  int dindex, j;
  double factor;
  charge *dp;
  Surface *surf = eval_panels[eval_row]->surf;

  /* do divided difference w/ three rows to get dielectric row */
  if (NUMDPT == 3) {
    /* - dielectric panel row first */
    factor = -(surf->outer_perm + surf->inner_perm)/
        (eval_panels[eval_row]->pos_dummy->area);
    if (sys->dpddif) {
      sys->msg("Center row, factor = %g\n", factor);
    }
    for(j = n_chg - 1; j >= 0; j--) { /* loop on columns */
      if(!chg_is_dummy[j])
          to_mat[row_offset + eval_row][col_offset + j]
              = from_mat[eval_row][j]*factor;
      if (sys->dpddif) {
        sys->msg(" %.16e", from_mat[eval_row][j]);
      }
    }
  }                          /* if (NUMDPT == 3) */
  /* - do positive dummy row */
  /*   first find the dummy row */
  dindex = -1;
  dp = eval_panels[eval_row]->pos_dummy; /* get dummy panel from eval panel */
  for(j = n_eval - 1; j >= 0; j--) {
    if(!eval_is_dummy[j]) continue;
    if(dp == eval_panels[j]) {
      dindex = j;
      break;
    }
  }
  if(dindex != -1) { /* dummy row found */
    if (NUMDPT == 3) {
      factor = surf->outer_perm/eval_panels[dindex]->area;
    } else {
      /* this is the only factor required for two dummy rows in two point case */
      factor = (surf->inner_perm - surf->outer_perm)
          /(eval_panels[eval_row]->neg_dummy->area
            + eval_panels[eval_row]->pos_dummy->area);
    }
    if (sys->dpddif) {
      sys->msg("\nPos dummy row, factor = %g\n", factor);
    }
    for(j = n_chg - 1; j >= 0; j--) {
      if (SKIPQD == ON) {
        if(chg_panels[j]->index == eval_panels[eval_row]->index) {
          to_mat[row_offset + eval_row][col_offset + j] = 0.0;
          continue;
        }
      }
      if(!chg_is_dummy[j]) {
        if (NUMDPT == 3) {
          to_mat[row_offset + eval_row][col_offset + j]
              += from_mat[dindex][j]*factor;
        } else {                      /* make sure to overwrite possible garbage */
          to_mat[row_offset + eval_row][col_offset + j]
              = -from_mat[dindex][j]*factor;
        }
      }
      if (sys->dpddif) {
        sys->msg(" %.16e (%d)", from_mat[dindex][j],chg_panels[j]->index);
      }
    }
  }
  else {                /* dummy row out of cube => build it w/calcp */
    if (NUMDPT == 3) {
      factor = surf->outer_perm/dp->area;
    } else {
      /* this is the only factor required for two dummy rows in two point case */
      factor = (surf->inner_perm - surf->outer_perm)
          /(eval_panels[eval_row]->neg_dummy->area
            + eval_panels[eval_row]->pos_dummy->area);
    }
    if (sys->dpddif) {
      sys->msg("\nPos dummy calcp row, factor = %g\n", factor);
    } else {
      sys->info("\nolmulMatPrecond: building pos. dummy row\n");
    }
    for(j = n_chg - 1; j >= 0; j--) {
      if (SKIPQD == ON) {
        if(chg_panels[j]->index == eval_panels[eval_row]->index) {
          to_mat[row_offset + eval_row][col_offset + j] = 0.0;
          continue;
        }
      }
      if(!chg_is_dummy[j]) {
        if (NUMDPT == 3) {
          to_mat[row_offset + eval_row][col_offset + j]
              += calcp(sys, chg_panels[j], dp->x, dp->y, dp->z, NULL)*factor;
        } else {
          to_mat[row_offset + eval_row][col_offset + j]
              = -calcp(sys, chg_panels[j], dp->x, dp->y, dp->z, NULL)*factor;
        }
        if (sys->dpddif) {
          sys->msg(" %.16e (%d)",
                  calcp(sys, chg_panels[j], dp->x, dp->y, dp->z, NULL),
                  chg_panels[j]->index);
        }
      }
      else {
        if (sys->dpddif) {
          sys->msg(" dummy");
        }
      }
    }
  }
  /* - do negative dummy row */
  /*   first find the dummy row */
  dindex = -1;
  dp = eval_panels[eval_row]->neg_dummy; /* get dummy panel from eval panel */
  for(j = n_eval - 1; j >= 0; j--) {
    if(!eval_is_dummy[j]) continue;
    if(dp == eval_panels[j]) {
      dindex = j;
      break;
    }
  }
  if(dindex != -1) { /* dummy row found */
    if (NUMDPT == 3) {
      factor = surf->inner_perm/eval_panels[dindex]->area;
    }
    if (sys->dpddif) {
      sys->msg("\nNeg dummy row, factor = %g\n", factor);
    }
    for(j = n_chg - 1; j >= 0; j--) {
      if (SKIPQD == ON) {
        if(chg_panels[j]->index == eval_panels[eval_row]->index) continue;
      }
      if(!chg_is_dummy[j])
          to_mat[row_offset + eval_row][col_offset + j] 
              += from_mat[dindex][j]*factor;
      if (sys->dpddif) {
        sys->msg(" %.16e (%d)", from_mat[dindex][j],chg_panels[j]->index);
      }
    }
  }
  else {                /* dummy row out of cube => build it w/calcp */
    factor = surf->inner_perm/dp->area;
    if (sys->dpddif) {
      sys->msg("\nNeg dummy calcp row, factor = %g\n", factor);
    } else {
      sys->info("olmulMatPrecond: building neg. dummy row\n");
    }
    for(j = n_chg - 1; j >= 0; j--) {
      if (SKIPQD == ON) {
        if(chg_panels[j]->index == eval_panels[eval_row]->index) continue;
      }
      if(!chg_is_dummy[j]) {
        to_mat[row_offset + eval_row][col_offset + j] 
            += calcp(sys, chg_panels[j], dp->x, dp->y, dp->z, NULL)*factor;
        if (sys->dpddif) {
          sys->msg(" %.16e (%d)",
                  calcp(sys, chg_panels[j], dp->x, dp->y, dp->z, NULL),
                  chg_panels[j]->index);
        }
      }
      else {
        if (sys->dpddif) {
          sys->msg(" dummy");
        }
      }
    }
  }
  if (NUMDPT == 2) {
    /* - do row entry due to panel contribution
       - entry only necessary if eval panel is in chg panel list */
    /*   search for the eval panel in the charge panel list */
    dp = NULL;
    for(j = n_chg - 1; j >= 0; j--) {
      if(!chg_is_dummy[j]) {
        if(eval_panels[eval_row] == chg_panels[j]) {
          dp = eval_panels[eval_row];
          break;
        }
      }
    }
    /*   set entry if eval panel found in chg panel list
         - this is an overwrite; contributions of other rows should cancel */
    if(dp != NULL) {
      to_mat[row_offset + eval_row][col_offset + j]
          = -(2*M_PI*(surf->inner_perm + surf->outer_perm)
              /eval_panels[eval_row]->area);
    }
  }

  if (sys->dpddif) {
    sys->msg("\nDivided difference row (%d)\n",
            eval_panels[eval_row]->index);
    for(j = n_chg - 1; j >= 0; j--) {
      sys->msg(" %.16e (%d)",
              to_mat[row_offset + eval_row][col_offset + j],
              chg_panels[j]->index);
    }
    sys->msg("\n\n");
  }
}


/* 
MulMatUp computes the multipole to multipole or charge to
multipole matrices that map to a parent's multipole coeffs from its
children's multipoles or charges. Note that only one set of
multipole to multipole matrices is computed per level by exploiting the
uniform break-up of three-space (ie many shifts have similar geometries).  
*/
void mulMatUp(ssystem *sys) 
{
  cube *nextc, *kid;
  int i, j, numterms, depth, order = sys->order;
  double **multimats[8];

  numterms = multerms(order);

  if(sys->depth < 2) {
    /* sys->msg("\nWarning: no multipole acceleration\n");*/
    return;     /* return if upward pass not possible */
  }

  /* Handle the lowest level cubes first (set up Q2M's). */
  for(nextc=sys->multilist[sys->depth]; nextc != NULL; nextc = nextc->mnext) {
    nextc->multisize = numterms;
    nextc->multi = sys->heap.alloc<double>(numterms, AMSC);
    nextc->upmats = sys->heap.alloc<double **>(1, AMSC);
    nextc->upmats[0] = mulQ2Multi(sys,
                                  nextc->chgs, nextc->nbr_is_dummy[0],
                                  nextc->upnumeles[0],
                                  nextc->x, nextc->y, nextc->z, order);

    if (sys->dissyn) {
      sys->mm.multicnt[nextc->level]++;
    }

    if (sys->dmtcnt) {
      sys->mm.Q2Mcnt[nextc->level][nextc->level]++;
    }

  }

  if(sys->locallist[sys->depth] == NULL
     && sys->multilist[sys->depth] == NULL) {
    sys->msg("No expansions at level %d (lowest)\n", sys->depth);
    /*if(sys->depth < 3) 
        sys->msg(" (Warning: no multipole acceleration)\n");*/
  }
  else if(sys->locallist[sys->depth] == NULL) {
    sys->msg("No local expansions at level %d (lowest)\n", sys->depth);
  }
  else if(sys->multilist[sys->depth] == NULL) {
    sys->msg("No multipole expansions at level %d (lowest)\n", 
            sys->depth); 
    /*if(sys->depth < 3) 
        sys->msg(" (Warning: no multipole acceleration)\n");*/
  }

  /* Allocate the vectors and matrices for the cubes. */
  /* no multipoles over root cube or its kids (would not be used if made) */
  for(depth = (sys->depth - 1); depth > 1; depth--) {

    /* set up M2M's and Q2M's to compute the multipoles needed for this level */
    if(sys->locallist[depth] == NULL && sys->multilist[depth] == NULL) {
      sys->msg("No expansions at level %d\n", depth);
      /*if(depth < 3) sys->msg(" (Warning: no multipole acceleration)\n");
      else sys->msg("\n");*/
    }
    else if(sys->locallist[depth] == NULL) {
      sys->msg("No local expansions at level %d\n", depth);
    }
    else if(sys->multilist[depth] == NULL) {
      sys->msg("No multipole expansions at level %d\n", depth); 
      /*if(depth < 3) sys->msg(" (Warning: no multipole acceleration)\n");
      else sys->msg("\n");*/
    }

    /* NULL out pointers to same-geometry M2M mats for this level */
    for (i = 0; i < int(sizeof(multimats) / sizeof(multimats[0])); i++) {
      multimats[i] = NULL;
    }

    /* Hit nonempty cubes at this level assigning ptrs to precomputed   */
    /* M2M mats (for this lev), or if a kid is exact, computing Q2M matrices. */
    for(nextc=sys->multilist[depth]; nextc != NULL; nextc = nextc->mnext) {
      
      if (sys->dissyn) {
        sys->mm.multicnt[nextc->level]++;
      }

      /* Save space for upvector sizes, upvect ptrs, and upmats. */
      nextc->multisize = numterms;
      if(numterms > 0) {
        nextc->multi = sys->heap.alloc<double>(numterms, AMSC);
      }
      if(nextc->upnumvects) {
        nextc->upnumeles = sys->heap.alloc<int>(nextc->upnumvects, AMSC);
        nextc->upvects = sys->heap.alloc<double*>(nextc->upnumvects, AMSC);
        nextc->upmats = sys->heap.alloc<double**>(nextc->upnumvects, AMSC);
      }

      /* Go through nonempty kids and fill in upvectors and upmats. */
      for(i=0, j=0; j < nextc->numkids; j++) {
        if((kid = nextc->kids[j]) != NULL) { /* NULL => empty kid cube */
          if(kid->mul_exact == FALSE) { /* if kid has a multi */
            nextc->upvects[i] = kid->multi;
            nextc->upnumeles[i] = kid->multisize;
            if(multimats[j] == NULL) { /* Build the needed matrix only once. */
              multimats[j] = mulMulti2Multi(sys,
                                            kid->x, kid->y, kid->z, nextc->x,
                                            nextc->y, nextc->z, order);
            }
            nextc->upmats[i] = multimats[j];

            if (sys->dmtcnt) {
              sys->mm.M2Mcnt[kid->level][nextc->level]++; /* cnts usage, ~computation */
            }

          }
          else {                /* if kid is exact, has no multi */
            nextc->upvects[i] = kid->upvects[0];
            nextc->upnumeles[i] = kid->upnumeles[0];
            nextc->upmats[i] = mulQ2Multi(sys,
                                          kid->chgs, kid->nbr_is_dummy[0],
                                          kid->upnumeles[0],
                                          nextc->x, nextc->y, nextc->z, order);

            if (sys->dmtcnt) {
              sys->mm.Q2Mcnt[kid->level][nextc->level]++;
            }

          }
          i++;                  /* only increments if kid is not empty */
        }
      }
    }
  }
}

/*
  builds the transformation matrices for the final evaluation pass (M2P, L2P)
  for all the direct list (generated by linkcubes(), non-empty
  lowest level cubes) cubes:

  for each cube A in the direct list:
  1) if the cube is not exact (always the case if ADAPT = OFF)
     a) and if DNTYPE = GRENGD build an L2P matrix from A to A 
     b) and if DNTYPE = NOSHFT build an L2P matrix from each of A's ancestors
        with level > 1 (including A) to A
     c) and if DNTYPE = NOLOCL build an M2P matrix from each of A's fake 
        ilist entries to A (same action as 2b)
  2) if the cube is exact, find the 1st ancestor of A, cube B, 
     which either is not exact and is at level 2,3,4... or is at level 1
     a) if B is at level 2,3,4... 
        i) if DNTYPE = GRENGD, construct an L2P from B to A and M2P's
           from the cubes in the true interaction lists of A and all its
           ancestors up to and including B (a partial fake interaction list)
        j) if DNTYPE = NOSHFT, find cube C, the ancestor of B at level 1;
           construct L2P's from the ancestors of B (including B but not C)
           to A and Q- or M2P's from the cubes in the true interaction lists 
           of A and all its ancestors up to and including B (a partial fake 
           interaction list)
        k) if DNTYPE = NOLOCL, do 2b
     b) if B is at level 1 construct M2P's for all the cubes in A's
        fake interaction list

  true interaction list - RADINTER = OFF, those sibling
  (same level) cubes of a given cube who are children of the neighbors
  of the given cube's parent and are not neighbors of the given cube 
  - ie those cubes required to cover charges well separated from the given
  cube but not accounted for in the parent's local expansion 
  - the flag NNBRS is the number of sibling cube "shells" taken as neighbors 
  
  fake interaction list - RADINTER = OFF, the combined true interaction lists
  of a given cube and all its ancestors at levels 2,3,4...

  if RADINTER = ON, any 8 siblings of the given cube which form a well 
  separated cube one level up are included in the lists as a single higher
  level cube
  
  if ADAPT = OFF, no cube is exact so step 2 is never done

  this routine is used alone if compiled with DNTYPE = NOLOCL or after
  mulMatDown, which produces M2L and L2L matrices (DNTYPE = GRENGD) or
  just M2L matrices (DNTYPE = NOSHFT) --  DNTYPE = GRENGD does the full
  Greengard hiearchical downward pass

*/
void mulMatEval(ssystem *sys)
{
  int i, j, ttlvects;
  cube *na, *nc, *nexti;

  if(sys->depth < 2) return;    /* ret if upward pass not possible/worth it */

  for(nc = sys->directlist; nc != NULL; nc = nc->dnext) {

    assert(nc->level == sys->depth);
    assert(nc->upnumvects > 0);

    /* allocate space for evaluation pass vectors; check nc's ancestors */
    /* First count the number of transformations to do. */
    for(na = nc, ttlvects = 0; na->level > 1; na = na->parent) { 
      if(na->loc_exact == FALSE && DNTYPE != NOLOCL) {
        ttlvects++;  /* allow for na to na local expansion (L2P) */
        if(DNTYPE == GRENGD) break; /* Only one local expansion if shifting. */
      }
      else {
        ttlvects += na->interSize; /* room for Q2P and M2P xformations */
      }
    }
    nc->evalnumvects = ttlvects; /* save ttl # of transformations to do */
    if(ttlvects > 0) {
      nc->evalvects = sys->heap.alloc<double*>(ttlvects, AMSC);
      nc->evalnumeles = sys->heap.alloc<int>(ttlvects, AMSC);
      nc->evalmats = sys->heap.alloc<double**>(ttlvects, AMSC);
    }
    
    if (sys->dilist) {
      sys->msg("\nInteraction list (%d entries) for ", ttlvects);
      disExParsimpcube(sys, nc);
    }
    
    /* set up exp/charge vectors and L2P, Q2P and/or M2P matrices as req'd */
    for(j=0, na = nc, ttlvects = 0; na->level > 1; na = na->parent) { 
      if(na->loc_exact == FALSE && DNTYPE != NOLOCL) {  
        /* build matrices for local expansion evaluation */
        nc->evalmats[j] = mulLocal2P(sys, na->x, na->y, na->z, nc->chgs,
                                     nc->upnumeles[0], sys->order);
        nc->evalnumeles[j] = na->localsize;
        nc->evalvects[j] = na->local;
        j++; 
        
        if (sys->dmtcnt) {
          sys->mm.L2Pcnt[na->level][nc->level]++;
        }

        if (sys->dilist) {
          sys->msg("L2P: ");
          disExtrasimpcube(sys, na);
        }

        if(DNTYPE == GRENGD) break; /* Only one local expansion if shifting. */
      }
      else { /* build matrices for ancestor's (or cube's if 1st time) ilist */
        for(i=0; i < na->interSize; i++) {
          nexti = na->interList[i];
          if(nexti->mul_exact == TRUE) {
            nc->evalvects[j] = nexti->upvects[0];
            nc->evalmats[j] = Q2P(sys, nexti->chgs, nexti->upnumeles[0],
                                  nexti->nbr_is_dummy[0], nc->chgs, 
                                  nc->upnumeles[0], TRUE);
            nc->evalnumeles[j] = nexti->upnumeles[0];
            j++;

            if (sys->dmtcnt) {
              sys->mm.Q2Pcnt[nexti->level][nc->level]++;
            }

            if (sys->dilist) {
              sys->msg("Q2P: ");
              disExtrasimpcube(sys, nexti);
            }
          }
          else {
            nc->evalvects[j] = nexti->multi;
            nc->evalmats[j] = mulMulti2P(sys,
                                         nexti->x, nexti->y, nexti->z,
                                         nc->chgs, nc->upnumeles[0], 
                                         sys->order);
            nc->evalnumeles[j] = nexti->multisize;
            j++;
            
            if (sys->dmtcnt) {
              sys->mm.M2Pcnt[nexti->level][nc->level]++;
            }

            if (sys->dilist) {
              sys->msg("M2P: ");
              disExtrasimpcube(sys, nexti);
            }
          }
        }
      }
    }
  }
}


/* 
  sets up matrices for the downward pass
  For each cube in local list (parents always in list before kids):
  1) parent's local to child's local unless DNTYPE = NOSHFT or no parent local
  2) multipoles for (Parent+parent's nbrs - child nbrs) to child's local
  -eval is sum of ancestral local evals for each lowest lev cube if NOSHFT
    otherwise only lowest level local is evaluated (see mulMatEval)
  -with ADAPT = OFF no cube is exact so local list is all non-empty cube lev>1
  -mats that give potentials (M2P, L2P, Q2P) are calculated in mulMatEval()
  -this routine makes only L2L, M2L and Q2L matrices
*/
void mulMatDown(ssystem *sys)
{
  int i, j, vects;
  cube *nc, *parent, *ni;
  int depth;

  assert(DNTYPE != NOLOCL);     /* use mulMatEval() alone if NOLOCL */

  for(depth = 2; depth <= sys->depth; depth++) { /* no locals before level 2 */
    for(nc=sys->locallist[depth]; nc != NULL; nc = nc->lnext) {

      /* Allocate for interaction list, include one for parent if needed */
      if((depth <= 2) || (DNTYPE == NOSHFT)) vects = nc->interSize;
      else vects = nc->interSize + 1;
      nc->downnumvects = vects;
      if(vects > 0) {
        nc->downvects = sys->heap.alloc<double*>(vects, AMSC);
        nc->downnumeles = sys->heap.alloc<int>(vects, AMSC);
        nc->downmats = sys->heap.alloc<double**>(vects, AMSC);
      }

      parent = nc->parent;
      assert(parent->loc_exact == FALSE); /* has >= #evals of any of its kids*/

      if (sys->dissyn) {
        sys->mm.localcnt[nc->level]++;
      }

      if((depth <= 2) || (DNTYPE == NOSHFT)) i = 0; /* No parent local. */
      else { /* Create the mapping matrix for the parent to kid. */
        i = 1;

        nc->downmats[0] = mulLocal2Local(sys, parent->x, parent->y, parent->z,
                                         nc->x, nc->y, nc->z, sys->order);
        nc->downnumeles[0] = parent->localsize;
        nc->downvects[0] = parent->local;

        if (sys->dmtcnt) {
          sys->mm.L2Lcnt[parent->level][nc->level]++;
        }
      }

      /* Go through the interaction list and create mapping matrices. */
      for(j = 0; j < nc->interSize; j++, i++) {
        ni = nc->interList[j];
        if(ni->mul_exact == TRUE) {     /* ex->ex (Q2P) xforms in mulMatEval */
          nc->downvects[i] = ni->upvects[0];
          nc->downmats[i] = mulQ2Local(sys, ni->chgs, ni->upnumeles[0],
                                       ni->nbr_is_dummy[0],
                                       nc->x, nc->y, nc->z, sys->order);
          nc->downnumeles[i] = ni->upnumeles[0];
          if (sys->dmtcnt) {
            sys->mm.Q2Lcnt[ni->level][nc->level]++;
          }
        }
        else {
          nc->downvects[i] = ni->multi;
          nc->downmats[i] = mulMulti2Local(sys, ni->x, ni->y, ni->z, nc->x,
                                           nc->y, nc->z, sys->order);
          nc->downnumeles[i] = ni->multisize;
          if (sys->dmtcnt) {
            sys->mm.M2Lcnt[ni->level][nc->level]++;
          }
        }
      }
    }
  }
}




