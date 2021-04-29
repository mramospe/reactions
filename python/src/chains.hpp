#pragma once

#define PY_SSIZE_T_CLEAN
#include "Python.h"

#include "reactions/element_traits.hpp"
#include "reactions/nubase.hpp"
#include "reactions/pdg.hpp"

#include "element.hpp"
#include "errors.hpp"
#include "node.hpp"

// default element type
#define REACTIONS_PYTHON_DEFAULT_ELEMENT_TYPE "string"

/// Compare two lists of nodes
inline int compare_nodes(PyObject *first, PyObject *second, int op) {

  auto size = PyList_Size(first);

  if (size != PyList_Size(second))
    return false;

  std::vector<bool> mask(size, false);

  // do a check for the object types
  for (auto i = 0u; i < size; ++i) {

    PyObject *fo = PyList_GetItem(first, i);
    PyObject *so = PyList_GetItem(second, i);

    if (!PyObject_IsInstance(fo, (PyObject *)&NodeType) ||
        !PyObject_IsInstance(so, (PyObject *)&NodeType)) {
      PyErr_SetString(PyExc_TypeError, "List objects must be of node type");
      return -1;
    }
  }

  for (auto i = 0u; i < size; ++i) {

    PyObject *fo = PyList_GetItem(first, i);

    for (auto j = 0u; j < size; ++j) {

      if (mask[j])
        continue;

      PyObject *so = PyList_GetItem(second, j);

      PyObject *ro = PyObject_RichCompare(fo, so, op);

      if (ro == NULL) {
        Py_DecRef(ro);
        return -1;
      } else if (PyObject_IsTrue(ro)) {
        mask[j] = true;
        Py_DecRef(ro);
        break;
      } else {
        Py_DecRef(ro);
        continue;
      }

      // no match has been found
      return false;
    }
  }

  return true;
}

//********************************************
// REACTION CLASS
//********************************************

// Wrapper for a reaction
typedef struct {
  // base class
  Node node;
  // attributes
  reactions::python::element_kind ek =
      reactions::python::element_kind::unknown_element_kind;
  PyObject *reactants; // list of reactants
  PyObject *products;  // list of products
} Reaction;

/// Way to fill a reaction in python
template <class Element>
inline bool python_node_fill_reaction(Reaction *,
                                      reactions::reaction<Element> const &);

// Initialize a new reaction
static int Reaction_init(Reaction *self, PyObject *args, PyObject *kwargs) {

  if (NodeType.tp_init((PyObject *)self, NULL, NULL) < 0)
    return -1;

  const char *str = nullptr;
  const char *kind = REACTIONS_PYTHON_DEFAULT_ELEMENT_TYPE;

  static const char *kwds[] = {"string", "kind", NULL};

  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|ss",
                                   const_cast<char **>(kwds), &str, &kind))
    return -1;

  self->ek = reactions::python::element_kind_properties::from_string(kind);

  if (!str) {
    // initialization with no values
    if (self->ek == reactions::python::unknown_element_kind) {
      PyErr_SetString(
          PyExc_ValueError,
          (std::string{"Unknown element type \""} + kind + "\"").c_str());
      return -1;
    } else {

      Py_DecRef(self->reactants);
      self->reactants = PyList_New(0);
      Py_DecRef(self->products);
      self->products = PyList_New(0);

      return 0;
    }
  }

  switch (self->ek) {

  case (reactions::python::element_kind::pdg): {

    try {
      auto reac = reactions::make_reaction_for<reactions::pdg_element>(
          str, [](std::string const &s) -> reactions::pdg_element {
            return reactions::pdg_database::instance()(s);
          });
      if (!python_node_fill_reaction(self, reac))
        return -1;
    }
    REACTIONS_PYTHON_CATCH_ERRORS(-1)

    break;
  }
  case (reactions::python::element_kind::nubase): {

    try {
      auto reac = reactions::make_reaction_for<reactions::nubase_element>(
          str, [](std::string const &s) -> reactions::nubase_element {
            return reactions::nubase_database::instance()(s);
          });
      if (!python_node_fill_reaction(self, reac))
        return -1;
    }
    REACTIONS_PYTHON_CATCH_ERRORS(-1)

    break;
  }
  case (reactions::python::element_kind::string): {

    try {
      auto reac = reactions::make_reaction<reactions::string_element>(str);
      if (!python_node_fill_reaction(self, reac))
        return -1;
      break;
    }
    REACTIONS_PYTHON_CATCH_ERRORS(-1)
  }
  case (reactions::python::element_kind::unknown_element_kind):

    PyErr_SetString(PyExc_ValueError,
                    (std::string{"Unknown element type "} + kind).c_str());
    return -1;
  }

  return 0;
}

