
#include "mulGlobal.h"
#include "mulMulti.h"
#include "mulLocal.h"
#include "mulStruct.h"
#include "mulDisplay.h"
#include <cmath>

/*
  initializes the factorial fraction array used in M2L, L2L matrix calculation
*/
void evalFacFra(ssystem *sys, double **array, int order)
/* int order: array is 2*order+1 x 2*order+1 */
/* double **array: array[num][den] = num!/den! */
{
  int d, i;
  for(i = 0; i <= 2*order; i++) {
    array[i][i] = 1.0; /* do main diagonal */
    if(i > 0 && i < 2*order) array[i+1][i] = i+1; /* do first sub diagonal */
  }
  for(d = 3; d <= 2*order; d++) { /* loop on lower triangular rows */
    for(i = 1; i < d-1; i++) {  /* loop on columns */
      array[d][i] = array[d-1][i]*array[d][d-1];
    }
  }
  /* invert lower part entries and copy to top */
  for(d = 2; d <= 2*order; d++) {
    for(i = 1; i <= d-1; i++) {
      array[i][d] = 1/array[d][i];
    }
  }
  /* copy 1st row and column from computed values */
  for(d = 1; d <= 2*order; d++) {
    array[0][d] = array[1][d];
    array[d][0] = array[d][1];
  }

  if (sys->disfac) {
    sys->msg("FACTORIAL FRACTION ARRAY:\n");
    dumpMat(sys, array, 2*order+1, 2*order+1);
  }
}

/*
  initializes sqrt((m+n)!/(n-m)!) lookup table (for L2L)
*/
void evalSqrtFac(ssystem * /*sys*/, double **arrayout, double **arrayin, int order)
{
  int n, m;                     /* arrayout[n][m] = sqrt((m+n)!/(n-m)!) */

  /* set up first column, always ones */
  for(n = 0; n < order+1; n++) arrayout[n][0] = 1.0;

  /* set up lower triangular (n+m)!/(n-m)! */
  for(n = 1; n <= order; n++) {
    for(m = 1; m <= n; m++) {
      arrayout[n][m] = sqrt(arrayin[n][m]);
    }
  }

  /* TODO: not available as option
  if (sys->dissfa) {
    sys->msg("SQUARE ROOT FACTORIAL ARRAY:\n");
    dumpMat(arrayout, order+1, order+1);
  }
  */
}


/*
  initializes cos[(m+-k)beta] and sin[(m+-k)beta] lookup tables (M2L and L2L)
*/
static void evalSinCos(ssystem *sys, double beta, int order)
{
  int i;
  double temp = beta;

  for(i = 1; i <= 2*order; beta += temp, i++) {
    sys->mm.sinmkB[i] = sin(beta);
    sys->mm.cosmkB[i] = cos(beta);
  }
}

/*
  looks up sin[(m+-k)beta]
*/
static double sinB(ssystem *sys, int sum)
{
  if(sum < 0) return(-sys->mm.sinmkB[abs(sum)]);
  else return(sys->mm.sinmkB[sum]);
}

/*
  looks up cos[(m+-k)beta]
*/
static double cosB(ssystem *sys, int sum)
{
  return(sys->mm.cosmkB[abs(sum)]);
}

