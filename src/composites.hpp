#ifndef REACTIONS_PYTHON_COMPOSITES_HPP
#define REACTIONS_PYTHON_COMPOSITES_HPP

#include "element.hpp"
#include "errors.hpp"
#include "node.hpp"

#include "reactions/element_traits.hpp"
#include "reactions/pdg.hpp"

#define PY_SSIZE_T_CLEAN
#include "Python.h"

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
  reactions::python::element_kind ek = reactions::python::element_kind::unknown;
  PyObject *reactants; // list of reactants
  PyObject *products;  // list of products
} Reaction;

/// Way to fill a reaction in python
template <class Element>
inline void python_node_fill_reaction(Reaction *,
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
    if (self->ek == reactions::python::unknown) {
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
      python_node_fill_reaction(self, reac);
    }
    REACTIONS_PYTHON_CATCH_ERRORS(-1)

    break;
  }
  case (reactions::python::element_kind::string): {

    try {
      auto reac = reactions::make_reaction<reactions::string_element>(str);
      python_node_fill_reaction(self, reac);
      break;
    }
    REACTIONS_PYTHON_CATCH_ERRORS(-1)
  }
  case (reactions::python::element_kind::unknown):

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
  self->node.c_type = reactions::processes::node_kind::reaction;

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
    {"reactants", (getter)Reaction_get_reactants, NULL, "Get the reactants",
     NULL},
    {"products", (getter)Reaction_get_products, NULL, "Get the products", NULL},
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
    "Reaction object",                        /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    Reaction_richcompare,                     /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    Reaction_methods,                         /* tp_methods */
    0,                                        /* tp_members */
    Reaction_getsetters,                      /* tp_getset */
    &NodeType,                                /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    (initproc)Reaction_init,                  /* tp_init */
    0,                                        /* tp_alloc */
    Reaction_new,                             /* tp_new */
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

/// Way to fill a reaction in python
template <class Element>
inline void
python_node_fill_reaction(Reaction *self,
                          reactions::reaction<Element> const &reac) {

  using object = python_element_object_o<Element>;
  constexpr static PyTypeObject *type_ptr = python_element_object_t<Element>;

  // will be re-assigned
  Py_DecRef(self->reactants);
  self->reactants = PyList_New(reac.reactants().size());
  Py_DecRef(self->products);
  self->products = PyList_New(reac.products().size());

  // fill reactants
  for (auto i = 0u; i < reac.reactants().size(); ++i) {

    auto &obj = reac.reactants().at(i);

    if (obj.is_element()) {
      object *e = (object *)type_ptr->tp_new(type_ptr, NULL, NULL);
      e->element = *(obj.ptr_as_element());
      PyList_SetItem(self->reactants, i, (PyObject *)e); // steals the reference
    } else {
      Reaction *r = (Reaction *)ReactionType.tp_new(&ReactionType, NULL, NULL);
      python_node_fill_reaction(r, *(obj.ptr_as_reaction()));
      PyList_SetItem(self->reactants, i, (PyObject *)r); // steals the reference
    }
  }

  // fill products
  for (auto i = 0u; i < reac.products().size(); ++i) {

    auto &obj = reac.products().at(i);

    if (obj.is_element()) {
      object *e = (object *)type_ptr->tp_new(type_ptr, NULL, NULL);
      e->element = *(obj.ptr_as_element());
      PyList_SetItem(self->products, i, (PyObject *)e); // steals the reference
    } else {
      Reaction *r = (Reaction *)ReactionType.tp_new(&ReactionType, NULL, NULL);
      python_node_fill_reaction(r, *(obj.ptr_as_reaction()));
      PyList_SetItem(self->products, i, (PyObject *)r); // steals the reference
    }
  }
}

//********************************************
// DECAY CLASS
//********************************************

// Wrapper for a reaction
typedef struct {
  // base class
  Node node;
  // attributes
  reactions::python::element_kind ek = reactions::python::element_kind::unknown;
  PyObject *head;     // head of the decay
  PyObject *products; // list of products
} Decay;

// Way to fill a decay in python
template <class Element>
inline void python_node_fill_decay(Decay *self,
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
    if (self->ek == reactions::python::element_kind::unknown) {
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
      auto reac = reactions::make_decay_for<reactions::pdg_element>(
          str, [](std::string const &s) -> reactions::pdg_element {
            return reactions::pdg_database::instance()(s);
          });
      python_node_fill_decay(self, reac);
    }
    REACTIONS_PYTHON_CATCH_ERRORS(-1)

    break;
  }
  case (reactions::python::element_kind::string): {

    try {
      auto reac = reactions::make_decay<reactions::string_element>(str);
      python_node_fill_decay(self, reac);
    }
    REACTIONS_PYTHON_CATCH_ERRORS(-1)

    break;
  }
  case (reactions::python::element_kind::unknown):

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
  self->node.c_type = reactions::processes::node_kind::decay;

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
     "Get the head of the decay", NULL},
    {"products", (getter)Decay_get_products, NULL, "Get the products", NULL},
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
    "Decay object",                                   /* tp_doc */
    0,                                                /* tp_traverse */
    0,                                                /* tp_clear */
    Decay_richcompare,                                /* tp_richcompare */
    0,                                                /* tp_weaklistoffset */
    0,                                                /* tp_iter */
    0,                                                /* tp_iternext */
    Decay_methods,                                    /* tp_methods */
    0,                                                /* tp_members */
    Decay_getsetters,                                 /* tp_getset */
    &NodeType,                                        /* tp_base */
    0,                                                /* tp_dict */
    0,                                                /* tp_descr_get */
    0,                                                /* tp_descr_set */
    0,                                                /* tp_dictoffset */
    (initproc)Decay_init,                             /* tp_init */
    0,                                                /* tp_alloc */
    Decay_new,                                        /* tp_new */
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

/// Way to fill a decay in python
template <class Element>
inline void python_node_fill_decay(Decay *self,
                                   reactions::decay<Element> const &reac) {

  using object = python_element_object_o<Element>;
  constexpr static PyTypeObject *type_ptr = python_element_object_t<Element>;

  // will be re-assigned
  Py_DecRef(self->head);
  self->head = (PyObject *)type_ptr->tp_new(type_ptr, NULL, NULL);
  ((object *)self->head)->element = reac.head();

  Py_DecRef(self->products);
  self->products = PyList_New(reac.products().size());

  // fill products
  for (auto i = 0u; i < reac.products().size(); ++i) {

    auto &obj = reac.products().at(i);

    if (obj.is_element()) {
      object *e = (object *)type_ptr->tp_new(type_ptr, NULL, NULL);
      e->element = *(obj.ptr_as_element());
      PyList_SetItem(self->products, i, (PyObject *)e); // steals the reference
    } else {
      Decay *r = (Decay *)DecayType.tp_new(&DecayType, NULL, NULL);
      python_node_fill_decay(r, *(obj.ptr_as_decay()));
      PyList_SetItem(self->products, i, (PyObject *)r); // steals the reference
    }
  }
}

#endif // REACTIONS_PYTHON_COMPOSITES_HPP
