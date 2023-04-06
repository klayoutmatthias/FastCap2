
#include <cstdio>
#include <cmath>
#include <cstring>
#include <algorithm>

#include "disrect.h"

const double DEFWID = 1.0;              /* default wire width */
const double DEFEFR = 0.1;              /* default edge-cell-width/inner-cell-width */
const int DEFNCL = 3;                /* default #cells on short side of faces */
const int DEFNWI = 2;                /* default problem is DEFNWI X DEFNWI bus xg */

const double X0 = 0.0;                 /* default origin */
const double Y0 = 0.0;
const double Z0 = 0.0;


/*
  writes a quickif.c format dicretization of a bar given by 4 corners
  - corners must all be one edge away from corner1
  - returns the number of panels used
*/
int disBar(FILE *fp, int cond, double edgefrac, int ncells, int wires, bool do_cond, bool do_dielec, bool no_disc,
             double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3, double x4, double y4, double z4)
/* int ncells;                     : number of cells on short side, > 2 */
/* int cond;                       : conductor number */
/* int wires;                      : bar to be used in a wiresXwires xing */
/* double edgefrac;                : edge cell widths =edgefrac*(inner widths) */
/* double x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4; : 4 corners */
{
  int nsec = 2*wires + 1, sec, shortside, npanels = 0, i;
  bool do_ref_side, do_opp_side;
  double x12, y12, z12, x13, y13, z13, x14, y14, z14;
  double rx1, ry1, rz1, rx2, ry2, rz2, rx3, ry3, rz3, rx4, ry4, rz4;
  double ratio;

  fprintf(fp,"*\n* Bar corners: (%g %g %g) (%g %g %g) (%g %g %g) (%g %g %g)\n",
          x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4);

  /* each pass through the loop makes panels for two long faces opposite
     from each other - the ends are done after the loop */
  /* do_cond => only opposite side 0 and ends
     do_dielec => only ref side 0, 1 and opposite side 1 (no ends) */
  for(i = 0; i < 2; i++) {

    /* do_cond => only opposite side 0 and ends
       do_dielec => only ref side 0, 1 and opposite side 1 (no ends) */
    do_ref_side = do_opp_side = false;
    if(do_cond) {
      if(i == 0) do_opp_side = true;
    }
    if(do_dielec) {
      do_ref_side = true;
      if(i == 1) do_opp_side = true;
    }

    /* figure vectors to other corners relative to c1 */
    x12 = x2 - x1; y12 = y2 - y1; z12 = z2 - z1;
    x13 = x3 - x1; y13 = y3 - y1; z13 = z3 - z1;
    x14 = x4 - x1; y14 = y4 - y1; z14 = z4 - z1;

    /* figure which of the "bottom" sides is shorter */
    if(x12*x12 + y12*y12 + z12*z12 > x13*x13 + y13*y13 + z13*z13) shortside =3;
    else shortside = 2;

    /* discretize the two long faces - will be over/under 'wires' bars 
       so break into 'wires' sections and discretize each */
    for(sec = 0; sec < nsec; sec++) {
      ratio = ((double)sec)/((double)nsec);
      if(shortside == 3) {
        rx1 = x1+ratio*x12; ry1 = y1+ratio*y12; rz1 = z1+ratio*z12;
        rx2 = rx1+x12/nsec; ry2 = ry1+y12/nsec; rz2 = rz1+z12/nsec;
        rx3 = rx1+x12/nsec+x13; ry3 = ry1+y12/nsec+y13; rz3 = rz1+z12/nsec+z13;
        rx4 = rx1+x13; ry4 = ry1+y13; rz4 = rz1+z13;
      }
      else {
        rx1 = x1+ratio*x13; ry1 = y1+ratio*y13; rz1 = z1+ratio*z13;
        rx2 = rx1+x12; ry2 = ry1+y12; rz2 = rz1+z12;
        rx3 = rx1+x13/nsec+x12; ry3 = ry1+y13/nsec+y12; rz3 =rz1+z13/nsec+z12;
        rx4 = rx1+x13/nsec; ry4 = ry1+y13/nsec; rz4 = rz1+z13/nsec;
      }
      /* face on reference side */
      if(do_ref_side) {
        fprintf(fp, "* Reference side %d\n", i);
        npanels += disRect(fp, cond, edgefrac, ncells, no_disc,
                           rx1, ry1, rz1, rx2, ry2, rz2, 
                           rx3, ry3, rz3, rx4, ry4, rz4);
      }
      
      /* opposite face */
      if(do_opp_side) {
        fprintf(fp, "* Opposite reference side %d\n", i);
        npanels += disRect(fp, cond, edgefrac, ncells, no_disc,
                         rx1+x14, ry1+y14, rz1+z14, rx2+x14, ry2+y14, rz2+z14, 
                         rx3+x14, ry3+y14, rz3+z14, rx4+x14, ry4+y14, rz4+z14);
      }
    }

    /* rotate reference coordinates to set up for last two long faces */
    if(shortside == 3) {
      x3 = x1; y3 = y1; z3 = z1;
      x1 = x4; y1 = y4; z1 = z4;
      x2 += x14; y2 += y14; z2 += z14;
      x4 += x13; y4 += y13; z4 += z13;
    }
    else {
      x2 = x1; y2 = y1; z2 = z1;
      x1 = x4; y1 = y4; z1 = z4;
      x3 += x14; y3 += y14; z3 += z14;
      x4 += x12; y4 += y12; z4 += z12;
    }
  }

  /* panel the ends */
  if(do_cond) {
    if(shortside == 3) {
      fprintf(fp, "* End faces\n");
      npanels += disRect(fp, cond, edgefrac, ncells, no_disc,
                         x1, y1, z1, x3, y3, z3,
                         x4+x13, y4+y13, z4+z13, x4, y4, z4);
      npanels += disRect(fp, cond, edgefrac, ncells, no_disc,
                         x1+x12, y1+y12, z1+z12, x3+x12, y3+y12, z3+z12,
                         x4+x13+x12, y4+y13+y12, z4+z13+z12, 
                         x4+x12, y4+y12, z4+z12);
    }
    else {
      fprintf(fp, "* End faces\n");
      npanels += disRect(fp, cond, edgefrac, ncells, no_disc,
                         x1, y1, z1, x2, y2, z2,
                         x4+x12, y4+y12, z4+z12, x4, y4, z4);
      npanels += disRect(fp, cond, edgefrac, ncells, no_disc,
                         x1+x13, y1+y13, z1+z13, x2+x13, y2+y13, z2+z13,
                         x4+x13+x12, y4+y13+y12, z4+z13+z12, 
                         x4+x13, y4+y13, z4+z13);
    }
  }

  return(npanels);
}

