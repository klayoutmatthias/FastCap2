
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "surface.h"
#include "mulStruct.h"
#include "mulGlobal.h"
#include "input.h"
#include "zbufGlobal.h"
#include "fastcap_solve.h"
#include "zbuf2fastcap.h"
#include "quickif.h"
#include "matrix.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <cstring>


extern PyTypeObject surface_type;

struct PyProblemObject {
  PyObject_HEAD
  ssystem sys;
};

static void
problem_dealloc(PyProblemObject *self)
{
  self->sys.~ssystem();
}

static PyObject *
problem_new(PyTypeObject *type, PyObject * /*args*/, PyObject * /*kwds*/)
{
  PyProblemObject *self;
  self = (PyProblemObject *) type->tp_alloc(type, 0);
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
problem_init(PyProblemObject *self, PyObject *args, PyObject *kwds)
{
  static char *kwlist[] = {(char *)"title", NULL};
  const char *title = 0;

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "|s", kwlist, &title)) {
    return -1;
  }

  if (title) {
    self->sys.title = self->sys.heap.strdup(title);
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
    if (*cp == ',') {
      PyErr_Format(PyExc_RuntimeError, "',' character is not allowed in this conductor name: '%s'", name);
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
problem_get_title(PyProblemObject *self)
{
  return PyUnicode_FromString(self->sys.title ? self->sys.title : "(null)");
}

static PyObject *
problem_set_title(PyProblemObject *self, PyObject *value)
{
  self->sys.reset_read();

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
problem_get_perm_factor(PyProblemObject *self)
{
  return PyFloat_FromDouble (self->sys.perm_factor);
}

static PyObject *
problem_set_perm_factor(PyProblemObject *self, PyObject *args)
{
  double d = 1.0;
  if (!PyArg_ParseTuple(args, "d", &d)) {
    return NULL;
  }

  self->sys.perm_factor = d;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_expansion_order(PyProblemObject *self)
{
  return PyLong_FromLong ((long) self->sys.order);
}

static PyObject *
problem_set_expansion_order(PyProblemObject *self, PyObject *args)
{
  int i = 0;
  if (!PyArg_ParseTuple(args, "i", &i)) {
    return NULL;
  }

  self->sys.order = i;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_partitioning_depth(PyProblemObject *self)
{
  return PyLong_FromLong ((long) self->sys.depth);
}

static PyObject *
problem_set_partitioning_depth(PyProblemObject *self, PyObject *args)
{
  int i = 0;
  if (!PyArg_ParseTuple(args, "i", &i)) {
    return NULL;
  }

  self->sys.depth = i;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_iter_tol(PyProblemObject *self)
{
  return PyFloat_FromDouble (self->sys.iter_tol);
}

static PyObject *
problem_set_iter_tol(PyProblemObject *self, PyObject *args)
{
  double d = 1.0;
  if (!PyArg_ParseTuple(args, "d", &d)) {
    return NULL;
  }

  self->sys.iter_tol = d;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_skip_conductors(PyProblemObject *self)
{
  return parse_conductor_list(self->sys.kill_name_list);
}

static PyObject *
problem_set_skip_conductors(PyProblemObject *self, PyObject *value)
{
  self->sys.reset_read();

  char *list = NULL;
  if (value == Py_None) {
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
problem_get_remove_conductors(PyProblemObject *self)
{
  return parse_conductor_list(self->sys.kinp_name_list);
}

static PyObject *
problem_set_remove_conductors(PyProblemObject *self, PyObject *value)
{
  self->sys.reset_read();

  char *list = NULL;
  if (value == Py_None) {
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
problem_get_qps_file_base(PyProblemObject *self)
{
  if (!self->sys.ps_file_base || !self->sys.q_) {
    Py_RETURN_NONE;
  } else {
    return PyUnicode_FromString(self->sys.ps_file_base);
  }
}

static PyObject *
problem_set_qps_file_base(PyProblemObject *self, PyObject *value)
{
  if (value == Py_None) {
    self->sys.ps_file_base = 0;
    self->sys.q_ = FALSE;
  } else {
    PyObject *qps_file_base_str = PyObject_Str(value);
    if (!qps_file_base_str) {
      return NULL;
    }
    const char *qps_file_base_utf8str = PyUnicode_AsUTF8(qps_file_base_str);
    if (!qps_file_base_utf8str) {
      return NULL;
    }
    self->sys.ps_file_base = self->sys.heap.strdup(qps_file_base_utf8str);
    self->sys.q_ = TRUE;
  }
  Py_RETURN_NONE;
}

static PyObject *
problem_get_qps_select_q(PyProblemObject *self)
{
  return parse_conductor_list(self->sys.qpic_name_list);
}

static PyObject *
problem_set_qps_select_q(PyProblemObject *self, PyObject *value)
{
  self->sys.reset_read();

  char *list = NULL;
  if (value == Py_None) {
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
problem_get_qps_remove_q(PyProblemObject *self)
{
  return parse_conductor_list(self->sys.kq_name_list);
}

static PyObject *
problem_set_qps_remove_q(PyProblemObject *self, PyObject *value)
{
  self->sys.reset_read();

  char *list = NULL;
  if (value == Py_None) {
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
problem_get_qps_no_key(PyProblemObject *self)
{
  return PyBool_FromLong (self->sys.rk_);
}

static PyObject *
problem_set_qps_no_key(PyProblemObject *self, PyObject *args)
{
  int b = 0;
  if (!PyArg_ParseTuple(args, "p", &b)) {
    return NULL;
  }

  self->sys.rk_ = b;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_qps_total_charges(PyProblemObject *self)
{
  return PyBool_FromLong (self->sys.dd_);
}

static PyObject *
problem_set_qps_total_charges(PyProblemObject *self, PyObject *args)
{
  int b = 0;
  if (!PyArg_ParseTuple(args, "p", &b)) {
    return NULL;
  }

  self->sys.dd_ = b;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_ps_no_dielectric(PyProblemObject *self)
{
  return PyBool_FromLong (self->sys.rd_);
}

static PyObject *
problem_set_ps_no_dielectric(PyProblemObject *self, PyObject *args)
{
  int b = 0;
  if (!PyArg_ParseTuple(args, "p", &b)) {
    return NULL;
  }

  self->sys.rd_ = b;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_ps_no_showpage(PyProblemObject *self)
{
  return PyBool_FromLong (self->sys.s_);
}

static PyObject *
problem_set_ps_no_showpage(PyProblemObject *self, PyObject *args)
{
  int b = 0;
  if (!PyArg_ParseTuple(args, "p", &b)) {
    return NULL;
  }

  self->sys.s_ = b;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_ps_number_faces(PyProblemObject *self)
{
  return PyBool_FromLong (self->sys.n_);
}

static PyObject *
problem_set_ps_number_faces(PyProblemObject *self, PyObject *args)
{
  int b = 0;
  if (!PyArg_ParseTuple(args, "p", &b)) {
    return NULL;
  }

  self->sys.n_ = b;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_ps_show_hidden(PyProblemObject *self)
{
  return PyBool_FromLong (self->sys.f_);
}

static PyObject *
problem_set_ps_show_hidden(PyProblemObject *self, PyObject *args)
{
  int b = 0;
  if (!PyArg_ParseTuple(args, "p", &b)) {
    return NULL;
  }

  self->sys.f_ = b;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_ps_azimuth(PyProblemObject *self)
{
  return PyFloat_FromDouble (self->sys.azimuth);
}

static PyObject *
problem_set_ps_azimuth(PyProblemObject *self, PyObject *args)
{
  double d = 1.0;
  if (!PyArg_ParseTuple(args, "d", &d)) {
    return NULL;
  }

  self->sys.azimuth = d;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_ps_elevation(PyProblemObject *self)
{
  return PyFloat_FromDouble (self->sys.elevation);
}

static PyObject *
problem_set_ps_elevation(PyProblemObject *self, PyObject *args)
{
  double d = 1.0;
  if (!PyArg_ParseTuple(args, "d", &d)) {
    return NULL;
  }

  self->sys.elevation = d;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_ps_rotation(PyProblemObject *self)
{
  return PyFloat_FromDouble (self->sys.rotation);
}

static PyObject *
problem_set_ps_rotation(PyProblemObject *self, PyObject *args)
{
  double d = 1.0;
  if (!PyArg_ParseTuple(args, "d", &d)) {
    return NULL;
  }

  self->sys.rotation = d;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_ps_upaxis(PyProblemObject *self)
{
  return PyLong_FromLong ((long) self->sys.up_axis);
}

static PyObject *
problem_set_ps_upaxis(PyProblemObject *self, PyObject *args)
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
problem_get_ps_distance(PyProblemObject *self)
{
  return PyFloat_FromDouble (self->sys.distance);
}

static PyObject *
problem_set_ps_distance(PyProblemObject *self, PyObject *args)
{
  double d = 1.0;
  if (!PyArg_ParseTuple(args, "d", &d)) {
    return NULL;
  }

  self->sys.distance = d;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_ps_scale(PyProblemObject *self)
{
  return PyFloat_FromDouble (self->sys.scale);
}

static PyObject *
problem_set_ps_scale(PyProblemObject *self, PyObject *args)
{
  double d = 1.0;
  if (!PyArg_ParseTuple(args, "d", &d)) {
    return NULL;
  }

  self->sys.scale = d;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_ps_linewidth(PyProblemObject *self)
{
  return PyFloat_FromDouble (self->sys.linewd);
}

static PyObject *
problem_set_ps_linewidth(PyProblemObject *self, PyObject *args)
{
  double d = 1.0;
  if (!PyArg_ParseTuple(args, "d", &d)) {
    return NULL;
  }

  self->sys.linewd = d;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_ps_axislength(PyProblemObject *self)
{
  return PyFloat_FromDouble (self->sys.axeslen);
}

static PyObject *
problem_set_ps_axislength(PyProblemObject *self, PyObject *args)
{
  double d = 1.0;
  if (!PyArg_ParseTuple(args, "d", &d)) {
    return NULL;
  }

  self->sys.axeslen = d;
  Py_RETURN_NONE;
}

static PyObject *
problem_get_verbose(PyProblemObject *self)
{
  return PyBool_FromLong (self->sys.log != NULL);
}

static PyObject *
problem_set_verbose(PyProblemObject *self, PyObject *args)
{
  int b = 0;
  if (!PyArg_ParseTuple(args, "p", &b)) {
    return NULL;
  }

  self->sys.log = (b ? stdout : NULL);
  Py_RETURN_NONE;
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
vector_to_pylist(const Vector3d &v)
{
  PyObject *pyv = PyList_New(3);
  for (int i = 0; i < 3; ++i) {
    PyList_SetItem(pyv, i, PyFloat_FromDouble(v[i]));
  }
  return pyv;
}

static double
round_sincos(double v)
{
  double epsilon = 1e-12;
  if (fabs(v) < epsilon) {
    return 0.0;
  } else if (fabs(v + 1.0) < epsilon) {
    return -1.0;
  } else if (fabs(v - 1.0) < epsilon) {
    return 1.0;
  } else {
    return v;
  }
}
static Matrix3d
build_transformation(int flipx, int flipy, int flipz,
                     double rotx, double roty, double rotz,
                     double scalex, double scaley, double scalez)
{
  Matrix3d res = unity<3>();
  res.set(0, 0, scalex * (flipx ? -1.0 : 1.0));
  res.set(1, 1, scaley * (flipy ? -1.0 : 1.0));
  res.set(2, 2, scalez * (flipz ? -1.0 : 1.0));

  double rotn[3] = { rotx, roty, rotz };

  for (unsigned int n = 0; n < 3; ++n) {

    unsigned int n1 = (n + 1) % 3;
    unsigned int n2 = (n + 2) % 3;

    double a = rotn[n] / 180.0 * M_PI;
    double sa = round_sincos(sin(a));
    double ca = round_sincos(cos(a));

    Matrix3d rot;
    rot.set(n, n, 1.0);
    rot.set(n1, n1, ca);
    rot.set(n1, n2, n == 1 ? sa : -sa);
    rot.set(n2, n1, n == 1 ? -sa : sa);
    rot.set(n2, n2, ca);

    res = res * rot;

  }

  return res;
}

static PyObject *
problem_load_or_add(PyProblemObject *self, PyObject *args, bool load)
{
  const char *filename = 0;
  SurfaceData *surf_data = 0;
  int link = 0;
  const char *group = 0;
  int kind = 0;
  int ref_point_inside = 0;
  double outer_perm = 1.0, inner_perm = 1.0;
  PyObject *d = NULL;
  PyObject *r = NULL;
  int flipx = 0, flipy = 0, flipz = 0;
  double rotx = 0.0, roty = 0.0, rotz = 0.0;
  double scalex = 1.0, scaley = 1.0, scalez = 1.0;

  self->sys.reset_read();

  if (load) {

    if (!PyArg_ParseTuple(args, "spzipddOOpppdddddd", &filename, &link, &group, &kind,
                                                      &ref_point_inside, &outer_perm, &inner_perm,
                                                      &d, &r, &flipx, &flipy, &flipz, &rotx, &roty, &rotz,
                                                      &scalex, &scaley, &scalez)) {
      return NULL;
    }

  } else {

    PyObject *py_surf = NULL;
    if (!PyArg_ParseTuple(args, "OpzipddOOpppdddddd", &py_surf, &link, &group, &kind,
                                                      &ref_point_inside, &outer_perm, &inner_perm,
                                                      &d, &r, &flipx, &flipy, &flipz, &rotx, &roty, &rotz,
                                                      &scalex, &scaley, &scalez)) {
      return NULL;
    }

    if (!PyObject_TypeCheck(py_surf, &surface_type)) {
      PyErr_SetString(PyExc_RuntimeError, "First argument is not of fastcap2.Surface type");
      return NULL;
    }

    surf_data = ((PySurfaceObject *)py_surf)->surface.clone(self->sys.heap);

    if (!surf_data->name && (kind == CONDTR || kind == BOTH)) {
      PyErr_SetString(PyExc_RuntimeError, "Surface needs to have name for conductor type");
      return NULL;
    }

  }

  Vector3d dv, rv;
  if (!parse_vector(d, dv)) {
    return NULL;
  }
  if (!parse_vector(r, rv)) {
    return NULL;
  }

  //  find end of list
  Surface *eol = self->sys.surf_list;
  if (eol) {
    for ( ; eol->next; eol = eol->next)
      ;
  }

  //  if linked, mark previous object as chained
  if (eol && link) {
    eol->end_of_chain = FALSE;
  } else {
    link = false;
  }

  //  append new surface element
  Surface *new_surf = self->sys.heap.create<Surface>(AMSC);
  if (eol) {
    eol->next = new_surf;
  } else {
    self->sys.surf_list = new_surf;
  }

  Matrix3d rot = build_transformation(flipx, flipy, flipz, rotx, roty, rotz, scalex, scaley, scalez);

  new_surf->type = CONDTR;
  new_surf->name = self->sys.heap.strdup(filename);
  new_surf->surf_data = surf_data;
  new_surf->outer_perm = outer_perm;
  new_surf->inner_perm = inner_perm;
  new_surf->type = kind;
  new_surf->ref_inside = ref_point_inside;
  new_surf->ref = rv;
  new_surf->trans = dv;
  new_surf->rot = rot;
  new_surf->end_of_chain = TRUE;

  //  set up group name
  if (link) {
    new_surf->group_name = eol->group_name;
  } else if (group) {
    new_surf->group_name = self->sys.heap.strdup(group);
  } else {
    char group_name[BUFSIZ];
    sprintf(group_name, "GROUP%d", ++self->sys.group_cnt);
    new_surf->group_name = self->sys.heap.strdup(group_name);
  }

  Py_RETURN_NONE;
}

static PyObject *
problem_load(PyProblemObject *self, PyObject *args)
{
  return problem_load_or_add(self, args, true /*load*/);
}

static PyObject *
problem_add(PyProblemObject *self, PyObject *args)
{
  return problem_load_or_add(self, args, false /*add*/);
}

static PyObject *
problem_load_list(PyProblemObject *self, PyObject *args)
{
  const char *filename = 0;
  if (!PyArg_ParseTuple(args, "s", &filename)) {
    return NULL;
  }

  self->sys.reset_read();

  try {
    read_list_file(&self->sys, &self->sys.surf_list, filename);
  } catch (std::runtime_error &ex) {
    return raise_error (ex);
  }

  Py_RETURN_NONE;
}

static PyObject *
problem_solve(PyProblemObject *self)
{
  double **capmat = 0;

  try {
    capmat = fastcap_solve(&self->sys);
  } catch (std::runtime_error &ex) {
    return raise_error (ex);
  }

  if (capmat) {

    int size = capmatrix_size(&self->sys);

    double mult = FPIEPS * self->sys.perm_factor;

    //  NOTE: the cap matrix if 1 based!
    PyObject *res = PyList_New(size);
    if (!res) {
      return NULL;
    }

    for (int i = 1; i <= size; ++i) {
      PyObject *row = PyList_New(size);
      if (! row) {
        Py_DECREF(res);
        return NULL;
      }
      PyList_SetItem(res, i - 1, row);
      for (int j = 1; j <= size; ++j) {
        PyList_SetItem(row, j - 1, PyFloat_FromDouble(mult * capmat[i][j]));
      }
    }

    return res;

  }

  Py_RETURN_NONE;
}

static PyObject *
problem_conductors(PyProblemObject *self)
{
  PyObject *res = PyList_New(0);
  if (!res) {
    return NULL;
  }

  //  building or updating the conductor list is a side effect of this function:
  try {
    build_charge_list(&self->sys);
  } catch (std::runtime_error &ex) {
    return raise_error(ex);
  }

  int i = 0;
  for (Name *cur_name = self->sys.cond_names; cur_name; cur_name = cur_name->next, ++i) {
    if (self->sys.kinp_num_list.find(i + 1) != self->sys.kinp_num_list.end()) {
      continue;
    }
    PyObject *name_str = PyUnicode_FromString(cur_name->last_alias ());
    if (!name_str) {
      Py_DECREF(res);
      return NULL;
    }
    PyList_Append(res, name_str);
  }

  return res;
}

static PyObject *
problem_dump_ps(PyProblemObject *self, PyObject *args)
{
  const char *filename = 0;
  if (!PyArg_ParseTuple(args, "s", &filename)) {
    return NULL;
  }

  try {

    charge *chglist = build_charge_list(&self->sys);
    if (!chglist) {
      PyErr_SetString(PyExc_RuntimeError, "Geometry is empty - cannot dump to PS");
      return NULL;
    }

    dump_ps_geometry(&self->sys, filename, chglist, NULL, self->sys.dd_);

  } catch (std::runtime_error &ex) {
    return raise_error(ex);
  }

  Py_RETURN_NONE;
}

static PyObject *
problem_extent(PyProblemObject *self)
{
  try {

    charge *chglist = build_charge_list(&self->sys);
    if (!chglist) {
      PyErr_SetString(PyExc_RuntimeError, "Geometry is empty - cannot dump to PS");
      return NULL;
    }

    Vector3d min, max;
    bool first = true;
    while (chglist) {
      Vector3d x(chglist->X), y(chglist->Y), z(chglist->Z);
      Vector3d o(chglist->x, chglist->y, chglist->z);
      for (int i = 0; i < chglist->shape; ++i) {
        Vector3d c = chglist->corner[i][0] * x + chglist->corner[i][1] * y + chglist->corner[i][2] * z + o;
        if (first) {
          min = max = c;
          first = false;
        } else {
          for (int j = 0; j < 3; ++j) {
            min[j] = std::min(min[j], c[j]);
          }
          for (int j = 0; j < 3; ++j) {
            max[j] = std::max(max[j], c[j]);
          }
        }
      }
      chglist = chglist->next;
    }

    PyObject *res = PyList_New(2);
    PyList_SetItem(res, 0, vector_to_pylist(min));
    PyList_SetItem(res, 1, vector_to_pylist(max));
    return res;

  } catch (std::runtime_error &ex) {
    return raise_error(ex);
  }
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
  { "_get_qps_file_base", (PyCFunction) problem_get_qps_file_base, METH_NOARGS, NULL },
  { "_set_qps_file_base", (PyCFunction) problem_set_qps_file_base, METH_O, NULL },
  { "_get_qps_select_q", (PyCFunction) problem_get_qps_select_q, METH_NOARGS, NULL },
  { "_set_qps_select_q", (PyCFunction) problem_set_qps_select_q, METH_O, NULL },
  { "_get_qps_remove_q", (PyCFunction) problem_get_qps_remove_q, METH_NOARGS, NULL },
  { "_set_qps_remove_q", (PyCFunction) problem_set_qps_remove_q, METH_O, NULL },
  { "_get_qps_no_key", (PyCFunction) problem_get_qps_no_key, METH_NOARGS, NULL },
  { "_set_qps_no_key", (PyCFunction) problem_set_qps_no_key, METH_VARARGS, NULL },
  { "_get_qps_total_charges", (PyCFunction) problem_get_qps_total_charges, METH_NOARGS, NULL },
  { "_set_qps_total_charges", (PyCFunction) problem_set_qps_total_charges, METH_VARARGS, NULL },
  { "_get_ps_no_dielectric", (PyCFunction) problem_get_ps_no_dielectric, METH_NOARGS, NULL },
  { "_set_ps_no_dielectric", (PyCFunction) problem_set_ps_no_dielectric, METH_VARARGS, NULL },
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
  { "_add", (PyCFunction) problem_add, METH_VARARGS, NULL },
  { "_solve", (PyCFunction) problem_solve, METH_NOARGS, NULL },
  { "_conductors", (PyCFunction) problem_conductors, METH_NOARGS, NULL },
  { "_dump_ps", (PyCFunction) problem_dump_ps, METH_VARARGS, NULL },
  { "_extent", (PyCFunction) problem_extent, METH_NOARGS, NULL },
  {NULL}
};

PyTypeObject problem_type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "fastcap2_core.Problem",
  .tp_basicsize = sizeof(PyProblemObject),
  .tp_itemsize = 0,
  .tp_dealloc = (destructor) problem_dealloc,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_methods = problem_methods,
  .tp_init = (initproc) problem_init,
  .tp_new = problem_new,
};
