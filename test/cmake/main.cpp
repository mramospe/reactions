#include "reactions/all.hpp"

#include <cassert>

int main() {
  auto r = reactions::make_reaction<reactions::string_element>("A B -> C D");
  assert(r.reactants().size() == 2);
  assert(r.products().size() == 2);
  return 0;
}
