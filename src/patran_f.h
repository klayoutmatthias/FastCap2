
#if !defined(patran_f_H)
#define patran_f_H

charge *patfront(ssystem *sys, FILE *stream, int *file_is_patran_type, int surf_type, double *trans_vector,
                 Name **name_list, int *num_cond, char *name_suffix);
char *delcr(char *str);

#endif
