
#if !defined(fastcap2_surface_H)
#define fastcap2_surface_H

#include <Python.h>

#include "heap.h"
#include "quickif.h"

struct PySurface
  : SurfaceData
{
  PySurface();

  void add_quad(int cond_num,
                const Vector3d &p1,
                const Vector3d &p2,
                const Vector3d &p3,
                const Vector3d &p4,
                const Vector3d *rp);

  void add_tri(int cond_num,
               const Vector3d &p1,
               const Vector3d &p2,
               const Vector3d &p3,
               const Vector3d *rp);

  void set_name(const char *name);
  void set_title(const char *title);

  Heap heap;

private:
  PySurface(const PySurface &);
  PySurface &operator=(const PySurface &);
};

struct PySurfaceObject {
  PyObject_HEAD
  PySurface surface;
};

#endif
