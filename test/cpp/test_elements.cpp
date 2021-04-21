#include "test_utils.hpp"

#include "reactions/all.hpp"

using namespace reactions;

int main() {

  test::collector string_element_coll("string_element");
  string_element_coll.add_test_function("test string element",
                                        []() -> test::errors {
                                          test::errors errors;
                                          string_element el{"A"};
                                          el = "B";
                                          return errors;
                                        });

  test::collector pdg_element_coll("pdg_element");
  pdg_element_coll.add_test_function("test PDG element", []() -> test::errors {
    test::errors errors;

    try {

      auto &db = pdg_database::instance();

      if (db("pi+").pdg_id() != db(+211).pdg_id())
        errors.push_back(
            "Accessors by string and PDG ID do not retrieve the same elements");

      pdg_element::base_type ta = {
          "a", 0, 0, reactions::missing, reactions::missing, false};
      pdg_element a = std::move(ta);

      if ( !a.has<pdg::name>() || !a.has<pdg::pdg_id>() || a.has<pdg::mass>() )
	errors.push_back("Errors checking the presence of values");

      auto const& name = a.get<pdg::name>();

      if ( name != a.name() )
	errors.push_back("Name accessed with the \"name\" member function and with \"get<pdg::name>\" do not coincide");

      a.get<pdg::mass>(); // simply try to get it

      pdg_element::base_type tb = {
          "b", 0, 0, reactions::missing, reactions::missing, false};
      pdg_element b{std::move(tb)};

      pdg_element c1 = {"c1", 0, 0, reactions::missing, reactions::missing,
                        false};
      pdg_element c2 = {
          "c2", 0, 0, reactions::fill{0., 0., 0.}, reactions::missing, false};
      pdg_element c3 = {
          "c3", 0, 0, reactions::fill{0., 0., 0.}, reactions::fill{0., 0., 0.},
          false};

      if (c1.has_mass() || c1.has_width() || !c2.has_mass() || c2.has_width() ||
          !c3.has_mass() || !c3.has_width())
        errors.push_back("Problems found building custom PDG elements");

      auto check_latex_name = [&db, &errors](std::string const &name,
                                             std::string const &reference) {
        auto const ln = db(name).latex_name();
        if (ln != reference)
          errors.push_back("Wrong LaTeX name for particle \"" + name +
                           "\": \"" + ln + "\" (reference: \"" + reference +
                           "\")");
      };

      try {
        check_latex_name("K(S)0", "K_{S}^{0}");
        check_latex_name("K+", "K^{+}");
        check_latex_name("pi+", "\\pi^{+}");
        check_latex_name("pi-", "\\pi^{-}");
        check_latex_name("Lambda", "\\Lambda");
        check_latex_name("eta'(958)", "\\eta^{'}(958)");
        check_latex_name("a(0)(980)0", "a_{0}(980)^{0}");
        check_latex_name("f(2)'(1525)", "f_{2}^{'}(1525)");
        check_latex_name("K(2)*(1430)~0", "\\bar{K}_{2}^{*}(1430)^{0}");
        check_latex_name("D(s2)*(2573)+", "D_{s2}^{*}(2573)^{+}");
        check_latex_name("Xi(c)'+", "\\Xi_{c}^{'+}");
        check_latex_name("Delta(1950)~-", "\\bar{\\Delta}(1950)^{-}");
      } catch (reactions::internal_error &) {
        errors.push_back("Internal error detected processing LaTeX names");
      }

      try {
        for (auto e : reactions::pdg_database::instance().all_elements())
          e.latex_name();
      } catch (reactions::internal_error &) {
        errors.push_back("Unable to compute LaTeX names for some elements");
      }
    }
    REACTIONS_TEST_UTILS_CATCH_EXCEPTIONS(errors);

    return errors;
  });

  auto str_status = !string_element_coll.run();
  auto pdg_status = !pdg_element_coll.run();

  return str_status || pdg_status;
}
