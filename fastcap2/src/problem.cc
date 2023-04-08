
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "mulStruct.h"

//  for in-place new
#include <memory>
#include <stdexcept>
#include <string>
#include <cstring>

static PyObject *raise_error(std::runtime_error &ex)
{
  PyErr_SetString(PyExc_RuntimeError, ex.what());
  return NULL;
}

static bool check_conductor_name(const char *name)
{
  if (!name || !*name) {
    PyErr_SetString(PyExc_RuntimeError, "A conductor name must not be an empty string");
    return false;
  }
  for (const char *cp = name; *cp; ++cp) {
    if (*cp == '%' || *cp == ',') {
      PyErr_Format(PyExc_RuntimeError, "'%%' or ',' characters are not allowed in this conductor name: '%s'", name);
      return false;
    }
  }
  return true;
}

static char *make_conductor_list(ssystem *sys, PyObject *list)
{
  if (!PyList_Check(list)) {
    PyErr_SetString(PyExc_RuntimeError, "Expected a list of conductor name strings for argument");
    return 0;
  }

  std::string csl;

  Py_ssize_t n = PyList_Size(list);
  for (Py_ssize_t i = 0; i < n; ++i) {
    PyObject *nstr = PyObject_Str(PyList_GetItem(list, i));
    if (!nstr) {
      return 0;
    }
    const char *nstr_utf8 = PyUnicode_AsUTF8(nstr);
    if (!nstr_utf8) {
      return 0;
    }
    if (!check_conductor_name (nstr_utf8)) {
      return 0;
    }
    if (i > 0) {
      csl += ",";
    }
    csl += nstr_utf8;
  }

  return sys->heap.strdup(csl.c_str());
}

static PyObject *parse_conductor_list(const char *l)
{
  if (!l) {
    Py_RETURN_NONE;
  }

  PyObject *list = PyList_New(0);
  if (! list) {
    return list;
  }

  const char *cp0 = l;
  for (const char *cp = l; *cp; ++cp) {
    if (cp[1] == ',' || !cp[1]) {
      PyObject *str = PyUnicode_FromStringAndSize(cp0, cp - cp0 + 1);
      if (!str) {
        Py_DECREF(list);
        return NULL;
      }
      PyList_Append(list, str);
      if (cp[1]) {
        ++cp;
        cp0 = cp + 1;
      }
    }
  }

  return list;
}

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

static PyObject *
problem_get_skip_conductors(ProblemObject *self)
{
  return parse_conductor_list(self->sys.kill_name_list);
}

static PyObject *
problem_set_skip_conductors(ProblemObject *self, PyObject *value)
{
  char *list = NULL;
  if (Py_IsNone(value)) {
    //  set list to NULL
  } else {
    list = make_conductor_list(&self->sys, value);
    if (!list) {
      return NULL;
    }
  }
  self->sys.kill_name_list = list;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_remove_conductors(ProblemObject *self)
{
  return parse_conductor_list(self->sys.kinp_name_list);
}

static PyObject *
problem_set_remove_conductors(ProblemObject *self, PyObject *value)
{
  char *list = NULL;
  if (Py_IsNone(value)) {
    //  set list to NULL
  } else {
    list = make_conductor_list(&self->sys, value);
    if (!list) {
      return NULL;
    }
  }
  self->sys.kinp_name_list = list;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_select_q_conductors(ProblemObject *self)
{
  return parse_conductor_list(self->sys.qpic_name_list);
}

static PyObject *
problem_set_select_q_conductors(ProblemObject *self, PyObject *value)
{
  char *list = NULL;
  if (Py_IsNone(value)) {
    //  set list to NULL
  } else {
    list = make_conductor_list(&self->sys, value);
    if (!list) {
      return NULL;
    }
  }
  self->sys.qpic_name_list = list;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_remove_q_conductors(ProblemObject *self)
{
  return parse_conductor_list(self->sys.kq_name_list);
}

static PyObject *
problem_set_remove_q_conductors(ProblemObject *self, PyObject *value)
{
  char *list = NULL;
  if (Py_IsNone(value)) {
    //  set list to NULL
  } else {
    list = make_conductor_list(&self->sys, value);
    if (!list) {
      return NULL;
    }
  }
  self->sys.kq_name_list = list;
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
  { "_get_skip_conductors", (PyCFunction) problem_get_skip_conductors, METH_NOARGS, NULL },
  { "_set_skip_conductors", (PyCFunction) problem_set_skip_conductors, METH_O, NULL },
  { "_get_remove_conductors", (PyCFunction) problem_get_remove_conductors, METH_NOARGS, NULL },
  { "_set_remove_conductors", (PyCFunction) problem_set_remove_conductors, METH_O, NULL },
  { "_get_select_q_conductors", (PyCFunction) problem_get_select_q_conductors, METH_NOARGS, NULL },
  { "_set_select_q_conductors", (PyCFunction) problem_set_select_q_conductors, METH_O, NULL },
  { "_get_remove_q_conductors", (PyCFunction) problem_get_remove_q_conductors, METH_NOARGS, NULL },
  { "_set_remove_q_conductors", (PyCFunction) problem_set_remove_q_conductors, METH_O, NULL },
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
