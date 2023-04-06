
#if !defined(calcp_H)
#define calcp_H

struct ssystem;
struct charge;

void dumpnums(ssystem *sys, int flag, int size);
double tilelength(charge *nq);

void initcalcp(ssystem *sys, charge *panel_list);
double calcp(ssystem *sys, charge *panel, double x, double y, double z, double *pfd);

#endif
