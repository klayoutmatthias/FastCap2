
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "mulStruct.h"

//  for in-place new
#include <memory>

struct ProblemObject {
  PyObject_HEAD
  ssystem sys;
};

static void
problem_dealloc(ProblemObject *self)
{
  self->sys.~ssystem();
}

static PyObject *
problem_new(PyTypeObject *type, PyObject * /*args*/, PyObject * /*kwds*/)
{
  ProblemObject *self;
  self = (ProblemObject *) type->tp_alloc(type, 0);
  if (self != NULL) {
    new (&self->sys) ssystem;
  }
  return (PyObject *) self;
}

static int
problem_init(ProblemObject *self, PyObject *args, PyObject *kwds)
{
  static char *kwlist[] = {(char *)"title", NULL};
  PyObject *title = NULL;

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O", kwlist,
                                   &title)) {
    return -1;
  }

  if (title) {
    PyObject *title_str = PyObject_Str(title);
    if (!title_str) {
      return -1;
    }
    const char *title_utf8str = PyUnicode_AsUTF8(title_str);
    if (!title_utf8str) {
      return -1;
    }
    self->sys.title = self->sys.heap.strdup(title_utf8str);
    Py_DECREF(title_str);
  }

  return 0;
}

static PyObject *
problem_get_title(ProblemObject *self)
{
  return PyUnicode_FromString(self->sys.title ? self->sys.title : "(null)");
}

static PyObject *
problem_set_title(ProblemObject *self, PyObject *value)
{
  PyObject *title_str = PyObject_Str(value);
  if (!title_str) {
    return NULL;
  }
  const char *title_utf8str = PyUnicode_AsUTF8(title_str);
  if (!title_utf8str) {
    return NULL;
  }
  self->sys.title = self->sys.heap.strdup(title_utf8str);
  Py_RETURN_NONE;
}

static PyObject *
problem_get_perm_factor(ProblemObject *self)
{
  return PyFloat_FromDouble (self->sys.perm_factor);
}

static PyObject *
problem_set_perm_factor(ProblemObject *self, PyObject *args)
{
  double d = 1.0;
  if (!PyArg_ParseTuple(args, "d", &d)) {
    return NULL;
  }

  self->sys.perm_factor = d;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_expansion_order(ProblemObject *self)
{
  return PyLong_FromLong ((long) self->sys.order);
}

static PyObject *
problem_set_expansion_order(ProblemObject *self, PyObject *args)
{
  int i = 0;
  if (!PyArg_ParseTuple(args, "i", &i)) {
    return NULL;
  }

  self->sys.order = i;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_partitioning_depth(ProblemObject *self)
{
  return PyLong_FromLong ((long) self->sys.depth);
}

static PyObject *
problem_set_partitioning_depth(ProblemObject *self, PyObject *args)
{
  int i = 0;
  if (!PyArg_ParseTuple(args, "i", &i)) {
    return NULL;
  }

  self->sys.depth = i;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_iter_tol(ProblemObject *self)
{
  return PyFloat_FromDouble (self->sys.iter_tol);
}

static PyObject *
problem_set_iter_tol(ProblemObject *self, PyObject *args)
{
  double d = 1.0;
  if (!PyArg_ParseTuple(args, "d", &d)) {
    return NULL;
  }

  self->sys.iter_tol = d;
  Py_RETURN_NONE;
}

static PyMethodDef problem_methods[] = {
  { "_get_title", (PyCFunction) problem_get_title, METH_NOARGS, NULL },
  { "_set_title", (PyCFunction) problem_set_title, METH_O, NULL },
  { "_get_perm_factor", (PyCFunction) problem_get_perm_factor, METH_NOARGS, NULL },
  { "_set_perm_factor", (PyCFunction) problem_set_perm_factor, METH_VARARGS, NULL },
  { "_get_expansion_order", (PyCFunction) problem_get_expansion_order, METH_NOARGS, NULL },
  { "_set_expansion_order", (PyCFunction) problem_set_expansion_order, METH_VARARGS, NULL },
  { "_get_partitioning_depth", (PyCFunction) problem_get_partitioning_depth, METH_NOARGS, NULL },
  { "_set_partitioning_depth", (PyCFunction) problem_set_partitioning_depth, METH_VARARGS, NULL },
  { "_get_iter_tol", (PyCFunction) problem_get_iter_tol, METH_NOARGS, NULL },
  { "_set_iter_tol", (PyCFunction) problem_set_iter_tol, METH_VARARGS, NULL },
  {NULL}
};

PyTypeObject problem_type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "fastcap2_core.Problem",
  .tp_basicsize = sizeof(ProblemObject),
  .tp_itemsize = 0,
  .tp_dealloc = (destructor) problem_dealloc,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_methods = problem_methods,
  .tp_init = (initproc) problem_init,
  .tp_new = problem_new,
};