/// Allocate a new object
static PyObject *Reaction_new(PyTypeObject *type, PyObject *Py_UNUSED(args),
                              PyObject *Py_UNUSED(kwds)) {

  Reaction *self = (Reaction *)type->tp_alloc(type, 0);

  if (!self)
    return NULL;

  // Set the type for the base class
  self->node.c_type = reactions::processes::node_type::reaction;

  return (PyObject *)self;
}

/// Comparison operator(s)
static PyObject *Reaction_richcompare(PyObject *obj1, PyObject *obj2, int op);

static PyMethodDef Reaction_methods[] = {
    // sentinel
    {NULL, NULL, 0, NULL}};

/// Getter (reactants)
static PyObject *Reaction_get_reactants(Reaction *self, void *) {
  Py_IncRef(self->reactants);
  return self->reactants;
}

/// Getter (products)
static PyObject *Reaction_get_products(Reaction *self, void *) {
  Py_IncRef(self->products);
  return self->products;
}

/// Properties of the Reaction class
static PyGetSetDef Reaction_getsetters[] = {
    {"reactants", (getter)Reaction_get_reactants, NULL,
     "list(node): Objects at the left hand-side of a reaction", NULL},
    {"products", (getter)Reaction_get_products, NULL,
     "list(node): Objects at the right hand-side of a reaction", NULL},
    {NULL},
};

static PyTypeObject ReactionType = {
    PyVarObject_HEAD_INIT(NULL, 0) "reactions.reaction", /* tp_name */
    sizeof(Reaction),                                    /* tp_basicsize */
    0,                                                   /* tp_itemsize */
    0,                                                   /* tp_dealloc */
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
    "Reaction involving several elements of the same type", /* tp_doc */
    0,                                                      /* tp_traverse */
    0,                                                      /* tp_clear */
    Reaction_richcompare,                                   /* tp_richcompare */
    0,                       /* tp_weaklistoffset */
    0,                       /* tp_iter */
    0,                       /* tp_iternext */
    Reaction_methods,        /* tp_methods */
    0,                       /* tp_members */
    Reaction_getsetters,     /* tp_getset */
    &NodeType,               /* tp_base */
    0,                       /* tp_dict */
    0,                       /* tp_descr_get */
    0,                       /* tp_descr_set */
    0,                       /* tp_dictoffset */
    (initproc)Reaction_init, /* tp_init */
    0,                       /* tp_alloc */
    Reaction_new,            /* tp_new */
};

/// Implementation of the comparison operators
static PyObject *Reaction_richcompare(PyObject *obj1, PyObject *obj2, int op) {

  if (!PyObject_IsInstance(obj1, (PyObject *)&ReactionType) ||
      !PyObject_IsInstance(obj2, (PyObject *)&ReactionType))
    Py_RETURN_FALSE;

  if (((Reaction *)obj1)->ek != ((Reaction *)obj2)->ek) {
    PyErr_SetString(PyExc_TypeError,
                    "Reactions contain objects of different types");
    return NULL;
  }

  auto code = compare_nodes(((Reaction *)obj1)->reactants,
                            ((Reaction *)obj2)->reactants, op) *
              compare_nodes(((Reaction *)obj1)->products,
                            ((Reaction *)obj2)->products, op);

  if (code < 0) // error set by compare_nodes
    return NULL;

  if (code)
    Py_RETURN_TRUE;
  else
    Py_RETURN_FALSE;
}

/// Create a new instance of the Reaction class
template <class Element>
PyObject *Reaction_New(reactions::reaction<Element> const &reaction) {
  Reaction *r = (Reaction *)ReactionType.tp_new(&ReactionType, NULL, NULL);
  if (!python_node_fill_reaction(r, reaction))
    return NULL;
  return (PyObject *)r;
}

/// Way to fill a reaction in python (set an error and return False on failure)
template <class Element>
inline bool
python_node_fill_reaction(Reaction *self,
                          reactions::reaction<Element> const &reac) {

  // will be re-assigned
  Py_DecRef(self->reactants);
  self->reactants = PyList_New(reac.reactants().size());
  Py_DecRef(self->products);
  self->products = PyList_New(reac.products().size());

  if (!self->reactants || !self->products) {
    PyErr_SetString(InternalError, "Unable to create a new list");
    return false;
  }

  // fill reactants
  for (auto i = 0u; i < reac.reactants().size(); ++i) {

    auto &obj = reac.reactants().at(i);

    PyObject *o = obj.is_element()
                      ? python_element<Element>::new_instance(obj.as_element())
                      : Reaction_New(obj.as_chain());

    if (!o) {
      PyErr_SetString(InternalError, "Unable to create element");
      return false;
    }

    PyList_SetItem(self->reactants, i, o); // steals the reference
  }

  // fill products
  for (auto i = 0u; i < reac.products().size(); ++i) {

    auto &obj = reac.products().at(i);

    PyObject *o = obj.is_element()
                      ? python_element<Element>::new_instance(obj.as_element())
                      : Reaction_New(obj.as_chain());

    if (!o) {
      PyErr_SetString(InternalError, "Unable to create element");
      return false;
    }

    PyList_SetItem(self->products, i, o); // steals the reference
  }

  return true;
}