/* 
  Used for all but no local downward pass. 
*/
double **mulMulti2Local(ssystem *sys, double x, double y, double z, double xp, double yp, double zp, int order)
/* double x, y, z, xp, yp, zp: multipole and local cube centers */
{
  int j, k, n, m;
  int terms = multerms(order);  /* the number of non-zero moments */
  int ct = costerms(order);     /* the number of non-zero cos (bar) moments */
  double **mat;                 /* the transformation matrix */
  double rho, cosA, beta;       /* spher. position of multi rel to local */
  double rhoJ, rhoN;            /* rho^j and (-1)^n*rho^(n+1) in main loop */
  double rhoFac;                /* = rhoJ*rhoN intermediate storage */
  double temp1, temp2, temp3;

  /* allocate the multi to local transformation matrix */
  mat = sys->heap.mat(terms, terms, AM2L);

  /* find relative spherical coordinates */
  xyz2sphere(x, y, z, xp, yp, zp, &rho, &cosA, &beta);

  /* generate legendre function evaluations */
  evalLegendre(cosA, sys->mm.tleg, 2*order); /* multi->loc needs 2x legendres */

  /* generate sin[(m+-k)beta] and cos[(m+-k)beta] look up arrays */
  /*  other lookup arrays generated in mulMultiAlloc() */
  evalSinCos(sys, beta, order);

  /* generate multi to local transformation matrix; uses NB12 pg30 */
  /*  rhoFac factor divides could be reduced to once per loop */
  for(j = 0, rhoJ = 1.0; j <= order; rhoJ *= rho, j++) {
    for(k = 0; k <= j; k++) {   /* loop on Nj^k's, local exp moments */
      for(n = 0, rhoN = rho; n <= order; rhoN *= (-rho), n++) {
        for(m = 0; m <= n; m++) { /* loop on On^m's, multipole moments */

          /* generate a bar(N)j^k and dblbar(N)j^k entry */
          rhoFac = rhoJ*rhoN;   /* divisor to give (-1)^n/rho^(j+n+1) factor */
          if(k == 0) {          /* use abbreviated formulae in this case */

            /* generate only bar(N)j^0 entry (dblbar(N)j^0 = 0 always) */
            if(m != 0) {
              temp1 = sys->mm.tleg[CINDEX(j+n, m)]*sys->mm.facFrA[j+n-m][n+m];
              mat[CINDEX(j, 0)][CINDEX(n, m)] += temp1*cosB(sys, m)/rhoFac;
              mat[CINDEX(j, 0)][SINDEX(n, m, ct)] += temp1*sinB(sys, m)/rhoFac;
            }
            else mat[CINDEX(j, 0)][CINDEX(n, 0)] 
                += sys->mm.tleg[CINDEX(j+n, 0)]*sys->mm.facFrA[j+n][n]/rhoFac;
          }
          else {
            temp1 = sys->mm.tleg[CINDEX(j+n, abs(m-k))]
                *sys->mm.facFrA[j+n-abs(m-k)][n+m]*iPwr(sys, abs(k-m)-k-m);
            temp2 = sys->mm.tleg[CINDEX(j+n, m+k)]*sys->mm.facFrA[j+n-m-k][n+m];
            temp3 = sys->mm.tleg[CINDEX(j+n, k)]*sys->mm.facFrA[j+n-k][n]*2;

            /* generate bar(N)j^k entry */
            if(m != 0) {
              mat[CINDEX(j, k)][CINDEX(n, m)] 
                  += (temp1*cosB(sys,m-k)+temp2*cosB(sys,m+k))/rhoFac;
              mat[CINDEX(j, k)][SINDEX(n, m, ct)] 
                  += (temp1*sinB(sys,m-k)+temp2*sinB(sys,m+k))/rhoFac;
            }
            else mat[CINDEX(j, k)][CINDEX(n, 0)] += temp3*cosB(sys,k)/rhoFac;

            /* generate dblbar(N)j^k entry */
            if(m != 0) {
              mat[SINDEX(j, k, ct)][CINDEX(n, m)] 
                  += (-temp1*sinB(sys,m-k)+temp2*sinB(sys,m+k))/rhoFac;
              mat[SINDEX(j, k, ct)][SINDEX(n, m, ct)] 
                  += (temp1*cosB(sys,m-k)-temp2*cosB(sys,m+k))/rhoFac;
            }
            else mat[SINDEX(j, k, ct)][CINDEX(n, 0)] += temp3*sinB(sys,k)/rhoFac;
          }
        }
      }
    }
  }

  if (sys->dism2l) {
    dispM2L(sys, mat, x, y, z, xp, yp, zp, order);
  }

  return(mat);
}

