
#include <cstdio>
#include <cmath>
#include <cstring>

#include "disrect.h"
#include "distri.h"

const double DEFSID = 1.0;             /* default side and height, meters */
const double DEFEFR = 0.1;             /* default edge-cell-width/inner-cell-width */
const int DEFNCL = 3;                  /* default #cells on short side of faces */

const double X0 = 0.0;
const double Y0 = 0.0;
const double Z0 = 0.0;
const double XH = 1.0;
const double YH = 1.0;
const double ZH = 1.0;

/*
  generates a pyramid example in quickif.c format
  - uses disRect() and discTri() for discretization of plates
*/
int main(int argc, char *argv[])
{
  char name[BUFSIZ] = { 0 };
  char **chkp = 0;
  char *chk = 0;
  bool center_on_origin = false;
  int npanels = 0;
  int ncells = DEFNCL;
  int right_cells = 0;
  int left_cells = 0;
  int top_cells = 0;
  bool no_bottom = false;
  bool no_perimeter = false;
  bool no_discr = false;
  bool no_perimeter_front_left = false;
  bool no_perimeter_front_right = false;
  bool no_perimeter_back_left = false;
  bool no_perimeter_back_right = false;
  bool name_given = false;
  double edgefrac = DEFEFR;
  double x1, y1, z1, x2, y2, z2, x3, y3, z3, x0, y0, z0; /* 4 corners */
  double xh, yh, zh, x4, y4, z4;
  bool cmderr = false;
  FILE *fp = NULL;

  /* load default parameters */
  x0 = X0; y0 = Y0; z0 = Z0;
  xh = XH; yh = YH; zh = ZH;

  /* parse command line */
  chkp = &chk;                  /* pointers for error checking */
  for(int i = 1; i < argc && cmderr == false; i++) {
    if(argv[i][0] != '-') {
      fprintf(stderr, "%s: illegal argument -- %s\n", argv[0], argv[i]);
      cmderr = true;
      break;
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
    else if(argv[i][1] == 'x' && argv[i][2] == 'h') {
      xh = strtod(&(argv[i][3]), chkp);
      if(*chkp == &(argv[i][3])) {
        fprintf(stderr, "%s: bad x height value `%s'\n", 
                argv[0], &argv[i][3]);
        cmderr = true;
        break;
      }
    }
    else if(argv[i][1] == 'y' && argv[i][2] == 'h') {
      yh = strtod(&(argv[i][3]), chkp);
      if(*chkp == &(argv[i][3])) {
        fprintf(stderr, "%s: bad y height value `%s'\n", 
                argv[0], &argv[i][3]);
        cmderr = true;
        break;
      }
    }
    else if(argv[i][1] == 'z' && argv[i][2] == 'h') {
      zh = strtod(&(argv[i][3]), chkp);
      if(*chkp == &(argv[i][3])) {
        fprintf(stderr, "%s: bad z height value `%s'\n", 
                argv[0], &argv[i][3]);
        cmderr = true;
        break;
      }
    }
    else if(!strcmp(&(argv[i][1]), "pfl")) {
      no_perimeter_front_left = true;
    }
    else if(!strcmp(&(argv[i][1]), "pfr")) {
      no_perimeter_front_right = true;
    }
    else if(!strcmp(&(argv[i][1]), "pbl")) {
      no_perimeter_back_left = true;
    }
    else if(!strcmp(&(argv[i][1]), "pbr")) {
      no_perimeter_back_right = true;
    }
    else if(!strcmp(&(argv[i][i]), "p")
) {
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
            "Usage: %s [-xo<originx>] [-yo<originy>] [-zo<originz>] \n               [-xh<heightx>] [-yh<heighty>] [-zh<heightz>] \n               [-n<num panels/side>] [-e<rel edge panel width>] \n               [-na<name>] [-b] [-p] [-pfl] [-pfr] [-pbl] [-pbr] [-d]\n", 
            argv[0]);
    fprintf(stdout, "DEFAULT VALUES:\n");
    fprintf(stderr, "  origin = (xo yo zo) = (%g %g %g)\n", X0, Y0, Z0);
    fprintf(stderr, "  side heights = (xh yh zh) = (%g %g %g)\n", XH, YH, ZH);
    fprintf(stdout, "  num panels/side = %d\n", DEFNCL);
    fprintf(stdout, "  rel edge panel width = %g\n", DEFEFR);
    fprintf(stdout, "  conductor name = `1'\n");
    fprintf(stdout, "OPTIONS:\n");
    fprintf(stdout, "  -b don't include bottom (rectangular) face\n");
    fprintf(stdout, "  -p don't include any perimeter (side) faces\n");
    fprintf(stdout, "  -pfl don't include perimeter front left face (view from positive orthant)\n");
    fprintf(stdout, "  -pfr don't include perimeter front right face\n");
    fprintf(stdout, "  -pbl don't include perimeter back left face\n");
    fprintf(stdout, "  -pbr don't include perimeter back right face\n");
    fprintf(stdout, "  -d don't discretize faces\n");
    exit(0);
  }

  /* open output file */
  fp = stdout;

  /* write title */
  if(center_on_origin) {
    fprintf(fp, 
            "0 %gmX%gmX%gm pyramid centered on origin (n=%d e=%.3g)\n",
            xh, yh, zh, ncells, edgefrac);
    x0 = -xh/2.0; y0 = -yh/2.0; z0 = -zh/2.0;
  }
  else {
    fprintf(fp, 
            "0 %gmX%gmX%gm pyramid (n=%d e=%.3g)\n",
            xh, yh, zh, ncells, edgefrac);
    fprintf(fp, "* xo = %g, yo = %g, zo = %g\n", x0, y0, z0);
  }
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

  /* set up corners */
  x1 = x0 + xh; y1 = y0; z1 = z0;
  x2 = x0; y2 = y0 + yh; z2 = z0;
  x3 = x0 + xh; y3 = y0 + yh; z3 = z0;
  x4 = x0 + xh/2.0; y4 = y0 + yh/2.0; z4 = z0 + zh;

  left_cells = right_cells = ncells; /* in case want to set one rel to other */
  top_cells = ncells;

  /* write panels with outward pointing normals */
  fprintf(stdout, "* view from -x, -y, +z\n");

  if(!no_perimeter) {
    if(!no_perimeter_front_right) {
      fprintf(stdout, "* front right\n");
      npanels += disTri(fp, 1, edgefrac, right_cells, no_discr,
                        x3, y3, z3,
                        x4, y4, z4,
                        x2, y2, z2);
    }
#if DEBUG == 0
    if(!no_perimeter_front_left) {
      fprintf(stdout, "* front left\n");
      npanels += disTri(fp, 1, edgefrac, left_cells, no_discr,
                        x1, y1, z1,
                        x4, y4, z4,
                        x3, y3, z3);
    }

    if(!no_perimeter_back_left) {
      fprintf(stdout, "* back left\n");
#if DEBUG == 1
      npanels += disTri(fp, 1, edgefrac, right_cells, no_discr,
                        x0, y0, z0,
                        x2, y2, z2,
                        x1, y1, z1);
#else
      npanels += disTri(fp, 1, edgefrac, right_cells, no_discr,
                        x0, y0, z0,
                        x4, y4, z4,
                        x1, y1, z1);
#endif
    }

    if(!no_perimeter_back_right) {
      fprintf(stdout, "* back right\n");
      npanels += disTri(fp, 1, edgefrac, left_cells, no_discr,
                        x2, y2, z2,
                        x4, y4, z4,
                        x0, y0, z0);
    }
#endif
  }

  if(!no_bottom) {
    fprintf(stdout, "* bottom\n");
    npanels += disRect(fp, 1, edgefrac, top_cells, no_discr,
                       x0, y0, z0,
                       x2, y2, z2,
                       x3, y3, z3,
                       x1, y1, z1);
  }

  if(name_given) fprintf(fp, "N 1 %s\n", name);

}
