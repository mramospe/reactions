/*! \file
  \brief Define exceptions that can be thrown when running the functions of the
  package.
 */
#ifndef REACTIONS_EXCEPTIONS_HPP
#define REACTIONS_EXCEPTIONS_HPP

#include <cstring>
#include <exception>
#include <stdexcept>
#include <string>

/*! \brief Exceptions and errors
 */
namespace reactions::exceptions {

  const char *mark_error(std::string const &str, const char *msg,
                         std::size_t rpos) {

    auto nb = strlen(msg) + 2;
    auto ps = strlen(msg) - rpos;

    std::string new_msg;
    new_msg.reserve(2 * nb + str.size() + ps + 2);
    new_msg.append(msg);
    new_msg.append(":\n"); // size 2
    new_msg.append(str);
    new_msg.push_back('\n'); // size 1
    new_msg.append(nb + ps, ' ');
    new_msg.push_back('^'); // size 1

    return new_msg.c_str();
  }

  class internal_error : std::runtime_error {
  public:
    internal_error(const char *msg) : std::runtime_error{msg} {};

    const char *what() const noexcept { return std::runtime_error::what(); }
  };

  class syntax_error : std::runtime_error {
  public:
    syntax_error(const char *msg) : std::runtime_error{msg} {};

    const char *what() const noexcept { return std::runtime_error::what(); }
  };

  class __syntax_error {

  public:
    __syntax_error(const char *msg, std::size_t rpos)
        : m_msg{msg}, m_rpos{rpos} {}

    const char *what() const noexcept { return m_msg; }

    syntax_error update(std::string const &str) {
      return {mark_error(str, m_msg, m_rpos)};
    }

  private:
    const char *m_msg;
    std::size_t m_rpos;
  };

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
} // namespace reactions::exceptions

#endif // REACTIONS_EXCEPTIONS_HPP
