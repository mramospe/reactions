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

// Get the charge-conjugate of the given element
static PyObject *DatabasePDG_charge_conjugate(DatabasePDG *self,
                                              PyObject *args) {
  PyObject *obj;
  if (!PyArg_ParseTuple(args, "O", &obj))
    return NULL;

  if (!PyObject_IsInstance(obj, (PyObject *)&ElementPDGType)) {
    PyErr_SetString(PyExc_ValueError,
                    "Argument must be a reactions.pdg_element object");
    return NULL;
  }

  return ElementPDG_New(
      self->database->charge_conjugate(((ElementPDG *)obj)->element));
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

  try {
    if (PyUnicode_Check(obj))
      return ElementPDG_New(
          reactions::pdg_database::instance()(PyUnicode_AsUTF8(obj)));

    if (PyLong_Check(obj))
      return ElementPDG_New(
          reactions::pdg_database::instance()(PyLong_AsLong(obj)));
  }
  REACTIONS_PYTHON_CATCH_ERRORS(NULL);

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
    {"__call__", (PyCFunction)DatabasePDG_call, METH_VARARGS | METH_COEXIST,
     R"(__call__(arg)
Access an element of the database by name or PDG ID

Parameters
----------
arg : str or int
    Element name or PDG ID

Returns
-------
pdg_element
    Corresponding element

Raises
------
reactions.LookupError
    If the element is not found
)"},
    {"all_elements", (PyCFunction)DatabasePDG_all_elements, METH_NOARGS,
     R"(Extract all the elements to a list. This includes all those elements
registered by the user.

Returns
-------
list(pdg_element)
    All the elements
)"},
    {"clear_cache", (PyCFunction)DatabasePDG_clear_cache, METH_NOARGS,
     "Clear the internal cache, removing also user-registered elements"},
    {"charge_conjugate", (PyCFunction)DatabasePDG_charge_conjugate,
     METH_VARARGS, R"(Get the charge-conjugate of an element.

Parameters
----------
pdg_element:
    element to calculate the charge-conjugate

Returns
-------
pdg_element
    Charge-conjugate of the given element
)"},
    {"disable_cache", (PyCFunction)DatabasePDG_disable_cache, METH_NOARGS,
     "Disable the internal cache"},
    {"enable_cache", (PyCFunction)DatabasePDG_enable_cache, METH_NOARGS,
     "Enable the internal cache. All the elements will be loaded in memory."},
    {"get_database_path", (PyCFunction)DatabasePDG_get_database_path,
     METH_NOARGS, R"(Get the path to the database file

Returns
-------
str
    Path to the database file
)"},
    {"register_element", (PyCFunction)DatabasePDG_register_element,
     METH_VARARGS | METH_KEYWORDS,
     R"(Register a new element in the database. Arguments
can be either a single PDG element object or the arguments to the constructor
of :class:`reactions.pdg_element`.)"},
    {"set_database_path", (PyCFunction)DatabasePDG_set_database_path,
     METH_VARARGS,
     R"(set_database_path(path)
Set the path to the database file. If the cache is enabled, reloads the
data. If the cache is enabled, elements are reloaded in memory.

Parameters
----------
path:
    Path to the database file.
)"},
    {NULL, NULL, 0, NULL}};

