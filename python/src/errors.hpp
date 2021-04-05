#pragma once

#include "reactions/exceptions.hpp"

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

// Definition of an internal error
static PyObject *InternalError =
    PyErr_NewException("reactions.InternalError", PyExc_RuntimeError, NULL);

#define REACTIONS_PYTHON_UNEXPECTED_BEHAVIOUR                                  \
  {                                                                            \
    PyErr_SetString(                                                           \
        PyExc_RuntimeError,                                                    \
        "Unexpected behaviour (internal error); please report the bug");       \
    return -1;                                                                 \
  }

#define REACTIONS_PYTHON_RETURN_INVALID_ARGUMENTS(returncode)                  \
  {                                                                            \
    PyErr_SetString(PyExc_RuntimeError, "Invalid arguments");                  \
    return returncode;                                                         \
  }

#define REACTIONS_PYTHON_CATCH_ERRORS(returncode, ...)                         \
  catch (reactions::syntax_error & e) {                                        \
    PyErr_SetString(SyntaxError, e.what());                                    \
    __VA_ARGS__;                                                               \
    return returncode;                                                         \
  }                                                                            \
  catch (reactions::database_error & e) {                                      \
    PyErr_SetString(DatabaseError, e.what());                                  \
    __VA_ARGS__;                                                               \
    return returncode;                                                         \
  }                                                                            \
  catch (reactions::lookup_error & e) {                                        \
    PyErr_SetString(LookupError, e.what());                                    \
    __VA_ARGS__;                                                               \
    return returncode;                                                         \
  }                                                                            \
  catch (reactions::internal_error & e) {                                      \
    PyErr_SetString(InternalError, e.what());                                  \
    __VA_ARGS__;                                                               \
    return returncode;                                                         \
  }                                                                            \
  catch (...) {                                                                \
    __VA_ARGS__;                                                               \
    return returncode;                                                         \
  }
