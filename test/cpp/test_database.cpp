#include "test_utils.hpp"

#include "reactions/all.hpp"

using namespace reactions;

int main() {

  test::collector pdg_database_coll("pdg_database");
  pdg_database_coll.add_test_function(
      "test PDG database", []() -> test::errors {
        test::errors errors;

        try {

          if (&pdg_database::instance() != &pdg_database::instance())
            errors.push_back("The PDG database is not a singleton");

          auto &db = pdg_database::instance();

          if (db("pi+").pdg_id() != db(+211).pdg_id())
            errors.push_back("Accessors by string and PDG ID do not retrieve "
                             "the same elements");
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
