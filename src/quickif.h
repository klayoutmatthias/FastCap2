
#if !defined(quickif_H)
#define quickif_H

#include <cstdio>

#include "vector.h"
#include "matrix.h"

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

charge *quickif(ssystem *sys, FILE *fp, const char *line, int surf_type, const Matrix3d &rot, const Vector3d &trans, const char *name_suffix, char **title);
charge *quickif2charges(ssystem *sys, quadl *fstquad, tri *fsttri, const Matrix3d &rot, const Vector3d &trans, int cond_num);

#endif
