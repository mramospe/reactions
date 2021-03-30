#include "test_utils.hpp"

#include "reactions/all.hpp"

using namespace reactions;

int main() {

  test::collector pdg_database_coll("pdg_database");
  pdg_database_coll.add_test_function("test database", []() -> test::errors {
    test::errors errors;

    try {

      if (&database_pdg::database::instance() !=
          &database_pdg::database::instance())
        errors.push_back("The PDG database is not a singleton");

      auto &db = database_pdg::database::instance();

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

      auto &db = database_pdg::database::instance();

      if (db("pi+").pdg_id() != db(+211).pdg_id())
        errors.push_back(
            "Accessors by string and PDG ID do not retrieve the same elements");

      database_pdg::element c1 = {
          "c1", 0, 0, reactions::missing, reactions::missing, false};
      database_pdg::element c2 = {
          "c2", 0, 0, reactions::fill{0., 0., 0.}, reactions::missing, false};
      database_pdg::element c3 = {
          "c3", 0, 0, reactions::fill{0., 0., 0.}, reactions::fill{0., 0., 0.},
          false};

      if (c1.has_mass() || c1.has_width() || !c2.has_mass() || c2.has_width() ||
          !c3.has_mass() || !c3.has_width())
        errors.push_back("Problems found building custom PDG elements");
    }
    REACTIONS_TEST_UTILS_CATCH_EXCEPTIONS(errors);

    return errors;
  });
  pdg_database_coll.add_test_function("test cache", []() -> test::errors {
    test::errors errors;

    try {

      auto &db = database_pdg::database::instance();

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

  return !pdg_database_coll.run();
}
