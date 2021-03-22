#ifndef REACTIONS_PYTHON_ERRORS_HPP
#define REACTIONS_PYTHON_ERRORS_HPP

#define PY_SSIZE_T_CLEAN
#include "Python.h"

// Definition of an error due to a missing database
static PyObject *DatabaseError =
    PyErr_NewException("reactions.DatabaseError", PyExc_RuntimeError, NULL);

// Definition of lookup error
static PyObject *LookupError =
    PyErr_NewException("reactions.LookupError", PyExc_RuntimeError, NULL);

// Definition of the syntax error
static PyObject *SyntaxError =
    PyErr_NewException("reactions.SyntaxError", PyExc_RuntimeError, NULL);

#define REACTIONS_PYTHON_UNEXPECTED_BEHAVIOUR                                  \
  {                                                                            \
    PyErr_SetString(                                                           \
        PyExc_RuntimeError,                                                    \
        "Unexpected behaviour (internal error); please report the bug");       \
    return -1;                                                                 \
  }

#define REACTIONS_PYTHON_RETURN_INVALID_ARGUMENTS                              \
  {                                                                            \
    PyErr_SetString(PyExc_RuntimeError, "Invalid arguments");                  \
    return -1;                                                                 \
  }

#endif
