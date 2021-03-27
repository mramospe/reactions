#ifndef REACTIONS_DATABASE_HPP
#define REACTIONS_DATABASE_HPP

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <iostream>

namespace reactions::database {

  /// Optional for float values
  using float_opt = std::optional<float>;

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

      auto const size = s.size();

      std::size_t begin = 0;
      for (; begin != size && s[begin] == ' '; ++begin)
        ;

      std::size_t end = size - 1;
      for (; end != 0 && s[end] == ' '; --end)
        ;

      out = s.substr(begin, end + 1 - begin);

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
    if (std::all_of(s.cbegin() + Begin, s.cbegin() + End,
                    [](char c) { return c == ' '; }))
      return empty;

    return detail::string_to_type(out, s.substr(Begin, End - Begin));
  }
} // namespace reactions::database

#endif // REACTIONS_DATABASE_HPP
