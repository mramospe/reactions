#pragma once
#include <optional>
#include <sstream>
#include <string>

#include "reactions/database.hpp"

namespace reactions::python {

  // Forward declaration
  template <class T> std::string to_string(T const &t);

  /// Type to convert fields to strings
  template <class T, class Enable = void> struct to_string_t {
    /// Convert the value to a string
    std::string operator()(T const &t) const { return std::to_string(t); }
  };

  /// \copydoc to_string_t
  template <class T>
  struct to_string_t<T, std::enable_if_t<std::is_floating_point_v<T>>> {
    /// \copydoc to_string_t::operator()
    std::string operator()(T const &t) const {

      std::stringstream ss;

      if (t >= 10000 || t < 0.01)
        ss << std::scientific << t;
      else
        ss << t;

      return ss.str();
    }
  };

  /// \copydoc to_string_t
  template <> struct to_string_t<std::string> {
    /// \copydoc to_string_t::operator()
    std::string operator()(std::string const &s) const { return '"' + s + '"'; }
  };

  /// \copydoc to_string_t
  template <> struct to_string_t<bool> {
    /// \copydoc to_string_t::operator()
    std::string operator()(bool const &b) const { return b ? "True" : "False"; }
  };

  /// \copydoc to_string_t
  template <class T>
  struct to_string_t<reactions::fields::value_and_errors<T>> {
    /// \copydoc to_string_t::operator()
    std::string
    operator()(reactions::fields::value_and_errors<T> const &vae) const {
      return std::string{"(value="} + to_string(vae.value) +
             std::string{", error_lower="} + to_string(vae.error_lower) +
             std::string{", error_upper="} + to_string(vae.error_upper) + ')';
    }
  };

  /// Function to convert a value to a string
  template <class T> std::string to_string(T const &t) {
    static constexpr to_string_t<std::decay_t<T>> converter;
    return converter(t);
  }
} // namespace reactions::python
