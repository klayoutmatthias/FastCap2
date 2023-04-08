
#include "mulGlobal.h"
#include "mulStruct.h"
#include "input.h"
#include "zbuf2fastcap.h"
#include "mulMulti.h"
#include "mulMats.h"
#include "mulSetup.h"
#include "mulDisplay.h"
#include "calcp.h"
#include "capsolve.h"
#include "psMatDisplay.h"
#include "resusage.h"
#include "counters.h"

#include <cstdlib>
#include <cstring>
#include <stdexcept>

int main_func(int argc, char *argv[]);

int main(int argc, char *argv[])
{
  try {

    return main_func(argc, argv);

  } catch (std::exception &ex) {
    fputs("ERROR: ", stderr);
    fputs(ex.what(), stderr);
    return -1;
  }
}

int main_func(int argc, char *argv[])
{
  ssystem sys;
  sys.argv = (const char **) argv;
  sys.argc = argc;

  int ttliter;
  charge *chglist, *nq;
  double **capmat, dirtimesav, mulsetup, initalltime, ttlsetup, ttlsolve;

  double *trimat = 0, *sqrmat = 0;
  int *real_index = 0;
  int num_dielec_panels = 0;            /* number of dielectric interface panels */
  int num_both_panels = 0;              /* number of thin-cond-on-dielec-i/f panels */
  int num_cond_panels = 0;              /* number of thick conductor panels */
  int up_size = 0;                      /* sum of above three (real panels) */
  int num_dummy_panels = 0;             /* number of off-panel eval pnt panels */
  int eval_size = 0;                    /* sum of above two (total panel structs) */

  char dump_filename[BUFSIZ];
  strcpy(dump_filename, "psmat.ps");

  /* get the list of all panels in the problem */
  /* - many command line parameters having to do with the postscript
       file dumping interface are passed back via globals (see mulGlobal.c) */
  chglist = input_problem(&sys);

  /* if no fastcap run is to be done, just dump the psfile */
  if(sys.capvew && sys.m_) {
    if(!sys.q_) get_ps_file_base(&sys);
    dump_ps_geometry(&sys, chglist, NULL, 0, sys.dd_);
    return 0;
  }

  starttimer;
  mulInit(&sys, chglist);  /* Set up cubes, charges. */
  stoptimer;
  initalltime = dtime;

  sys.msg("\nINPUT SUMMARY\n");

  if (sys.cmddat) {
    sys.msg("  Expansion order: %d\n", sys.order);
    sys.msg("  Number of partitioning levels: %d\n", sys.depth);
    sys.msg("  Overall permittivity factor: %.3g\n", sys.perm_factor);
  }

  /* Figure out number of panels and conductors. */
  eval_size = up_size = num_dummy_panels = num_dielec_panels = 0;
  num_both_panels = num_cond_panels = 0;
  for(nq = chglist; nq != NULL; nq = nq->next) {
    if(nq->dummy) num_dummy_panels++;
    else if(nq->surf->type == CONDTR) {
      num_cond_panels++;
    }
    else if(nq->surf->type == DIELEC) num_dielec_panels++;
    else if(nq->surf->type == BOTH) num_both_panels++;
  }
  up_size = num_cond_panels + num_both_panels + num_dielec_panels;
  eval_size = up_size + num_dummy_panels;

  if (! sys.dissrf) {
    sys.msg("Title: `%s'\n", sys.title ? sys.title : "");
  }
  sys.msg("  Total number of panels: %d\n", up_size);
  sys.msg("    Number of conductor panels: %d\n", num_cond_panels);
  sys.msg("    Number of dielectric interface panels: %d\n",
          num_dielec_panels);
  sys.msg(
          "    Number of thin conductor on dielectric interface panels: %d\n", 
          num_both_panels);
  /*sys.msg("  Number of extra evaluation points: %d\n",
          num_dummy_panels);*/
  sys.msg("  Number of conductors: %d\n", sys.num_cond);

  if (sys.namdat) {
    dumpCondNames(stdout, sys.cond_names);
  }

  if(num_both_panels > 0) {
    sys.error("Thin cond panels on dielectric interfaces not supported\n");
  }

  if (sys.ckclst) {
    sys.msg("Checking panels...");
    if(has_duplicate_panels(stdout, chglist)) {
      sys.error("charge list has duplicates\n");
    }
    sys.msg("no duplicates\n");
  }

  if (sys.muldat) {
    dumpMulSet(&sys);
  }
  fflush(stdout);

  starttimer;
  mulMultiAlloc(&sys, MAX(sys.max_eval_pnt, sys.max_panel), sys.order, sys.depth);
  stoptimer;
  initalltime += dtime;         /* save initial allocation time */

  if (sys.dumpps == DUMPPS_ON || sys.dumpps == DUMPPS_ALL) {
    dump_ps_mat(&sys, dump_filename, 0, 0, eval_size, eval_size, argv, argc, OPEN);
  }

  mulMatDirect(&sys, &trimat, &sqrmat, &real_index, up_size, eval_size);                /* Compute the direct part matrices. */

  if (! sys.dirsol) {           /* with DIRSOL just want to skip to solve */

    if (PRECOND == BD) {
      starttimer;
      bdmulMatPrecond(&sys);
      stoptimer;
      counters.prsetime = dtime;                /* preconditioner set up time */
    }

    if (PRECOND == OL) {
      starttimer;
      olmulMatPrecond(&sys);
      stoptimer;
      counters.prsetime = dtime;                /* preconditioner set up time */
    }

    if (sys.dmprec) {
      dump_preconditioner(&sys, chglist, 1);    /* dump prec. and P to matlab file */
    }

    if (sys.dpsysd) {
      dissys(&sys);
    }

    if (sys.ckdlst) {
      chkList(&sys, DIRECT);
    }
  }

  dumpnums(&sys, ON, eval_size);    /* save num/type of pot. coeff calcs */

  dirtimesav = counters.dirtime;    /* save direct matrix setup time */
  counters.dirtime = 0.0;           /* make way for direct solve time */

  if (! sys.dirsol) {

    if (sys.dumpps == DUMPPS_ON) {
      dump_ps_mat(&sys, dump_filename, 0, 0, eval_size, eval_size, argv, argc, CLOSE);
    }

    starttimer;
    mulMatUp(&sys);             /* Compute the upward pass matrices. */

    if (DNTYPE == NOSHFT) {
      mulMatDown(&sys);         /* find matrices for no L2L shift dwnwd pass */
    }

    if (DNTYPE == GRENGD) {
      mulMatDown(&sys);         /* find matrices for full Greengard dnwd pass*/
    }

    if (sys.ckdlst) {
      chkList(&sys, DIRECT);
      chkLowLev(&sys, DIRECT);
      //  Not available anywhere: chkEvalLstD(sys, DIRECT);
    }

    mulMatEval(&sys);           /* set up matrices for evaluation pass */

    stoptimer;
    mulsetup = dtime;           /* save multipole matrix setup time */

    dumpnums(&sys, OFF, eval_size);     /* dump num/type of pot. coeff calcs */

    if (sys.dumpps == DUMPPS_ALL) {
      dump_ps_mat(&sys, dump_filename, 0, 0, eval_size, eval_size, argv, argc, CLOSE);
    }

    if (sys.dissyn) {
      dumpSynop(&sys);
    }

    if (sys.dmtcnt) {
      dumpMatBldCnts(&sys);
    }

  }

  sys.msg("\nITERATION DATA");
  ttliter = capsolve(&capmat, &sys, chglist, eval_size, up_size, trimat, sqrmat, real_index);

  if (sys.mksdat) {
    mksCapDump(&sys, capmat);
  }

  if (sys.timdat) {
    ttlsetup = initalltime + dirtimesav + mulsetup;
    counters.multime = counters.uptime + counters.downtime + counters.evaltime;
    ttlsolve = counters.dirtime + counters.multime + counters.prectime + counters.conjtime;

    sys.msg("\nTIME AND MEMORY USAGE SYNOPSIS\n");
  }

  if (sys.timdat) {

    sys.msg("Total time: %g\n", ttlsetup + ttlsolve);
    sys.msg("  Total setup time: %g\n", ttlsetup);
    sys.msg("    Direct matrix setup time: %g\n", dirtimesav);
    sys.msg("    Multipole matrix setup time: %g\n", mulsetup);
    sys.msg("    Initial misc. allocation time: %g\n", initalltime);
    sys.msg("  Total iterative P*q = psi solve time: %g\n", ttlsolve);
    sys.msg("    P*q product time, direct part: %g\n", counters.dirtime);
    sys.msg("    Total P*q time, multipole part: %g\n", counters.multime);
    sys.msg("      Upward pass time: %g\n", counters.uptime);
    sys.msg("      Downward pass time: %g\n", counters.downtime);
    sys.msg("      Evaluation pass time: %g\n", counters.evaltime);
    sys.msg("    Preconditioner solution time: %g\n", counters.prectime);
    sys.msg("    Iterative loop overhead time: %g\n", counters.conjtime);

    if(sys.dirsol) {            /* if solution is done by Gaussian elim. */
      sys.msg("\nTotal direct, full matrix LU factor time: %g\n", counters.lutime);
      sys.msg("Total direct, full matrix solve time: %g\n", counters.fullsoltime);
      sys.msg("Total direct operations: %d\n", counters.fulldirops);
    }
    else if (sys.expgcr) {      /* if solution done iteratively w/o multis */
      sys.msg("\nTotal A*q operations: %d (%d/iter)\n",
              counters.fullPqops, counters.fullPqops/ttliter);
    }

    sys.msg("Total memory allocated: %d kilobytes ", int(sys.heap.total_memory()/1024));

    sys.msg("  Q2M  matrix memory allocated: %7.d kilobytes\n",
            int(sys.heap.memory(AQ2M)/1024));
    sys.msg("  Q2L  matrix memory allocated: %7.d kilobytes\n",
            int(sys.heap.memory(AQ2L)/1024));
    sys.msg("  Q2P  matrix memory allocated: %7.d kilobytes\n",
            int(sys.heap.memory(AQ2P)/1024));
    sys.msg("  L2L  matrix memory allocated: %7.d kilobytes\n",
            int(sys.heap.memory(AL2L)/1024));
    sys.msg("  M2M  matrix memory allocated: %7.d kilobytes\n",
            int(sys.heap.memory(AM2M)/1024));
    sys.msg("  M2L  matrix memory allocated: %7.d kilobytes\n",
            int(sys.heap.memory(AM2L)/1024));
    sys.msg("  M2P  matrix memory allocated: %7.d kilobytes\n",
            int(sys.heap.memory(AM2P)/1024));
    sys.msg("  L2P  matrix memory allocated: %7.d kilobytes\n",
            int(sys.heap.memory(AL2P)/1024));
    sys.msg("  Q2PD matrix memory allocated: %7.d kilobytes\n",
            int(sys.heap.memory(AQ2PD)/1024));
    sys.msg("  Miscellaneous mem. allocated: %7.d kilobytes\n",
            int(sys.heap.memory(AMSC)/1024));

  }

  return 0;
}
