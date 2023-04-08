
#if !defined(patran_f_H)
#define patran_f_H

#include <cstdio>

struct ssystem;
struct Name;

charge *patfront(ssystem *sys, FILE *stream, int *file_is_patran_type, int surf_type, double *trans_vector, int *num_cond, char *name_suffix);
char *delcr(char *str);

#endif
