
#include <cstdio>
#include <cmath>
#include <algorithm>

#include "disrect.h"
#include "epsilon.h"

/*
  write a set of quadralateral panels for a rectancle in quickif.c format
  - produces edge cells that are always sedgewid in width and either
    sinerwid or linerwid in length if on short or long side of rectangle
    - those on corners are sedgewidXsedgewid
  - produces inner cells that are all sinerwid long on the short side
    of the rectangle and linerwid long on the long side
  - there are sinernum inner panels along the short egde, linernum along long
  - zero with sedgewid ok but sinerwid and linerwid must always be nonzero
  - returns number of panels generated
*/

static int wrQuadCells(FILE *fp, int cond, int sinernum, int linernum, double sinerwid, double linerwid, double sedgewid,
                 double x1, double y1, double z1, double x2, double y2, double z2, double x4, double y4, double z4)
{
  int sncell, lncell, npanels = 0;
  double x12, y12, z12, x14, y14, z14; /* unit vector from p1 to p2, p4 */
  double norm2, norm4, x, y, z;
  double xt, yt, zt;

  /* set up unit vectors - p1-p2 must be the short side */
  x12 = x2 - x1;
  y12 = y2 - y1;
  z12 = z2 - z1;
  norm2 = sqrt(x12*x12 + y12*y12 + z12*z12);
  x12 /= norm2;
  y12 /= norm2;
  z12 /= norm2;

  x14 = x4 - x1;
  y14 = y4 - y1;
  z14 = z4 - z1;
  norm4 = sqrt(x14*x14 + y14*y14 + z14*z14);
  x14 /= norm4;
  y14 /= norm4;
  z14 /= norm4;

  /* loop through long and short side points and make quads */
  for(sncell = 0; sncell < sinernum+2; sncell++) {
    for(lncell = 0; lncell < linernum+2; lncell++) {
      /* dump the quad line if cell has nonzero area
         - ie if not an edge cell or if an edge cell with nonzero width */
      if(sedgewid != 0.0 ||
         (sncell != 0 && sncell != sinernum+1 &&
          lncell != 0 && lncell != linernum+1 && sedgewid == 0.0)) {
        /* figure the current lower left point */
        x = x1;
        y = y1;
        z = z1;
        if(sncell > 0) {                /* add in short side edge cell width */
          x += (sedgewid*x12);
          y += (sedgewid*y12);
          z += (sedgewid*z12);
          if(sncell > 1) {      /* add in short side inner cell widths */
            x += ((double)sncell-1)*(sinerwid*x12);
            y += ((double)sncell-1)*(sinerwid*y12);
            z += ((double)sncell-1)*(sinerwid*z12);
          }
        }
        if(lncell > 0) {                /* add in long side edge cell width */
          x += (sedgewid*x14);
          y += (sedgewid*y14);
          z += (sedgewid*z14);
          if(lncell > 1) {      /* add in long side inner cell widths */
            x += ((double)lncell-1)*(linerwid*x14);
            y += ((double)lncell-1)*(linerwid*y14);
            z += ((double)lncell-1)*(linerwid*z14);
          }
        }

        fprintf(fp, "Q %d ", cond);
        /* dump point 1 */
        fprintf(fp, "%.5e %.5e %.5e ", x, y, z);
        /* dump point 2 */
        if(sncell == 0 || sncell == sinernum+1) { /* if short side edge cell */
          xt = x+sedgewid*x12;  /* t coordinates are of point 2 */
          yt = y+sedgewid*y12;
          zt = z+sedgewid*z12;
          fprintf(fp, "%.5e %.5e %.5e ", xt, yt, zt);
        }
        else {                  /* if a short side inner cell */
          xt = x+sinerwid*x12;
          yt = y+sinerwid*y12;
          zt = z+sinerwid*z12;
          fprintf(fp, "%.5e %.5e %.5e ", xt, yt, zt);
        }
        /* dump point 3 (across from point 1 = (x,y,z)) and point 4 */
        if(lncell == 0 || lncell == linernum+1) {  /* if long side edge cell */
          fprintf(fp, "%.5e %.5e %.5e ",
                  xt+sedgewid*x14, yt+sedgewid*y14, zt+sedgewid*z14); /* p3 */
          fprintf(fp, "%.5e %.5e %.5e\n",
                  x+sedgewid*x14, y+sedgewid*y14, z+sedgewid*z14); /* p4 */
        }
        else {                  /* if a long side inner cell */
          fprintf(fp, "%.5e %.5e %.5e ",
                  xt+linerwid*x14, yt+linerwid*y14, zt+linerwid*z14);
          fprintf(fp, "%.5e %.5e %.5e\n",
                  x+linerwid*x14, y+linerwid*y14, z+linerwid*z14);
        }
        npanels++;
      }
    }
  }
  return(npanels);
}

