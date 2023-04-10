
#if !defined(patran_f_H)
#define patran_f_H

#include <cstdio>

struct ssystem;
struct charge;

charge *patfront(ssystem *sys, FILE *stream, const char *header, int surf_type, double *trans_vector, const char *name_suffix, char **title);

#endif
