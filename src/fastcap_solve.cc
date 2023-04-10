
#include "fastcap_solve.h"

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
#include <cassert>
#include <unistd.h>

int capmatrix_size(const ssystem *sys)
{
  int valid_conductors = 0;

  /* count the actual number of conductors */
  for (int i = 1; i <= sys->num_cond; i++) {
    if (sys->kinp_num_list.find(i) == sys->kinp_num_list.end()) {
      ++valid_conductors;
    }
  }

  return valid_conductors;
}

double **symmetrize_and_clean(ssystem *sys, double **capmat)
{
  int i, j, ii, jj, i_killed, j_killed, actual_count;
  double rowttl, **sym_mat;
  double mat_entry;

  actual_count = capmatrix_size(sys);

  /* set up symetrized matrix storage */
  /* NOTE: one element more as the conductor indexes are 1-based */
  sym_mat = sys->heap.alloc<double*>(actual_count+1, AMSC);
  for (i=1; i <= actual_count; i++)  {
    sym_mat[i] = sys->heap.alloc<double>(actual_count+1, AMSC);
  }

  for (i = 1; i <= actual_count; i++) {
    for (j = 1; j <= actual_count; j++) {
      sym_mat[i][j] = 0.0;
    }
  }

  /* get the smallest and largest (absolute value) symmetrized elements */
  /* check for non-M-matrix symmetrized capacitance matrix */
  for (i = 1, ii = 1; i <= sys->num_cond; i++) {

    /* skip conductors removed from input */
    if (sys->kinp_num_list.find(i) != sys->kinp_num_list.end()) {
      continue;
    }

    i_killed = (sys->kill_num_list.find(i) != sys->kill_num_list.end());

    if (capmat[i][i] <= 0.0 && !i_killed) {
      sys->info("\nmksCapDump: Warning - capacitance matrix has non-positive diagonal\n  row %d\n", i+1);
    }

    rowttl = 0.0;
    assert(ii <= actual_count);

    for (j = 1, jj = 1; j <= sys->num_cond; j++) {

      /* skip conductors removed from input */
      if (sys->kinp_num_list.find(j) != sys->kinp_num_list.end()) {
        continue;
      }

      if (j == i) {

        assert(ii == jj);
        sym_mat[ii][ii] = capmat[i][i];

      } else {

        /* if this column was not calculated and neither was the column
           with the same number as the current row, then symetrized mat has
           no entry at [i][j], [j][i] */
        j_killed = (sys->kill_num_list.find(j) != sys->kill_num_list.end());

        if (i_killed && j_killed) mat_entry = 0.0;

        /* if this column was calculated but column with the same number
           as the current row wasnt, then symmetrized mat has unaveraged entry
           at [i][j], [j][i] */
        else if(i_killed && !j_killed) mat_entry = capmat[i][j];

        /* if this column was not calculated but column with the same number
           as the current row was, then symmetrized mat has unaveraged entry
           at [i][j], [j][i] */
        else if(!i_killed && j_killed) mat_entry = capmat[j][i];

        /* if this column was calculated and column with the same number
           as the current row was also, then symmetrized mat has averaged entry
           at [i][j], [j][i] */
        else mat_entry = (capmat[i][j] + capmat[j][i])/2.0;

        rowttl += mat_entry;

        if (!(i_killed && j_killed) && mat_entry >= 0.0) {
          sys->info("\nmksCapDump: Warning - capacitance matrix has non-negative off-diagonals\n  row %d col %d\n", i, j);
        }

        assert(jj <= actual_count);
        sym_mat[ii][jj] = mat_entry;

      }

      ++jj;

    }

    if (rowttl + capmat[i][i] <= 0.0 && !i_killed) {
      sys->info("\nmksCapDump: Warning - capacitance matrix is not strictly diagonally dominant\n  due to row %d\n", i);
    }

    ++ii;

  }

  return sym_mat;
}

