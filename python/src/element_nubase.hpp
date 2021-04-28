#pragma once

#include <tuple>

#include "reactions/fields.hpp"
#include "reactions/nubase.hpp"

#include "errors.hpp"
#include "node.hpp"
#include "types.hpp"
#include "utils.hpp"

// Wrapper for a NuBase element
typedef struct {
  // base class
  Node node;
  // attributes (same as nubase_element)
  reactions::nubase_element element;
} ElementNuBase;

// Allocate a new ElementNuBase
static PyObject *ElementNuBase_new(PyTypeObject *type,
                                   PyObject *Py_UNUSED(args),
                                   PyObject *Py_UNUSED(kwargs)) {

  ElementNuBase *self = (ElementNuBase *)type->tp_alloc(type, 0);

  if (!self)
    return NULL;

  // Set the type for the base class
  self->node.c_type = reactions::processes::node_kind::element;

  return (PyObject *)self;
}

/// Initialize the element
static int ElementNuBase_init(ElementNuBase *self, PyObject *args,
                              PyObject *kwargs) {

  if (NodeType.tp_init((PyObject *)self, NULL, NULL) < 0)
    return -1;

  if (args != NULL && PyTuple_Size(args) == 1 && kwargs == NULL) {
    // the first element is the name, and the database is used to access it

    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj))
      return -1;

    try {
      auto const &nubase_instance = reactions::nubase_database::instance();

      if (PyObject_IsInstance(obj, (PyObject *)&PyUnicode_Type)) {
        const char *str = PyUnicode_AsUTF8(obj);
        self->element = nubase_instance(str);
      } else if (PyObject_IsInstance(obj, (PyObject *)&PyLong_Type)) {
        int id = PyLong_AsLong(obj);
        self->element = nubase_instance(id);
      } else {
        PyErr_SetString(PyExc_TypeError, "Argument must be either a string "
                                         "(name) or an integer (NuBase ID)");
        return -1;
      }
    }
    REACTIONS_PYTHON_CATCH_ERRORS(-1)
  } else {

    using parser_type = cpp_to_py_type<reactions::nubase_element::base_type>;

    // the instance is built from the input values
    typename parser_type::type tup;

    PyObject *mass_excess = nullptr, *half_life = nullptr;

    if (args != NULL &&
        PyTuple_Size(args) == reactions::nubase_element::number_of_fields) {

      if (!PyArg_ParseTuple(args, parser_type::descriptor_args,
                            &std::get<0>(tup), &std::get<1>(tup),
                            &std::get<2>(tup), &std::get<3>(tup), &mass_excess,
                            &std::get<5>(tup), &half_life, &std::get<7>(tup)))
        REACTIONS_PYTHON_RETURN_INVALID_ARGUMENTS(-1);

    } else if (kwargs != NULL &&
               PyDict_Size(kwargs) ==
                   reactions::nubase_element::number_of_fields) {
      // the instance is built from the keyword arguments
      static const char *kwds[] = {"name",
                                   "nubase_id",
                                   "atomic_number",
                                   "mass_number",
                                   "mass_excess_and_error_with_tag",
                                   "is_stable",
                                   "half_life_and_error_with_tag",
                                   "is_ground_state",
                                   NULL};

      if (!PyArg_ParseTupleAndKeywords(
              args, kwargs, parser_type::descriptor_kwargs,
              const_cast<char **>(kwds), &std::get<0>(tup), &std::get<1>(tup),
              &std::get<2>(tup), &std::get<3>(tup), &mass_excess,
              &std::get<5>(tup), &half_life, &std::get<7>(tup)))
        REACTIONS_PYTHON_RETURN_INVALID_ARGUMENTS(-1);

    } else
      // Invalid arguments
      REACTIONS_PYTHON_RETURN_INVALID_ARGUMENTS(-1);

    if (mass_excess && mass_excess != Py_None) {

      auto length = PySequence_Length(mass_excess);

      if (length < 1 || length > 3) {
        PyErr_SetString(PyExc_ValueError,
                        "Mass argument must be a sequence of three values");
        return -1;
      }

      double value, error;
      bool from_systematics;
      if (!PyArg_ParseTuple(mass_excess, "ddp", &value, &error,
                            &from_systematics)) {
        PyErr_SetString(
            PyExc_ValueError,
            "Mass argument must be a sequence of three floating point objects");
        return -1;
      }

      std::get<4>(tup) = {value, error, from_systematics};
    }

    if (half_life && half_life != Py_None) {

      auto length = PySequence_Length(half_life);

      if (length < 1 || length > 3) {
        PyErr_SetString(PyExc_ValueError,
                        "Half-life argument must be a sequence "
                        "of two floating point objects and a boolean or None");
        return -1;
      }

      double value, error;
      bool from_systematics;
      if (!PyArg_ParseTuple(half_life, "ddp", &value, &error,
                            &from_systematics)) {
        PyErr_SetString(PyExc_ValueError,
                        "Half-life argument must be a sequence "
                        "of two floating point objects and a boolean");
        return -1;
      }

      std::get<6>(tup) = {value, error, from_systematics};
    }

    self->element = std::move(tup);
  }

  return 0;
}

