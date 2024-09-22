
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
  Vector3d p1, p2, p3, p4;      /* four corner coordinates */
  Vector3d rp;                  /* per-panel reference point */
  bool has_rp;                  /* has per-panel reference point */
};

struct tri {                    /* triangular element */
  int cond;                     /* conductor number */
  struct tri *next;             /* linked list pntr */
  Vector3d p1, p2, p3;          /* three corner coordinates */
  Vector3d rp;                  /* per-panel reference point */
  bool has_rp;                  /* has per-panel reference point */
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
