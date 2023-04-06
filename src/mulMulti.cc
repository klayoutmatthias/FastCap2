
#include "mulGlobal.h"
#include "mulMulti.h"
#include "mulLocal.h"
#include "mulDisplay.h"
#include "mulStruct.h"
#include <cmath>

void evalFactFac(double **array, int order);

/*
   Used various places.  Returns number of coefficients in the multipole 
   expansion. 
*/
int multerms(int order)
{
  return(costerms(order) + sinterms(order));
}

/*
  returns number of cos(m*phi)-weighted terms in the real (not cmpx) multi exp
*/
int costerms(int order)
{
  return(((order + 1) * (order + 2)) / 2);
}

/*
  returns number of sin(m*phi)-weighted terms in the real (not cmpx) multi exp
*/
int sinterms(int order)
{
  return((((order + 1) * (order + 2)) / 2) - (order+1));
}


/*
  takes two sets of cartesian absolute coordinates; finds rel. spherical coor.
*/
void xyz2sphere(double x, double y, double z, double x0, double y0, double z0, double *rho, double *cosA, double *beta)
{
  /* get relative coordinates */
  x -= x0;                      /* "0" coordinates play the role of origin */
  y -= y0;
  z -= z0;
  /* get spherical coordinates */
  *rho = sqrt(x*x + y*y + z*z);

  if(*rho == 0.0) *cosA = 1.0;
  else *cosA = z/(*rho);

  if(x == 0.0 && y == 0.0) *beta = 0.0;
  else *beta = atan2(y, x);

}

/*
  returns i = sqrt(-1) to the power of the argument
*/
double iPwr(ssystem *sys, int e)
/* int e: exponent, computes i^e */
{
  if(e == 0) return(1.0);
  if(e % 2 != 0) {
    sys->error("iPwr: odd exponent %d\n", e);
  }
  else {
    e = e/2;                    /* get power of negative 1 */
    if(e % 2 == 0) return(1.0);
    else return(-1.0);
  }
}

/*
  returns factorial of the argument (x!)
*/
double fact(ssystem *sys, int x)
{
  double ret = 1.0;
  if(x == 0 || x == 1) return(1.0);
  else if(x < 0) {
    sys->error("fact: attempt to take factorial of neg no. %d\n", x);
  }
  else {
    while(x > 1) {
      ret *= x;
      x--;
    }
    return(ret);
  }
}

/*
  produces factorial factor array for mulMulti2P
*/
void evalFactFac(double **array, int order)
{
  int n, m;                     /* array[n][m] = (m+n)!/(n-m)! */

  /* do first column of lower triangular part - always 1's */
  for(n = 0; n < order+1; n++) array[n][0] = 1.0;

  /* do remaining columns of lower triangular part */
  /*  use (n-m)!/(n+m)! = 1/(n-m+1)...(n+m) since m \leq n */
  /*  (array entry is divided into number to effect mul by (n-m)!/(n+m)!) */
  for(n = 1; n <= order; n++) {
    for(m = 1; m <= n; m++) {
      array[n][m] = (n-(m-1)) * array[n][m-1] * (n+m);
    }
  }
}