// Represent the field of a NuBase element as a string
template <std::size_t I>
std::string ElementNuBase_field_to_string(reactions::nubase_element const &el) {

  using field_type =
      std::tuple_element_t<I, reactions::nubase_element::fields_type>;
  using underlying_type_no_opt =
      reactions::fields::remove_optional_t<typename field_type::value_type>;

  std::string title =
      reactions::utils::is_template_specialization_v<
          underlying_type_no_opt, reactions::fields::value_and_error_with_tag>
          ? std::string{field_type::title} + "_and_error_with_tag"
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

// Represent a NuBase element as a string (implementation)
template <std::size_t... I>
std::string ElementNuBase_to_string_impl(reactions::nubase_element const &el,
                                         std::index_sequence<I...>) {
  return (ElementNuBase_field_to_string<I>(el) + ...);
}

/// Representation of the class as a string
static PyObject *ElementNuBase_to_string(ElementNuBase *self) {

  PyTypeObject *type = (PyTypeObject *)PyObject_Type((PyObject *)self);
  if (!type)
    return NULL;

  auto const str =
      std::string{type->tp_name} + '(' +
      ElementNuBase_to_string_impl(
          self->element,
          std::make_index_sequence<
              std::tuple_size_v<reactions::nubase_element::fields_type>>()) +
      ')';
  return PyUnicode_FromString(str.c_str());
}

/// Comparison operator(s)
static PyObject *ElementNuBase_richcompare(PyObject *obj1, PyObject *obj2,
                                           int op);

/// Methods
static PyMethodDef ElementNuBase_methods[] = {
    // sentinel
    {NULL, NULL, 0, NULL}};

/// Define a function to get access to an attribute of a NuBase element
#define REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_DEF(name, converter)             \
  static PyObject *ElementNuBase_get_##name(ElementNuBase *self, void *) {     \
    return converter(self->element.name());                                    \
  }

/// Define a function to get access to an attribute of a NuBase element
#define REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_CHECK_DEF(name, check,           \
                                                        converter)             \
  static PyObject *ElementNuBase_get_##name(ElementNuBase *self, void *) {     \
    if constexpr (reactions::fields::is_optional_field_v<                      \
                      reactions::nubase::check>)                               \
      if (!self->element.has_##check())                                        \
        Py_RETURN_NONE;                                                        \
      else                                                                     \
        return converter(self->element.name());                                \
    else                                                                       \
      return converter(self->element.name());                                  \
  }

/// Define a function to call a member function of a NuBase element
#define REACTIONS_PYTHON_ELEMENTNUBASE_FUNCTION_DEF(name, converter)           \
  static PyObject *ElementNuBase_get_##name(ElementNuBase *self, void *) {     \
    try {                                                                      \
      return converter(self->element.name());                                  \
    }                                                                          \
    REACTIONS_PYTHON_CATCH_ERRORS(NULL)                                        \
  }

/// Convert a std::string to a python unicode string
#define REACTIONS_PYTHON_CPP_STRING_TOPY(string)                               \
  PyUnicode_FromString(string.c_str())

// Define the getters of the ElementNuBase class
REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_DEF(name,
                                          REACTIONS_PYTHON_CPP_STRING_TOPY)
REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_DEF(nubase_id, PyLong_FromLong)
REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_DEF(atomic_number, PyLong_FromLong)
REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_DEF(mass_number, PyLong_FromLong)
REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_CHECK_DEF(mass_excess, mass_excess,
                                                PyFloat_FromDouble)
REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_CHECK_DEF(mass_excess_error, mass_excess,
                                                PyFloat_FromDouble)
REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_CHECK_DEF(mass_excess_from_systematics,
                                                mass_excess, PyBool_FromLong)
REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_CHECK_DEF(half_life, half_life,
                                                PyFloat_FromDouble)
REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_CHECK_DEF(half_life_error, half_life,
                                                PyFloat_FromDouble)
REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_CHECK_DEF(half_life_from_systematics,
                                                half_life, PyBool_FromLong)
REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_DEF(is_stable, PyBool_FromLong)
REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_DEF(is_ground_state, PyBool_FromLong)

// Attributes from functions
REACTIONS_PYTHON_ELEMENTNUBASE_FUNCTION_DEF(latex_name,
                                            REACTIONS_PYTHON_CPP_STRING_TOPY)

