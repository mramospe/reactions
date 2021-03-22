#ifndef REACTIONS_PYTHON_DATABASE_HPP
#define REACTIONS_PYTHON_DATABASE_HPP

#include "reactions/database_pdg.hpp"

#define PY_SSIZE_T_CLEAN
#include "Python.h"

#define REACTIONS_INSTANCE_ACCESSOR "__instance"

#define REACTIONS_PYTHON_UPDATE_DATABASE_TYPE(type, self)                      \
  if (PyDict_SetItemString(type->tp_dict, REACTIONS_INSTANCE_ACCESSOR,         \
                           (PyObject *)self) != 0) {                           \
    Py_DecRef((PyObject *)self);                                               \
    PyErr_SetString(PyExc_RuntimeError,                                        \
                    "Unable to define the database as a singleton ("           \
                    "internal error); please report the bug");                 \
    return NULL;                                                               \
  }                                                                            \
  PyType_Modified(type);

#define REACTIONS_PYTHON_CHECK_DATABASE(database)                              \
  if (!database) {                                                             \
    PyErr_SetString(PyExc_RuntimeError,                                        \
                    "Unable to access the database instance (internal "        \
                    "error); please report the bug");                          \
    return -1;                                                                 \
  }

#define REACTIONS_PYTHON_DATABASE_CATCH_ERRORS(database)                       \
  catch (exceptions::syntax_error & e) {                                       \
    PyErr_SetString(SyntaxError, e.what());                                    \
    Py_DecRef((PyObject *)database);                                           \
    return -1;                                                                 \
  }                                                                            \
  catch (exceptions::database_error & e) {                                     \
    PyErr_SetString(DatabaseError, e.what());                                  \
    Py_DecRef((PyObject *)database);                                           \
    return -1;                                                                 \
  }                                                                            \
  catch (exceptions::lookup_error & e) {                                       \
    PyErr_SetString(LookupError, e.what());                                    \
    Py_DecRef((PyObject *)database);                                           \
    return -1;                                                                 \
  }                                                                            \
  catch (...) {                                                                \
    Py_DecRef((PyObject *)database);                                           \
    return -1;                                                                 \
  }

/// Object for a PDG database
typedef struct {
  PyObject_HEAD reactions::database_pdg::database *database = nullptr;
} DatabasePDG;

// Create a new node
static PyObject *DatabasePDG_new(PyTypeObject *type, PyObject *Py_UNUSED(args),
                                 PyObject *Py_UNUSED(kwargs));

// This is an abstract class; do nothing
static int DatabasePDG_init(DatabasePDG *self, PyObject *Py_UNUSED(args),
                            PyObject *Py_UNUSED(kwds)) {
  return 0;
}

// Disable the internal cache
static PyObject *DatabasePDG_disable_cache(DatabasePDG *self) {
  self->database->disable_cache();
  Py_RETURN_NONE;
}

// Enable the internal cache
static PyObject *DatabasePDG_enable_cache(DatabasePDG *self) {
  self->database->enable_cache();
  Py_RETURN_NONE;
}

// Set the location of the PDG database
static PyObject *DatabasePDG_get_database(DatabasePDG *self) {

  return PyUnicode_FromString(self->database->get_database_path().c_str());
}

// Set the location of the PDG database
static PyObject *DatabasePDG_set_database(DatabasePDG *self, PyObject *args) {

  const char *str = nullptr;
  if (!PyArg_ParseTuple(args, "s", &str))
    return NULL;

  self->database->set_database_path(str);

  Py_RETURN_NONE;
}

// Methods of the DatabasePDG class
static PyMethodDef DatabasePDG_methods[] = {
    {"disable_cache", (PyCFunction)DatabasePDG_disable_cache, METH_NOARGS,
     "Enable the internal cache"},
    {"enable_cache", (PyCFunction)DatabasePDG_enable_cache, METH_NOARGS,
     "Enable the internal cache"},
    {"get_database", (PyCFunction)DatabasePDG_get_database, METH_NOARGS,
     "Get the path to the database file"},
    {"set_database", (PyCFunction)DatabasePDG_set_database, METH_VARARGS,
     "Set the path to the database file. If the cache is enabled, reloads the "
     "data."},
    {NULL, NULL, 0, NULL}};

// Definition of the DatabasePDG type
static PyTypeObject DatabasePDGType = {
    PyVarObject_HEAD_INIT(NULL, 0) "reactions.pdg_database", /* tp_name */
    sizeof(DatabasePDG),                                     /* tp_basicsize */
    0,                                                       /* tp_itemsize */
    0,                                                       /* tp_dealloc */
    0,                          /* tp_vectorcall_offset */
    0,                          /* tp_getattr */
    0,                          /* tp_setattr */
    0,                          /* tp_as_async */
    0,                          /* tp_repr */
    0,                          /* tp_as_number */
    0,                          /* tp_as_sequence */
    0,                          /* tp_as_mapping */
    0,                          /* tp_hash */
    0,                          /* tp_call */
    0,                          /* tp_str */
    0,                          /* tp_getattro */
    0,                          /* tp_setattro */
    0,                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,         /* tp_flags */
    "pdg_database object",      /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    DatabasePDG_methods,        /* tp_methods */
    0,                          /* tp_members */
    0,                          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    (initproc)DatabasePDG_init, /* tp_init */
    0,                          /* tp_alloc */
    DatabasePDG_new,            /* tp_new */
};

// Create a new node
static PyObject *DatabasePDG_new(PyTypeObject *type, PyObject *Py_UNUSED(args),
                                 PyObject *Py_UNUSED(kwargs)) {

  DatabasePDG *self = (DatabasePDG *)PyDict_GetItemString(
      type->tp_dict, REACTIONS_INSTANCE_ACCESSOR);

  if (!self) {

    self = (DatabasePDG *)type->tp_alloc(type, 0);
    if (!self)
      return NULL;

    self->database = &reactions::database_pdg::database::instance();

    REACTIONS_PYTHON_UPDATE_DATABASE_TYPE(type, self);

  } else
    // we are not calling "tp_alloc"
    Py_IncRef((PyObject *)self);

  return (PyObject *)self;
}

#endif // REACTIONS_PYTHON_DATABASE_HPP
