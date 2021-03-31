#ifndef REACTIONS_PYTHON_ELEMENTSTRING_HPP
#define REACTIONS_PYTHON_ELEMENTSTRING_HPP

#include "reactions/processes.hpp"

#include <string>

// Wrapper for a string element
typedef struct {
  // base class
  Node node;
  // attributes
  std::string element;
} ElementString;

// Allocate a new ElementString
static PyObject *ElementString_new(PyTypeObject *type,
                                   PyObject *Py_UNUSED(args),
                                   PyObject *Py_UNUSED(kwargs)) {

  ElementString *self = (ElementString *)type->tp_alloc(type, 0);

  if (!self)
    return NULL;

  // Set the type for the base class
  self->node.c_type = processes::detail::node_kind::element;

  return (PyObject *)self;
}

/// Initialize the element
static int ElementString_init(ElementString *self, PyObject *args,
                              PyObject *kwargs) {

  if (NodeType.tp_init((PyObject *)self, NULL, NULL) < 0)
    return -1;

  const char *str;
  if (!PyArg_ParseTuple(args, "s", &str))
    return -1;

  // no database for this class
  self->element = str;

  return 0;
}

/// Access the name
static PyObject *ElementString_get_name(ElementString *self, void *) {
  return PyUnicode_FromString(self->element.c_str());
}

/// Properties of the Reaction class
static PyGetSetDef ElementString_getsetters[] = {
    {"name", (getter)ElementString_get_name, NULL, "Get the name", NULL},
};

/// Comparison operator(s)
static PyObject *ElementString_richcompare(PyObject *obj1, PyObject *obj2,
                                           int op);

/// Methods
static PyMethodDef ElementString_methods[] = {
    // sentinel
    {NULL, NULL, 0, NULL}};

/// Type declaration
static PyTypeObject ElementStringType = {
    PyVarObject_HEAD_INIT(NULL, 0) "reactions.string_element", /* tp_name */
    sizeof(ElementString),                    /* tp_basicsize */
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
    0,                                        /* tp_call */
    0,                                        /* tp_str */
    0,                                        /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    "ElementString object",                   /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    ElementString_richcompare,                /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    ElementString_methods,                    /* tp_methods */
    0,                                        /* tp_members */
    ElementString_getsetters,                 /* tp_getset */
    &NodeType,                                /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    (initproc)ElementString_init,             /* tp_init */
    0,                                        /* tp_alloc */
    ElementString_new,                        /* tp_new */
};

static PyObject *ElementString_richcompare(PyObject *obj1, PyObject *obj2,
                                           int op) {

  if (!PyObject_IsInstance(obj1, (PyObject *)&ElementStringType) ||
      !PyObject_IsInstance(obj2, (PyObject *)&ElementStringType)) {
    PyErr_SetString(PyExc_TypeError,
                    "Can only compare \"string_element\" objects");
    return NULL;
  }

  bool result;
  switch (op) {
  case Py_EQ:
    result =
        ((ElementString *)obj1)->element == ((ElementString *)obj2)->element;
    break;
  case Py_NE:
    result =
        ((ElementString *)obj1)->element != ((ElementString *)obj2)->element;
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

#endif // REACTIONS_PYTHON_ELEMENTSTRING_HPP