/// Provide the data needed to define a "getter" function
#define REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_DESC(name, description)          \
  { #name, (getter)ElementNuBase_get_##name, NULL, description, NULL }

/// Properties of the ElementNuBase class
static PyGetSetDef ElementNuBase_getsetters[] = {
    REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_DESC(name,
                                               "str: Name of the element"),
    REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_DESC(nubase_id, "int: NuBase ID"),
    REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_DESC(atomic_number,
                                               "int: The atomic number"),
    REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_DESC(mass_number,
                                               "int: The mass number"),
    REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_DESC(
        mass_excess,
        "float or None: Mass excess value (:py:obj:`None` if missing)"),
    REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_DESC(
        mass_excess_error,
        "float or None: Mass excess error (:py:obj:`None` if missing)"),
    REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_DESC(
        mass_excess_from_systematics,
        "bool or None: Whether the mass excess has been calculated from "
        "systematics (:py:obj:`None` if missing)"),
    REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_DESC(
        half_life,
        "float or None: Half-life value (:py:obj:`None` if missing)"),
    REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_DESC(
        half_life_error,
        "float or None: Lower half-life error (:py:obj:`None` if missing)"),
    REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_DESC(
        half_life_from_systematics,
        "bool or None: Whether the half-life has been calculated from "
        "systematics (:py:obj:`None` if missing)"),
    REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_DESC(
        is_stable, "bool: Whether this element is stable"),
    REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_DESC(
        is_ground_state, "bool: Whether this element is at its ground state"),
    REACTIONS_PYTHON_ELEMENTNUBASE_GETTER_DESC(
        latex_name, "str: Representation of the name to be processed by LaTeX "
                    "(needs to be inserted inside a mathematical expression)"),
};

/// Type declaration
static PyTypeObject ElementNuBaseType = {
    PyVarObject_HEAD_INIT(NULL, 0) "reactions.nubase_element", /* tp_name */
    sizeof(ElementNuBase),                    /* tp_basicsize */
    0,                                        /* tp_itemsize */
    0,                                        /* tp_dealloc */
    0,                                        /* tp_vectorcall_offset */
    0,                                        /* tp_getattr */
    0,                                        /* tp_setattr */
    0,                                        /* tp_as_async */
    (reprfunc)ElementNuBase_to_string,        /* tp_repr */
    0,                                        /* tp_as_number */
    0,                                        /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    0,                                        /* tp_hash */
    0,                                        /* tp_call */
    (reprfunc)ElementNuBase_to_string,        /* tp_str */
    0,                                        /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    R"(Represent an element of the NuBase database. The values that are accessible
can be consulted below. The values of the mass and the width (and their
corresponding errors) can be missing for certain elements. Having defined the
mass or the width means that the errors for this magnitude are also defined.)", /* tp_doc */
    0,                            /* tp_traverse */
    0,                            /* tp_clear */
    ElementNuBase_richcompare,    /* tp_richcompare */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter */
    0,                            /* tp_iternext */
    ElementNuBase_methods,        /* tp_methods */
    0,                            /* tp_members */
    ElementNuBase_getsetters,     /* tp_getset */
    &NodeType,                    /* tp_base */
    0,                            /* tp_dict */
    0,                            /* tp_descr_get */
    0,                            /* tp_descr_set */
    0,                            /* tp_dictoffset */
    (initproc)ElementNuBase_init, /* tp_init */
    0,                            /* tp_alloc */
    ElementNuBase_new,            /* tp_new */
};

static PyObject *ElementNuBase_richcompare(PyObject *obj1, PyObject *obj2,
                                           int op) {

  if (!PyObject_IsInstance(obj1, (PyObject *)&ElementNuBaseType) ||
      !PyObject_IsInstance(obj2, (PyObject *)&ElementNuBaseType)) {
    PyErr_SetString(PyExc_TypeError,
                    "Can only compare \"nubase_element\" objects");
    return NULL;
  }

  bool result;
  switch (op) {
  case Py_EQ:
    result =
        ((ElementNuBase *)obj1)->element == ((ElementNuBase *)obj2)->element;
    break;
  case Py_NE:
    result =
        ((ElementNuBase *)obj1)->element != ((ElementNuBase *)obj2)->element;
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
PyObject *ElementNuBase_New(reactions::nubase_element &&el) {
  PyObject *obj =
      ElementNuBaseType.tp_new((PyTypeObject *)&ElementNuBaseType, NULL, NULL);
  if (NodeType.tp_init(obj, NULL, NULL) < 0)
    return NULL;
  ((ElementNuBase *)obj)->element = el;
  return obj;
}

/// Create a new element using the python API
PyObject *ElementNuBase_New(reactions::nubase_element const &el) {
  PyObject *obj =
      ElementNuBaseType.tp_new((PyTypeObject *)&ElementNuBaseType, NULL, NULL);
  if (NodeType.tp_init(obj, NULL, NULL) < 0)
    return NULL;
  ((ElementNuBase *)obj)->element = el;
  return obj;
}