/* 
  Used only for true (Greengard) downward pass - similar to Multi2Local
*/
double **mulLocal2Local(ssystem *sys, double x, double y, double z, double xc, double yc, double zc, int order)
/* double x, y, z, xc, yc, zc: parent and child cube centers */
{
  int j, k, n, m;
  int terms = multerms(order);  /* the number of non-zero moments */
  int ct = costerms(order);     /* the number of non-zero cos (bar) moments */
  double **mat;                 /* the transformation matrix */
  double rho, cosA, beta;       /* spher. position of multi rel to local */
  double rhoJ, rhoN;            /* rho^j and (-1)^n*rho^(n+1) in main loop */
  double rhoFac;                /* = rhoJ*rhoN intermediate storage */
  double temp1, temp2, temp3;

  /* allocate the local to local transformation matrix */
  mat = sys->heap.mat(terms, terms, AL2L);

  /* find relative spherical coordinates */
  xyz2sphere(x, y, z, xc, yc, zc, &rho, &cosA, &beta);

  /* generate legendre function evaluations */
  evalLegendre(cosA, sys->mm.tleg, 2*order); /* local->local needs 2x legendres */

  /* generate sin[(m+-k)beta] and cos[(m+-k)beta] look up arrays */
  /*  other lookup arrays generated in mulMultiAlloc() */
  evalSinCos(sys, beta, order);

  /* generate local to local transformation matrix; uses NB12 pg36Y */
  /*  rhoFac factor divides could be reduced to once per loop */
  for(j = 0, rhoJ = 1.0; j <= order; rhoJ *= (-rho), j++) {
    for(k = 0; k <= j; k++) {   /* loop on Nj^k's, local exp moments */
      for(n = j, rhoN = rhoJ; n <= order; rhoN *= (-rho), n++) {
        for(m = 0; m <= n; m++) { /* loop on On^m's, old local moments */

          /* generate a bar(N)j^k and dblbar(N)j^k entry */
          rhoFac = rhoN/rhoJ;   /* divide to give (-rho)^(n-j) factor */
          if(k == 0 && n-j >= m) {  /* use abbreviated formulae in this case */

            /* generate only bar(N)j^0 entry (dblbar(N)j^0 = 0 always) */
            if(m != 0) {
              temp1 = sys->mm.tleg[CINDEX(n-j, m)]*sys->mm.facFrA[0][n-j+m]*rhoFac;
              mat[CINDEX(j, 0)][CINDEX(n, m)] += temp1*cosB(sys,m);
              mat[CINDEX(j, 0)][SINDEX(n, m, ct)] += temp1*sinB(sys,m);
            }
            else mat[CINDEX(j, 0)][CINDEX(n, 0)] += sys->mm.tleg[CINDEX(n-j, 0)]
                    *sys->mm.facFrA[0][n-j]*rhoFac;
          }
          else {
            if(n-j >= abs(m-k)) temp1 = sys->mm.tleg[CINDEX(n-j, abs(m-k))]
                *sys->mm.facFrA[0][n-j+abs(m-k)]*iPwr(sys, m-k-abs(m-k))*rhoFac;
            if(n-j >= m+k) temp2 = sys->mm.tleg[CINDEX(n-j, m+k)]
                *sys->mm.facFrA[0][n-j+m+k]*iPwr(sys, 2*k)*rhoFac;
            if(n-j >= k) temp3 = 2*sys->mm.tleg[CINDEX(n-j, k)]
                *sys->mm.facFrA[0][n-j+k]*iPwr(sys, 2*k)*rhoFac;

            /* generate bar(N)j^k entry */
            if(m != 0) {
              if(n-j >= abs(m-k)) {
                mat[CINDEX(j, k)][CINDEX(n, m)] += temp1*cosB(sys,m-k);
                mat[CINDEX(j, k)][SINDEX(n, m, ct)] += temp1*sinB(sys,m-k);
              }
              if(n-j >= m+k) {
                mat[CINDEX(j, k)][CINDEX(n, m)] += temp2*cosB(sys,m+k);
                mat[CINDEX(j, k)][SINDEX(n, m, ct)] += temp2*sinB(sys,m+k);
              }
            }
            else if(n-j >= k) mat[CINDEX(j, k)][CINDEX(n, 0)] += temp3*cosB(sys,k);

            /* generate dblbar(N)j^k entry */
            if(m != 0) {
              if(n-j >= abs(m-k)) {
                mat[SINDEX(j, k, ct)][CINDEX(n, m)] += (-temp1*sinB(sys,m-k));
                mat[SINDEX(j, k, ct)][SINDEX(n, m, ct)] += temp1*cosB(sys,m-k);
              }
              if(n-j >= m+k) {
                mat[SINDEX(j, k, ct)][CINDEX(n, m)] += (-temp2*sinB(sys,m+k));
                mat[SINDEX(j, k, ct)][SINDEX(n, m, ct)] += temp2*cosB(sys,m+k);
              }
            }
            else if(n-j >= k) 
                mat[SINDEX(j, k, ct)][CINDEX(n, 0)] += temp3*sinB(sys,k);
          }
        }
      }
    }
  }

  if (sys->disl2l) {
    dispL2L(sys, mat, x, y, z, xc, yc, zc, order);
  }

  return(mat);
}

