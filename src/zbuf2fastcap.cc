
#include "mulGlobal.h"
#include "mulStruct.h"
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
void dump_ps_geometry(ssystem *sys, const char *filename, charge *chglist, double *q, int use_ttl_chg)
{
  int numlines = 0, numfaces = 0, use_density;
  face **faces = NULL, **sfaces;
  double normal[3], rhs;
  double *avg, radius;
  line **lines;
  FILE *fp;
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
    sys->info(" %d faces read\n", numfaces);
    sys->info(" %d lines read\n", numlines);
    sys->info(" average obj point: (%g %g %g), radius = %g\n",
            avg[0], avg[1], avg[2], radius);
    sys->info(" absolute view point: (%g %g %g)\n",
            sys->view[0],sys->view[1],sys->view[2]);
  }

  /* set up all the normals and rhs for the faces (needed for sort) */
  initFaces(faces, numfaces, sys->view);

  /* set up the adjacency graph for the depth sort */
  sys->msg("\nSorting %d faces for %s ...", numfaces, filename);
  sys->flush();
  getAdjGraph(sys, faces, numfaces, sys->view, rhs, normal);
  sys->msg("done.\n");

  /* depth sort the faces */
  /*sys->info("Starting depth sort...");*/
  sfaces = depthSortFaces(sys, faces, numfaces);
  /*sys->info("done.\n");*/

  /* get the 2d figure and dump to ps file */
  image(sys, sfaces, numfaces, lines, numlines, normal, rhs, sys->view);
  flatten(sys, sfaces, numfaces, lines, numlines, rhs, sys->rotation, normal, sys->view);
  makePos(sys, sfaces, numfaces, lines, numlines);
  scale2d(sys, sfaces, numfaces, lines, numlines, sys->scale, sys->moffset);
  if(sys->g_) {
    dumpCycles(sys, sfaces, numfaces); /* DANGER - doesnt work (?) */
    dumpFaceText(sys, sfaces, numfaces);
  }
  else {
    if((fp = fopen(filename, "w")) == NULL) {
      sys->error("dump_ps_geometry: can't open\n `%s'\nto write", filename);
    }
    sys->msg("Writing %s ...", filename);
    dumpPs(sys, sfaces, numfaces, lines, numlines, fp, sys->argv, sys->argc, use_density, black, white);
    sys->msg("done.\n");
    fclose(fp);
  }
}
