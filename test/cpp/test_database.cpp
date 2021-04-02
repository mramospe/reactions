#include "test_utils.hpp"

#include "reactions/all.hpp"

using namespace reactions;

int main() {

  test::collector pdg_database_coll("pdg_database");
  pdg_database_coll.add_test_function("test database", []() -> test::errors {
    test::errors errors;

    try {

      if (&pdg_database::instance() != &pdg_database::instance())
        errors.push_back("The PDG database is not a singleton");

      auto &db = pdg_database::instance();

      if (db("pi+").pdg_id() != db(+211).pdg_id())
        errors.push_back(
            "Accessors by string and PDG ID do not retrieve the same elements");
    }
    REACTIONS_TEST_UTILS_CATCH_EXCEPTIONS(errors);

    return errors;
  });
  pdg_database_coll.add_test_function("test PDG element", []() -> test::errors {
    test::errors errors;

    try {

      auto &db = pdg_database::instance();

      if (db("pi+").pdg_id() != db(+211).pdg_id())
        errors.push_back(
            "Accessors by string and PDG ID do not retrieve the same elements");

      pdg_element::base_type ta = {
          "a", 0, 0, reactions::missing, reactions::missing, false};
      pdg_element a = std::move(ta);

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
        check_latex_name("KS0", "K_{S}^{0}");
        check_latex_name("K+", "K^{+}");
        check_latex_name("pi+", "\\pi^{+}");
        check_latex_name("pi-", "\\pi^{-}");
        check_latex_name("Lambda", "\\Lambda");
        check_latex_name("eta'(958)", "\\eta^{'}(958)");
        check_latex_name("a_0(980)0", "a_{0}(980)^{0}");
        check_latex_name("f'_2(1525)", "f_{2}^{'}(1525)");
        check_latex_name("K_2(1430)*~0", "\\bar{K}_{2}(1430)^{*0}");
        check_latex_name("D_s2(2573)*+", "D_{s2}(2573)^{*+}");
        check_latex_name("Xi'_c+", "\\Xi_{c}^{'+}");
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
  pdg_database_coll.add_test_function("test cache", []() -> test::errors {
    test::errors errors;

    try {

      auto &db = pdg_database::instance();

      auto path = db.get_database_path();

      db.enable_cache();

      db.set_database_path(db.get_database_path());

      if (db("pi+").pdg_id() != db(+211).pdg_id())
        errors.push_back(
            "Accessors by string and PDG ID do not retrieve the same elements");
    }
    REACTIONS_TEST_UTILS_CATCH_EXCEPTIONS(errors);

    return errors;
  });
  pdg_database_coll.add_test_function(
      "test user elements", []() -> test::errors {
        test::errors errors;

        auto &db = pdg_database::instance();

        try {

          db.register_element("Z'0", 99999999, 0,
                              reactions::fill{100.f, 10.f, 10.f},
                              reactions::missing, true);
          db.enable_cache();
          db.register_element("Z''0", 99999998, 0, reactions::missing,
                              reactions::missing, true);
        }
        REACTIONS_TEST_UTILS_CATCH_EXCEPTIONS(errors);

        auto toggle_cache_on_off = [&errors, &db]() -> void {
          try {
            db.register_element("custom", 99999998, 0, reactions::missing,
                                reactions::missing, true);
            errors.push_back("Should have thrown an error when registering an "
                             "element with the same PDG ID twice");
          } catch (...) {
          };

          try {
            db.register_element("Z''0", 99999997, 0, reactions::missing,
                                reactions::missing, true);
            errors.push_back("Should have thrown an error when registering an "
                             "element with the same name twice");
          } catch (...) {
          };

          try {
            db.register_element("Z0", 99999997, 0, reactions::missing,
                                reactions::missing, true);
            errors.push_back(
                "Should have thrown an error when registering an element with "
                "the same name as one in the database");
          } catch (...) {
          };

          try {
            db.register_element("custom", 1, 0, reactions::missing,
                                reactions::missing, true);
            errors.push_back(
                "Should have thrown an error when registering an element with "
                "the same PDG ID as one in the database");
          } catch (...) {
          };

          // get all the elements and access them
          for (auto e : db.all_elements()) {
            if (db(e.name()) != db(e.pdg_id())) {
              errors.push_back("Failed to load and read all elements");
              break;
            }
          }
        };

        // cache is already enabled
        toggle_cache_on_off();
        // disable cache and run again
        db.disable_cache();
        toggle_cache_on_off();

        return errors;
      });

  return !pdg_database_coll.run();
}
