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

  class internal_error : std::runtime_error {
  public:
    internal_error(const char *msg) : std::runtime_error{msg} {};

    const char *what() const noexcept { return std::runtime_error::what(); }
  };

  class missing_fields_error : std::runtime_error {
  public:
    missing_fields_error(const char *msg) : std::runtime_error{msg} {};

    const char *what() const noexcept { return std::runtime_error::what(); }
  };

  class syntax_error : std::runtime_error {
  public:
    syntax_error(const char *msg) : std::runtime_error{msg} {};

    const char *what() const noexcept { return std::runtime_error::what(); }
  };

  /// Exceptions that are handled internally
  namespace exceptions {

    class __syntax_error {

    public:
      __syntax_error(const char *msg, std::size_t rpos)
          : m_msg{msg}, m_rpos{rpos} {}

      const char *what() const noexcept { return m_msg; }

      syntax_error update(std::string const &str) {
        return {mark_error(str, m_msg, m_rpos).c_str()};
      }

    private:
      const char *m_msg;
      std::size_t m_rpos;
    };
  } // namespace exceptions

  class lookup_error : std::runtime_error {

  public:
    lookup_error(const char *msg) : std::runtime_error{msg} {};

    const char *what() const noexcept { return std::runtime_error::what(); }
  };

  class database_error : std::runtime_error {

  public:
    database_error(const char *msg) : std::runtime_error{msg} {};

    const char *what() const noexcept { return std::runtime_error::what(); }
  };
} // namespace reactions
