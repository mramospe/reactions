#pragma once

#include <tuple>

#include "reactions/fields.hpp"
#include "reactions/pdg.hpp"

#include "errors.hpp"
#include "node.hpp"
#include "types.hpp"
#include "utils.hpp"

// Wrapper for a PDG element
typedef struct {
  // base class
  Node node;
  // attributes (same as pdg::pdg_element)
  reactions::pdg_element element;
} ElementPDG;

// Allocate a new ElementPDG
static PyObject *ElementPDG_new(PyTypeObject *type, PyObject *Py_UNUSED(args),
                                PyObject *Py_UNUSED(kwargs)) {

  ElementPDG *self = (ElementPDG *)type->tp_alloc(type, 0);

  if (!self)
    return NULL;

  // Set the type for the base class
  self->node.c_type = reactions::processes::node_kind::element;

  return (PyObject *)self;
}

/// Initialize the element
static int ElementPDG_init(ElementPDG *self, PyObject *args, PyObject *kwargs) {

  if (NodeType.tp_init((PyObject *)self, NULL, NULL) < 0)
    return -1;

  if (args != NULL && PyTuple_Size(args) == 1 && kwargs == NULL) {
    // the first element is the name, and the database is used to access it

    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj))
      return -1;

    try {
      auto const &pdg_instance = reactions::pdg_database::instance();

      if (PyObject_IsInstance(obj, (PyObject *)&PyUnicode_Type)) {
        const char *str = PyUnicode_AsUTF8(obj);
        self->element = pdg_instance(str);
      } else if (PyObject_IsInstance(obj, (PyObject *)&PyLong_Type)) {
        int id = PyLong_AsLong(obj);
        self->element = pdg_instance(id);
      } else {
        PyErr_SetString(
            PyExc_TypeError,
            "Argument must be either a string (name) or an integer (PDG ID)");
        return -1;
      }
    }
    REACTIONS_PYTHON_CATCH_ERRORS(-1)
  } else {

    using parser_type = cpp_to_py_type<reactions::pdg_element::base_type>;

    // the instance is built from the input values
    typename parser_type::type tup;

    PyObject *mass = nullptr, *width = nullptr;

    if (args != NULL &&
        PyTuple_Size(args) == reactions::pdg_element::number_of_fields) {

      if (!PyArg_ParseTuple(args, parser_type::descriptor_args,
                            &std::get<0>(tup), &std::get<1>(tup),
                            &std::get<2>(tup), &mass, &width,
                            &std::get<5>(tup)))
        REACTIONS_PYTHON_RETURN_INVALID_ARGUMENTS(-1);

    } else if (kwargs != NULL && PyDict_Size(kwargs) ==
                                     reactions::pdg_element::number_of_fields) {
      // the instance is built from the keyword arguments
      static const char *kwds[] = {"name",
                                   "pdg_id",
                                   "three_charge",
                                   "mass_and_errors",
                                   "width_and_errors",
                                   "is_self_cc",
                                   NULL};

      if (!PyArg_ParseTupleAndKeywords(
              args, kwargs, parser_type::descriptor_kwargs,
              const_cast<char **>(kwds), &std::get<0>(tup), &std::get<1>(tup),
              &std::get<2>(tup), &mass, &width, &std::get<5>(tup)))
        REACTIONS_PYTHON_RETURN_INVALID_ARGUMENTS(-1);

    } else
      // Invalid arguments
      REACTIONS_PYTHON_RETURN_INVALID_ARGUMENTS(-1);

    if (mass && mass != Py_None) {

      auto length = PySequence_Length(mass);

      if (length < 1 || length > 3) {
        PyErr_SetString(PyExc_ValueError,
                        "Mass argument must be a sequence of three values");
        return -1;
      }

      double value, error_lower, error_upper;
      if (!PyArg_ParseTuple(mass, "ddd", &value, &error_lower, &error_upper)) {
        PyErr_SetString(
            PyExc_ValueError,
            "Mass argument must be a sequence of three floating point objects");
        return -1;
      }

      std::get<3>(tup) = {value, error_lower, error_upper};
    }

    if (width && width != Py_None) {

      auto length = PySequence_Length(width);

      if (length < 1 || length > 3) {
        PyErr_SetString(PyExc_ValueError, "Width argument must be a sequence "
                                          "of three floating point objects");
        return -1;
      }

      double value, error_lower, error_upper;
      if (!PyArg_ParseTuple(width, "ddd", &value, &error_lower, &error_upper)) {
        PyErr_SetString(PyExc_ValueError, "Width argument must be a sequence "
                                          "of three floating point objects");
        return -1;
      }

      std::get<4>(tup) = {value, error_lower, error_upper};
    }

    self->element = std::move(tup);
  }

  return 0;
}

