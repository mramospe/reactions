#pragma once
#define PY_SSIZE_T_CLEAN
#include "Python.h"

#include "reactions/exceptions.hpp"
#include "reactions/nubase.hpp"
#include "reactions/pdg.hpp"

#include "element_nubase.hpp"
#include "element_pdg.hpp"
#include "errors.hpp"
#include "utils.hpp"

#define REACTIONS_PYTHON_CHECK_DATABASE(database)                              \
  if (!database) {                                                             \
    PyErr_SetString(PyExc_RuntimeError,                                        \
                    "Unable to access the database instance (internal "        \
                    "error); please report the bug");                          \
    return -1;                                                                 \
  }

/// Create a new PDG element
static PyObject *element_new(reactions::pdg_element &&el) {
  return ElementPDG_New(el);
}

/// Create a new NuBase element
static PyObject *element_new(reactions::nubase_element &&el) {
  return ElementNuBase_New(el);
}

/// General methods to a database
template <class Database, class Element, PyTypeObject *ElementType>
struct database_accessors {

  /// Create a new database
  static PyObject *new_object(PyTypeObject *type, PyObject *Py_UNUSED(args),
                              PyObject *Py_UNUSED(kwargs)) {
    return reactions::python::singleton_new<Database>(type);
  }

  /// Clear the internal cache
  static PyObject *clear_cache(Database *self) {
    self->instance->clear_cache();
    Py_RETURN_NONE;
  }

  /// Disable the internal cache
  static PyObject *disable_cache(Database *self) {
    self->instance->disable_cache();
    Py_RETURN_NONE;
  }

  /// Enable the internal cache
  static PyObject *enable_cache(Database *self) {
    self->instance->enable_cache();
    Py_RETURN_NONE;
  }

  /// Set the location of the database
  static PyObject *get_database_path(Database *self) {

    return PyUnicode_FromString(self->instance->get_database_path().c_str());
  }

  /// Register a new element
  static PyObject *register_element(Database *self, PyObject *args,
                                    PyObject *kwargs) {

    PyObject *obj = NULL;

    if (args != NULL && PyTuple_Size(args) == 1 && kwargs == NULL) {

      if (!PyArg_ParseTuple(args, "O", &obj))
        return NULL;

      if (!PyObject_IsInstance(obj, (PyObject *)ElementType)) {
        PyErr_SetString(PyExc_ValueError,
                        "If only one argument is provided it must be an "
                        "element object");
        return NULL;
      }
    } else if (args != NULL || kwargs != NULL) {

      obj = ElementType->tp_new((PyTypeObject *)ElementType, NULL, NULL);
      if (ElementType->tp_init(obj, args, kwargs) < 0)
        REACTIONS_PYTHON_RETURN_INVALID_ARGUMENTS(NULL);

    } else {
      PyErr_SetString(PyExc_ValueError, "Invalid arguments");
      return NULL;
    }

    try {
      self->instance->register_element(((Element *)obj)->element);
    }
    REACTIONS_PYTHON_CATCH_ERRORS(NULL);

    Py_RETURN_NONE;
  }

  /// Access an element of the database
  static PyObject *call(Database *self, PyObject *args, PyObject *kwargs) {

    if (args == NULL || PyTuple_Size(args) != 1)
      REACTIONS_PYTHON_RETURN_INVALID_ARGUMENTS(NULL);

    if (kwargs != NULL)
      REACTIONS_PYTHON_RETURN_INVALID_ARGUMENTS(NULL);

    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj))
      REACTIONS_PYTHON_RETURN_INVALID_ARGUMENTS(NULL);

    try {
      if (PyUnicode_Check(obj))
        return element_new(
            std::remove_pointer_t<decltype(self->instance)>::instance()(
                PyUnicode_AsUTF8(obj)));

      if (PyLong_Check(obj))
        return element_new(
            std::remove_pointer_t<decltype(self->instance)>::instance()(
                PyLong_AsLong(obj)));
    }
    REACTIONS_PYTHON_CATCH_ERRORS(NULL);

    PyErr_SetString(PyExc_ValueError,
                    "Argument must be either the ID or the name");
    return NULL;
  }

  /// Set the location of the database
  static PyObject *set_database_path(Database *self, PyObject *args) {

    const char *str = nullptr;
    if (!PyArg_ParseTuple(args, "s", &str))
      return NULL;

    self->instance->set_database_path(str);

    Py_RETURN_NONE;
  }

  /// Get all the elements
  static PyObject *all_elements(Database *self) {
    PyObject *list = NULL;

    try {
      auto all_elements = self->instance->all_elements();

      list = PyList_New(all_elements.size());

      for (auto i = 0u; i < all_elements.size(); ++i)
        PyList_SetItem(list, i, element_new(std::move(all_elements[i])));
    }
    REACTIONS_PYTHON_CATCH_ERRORS(NULL);

    return list;
  }
};

