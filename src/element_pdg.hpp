#ifndef REACTIONS_PYTHON_ELEMENTPDG_HPP
#define REACTIONS_PYTHON_ELEMENTPDG_HPP

#include "database.hpp"
#include "errors.hpp"
#include "node.hpp"

// Wrapper for a PDG element
typedef struct {
  // base class
  Node node;
  // attributes (same as pdg::pdg_element)
  reactions::database_pdg::element element;
} ElementPDG;

/// Way to fill an element of the PDG database
inline void python_node_fill_element(ElementPDG *pyel,
                                     database_pdg::element const &el) {
  pyel->element = el;
}

/// Way to fill an element of the PDG database from the base type
inline void python_node_fill_element(ElementPDG *pyel,
                                     database_pdg::element::base_type &&el) {
  pyel->element = std::move(el);
}

// Allocate a new ElementPDG
static PyObject *ElementPDG_new(PyTypeObject *type, PyObject *Py_UNUSED(args),
                                PyObject *Py_UNUSED(kwargs)) {

  ElementPDG *self = (ElementPDG *)type->tp_alloc(type, 0);

  if (!self)
    return NULL;

  // Set the type for the base class
  self->node.c_type = processes::detail::node_kind::element;

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

    DatabasePDG *pdg_instance =
        (DatabasePDG *)DatabasePDGType.tp_new(&DatabasePDGType, NULL, NULL);

    REACTIONS_PYTHON_CHECK_DATABASE(pdg_instance);

    try {
      if (PyObject_IsInstance(obj, (PyObject *)&PyUnicode_Type)) {
        const char *str = PyUnicode_AsUTF8(obj);
        python_node_fill_element(self, pdg_instance->database->operator()(str));
      } else if (PyObject_IsInstance(obj, (PyObject *)&PyLong_Type)) {
        int id = PyLong_AsLong(obj);
        python_node_fill_element(self, pdg_instance->database->operator()(id));
      } else {
        PyErr_SetString(
            PyExc_TypeError,
            "Argument must be either a string (name) or an integer (PDG ID)");
        return -1;
      }
    } catch (exceptions::database_error &e) {

      PyErr_SetString(DatabaseError, e.what());
      return -1;
    }
  } else {

    // the instance is built from the input values
    database_pdg::element::base_type tup;

    if (args != NULL && PyTuple_Size(args) == 9) {
      if (!PyArg_ParseTuple(args, "siifffsif", &std::get<0>(tup),
                            &std::get<1>(tup), &std::get<2>(tup),
                            &std::get<3>(tup), &std::get<4>(tup),
                            &std::get<5>(tup), &std::get<6>(tup),
                            &std::get<7>(tup), &std::get<8>(tup)))
        return -1;
    } else if (kwargs != NULL && PyDict_Size(kwargs) == 9) {
      // the instance is built from the keyword arguments
      static const char *kwds[] = {
          "name", "geant_id",    "pdg_id",    "charge",    "mass",
          "tau",  "evtgen_name", "pythia_id", "max_width", NULL};

      if (!PyArg_ParseTupleAndKeywords(
              args, kwargs, "|$siifffsif", const_cast<char **>(kwds),
              &std::get<0>(tup), &std::get<1>(tup), &std::get<2>(tup),
              &std::get<3>(tup), &std::get<4>(tup), &std::get<5>(tup),
              &std::get<6>(tup), &std::get<7>(tup), &std::get<8>(tup)))
        return -1;
    } else
      // Invalid arguments
      REACTIONS_PYTHON_RETURN_INVALID_ARGUMENTS;

    python_node_fill_element(self, std::move(tup));
  }

  return 0;
}

/// Comparison operator(s)
static PyObject *ElementPDG_richcompare(PyObject *obj1, PyObject *obj2, int op);

/// Methods
static PyMethodDef ElementPDG_methods[] = {
    // sentinel
    {NULL, NULL, 0, NULL}};

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
    "ElementPDG object",                      /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    ElementPDG_richcompare,                   /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    ElementPDG_methods,                       /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    &NodeType,                                /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    (initproc)ElementPDG_init,                /* tp_init */
    0,                                        /* tp_alloc */
    ElementPDG_new,                           /* tp_new */
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

#endif // REACTIONS_PYTHON_PDGELEMENT_HPP
