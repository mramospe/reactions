#ifndef REACTIONS_DATABASE_HPP
#define REACTIONS_DATABASE_HPP

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace reactions::database {

  /// Aliases for optional values
  using float_opt = std::optional<float>;
  using double_opt = std::optional<double>;

  /// Check if a type represents an optional
  template <class> struct is_optional : std::false_type {};

  /// Check if a type represents an optional
  template <class T> struct is_optional<std::optional<T>> : std::true_type {};

  /// Alias to check if a type represents an optional
  template <class T> constexpr auto is_optional_v = is_optional<T>::value;

  /// Status code of a conversion to an arithmetic type
  enum conversion_status : int { success, empty, failed };

  namespace detail {

    /// Convert a range of characters to an integral
    conversion_status string_to_type(int &out, std::string const &s) {

      if (s.empty())
        return empty;

      try {
        out = std::stoi(s);
      } catch (std::invalid_argument &e) {
        return failed;
      }

      return success;
    }

    /// Convert a range of characters to a floating point
    conversion_status string_to_type(float &out, std::string const &s) {

      if (s.empty())
        return empty;

      try {
        out = std::stof(s);
      } catch (std::invalid_argument &e) {
        return failed;
      }

      return success;
    }

    /// Convert a range of characters to a floating point (double precision)
    conversion_status string_to_type(double &out, std::string const &s) {

      if (s.empty())
        return empty;

      try {
        out = std::stod(s);
      } catch (std::invalid_argument &e) {
        return failed;
      }

      return success;
    }

    /// Convert a range of characters to a boolean
    conversion_status string_to_type(bool &out, std::string const &s) {

      if (s.empty())
        return empty;

      try {
        out = std::stoi(s);
      } catch (std::invalid_argument &e) {
        return failed;
      }

      return success;
    }

    /// Convert a range of characters to string
    conversion_status string_to_type(std::string &out, std::string const &s) {

      out = s;

      if (out.size() == 0)
        return empty;
      else
        return success;
    }
  } // namespace detail

  /// Read a field in a line from a file
  template <std::size_t Begin, std::size_t End, class T>
  conversion_status read_field(T &out, std::string const &s) {
    // std::from_chars is not correctly implemented in GCC, and
    // we can not work with std::string_view objects
    auto b = s.find_first_not_of(' ', Begin);

    if (b >= End)
      return empty;

    auto e = s.find_last_not_of(' ', End) + 1;

    return detail::string_to_type(out, s.substr(b, e - b));
  }
} // namespace reactions::database

#endif // REACTIONS_DATABASE_HPP
