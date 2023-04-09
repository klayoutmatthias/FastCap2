
#include "surface.h"

#include <memory>
#include <sstream>

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
  if (!n) {
    name = 0;
  } else {
    name = heap.strdup(n);
  }
}

void Surface::set_title(const char *n)
{
  if (!n) {
    title = 0;
  } else {
    title = heap.strdup(n);
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
  static char *kwlist[] = {(char *)"name", (char *)"title", NULL};
  const char *name = 0;
  const char *title = 0;

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "|ss", kwlist, &name, &title)) {
    return -1;
  }

  if (name) {
    self->surface.set_name(name);
  }

  if (title) {
    self->surface.set_title(title);
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

static PyObject *
surface_get_title(SurfaceObject *self)
{
  if (self->surface.title) {
    return PyUnicode_FromString(self->surface.title);
  } else {
    Py_RETURN_NONE;
  }
}

static PyObject *
surface_set_title(SurfaceObject *self, PyObject *value)
{
  if (Py_IsNone(value)) {
    self->surface.set_title(0);
  } else {
    PyObject *title_str = PyObject_Str(value);
    if (!title_str) {
      return NULL;
    }
    const char *title_utf8str = PyUnicode_AsUTF8(title_str);
    if (!title_utf8str) {
      return NULL;
    }
    self->surface.set_title(title_utf8str);
  }
  Py_RETURN_NONE;
}

static double vprod(double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3)
{
  double xa = x2 - x1;
  double ya = y2 - y1;
  double za = z2 - z1;
  double xb = x3 - x1;
  double yb = y3 - y1;
  double zb = z3 - z1;
  return ((ya * zb) - (za * yb)) - ((xa * zb) - (za * xb)) + ((xa * yb) - (ya * xb));
}

static PyObject *
surface_quad_count(SurfaceObject *self)
{
  size_t n = 0;
  for (quadl *q = self->surface.quads; q; q = q->next) {
    ++n;
  }
  return PyLong_FromLong(n);
}

static PyObject *
surface_quad_area(SurfaceObject *self)
{
  double area = 0;
  for (quadl *q = self->surface.quads; q; q = q->next) {
    area += 0.5 * vprod(q->x1, q->y1, q->z1, q->x4, q->y4, q->z4, q->x2, q->y2, q->z2);
    area += 0.5 * vprod(q->x3, q->y3, q->z3, q->x2, q->y2, q->z2, q->x4, q->y4, q->z4);
  }
  return PyFloat_FromDouble(area);
}

static PyObject *
surface_tri_count(SurfaceObject *self)
{
  size_t n = 0;
  for (tri *t = self->surface.tris; t; t = t->next) {
    ++n;
  }
  return PyLong_FromLong(n);
}

static PyObject *
surface_tri_area(SurfaceObject *self)
{
  double area = 0;
  for (tri *t = self->surface.tris; t; t = t->next) {
    area += 0.5 * vprod(t->x1, t->y1, t->z1, t->x3, t->y3, t->z3, t->x2, t->y2, t->z2);
  }
  return PyFloat_FromDouble(area);
}

static PyObject *
surface_to_string(SurfaceObject *self)
{
  std::ostringstream os;
  for (quadl *q = self->surface.quads; q; q = q->next) {
    os << "Q (" << q->x1 << "," << q->y1 << "," << q->z1 << ")"
           " (" << q->x2 << "," << q->y2 << "," << q->z2 << ")"
           " (" << q->x3 << "," << q->y3 << "," << q->z3 << ")"
           " (" << q->x4 << "," << q->y4 << "," << q->z4 << ")";
    os << std::endl;
  }
  for (tri *t = self->surface.tris; t; t = t->next) {
    os << "T (" << t->x1 << "," << t->y1 << "," << t->z1 << ")"
           " (" << t->x2 << "," << t->y2 << "," << t->z2 << ")"
           " (" << t->x3 << "," << t->y3 << "," << t->z3 << ")";
    os << std::endl;
  }
  return PyUnicode_FromString(os.str().c_str());
}

static PyMethodDef surface_methods[] = {
  { "_get_name", (PyCFunction) surface_get_name, METH_NOARGS, NULL },
  { "_set_name", (PyCFunction) surface_set_name, METH_O, NULL },
  { "_get_title", (PyCFunction) surface_get_title, METH_NOARGS, NULL },
  { "_set_title", (PyCFunction) surface_set_title, METH_O, NULL },
  { "_add_quad", (PyCFunction) surface_add_quad, METH_VARARGS, NULL },
  { "_add_tri", (PyCFunction) surface_add_tri, METH_VARARGS, NULL },
  //  for testing:
  { "_quad_count", (PyCFunction) surface_quad_count, METH_NOARGS, NULL },
  { "_quad_area", (PyCFunction) surface_quad_area, METH_NOARGS, NULL },
  { "_tri_count", (PyCFunction) surface_tri_count, METH_NOARGS, NULL },
  { "_tri_area", (PyCFunction) surface_tri_area, METH_NOARGS, NULL },
  { "_to_string", (PyCFunction) surface_to_string, METH_NOARGS, NULL },
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

