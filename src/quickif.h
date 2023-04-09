
#if !defined(quickif_H)
#define quickif_H

#include <cstdio>

struct ssystem;
struct Name;
struct charge;
class Heap;

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

struct SurfaceData
{
  SurfaceData();

  const char *name;
  quadl *quads;
  tri *tris;

  SurfaceData *clone(Heap &heap);

private:
  SurfaceData(const SurfaceData &);
  SurfaceData &operator=(const SurfaceData &);
};

int getConductorNum(ssystem *sys, char *name, int *num_cond);
char *getConductorName(ssystem *sys, int cond_num);
char *last_alias(Name *cur_name);
charge *quickif(ssystem *sys, FILE *fp, char *line, int surf_type, double *trans, int *num_cond, char *name_suffix);

#endif
