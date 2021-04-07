/*! \file
  \brief Exceptions that can be thrown when running the functions of the
  package.
 */
#pragma once

#include <cstring>
#include <exception>
#include <stdexcept>
#include <string>

namespace reactions {

  /// Utilitites to handle internal exceptions
  namespace exceptions::detail {
    /*! \brief Format an error message referring to a syntax error.
     *
     * Indicates the place where the syntax error was detected.
     */
    std::string mark_error(std::string const &str, const char *msg,
                           std::size_t rpos) {

      auto nb = strlen(msg) + 3;
      auto ps = str.size() - rpos;
      std::string new_msg;
      new_msg.reserve(nb + str.size() + ps + 3);
      new_msg.append(msg);
      new_msg.append(":\n "); // size 3
      new_msg.append(str);
      new_msg.push_back('\n'); // size 1
      new_msg.append(ps + 1, ' ');
      new_msg.push_back('^'); // size 1
      return new_msg;
    }
  } // namespace exceptions::detail

  /// Raised when unexpected problems appear, which should be reported as bugs
  class internal_error : std::runtime_error {
  public:
    internal_error(const char *msg) : std::runtime_error{msg} {};

    const char *what() const noexcept { return std::runtime_error::what(); }
  };

  /*! \brief Raised when trying to access a field that is not defined
   *
   * This kind of error is raised when a member is of type \ref std::optional
   * and an attempt is made to access it when it has not been defined.
   */
  class missing_fields_error : std::runtime_error {
  public:
    missing_fields_error(const char *msg) : std::runtime_error{msg} {};

    const char *what() const noexcept { return std::runtime_error::what(); }
  };

  /// Raised when processing the syntax of reactions and decays
  class syntax_error : std::runtime_error {
  public:
    syntax_error(const char *msg) : std::runtime_error{msg} {};

    const char *what() const noexcept { return std::runtime_error::what(); }
  };

  /// Exceptions that are handled internally
  namespace exceptions {

    /// Syntax error with an unformatted message
    class __syntax_error {

    public:
      __syntax_error(const char *msg, std::size_t rpos)
          : m_msg{msg}, m_rpos{rpos} {}

      const char *what() const noexcept { return m_msg; }

      syntax_error update(std::string const &str) {
        return {detail::mark_error(str, m_msg, m_rpos).c_str()};
      }

    private:
      const char *m_msg;
      std::size_t m_rpos;
    };
  } // namespace exceptions

  /// Raised when an element is not found within a database
  class lookup_error : std::runtime_error {

  public:
    lookup_error(const char *msg) : std::runtime_error{msg} {};

    const char *what() const noexcept { return std::runtime_error::what(); }
  };

  /*!\brief Raised whenever a problem is detected in the database
   *
   * This error can refer to either the database file not being present,
   * problems with the cache, clash of elements defined by the user, ...
   */
  class database_error : std::runtime_error {

  public:
    database_error(const char *msg) : std::runtime_error{msg} {};

    const char *what() const noexcept { return std::runtime_error::what(); }
  };

  /// Raised whenever a problem is detected with an input value
  class value_error : std::runtime_error {

  public:
    value_error(const char *msg) : std::runtime_error{msg} {};

    const char *what() const noexcept { return std::runtime_error::what(); }
  };
} // namespace reactions