// Represent the field of a PDG element as a string
template <std::size_t I>
std::string ElementPDG_field_to_string(reactions::pdg_element const &el) {

  using field_type =
      std::tuple_element_t<I, reactions::pdg_element::fields_type>;
  using underlying_type_no_opt =
      reactions::fields::remove_optional_t<typename field_type::value_type>;

  std::string title =
      reactions::utils::is_template_specialization_v<
          underlying_type_no_opt, reactions::fields::value_and_errors>
          ? std::string{field_type::title} + "_and_errors"
          : field_type::title;

  if constexpr (I == 0) {
    if (el.has<field_type>())
      return title + '=' + reactions::python::to_string(el.get<field_type>());
    else
      return title + "=None";
  } else {
    if (el.has<field_type>())
      return std::string{", "} + title + '=' +
             reactions::python::to_string(el.get<field_type>());
    else
      return std::string{", "} + title + "=None";
  }
}

// Represent a PDG element as a string (implementation)
template <std::size_t... I>
std::string ElementPDG_to_string_impl(reactions::pdg_element const &el,
                                      std::index_sequence<I...>) {
  return (ElementPDG_field_to_string<I>(el) + ...);
}

/// Representation of the class as a string
static PyObject *ElementPDG_to_string(ElementPDG *self) {

  PyTypeObject *type = (PyTypeObject *)PyObject_Type((PyObject *)self);
  if (!type)
    return NULL;

  auto const str =
      std::string{type->tp_name} + '(' +
      ElementPDG_to_string_impl(
          self->element,
          std::make_index_sequence<
              std::tuple_size_v<reactions::pdg_element::fields_type>>()) +
      ')';
  return PyUnicode_FromString(str.c_str());
}

/// Comparison operator(s)
static PyObject *ElementPDG_richcompare(PyObject *obj1, PyObject *obj2, int op);

/// Methods
static PyMethodDef ElementPDG_methods[] = {
    // sentinel
    {NULL, NULL, 0, NULL}};

/// Define a function to get access to an attribute of a PDG element
#define REACTIONS_PYTHON_ELEMENTPDG_GETTER_DEF(name, converter)                \
  static PyObject *ElementPDG_get_##name(ElementPDG *self, void *) {           \
    return converter(self->element.name());                                    \
  }

/// Define a function to get access to an attribute of a PDG element
#define REACTIONS_PYTHON_ELEMENTPDG_GETTER_CHECK_DEF(name, check, converter)   \
  static PyObject *ElementPDG_get_##name(ElementPDG *self, void *) {           \
    if constexpr (reactions::fields::is_optional_field_v<                      \
                      reactions::pdg::check>)                                  \
      if (!self->element.has_##check())                                        \
        Py_RETURN_NONE;                                                        \
      else                                                                     \
        return converter(self->element.name());                                \
    else                                                                       \
      return converter(self->element.name());                                  \
  }

/// Define a function to call a member function of a PDG element
#define REACTIONS_PYTHON_ELEMENTPDG_FUNCTION_DEF(name, converter)              \
  static PyObject *ElementPDG_get_##name(ElementPDG *self, void *) {           \
    try {                                                                      \
      return converter(self->element.name());                                  \
    }                                                                          \
    REACTIONS_PYTHON_CATCH_ERRORS(NULL)                                        \
  }

