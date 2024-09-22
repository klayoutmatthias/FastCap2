
#include "surface.h"

#include <memory>
#include <sstream>

// --------------------------------------------------------------------------

PySurface::PySurface()
  : SurfaceData(), heap()
{
}

void PySurface::add_quad(int cond_num,
                         const Vector3d &p1,
                         const Vector3d &p2,
                         const Vector3d &p3,
                         const Vector3d &p4,
                         const Vector3d *rp)
{
  quadl *new_quad = heap.alloc<quadl>(1);
  new_quad->cond = cond_num;
  new_quad->p1 = p1;
  new_quad->p2 = p2;
  new_quad->p3 = p3;
  new_quad->p4 = p4;
  new_quad->rp = (rp != 0 ? *rp : Vector3d());
  new_quad->has_rp = (rp != 0);
  new_quad->next = quads;
  quads = new_quad;
}

void PySurface::add_tri(int cond_num,
                         const Vector3d &p1,
                         const Vector3d &p2,
                         const Vector3d &p3,
                         const Vector3d *rp)
{
  tri *new_tri = heap.alloc<tri>(1);
  new_tri->cond = cond_num;
  new_tri->p1 = p1;
  new_tri->p2 = p2;
  new_tri->p3 = p3;
  new_tri->rp = (rp != 0 ? *rp : Vector3d());
  new_tri->has_rp = (rp != 0);
  new_tri->next = tris;
  tris = new_tri;
}

void PySurface::set_name(const char *n)
{
  if (!n) {
    name = 0;
  } else {
    name = heap.strdup(n);
  }
}

void PySurface::set_title(const char *n)
{
  if (!n) {
    title = 0;
  } else {
    title = heap.strdup(n);
  }
}

// --------------------------------------------------------------------------

static void
surface_dealloc(PySurfaceObject *self)
{
  self->surface.~PySurface();
}

static PyObject *
surface_new(PyTypeObject *type, PyObject * /*args*/, PyObject * /*kwds*/)
{
  PySurfaceObject *self;
  self = (PySurfaceObject *) type->tp_alloc(type, 0);
  if (self != NULL) {
    new (&self->surface) PySurface;
  }

  return (PyObject *) self;
}

static int
surface_init(PySurfaceObject *self, PyObject *args, PyObject *kwds)
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

static bool
parse_vector(PyObject *arg, Vector3d &v)
{
  double x, y, z;
  if (!PyArg_ParseTuple(arg, "ddd", &x, &y, &z)) {
    return false;
  } else {
    v = Vector3d(x, y, z);
    return true;
  }
}

static PyObject *
surface_add_quad(PySurfaceObject *self, PyObject *args)
{
  PyObject *p1, *p2, *p3, *p4, *rp = NULL;
  if (!PyArg_ParseTuple(args, "OOOO|O", &p1, &p2, &p3, &p4, &rp)) {
    return NULL;
  }

  Vector3d c1, c2, c3, c4, crp;

  int ok = parse_vector(p1, c1) &&
           parse_vector(p2, c2) &&
           parse_vector(p3, c3) &&
           parse_vector(p4, c4) &&
           (rp == NULL || parse_vector(rp, crp));

  if (! ok) {
    return NULL;
  } else {
    self->surface.add_quad(0, c1, c2, c3, c4, rp == NULL ? 0 : &crp);
    Py_RETURN_NONE;
  }
}

static PyObject *
surface_add_tri(PySurfaceObject *self, PyObject *args)
{
  PyObject *p1, *p2, *p3, *rp = NULL;
  if (!PyArg_ParseTuple(args, "OOO|O", &p1, &p2, &p3, &rp)) {
    return NULL;
  }

  Vector3d c1, c2, c3, crp;

  int ok = parse_vector(p1, c1) &&
           parse_vector(p2, c2) &&
           parse_vector(p3, c3) &&
           (rp == NULL || parse_vector(rp, crp));

  if (! ok) {
    return NULL;
  } else {
    self->surface.add_tri(0, c1, c2, c3, rp == NULL ? 0 : &crp);
    Py_RETURN_NONE;
  }
}

static PyObject *
surface_get_name(PySurfaceObject *self)
{
  if (self->surface.name) {
    return PyUnicode_FromString(self->surface.name);
  } else {
    Py_RETURN_NONE;
  }
}

static PyObject *
surface_set_name(PySurfaceObject *self, PyObject *value)
{
  if (value == Py_None) {
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
surface_get_title(PySurfaceObject *self)
{
  if (self->surface.title) {
    return PyUnicode_FromString(self->surface.title);
  } else {
    Py_RETURN_NONE;
  }
}

static PyObject *
surface_set_title(PySurfaceObject *self, PyObject *value)
{
  if (value == Py_None) {
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

static PyObject *
surface_quad_count(PySurfaceObject *self)
{
  size_t n = 0;
  for (quadl *q = self->surface.quads; q; q = q->next) {
    ++n;
  }
  return PyLong_FromLong(n);
}

static PyObject *
surface_quad_area(PySurfaceObject *self)
{
  double area = 0;
  for (quadl *q = self->surface.quads; q; q = q->next) {
    area += 0.5 * cross_prod(q->p4 - q->p1, q->p2 - q->p1).norm();
    area += 0.5 * cross_prod(q->p2 - q->p3, q->p4 - q->p3).norm();
  }
  return PyFloat_FromDouble(area);
}

static PyObject *
surface_tri_count(PySurfaceObject *self)
{
  size_t n = 0;
  for (tri *t = self->surface.tris; t; t = t->next) {
    ++n;
  }
  return PyLong_FromLong(n);
}

static PyObject *
surface_tri_area(PySurfaceObject *self)
{
  double area = 0;
  for (tri *t = self->surface.tris; t; t = t->next) {
    area += 0.5 * cross_prod(t->p3 - t->p1, t->p2 - t->p1).norm();
  }
  return PyFloat_FromDouble(area);
}

static PyObject *
surface_to_string(PySurfaceObject *self)
{
  std::ostringstream os;
  for (quadl *q = self->surface.quads; q; q = q->next) {
    os << "Q " << q->p1.to_string() <<
           " " << q->p2.to_string() <<
           " " << q->p3.to_string() <<
           " " << q->p4.to_string();
    os << std::endl;
  }
  for (tri *t = self->surface.tris; t; t = t->next) {
    os << "T " << t->p1.to_string() <<
           " " << t->p2.to_string() <<
           " " << t->p3.to_string();
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
  .tp_basicsize = sizeof(PySurfaceObject),
  .tp_itemsize = 0,
  .tp_dealloc = (destructor) surface_dealloc,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_methods = surface_methods,
  .tp_init = (initproc) surface_init,
  .tp_new = surface_new,
};

