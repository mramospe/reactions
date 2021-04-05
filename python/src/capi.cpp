#include "composites.hpp"
#include "database.hpp"
#include "element_pdg.hpp"
#include "element_string.hpp"
#include "node.hpp"

#define PY_SSIZE_T_CLEAN
#include "Python.h"

// Global function to check whether the input object is an element
PyObject *is_element(PyObject *module, PyObject *args) {

  PyObject *obj = nullptr;
  if (!PyArg_ParseTuple(args, "O", &obj))
    return NULL;

  auto dec = object_is_element(obj);

  if (dec < 0)
    return NULL;
  else if (dec)
    Py_RETURN_TRUE;
  else
    Py_RETURN_FALSE;
}

// Get the type of the node as a string
PyObject *node_type(PyObject *module, PyObject *args) {

  PyObject *obj = nullptr;
  if (!PyArg_ParseTuple(args, "O", &obj))
    return NULL;

  if (!PyObject_IsInstance(obj, (PyObject *)&NodeType)) {
    PyErr_SetString(PyExc_RuntimeError, "Input argument must be a node");
    return NULL;
  }

  REACTIONS_PYTHON_NODE_CHECK_UNKNOWN(((Node *)obj));

  return PyUnicode_FromString(
      reactions::processes::node_kind_properties::to_c_string(
          ((Node *)obj)->c_type));
}

// Module global functions
static PyMethodDef capi_methods[] = {
    {"is_element", (PyCFunction)is_element, METH_VARARGS,
     "Check if an object is of element type"},
    {"node_type", (PyCFunction)node_type, METH_VARARGS,
     "Get the node type as a string"},
    {NULL, NULL, 0, NULL}};

// Module definition
static PyModuleDef capi_module = {PyModuleDef_HEAD_INIT,
                                  "capi",
                                  "C++ bindings of the reactions package",
                                  -1,
                                  capi_methods,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL};

// Prepare the class and return on failure
#define REACTIONS_PYTHON_CLASS_READY(type)                                     \
  if (PyType_Ready(&type) < 0)                                                 \
    return NULL;

// Register the class and return on failure
#define REACTIONS_PYTHON_REGISTER_CLASS(module, name, type)                    \
  Py_INCREF(&type);                                                            \
  if (PyModule_AddObject(module, name, (PyObject *)&type) < 0) {               \
    Py_DECREF(&type);                                                          \
    Py_DECREF(module);                                                         \
    return NULL;                                                               \
  }

#define REACTIONS_PYTHON_REGISTER_ERROR(module, name)                          \
  if (PyModule_AddObject(module, #name, name) < 0) {                           \
    Py_DECREF(module);                                                         \
    return NULL;                                                               \
  }

// Initialize the module
PyMODINIT_FUNC PyInit_capi(void) {

  // Types
  REACTIONS_PYTHON_CLASS_READY(NodeType);
  REACTIONS_PYTHON_CLASS_READY(ElementStringType);
  REACTIONS_PYTHON_CLASS_READY(ElementPDGType);
  REACTIONS_PYTHON_CLASS_READY(ReactionType);
  REACTIONS_PYTHON_CLASS_READY(DecayType);
  REACTIONS_PYTHON_CLASS_READY(DatabasePDGType);
  REACTIONS_PYTHON_CLASS_READY(SystemOfUnitsPDGType);

  // Create the module
  PyObject *m = PyModule_Create(&capi_module);
  if (!m)
    return NULL;

  // Add types
  REACTIONS_PYTHON_REGISTER_CLASS(m, "node", NodeType);
  REACTIONS_PYTHON_REGISTER_CLASS(m, "pdg_element", ElementPDGType);
  REACTIONS_PYTHON_REGISTER_CLASS(m, "string_element", ElementStringType);
  REACTIONS_PYTHON_REGISTER_CLASS(m, "reaction", ReactionType);
  REACTIONS_PYTHON_REGISTER_CLASS(m, "decay", DecayType);
  REACTIONS_PYTHON_REGISTER_CLASS(m, "pdg_database_sgl", DatabasePDGType);
  REACTIONS_PYTHON_REGISTER_CLASS(m, "pdg_system_of_units_sgl",
                                  SystemOfUnitsPDGType);

  // Add errors
  REACTIONS_PYTHON_REGISTER_ERROR(m, DatabaseError);
  REACTIONS_PYTHON_REGISTER_ERROR(m, LookupError);
  REACTIONS_PYTHON_REGISTER_ERROR(m, SyntaxError);
  REACTIONS_PYTHON_REGISTER_ERROR(m, InternalError);
  REACTIONS_PYTHON_REGISTER_ERROR(m, ValueError);

  return m;
}