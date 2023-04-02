
#if !defined(quickif_H)
#define quickif_H

#include "mulStruct.h"
#include <cstdio>

struct quadl {                  /* quadralateral element */
  int cond;                     /* conductor number */
  struct quadl *next;           /* linked list pntr */
  double x1;                    /* four corner coordinates */
  double x2;
  double x3;
  double x4;
  double y1;  
  double y2;
  double y3;
  double y4;
  double z1;  
  double z2;
  double z3;
  double z4;
};
typedef struct quadl quadl;

struct tri {                    /* triangular element */
  int cond;                     /* conductor number */
  struct tri *next;             /* linked list pntr */
  double x1;                    /* three corner coordinates */
  double x2;
  double x3;
  double y1;  
  double y2;
  double y3;
  double z1;  
  double z2;
  double z3;
};
typedef struct tri tri;

/* #define MAXCON 10000            assumes never more conductors than this */

int getConductorNum(ssystem *sys, char *name, Name **name_list, int *num_cond);
char *getConductorName(ssystem *sys, int cond_num, Name **name_list);
char *last_alias(Name *cur_name);
charge *quickif(ssystem *sys, FILE *fp, char *line, int surf_type, double *trans, int *num_cond, Name **name_list, char *name_suffix);

#endif
