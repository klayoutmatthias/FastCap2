
#include "mulStruct.h"
#include "mulGlobal.h"
#include "zbufGlobal.h"

#include <cstdarg>
#include <stdexcept>

ssystem::ssystem() :
  argv(0),
  argc(0),
  log(stdout),
  start_name(0),
  current_name(0),
  start_name_this_time(0),
  kill_name_list(0),
  kill_num_list(0),
  kinp_name_list(0),
  kinp_num_list(0),
  qpic_name_list(0),
  qpic_num_list(0),
  kq_name_list(0),
  kq_num_list(0),
  iter_tol(ABSTOL),
  s_(false),
  n_(false),
  g_(false),
  c_(false),
  x_(false),
  k_(false),
  rc_(false),
  rd_(false),
  rb_(false),
  q_(false),
  rk_(false),
  m_(false),
  f_(false),
  dd_(false),
  elevation(DEFELE),
  azimuth(DEFAZM),
  rotation(DEFROT),
  distance(DEFDST),
  linewd(DEFWID),
  scale(DEFSCL),
  axeslen(DEFAXE),
  up_axis(DEFUAX),
  line_file(0),
  dirsol(false),
  expgcr(false),
  timdat(false),
  mksdat(true),
  dumpps(DUMPPS_OFF),
  capvew(true),
  cmddat(true),
  rawdat(false),
  itrdat(false),
  cfgdat(false),
  muldat(false),
  dissyn(false),
  dmtcnt(false),
  dissrf(true),
  namdat(false),
  disq2m(false),
  dism2m(false),
  dism2p(false),
  disl2p(false),
  disq2p(false),
  dsq2pd(false),
  disq2l(false),
  dism2l(false),
  disl2l(false),
  dalq2m(false),
  dalm2p(false),
  dall2p(false),
  dalq2l(false),
  dupvec(false),
  disfac(false),
  dpsysd(false),
  dilist(false),
  dmpele(false),
  dmpchg(DMPCHG_OFF),
  ckdlst(false),
  dmprec(false),
  ckclst(false),
  dpcomp(false),
  dpddif(false),
  chkdum(false),
  jacdbg(false),
  ps_file_base(0),
  axes(0),
  title(0),
  side(0),
  depth(-1),
  order(DEFORD),
  num_cond(0),
  cond_names(0),
  perm_factor(1.0),
  length(0.0),
  minx(0.0),
  miny(0.0),
  minz(0.0),
  mul_maxq(0),
  mul_maxlq(0),
  max_panel(0),
  loc_maxq(0),
  loc_maxlq(0),
  max_eval_pnt(0),
  q(0),
  p(0),
  panels(0),
  cubes(0),
  multilist(0),
  locallist(0),
  directlist(0),
  precondlist(0),
  revprecondlist(0),
  is_dummy(0),
  is_dielec(0)
{
  /* initialize defaults, etc */
  axes = heap.alloc<double **>(10);
  for (int i = 0; i < 10; i++) {
    axes[i] = heap.mat(2, 3);
  }

  for (int i = 0; i < int(sizeof(view) / sizeof(view[0])); ++i) {
    view[i] = 0.0;
  }

  moffset[0] = OFFSETX;         /* puts the origin this dist from lower left */
  moffset[1] = OFFSETY;

#if defined(DIRSOL) && DIRSOL == ON
  dirsol = true;
#endif
#if defined(EXPGCR) && EXPGCR == ON
  expgcr = true;
#endif

#if defined(TIMDAT) && TIMDAT == ON
  timdat = true;
#endif
#if defined(MKSDAT) && MKSDAT == OFF
  mksdat = false;
#endif
#if defined(CAPVEW) && CAPVEW == OFF
  capvew = false;
#endif
#if defined(CMDDAT) && CMDDAT == OFF
  cmddat = false;
#endif
#if defined(RAWDAT) && RAWDAT == ON
  rawdat = true;
#endif
#if defined(ITRDAT) && ITRDAT == ON
  itrdat = true;
#endif
#if defined(CFGDAT) && CFGDAT == ON
  cfgdat = true;
#endif
#if defined(MULDAT) && MULDAT == ON
  muldat = true;
#endif
#if defined(DISSYN) && DISSYN == ON
  dissyn = true;
#endif
#if defined(DMTCNT) && DMTCNT == ON
  dmtcnt = true;
#endif
#if defined(DISSRF) && DISSRF == OFF
  dissrf = false;
#endif
#if defined(NAMDAT) && NAMDAT == ON
  namdat = true;
#endif
#if defined(DUMPPS) && DUMPPS == ON
  dumpps = DUMPPS_ON;
#elif defined(DUMPPS) && DUMPPS == ALL
  dumpps = DUMPPS_ALL;
#endif

#if defined(DISQ2M) && DISQ2M == ON
  disq2m = true;
#endif
#if defined(DISM2M) && DISM2M == ON
  dism2m = true;
#endif
#if defined(DISM2P) && DISM2P == ON
  dis2mp = true;
#endif
#if defined(DISL2P) && DISL2P == ON
  disl2p = true;
#endif
#if defined(DISQ2P) && DISQ2P == ON
  disq2p = true;
#endif
#if defined(DSQ2PD) && DSQ2PD == ON
  dsq2pd = true;
#endif
#if defined(DISQ2L) && DISQ2L == ON
  disq2l = true;
#endif
#if defined(DISM2L) && DISM2L == ON
  dism2l = true;
#endif
#if defined(DISL2L) && DISL2L == ON
  disl2l = true;
#endif
#if defined(DALQ2M) && DALQ2M == ON
  dalq2m = true;
#endif
#if defined(DALM2P) && DALM2P == ON
  dalm2p = true;
#endif
#if defined(DALL2P) && DALL2P == ON
  dall2p = true;
#endif
#if defined(DALQ2L) && DALQ2L == ON
  dalq2l = true;
#endif

#if defined(DUPVEC) && DUPVEC == ON
  dupvec = true;
#endif
#if defined(DISFAC) && DISFAC == ON
  disfac = true;
#endif
#if defined(DPSYSD) && DPSYSD == ON
  dpsysd = true;
#endif
#if defined(DILIST) && DILIST == ON
  dilist = true;
#endif
#if defined(DMPELE) && DMPELE == ON
  dmpele = true;
#endif
#if defined(DMPCHG) && DMPCHG == ON
  dmpchg = DMPCHG_ON;
#elif defined(DMPCHG) && DMPCHG == LAST
  dmpchg = DMPCHG_LAST;
#endif

#if defined(CKDLST) && CKDLST == ON
  ckdlst = true;
#endif
#if defined(DMPREC) && DMPREC == ON
  dmprec = true;
#endif
#if defined(CKCLST) && CKCLST == ON
  ckclst = true;
#endif
#if defined(DPCOMP) && DPCOMP == ON
  dpcomp = true;
#endif
#if defined(DPDDIF) && DPDDIF == ON
  dpddif = true;
#endif
#if defined(CHKDUM) && CHKDUM == ON
  chkdum = true;
#endif
#if defined(JACDBG) && JACDBG == ON
  jacdbg = true;
#endif
}

void ssystem::msg(const char *fmt, ...)
{
  if (!log) {
    return;
  }

  va_list args;
  va_start(args, fmt);
  vfprintf(log, fmt, args);
}

void ssystem::info(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  FILE *out = log ? log : stderr;
  vfprintf(out, fmt, args);
}

void ssystem::warn(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  FILE *out = log ? log : stderr;
  fputs("WARNING: ", out);
  vfprintf(out, fmt, args);
}

void ssystem::error(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  char buf[1024];
  vsnprintf (buf, sizeof(buf), fmt, args);
  throw std::runtime_error(buf);
}

// ---------------------------------------------------------------------------------

multi_mats::multi_mats()
  : localcnt(0), multicnt(0), evalcnt(0),
    Q2Mcnt(0), Q2Lcnt(0), Q2Pcnt(0), L2Lcnt(0),
    M2Mcnt(0), M2Lcnt(0), M2Pcnt(0), L2Pcnt(0), Q2PDcnt(0),
    Irn(0), Mphi(0),
    Ir(0), phi(0),
    Rho(0), Rhon(0),
    Beta(0), Betam(0),
    tleg(0), factFac(0),
    sinmkB(0), cosmkB(0), facFrA(0)
{
}
