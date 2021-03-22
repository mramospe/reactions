#ifndef REACTIONS_PYTHON_ELEMENT_HPP
#define REACTIONS_PYTHON_ELEMENT_HPP

#include "element_pdg.hpp"
#include "element_string.hpp"

#include "reactions/database_pdg.hpp"

/// Element by type
template <class Element> struct python_element_object {};

// Properties of a PDG element
template <> struct python_element_object<reactions::database_pdg::element> {
  using object = ElementPDG;
  constexpr static PyTypeObject *type_ptr = &ElementPDGType;
};

// Properties of a string element
template <> struct python_element_object<std::string> {
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

#endif // REACTIONS_PYTHON_ELEMENT_HPP
