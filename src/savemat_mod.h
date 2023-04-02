
#if !defined(savemat_mod_H)
#define savemat_mod_H

#include <cstdio>

void savemat_mod(FILE *fp, int type, const char *pname, int mrows, int ncols,
                 int imagf, double *preal, double *pimag, int wr_flag, int mn);
void savemat(FILE *fp, int type, const char *pname, int mrows, int ncols,
             int imagf, double *preal, double *pimag);

#endif
