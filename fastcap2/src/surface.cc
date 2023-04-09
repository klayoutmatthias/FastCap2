
#include "surface.h"

#include <memory>

// --------------------------------------------------------------------------

Surface::Surface()
  : SurfaceData(), heap()
{
}

void Surface::add_quad(int cond_num,
                       double x1, double y1, double z1,
                       double x2, double y2, double z2,
                       double x3, double y3, double z3,
                       double x4, double y4, double z4)
{
  quadl *new_quad = heap.alloc<quadl>(1);
  new_quad->cond = cond_num;
  new_quad->x1 = x1;
  new_quad->y1 = y1;
  new_quad->z1 = z1;
  new_quad->x2 = x2;
  new_quad->y2 = y2;
  new_quad->z2 = z2;
  new_quad->x3 = x3;
  new_quad->y3 = y3;
  new_quad->z3 = z3;
  new_quad->x4 = x4;
  new_quad->y4 = y4;
  new_quad->z4 = z4;
  new_quad->next = quads;
  quads = new_quad;
}

void Surface::add_tri(int cond_num,
                      double x1, double y1, double z1,
                      double x2, double y2, double z2,
                      double x3, double y3, double z3)
{
  tri *new_tri = heap.alloc<tri>(1);
  new_tri->cond = cond_num;
  new_tri->x1 = x1;
  new_tri->y1 = y1;
  new_tri->z1 = z1;
  new_tri->x2 = x2;
  new_tri->y2 = y2;
  new_tri->z2 = z2;
  new_tri->x3 = x3;
  new_tri->y3 = y3;
  new_tri->z3 = z3;
  new_tri->next = tris;
  tris = new_tri;
}

void Surface::set_name(const char *n)
{
  if (!name) {
    name = 0;
  } else {
    name = heap.strdup(n);
  }
}

// --------------------------------------------------------------------------

static void
surface_dealloc(SurfaceObject *self)
{
  self->surface.~Surface();
}

static PyObject *
surface_new(PyTypeObject *type, PyObject * /*args*/, PyObject * /*kwds*/)
{
  SurfaceObject *self;
  self = (SurfaceObject *) type->tp_alloc(type, 0);
  if (self != NULL) {
    new (&self->surface) Surface;
  }

  return (PyObject *) self;
}

static int
surface_init(SurfaceObject *self, PyObject *args, PyObject *kwds)
{
  static char *kwlist[] = {(char *)"name", NULL};
  PyObject *name = NULL;

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O", kwlist,
                                   &name)) {
    return -1;
  }

  if (name) {
    PyObject *name_str = PyObject_Str(name);
    if (!name_str) {
      return -1;
    }
    const char *name_utf8str = PyUnicode_AsUTF8(name_str);
    if (!name_utf8str) {
      return -1;
    }
    self->surface.set_name(name_utf8str);
    Py_DECREF(name_str);
  }

  return 0;
}

static PyObject *
surface_add_quad(SurfaceObject *self, PyObject *args)
{
  PyObject *p1, *p2, *p3, *p4;
  if (!PyArg_ParseTuple(args, "OOOO", &p1, &p2, &p3, &p4)) {
    return NULL;
  }

  double x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;

  int ok = PyArg_ParseTuple(p1, "ddd", &x1, &y1, &z1) &&
           PyArg_ParseTuple(p2, "ddd", &x2, &y2, &z2) &&
           PyArg_ParseTuple(p3, "ddd", &x3, &y3, &z3) &&
           PyArg_ParseTuple(p4, "ddd", &x4, &y4, &z4);

  Py_DECREF(p1);
  Py_DECREF(p2);
  Py_DECREF(p3);
  Py_DECREF(p4);

  if (! ok) {
    return NULL;
  } else {
    self->surface.add_quad(0, x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4);
    Py_RETURN_NONE;
  }
}

static PyObject *
surface_add_tri(SurfaceObject *self, PyObject *args)
{
  PyObject *p1, *p2, *p3;
  if (!PyArg_ParseTuple(args, "OOO", &p1, &p2, &p3)) {
    return NULL;
  }

  double x1, y1, z1, x2, y2, z2, x3, y3, z3;

  int ok = PyArg_ParseTuple(p1, "ddd", &x1, &y1, &z1) &&
           PyArg_ParseTuple(p2, "ddd", &x2, &y2, &z2) &&
           PyArg_ParseTuple(p3, "ddd", &x3, &y3, &z3);

  Py_DECREF(p1);
  Py_DECREF(p2);
  Py_DECREF(p3);

  if (! ok) {
    return NULL;
  } else {
    self->surface.add_tri(0, x1, y1, z1, x2, y2, z2, x3, y3, z3);
    Py_RETURN_NONE;
  }
}

static PyObject *
surface_get_name(SurfaceObject *self)
{
  if (self->surface.name) {
    return PyUnicode_FromString(self->surface.name);
  } else {
    Py_RETURN_NONE;
  }
}

static PyObject *
surface_set_name(SurfaceObject *self, PyObject *value)
{
  if (Py_IsNone(value)) {
    self->surface.set_name(0);
  } else {
    PyObject *name_str = PyObject_Str(value);
    if (!name_str) {
      return NULL;
    }
    const char *name_utf8str = PyUnicode_AsUTF8(name_str);
    if (!name_utf8str) {
      return NULL;
    }
    self->surface.set_name(name_utf8str);
  }
  Py_RETURN_NONE;
}

static PyMethodDef surface_methods[] = {
  { "_get_name", (PyCFunction) surface_get_name, METH_NOARGS, NULL },
  { "_set_name", (PyCFunction) surface_set_name, METH_O, NULL },
  { "_add_quad", (PyCFunction) surface_add_quad, METH_VARARGS, NULL },
  { "_add_tri", (PyCFunction) surface_add_tri, METH_VARARGS, NULL },
  {NULL}
};

PyTypeObject surface_type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "fastcap2_core.Surface",
  .tp_basicsize = sizeof(SurfaceObject),
  .tp_itemsize = 0,
  .tp_dealloc = (destructor) surface_dealloc,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_methods = surface_methods,
  .tp_init = (initproc) surface_init,
  .tp_new = surface_new,
};

