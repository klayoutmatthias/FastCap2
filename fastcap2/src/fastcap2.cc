
#define PY_SSIZE_T_CLEAN
#include <Python.h>

extern PyTypeObject problem_type;
extern PyTypeObject surface_type;

static struct PyModuleDef fastcap2_module = {
  PyModuleDef_HEAD_INIT,
  .m_name = "fastcap2_core",
  .m_size = -1
};

PyMODINIT_FUNC
PyInit_fastcap2_core()
{
  PyObject *m;

  if (PyType_Ready(&problem_type) < 0) {
    return NULL;
  }
  if (PyType_Ready(&surface_type) < 0) {
    return NULL;
  }

  m = PyModule_Create(&fastcap2_module);
  if (m == NULL) {
    return NULL;
  }

  Py_INCREF (&problem_type);
  if (PyModule_AddObject(m, "Problem", (PyObject *) &problem_type) < 0) {
    Py_DECREF(&problem_type);
    Py_DECREF(m);
    return NULL;
  }

  Py_INCREF (&surface_type);
  if (PyModule_AddObject(m, "Surface", (PyObject *) &surface_type) < 0) {
    Py_DECREF(&surface_type);
    Py_DECREF(m);
    return NULL;
  }

  return m;
}
