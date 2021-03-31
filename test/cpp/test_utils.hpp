#ifndef REACTIONS_TEST_UTILS_HPP
#define REACTIONS_TEST_UTILS_HPP

#include "reactions/exceptions.hpp"

#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace reactions::test {

  using errors = std::vector<std::string>;

  /* \brief Object handling a collection of test functions
   *
   */
  class collector {

  public:
    /// Functions must return a list with the error messages
    using function_type = std::function<errors(void)>;

    /// Constructor from a name
    collector(std::string const &name) : m_name{name} {}
    /// Default destructor
    ~collector() = default;

    // Invalidate other constructors
    collector(collector const &) = delete;
    collector(collector &&) = delete;
    collector &operator=(collector const &) = delete;

    /// Add a new test function
    void add_test_function(std::string name, function_type const &function) {
      m_functions.push_back(std::make_pair(std::move(name), function));
    }

    /// Run the stored tests and return the status
    bool run() const {

      std::map<std::size_t, std::vector<std::string>> error_map;

      for (auto i = 0u; i < m_functions.size(); ++i) {
        auto v = m_functions[i].second();
        if (v.size() != 0)
          error_map[i] = std::move(v);
      }

      std::cout << "Results for collector \"" << m_name << '\"' << std::endl;
      for (auto i = 0u; i < m_functions.size(); ++i)
        std::cout << "- "
                  << (error_map.find(i) == error_map.cend() ? "(success) "
                                                            : "(failed) ")
                  << m_functions[i].first << std::endl;

      if (error_map.size() != 0) {
        std::cerr << "summary of errors:" << std::endl;
        for (auto const &p : error_map) {
          std::cerr << "* " << m_functions[p.first].first << ':' << std::endl;
          for (auto const &what : p.second)
            std::cerr << " - " << what << std::endl;
        }
        return false;
      }

      return true;
    }

  protected:
    /// Names and functions
    std::vector<std::pair<std::string, function_type>> m_functions;
    /// Name of the collector
    std::string m_name;
  };
} // namespace reactions::test

#define REACTIONS_TEST_UTILS_ADD_TEST(collector, function)                     \
  collector.add_test_function(#function, function);

#define REACTIONS_TEST_UTILS_CATCH_EXCEPTIONS(errors)                          \
  catch (syntax_error & e) {                                                   \
    errors.push_back(e.what());                                                \
  }                                                                            \
  catch (database_error & e) {                                                 \
    errors.push_back(e.what());                                                \
  }                                                                            \
  catch (lookup_error & e) {                                                   \
    errors.push_back(e.what());                                                \
  }                                                                            \
  catch (...) {                                                                \
    errors.push_back("Unknown error detected");                                \
  }

#endif // REACTIONS_TEST_UTILS_HPP