/*
  Allocates space for temporary vectors.
*/
void mulMultiAlloc(ssystem *sys, int maxchgs, int order, int depth)
{
  int x;

  if(maxchgs > 0) {
    sys->mm.Rho = sys->heap.alloc<double>(maxchgs, AMSC); /* rho array */
    sys->mm.Rhon = sys->heap.alloc<double>(maxchgs, AMSC); /* rho^n array */
    sys->mm.Beta = sys->heap.alloc<double>(maxchgs, AMSC); /* beta array */
    sys->mm.Betam = sys->heap.alloc<double>(maxchgs, AMSC); /* beta*m array */
    sys->mm.Irn = sys->heap.alloc<double>(maxchgs, AMSC); /* (1/r)^n+1 vector */
    sys->mm.Ir = sys->heap.alloc<double>(maxchgs, AMSC); /* 1/r vector */
    sys->mm.Mphi = sys->heap.alloc<double>(maxchgs, AMSC); /* m*phi vector */
    sys->mm.phi = sys->heap.alloc<double>(maxchgs, AMSC); /* phi vector */
  }
  sys->mm.tleg = sys->heap.alloc<double>(costerms(2*order), AMSC);
                /* temp legendre storage (2*order needed for local exp) */
  sys->mm.factFac = sys->heap.mat(order+1, order+1, AMSC);
  evalFactFac(sys->mm.factFac, order);  /* get factorial factors for mulMulti2P */

  if (sys->dissyn) {
    /* for counts of local/multipole expansions and eval mat builds by level */
    sys->mm.localcnt = sys->heap.alloc<int>(depth + 1, AMSC);
    sys->mm.multicnt = sys->heap.alloc<int>(depth + 1, AMSC);
    sys->mm.evalcnt = sys->heap.alloc<int>(depth + 1, AMSC);
  }

  if (sys->dmtcnt) {
    /* for counts of transformation matrices by level  */
    sys->mm.Q2Mcnt = sys->heap.alloc<int *>(depth+1, AMSC);
    sys->mm.Q2Lcnt = sys->heap.alloc<int *>(depth+1, AMSC);
    sys->mm.Q2Pcnt = sys->heap.alloc<int *>(depth+1, AMSC);
    sys->mm.L2Lcnt = sys->heap.alloc<int *>(depth+1, AMSC);
    sys->mm.M2Mcnt = sys->heap.alloc<int *>(depth+1, AMSC);
    sys->mm.M2Lcnt = sys->heap.alloc<int *>(depth+1, AMSC);
    sys->mm.M2Pcnt = sys->heap.alloc<int *>(depth+1, AMSC);
    sys->mm.L2Pcnt = sys->heap.alloc<int *>(depth+1, AMSC);
    sys->mm.Q2PDcnt = sys->heap.alloc<int *>(depth+1, AMSC);
    for(x = 0; x < depth+1; x++) {
      sys->mm.Q2Mcnt[x] = sys->heap.alloc<int>(depth+1, AMSC);
      sys->mm.Q2Lcnt[x] = sys->heap.alloc<int>(depth+1, AMSC);
      sys->mm.Q2Pcnt[x] = sys->heap.alloc<int>(depth+1, AMSC);
      sys->mm.L2Lcnt[x] = sys->heap.alloc<int>(depth+1, AMSC);
      sys->mm.M2Mcnt[x] = sys->heap.alloc<int>(depth+1, AMSC);
      sys->mm.M2Lcnt[x] = sys->heap.alloc<int>(depth+1, AMSC);
      sys->mm.M2Pcnt[x] = sys->heap.alloc<int>(depth+1, AMSC);
      sys->mm.L2Pcnt[x] = sys->heap.alloc<int>(depth+1, AMSC);
      sys->mm.Q2PDcnt[x] = sys->heap.alloc<int>(depth+1, AMSC);
    }
  }

  /* from here down could be switched out when the fake dwnwd pass is used */
  sys->mm.facFrA = sys->heap.mat(2*order+1, 2*order+1, AMSC);
  /* generate table of factorial fraction evaluations (for M2L and L2L) */
  evalFacFra(sys, sys->mm.facFrA, order);
  sys->mm.sinmkB = sys->heap.alloc<double>(2*order+1, AMSC);
  sys->mm.cosmkB = sys->heap.alloc<double>(2*order+1, AMSC);
  sys->mm.cosmkB[0] = 1.0;              /* look up arrays used for local exp */
  /* generate array of sqrt((n+m)!(n-m)!)'s for L2L
  evalSqrtFac(sqrtFac, factFac, order); */
}

