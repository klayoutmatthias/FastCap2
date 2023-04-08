
#if !defined(mulGlobal_H)
#define mulGlobal_H

#include <time.h>
#include <cstdlib>

#define VERSION 2.0

/*****************************************************************************

misc. global macros

*****************************************************************************/

#define INNER(pap,p,ap,size) for(pap=0.0,i=1; i<=size; i++) pap += p[i]*ap[i];

#ifndef MAX
#define MAX(A,B)  ( (A) > (B) ? (A) : (B) )
#endif

#ifndef MIN
#define MIN(A,B)  ( (A) > (B) ? (B) : (A) )
#endif

#define ABS(A) ( ( (A) > 0 ) ? (A) : (-(A)) )

#define VCOPY(A, B) A[0] = B[0]; A[1] = B[1]; A[2] = B[2];

#define TRUE 1
#define FALSE 0

#define ON 1
#define OFF 0

#define LAST 2
#define ALL 2

#ifndef M_PI
/* pi constant included here since won't be in ANSI C */
#define M_PI       3.1415926535897931160E0  /*Hex  2^ 1 * 1.921FB54442D18 */
#endif

#define E_0 8.854187818E-12     /* epsilon0 +- .000000071E-12 F/m */
#define FPIEPS 4.0*M_PI*E_0     /* 4 pi times the dielectric permittivity,
                                   free-space permittivity is the default,
                                   units are F/m - all dimensions in meters */

/* flags in chkList() in mulDisplay.c (chks direct, local or eval cube lsts) */
#define DIRECT 0
#define LOCAL 1
#define EVAL 3

/* types of surfaces */
#define CONDTR 0                /* conductor surfaces */
#define DIELEC 1                /* dielectric interface surface */
#define BOTH 3                  /* dielectric i/f w/very thin cond on it */

/* used in ps file dump */
#define OPEN 0                  /* open ps file, print hdr, ignore row/col */
#define CLOSE 1                 /* print trailer, close ps file */
#define UPDATE 2                /* => add 2 dots for this row and col */

/* divided difference distances, see electric.c */
#define HPOS (1e-6*cur_panel->max_diag) /* h in positive normal dir */
#define HNEG HPOS               /* h for divided difference in neg nrml dir */

/* level set mode, see placeq, mulSetup.c and input.c */
#define ONELES 2                /* => auto set levs to 1 up fr fully exact */

/* expansion moment index generating macros (see mulMulti.c, mulLocal.c) */
#define CINDEX(N, M) ( (M) + ((N)*((N)+1))/2 )
#define SINDEX(N, M, CTERMS) ( (CTERMS) + (M) + ((N)*((N)+1))/2 - ((N)+1) )

/* used in get_kill_num_list and routines it calls */
#define NOTUNI -1
#define NOTFND -2

/***********************************************************************

  configuration and debug flags

***********************************************************************/

/* types of downward/eval passes */
#define NOLOCL 0                /* multipoles evaluated directly - no locals */
#define NOSHFT 1                /* multis to locals w/o local2local shifts */
#define GRENGD 3                /* full Greengard downward pass/eval */

/* types of iterative methods (values of ITRTYP below) */
#define GCR 0                   /* GCR with single (not block) vector iters */
#define GMRES 1                 /* GMRES with vector iterates */

/* types of finite elements (NOTE: only const. chg den. panels implemented) */
#define CONST 0                 /* constant charge density on panels */
#define AFFINE 1
#define QUADRA 2

/* types of weighted residuals methods (NOTE: only collocation implemented) */
#define COLLOC 0                /* point collocation */
#define SUBDOM 1                /* subdomain collocation */
#define GALKIN 2                /* Galerkin */

/* types of preconditioners. */
#define NONE 0
#define BD 1                    /* Block diagonal (not set up for dielecs). */
#define OL 2                    /* OverLap */

/* Discretization Configuration */
#define WRMETH COLLOC           /* weighted res meth type (COLLOC only now) */
#define ELTYPE CONST            /* finite element type (CONST only now) */
/* Multipole Configuration */
#define DNTYPE GRENGD           /* type of downward/eval pass - see above */
#define MULTI ON                /* ON=> add in multipole contribution to P*q */
#define RADINTER ON             /* ON=> Parent level multis in interlist. */
#define NNBRS 2                 /* Distance to consider a nearest nbr. */
#define ADAPT ON                /* ON=> use adaptive algorithm */
#define OPCNT OFF               /* Counts the Matrix-Vector multiply ops. */
#define DEFORD 2                /* default expansion order */
#define MAXORDER 6              /* Maximum expansion order (sets ary sizes) */
#define MAXDEP 20               /* maximum partitioning depth */
#define NUMDPT 2                /* num pnts for ea dielec panel (2 or 3) */
#define SKIPQD OFF              /* ON => skip dielec panel chg in E eval */
/* Linear System Solution Configuration */
#define ITRTYP GMRES            /* type of iterative method */
#define PRECOND OL              /* NONE=> no preconditioner OL=> use prec. */
#define ABSTOL 0.01             /* iterations until ||res||inf < ABSTOL */
#define MAXITER size            /* max num iterations ('size' => # panels) */
#define EXRTSH 0.9              /* exact/ttl>EXRTSH for lev => make last lev */
/* (add any new configuration flags to dumpConfig() in mulDisplay.c) */

/* blkDirect.c related flags - used only when DIRSOL == ON || EXPGCR == ON */
#define MAXSIZ 0                /* any more tiles than this uses matrix on disk
                                   for DIRSOL == ON or EXPGCR == ON */
#endif
