
#if !defined(input_H)
#define input_H

struct ssystem;
struct charge;
struct ITER;
struct Name;

void get_ps_file_base(ssystem *sys);
char *hack_path(char *str);
int want_this_iter(ITER *iter_list, int iter_num);

charge *input_problem(ssystem *sys, int *autmom, int *autlev, double *relperm,
                      int *numMom, int *numLev, Name **name_list, int *num_cond);

#endif