/*
  writes quad panel lines in quickif.c format to given file pointer
  - corners must be ordered around the rectangle, not across the diagonal
    (p1 must be opposite p3 and p2 must be opposite p4)
  - tries to panel specified rectangle with uniform inner and skinny edge cells
  - makes edge cell widths percentage of inner cell widths (10% best says A.R.)
  - ncells must be greater than zero, edge frac must be non-negative
  - ways to get uniform panels: edgefrac = 1.0, edgefrac = 0.0
    both give ncells panels on a side or ncells = 2 if want 2 on a side
  - ways to get one panel the same as the rectangle: anything with ncells = 1
  - should also work with parallelograms
  - returns the number of panels made
  - no_discr 
*/
int disRect(FILE *fp, int cond, double edgefrac, int ncells, bool no_discr,
            double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3, double x4, double y4, double z4)
// int ncells;                     number of cells on short side, > 2 
// int cond;                       conductor number 
// int no_discr;                   true => no discr., just wr the four pnts 
// double edgefrac;                edge cell widths =edgefrac*(inner widths) 
// double x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;  4 corners 
// FILE *fp;
{
  int lflag, linernum, sinernum, npanels;
  double lside, sside, temp, sedgewid, sinerwid, linerwid;
  static double lepsilon, uepsilon;
  static int fstflag = 1;

  if(fp == NULL) {
    fprintf(stderr, "\ndisRect: bad output file pointer\n");
    exit(0);
  }

  if(no_discr) {
    fprintf(fp, "Q %d %.5e %.5e %.5e  %.5e %.5e %.5e",
            cond, x1, y1, z1, x2, y2, z2);
    fprintf(fp, " %.5e %.5e %.5e  %.5e %.5e %.5e\n",
            x3, y3, z3, x4, y4, z4);
    return(1);
  }

  /* setup bounds on machine precision on first call */
  if(fstflag == 1) {
    fstflag = 0;
    getEpsBnds(&uepsilon, &lepsilon);
  }

  /* find the sides */
  lflag = 2;                    /* implies long side is p1-p2 side */
  lside = sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) + (z1-z2)*(z1-z2));
  sside = sqrt((x1-x4)*(x1-x4) + (y1-y4)*(y1-y4) + (z1-z4)*(z1-z4));
  if(lside < sside) {
    temp = lside;
    lside = sside;
    sside = temp;
    lflag = 4;
  }

  /* figure the short side cell widths */
  if(ncells == 1 || ncells == 2) edgefrac = 0.0;
  if(edgefrac > 0.0) {
    sinernum = ncells-2;        /* sinernum = number of inner cells */
    sinerwid = sside/(2.0*edgefrac + (double)sinernum);
    sedgewid = edgefrac*sinerwid;
  }
  else if(edgefrac == 0.0) {
    sinernum = ncells;  /* ncells = sinernum = #nonzero width cells */
    sinerwid = sside/((double)(ncells));
    sedgewid = 0.0;
  }
  else {
    fprintf(stderr, "\ndisRect: negative edge to inner panel ratio = %g\n",
            edgefrac);
    exit(0);
  }

  /* figure the long side inner cell widths (edge cell widths are the same) */
  temp = (lside/sinerwid - 2.0*edgefrac);
  /* allow for cancellation error in temp (underestimates only)
     - otherwise possible for truncation to eliminate a wanted panel
     - if this screws up, the worst it'll do is produce an extra inner sec'n */
  /* add scaled epsilon to result to get close numbers over trunc. threshold */
  temp += (std::max(lside/sinerwid, 2.0*edgefrac)*uepsilon);
  linernum = temp;              /* truncate */
  linerwid = (lside-2.0*sedgewid)/((double)linernum); /* long side cell wdth */

  /* write out the quad cell lines */
  if(lflag == 4) npanels = wrQuadCells(fp, cond, sinernum, linernum, 
                                       sinerwid, linerwid, sedgewid, 
                                       x1, y1, z1, x2, y2, z2, x4, y4, z4);
  else npanels = wrQuadCells(fp, cond, sinernum, linernum, sinerwid, linerwid, 
                   sedgewid, x1, y1, z1, x4, y4, z4, x2, y2, z2);
  fprintf(fp, "*\n");

  return(npanels);
}