/*
  sets up xformation for distant cube charges to local expansion
  form almost identical to mulQ2Multi - follows NB12 pg 32 w/m,n replacing k,j
  OPTIMIZATIONS INVOLVING is_dummy HAVE NOT BEEN COMPLETELY IMPLEMENTED
*/
double **mulQ2Local(ssystem *sys, charge **chgs, int numchgs, int *is_dummy, double x, double y, double z, int order)
{
  int i, j, n, m;
  int cterms = costerms(order), terms = multerms(order);
  double **mat, temp;
  double cosA;                  /* cosine of elevation coordinate */

  /* Allocate the matrix. */
  mat = sys->heap.mat(terms, numchgs, AQ2L);

  /* get Legendre function evaluations, one set for each charge */
  /*  also get charge coordinates, set up for subsequent evals */
  for(j = 0; j < numchgs; j++) { /* for each charge */

    /* get cosA for eval; save rho, beta in rho^n and cos/sin(m*beta) arrays */
    xyz2sphere(chgs[j]->x, chgs[j]->y, chgs[j]->z,
               x, y, z, &(sys->mm.Rho[j]), &cosA, &(sys->mm.Beta[j]));
    sys->mm.Rhon[j] = sys->mm.Rho[j]; /* init powers of rho_i's */
    sys->mm.Betam[j] = sys->mm.Beta[j];         /* init multiples of beta */
    evalLegendre(cosA, sys->mm.tleg, order);    /* write moments to temporary array */
    /* write a column of the matrix with each set of legendre evaluations */
    for(i = 0; i < cterms; i++) mat[i][j] = sys->mm.tleg[i]; /* copy for cos terms */
  }

  if (sys->dalq2l) {
    sys->msg(
            "\nQ2L MATRIX BUILD:\n    AFTER LEGENDRE FUNCTION EVALUATON\n");
    dumpMat(sys, mat, terms, numchgs);
  }

  /* add the rho^n+1 factors to the cos matrix entries. */
  for(n = 0; n <= order; n++) { /* loop on rows of matrix */
    for(m = 0; m <= n; m++) {
      for(j = 0; j < numchgs; j++) 
          mat[CINDEX(n, m)][j] /= sys->mm.Rhon[j]; /* divide by factor */
    }
    for(j = 0; j < numchgs; j++) sys->mm.Rhon[j] *= sys->mm.Rho[j];     /* rho^n -> rho^n+1 */
  }

  if (sys->dalq2l) {
    sys->msg("    AFTER ADDITION OF (1/RHO)^N+1 FACTORS\n");
    dumpMat(sys, mat, terms, numchgs);
  }

  /* copy result to lower (sine) part of matrix */
  for(n = 1; n <= order; n++) { /* loop on rows of matrix */
    for(m = 1; m <= n; m++) {
      for(j = 0; j < numchgs; j++) { /* copy a row */
        mat[SINDEX(n, m, cterms)][j] = mat[CINDEX(n, m)][j];
      }
    }
  }

  if (sys->dalq2l) {
    sys->msg("    AFTER COPYING SINE (LOWER) HALF\n");
    dumpMat(sys, mat, terms, numchgs);
  }

  /* add factors of cos(m*beta) and sin(m*beta) to matrix entries */
  for(m = 0; m <= order; m++) { /* lp on m in Mn^m */
    for(n = m; n <= order; n++) { /* loop over rows with same m */
      for(j = 0; j < numchgs; j++) { /* add factors to a row */
        if(m == 0)  mat[CINDEX(n, m)][j] *= fact(sys, n); /* j! part of bar(N)j^0 */
        else {                  /* for Nj^k, k != 0 */
          temp = 2.0*fact(sys, n-m);            /* find the factorial for moment */
          mat[CINDEX(n, m)][j] *= (temp*cos(sys->mm.Betam[j]));   /* note mul by 2 */
          mat[SINDEX(n, m, cterms)][j] *= (temp*sin(sys->mm.Betam[j]));
        }
      }
    }
    if(m > 0) {
      for(j = 0; j < numchgs; j++) sys->mm.Betam[j] += sys->mm.Beta[j];/* (m-1)*beta->m*beta */
    }
  }

  /* THIS IS NOT VERY GOOD: zero out columns corresponding to dummy panels */
  for(j = 0; j < numchgs; j++) {
    if(is_dummy[j]) {
      for(i = 0; i < terms; i++) {
        mat[i][j] = 0.0;
      }
    }
  }

  if (sys->disq2l) {
     dispQ2L(sys, mat, chgs, numchgs, x, y, z, order);
  }

  return(mat);
}

