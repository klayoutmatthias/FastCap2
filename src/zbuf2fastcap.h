
#if !defined(zbuf2fastcap_H)
#define zbuf2fastcap_H

struct ssystem;
struct charge;

void dump_ps_geometry(ssystem *sys, const char *filename, charge *chglist, double *q, int use_ttl_chg);

#endif
