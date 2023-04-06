
#include <cstdio>
#include <cmath>

#include "disrect.h"

const double DEFWID = 1.0;             /* default plate width, meters */
const double DEFEFR = 1E-1;            /* default edge-cell-width/inner-cell-width */
const int DEFNCL = 3;                  /* default #cells on short side of faces */
const double DEFSEP = 1E-1;            /* default plate separation, meters */
const int DEFPLT = 2;                  /* default number of || plates */

/*
  generates a parallel (square) plate capacitor example in quickif.c format
  - uses disRect() for discretization plates
*/
int main(int argc, char *argv[])
{
  char **chkp = 0;
  char *chk = 0;
  char name[BUFSIZ];
  int npanels = 0;
  int ncells = 0;
  bool cmderr = false;
  int numplt = 0;
  bool name_given = false;
  bool align_on_x = false;
  bool no_disc = false;
  double edgefrac = 0.0;
  double width = 0.0;
  double sep = 0.0;
  FILE *fp = NULL;

  /* load default parameters */
  width = DEFWID;
  edgefrac = DEFEFR;
  ncells = DEFNCL;
  sep = DEFSEP;
  numplt = DEFPLT;

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
                "%s: bad number of panels/plate side `%s'\n", 
                argv[0], &argv[i][2]);
        cmderr = true;
        break;
      }
    }
    else if(argv[i][1] == 'p') {
      numplt = (int) strtol(&(argv[i][2]), chkp, 10);
      if(*chkp == &(argv[i][2]) || numplt < 1) {
        fprintf(stderr, 
                "%s: bad number of parallel plates `%s'\n", 
                argv[0], &argv[i][2]);
        cmderr = true;
        break;
      }
    }
    else if(argv[i][1] == 's') {
      sep = strtod(&(argv[i][2]), chkp);
      if(*chkp == &(argv[i][2]) || sep <= 0.0) {
        fprintf(stderr, "%s: bad plate separation `%s'\n", 
                argv[0], &argv[i][2]);
        cmderr = true;
        break;
      }
    }
    else if(argv[i][1] == 'w') {
      width = strtod(&(argv[i][2]), chkp);
      if(*chkp == &(argv[i][2]) || width <= 0.0) {
        fprintf(stderr, "%s: bad plate width `%s'\n", 
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
    else if(argv[i][1] == 'x') {
      align_on_x = true;
    }
    else if(argv[i][1] == 'd') {
      no_disc = true;
    }
    else {
      fprintf(stderr, "%s: illegal option -- %s\n", argv[0], &(argv[i][1]));
      cmderr = true;
      break;
    }
  }

  if(cmderr == true) {
    fprintf(stderr,
            "Usage: %s [-s<plate sep>] [-w<plate width>] [-p<num plates>]\n              [-n<num panels/plate width>] [-e<rel edge panel width>]\n              [-na<cond name base>] [-d]\n", argv[0]);
    fprintf(stderr, "DEFAULT VALUES:\n");
    fprintf(stderr, "  plate sep = %g\n", DEFSEP);
    fprintf(stderr, "  plate width = %g\n", DEFWID);
    fprintf(stderr, "  num plates = %d\n", DEFPLT);
    fprintf(stderr, "  num panels/plate width = %d\n", DEFNCL);
    fprintf(stderr, "  rel edge panel width = %g\n", DEFEFR);
    fprintf(stderr, "  conductor name base = <none>\n");
    fprintf(stderr, "OPTIONS:\n");
    fprintf(stderr, "  -d = do not discretize faces\n");
    exit(0);
  }

  /* open output file */
  fp = stdout;

  /* write title */
  if(numplt > 1)
      fprintf(fp, 
              "0 %gmX%gm %d || plate capacitor with %gm separation (n=%d e=%.3g)\n",
              width, width, numplt, sep, ncells, edgefrac);
  else 
      fprintf(fp, 
              "0 %gmX%gm single plate capacitor with %gm separation (n=%d e=%.3g)\n",
              width, width, sep, ncells, edgefrac);


  /* write panels */
  if(align_on_x) {
    if(numplt % 2 != 0) {       /* odd number of plates */
      for(int i = -(numplt / 2); i <= numplt / 2; i++) {
        npanels += disRect(fp, i+numplt, edgefrac, ncells, no_disc,
                           sep*(double)i, width/2.0, width/2.0,
                           sep*(double)i, -width/2.0, width/2.0,
                           sep*(double)i, -width/2.0, -width/2.0,
                           sep*(double)i, width/2.0, -width/2.0);
      }
    }
    else {
      for(int i = 1, cond = 1; i < numplt; i += 2) {
        npanels += disRect(fp, cond++, edgefrac, ncells, no_disc,
                           -sep*(double)i/2, width/2.0, width/2.0,
                           -sep*(double)i/2, -width/2.0, width/2.0,
                           -sep*(double)i/2, -width/2.0, -width/2.0,
                           -sep*(double)i/2, width/2.0, -width/2.0);
        npanels += disRect(fp, cond++, edgefrac, ncells, no_disc,
                           sep*(double)i/2, width/2.0, width/2.0,
                           sep*(double)i/2, -width/2.0, width/2.0,
                           sep*(double)i/2, -width/2.0, -width/2.0,
                           sep*(double)i/2, width/2.0, -width/2.0);
      }
    }
  }
  else {
    for(int i = 0; i < numplt; i++) {
      npanels += disRect(fp, i+1, edgefrac, ncells, no_disc,
                         0.0, 0.0, sep*(double)i, 
                         0.0, width, sep*(double)i, 
                         width, width, sep*(double)i, 
                         width, 0.0, sep*(double)i);
    }
  }

  if(name_given) {
    for(int i = 1; i <= numplt; i++) fprintf(fp, "N %d %s%d\n", i, name, i);
  }

}