/*
  returns an vector of Legendre function evaluations of form Pn^m(cosA)
  n and m have maximum value order
  vector entries correspond to (n,m) = (0,0) (1,0) (1,1) (2,0) (2,1)...
*/
void evalLegendre(double cosA, double *vector, int order)
{
  int x;
  int m;                        /* as in Pn^m, both <= order */
  double sinMA;                 /* becomes sin^m(alpha) in higher order P's */
  double fact;                  /* factorial factor */

  /* do evaluations of first four functions separately w/o recursions */
  vector[CINDEX(0, 0)] = 1.0;   /* P0^0 */
  if(order > 0) {
    vector[CINDEX(1, 0)] = cosA;        /* P1^0 */
    vector[CINDEX(1, 1)] = sinMA = -sqrt(1-cosA*cosA); /* P1^1 = -sin(alpha) */
  }
  if(order > 1) vector[CINDEX(2, 1)] = 3*sinMA*cosA; /* P2^1 = -3sin()cos() */

  /* generate remaining evaluations by recursion on lower triangular array */
  fact = 1.0;
  for(m = 0; m < order+1; m++) {
    if(m != 0 && m != 1) {      /* set up first two evaluations in row */
      fact *= (2*m - 1); /* (2(m-1)-1)!! -> (2m-1)!! */
      /* use recursion on m */
      if(vector[CINDEX(1, 1)] == 0.0) {
        vector[CINDEX(m, m)] = 0.0;
        if(m != order) vector[CINDEX(m+1, m)] = 0.0;    /* if not last row */
      }
      else {
        cosA = vector[CINDEX(1,0)]/vector[CINDEX(1,1)]; /* cosA= -cot(theta) */
        sinMA *= vector[CINDEX(1,1)]; /*(-sin(alpha))^(m-1)->(-sin(alpha))^m*/
        vector[CINDEX(m, m)] = fact * sinMA;
        if(m != order) {                /* do if not on last row */
          vector[CINDEX(m+1, m)] = vector[CINDEX(1, 0)]*(2*m+1)
              *vector[CINDEX(m, m)];
        }
      }
    }
    for(x = 2; x < order-m+1; x++) { /* generate row of evals recursively */
      vector[CINDEX(x+m, m)] = 
          ((2*(x+m)-1)*vector[CINDEX(1, 0)]*vector[CINDEX(x+m-1, m)]
           - (x + 2*m - 1)*vector[CINDEX(x+m-2, m)])/x;
    }
  }
}
    
  
/* 
  Returns a matrix which gives a cube's multipole expansion when *'d by chg vec
  OPTIMIZATIONS USING is_dummy HAVE NOT BEEN COMPLETELY IMPLEMENTED
*/
double **mulQ2Multi(ssystem *sys, charge **chgs, int *is_dummy, int numchgs, double x, double y, double z, int order)
{
  double **mat;
  double cosA;                  /* cosine of elevation coordinate */
  int i, j, k, kold, n, m;
  int cterms = costerms(order), terms = multerms(order);

  /* Allocate the matrix. */
  mat = sys->heap.alloc<double*>(terms, AQ2M);
  for(i=0; i < terms; i++) 
    mat[i] = sys->heap.alloc<double>(numchgs, AQ2M);

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

  if (sys->dalq2m) {
    sys->msg(
            "\nQ2M MATRIX BUILD:\n    AFTER LEGENDRE FUNCTION EVALUATON\n");
    dumpMat(sys, mat, terms, numchgs);
  }

  /* some of this can be avoided using is_dummy to skip unused columns */
  /* add the rho^n factors to the cos matrix entries. */
  for(i = 1, k = kold = 2; i < cterms; i++) { /* loop on rows of matrix */
    for(j = 0; j < numchgs; j++) mat[i][j] *= sys->mm.Rhon[j]; /* mul in factor */
    k -= 1;
    if(k == 0) {                /* so that effective n varys appropriately */
      kold = k = kold + 1;
      for(j = 0; j < numchgs; j++) sys->mm.Rhon[j] *= sys->mm.Rho[j];   /* r^n-1 -> r^n */
    }
  }

  if (sys->dalq2m) {
    sys->msg("    AFTER ADDITION OF RHO^N FACTORS\n");
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

  if (sys->dalq2m) {
    sys->msg("    AFTER COPYING SINE (LOWER) HALF\n");
    dumpMat(sys, mat, terms, numchgs);
  }

  /* add factors of cos(m*beta) and sin(m*beta) to matrix entries */
  for(m = 1; m <= order; m++) { /* lp on m in Mn^m (no m=0 since cos(0)=1) */
    for(n = m; n <= order; n++) { /* loop over rows with same m */
      for(j = 0; j < numchgs; j++) { /* add factors to a row */
        mat[CINDEX(n, m)][j] *= (2.0*cos(sys->mm.Betam[j]));   /* note factors of 2 */
        mat[SINDEX(n, m, cterms)][j] *= (2.0*sin(sys->mm.Betam[j]));
      }
    }
    for(j = 0; j < numchgs; j++) sys->mm.Betam[j] += sys->mm.Beta[j]; /* (m-1)*beta->m*beta */
  }

  /* THIS IS NOT VERY GOOD: zero out columns corresponding to dummy panels */
  for(j = 0; j < numchgs; j++) {
    if(is_dummy[j]) {
      for(i = 0; i < terms; i++) {
        mat[i][j] = 0.0;
      }
    }
  }

  if (sys->disq2m) {
    dispQ2M(sys, mat, chgs, numchgs, x, y, z, order);
  }

  return(mat);
}

double **mulMulti2Multi(ssystem *sys, double x, double y, double z, double xp, double yp, double zp, int order)
/* double x, y, z, xp, yp, zp: cube center, parent cube center */
{
  double **mat, rho, rhoPwr, cosA, beta, mBeta, temp1, temp2; 
  int r, j, k, m, n, c;
  int cterms = costerms(order), sterms = sinterms(order);
  int terms = cterms + sterms;

  /* Allocate the matrix (terms x terms ) */
  mat = sys->heap.alloc<double*>(terms, AM2M);
  for(r=0; r < terms; r++)
    mat[r] = sys->heap.alloc<double>(terms, AM2M);
  for(r = 0; r < terms; r++)
      for(c = 0; c < terms; c++) mat[r][c] = 0.0;

  /* get relative distance in spherical coordinates */
  xyz2sphere(x, y, z, xp, yp, zp, &rho, &cosA, &beta);

  /* get the requisite Legendre function evaluations */
  evalLegendre(cosA, sys->mm.tleg, order);

  /* for each new moment (Nj^k) stuff the appropriate matrix entries */
  /* done completely brute force, one term at a time; uses exp in nb 12, p29 */
  for(j = 0; j <= order; j++) {
    for(k = 0; k <= j; k++) {
      for(n = 0, rhoPwr = 1.0; n <= j; n++, rhoPwr *= rho) {
        for(m = 0, mBeta = 0.0; m <= n; m++, mBeta += beta) {

          if(k == 0) {          /* figure terms for Nj^0, ie k = 0 */
            if(m <= j-n) {      /* if O moments are nonzero */
              temp1 = fact(sys, j)*rhoPwr*iPwr(sys, 2*m)*sys->mm.tleg[CINDEX(n, m)];
              temp1 /= (fact(sys, j-n+m)*fact(sys, n+m));
              mat[CINDEX(j, k)][CINDEX(j-n, m)] += temp1*cos(mBeta);
              if(m != 0) {              /* if sin term is non-zero */
                mat[CINDEX(j, k)][SINDEX(j-n, m, cterms)] += temp1*sin(mBeta);
              }
            }
          }
          else {                /* figure terms for Nj^k, k != 0 */
            temp1 = fact(sys, j+k)*rhoPwr*sys->mm.tleg[CINDEX(n, m)]/fact(sys, n+m);
            temp2 = temp1*iPwr(sys, 2*m)/fact(sys, j-n+k+m);
            temp1 = temp1*iPwr(sys, k-m-abs(k-m))/fact(sys, j-n+abs(k-m));

            /* write the cos(kPhi) coeff, bar(N)j^k */
            if(m != 0) {
              if(k-m < 0 && abs(k-m) <= j-n) {  /* use conjugates here */
                mat[CINDEX(j, k)][CINDEX(j-n, m-k)] += temp1*cos(mBeta);
                mat[CINDEX(j, k)][SINDEX(j-n, m-k,cterms)] += temp1*sin(mBeta);
              }
              else if(k-m == 0) {       /* double to compensate for 2Re sub. */
                mat[CINDEX(j, k)][CINDEX(j-n, k-m)] += 2*temp1*cos(mBeta);
                /* sin term is always zero */
              }
              else if(k-m > 0 && k-m <= j-n) {
                mat[CINDEX(j, k)][CINDEX(j-n, k-m)] += temp1*cos(mBeta);
                mat[CINDEX(j, k)][SINDEX(j-n, k-m,cterms)] -= temp1*sin(mBeta);
              }
              if(k+m <= j-n) {
                mat[CINDEX(j, k)][CINDEX(j-n, k+m)] += temp2*cos(mBeta);
                mat[CINDEX(j, k)][SINDEX(j-n, k+m,cterms)] += temp2*sin(mBeta);
              }
            }                   /* do if m = 0 and O moments not zero */
            else if(k <= j-n) mat[CINDEX(j, k)][CINDEX(j-n, k)] += temp2;

            /* write the sin(kPhi) coeff, dblbar(N)j^k, if it is non-zero */
            if(m != 0) {
              if(k-m < 0 && abs(k-m) <= j-n) {  /* use conjugates here */
                mat[SINDEX(j, k,cterms)][CINDEX(j-n, m-k)] += temp1*sin(mBeta);
                mat[SINDEX(j, k, cterms)][SINDEX(j-n, m-k, cterms)] 
                    -= temp1*cos(mBeta);
              }
              else if(k-m == 0) {/* double to compensate for 2Re sub */
                mat[SINDEX(j, k, cterms)][CINDEX(j-n, k-m)] 
                    += 2*temp1*sin(mBeta);
                /* sine term is always zero */
              }
              else if(k-m > 0 && k-m <= j-n) {
                mat[SINDEX(j, k,cterms)][CINDEX(j-n, k-m)] += temp1*sin(mBeta);
                mat[SINDEX(j, k, cterms)][SINDEX(j-n, k-m, cterms)] 
                    += temp1*cos(mBeta);
              }
              if(k+m <= j-n) {
                mat[SINDEX(j, k,cterms)][CINDEX(j-n, k+m)] -= temp2*sin(mBeta);
                mat[SINDEX(j, k, cterms)][SINDEX(j-n, k+m, cterms)] 
                    += temp2*cos(mBeta);
              }
            }                   /* do if m = 0 and moments not zero */
            else if(k <= j-n) mat[SINDEX(j, k,cterms)][SINDEX(j-n, k, cterms)] 
                += temp2;
          }
        }
      }
    }
  }
  if (sys->dism2m) {
    dispM2M(sys, mat, x, y, z, xp, yp, zp, order);
  }
  return(mat);
}

/* 
  builds multipole evaluation matrix; used only for fake downward pass 
*/
double **mulMulti2P(ssystem *sys, double x, double y, double z, charge **chgs, int numchgs, int order)
/* double x, y, z: multipole expansion origin */
{
  double **mat;
  double cosTh;                 /* cosine of elevation coordinate */
  int i, j, k, m, n, kold;
  int cterms = costerms(order), sterms = sinterms(order);
  int terms = cterms + sterms;

  mat = sys->heap.mat(numchgs, terms, AM2P);

  /* get Legendre function evaluations, one set for each charge */
  /*   also get charge coordinates to set up rest of matrix */
  for(i = 0; i < numchgs; i++) { /* for each charge, do a legendre eval set */
    xyz2sphere(chgs[i]->x, chgs[i]->y, chgs[i]->z,
               x, y, z, &(sys->mm.Ir[i]), &cosTh, &(sys->mm.phi[i]));

    sys->mm.Irn[i] = sys->mm.Ir[i]; /* initialize (1/r)^n+1 vec. */
    sys->mm.Mphi[i] = sys->mm.phi[i];           /* initialize m*phi vector */

    evalLegendre(cosTh, mat[i], order); /* wr moms to 1st (cos) half of row */

  }

  if (sys->dalm2p) {
    sys->msg(
            "\nM2P MATRIX BUILD:\n    AFTER LEGENDRE FUNCTION EVALUATON\n");
    dumpMat(sys, mat, numchgs, terms);
  }

  /* add the (1/r)^n+1 factors to the left (cos(m*phi)) half of the matrix */
  for(j = 0, k = kold = 1; j < cterms; j++) { /* loop on columns of matrix */
    for(i = 0; i < numchgs; i++) mat[i][j] /= sys->mm.Irn[i]; /* divide by r^n+1 */
    k -= 1;
    if(k == 0) {                /* so that n changes as appropriate */
      kold = k = kold + 1;
      for(i = 0; i < numchgs; i++) sys->mm.Irn[i] *= sys->mm.Ir[i]; /* r^n -> r^n+1 */
    }
  }

  if (sys->dalm2p) {
    sys->msg(
            "    AFTER ADDITION OF (1/R)^N+1 FACTORS\n");
    dumpMat(sys, mat, numchgs, terms);
  }

  /* add the factorial fraction factors to the left (cos(m*phi)) part of mat */
  /*  note that (n-m)!/(n+m)! = 1/(n-m+1)...(n+m) since m \leq n */
  for(n = 1; n <= order; n++) {
    for(m = 1; m <= n; m++) {
      for(i = 0; i < numchgs; i++) mat[i][CINDEX(n, m)] /= sys->mm.factFac[n][m];
    }
  }

  if (sys->dalm2p) {
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

  if (sys->dalm2p) {
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

  if (sys->dism2p) {
    dispM2P(sys, mat, x, y, z, chgs, numchgs, order);
  }

  return(mat);
}







