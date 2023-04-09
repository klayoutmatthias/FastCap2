
#if !defined(fastcap2_surface_H)
#define fastcap2_surface_H

#include <Python.h>

#include "heap.h"
#include "quickif.h"

struct Surface
  : SurfaceData
{
  Surface();

  void add_quad(int cond_num,
                double x1, double y1, double z1,
                double x2, double y2, double z2,
                double x3, double y3, double z3,
                double x4, double y4, double z4);

  void add_tri(int cond_num,
               double x1, double y1, double z1,
               double x2, double y2, double z2,
               double x3, double y3, double z3);

  void set_name(const char *name);
  void set_title(const char *title);

  Heap heap;

private:
  Surface(const Surface &);
  Surface &operator=(const Surface &);
};

struct SurfaceObject {
  PyObject_HEAD
  Surface surface;
};

#endif
