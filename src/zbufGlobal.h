
#if !defined(zbufGlobal_H)
#define zbufGlobal_H

/* #include <stdio.h> */

/* zbuf data structures */
#include "zbufStruct.h"

#ifndef MIN
#define MIN(A,B)  ( (A) > (B) ? (B) : (A) )
#endif
#ifndef MAX
#define MAX(A,B)  ( (A) > (B) ? (A) : (B) )
#endif

#ifndef M_PI
#define M_PI 3.1415926535897931160E0
#endif

#ifndef ON
#define ON 1
#endif
#ifndef OFF
#define OFF 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define FONT 5.0                /* font size used to label lines in ps file */
#define CMDFONT 10.0            /* font used to write command line */

#define OFFSETX 34.0            /* offset on x from lower left (pnts) */
#define OFFSETY 34.0            /* offset on y from lower left (pnts) */
#define IMAGEX 540.0            /* x height of image (chosen to be points) */
#define IMAGEY 720.0            /* y height of image (chosen to be points)
                                   comand line scale parameter muliplies
                                   IMAGEX/Y to get final image size
                                   - these values should allow 7.5x10 images */
#define KEYHGT IMAGEY/7.0       /* height of the shading key, -q option only */
#define KEYWID IMAGEX/7.5       /* width of the shading key, -q option only */
#define KEYBLKS 5               /* number of blocks in the key */
#define KEYPREC 5               /* precision of labels in key */
#define KEYFONT 10.0            /* font used to label key */

#define MARGIN 1e-5             /* 1 + MARGIN is considered 1; > -MARGIN=0;
                                   MARGIN should be approx machine epsilon
                                   except that PATRAN files can lead to probs*/
#define PARMGN 1.0              /* multiplies MARGIN in plane equivalence
                                   checks (see is1stFaceDeeper()) to allow the
                                   extra slop those checks apparently need */

#define LINE 1                  /* used directly in setlinewidth as default */
#define LINCAP 1                /* 0 = square, 1 = round cap, 2 = square cap */
#define LINJIN 1                /* 0 = miter, 1 = round, 2 = knock off cnrs */
#define GREYLEV 0.0             /* 1 = black fill */
#define DASHED -1               /* = width arg that means draw a dashed line */
#define DASWTH 2                /* width of dashed lines */
#define OVRWTH 0                /* overide line width - get with -w option
                                   (not implemented) */
#define AXEWID 0.0              /* width of axis lines */
#define MAXSIDES 4              /* maximum #sides allowed for a face */

#define DEBUG 1                 /* controls view point dump, etc. */
#define DEBUGX OFF              /* doLinesIntersect stats */
#define DMPINFO OFF             /* dumps face/line info when in a bind */
#define DMPMATLAB OFF           /* dmps info in MATLAB format if DMPINFO==ON */
#define RMWEDGE OFF             /* removes 1st quadrant dielectric panels */

#define ALEN 8                  /* default arrow length in points */
#define AWID 4                  /* default arrow width in points */
#define DOTSIZ 2                /* default dot radius, points */

#define POS 0                   /* defined in whichSide() */
#define NEG 1
#define SPLIT 2
#define SAME 3

#define REVERSE 2               /* defined in is1stFaceDeeper() */

#define XOVTST OFF              /* cross overlap test enable--see zbufSort.c */

#ifndef XI
#define XI 0                    /* upward-pointing axes types */
#endif
#ifndef YI
#define YI 1
#endif
#ifndef ZI
#define ZI 2
#endif

/* default command line options */
#define DEFAZM 50.0             /* default azimuth, degrees (-a) */
#define DEFELE 50.0             /* default elevation, degrees (-e) */
#define DEFROT 0.0              /* default rotation rel z axis, degrees (-r) */
#define DEFDST 2.0              /* default view dist, 0 = on obj edge (-d) */
#define DEFSCL 1.0              /* default scale, fractions of IMAGEX,Y (-s) */
#define DEFWID 1.0              /* default line width, points (-w) */
#define DEFAXE 1.0              /* default axes length (-x) */
#define DEFUAX ZI               /* default upward-pointing axis (-u) */

#endif