/// Convert a std::string to a python unicode string
#define REACTIONS_PYTHON_CPP_STRING_TOPY(string)                               \
  PyUnicode_FromString(string.c_str())

// Define the getters of the ElementPDG class
REACTIONS_PYTHON_ELEMENTPDG_GETTER_DEF(name, REACTIONS_PYTHON_CPP_STRING_TOPY)
REACTIONS_PYTHON_ELEMENTPDG_GETTER_DEF(pdg_id, PyLong_FromLong)
REACTIONS_PYTHON_ELEMENTPDG_GETTER_DEF(three_charge, PyLong_FromLong)
REACTIONS_PYTHON_ELEMENTPDG_GETTER_CHECK_DEF(mass, mass, PyFloat_FromDouble)
REACTIONS_PYTHON_ELEMENTPDG_GETTER_CHECK_DEF(mass_error_lower, mass,
                                             PyFloat_FromDouble)
REACTIONS_PYTHON_ELEMENTPDG_GETTER_CHECK_DEF(mass_error_upper, mass,
                                             PyFloat_FromDouble)
REACTIONS_PYTHON_ELEMENTPDG_GETTER_CHECK_DEF(width, width, PyFloat_FromDouble)
REACTIONS_PYTHON_ELEMENTPDG_GETTER_CHECK_DEF(width_error_lower, width,
                                             PyFloat_FromDouble)
REACTIONS_PYTHON_ELEMENTPDG_GETTER_CHECK_DEF(width_error_upper, width,
                                             PyFloat_FromDouble)
REACTIONS_PYTHON_ELEMENTPDG_GETTER_DEF(is_self_cc, PyBool_FromLong)

// Define the functions (used later also to define "getters")
REACTIONS_PYTHON_ELEMENTPDG_GETTER_DEF(charge, PyFloat_FromDouble)
REACTIONS_PYTHON_ELEMENTPDG_GETTER_CHECK_DEF(mass_error, mass,
                                             PyFloat_FromDouble)
REACTIONS_PYTHON_ELEMENTPDG_GETTER_CHECK_DEF(width_error, width,
                                             PyFloat_FromDouble)
// Attributes from functions
REACTIONS_PYTHON_ELEMENTPDG_FUNCTION_DEF(latex_name,
                                         REACTIONS_PYTHON_CPP_STRING_TOPY)

