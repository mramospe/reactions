#ifndef REACTIONS_PYTHON_NODE_HPP
#define REACTIONS_PYTHON_NODE_HPP

#include "reactions/processes.hpp"

#define PY_SSIZE_T_CLEAN
#include "Python.h"

namespace {
  using namespace reactions;
}

// Wrapper for a node
typedef struct {
  PyObject_HEAD processes::detail::node_kind c_type =
      processes::detail::node_kind::unknown;
} Node;

#define REACTIONS_PYTHON_NODE_CHECK_UNKNOWN(self)                              \
  if (self->c_type == processes::detail::node_kind::unknown) {                 \
    PyErr_SetString(PyExc_RuntimeError,                                        \
                    "Node type is not defined (internal error); please "       \
                    "report the bug");                                         \
    return NULL;                                                               \
  }

/// Initialize a node
static int Node_init(Node *self, PyObject *Py_UNUSED(args),
                     PyObject *Py_UNUSED(kwargs)) {
  return 0;
}

// Create a new node
static PyObject *Node_new(PyTypeObject *type, PyObject *args,
                          PyObject *Py_UNUSED(kwargs)) {

  Node *self = (Node *)type->tp_alloc(type, 0);

  if (!self)
    return NULL;

  self->c_type = processes::detail::node_kind::unknown;

  return (PyObject *)self;
}

// Definition of the Node type
static PyTypeObject NodeType = {
    PyVarObject_HEAD_INIT(NULL, 0) "reactions.node", /* tp_name */
    sizeof(Node),                                    /* tp_basicsize */
    0,                                               /* tp_itemsize */
    0,                                               /* tp_dealloc */
    0,                                               /* tp_vectorcall_offset */
    0,                                               /* tp_getattr */
    0,                                               /* tp_setattr */
    0,                                               /* tp_as_async */
    0,                                               /* tp_repr */
    0,                                               /* tp_as_number */
    0,                                               /* tp_as_sequence */
    0,                                               /* tp_as_mapping */
    0,                                               /* tp_hash */
    0,                                               /* tp_call */
    0,                                               /* tp_str */
    0,                                               /* tp_getattro */
    0,                                               /* tp_setattro */
    0,                                               /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /* tp_flags */
    "Node object",                                   /* tp_doc */
    0,                                               /* tp_traverse */
    0,                                               /* tp_clear */
    0,                                               /* tp_richcompare */
    0,                                               /* tp_weaklistoffset */
    0,                                               /* tp_iter */
    0,                                               /* tp_iternext */
    0,                                               /* tp_methods */
    0,                                               /* tp_members */
    0,                                               /* tp_getset */
    0,                                               /* tp_base */
    0,                                               /* tp_dict */
    0,                                               /* tp_descr_get */
    0,                                               /* tp_descr_set */
    0,                                               /* tp_dictoffset */
    (initproc)Node_init,                             /* tp_init */
    0,                                               /* tp_alloc */
    Node_new,                                        /* tp_new */
};

/// Check if the object is a node
bool is_node(PyObject *obj) {

  return PyObject_IsInstance(obj, (PyObject *)&NodeType);
}

#endif // REACTIONS_PYTHON_NODE_HPP