/// Object for a PDG database
typedef struct {
  PyObject_HEAD reactions::pdg_database *instance = nullptr;
} DatabasePDG;

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
      self->instance->charge_conjugate(((ElementPDG *)obj)->element));
}

using DatabasePDG_accessors =
    database_accessors<DatabasePDG, ElementPDG, &ElementPDGType>;

// Methods of the DatabasePDG class
static PyMethodDef DatabasePDG_methods[] = {
    {"__call__", (PyCFunction)DatabasePDG_accessors::call,
     METH_VARARGS | METH_COEXIST,
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
    {"all_elements", (PyCFunction)DatabasePDG_accessors::all_elements,
     METH_NOARGS,
     R"(Extract all the elements to a list. This includes all those elements
registered by the user.

Returns
-------
list(pdg_element)
    All the elements
)"},
    {"clear_cache", (PyCFunction)DatabasePDG_accessors::clear_cache,
     METH_NOARGS,
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
    {"disable_cache", (PyCFunction)DatabasePDG_accessors::disable_cache,
     METH_NOARGS, "Disable the internal cache"},
    {"enable_cache", (PyCFunction)DatabasePDG_accessors::enable_cache,
     METH_NOARGS,
     "Enable the internal cache. All the elements will be loaded in memory."},
    {"get_database_path", (PyCFunction)DatabasePDG_accessors::get_database_path,
     METH_NOARGS, R"(Get the path to the database file

Returns
-------
str
    Path to the database file
)"},
    {"register_element", (PyCFunction)DatabasePDG_accessors::register_element,
     METH_VARARGS | METH_KEYWORDS,
     R"(Register a new element in the database. Arguments
can be either a single PDG element object or the arguments to the constructor
of :class:`reactions.pdg_element`.)"},
    {"set_database_path", (PyCFunction)DatabasePDG_accessors::set_database_path,
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
    sizeof(DatabasePDG),                      /* tp_basicsize */
    0,                                        /* tp_itemsize */
    0,                                        /* tp_dealloc */
    0,                                        /* tp_vectorcall_offset */
    0,                                        /* tp_getattr */
    0,                                        /* tp_setattr */
    0,                                        /* tp_as_async */
    0,                                        /* tp_repr */
    0,                                        /* tp_as_number */
    0,                                        /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    0,                                        /* tp_hash */
    (ternaryfunc)DatabasePDG_accessors::call, /* tp_call */
    0,                                        /* tp_str */
    0,                                        /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                       /* tp_flags */
    R"(Object to serve as an interface with the PDG database. This object is declared
as a singleton, using a single database file that can be modified using the
:func:`set_database_path` function. This ensures that reactions are composed of PDG
elements from the same file. The database can make use of an internal cache to avoid
reading from the file many times.)",          /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    DatabasePDG_methods,                      /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    DatabasePDG_accessors::new_object,        /* tp_new */
};

/// Object for a NuBase database
typedef struct {
  PyObject_HEAD reactions::nubase_database *instance = nullptr;
} DatabaseNuBase;

using DatabaseNuBase_accessors =
    database_accessors<DatabaseNuBase, ElementNuBase, &ElementNuBaseType>;

// Methods of the DatabaseNuBase class
static PyMethodDef DatabaseNuBase_methods[] = {
    {"__call__", (PyCFunction)DatabaseNuBase_accessors::call,
     METH_VARARGS | METH_COEXIST,
     R"(__call__(arg)

Access an element of the database by name or NuBase ID

Parameters
----------
arg : str or int
    Element name or NuBase ID

Returns
-------
nubase_element
    Corresponding element

Raises
------
reactions.LookupError
    If the element is not found
)"},
    {"all_elements", (PyCFunction)DatabaseNuBase_accessors::all_elements,
     METH_NOARGS,
     R"(Extract all the elements to a list. This includes all those elements
registered by the user.

Returns
-------
list(nubase_element)
    All the elements
)"},
    {"clear_cache", (PyCFunction)DatabaseNuBase_accessors::clear_cache,
     METH_NOARGS,
     "Clear the internal cache, removing also user-registered elements"},
    {"disable_cache", (PyCFunction)DatabaseNuBase_accessors::disable_cache,
     METH_NOARGS, "Disable the internal cache"},
    {"enable_cache", (PyCFunction)DatabaseNuBase_accessors::enable_cache,
     METH_NOARGS,
     "Enable the internal cache. All the elements will be loaded in memory."},
    {"get_database_path",
     (PyCFunction)DatabaseNuBase_accessors::get_database_path, METH_NOARGS,
     R"(Get the path to the database file

Returns
-------
str
    Path to the database file
)"},
    {"register_element",
     (PyCFunction)DatabaseNuBase_accessors::register_element,
     METH_VARARGS | METH_KEYWORDS,
     R"(Register a new element in the database. Arguments
can be either a single NuBase element object or the arguments to the constructor
of :class:`reactions.nubase_element`.)"},
    {"set_database_path",
     (PyCFunction)DatabaseNuBase_accessors::set_database_path, METH_VARARGS,
     R"(set_database_path(path)

Set the path to the database file. If the cache is enabled, reloads the
data. If the cache is enabled, elements are reloaded in memory.

Parameters
----------
path:
    Path to the database file.
)"},
    {NULL, NULL, 0, NULL}};

// Definition of the DatabaseNuBase type
static PyTypeObject DatabaseNuBaseType = {
    PyVarObject_HEAD_INIT(NULL,
                          0) "reactions.nubase_database_sgl", /* tp_name */
    sizeof(DatabaseNuBase),                                   /* tp_basicsize */
    0,                                                        /* tp_itemsize */
    0,                                                        /* tp_dealloc */
    0,                                           /* tp_vectorcall_offset */
    0,                                           /* tp_getattr */
    0,                                           /* tp_setattr */
    0,                                           /* tp_as_async */
    0,                                           /* tp_repr */
    0,                                           /* tp_as_number */
    0,                                           /* tp_as_sequence */
    0,                                           /* tp_as_mapping */
    0,                                           /* tp_hash */
    (ternaryfunc)DatabaseNuBase_accessors::call, /* tp_call */
    0,                                           /* tp_str */
    0,                                           /* tp_getattro */
    0,                                           /* tp_setattro */
    0,                                           /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                          /* tp_flags */
    R"(Object to serve as an interface with the NuBase database. This object is declared
as a singleton, using a single database file that can be modified using the
:func:`set_database_path` function. This ensures that reactions are composed of NuBase
elements from the same file. The database can make use of an internal cache to avoid
reading from the file many times.)",             /* tp_doc */
    0,                                           /* tp_traverse */
    0,                                           /* tp_clear */
    0,                                           /* tp_richcompare */
    0,                                           /* tp_weaklistoffset */
    0,                                           /* tp_iter */
    0,                                           /* tp_iternext */
    DatabaseNuBase_methods,                      /* tp_methods */
    0,                                           /* tp_members */
    0,                                           /* tp_getset */
    0,                                           /* tp_base */
    0,                                           /* tp_dict */
    0,                                           /* tp_descr_get */
    0,                                           /* tp_descr_set */
    0,                                           /* tp_dictoffset */
    0,                                           /* tp_init */
    0,                                           /* tp_alloc */
    DatabaseNuBase_accessors::new_object,        /* tp_new */
};
