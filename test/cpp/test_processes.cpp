#include "test_utils.hpp"

#include "reactions/all.hpp"

using namespace reactions;

template <element_kind E> struct reaction_tester {
  const char *reaction;
  test::errors operator()() const {
    test::errors errors;
    try {
      auto d = make_reaction<E>(reaction);

      if (d.reactants().size() == 0)
        errors.push_back("Reactants are not set");
      if (d.products().size() == 0)
        errors.push_back("Products are not set");
    }
    REACTIONS_TEST_UTILS_CATCH_EXCEPTIONS(errors);

    return errors;
  }
};

template <element_kind E> struct decay_tester {
  const char *decay;
  test::errors operator()() {
    test::errors errors;
    try {
      auto d = make_decay<E>(decay);

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
                                reaction_tester<string>{"A -> B C"});
  REACTIONS_TEST_UTILS_ADD_TEST(reaction_coll,
                                reaction_tester<string>{"A B -> C D"});
  REACTIONS_TEST_UTILS_ADD_TEST(
      reaction_coll, reaction_tester<string>{"A B -> {C -> D E} {F G -> H I}"});
  REACTIONS_TEST_UTILS_ADD_TEST(reaction_coll,
                                reaction_tester<string>{"pi+ -> mu+ nu_mu"});
  REACTIONS_TEST_UTILS_ADD_TEST(
      reaction_coll, reaction_tester<string>{"pi+->mu+ nu_mu"}); // spaces
  REACTIONS_TEST_UTILS_ADD_TEST(
      reaction_coll,
      reaction_tester<string>{"KS0 -> {pi+ -> mu+ nu_mu} mu- phi(1020)"});
  REACTIONS_TEST_UTILS_ADD_TEST(
      reaction_coll, reaction_tester<string>{
                         "KS0->{ pi+->mu+ nu_mu } mu- phi(1020)"}); // spaces

  REACTIONS_TEST_UTILS_ADD_TEST(reaction_coll,
                                reaction_tester<pdg>{"pi+ -> mu+ nu_mu"});

  // Test the decay class
  test::collector decay_coll("decay tests");
  REACTIONS_TEST_UTILS_ADD_TEST(decay_coll,
                                decay_tester<string>{"pi+ -> mu+ nu_mu"});
  REACTIONS_TEST_UTILS_ADD_TEST(
      decay_coll, decay_tester<string>{"pi+->mu+ nu_mu"}); // spaces
  REACTIONS_TEST_UTILS_ADD_TEST(
      decay_coll,
      decay_tester<string>{"KS0 -> {pi+ -> mu+ nu_mu} mu- phi(1020)"});
  REACTIONS_TEST_UTILS_ADD_TEST(
      decay_coll,
      decay_tester<string>{"KS0->{ pi+->mu+ nu_mu } mu- phi(1020)"}); // spaces

  REACTIONS_TEST_UTILS_ADD_TEST(decay_coll,
                                decay_tester<pdg>{"pi+ -> mu+ nu_mu"});

  bool const reaction_code = reaction_coll.run();
  bool const decay_code = decay_coll.run();

  if (reaction_code && decay_code)
    return 0;
  else
    return 1;
}
