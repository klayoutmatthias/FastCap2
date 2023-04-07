
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

/*@@@
static PyObject *
problem_name(ProblemObject *self, PyObject *Py_UNUSED(ignored))
{
  if (self->first == NULL) {
    PyErr_SetString(PyExc_AttributeError, "first");
    return NULL;
  }
  if (self->last == NULL) {
    PyErr_SetString(PyExc_AttributeError, "last");
    return NULL;
  }
  return PyUnicode_FromFormat("%S %S", self->first, self->last);
}
@@@*/

static PyMethodDef problem_methods[] = {
/* @@@
  {"name", (PyCFunction) Custom_name, METH_NOARGS,
   "Return the name, combining the first and last name"
  },
*/
  {NULL}
};

static PyObject*
problem_title(ProblemObject *self, void *)
{
  return PyUnicode_FromString(self->sys.title ? self->sys.title : "(null)");
}

static int
problem_set_title(ProblemObject *self, PyObject *value, void *)
{
  PyObject *title_str = PyObject_Str(value);
  if (!title_str) {
    return -1;
  }
  const char *title_utf8str = PyUnicode_AsUTF8(title_str);
  if (!title_utf8str) {
    return -1;
  }
  self->sys.title = self->sys.heap.strdup(title_utf8str);
  return 0;
}

static PyGetSetDef problem_getsets[] = {
  { "title", (getter) problem_title, (setter) problem_set_title,
    "Sets the project title. This is an arbitrary string.",
    NULL },
  //  ...
};

PyTypeObject problem_type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "fastcap2_core.Problem",
  .tp_basicsize = sizeof(ProblemObject),
  .tp_itemsize = 0,
  .tp_dealloc = (destructor) problem_dealloc,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_methods = problem_methods,
  .tp_getset = problem_getsets,
  .tp_init = (initproc) problem_init,
  .tp_new = problem_new,
};
