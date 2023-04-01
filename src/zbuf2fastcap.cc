/*!\page LICENSE LICENSE
 
Copyright (C) 2003 by the Board of Trustees of Massachusetts Institute of Technology, hereafter designated as the Copyright Owners.
 
License to use, copy, modify, sell and/or distribute this software and
its documentation for any purpose is hereby granted without royalty,
subject to the following terms and conditions:
 
1.  The above copyright notice and this permission notice must
appear in all copies of the software and related documentation.
 
2.  The names of the Copyright Owners may not be used in advertising or
publicity pertaining to distribution of the software without the specific,
prior written permission of the Copyright Owners.
 
3.  THE SOFTWARE IS PROVIDED "AS-IS" AND THE COPYRIGHT OWNERS MAKE NO
REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED, BY WAY OF EXAMPLE, BUT NOT
LIMITATION.  THE COPYRIGHT OWNERS MAKE NO REPRESENTATIONS OR WARRANTIES OF
MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE OR THAT THE USE OF THE
SOFTWARE WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS TRADEMARKS OR OTHER
RIGHTS. THE COPYRIGHT OWNERS SHALL NOT BE LIABLE FOR ANY LIABILITY OR DAMAGES
WITH RESPECT TO ANY CLAIM BY LICENSEE OR ANY THIRD PARTY ON ACCOUNT OF, OR
ARISING FROM THE LICENSE, OR ANY SUBLICENSE OR USE OF THE SOFTWARE OR ANY
SERVICE OR SUPPORT.
 
LICENSEE shall indemnify, hold harmless and defend the Copyright Owners and
their trustees, officers, employees, students and agents against any and all
claims arising out of the exercise of any rights under this Agreement,
including, without limiting the generality of the foregoing, against any
damages, losses or liabilities whatsoever with respect to death or injury to
person or damage to property arising from or out of the possession, use, or
operation of Software or Licensed Program(s) by LICENSEE or its customers.
 
*/

#include "mulGlobal.h"
#include "zbufGlobal.h"
#include "zbuf2fastcap.h"
#include "zbufProj.h"
#include "zbufSort.h"
#include "zbufInOut.h"

/*
  main interface between old zbuf code and fastcap
  - replaces functionality of zbuf.c (main())
  - dumps a geometry in .ps file format
  - panels are shaded using the vector entries of q; 
    q = NULL => no shading; use_density = TRUE => divide q's by areas
  - file name used is <ps_file_base><iter>.ps; ps_file_base is either
    the list file base, the input file base or "stdin" (see get_ps_file_info())
*/
void dump_ps_geometry(ssystem *sys, charge *chglist, double *q, int cond, int use_ttl_chg)
{
  int numlines, numfaces, use_density;
  face **faces, **sfaces;
  double normal[3], rhs;
  double *avg, radius;
  line **lines;
  FILE *fp;
  char str[BUFSIZ];
  double black = 0.0, white = 0.0;

  /* set up use density flag---not too clean; saves changes in called funcs */
  if(use_ttl_chg) use_density = FALSE;
  else use_density = TRUE;

  /* convert fastcap structs to zbuf structs - COULD ELIMINATE THIS */
  faces = fastcap2faces(sys, &numfaces, chglist, q, use_density, &black, &white);
  
  /* get .fig format lines in file specified with -b option */
  lines = getLines(sys, sys->line_file, &numlines);

  /* figure the cntr of extremal (average) coordinates of all points */
  avg = getAvg(sys, faces, numfaces, lines, numlines, OFF);

  /* get the radius of the smallest sphere enclosing all the lines */
  radius = getSphere(sys, avg, faces, numfaces, lines, numlines);

  /* get normal to image plane, adjust view point to be (1+distance)*radius 
     away from object center point avg, view plane (1+distance/2)*radius away
     - view coordinates taken rel to obj center but changed to absolute */
  sys->view[0] = sys->azimuth; sys->view[1] = sys->elevation;
  rhs = getNormal(sys, normal, radius, avg, sys->view, sys->distance);

  if (false) {
    fprintf(stderr, " %d faces read\n", numfaces);
    fprintf(stderr, " %d lines read\n", numlines);
    fprintf(stderr, " average obj point: (%g %g %g), radius = %g\n",
            avg[0], avg[1], avg[2], radius);
    fprintf(stderr, " absolute view point: (%g %g %g)\n",
            sys->view[0],sys->view[1],sys->view[2]);
  }

  /* set up all the normals and rhs for the faces (needed for sort) */
  initFaces(faces, numfaces, sys->view);

  /* set up ps file name */
  if(q == NULL) sprintf(str, "%s.ps", sys->ps_file_base);
  else sprintf(str, "%s%d.ps", sys->ps_file_base, cond);

  /* set up the adjacency graph for the depth sort */
  fprintf(stdout, "\nSorting %d faces for %s...", numfaces, str);
  fflush(stdout);
  getAdjGraph(sys, faces, numfaces, sys->view, rhs, normal);
  fprintf(stdout, "done.\n");

  /* depth sort the faces */
  /*fprintf(stderr, "Starting depth sort...");*/
  sfaces = depthSortFaces(sys, faces, numfaces);
  /*fprintf(stderr, "done.\n");*/

  /* get the 2d figure and dump to ps file */
  image(sys, sfaces, numfaces, lines, numlines, normal, rhs, sys->view);
  flatten(sys, sfaces, numfaces, lines, numlines, rhs, sys->rotation, normal, sys->view);
  makePos(sys, sfaces, numfaces, lines, numlines);
  scale2d(sys, sfaces, numfaces, lines, numlines, sys->scale, sys->moffset);
  if(sys->g_) {
    dumpCycles(sfaces, numfaces, stdout); /* DANGER - doesnt work (?) */
    dumpFaceText(sfaces, numfaces, stdout);
  }
  else {
    if((fp = fopen(str, "w")) == NULL) {
      fprintf(stderr, "dump_ps_geometry: can't open\n `%s'\nto write\n", str);
      exit(0);
    }
    fprintf(stdout, "Writing %s...", str);
    dumpPs(sys, sfaces, numfaces, lines, numlines, fp, sys->argv, sys->argc, use_density, black, white);
    fprintf(stdout, "done.\n");
    fclose(fp);
  }
}
