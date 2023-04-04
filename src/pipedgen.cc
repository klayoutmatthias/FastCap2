
#include <cstdio>
#include <cmath>
#include <cstring>
#include <algorithm>

#include "disrect.h"

const double DEFSID = 1.0;              /* default cube side, meters */
const double DEFEFR = 0.1;              /* default edge-cell-width/inner-cell-width */
const int DEFNCL = 3;                   /* default #cells on short side of faces */
const int DEFPLT = 2;                   /* default number of || plates */

const double X0 = 0.0;
const double Y0 = 0.0;
const double Z0 = 0.0;
const double XH = 1.0;
const double YH = 1.0;
const double ZH = 1.0;

/*
  generates a parallelipiped example in quickif.c format
  - uses disRect() for discretization plates
*/
int main(int argc, char *argv[])
{
  char name[BUFSIZ] = { 0 };
  char **chkp = 0;
  char *chk = 0;
  int npanels = 0;
  int ncells = DEFNCL;
  int right_cells = 0;
  int left_cells = 0;
  int top_cells = 0;
  bool no_bottom = false;
  bool no_top = false;
  bool no_perimeter = false;
  bool no_discr = false;
  bool top_cells_given = false;
  bool no_perimeter_front_left = false;
  bool no_perimeter_front_right = false;
  bool no_perimeter_back_left = false;
  bool no_perimeter_back_right = false;
  bool name_given = false;
  double edgefrac = DEFEFR;
  double xr = X0,    yr = Y0,    zr = Z0;
  double x1 = X0+XH, y1 = Y0,    z1 = Z0;
  double x2 = X0,    y2 = Y0+YH, z2 = Z0;
  double x3 = X0,    y3 = Y0,    z3 = Z0+YH;
  bool cmderr = false;
  FILE *fp = NULL;

  /* parse command line */
  chkp = &chk;                  /* pointers for error checking */
  for(int i = 1; i < argc && cmderr == false; i++) {
    if(argv[i][0] != '-') {
      fprintf(stderr, "%s: illegal argument -- %s\n", argv[0], argv[i]);
      cmderr = true;
      break;
    }
    else if(!strcmp(argv[i], "-cr")) {
      if(i+4>argc) {
        fprintf(stderr, "%s: not enough coordinate values given for %s", argv[0], argv[i]);
        cmderr = true;
      }
      xr = strtod(argv[i+1], chkp);
      if(*chkp == argv[i+1]) {
        fprintf(stderr, "%s: bad reference corner x coordinate `%s'\n",
                argv[0], argv[i+1]);
        cmderr = true;
        break;
      }
      yr = strtod(argv[i+2], chkp);
      if(*chkp == argv[i+2]) {
        fprintf(stderr, "%s: bad reference corner y coordinate `%s'\n",
                argv[0], argv[i+2]);
        cmderr = true;
        break;
      }
      zr = strtod(argv[i+3], chkp);
      if(*chkp == argv[i+3]) {
        fprintf(stderr, "%s: bad reference corner z coordinate `%s'\n",
                argv[0], argv[i+3]);
        cmderr = true;
        break;
      }
      i = i + 3;
    }
    else if(!strcmp(argv[i], "-c1")) {
      if(i+4>argc) {
        fprintf(stderr, "%s: not enough coordinate values given for %s", argv[0], argv[i]);
        cmderr = true;
      }
      if(sscanf(argv[i+1], "%lf", &x1) != 1) {
        fprintf(stderr, "%s: bad first corner x coordinate `%s'\n",
                argv[0], argv[i+1]);
        cmderr = true;
        break;
      }
      if(sscanf(argv[i+2], "%lf", &y1) != 1) {
        fprintf(stderr, "%s: bad first corner y coordinate `%s'\n",
                argv[0], argv[i+2]);
        cmderr = true;
        break;
      }
      if(sscanf(argv[i+3], "%lf", &z1) != 1) {
        fprintf(stderr, "%s: bad first corner z coordinate `%s'\n",
                argv[0], argv[i+3]);
        cmderr = true;
        break;
      }
      i = i + 3;
    }
    else if(!strcmp(argv[i], "-c2")) {
      if(i+4>argc) {
        fprintf(stderr, "%s: not enough coordinate values given for %s", argv[0], argv[i]);
        cmderr = true;
      }
      if(sscanf(argv[i+1], "%lf", &x2) != 1) {
        fprintf(stderr, "%s: bad second corner x coordinate `%s'\n",
                argv[0], argv[i+1]);
        cmderr = true;
        break;
      }
      if(sscanf(argv[i+2], "%lf", &y2) != 1) {
        fprintf(stderr, "%s: bad second corner y coordinate `%s'\n",
                argv[0], argv[i+2]);
        cmderr = true;
        break;
      }
      if(sscanf(argv[i+3], "%lf", &z2) != 1) {
        fprintf(stderr, "%s: bad second corner z coordinate `%s'\n",
                argv[0], argv[i+3]);
        cmderr = true;
        break;
      }
      i = i + 3;
    }
    else if(!strcmp(argv[i], "-c3")) {
      if(i+4>argc) {
        fprintf(stderr, "%s: not enough coordinate values given for %s", argv[0], argv[i]);
        cmderr = true;
      }
      if(sscanf(argv[i+1], "%lf", &x3) != 1) {
        fprintf(stderr, "%s: bad third corner x coordinate `%s'\n",
                argv[0], argv[i+1]);
        cmderr = true;
        break;
      }
      if(sscanf(argv[i+2], "%lf", &y3) != 1) {
        fprintf(stderr, "%s: bad third corner y coordinate `%s'\n",
                argv[0], argv[i+2]);
        cmderr = true;
        break;
      }
      if(sscanf(argv[i+3], "%lf", &z3) != 1) {
        fprintf(stderr, "%s: bad third corner z coordinate `%s'\n",
                argv[0], argv[i+3]);
        cmderr = true;
        break;
      }
      i = i + 3;
    }
    else if(argv[i][1] == 'n' && argv[i][2] == 'a') {
      if(sscanf(&(argv[i][3]), "%s", name) != 1) {
        fprintf(stderr, "%s: bad name `%s'\n", argv[0], &argv[i][3]);
        cmderr = true;
        break;
      }
      name_given = true;
    }
    else if(argv[i][1] == 'n' && argv[i][2] == 't') {
      top_cells = (int) strtol(&(argv[i][3]), chkp, 10);
      if(*chkp == &(argv[i][3]) || top_cells < 1) {
        fprintf(stderr,
                "%s: bad number of panels/top `%s'\n",
                argv[0], &argv[i][3]);
        cmderr = true;
        break;
      }
      top_cells_given = true;
    }
    else if(argv[i][1] == 'n') {
      ncells = (int) strtol(&(argv[i][2]), chkp, 10);
      if(*chkp == &(argv[i][2]) || ncells < 1) {
        fprintf(stderr, 
                "%s: bad number of panels/side `%s'\n", 
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
    else if(argv[i][1] == 't') {
      no_top = true;
    }
    else if(!strcmp(&(argv[i][1]),"pfl")) {
      no_perimeter_front_left = true;
    }
    else if(!strcmp(&(argv[i][1]),"pfr")) {
      no_perimeter_front_right = true;
    }
    else if(!strcmp(&(argv[i][1]),"pbl")) {
      no_perimeter_back_left = true;
    }
    else if(!strcmp(&(argv[i][1]),"pbr")) {
      no_perimeter_back_right = true;
    }
    else if(!strcmp(&(argv[i][1]),"p")) {
      no_perimeter = true;
    }
    else if(argv[i][1] == 'b') {
      no_bottom = true;
    }
    else if(argv[i][1] == 'd') {
      no_discr = true;
    }
    else {
      fprintf(stderr, "%s: illegal option -- %s\n", argv[0], &(argv[i][1]));
      cmderr = true;
      break;
    }
  }      

  if(cmderr == true) {
    fprintf(stderr,
            "Usage: %s [-cr <x y z>] [-c1 <x y z>] [-c2 <x y z>] [-c3 <x y z>] \n"
            "                [-n<num panels/side>] [-nt<num panels/top>] [-e<rel edge panel width>] \n"
            "                [-na<name>] [-t] [-b] [-p] [-pfl] [-pfr] [-pbl] [-pbr] [-d]\n",
            argv[0]);
    fprintf(stdout, "DEFAULT VALUES:\n");
    fprintf(stdout, "  -cr reference corner = (%g %g %g)\n", X0, Y0, Z0);
    fprintf(stdout, "  -c1 corner 1 = (%g %g %g)\n", X0+XH, Y0, Z0);
    fprintf(stdout, "  -c2 corner 2 = (%g %g %g)\n", X0, Y0+YH, Z0);
    fprintf(stdout, "  -c3 corner 3 = (%g %g %g)\n", X0, Y0, Z0+ZH);
    fprintf(stdout, "  num panels/side = %d\n", DEFNCL);
    fprintf(stdout, "  rel edge panel width = %g\n", DEFEFR);
    fprintf(stdout, "  conductor name = `1'\n");
    fprintf(stdout, "OPTIONS:\n");
    fprintf(stdout, "  -t don't include top (c1-cr-c2 plane) face\n");
    fprintf(stdout, "  -b don't include bottom (|| top) face\n");
    fprintf(stdout, "  -p don't include any perimeter (side) faces\n");
    fprintf(stdout, 
         "  -pfl don't include perimeter front left (c1-cr-c3 plane) face\n");
    fprintf(stdout, 
         "  -pfr don't include perimeter front right (c2-cr-c3 plane) face\n");
    fprintf(stdout, 
         "  -pbl don't include perimeter back left (|| front right) face\n");
    fprintf(stdout, 
         "  -pbr don't include perimeter back right (|| front left) face\n");
    fprintf(stdout, "  -d don't discretize faces\n");
    exit(0);
  }

  /* set up number of cells on top and bottom */
  if(!top_cells_given) top_cells = ncells;

  /* open output file */
  fp = stdout;

  /* write title */
  fprintf(fp, "0 parallelepiped, ref corner (%g %g %g)\n", xr, yr, zr);
  fprintf(fp, "* other corners: (%g %g %g) (%g %g %g) (%g %g %g)\n",
          x1, y1, z1, x2, y2, z2, x3, y3, z3);
  if(no_top) fprintf(fp, "* top panel omitted\n");
  if(no_bottom) fprintf(fp, "* bottom panel omitted\n");
  if(no_perimeter) fprintf(fp, "* all side panels omitted\n");
  else {
    if(no_perimeter_front_left)
        fprintf(fp, "* front left side panel omitted\n");
    if(no_perimeter_front_right)
        fprintf(fp, "* front right side panel omitted\n");
    if(no_perimeter_back_left)
        fprintf(fp, "* back left side panel omitted\n");
    if(no_perimeter_back_right)
        fprintf(fp, "* back right side panel omitted\n");
  }

  left_cells = right_cells = ncells; /* in case want to set one rel to other */

  /* write panels with outward pointing normals */
  fprintf(fp, "* view looking at cr; c1 to left, c2 to right, c3 down\n");

  if(!no_perimeter) {
    if(!no_perimeter_front_left) {
      fprintf(fp, "* front left\n");
      npanels += disRect(fp, 1, edgefrac, left_cells, no_discr,
                         xr, yr, zr,
                         x1, y1, z1,
                         x1+(x3-xr), y1+(y3-yr), z1+(z3-zr),
                         x3, y3, z3);
    }

    if(!no_perimeter_front_right) {
      fprintf(fp, "* front right\n");
      npanels += disRect(fp, 1, edgefrac, right_cells, no_discr,
                         xr, yr, zr,
                         x3, y3, z3,
                         x2+(x3-xr), y2+(y3-yr), z2+(z3-zr),
                         x2, y2, z2);
    }

    if(!no_perimeter_back_left) {
      fprintf(fp, "* back left\n");
      npanels += disRect(fp, 1, edgefrac, right_cells, no_discr,
                         x1, y1, z1,
                         x1+(x2-xr), y1+(y2-yr), z1+(z2-zr),
                         x1+(x2-xr)+(x3-xr), y1+(y2-yr)+(y3-yr), 
                         z1+(z2-zr)+(z3-zr),
                         x1+(x3-xr), y1+(y3-yr), z1+(z3-zr));
    }

    if(!no_perimeter_back_right) {
      fprintf(fp, "* back right\n");
      npanels += disRect(fp, 1, edgefrac, left_cells, no_discr,
                         x2, y2, z2,
                         x2+(x3-xr), y2+(y3-yr), z2+(z3-zr),
                         x1+(x2-xr)+(x3-xr), y1+(y2-yr)+(y3-yr), 
                         z1+(z2-zr)+(z3-zr),
                         x1+(x2-xr), y1+(y2-yr), z1+(z2-zr));
    }
  }

  if(!no_bottom) {
    fprintf(fp, "* bottom\n");
    npanels += disRect(fp, 1, edgefrac, top_cells, no_discr,
                       x3, y3, z3,
                       x1+(x3-xr), y1+(y3-yr), z1+(z3-zr),
                       x1+(x2-xr)+(x3-xr), y1+(y2-yr)+(y3-yr), 
                       z1+(z2-zr)+(z3-zr),
                       x2+(x3-xr), y2+(y3-yr), z2+(z3-zr));
  }

  if(!no_top) {
    fprintf(fp, "* top\n");
    npanels += disRect(fp, 1, edgefrac, top_cells, no_discr,
                       xr, yr, zr,
                       x2, y2, z2,
                       x1+(x2-xr), y1+(y2-yr), z1+(z2-zr),
                       x1, y1, z1);
  }

  if(name_given) fprintf(fp, "N 1 %s\n", name);

}