//********************************************
// DECAY CLASS
//********************************************

// Wrapper for a reaction
typedef struct {
  // base class
  Node node;
  // attributes
  reactions::python::element_kind ek =
      reactions::python::element_kind::unknown_element_kind;
  PyObject *head;     // head of the decay
  PyObject *products; // list of products
} Decay;

// Way to fill a decay in python
template <class Element>
inline bool python_node_fill_decay(Decay *self,
                                   reactions::decay<Element> const &reac);

// Initialize a new reaction
static int Decay_init(Decay *self, PyObject *args, PyObject *kwargs) {

  if (NodeType.tp_init((PyObject *)self, NULL, NULL) < 0)
    return -1;

  const char *str = nullptr;
  const char *kind = REACTIONS_PYTHON_DEFAULT_ELEMENT_TYPE;

  static const char *kwds[] = {"string", "kind", NULL};

  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|ss",
                                   const_cast<char **>(kwds), &str, &kind))
    return -1;

  self->ek = reactions::python::element_kind_properties::from_string(kind);

  if (!str) {
    // initialization with no values
    if (self->ek == reactions::python::element_kind::unknown_element_kind) {
      PyErr_SetString(
          PyExc_ValueError,
          (std::string{"Unknown element type \""} + kind + "\"").c_str());
      return -1;
    } else {

      Py_DecRef(self->head);
      self->head = Py_None;
      Py_IncRef(self->head);

      Py_DecRef(self->products);
      self->products = PyList_New(0);

      return 0;
    }
  }

  switch (self->ek) {

  case (reactions::python::element_kind::pdg): {

    try {
      auto dec = reactions::make_decay_for<reactions::pdg_element>(
          str, [](std::string const &s) -> reactions::pdg_element {
            return reactions::pdg_database::instance()(s);
          });
      if (!python_node_fill_decay(self, dec))
        return -1;
    }
    REACTIONS_PYTHON_CATCH_ERRORS(-1)

    break;
  }
  case (reactions::python::element_kind::nubase): {

    try {
      auto dec = reactions::make_decay_for<reactions::nubase_element>(
          str, [](std::string const &s) -> reactions::nubase_element {
            return reactions::nubase_database::instance()(s);
          });
      if (!python_node_fill_decay(self, dec))
        return -1;
    }
    REACTIONS_PYTHON_CATCH_ERRORS(-1)

    break;
  }
  case (reactions::python::element_kind::string): {

    try {
      auto dec = reactions::make_decay<reactions::string_element>(str);
      if (!python_node_fill_decay(self, dec))
        return -1;
    }
    REACTIONS_PYTHON_CATCH_ERRORS(-1)

    break;
  }
  case (reactions::python::element_kind::unknown_element_kind):

    PyErr_SetString(PyExc_ValueError,
                    (std::string{"Unknown element type "} + kind).c_str());
    return -1;
  }

  return 0;
}

/// Allocate a new object
static PyObject *Decay_new(PyTypeObject *type, PyObject *Py_UNUSED(args),
                           PyObject *Py_UNUSED(kwds)) {

  Decay *self = (Decay *)type->tp_alloc(type, 0);

  if (!self)
    return NULL;

  // Set the type for the base class
  self->node.c_type = reactions::processes::node_type::decay;

  return (PyObject *)self;
}

/// Comparison operator(s)
static PyObject *Decay_richcompare(PyObject *obj1, PyObject *obj2, int op);

static PyMethodDef Decay_methods[] = {
    // sentinel
    {NULL, NULL, 0, NULL}};

/// Getter (head)
static PyObject *Decay_get_head(Decay *self, void *) {
  Py_IncRef(self->head);
  return self->head;
}

/// Setter (head)
static int Decay_set_head(Decay *self, PyObject *obj, void *) {
  self->head = obj;
  Py_IncRef(self->head);
  auto dec = object_is_element(self->head);
  if (dec < 0) {
    PyErr_SetString(PyExc_TypeError, "Head type must be an element");
    return -1;
  } else
    return 0;
}

