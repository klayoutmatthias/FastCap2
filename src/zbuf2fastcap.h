
#if !defined(zbuf2fastcap_H)
#define zbuf2fastcap_H

struct ssystem;
struct charge;

void dump_ps_geometry(ssystem *sys, charge *chglist, double *q, int cond, int use_ttl_chg);

#endif