double **fastcap_solve(ssystem *sys)
{
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

  char hostname[BUFSIZ];
  long clock;

  char dump_filename[BUFSIZ];
  strcpy(dump_filename, "psmat.ps");

  /* get the list of all panels in the problem */
  /* - many command line parameters having to do with the postscript
       file dumping interface are passed back via globals (see mulGlobal.c) */
  chglist = build_charge_list(sys);
  if (!chglist) {
    throw std::runtime_error("No surfaces present - cannot compute capacitance matrix");
  }

  if (sys->dissrf && sys->log) {
    dumpSurfDat(sys);
  }

  if (sys->log) {
    time(&clock);
    sys->msg("  Date: %s", ctime(&clock));
    if (gethostname(hostname, BUFSIZ) != -1) {
      sys->msg("  Host: %s\n", hostname);
    } else {
      sys->msg("  Host: ? (gethostname() failure)\n");
    }
  }

  if (sys->cfgdat && sys->log) {
    dumpConfig(sys, sys->argv[0]);
  }

  starttimer;
  mulInit(sys, chglist);  /* Set up cubes, charges. */
  stoptimer;
  initalltime = dtime;

  sys->msg("\nINPUT SUMMARY\n");

  if (sys->cmddat) {
    sys->msg("  Expansion order: %d\n", sys->order);
    sys->msg("  Number of partitioning levels: %d\n", sys->depth);
    sys->msg("  Overall permittivity factor: %.3g\n", sys->perm_factor);
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

  if (! sys->dissrf) {
    sys->msg("Title: `%s'\n", sys->title ? sys->title : "");
  }
  sys->msg("  Total number of panels: %d\n", up_size);
  sys->msg("    Number of conductor panels: %d\n", num_cond_panels);
  sys->msg("    Number of dielectric interface panels: %d\n",
          num_dielec_panels);
  sys->msg(
          "    Number of thin conductor on dielectric interface panels: %d\n",
          num_both_panels);
  /*sys->msg("  Number of extra evaluation points: %d\n",
          num_dummy_panels);*/
  sys->msg("  Number of conductors: %d\n", sys->num_cond);

  if (sys->namdat && sys->log) {
    dumpCondNames(sys);
  }

  if (num_both_panels > 0) {
    sys->error("Thin cond panels on dielectric interfaces not supported");
  }

  if (sys->ckclst) {
    sys->msg("Checking panels...");
    if(has_duplicate_panels(sys, chglist)) {
      sys->error("charge list has duplicates");
    }
    sys->msg("no duplicates\n");
  }

  if (sys->muldat) {
    dumpMulSet(sys);
  }

  sys->flush();

  starttimer;
  mulMultiAlloc(sys, MAX(sys->max_eval_pnt, sys->max_panel), sys->order, sys->depth);
  stoptimer;
  initalltime += dtime;         /* save initial allocation time */

  if (sys->dumpps == DUMPPS_ON || sys->dumpps == DUMPPS_ALL) {
    dump_ps_mat(sys, dump_filename, 0, 0, eval_size, eval_size, sys->argv, sys->argc, OPEN);
  }

  mulMatDirect(sys, &trimat, &sqrmat, &real_index, up_size, eval_size);                /* Compute the direct part matrices. */

  if (! sys->dirsol) {           /* with DIRSOL just want to skip to solve */

    if (PRECOND == BD) {
      starttimer;
      bdmulMatPrecond(sys);
      stoptimer;
      counters.prsetime = dtime;                /* preconditioner set up time */
    }

    if (PRECOND == OL) {
      starttimer;
      olmulMatPrecond(sys);
      stoptimer;
      counters.prsetime = dtime;                /* preconditioner set up time */
    }

    if (sys->dmprec) {
      dump_preconditioner(sys, chglist, 1);    /* dump prec. and P to matlab file */
    }

    if (sys->dpsysd) {
      dissys(sys);
    }

    if (sys->ckdlst) {
      chkList(sys, DIRECT);
    }
  }

  dumpnums(sys, ON, eval_size);    /* save num/type of pot. coeff calcs */

  dirtimesav = counters.dirtime;    /* save direct matrix setup time */
  counters.dirtime = 0.0;           /* make way for direct solve time */

  if (! sys->dirsol) {

    if (sys->dumpps == DUMPPS_ON) {
      dump_ps_mat(sys, dump_filename, 0, 0, eval_size, eval_size, sys->argv, sys->argc, CLOSE);
    }

    starttimer;
    mulMatUp(sys);             /* Compute the upward pass matrices. */

    if (DNTYPE == NOSHFT) {
      mulMatDown(sys);         /* find matrices for no L2L shift dwnwd pass */
    }

    if (DNTYPE == GRENGD) {
      mulMatDown(sys);         /* find matrices for full Greengard dnwd pass*/
    }

    if (sys->ckdlst) {
      chkList(sys, DIRECT);
      chkLowLev(sys, DIRECT);
      //  Not available anywhere: chkEvalLstD(sys, DIRECT);
    }

    mulMatEval(sys);           /* set up matrices for evaluation pass */

    stoptimer;
    mulsetup = dtime;           /* save multipole matrix setup time */

    dumpnums(sys, OFF, eval_size);     /* dump num/type of pot. coeff calcs */

    if (sys->dumpps == DUMPPS_ALL) {
      dump_ps_mat(sys, dump_filename, 0, 0, eval_size, eval_size, sys->argv, sys->argc, CLOSE);
    }

    if (sys->dissyn) {
      dumpSynop(sys);
    }

    if (sys->dmtcnt) {
      dumpMatBldCnts(sys);
    }

  }

  sys->msg("\nITERATION DATA");
  ttliter = capsolve(&capmat, sys, chglist, eval_size, up_size, trimat, sqrmat, real_index);

  capmat = symmetrize_and_clean(sys, capmat);

  if (sys->mksdat && sys->log) {
    mksCapDump(sys, capmat);
  }

  if (sys->timdat && sys->log) {

    ttlsetup = initalltime + dirtimesav + mulsetup;
    counters.multime = counters.uptime + counters.downtime + counters.evaltime;
    ttlsolve = counters.dirtime + counters.multime + counters.prectime + counters.conjtime;

    sys->msg("\nTIME AND MEMORY USAGE SYNOPSIS\n");

    sys->msg("Total time: %g\n", ttlsetup + ttlsolve);
    sys->msg("  Total setup time: %g\n", ttlsetup);
    sys->msg("    Direct matrix setup time: %g\n", dirtimesav);
    sys->msg("    Multipole matrix setup time: %g\n", mulsetup);
    sys->msg("    Initial misc. allocation time: %g\n", initalltime);
    sys->msg("  Total iterative P*q = psi solve time: %g\n", ttlsolve);
    sys->msg("    P*q product time, direct part: %g\n", counters.dirtime);
    sys->msg("    Total P*q time, multipole part: %g\n", counters.multime);
    sys->msg("      Upward pass time: %g\n", counters.uptime);
    sys->msg("      Downward pass time: %g\n", counters.downtime);
    sys->msg("      Evaluation pass time: %g\n", counters.evaltime);
    sys->msg("    Preconditioner solution time: %g\n", counters.prectime);
    sys->msg("    Iterative loop overhead time: %g\n", counters.conjtime);

    if(sys->dirsol) {            /* if solution is done by Gaussian elim. */
      sys->msg("\nTotal direct, full matrix LU factor time: %g\n", counters.lutime);
      sys->msg("Total direct, full matrix solve time: %g\n", counters.fullsoltime);
      sys->msg("Total direct operations: %d\n", counters.fulldirops);
    }
    else if (sys->expgcr) {      /* if solution done iteratively w/o multis */
      sys->msg("\nTotal A*q operations: %d (%d/iter)\n",
              counters.fullPqops, counters.fullPqops/ttliter);
    }

    sys->msg("Total memory allocated: %d kilobytes ", int(sys->heap.total_memory()/1024));

    sys->msg("  Q2M  matrix memory allocated: %7.d kilobytes\n",
            int(sys->heap.memory(AQ2M)/1024));
    sys->msg("  Q2L  matrix memory allocated: %7.d kilobytes\n",
            int(sys->heap.memory(AQ2L)/1024));
    sys->msg("  Q2P  matrix memory allocated: %7.d kilobytes\n",
            int(sys->heap.memory(AQ2P)/1024));
    sys->msg("  L2L  matrix memory allocated: %7.d kilobytes\n",
            int(sys->heap.memory(AL2L)/1024));
    sys->msg("  M2M  matrix memory allocated: %7.d kilobytes\n",
            int(sys->heap.memory(AM2M)/1024));
    sys->msg("  M2L  matrix memory allocated: %7.d kilobytes\n",
            int(sys->heap.memory(AM2L)/1024));
    sys->msg("  M2P  matrix memory allocated: %7.d kilobytes\n",
            int(sys->heap.memory(AM2P)/1024));
    sys->msg("  L2P  matrix memory allocated: %7.d kilobytes\n",
            int(sys->heap.memory(AL2P)/1024));
    sys->msg("  Q2PD matrix memory allocated: %7.d kilobytes\n",
            int(sys->heap.memory(AQ2PD)/1024));
    sys->msg("  Miscellaneous mem. allocated: %7.d kilobytes\n",
            int(sys->heap.memory(AMSC)/1024));

  }

  return capmat;
}
