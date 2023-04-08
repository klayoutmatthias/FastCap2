
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "mulStruct.h"
#include "mulGlobal.h"
#include "input.h"
#include "zbufGlobal.h"
#include "fastcap_solve.h"

//  for in-place new
#include <memory>
#include <stdexcept>
#include <string>
#include <cstring>

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

  //  prepare a dummy argument list
  static const char *arg0 = "Python";
  static const char *argv[] = { arg0 };
  self->sys.argc = 1;
  self->sys.argv = argv;

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
problem_get_ps_select_q(ProblemObject *self)
{
  return parse_conductor_list(self->sys.qpic_name_list);
}

static PyObject *
problem_set_ps_select_q(ProblemObject *self, PyObject *value)
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
problem_get_ps_remove_q(ProblemObject *self)
{
  return parse_conductor_list(self->sys.kq_name_list);
}

static PyObject *
problem_set_ps_remove_q(ProblemObject *self, PyObject *value)
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

static PyObject *
problem_get_ps_no_key(ProblemObject *self)
{
  return PyBool_FromLong (self->sys.rk_);
}

static PyObject *
problem_set_ps_no_key(ProblemObject *self, PyObject *args)
{
  int b = 0;
  if (!PyArg_ParseTuple(args, "p", &b)) {
    return NULL;
  }

  self->sys.rk_ = b;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_ps_no_dielectric(ProblemObject *self)
{
  return PyBool_FromLong (self->sys.rd_);
}

static PyObject *
problem_set_ps_no_dielectric(ProblemObject *self, PyObject *args)
{
  int b = 0;
  if (!PyArg_ParseTuple(args, "p", &b)) {
    return NULL;
  }

  self->sys.rd_ = b;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_ps_total_charges(ProblemObject *self)
{
  return PyBool_FromLong (self->sys.dd_);
}

static PyObject *
problem_set_ps_total_charges(ProblemObject *self, PyObject *args)
{
  int b = 0;
  if (!PyArg_ParseTuple(args, "p", &b)) {
    return NULL;
  }

  self->sys.dd_ = b;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_ps_no_showpage(ProblemObject *self)
{
  return PyBool_FromLong (self->sys.s_);
}

static PyObject *
problem_set_ps_no_showpage(ProblemObject *self, PyObject *args)
{
  int b = 0;
  if (!PyArg_ParseTuple(args, "p", &b)) {
    return NULL;
  }

  self->sys.s_ = b;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_ps_number_faces(ProblemObject *self)
{
  return PyBool_FromLong (self->sys.n_);
}

static PyObject *
problem_set_ps_number_faces(ProblemObject *self, PyObject *args)
{
  int b = 0;
  if (!PyArg_ParseTuple(args, "p", &b)) {
    return NULL;
  }

  self->sys.n_ = b;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_ps_show_hidden(ProblemObject *self)
{
  return PyBool_FromLong (self->sys.f_);
}

static PyObject *
problem_set_ps_show_hidden(ProblemObject *self, PyObject *args)
{
  int b = 0;
  if (!PyArg_ParseTuple(args, "p", &b)) {
    return NULL;
  }

  self->sys.f_ = b;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_ps_azimuth(ProblemObject *self)
{
  return PyFloat_FromDouble (self->sys.azimuth);
}

static PyObject *
problem_set_ps_azimuth(ProblemObject *self, PyObject *args)
{
  double d = 1.0;
  if (!PyArg_ParseTuple(args, "d", &d)) {
    return NULL;
  }

  self->sys.azimuth = d;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_ps_elevation(ProblemObject *self)
{
  return PyFloat_FromDouble (self->sys.elevation);
}

static PyObject *
problem_set_ps_elevation(ProblemObject *self, PyObject *args)
{
  double d = 1.0;
  if (!PyArg_ParseTuple(args, "d", &d)) {
    return NULL;
  }

  self->sys.elevation = d;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_ps_rotation(ProblemObject *self)
{
  return PyFloat_FromDouble (self->sys.rotation);
}

static PyObject *
problem_set_ps_rotation(ProblemObject *self, PyObject *args)
{
  double d = 1.0;
  if (!PyArg_ParseTuple(args, "d", &d)) {
    return NULL;
  }

  self->sys.rotation = d;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_ps_upaxis(ProblemObject *self)
{
  return PyLong_FromLong ((long) self->sys.up_axis);
}

static PyObject *
problem_set_ps_upaxis(ProblemObject *self, PyObject *args)
{
  int i = 0;
  if (!PyArg_ParseTuple(args, "i", &i)) {
    return NULL;
  }

  if (i < XI || i > ZI) {
    PyErr_Format(PyExc_RuntimeError, "'ps_upaxis' value needs to be between %d and %d (but is %d)", XI, ZI, i);
    return NULL;
  }

  self->sys.up_axis = i;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_ps_distance(ProblemObject *self)
{
  return PyFloat_FromDouble (self->sys.distance);
}

static PyObject *
problem_set_ps_distance(ProblemObject *self, PyObject *args)
{
  double d = 1.0;
  if (!PyArg_ParseTuple(args, "d", &d)) {
    return NULL;
  }

  self->sys.distance = d;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_ps_scale(ProblemObject *self)
{
  return PyFloat_FromDouble (self->sys.scale);
}

static PyObject *
problem_set_ps_scale(ProblemObject *self, PyObject *args)
{
  double d = 1.0;
  if (!PyArg_ParseTuple(args, "d", &d)) {
    return NULL;
  }

  self->sys.scale = d;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_ps_linewidth(ProblemObject *self)
{
  return PyFloat_FromDouble (self->sys.linewd);
}

static PyObject *
problem_set_ps_linewidth(ProblemObject *self, PyObject *args)
{
  double d = 1.0;
  if (!PyArg_ParseTuple(args, "d", &d)) {
    return NULL;
  }

  self->sys.linewd = d;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_ps_axislength(ProblemObject *self)
{
  return PyFloat_FromDouble (self->sys.axeslen);
}

static PyObject *
problem_set_ps_axislength(ProblemObject *self, PyObject *args)
{
  double d = 1.0;
  if (!PyArg_ParseTuple(args, "d", &d)) {
    return NULL;
  }

  self->sys.axeslen = d;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_verbose(ProblemObject *self)
{
  return PyBool_FromLong (self->sys.log != NULL);
}

static PyObject *
problem_set_verbose(ProblemObject *self, PyObject *args)
{
  int b = 0;
  if (!PyArg_ParseTuple(args, "p", &b)) {
    return NULL;
  }

  self->sys.log = (b ? stdout : NULL);
  Py_RETURN_NONE;
}

static PyObject *
problem_load(ProblemObject *self, PyObject *args)
{
  const char *filename = 0;
  int link = 0;
  const char *group = 0;
  if (!PyArg_ParseTuple(args, "spz", &filename, &link, &group)) {
    return NULL;
  }

  //  find end of list
  surface *eol = self->sys.surf_list;
  if (link && eol) {
    for ( ; eol->next; eol = eol->next)
      ;
  }

  //  if linked, mark previous object as chained
  if (eol && link) {
    eol->end_of_chain = FALSE;
  }

  //  append new surface element
  surface *new_surf = self->sys.heap.alloc<surface>(1);
  if (eol) {
    eol->next = new_surf;
  } else {
    self->sys.surf_list = new_surf;
  }

  new_surf->type = CONDTR;
  new_surf->name = self->sys.heap.strdup(filename);
  new_surf->outer_perm = self->sys.perm_factor;
  new_surf->end_of_chain = TRUE;

  //  set up group name
  if (group) {
    new_surf->group_name = self->sys.heap.strdup(group);
  } else {
    char group_name[BUFSIZ];
    sprintf(group_name, "GROUP%d", ++self->sys.group_cnt);
    new_surf->group_name = self->sys.heap.strdup(group_name);
  }

  Py_RETURN_NONE;
}

static PyObject *
problem_load_list(ProblemObject *self, PyObject *args)
{
  const char *filename = 0;
  if (!PyArg_ParseTuple(args, "s", &filename)) {
    return NULL;
  }

  try {
    read_list_file (&self->sys, &self->sys.surf_list, filename);
  } catch (std::runtime_error &ex) {
    return raise_error (ex);
  }

  Py_RETURN_NONE;
}

static PyObject *
problem_solve(ProblemObject *self)
{
  double **capmat = 0;

  try {
    capmat = solve(&self->sys);
  } catch (std::runtime_error &ex) {
    return raise_error (ex);
  }

  if (capmat) {

    // ...

  }

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
  { "_get_ps_select_q", (PyCFunction) problem_get_ps_select_q, METH_NOARGS, NULL },
  { "_set_ps_select_q", (PyCFunction) problem_set_ps_select_q, METH_O, NULL },
  { "_get_ps_remove_q", (PyCFunction) problem_get_ps_remove_q, METH_NOARGS, NULL },
  { "_set_ps_remove_q", (PyCFunction) problem_set_ps_remove_q, METH_O, NULL },
  { "_get_ps_no_key", (PyCFunction) problem_get_ps_no_key, METH_NOARGS, NULL },
  { "_set_ps_no_key", (PyCFunction) problem_set_ps_no_key, METH_VARARGS, NULL },
  { "_get_ps_no_dielectric", (PyCFunction) problem_get_ps_no_dielectric, METH_NOARGS, NULL },
  { "_set_ps_no_dielectric", (PyCFunction) problem_set_ps_no_dielectric, METH_VARARGS, NULL },
  { "_get_ps_total_charges", (PyCFunction) problem_get_ps_total_charges, METH_NOARGS, NULL },
  { "_set_ps_total_charges", (PyCFunction) problem_set_ps_total_charges, METH_VARARGS, NULL },
  { "_get_ps_no_showpage", (PyCFunction) problem_get_ps_no_showpage, METH_NOARGS, NULL },
  { "_set_ps_no_showpage", (PyCFunction) problem_set_ps_no_showpage, METH_VARARGS, NULL },
  { "_get_ps_number_faces", (PyCFunction) problem_get_ps_number_faces, METH_NOARGS, NULL },
  { "_set_ps_number_faces", (PyCFunction) problem_set_ps_number_faces, METH_VARARGS, NULL },
  { "_get_ps_show_hidden", (PyCFunction) problem_get_ps_show_hidden, METH_NOARGS, NULL },
  { "_set_ps_show_hidden", (PyCFunction) problem_set_ps_show_hidden, METH_VARARGS, NULL },
  { "_get_ps_azimuth", (PyCFunction) problem_get_ps_azimuth, METH_NOARGS, NULL },
  { "_set_ps_azimuth", (PyCFunction) problem_set_ps_azimuth, METH_VARARGS, NULL },
  { "_get_ps_elevation", (PyCFunction) problem_get_ps_elevation, METH_NOARGS, NULL },
  { "_set_ps_elevation", (PyCFunction) problem_set_ps_elevation, METH_VARARGS, NULL },
  { "_get_ps_rotation", (PyCFunction) problem_get_ps_rotation, METH_NOARGS, NULL },
  { "_set_ps_rotation", (PyCFunction) problem_set_ps_rotation, METH_VARARGS, NULL },
  { "_get_ps_upaxis", (PyCFunction) problem_get_ps_upaxis, METH_NOARGS, NULL },
  { "_set_ps_upaxis", (PyCFunction) problem_set_ps_upaxis, METH_VARARGS, NULL },
  { "_get_ps_distance", (PyCFunction) problem_get_ps_distance, METH_NOARGS, NULL },
  { "_set_ps_distance", (PyCFunction) problem_set_ps_distance, METH_VARARGS, NULL },
  { "_get_ps_scale", (PyCFunction) problem_get_ps_scale, METH_NOARGS, NULL },
  { "_set_ps_scale", (PyCFunction) problem_set_ps_scale, METH_VARARGS, NULL },
  { "_get_ps_linewidth", (PyCFunction) problem_get_ps_linewidth, METH_NOARGS, NULL },
  { "_set_ps_linewidth", (PyCFunction) problem_set_ps_linewidth, METH_VARARGS, NULL },
  { "_get_ps_axislength", (PyCFunction) problem_get_ps_axislength, METH_NOARGS, NULL },
  { "_set_ps_axislength", (PyCFunction) problem_set_ps_axislength, METH_VARARGS, NULL },
  { "_get_verbose", (PyCFunction) problem_get_verbose, METH_NOARGS, NULL },
  { "_set_verbose", (PyCFunction) problem_set_verbose, METH_VARARGS, NULL },
  { "_load", (PyCFunction) problem_load, METH_VARARGS, NULL },
  { "_load_list", (PyCFunction) problem_load_list, METH_VARARGS, NULL },
  { "_solve", (PyCFunction) problem_solve, METH_NOARGS, NULL },
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
