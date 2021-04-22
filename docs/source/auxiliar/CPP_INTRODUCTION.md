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

However, in order to correctly set the paths to the databases it is needed to execute build the
package.
This can be easily done by calling
```shell
mkdir build
cd build
cmake ../
make install
```
The installed headers will be placed under `build/include`.
Afterwards, the package can be easily used by simply including the previous
path in your set of directories to search for headers.
Consider the following code snippet:

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

To compile it, you just need to place the latter in a `test.cpp` script and compile it with

```shell
gcc -std=c++17 -I build/include -o test.cpp
```

This package uses the `C++` standard greater or equal to 17, so remember to specify the
`-std=c++17` flag to your preferred compiler.
