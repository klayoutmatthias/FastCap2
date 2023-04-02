
#if !defined(counters_H)
#define counters_H

struct Counters
{
  Counters();

  double prectime;            //  time spent doing back solve for prec
  double prsetime;            //  time spent calculating preconditioner
  double conjtime;            //  time spent doing everything but A*q
  double dirtime;             //  time for direct part of P*q
  double multime;             //  time for multipole part of P*q
  double uptime;              //  time in mulUp(), upward pass
  double downtime;            //  time in mulDown(), downward pass
  double evaltime;            //  time in mulEval(), evaluation pass
  int fulldirops;             //  total direct operations - DIRSOL=ON only
  double lutime;              //  factorization time DIRSOL=ON only
  double fullsoltime;         //  time for solves, DIRSOL=ON only
  int fullPqops;              //  total P*q ops using P on disk - EXPGCR=ON
};

extern Counters counters;

#endif