/// Provide the data needed to define a "getter" function
#define REACTIONS_PYTHON_ELEMENTPDG_GETTER_DESC(name, description)             \
  { #name, (getter)ElementPDG_get_##name, NULL, description, NULL }

/// Properties of the ElementPDG class
static PyGetSetDef ElementPDG_getsetters[] = {
    REACTIONS_PYTHON_ELEMENTPDG_GETTER_DESC(name, "str: Name of the element"),
    REACTIONS_PYTHON_ELEMENTPDG_GETTER_DESC(pdg_id, "int: PDG ID"),
    REACTIONS_PYTHON_ELEMENTPDG_GETTER_DESC(three_charge,
                                            "int: Three times the charge"),
    REACTIONS_PYTHON_ELEMENTPDG_GETTER_DESC(
        mass, "float or None: Mass value (:py:obj:`None` if missing)"),
    REACTIONS_PYTHON_ELEMENTPDG_GETTER_DESC(
        mass_error_lower,
        "float or None: Lower mass error (:py:obj:`None` if missing)"),
    REACTIONS_PYTHON_ELEMENTPDG_GETTER_DESC(
        mass_error_upper,
        "float or None: Upper mass error (:py:obj:`None` if missing)"),
    REACTIONS_PYTHON_ELEMENTPDG_GETTER_DESC(
        width, "float or None: Width value (:py:obj:`None` if missing)"),
    REACTIONS_PYTHON_ELEMENTPDG_GETTER_DESC(
        width_error_lower,
        "float or None: Lower width error (:py:obj:`None` if missing)"),
    REACTIONS_PYTHON_ELEMENTPDG_GETTER_DESC(
        width_error_upper,
        "float or None: Upper width error (:py:obj:`None` if missing)"),
    REACTIONS_PYTHON_ELEMENTPDG_GETTER_DESC(
        is_self_cc, "bool: Whether this element is self charge-conjugate"),
    REACTIONS_PYTHON_ELEMENTPDG_GETTER_DESC(charge,
                                            "float: Value of the charge"),
    REACTIONS_PYTHON_ELEMENTPDG_GETTER_DESC(
        mass_error, "float or None: Mass error (:py:obj:`None` if missing)"),
    REACTIONS_PYTHON_ELEMENTPDG_GETTER_DESC(
        width_error, "float or None: Width error (:py:obj:`None` if missing)"),
    REACTIONS_PYTHON_ELEMENTPDG_GETTER_DESC(
        latex_name, "str: Representation of the name to be processed by LaTeX "
                    "(needs to be inserted inside a mathematical expression)"),
};

/// Type declaration
static PyTypeObject ElementPDGType = {
    PyVarObject_HEAD_INIT(NULL, 0) "reactions.pdg_element", /* tp_name */
    sizeof(ElementPDG),                                     /* tp_basicsize */
    0,                                                      /* tp_itemsize */
    0,                                                      /* tp_dealloc */
    0,                                        /* tp_vectorcall_offset */
    0,                                        /* tp_getattr */
    0,                                        /* tp_setattr */
    0,                                        /* tp_as_async */
    (reprfunc)ElementPDG_to_string,           /* tp_repr */
    0,                                        /* tp_as_number */
    0,                                        /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    0,                                        /* tp_hash */
    0,                                        /* tp_call */
    (reprfunc)ElementPDG_to_string,           /* tp_str */
    0,                                        /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    R"(Represent an element of the PDG database. The values that are accessible
can be consulted below. The values of the mass and the width (and their
corresponding errors) can be missing for certain elements. Having defined the
mass or the width means that the errors for this magnitude are also defined.)", /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    ElementPDG_richcompare,    /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    ElementPDG_methods,        /* tp_methods */
    0,                         /* tp_members */
    ElementPDG_getsetters,     /* tp_getset */
    &NodeType,                 /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)ElementPDG_init, /* tp_init */
    0,                         /* tp_alloc */
    ElementPDG_new,            /* tp_new */
};

static PyObject *ElementPDG_richcompare(PyObject *obj1, PyObject *obj2,
                                        int op) {

  if (!PyObject_IsInstance(obj1, (PyObject *)&ElementPDGType) ||
      !PyObject_IsInstance(obj2, (PyObject *)&ElementPDGType)) {
    PyErr_SetString(PyExc_TypeError,
                    "Can only compare \"pdg_element\" objects");
    return NULL;
  }

  bool result;
  switch (op) {
  case Py_EQ:
    result = ((ElementPDG *)obj1)->element == ((ElementPDG *)obj2)->element;
    break;
  case Py_NE:
    result = ((ElementPDG *)obj1)->element != ((ElementPDG *)obj2)->element;
    break;
  default:
    PyErr_SetString(PyExc_NotImplementedError,
                    "Comparison operator not defined");
    return NULL;
  }

  if (result)
    Py_RETURN_TRUE;
  else
    Py_RETURN_FALSE;
}

/// Create a new element using the python API
PyObject *ElementPDG_New(reactions::pdg_element &&el) {
  PyObject *obj =
      ElementPDGType.tp_new((PyTypeObject *)&ElementPDGType, NULL, NULL);
  if (NodeType.tp_init(obj, NULL, NULL) < 0)
    return NULL;
  ((ElementPDG *)obj)->element = el;
  return obj;
}

/// Create a new element using the python API
PyObject *ElementPDG_New(reactions::pdg_element const &el) {
  PyObject *obj =
      ElementPDGType.tp_new((PyTypeObject *)&ElementPDGType, NULL, NULL);
  if (NodeType.tp_init(obj, NULL, NULL) < 0)
    return NULL;
  ((ElementPDG *)obj)->element = el;
  return obj;
}
