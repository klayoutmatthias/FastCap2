
#if !defined(psMatDisplay_H)
#define psMatDisplay_H

struct ssystem;

void dump_ps_mat(ssystem *sys, char *filename, int row, int col, int num_row, int num_col, const char **argv, int argc, int type);

#endif
