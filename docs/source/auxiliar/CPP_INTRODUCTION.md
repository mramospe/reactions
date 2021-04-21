This package provides functionalities to define and handle reactions and decays.
Elements involved in these processes can be customized given a user-defined class and data-base.
A bulitin implementation of the [PDG](https://pdg.lbl.gov) database of particles and nuclei
is included.

The Reactions package allows to define a work with trees of processes among different
elements.
These processes can be either a reaction, where one or more reactants generate a set
of products; or a decay, where a single element generates a set of products.
This package is written in `C++` as a header-only library, and has a `python`
interface.
The usage is very similar in both of them.

This package is distributed as a header-only library for `C++`  and pip-installable package in python.
There is no need to install any external software.

The only necessary thing to work in `C++` is to add `include` as the directory
to search for header files and use the `C++17` standard (or greater).

```cpp

#include "reactions/all.hpp"

int main() {

   auto r = reactions::make_reaction<reactions::string_element>("A B -> C D");

   std::cout << "Reaction number of reactants/products: " << r.reactants().size() << '/' << r.products().size() << std::endl;

   auto d = reactions::make_decay<reactions::pdg_element>("pi+ -> mu+ nu_mu");

   std::cout << "Decay head/number of products: " << r.head().ptr_as_element()->name() << '/' << r.products().size() << std::endl;

   return 0;
}
```

You just need to place the latter in a `test.cpp` script and compile it with

```bash
gcc -std=c++17 -Iinclude -o test.cpp
```
