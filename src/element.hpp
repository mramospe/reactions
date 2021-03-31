#pragma once

#include "element_pdg.hpp"
#include "element_string.hpp"

#include "reactions/pdg.hpp"
#include "reactions/pow_enum.hpp"

namespace reactions::python {
  /*\brief Define the element types available in python
   *
   */
  REACTIONS_POW_ENUM_WITH_UNKNOWN(element_kind, pdg, string);
} // namespace reactions::python

/// Element by type
template <class Element> struct python_element_object {};

// Properties of a PDG element
template <> struct python_element_object<reactions::pdg_element> {
  using object = ElementPDG;
  constexpr static PyTypeObject *type_ptr = &ElementPDGType;
};

// Properties of a string element
template <> struct python_element_object<reactions::string_element> {
  using object = ElementString;
  constexpr static PyTypeObject *type_ptr = &ElementStringType;
};

// Alias to the associated python object
template <class Element>
using python_element_object_o = typename python_element_object<Element>::object;

// Alias to the associated python type
template <class Element>
constexpr static PyTypeObject *python_element_object_t =
    python_element_object<Element>::type_ptr;

// Global function to check whether the input object is an element
int object_is_element(PyObject *obj) {

  PyObject *tuple_elements =
      Py_BuildValue("(OO)", &ElementPDGType, &ElementStringType);

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