/// Getter (products)
static PyObject *Decay_get_products(Decay *self, void *) {
  Py_IncRef(self->products);
  return self->products;
}

/// Properties of the Decay class
static PyGetSetDef Decay_getsetters[] = {
    {"head", (getter)Decay_get_head, (setter)Decay_set_head,
     "pdg_element, nubase_element or string_element: Element at the left "
     "hand-side of a decay",
     NULL},
    {"products", (getter)Decay_get_products, NULL,
     "list(node): Objects at the right hand-side of a decay", NULL},
    {NULL},
};

static PyTypeObject DecayType = {
    PyVarObject_HEAD_INIT(NULL, 0) "reactions.decay", /* tp_name */
    sizeof(Decay),                                    /* tp_basicsize */
    0,                                                /* tp_itemsize */
    0,                                                /* tp_dealloc */
    0,                                                /* tp_vectorcall_offset */
    0,                                                /* tp_getattr */
    0,                                                /* tp_setattr */
    0,                                                /* tp_as_async */
    0,                                                /* tp_repr */
    0,                                                /* tp_as_number */
    0,                                                /* tp_as_sequence */
    0,                                                /* tp_as_mapping */
    0,                                                /* tp_hash */
    0,                                                /* tp_call */
    0,                                                /* tp_str */
    0,                                                /* tp_getattro */
    0,                                                /* tp_setattro */
    0,                                                /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,         /* tp_flags */
    "Decay involving several elements of the same type", /* tp_doc */
    0,                                                   /* tp_traverse */
    0,                                                   /* tp_clear */
    Decay_richcompare,                                   /* tp_richcompare */
    0,                                                   /* tp_weaklistoffset */
    0,                                                   /* tp_iter */
    0,                                                   /* tp_iternext */
    Decay_methods,                                       /* tp_methods */
    0,                                                   /* tp_members */
    Decay_getsetters,                                    /* tp_getset */
    &NodeType,                                           /* tp_base */
    0,                                                   /* tp_dict */
    0,                                                   /* tp_descr_get */
    0,                                                   /* tp_descr_set */
    0,                                                   /* tp_dictoffset */
    (initproc)Decay_init,                                /* tp_init */
    0,                                                   /* tp_alloc */
    Decay_new,                                           /* tp_new */
};

/// Implementation of the comparison operators
static PyObject *Decay_richcompare(PyObject *obj1, PyObject *obj2, int op) {

  if (!PyObject_IsInstance(obj1, (PyObject *)&DecayType) ||
      !PyObject_IsInstance(obj2, (PyObject *)&DecayType))
    Py_RETURN_FALSE;

  if (((Decay *)obj1)->ek != ((Decay *)obj2)->ek) {
    PyErr_SetString(PyExc_TypeError,
                    "Decays contain objects of different types");
    return NULL;
  }

  auto ro =
      PyObject_RichCompare(((Decay *)obj1)->head, ((Decay *)obj2)->head, op);

  if (!PyObject_IsTrue(ro))
    return ro;

  auto code =
      compare_nodes(((Decay *)obj1)->products, ((Decay *)obj2)->products, op);

  if (code < 0) // error set by compare_nodes
    return NULL;

  if (code)
    Py_RETURN_TRUE;
  else
    Py_RETURN_FALSE;
}

/// Create a new instance of the Decay class
template <class Element>
PyObject *Decay_New(reactions::decay<Element> const &decay) {
  Decay *d = (Decay *)DecayType.tp_new(&DecayType, NULL, NULL);
  if (!python_node_fill_decay(d, decay))
    return NULL;
  return (PyObject *)d;
}

/// Way to fill a decay in python
template <class Element>
inline bool python_node_fill_decay(Decay *self,
                                   reactions::decay<Element> const &reac) {

  // will be re-assigned
  Py_DecRef(self->head);
  self->head = python_element<Element>::new_instance(reac.head());

  if (!self->head) {
    PyErr_SetString(InternalError, "Unable to create element");
    return false;
  }

  Py_DecRef(self->products);
  self->products = PyList_New(reac.products().size());

  if (!self->products) {
    PyErr_SetString(InternalError, "Unable to create list");
    return false;
  }

  // fill products
  for (auto i = 0u; i < reac.products().size(); ++i) {

    auto &obj = reac.products().at(i);

    PyObject *o = obj.is_element()
                      ? python_element<Element>::new_instance(obj.as_element())
                      : Decay_New(obj.as_chain());

    if (!o) {
      PyErr_SetString(InternalError, "Unable to create decay");
      return false;
    }

    PyList_SetItem(self->products, i, o); // steals the reference
  }

  return true;
}