/*
  generates a wires crossing wires bus crossing example in quickif.c format
  - uses disRect() for all discretization of rectangular faces
*/
int main(int argc, char *argv[])
{
  char **chkp = 0;
  char *chk = 0;
  char name[BUFSIZ];
  bool no_disc = false;
  bool name_given = false;
  int wires = DEFNWI;
  int npanels = 0;
  int ncells = DEFNCL;
  bool cmderr = false;
  int cond = 0;
  bool do_dielec = true;
  bool do_cond = true;
  double edgefrac = DEFEFR;
  double pitch = 2*DEFWID;
  double x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4; /* 4 corners */
  double x0, y0, z0;            /* master origin */
  FILE *fp = 0;

  /* load default parameters */
  x0 = X0; y0 = Y0; z0 = Z0;

  /* parse command line */
  chkp = &chk;                  /* pointers for error checking */
  for(int i = 1; i < argc && cmderr == false; i++) {
    if(argv[i][0] != '-') {
      fprintf(stderr, "%s: illegal argument -- %s\n", argv[0], argv[i]);
      cmderr = true;
      break;
    }
    else if(argv[i][1] == 'c') {
      wires = (int) strtol(&(argv[i][2]), chkp, 10);
      if(*chkp == &(argv[i][2]) || wires < 1) {
        fprintf(stderr, "%s: bad number of conductors/bus `%s'\n", 
                argv[0], &argv[i][2]);
        cmderr = true;
        break;
      }
    }
    else if(argv[i][1] == 'n' && argv[i][2] == 'a') {
      if(sscanf(&(argv[i][3]), "%s", name) != 1) {
        fprintf(stderr, "%s: bad name `%s'\n", argv[0], &argv[i][3]);
        cmderr = true;
        break;
      }
      name_given = true;
    }
    else if(argv[i][1] == 'n') {
      ncells = (int) strtol(&(argv[i][2]), chkp, 10);
      if(*chkp == &(argv[i][2]) || ncells < 1) {
        fprintf(stderr, 
                "%s: bad number of short side panels `%s'\n", 
                argv[0], &argv[i][2]);
        cmderr = true;
        break;
      }
    }
    else if(argv[i][1] == 'w') {
      pitch = 2*strtod(&(argv[i][2]), chkp);
      if(*chkp == &(argv[i][2]) || pitch <= 0.0) {
        fprintf(stderr, "%s: bad wire width `%s'\n", 
                argv[0], &argv[i][2]);
        cmderr = true;
        break;
      }
    }
    else if(argv[i][1] == 'e') {
      edgefrac = strtod(&(argv[i][2]), chkp);
      if(*chkp == &(argv[i][2]) || edgefrac < 0.0) {
        fprintf(stderr, "%s: bad edge panel fraction `%s'\n", 
                argv[0], &argv[i][2]);
        cmderr = true;
        break;
      }
    }
    else if(argv[i][1] == 'x' && argv[i][2] == 'o') {
      x0 = strtod(&(argv[i][3]), chkp);
      if(*chkp == &(argv[i][3])) {
        fprintf(stderr, "%s: bad x origin value `%s'\n", 
                argv[0], &argv[i][3]);
        cmderr = true;
        break;
      }
    }
    else if(argv[i][1] == 'y' && argv[i][2] == 'o') {
      y0 = strtod(&(argv[i][3]), chkp);
      if(*chkp == &(argv[i][3])) {
        fprintf(stderr, "%s: bad y origin value `%s'\n", 
                argv[0], &argv[i][3]);
        cmderr = true;
        break;
      }
    }
    else if(argv[i][1] == 'z' && argv[i][2] == 'o') {
      z0 = strtod(&(argv[i][3]), chkp);
      if(*chkp == &(argv[i][3])) {
        fprintf(stderr, "%s: bad z origin value `%s'\n", 
                argv[0], &argv[i][3]);
        cmderr = true;
        break;
      }
    }
    else if(argv[i][1] == 'd') no_disc = true;
/*    else if(argv[i][1] == 'k') do_dielec = false; */
    else {
      fprintf(stderr, "%s: illegal option -- %s\n", argv[0], &(argv[i][1]));
      cmderr = true;
      break;
    }
  }

  if(cmderr == true) {
    fprintf(stderr,
            "Usage: %s [-xo<originx>] [-yo<originy>] [-zo<originz>]\n              [-c<conductors/bus>] [-w<wire width>]\n              [-n<num panels/wire width>] [-e<rel edge panel width>]\n              [-na<name base>] [-d]\n", 
            argv[0]);
    fprintf(stderr, "DEFAULT VALUES:\n");
    fprintf(stderr, "  origin = (xo yo zo) = (%g %g %g)\n", X0, Y0, Z0);
    fprintf(stderr, "  conductors/bus = %d\n", DEFNWI);
    fprintf(stderr, "  wire width = %g\n", DEFWID);
    fprintf(stderr, "  num panels/wire width = %d\n", DEFNCL);
    fprintf(stderr, "  rel edge panel width = %g\n", DEFEFR);
    fprintf(stderr, "  conductor name base = <none> (wires get numbered)\n");
    fprintf(stderr, "OPTIONS:\n");
    fprintf(stderr, "  -d = don't discretize faces\n");
    exit(0);
  }

  /* open output file */
  fp = stdout;

  /* write title */
  fprintf(fp, "0 %dX%d bus crossing problem with %.3gm wires (n=%d e=%.3g)\n",
          wires, wires, pitch/2.0, ncells, edgefrac);
  fprintf(fp, "* origin = (%g %g %g)\n", x0, y0, z0);
  if(!do_cond) fprintf(fp, "* conductor panels omitted\n");
  if(!do_dielec) fprintf(fp, "* dielectric panels omitted\n");

  /* set up lower bars in bus crossing geometry */
  x1 = pitch/2.0; y1 = 0.0; z1 = pitch/2.0; /* setup first c1 */
  y2 = (pitch/2.0)*(2*wires+1); z2 = pitch/2.0; /* constant parts of c2 */
  y3 = 0.0; z3 = pitch/2.0;     /* constant parts of c3 */
  y4 = z4 = 0.0;                /* constant parts of c4 */
  for(cond = 1; cond <= wires; cond++) {
    x2 = x4 = x1; x3 = x1 + pitch/2.0; 
    npanels += disBar(fp, cond, edgefrac, ncells, wires, do_cond, do_dielec,
                      no_disc, x0+x1, y0+y1, z0+z1, x0+x2, y0+y2, z0+z2, 
                      x0+x3, y0+y3, z0+z3, x0+x4, y0+y4, z0+z4);
    x1 += pitch;
  }

  if(do_cond) {
    /* setup upper bars (step in y rather than x) */
    x1 = 0.0; y1 = pitch/2.0; z1 = pitch; /* setup first c1 */
    x3 = (pitch/2.0)*(2*wires+1); z3 = pitch;   /* constant parts of c3 */
    x2 = 0.0; z2 = pitch;       /* constant parts of c2 */
    x4 = 0.0; z4 = 1.5*pitch;           /* constant parts of c4 */
    for(; cond <= 2*wires; cond++) {
      y3 = y4 = y1; y2 = y1 + pitch/2.0; 
      npanels += disBar(fp, cond, edgefrac, ncells, wires, true, true, no_disc,
                        x0+x1, y0+y1, z0+z1, x0+x2, y0+y2, z0+z2, 
                        x0+x3, y0+y3, z0+z3, x0+x4, y0+y4, z0+z4);
      y1 += pitch;
    }
  }
  
  if(name_given) {
    for(int i = 1; i <= wires; i++) {
      fprintf(fp, "N %d %s%d\n", i, name, i);
      fprintf(fp, "N %d %s%d\n", wires+i, name, wires+i);
    }
  }
}