// Definition of the DatabasePDG type
static PyTypeObject DatabasePDGType = {
    PyVarObject_HEAD_INIT(NULL, 0) "reactions.pdg_database_sgl", /* tp_name */
    sizeof(DatabasePDG),             /* tp_basicsize */
    0,                               /* tp_itemsize */
    0,                               /* tp_dealloc */
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
:func:`set_database_path` function. This ensures that reactions are composed of PDG
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

/// Object for the PDG system of units
typedef struct {
  PyObject_HEAD reactions::pdg_system_of_units *system_of_units = nullptr;
} SystemOfUnitsPDG;

// Create a new node
static PyObject *SystemOfUnitsPDG_new(PyTypeObject *type,
                                      PyObject *Py_UNUSED(args),
                                      PyObject *Py_UNUSED(kwargs));

// Initialize the object
static int SystemOfUnitsPDG_init(SystemOfUnitsPDG *self,
                                 PyObject *Py_UNUSED(args),
                                 PyObject *Py_UNUSED(kwds)) {
  return 0;
}

// Get the units of energy
static PyObject *SystemOfUnitsPDG_get_energy_units(SystemOfUnitsPDG *self) {

  return PyUnicode_FromString(reactions::energy_units_properties::to_string(
                                  self->system_of_units->get_energy_units())
                                  .c_str());
}

// Set the units of energy
static PyObject *SystemOfUnitsPDG_set_energy_units(SystemOfUnitsPDG *self,
                                                   PyObject *args) {

  const char *units = nullptr;
  if (!PyArg_ParseTuple(args, "s", &units))
    return NULL;

  try {
    self->system_of_units->set_energy_units(
        reactions::energy_units_properties::from_string(units));
  }
  REACTIONS_PYTHON_CATCH_ERRORS(NULL);

  Py_RETURN_NONE;
}

// Methods of the SystemOfUnitsPDG class
static PyMethodDef SystemOfUnitsPDG_methods[] = {
    {"get_energy_units", (PyCFunction)SystemOfUnitsPDG_get_energy_units,
     METH_NOARGS,
     R"(Get the current units of energy for the masses and widths of PDG elements

Returns
-------
str
    Units of energy
)"},
    {"set_energy_units", (PyCFunction)SystemOfUnitsPDG_set_energy_units,
     METH_VARARGS,
     R"(set_energy_units(units)
Set the units of energy for the masses and widths of PDG elements

Parameters
----------
str:
    Units of energy to use.

Raises
------
reactions.ValueError
    If the provided units are unknown
)"},
    {NULL, NULL, 0, NULL}};

// Definition of the SystemOfUnitsPDG type
static PyTypeObject SystemOfUnitsPDGType = {
    PyVarObject_HEAD_INIT(NULL,
                          0) "reactions.pdg_system_of_units_sgl", /* tp_name */
    sizeof(SystemOfUnitsPDG), /* tp_basicsize */
    0,                        /* tp_itemsize */
    0,                        /* tp_dealloc */
    0,                        /* tp_vectorcall_offset */
    0,                        /* tp_getattr */
    0,                        /* tp_setattr */
    0,                        /* tp_as_async */
    0,                        /* tp_repr */
    0,                        /* tp_as_number */
    0,                        /* tp_as_sequence */
    0,                        /* tp_as_mapping */
    0,                        /* tp_hash */
    0,                        /* tp_call */
    0,                        /* tp_str */
    0,                        /* tp_getattro */
    0,                        /* tp_setattro */
    0,                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,       /* tp_flags */
    R"(Object to serve as an interface with the PDG system of units. This object is declared
as a singleton. Any call from a PDG element to an accessor will use the system of units
defined in this class. Currently only the energy units can be modified, which must
be `eV`, `keV`, `MeV`, `GeV`, `TeV` or `PeV`. The `GeV` units are used by default.)", /* tp_doc */
    0,                               /* tp_traverse */
    0,                               /* tp_clear */
    0,                               /* tp_richcompare */
    0,                               /* tp_weaklistoffset */
    0,                               /* tp_iter */
    0,                               /* tp_iternext */
    SystemOfUnitsPDG_methods,        /* tp_methods */
    0,                               /* tp_members */
    0,                               /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    (initproc)SystemOfUnitsPDG_init, /* tp_init */
    0,                               /* tp_alloc */
    SystemOfUnitsPDG_new,            /* tp_new */
};

// Create a new node
static PyObject *SystemOfUnitsPDG_new(PyTypeObject *type,
                                      PyObject *Py_UNUSED(args),
                                      PyObject *Py_UNUSED(kwargs)) {

  SystemOfUnitsPDG *self = (SystemOfUnitsPDG *)PyDict_GetItemString(
      type->tp_dict, REACTIONS_INSTANCE_ACCESSOR);

  if (!self) {

    self = (SystemOfUnitsPDG *)type->tp_alloc(type, 0);
    if (!self)
      return NULL;

    self->system_of_units = &reactions::pdg_system_of_units::instance();

    if (PyDict_SetItemString(type->tp_dict, REACTIONS_INSTANCE_ACCESSOR,
                             (PyObject *)self) != 0) {
      Py_DecRef((PyObject *)self);
      PyErr_SetString(PyExc_RuntimeError,
                      "Unable to define the object as a singleton ("
                      "internal error); please report the bug");
      return NULL;
    }
    PyType_Modified(type);

  } else
    // we are not calling "tp_alloc"
    Py_IncRef((PyObject *)self);

  return (PyObject *)self;
}
