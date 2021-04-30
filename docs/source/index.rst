.. minkit documentation master file, created by
   sphinx-quickstart on Fri Dec  8 18:24:26 2017.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Introduction
============

The Reactions package allows to define a work with trees of processes among different
elements.
These processes can be either a reaction, where one or more reactants generate a set
of products; or a decay, where a single element generates a set of products.
The current implementation allows to work directly with named objects (where each
element is identified by a string); with PDG elements, using the information from
the PDG database; and with NuBase elements, using the information
from the NuBase database.
In the last two cases, objects can be accessed either by name or by PDG/NuBase ID,
respectively.

.. code-block:: python

   import reactions
   se = reactions.string_element('A') # by string

   reac = reactions.reaction('A B -> C D E F')
   assert len(reac.reactants) == 2
   assert len(reac.products) == 4

   dec = reactions.decay('A -> B C')
   assert dec.head.name == 'A'
   assert len(dec.products) == 2

   reactions.decay('pi+ -> mu+ nu(mu)', kind='pdg') # use the PDG database

   reactions.decay('1n -> 1H e-', kind='nubase') # use the NuBase database

The keyword argument `kind` determines what kind of elements will constitute the
reaction/decay.
String elements are considered by default.
This package also provides the function `is_element`, which allows to check if a
node in a reaction/decay tree is an element or not.

Reaction, decay and element objects can be compared.
In the two first cases, the check is done recursively in the tree on a non-order
sensitive basis, so `A -> B C` is the same as `A -> C B`.

.. code-block:: python

   assert reactions.decay('A -> B C') == reactions.decay('A -> C B')

It is possible to check the syntax of several reactions and/or decays executing
the package as a script

.. code-block:: bash

   python -m reactions check-syntax --decays 'A -> B C' --reactions 'A B -> C D' --kind string
   python -m reactions check-syntax --decays 'K(S)0 -> pi+ pi-' --reactions 'e+ e- -> gamma gamma' --kind pdg
   python -m reactions check-syntax --decays '1n -> 1H e-' --reactions '2H 2H -> 4He' --kind nubase

You can check the script options invoking:

.. code-block:: bash

   python -m reactions --help

This package is also available in `C++`, and the documentation can be visited
`here <_static/cpp/index.html>`_.

The documentation covers the following aspects:

.. toctree::
   :maxdepth: 2

   installation
   notebooks/user_guide
   syntax
   reference
   auxiliar/tmp/changelog
