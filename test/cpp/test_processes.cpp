#include "test_utils.hpp"

#include "reactions/all.hpp"

using namespace reactions;

template <class Element> struct reaction_tester {
  const char *reaction;
  test::errors operator()() const {
    test::errors errors;
    try {
      auto d = make_reaction<Element>(reaction);

      if (d.reactants().size() == 0)
        errors.push_back("Reactants are not set");
      if (d.products().size() == 0)
        errors.push_back("Products are not set");
    }
    REACTIONS_TEST_UTILS_CATCH_EXCEPTIONS(errors);

    return errors;
  }
};

template <class Element> struct decay_tester {
  const char *decay;
  test::errors operator()() {
    test::errors errors;
    try {
      auto d = make_decay<Element>(decay);

      if (d.products().size() == 0)
        errors.push_back("Products are not set");
    }
    REACTIONS_TEST_UTILS_CATCH_EXCEPTIONS(errors);

    return errors;
  }
};

int main() {

  // Test the reaction class
  test::collector reaction_coll("reaction tests");
  REACTIONS_TEST_UTILS_ADD_TEST(reaction_coll,
                                reaction_tester<string_element>{"A -> B C"});
  REACTIONS_TEST_UTILS_ADD_TEST(reaction_coll,
                                reaction_tester<string_element>{"A B -> C D"});
  REACTIONS_TEST_UTILS_ADD_TEST(
      reaction_coll,
      reaction_tester<string_element>{"A B -> {C -> D E} {F G -> H I}"});
  REACTIONS_TEST_UTILS_ADD_TEST(
      reaction_coll, reaction_tester<string_element>{"pi+ -> mu+ nu(mu)"});
  REACTIONS_TEST_UTILS_ADD_TEST(
      reaction_coll,
      reaction_tester<string_element>{"pi+->mu+ nu(mu)"}); // spaces
  REACTIONS_TEST_UTILS_ADD_TEST(
      reaction_coll, reaction_tester<string_element>{
                         "K(S)0 -> {pi+ -> mu+ nu(mu)} mu- phi(1020)"});
  REACTIONS_TEST_UTILS_ADD_TEST(
      reaction_coll, reaction_tester<string_element>{
                         "K(S)0->{ pi+->mu+ nu(mu) } mu- phi(1020)"}); // spaces

  REACTIONS_TEST_UTILS_ADD_TEST(
      reaction_coll, reaction_tester<pdg_element>{"pi+ -> mu+ nu(mu)"});

  // Test the decay class
  test::collector decay_coll("decay tests");
  REACTIONS_TEST_UTILS_ADD_TEST(
      decay_coll, decay_tester<string_element>{"pi+ -> mu+ nu(mu)"});
  REACTIONS_TEST_UTILS_ADD_TEST(
      decay_coll, decay_tester<string_element>{"pi+->mu+ nu(mu)"}); // spaces
  REACTIONS_TEST_UTILS_ADD_TEST(
      decay_coll, decay_tester<string_element>{
                      "K(S)0 -> {pi+ -> mu+ nu(mu)} mu- phi(1020)"});
  REACTIONS_TEST_UTILS_ADD_TEST(
      decay_coll, decay_tester<string_element>{
                      "K(S)0->{ pi+->mu+ nu(mu) } mu- phi(1020)"}); // spaces

  REACTIONS_TEST_UTILS_ADD_TEST(decay_coll,
                                decay_tester<pdg_element>{"pi+ -> mu+ nu(mu)"});

  auto reaction_status = !reaction_coll.run();
  auto decay_status = !decay_coll.run();

  return reaction_status || decay_status;
}
