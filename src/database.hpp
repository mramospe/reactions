#pragma once

#include "element_pdg.hpp"
#include "errors.hpp"

#include "reactions/exceptions.hpp"
#include "reactions/pdg.hpp"

#define PY_SSIZE_T_CLEAN
#include "Python.h"

#define REACTIONS_INSTANCE_ACCESSOR "__instance"

#define REACTIONS_PYTHON_CHECK_DATABASE(database)                              \
  if (!database) {                                                             \
    PyErr_SetString(PyExc_RuntimeError,                                        \
                    "Unable to access the database instance (internal "        \
                    "error); please report the bug");                          \
    return -1;                                                                 \
  }

/// Object for a PDG database
typedef struct {
  PyObject_HEAD reactions::pdg_database *database = nullptr;
} DatabasePDG;

// Create a new node
static PyObject *DatabasePDG_new(PyTypeObject *type, PyObject *Py_UNUSED(args),
                                 PyObject *Py_UNUSED(kwargs));

// Initialize the database
static int DatabasePDG_init(DatabasePDG *self, PyObject *Py_UNUSED(args),
                            PyObject *Py_UNUSED(kwds)) {
  return 0;
}

// Clear the internal cache
static PyObject *DatabasePDG_clear_cache(DatabasePDG *self) {
  self->database->clear_cache();
  Py_RETURN_NONE;
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
static PyObject *DatabasePDG_get_database_path(DatabasePDG *self) {

  return PyUnicode_FromString(self->database->get_database_path().c_str());
}

// Register a new element
static PyObject *DatabasePDG_register_element(DatabasePDG *self, PyObject *args,
                                              PyObject *kwargs) {

  PyObject *obj = NULL;

  if (args != NULL && PyTuple_Size(args) == 1 && kwargs == NULL) {

    if (!PyArg_ParseTuple(args, "O", &obj))
      return NULL;

    if (!PyObject_IsInstance(obj, (PyObject *)&ElementPDGType)) {
      PyErr_SetString(PyExc_ValueError,
                      "If only one argument is provided it must be a "
                      "reactions.pdg_element object");
      return NULL;
    }
  } else if (args != NULL || kwargs != NULL) {

    obj = ElementPDGType.tp_new((PyTypeObject *)&ElementPDGType, NULL, NULL);
    if (ElementPDGType.tp_init(obj, args, kwargs) < 0)
      REACTIONS_PYTHON_RETURN_INVALID_ARGUMENTS(NULL);

  } else {
    PyErr_SetString(PyExc_ValueError, "Invalid arguments");
    return NULL;
  }

  try {
    self->database->register_element(((ElementPDG *)obj)->element);
  }
  REACTIONS_PYTHON_CATCH_ERRORS(NULL);

  Py_RETURN_NONE;
}

/// Access an element of the database
static PyObject *DatabasePDG_call(DatabasePDG *self, PyObject *args,
                                  PyObject *kwargs) {

  if (args == NULL || PyTuple_Size(args) != 1)
    REACTIONS_PYTHON_RETURN_INVALID_ARGUMENTS(NULL);

  if (kwargs != NULL)
    REACTIONS_PYTHON_RETURN_INVALID_ARGUMENTS(NULL);

  PyObject *obj;
  if (!PyArg_ParseTuple(args, "O", &obj))
    REACTIONS_PYTHON_RETURN_INVALID_ARGUMENTS(NULL);

  if (PyUnicode_Check(obj))
    return ElementPDG_New(
        reactions::pdg_database::instance()(PyUnicode_AsUTF8(obj)));

  if (PyLong_Check(obj))
    return ElementPDG_New(
        reactions::pdg_database::instance()(PyLong_AsLong(obj)));

  PyErr_SetString(PyExc_ValueError,
                  "Argument must be either the PDG ID or the name");
  return NULL;
}

// Set the location of the PDG database
static PyObject *DatabasePDG_set_database_path(DatabasePDG *self,
                                               PyObject *args) {

  const char *str = nullptr;
  if (!PyArg_ParseTuple(args, "s", &str))
    return NULL;

  self->database->set_database_path(str);

  Py_RETURN_NONE;
}

// Get all the elements
static PyObject *DatabasePDG_all_elements(DatabasePDG *self) {

  auto all_elements = self->database->all_elements();

  PyObject *list = PyList_New(all_elements.size());

  for (auto i = 0u; i < all_elements.size(); ++i)
    PyList_SetItem(list, i, ElementPDG_New(std::move(all_elements[i])));

  return list;
}

// Methods of the DatabasePDG class
static PyMethodDef DatabasePDG_methods[] = {
    {"all_elements", (PyCFunction)DatabasePDG_all_elements, METH_NOARGS,
     "Extract all the elements to a list"},
    {"clear_cache", (PyCFunction)DatabasePDG_clear_cache, METH_NOARGS,
     "Clear the internal cache, removing also user-registered elements"},
    {"disable_cache", (PyCFunction)DatabasePDG_disable_cache, METH_NOARGS,
     "Disable the internal cache"},
    {"enable_cache", (PyCFunction)DatabasePDG_enable_cache, METH_NOARGS,
     "Enable the internal cache. All the elements will be loaded in memory."},
    {"get_database_path", (PyCFunction)DatabasePDG_get_database_path,
     METH_NOARGS, "Get the path to the database file"},
    {"register_element", (PyCFunction)DatabasePDG_register_element,
     METH_VARARGS | METH_KEYWORDS, "Register a new element in the database"},
    {"set_database_path", (PyCFunction)DatabasePDG_set_database_path,
     METH_VARARGS,
     "Set the path to the database file. If the cache is enabled, reloads the "
     "data. If the cache is enabled, elements are reloaded in memory."},
    {NULL, NULL, 0, NULL}};

// Definition of the DatabasePDG type
static PyTypeObject DatabasePDGType = {
    PyVarObject_HEAD_INIT(NULL, 0) "reactions.pdg_database", /* tp_name */
    sizeof(DatabasePDG),                                     /* tp_basicsize */
    0,                                                       /* tp_itemsize */
    0,                                                       /* tp_dealloc */
    0,                               /* tp_vectorcall_offset */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    0,                               /* tp_as_async */
    0,                               /* tp_repr */
    0,                               /* tp_as_number */
    0,                               /* tp_as_sequence */
    0,                               /* tp_as_mapping */
    0,                               /* tp_hash */
    (ternaryfunc)DatabasePDG_call,   /* tp_call */
    0,                               /* tp_str */
    0,                               /* tp_getattro */
    0,                               /* tp_setattro */
    0,                               /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,              /* tp_flags */
    R"(Object to serve as an interface with the PDG database. This object is declared
as a singleton, using a single database file that can be modified using the
:func:`set_pdg_database` function. This ensures that reactions are composed of PDG
elements from the same file. The database can make use of an internal cache to avoid
reading from the file many times.)", /* tp_doc */
    0,                               /* tp_traverse */
    0,                               /* tp_clear */
    0,                               /* tp_richcompare */
    0,                               /* tp_weaklistoffset */
    0,                               /* tp_iter */
    0,                               /* tp_iternext */
    DatabasePDG_methods,             /* tp_methods */
    0,                               /* tp_members */
    0,                               /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    (initproc)DatabasePDG_init,      /* tp_init */
    0,                               /* tp_alloc */
    DatabasePDG_new,                 /* tp_new */
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

    self->database = &reactions::pdg_database::instance();

    if (PyDict_SetItemString(type->tp_dict, REACTIONS_INSTANCE_ACCESSOR,
                             (PyObject *)self) != 0) {
      Py_DecRef((PyObject *)self);
      PyErr_SetString(PyExc_RuntimeError,
                      "Unable to define the database as a singleton ("
                      "internal error); please report the bug");
      return NULL;
    }
    PyType_Modified(type);

  } else
    // we are not calling "tp_alloc"
    Py_IncRef((PyObject *)self);

  return (PyObject *)self;
}
