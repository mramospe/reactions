#pragma once
#include "reactions/pow_enum.hpp"

#include "element_nubase.hpp"
#include "element_pdg.hpp"
#include "element_string.hpp"

namespace reactions::python {
  /*\brief Define the element types available in python
   *
   */
  REACTIONS_POW_ENUM_WITH_UNKNOWN(element_kind, pdg, nubase, string);
} // namespace reactions::python

/// Create a new instance from a template argument
template <class Element> struct python_element;

/// \copydoc python_element_object
template <> struct python_element<reactions::pdg_element> {
  /// Create a new instance
  static PyObject *new_instance(reactions::pdg_element const &e) {
    return ElementPDG_New(e);
  }
};

/// \copydoc python_element_object
template <> struct python_element<reactions::nubase_element> {
  /// Create a new instance
  static PyObject *new_instance(reactions::nubase_element const &e) {
    return ElementNuBase_New(e);
  }
};

/// \copydoc python_element_object
template <> struct python_element<reactions::string_element> {
  /// Create a new instance
  static PyObject *new_instance(reactions::string_element const &e) {
    return ElementString_New(e);
  }
};

// Global function to check whether the input object is an element
int object_is_element(PyObject *obj) {

  PyObject *tuple_elements = Py_BuildValue(
      "(OOO)", &ElementPDGType, &ElementNuBaseType, &ElementStringType);

  auto dec = PyObject_IsInstance(obj, tuple_elements);

  Py_DecRef(tuple_elements);

  if (dec < 0) {
    PyErr_SetString(
        PyExc_RuntimeError,
        "Unable to check object (internal error); please report the bug");
    return -1;
  }

  return dec;
}
