#include "test_utils.hpp"

#include "reactions/all.hpp"

using namespace reactions;

int main() {

  /*
    Tests for the PDG database
  */
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
  test::collector pdg_system_of_units_coll("pdg_system_of_units");
  pdg_system_of_units_coll.add_test_function(
      "test units", []() -> test::errors {
        test::errors errors;
        auto &db = reactions::pdg_database::instance();
        auto &sou = reactions::pdg_system_of_units::instance();

        sou.units<reactions::energy_units>(); // simply access it

        auto z0_mass_gev = db("Z0").mass();

        sou.set_energy_units(reactions::MeV);

        auto z0_pos = db("Z0");
        if (z0_pos.mass() < 100.)
          errors.push_back("Unable to change units");

        auto diff = z0_mass_gev - z0_pos.mass() * 1e-3;
        if ((diff > 0 ? diff : -diff) > 1e-12)
          errors.push_back("Wrong unit scale factors");

        return errors;
      });

  /*
    Tests for the NuBase database
  */
  test::collector nubase_database_coll("nubase_database");
  nubase_database_coll.add_test_function(
      "test NuBase database", []() -> test::errors {
        test::errors errors;

        try {

          if (&nubase_database::instance() != &nubase_database::instance())
            errors.push_back("The NuBase database is not a singleton");

          auto &db = nubase_database::instance();

          if (db("1H").nubase_id() != db(1001000).nubase_id())
            errors.push_back(
                "Accessors by string and NuBase ID do not retrieve "
                "the same elements");
        }
        REACTIONS_TEST_UTILS_CATCH_EXCEPTIONS(errors);

        return errors;
      });
  nubase_database_coll.add_test_function("test cache", []() -> test::errors {
    test::errors errors;

    try {

      auto &db = nubase_database::instance();

      auto path = db.get_database_path();

      db.enable_cache();

      db.set_database_path(db.get_database_path());

      if (db("1H").nubase_id() != db(1001000).nubase_id())
        errors.push_back("Accessors by string and NuBase ID do not retrieve "
                         "the same elements");
    }
    REACTIONS_TEST_UTILS_CATCH_EXCEPTIONS(errors);

    return errors;
  });
  nubase_database_coll.add_test_function(
      "test user elements", []() -> test::errors {
        test::errors errors;

        auto &db = nubase_database::instance();

        try {

          db.register_element("998Un", 999999998, 999, 998,
                              reactions::fill{100.f, 10.f, false}, false,
                              reactions::missing, true);
          db.enable_cache();
          db.register_element("999Un", 999999999, 999, 999, reactions::missing,
                              false, reactions::missing, true);
        }
        REACTIONS_TEST_UTILS_CATCH_EXCEPTIONS(errors);

        auto toggle_cache_on_off = [&errors, &db]() -> void {
          try {
            db.register_element("999custom", 999999998, 999, 999,
                                reactions::missing, true, reactions::missing,
                                true);
            errors.push_back("Should have thrown an error when registering an "
                             "element with the same NuBase ID twice");
          } catch (...) {
          };

          try {
            db.register_element("999Un", 999999997, 999, 999,
                                reactions::missing, true, reactions::missing,
                                true);
            errors.push_back("Should have thrown an error when registering an "
                             "element with the same name twice");
          } catch (...) {
          };

          try {
            db.register_element("1H", 999999997, 1, 1, reactions::missing, true,
                                reactions::missing, true);
            errors.push_back(
                "Should have thrown an error when registering an element with "
                "the same name as one in the database");
          } catch (...) {
          };

          try {
            db.register_element("999custom", 999999999, 999, 999,
                                reactions::missing, true, reactions::missing,
                                true);
            errors.push_back(
                "Should have thrown an error when registering an element with "
                "the same NuBase ID as one in the database");
          } catch (...) {
          };

          // get all the elements and access them
          for (auto e : db.all_elements()) {
            if (db(e.name()) != db(e.nubase_id())) {
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
  test::collector nubase_system_of_units_coll("nubase_system_of_units");
  nubase_system_of_units_coll.add_test_function(
      "test units", []() -> test::errors {
        test::errors errors;
        auto &db = reactions::nubase_database::instance();
        auto &sou = reactions::nubase_system_of_units::instance();

        sou.units<reactions::energy_units>(); // simply access it
        sou.units<reactions::time_units>();   // simply access it

        // test energy units
        auto proton_mass_excess_kev = db("1H").mass_excess();

        sou.set_energy_units(reactions::eV);

        auto proton = db("1H");
        if (proton.mass_excess() < 7.5e3)
          errors.push_back("Unable to change units");

        auto diff_me = proton_mass_excess_kev - proton.mass_excess() * 1e-3;
        if ((diff_me > 0 ? diff_me : -diff_me) > 1e-12)
          errors.push_back("Wrong energy units scale factors");

        // test time units
        auto neutron_half_life_s = db("1n").half_life();

        sou.set_time_units(reactions::ms);

        auto neutron = db("1n");
        if (neutron.half_life() < 700.)
          errors.push_back("Unable to change units");

        auto diff_hl = neutron_half_life_s - neutron.half_life() * 1e-3;
        if ((diff_hl > 0 ? diff_hl : -diff_hl) > 1e-12)
          errors.push_back("Wrong time units scale factors");

        return errors;
      });

  /*
    Run the tests
   */
  auto pdg_status = !pdg_database_coll.run();
  auto pdg_sou_status = !pdg_system_of_units_coll.run();
  auto nubase_status = !nubase_database_coll.run();
  auto nubase_sou_status = !nubase_system_of_units_coll.run();

  return pdg_status || pdg_sou_status || nubase_status || nubase_sou_status;
}
