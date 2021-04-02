Syntax
======

The syntax to write reactions is slightly different to that of decays, but
they share many point in common.
Both are composed by two different parts, separated by an arrow `->`.
The part on the left of the reaction(decay) defines the reactants(head)
and the part on the right the products.
All names must be separated by a whitespace.
The only syntax difference between a reaction and a decay is the presence
of more than one element in the left side of the arrow operator `->`.

It is possible to specify several subsequent reaction/decays by enclosing
them between curly braces, in such a way that

.. code-block::

   A B -> {C -> D E} F

corresponds to the reactants `A` and `B` generating `C` and `F` where `C`
produces `D` and `E`.
Left curly braces `{` must be located either after an arrow `->` or after
a space, otherwise it is considered to be part of the previous name.
Examples of this syntax are

.. code-block::

   KS0 -> pi+ {pi- -> mu- nu_mu~}
   phi(1020) -> K+ K-

We could write the previous in a more compact form as

.. code-block::

   KS0->pi+ {pi-->mu- nu_mu~}
   phi(1020)->K+ K-

but the proceses become less readable.
