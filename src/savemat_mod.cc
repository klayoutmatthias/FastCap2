
/*
 * savemat - C language routine to save a matrix in a MAT-file.
 *
 * We recommend that you use this routine, and its companion loadmat.c,
 * for all writing and reading of MAT-files.  These routines implement
 * "access methods" for MAT-files.  By using these routines, instead
 * of writing your own code to directly access the MAT-file format,
 * you will be unaffected by any changes that may be made to the MAT-file
 * structure at a future date.
 *
 * Here is an example that uses 'savemat' to save two matrices to disk,
 * the second of which is complex:
 *
 *      FILE *fp;
 *      double xyz[1000], ar[1000], ai[1000];
 *      fp = fopen("foo.mat","wb");
 *      savemat(fp, 2000, "xyz", 2, 3, 0, xyz, (double *)0);
 *      savemat(fp, 2000, "a", 5, 5, 1, ar, ai);
 *      fclose(fp);
 *
 * Author J.N. Little 11-3-86
 * Revised 7-23-91 to support ANSI-C
 */
#include <cstdio>
#include <cstring>

#include "savemat_mod.h"

typedef struct {
     long type;   /* type */
     long mrows;  /* row dimension */
     long ncols;  /* column dimension */
     long imagf;  /* flag indicating imag part */
     long namlen; /* name length (including NULL) */
} Fmatrix;

void savemat(FILE *fp, int type, const char *pname, int mrows, int ncols,
             int imagf, double *preal, double *pimag)
{
        Fmatrix x;
        int mn;
        
        x.type = type;
        x.mrows = mrows;
        x.ncols = ncols;
        x.imagf = imagf;
        x.namlen = strlen(pname) + 1;
        mn = x.mrows * x.ncols;

        fwrite(&x, sizeof(Fmatrix), 1, fp);
        fwrite(pname, sizeof(char), (int)x.namlen, fp);
        fwrite(preal, sizeof(double), mn, fp);
        if (imagf) {
             fwrite(pimag, sizeof(double), mn, fp);
        }
}

/*
  MODIFIED version of above: added wr_flag to allow multiple writes 
  to same matrix 
  wr_flag = 0 => open, print header (like old matlab setup)
  wr_flag = 1 => update, print without header
*/
void savemat_mod(FILE *fp, int type, const char *pname, int mrows, int ncols,
                 int imagf, double *preal, double *pimag, int wr_flag, int mn)
{
        Fmatrix x;
        
        if(wr_flag == 0) {
          x.type = type;
          x.mrows = mrows;
          x.ncols = ncols;
          x.imagf = imagf;
          x.namlen = strlen(pname) + 1;
        
          fwrite(&x, sizeof(Fmatrix), 1, fp);
          fwrite(pname, sizeof(char), (int)x.namlen, fp);
        }
        fwrite(preal, sizeof(double), mn, fp);
        if (imagf) {
             fwrite(pimag, sizeof(double), mn, fp);
        }
}
