#pragma once

#define PY_SSIZE_T_CLEAN
#include "Python.h"

#include "reactions/nubase.hpp"
#include "reactions/pdg.hpp"

#include "utils.hpp"

/// Accessor to energy units
template <class SystemOfUnits> struct energy_units_accessor {

  /// Get the energy units
  static PyObject *get_energy_units(SystemOfUnits *self) {
    return PyUnicode_FromString(reactions::energy_units_properties::to_string(
                                    self->instance->get_energy_units())
                                    .c_str());
  }

  /// Set the energy units
  static PyObject *set_energy_units(SystemOfUnits *self, PyObject *args) {

    const char *units = nullptr;
    if (!PyArg_ParseTuple(args, "s", &units))
      return NULL;

    try {
      self->instance->set_energy_units(
          reactions::energy_units_properties::from_string(units));
    }
    REACTIONS_PYTHON_CATCH_ERRORS(NULL);

    Py_RETURN_NONE;
  }
};

/// Accessor to time units
template <class SystemOfUnits> struct time_units_accessor {

  /// Get the time units
  static PyObject *get_time_units(SystemOfUnits *self) {
    return PyUnicode_FromString(reactions::time_units_properties::to_string(
                                    self->instance->get_time_units())
                                    .c_str());
  }

  /// Set the time units
  static PyObject *set_time_units(SystemOfUnits *self, PyObject *args) {

    const char *units = nullptr;
    if (!PyArg_ParseTuple(args, "s", &units))
      return NULL;

    try {
      self->instance->set_time_units(
          reactions::time_units_properties::from_string(units));
    }
    REACTIONS_PYTHON_CATCH_ERRORS(NULL);

    Py_RETURN_NONE;
  }
};

/// Object for the PDG system of units
typedef struct {
  PyObject_HEAD reactions::pdg_system_of_units *instance = nullptr;
} SystemOfUnitsPDG;

// Create a new node
static PyObject *SystemOfUnitsPDG_new(PyTypeObject *type,
                                      PyObject *Py_UNUSED(args),
                                      PyObject *Py_UNUSED(kwargs)) {
  return reactions::python::singleton_new<SystemOfUnitsPDG>(type);
}

// Methods of the SystemOfUnitsPDG class
static PyMethodDef SystemOfUnitsPDG_methods[] = {
    {"get_energy_units",
     (PyCFunction)energy_units_accessor<SystemOfUnitsPDG>::get_energy_units,
     METH_NOARGS,
     R"(Get the current units of energy for the masses and widths of PDG elements

Returns
-------
str
    Units of energy
)"},
    {"set_energy_units",
     (PyCFunction)energy_units_accessor<SystemOfUnitsPDG>::set_energy_units,
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
defined in this class.)",     /* tp_doc */
    0,                        /* tp_traverse */
    0,                        /* tp_clear */
    0,                        /* tp_richcompare */
    0,                        /* tp_weaklistoffset */
    0,                        /* tp_iter */
    0,                        /* tp_iternext */
    SystemOfUnitsPDG_methods, /* tp_methods */
    0,                        /* tp_members */
    0,                        /* tp_getset */
    0,                        /* tp_base */
    0,                        /* tp_dict */
    0,                        /* tp_descr_get */
    0,                        /* tp_descr_set */
    0,                        /* tp_dictoffset */
    0,                        /* tp_init */
    0,                        /* tp_alloc */
    SystemOfUnitsPDG_new,     /* tp_new */
};

/// Object for the PDG system of units
typedef struct {
  PyObject_HEAD reactions::nubase_system_of_units *instance = nullptr;
} SystemOfUnitsNuBase;

// Create a new node
static PyObject *SystemOfUnitsNuBase_new(PyTypeObject *type,
                                         PyObject *Py_UNUSED(args),
                                         PyObject *Py_UNUSED(kwargs)) {
  return reactions::python::singleton_new<SystemOfUnitsNuBase>(type);
}

// Methods of the SystemOfUnitsNuBase class
static PyMethodDef SystemOfUnitsNuBase_methods[] = {
    {"get_energy_units",
     (PyCFunction)energy_units_accessor<SystemOfUnitsNuBase>::get_energy_units,
     METH_NOARGS,
     R"(Get the current units of energy for the masses and widths of NuBase elements

Returns
-------
str
    Units of energy
)"},
    {"set_energy_units",
     (PyCFunction)energy_units_accessor<SystemOfUnitsNuBase>::set_energy_units,
     METH_VARARGS,
     R"(set_energy_units(units)

Set the units of energy for the masses and widths of NuBase elements

Parameters
----------
str:
    Units of energy to use.

Raises
------
reactions.ValueError
    If the provided units are unknown
)"},
    {"get_time_units",
     (PyCFunction)time_units_accessor<SystemOfUnitsNuBase>::get_time_units,
     METH_NOARGS,
     R"(Get the current units of time for the masses and widths of NuBase elements

Returns
-------
str
    Units of time
)"},
    {"set_time_units",
     (PyCFunction)time_units_accessor<SystemOfUnitsNuBase>::set_time_units,
     METH_VARARGS,
     R"(set_time_units(units)

Set the units of time for the masses and widths of NuBase elements

Parameters
----------
str:
    Units of time to use.

Raises
------
reactions.ValueError
    If the provided units are unknown
)"},
    {NULL, NULL, 0, NULL}};

// Definition of the SystemOfUnitsNuBase type
static PyTypeObject SystemOfUnitsNuBaseType = {
    PyVarObject_HEAD_INIT(
        NULL, 0) "reactions.nubase_system_of_units_sgl", /* tp_name */
    sizeof(SystemOfUnitsNuBase),                         /* tp_basicsize */
    0,                                                   /* tp_itemsize */
    0,                                                   /* tp_dealloc */
    0,                           /* tp_vectorcall_offset */
    0,                           /* tp_getattr */
    0,                           /* tp_setattr */
    0,                           /* tp_as_async */
    0,                           /* tp_repr */
    0,                           /* tp_as_number */
    0,                           /* tp_as_sequence */
    0,                           /* tp_as_mapping */
    0,                           /* tp_hash */
    0,                           /* tp_call */
    0,                           /* tp_str */
    0,                           /* tp_getattro */
    0,                           /* tp_setattro */
    0,                           /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,          /* tp_flags */
    R"(Object to serve as an interface with the NuBase system of units. This object is declared
as a singleton. Any call from a NuBase element to an accessor will use the system of units
defined in this class.)",        /* tp_doc */
    0,                           /* tp_traverse */
    0,                           /* tp_clear */
    0,                           /* tp_richcompare */
    0,                           /* tp_weaklistoffset */
    0,                           /* tp_iter */
    0,                           /* tp_iternext */
    SystemOfUnitsNuBase_methods, /* tp_methods */
    0,                           /* tp_members */
    0,                           /* tp_getset */
    0,                           /* tp_base */
    0,                           /* tp_dict */
    0,                           /* tp_descr_get */
    0,                           /* tp_descr_set */
    0,                           /* tp_dictoffset */
    0,                           /* tp_init */
    0,                           /* tp_alloc */
    SystemOfUnitsNuBase_new,     /* tp_new */
};