/*
  builds local expansion evaluation matrix; not used for fake dwnwd pass
  follows NB10 equation marked circle(2A) except roles of j,k and n,m switched
  very similar to mulMulti2P()
*/
double **mulLocal2P(ssystem *sys, double x, double y, double z, charge **chgs, int numchgs, int order)
{
  double **mat;
  double cosTh;                 /* cosine of elevation coordinate */
  int i, j, k, m, n, kold;
  int cterms = costerms(order), terms = multerms(order);

  mat = sys->heap.mat(numchgs, terms, AL2P);

  /* get Legendre function evaluations, one set for each charge */
  /*   also get charge coordinates to set up rest of matrix */
  for(i = 0; i < numchgs; i++) { /* for each charge, do a legendre eval set */
    xyz2sphere(chgs[i]->x, chgs[i]->y, chgs[i]->z,
               x, y, z, &(sys->mm.Ir[i]), &cosTh, &(sys->mm.phi[i]));
    sys->mm.Irn[i] = 1.0; /* initialize r^n vec. */
    sys->mm.Mphi[i] = sys->mm.phi[i];           /* initialize m*phi vector */
    evalLegendre(cosTh, mat[i], order); /* wr moms to 1st (cos) half of row */
  }

  if (sys->dall2p) {
    sys->msg(
            "\nL2P MATRIX BUILD:\n    AFTER LEGENDRE FUNCTION EVALUATON\n");
    dumpMat(sys, mat, numchgs, terms);
  }

  /* add the r^n factors to the left (cos(m*phi)) half of the matrix */
  for(j = 0, k = kold = 1; j < cterms; j++) { /* loop on columns of matrix */
    for(i = 0; i < numchgs; i++) mat[i][j] *= sys->mm.Irn[i]; /* multiply by r^n */
    k -= 1;
    if(k == 0) {                /* so that n changes as appropriate */
      kold = k = kold + 1;
      for(i = 0; i < numchgs; i++) sys->mm.Irn[i] *= sys->mm.Ir[i]; /* r^n -> r^n+1 */
    }
  }

  if (sys->dall2p) {
    sys->msg(
            "    AFTER ADDITION OF R^N FACTORS\n");
    dumpMat(sys, mat, numchgs, terms);
  }

  /* add the factorial fraction factors to the left (cos(m*phi)) part of mat */
  for(n = 0; n <= order; n++) {
    for(m = 0; m <= n; m++) {
      for(i = 0; i < numchgs; i++) mat[i][CINDEX(n, m)] /= fact(sys, n+m);
    }
  }

  if (sys->dall2p) {
    sys->msg(
            "    AFTER ADDITION OF FACTORIAL FRACTION FACTORS\n");
    dumpMat(sys, mat, numchgs, terms);
  }

  /* copy left half of matrix to right half for sin(m*phi) terms */
  for(i = 0; i < numchgs; i++) { /* loop on rows of matrix */
    for(n = 1; n <= order; n++) { 
      for(m = 1; m <= n; m++) { /* copy a row */
        mat[i][SINDEX(n, m, cterms)] = mat[i][CINDEX(n, m)];
      }
    }
  }

  if (sys->dall2p) {
    sys->msg(
            "    AFTER COPYING SINE (RIGHT) HALF\n");
    dumpMat(sys, mat, numchgs, terms);
  }

  /* add factors of cos(m*phi) and sin(m*phi) to left and right halves resp. */
  for(m = 1; m <= order; m++) { /* lp on m in Mn^m (no m=0 since cos(0)=1) */
    for(n = m; n <= order; n++) { /* loop over cols with same m */
      for(i = 0; i < numchgs; i++) { /* add factors to a column */
        mat[i][CINDEX(n, m)] *= cos(sys->mm.Mphi[i]);
        mat[i][SINDEX(n, m, cterms)] *= sin(sys->mm.Mphi[i]);
      }
    }
    for(i = 0; i < numchgs; i++) sys->mm.Mphi[i] += sys->mm.phi[i]; /* (m-1)*phi->m*phi */
  }

  if (sys->disl2p) {
    dispL2P(sys, mat, x, y, z, chgs, numchgs, order);
  }

  return(mat);
}
