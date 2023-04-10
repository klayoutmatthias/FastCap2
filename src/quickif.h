
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
  const char *title;
  quadl *quads;
  tri *tris;

  SurfaceData *clone(Heap &heap);

private:
  SurfaceData(const SurfaceData &);
  SurfaceData &operator=(const SurfaceData &);
};

int getConductorNum(ssystem *sys, const char *name, int *num_cond);
const char *getConductorName(const ssystem *sys, int cond_num);
const char *last_alias(const Name *cur_name);
charge *quickif(ssystem *sys, FILE *fp, const char *line, int surf_type, double *trans, int *num_cond, const char *name_suffix, char **title);
charge *quickif2charges(ssystem *sys, quadl *fstquad, tri *fsttri, double *trans, int cond_num);

#endif
