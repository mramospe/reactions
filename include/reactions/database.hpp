/*! \file
 * \brief Common operations on databases
 */
#pragma once

#include <algorithm>
#include <cmath>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>

namespace reactions {

  /*! \brief Small structure to define fields without explicitely knowing its
   * type
   *
   * This class serves as an interface, forwarding its contents. Any class which
   * represents a field must define a constructor from this type.
   */
  template <class... Args> struct fill : std::tuple<Args...> {
    /// Build the class from the contents
    using base_type = std::tuple<Args...>;
    constexpr fill(Args &&... args) : base_type{std::forward<Args>(args)...} {}
  };

  /// Static object that defines a missing field within an object
  static constexpr auto missing = std::nullopt;
} // namespace reactions

/// Utilities to handle database objects and their elements
namespace reactions::database {

  /// Define a \ref std::tuple with the types of the fields provided
  template <class Tuple> struct underlying_types;

  /// \copydoc underlying_types
  template <class... F> struct underlying_types<std::tuple<F...>> {
    using type = std::tuple<typename F::value_type...>;
  };

  /// Check if a type represents an optional
  template <class> struct is_optional : std::false_type {};

  /// Check if a type represents an optional
  template <class T> struct is_optional<std::optional<T>> : std::true_type {};

  /// Alias to check if a type represents an optional
  template <class T> constexpr auto is_optional_v = is_optional<T>::value;

  /// Contain the information whether a field is optional or not
  template <class F, class Enable = void>
  struct is_optional_field : std::false_type {};

  /// \copydoc is_optional_field
  template <class F>
  struct is_optional_field<
      F, std::enable_if_t<is_optional_v<typename F::value_type>>>
      : std::true_type {};

  /// \copydoc is_optional_field
  template <class F>
  constexpr auto is_optional_field_v = is_optional_field<F>::value;

  /// If the input type is an optional, get the underying type
  template <class T> struct remove_optional { using type = T; };

  /// \copydoc remove_optional
  template <class T> struct remove_optional<std::optional<T>> {
    using type = T;
  };

  /// \copydoc remove_optional
  template <class T>
  using remove_optional_t = typename remove_optional<T>::type;

  /// Field for a value
  struct value {};
  /// Field for an error
  struct error {};
  /// Field for a lower error
  struct error_lower {};
  /// Field for an lower error
  struct error_upper {};

  /// Define a range with minimum and maximum indices
  template <std::size_t Min, std::size_t Max> struct range {
    static constexpr auto min = Min, max = Max;
  };

  /// Define a collection of ranges (a single variable with subvariables)
  template <class... R> using range_collection = std::tuple<R...>;

  /// Get the overall range for a range with/without subranges
  template <class R> struct overall_range {
    using type = range<R::min, R::max>;
  };

  /// Overall range of a variable (that can be a composite)
  template <class R0, class... R, class Rn>
  struct overall_range<range_collection<R0, R..., Rn>> {
    using type = range<R0::min, Rn::max>;
  };

  /// Overall range of a variable (that can be a composite)
  template <class R> using overall_range_t = typename overall_range<R>::type;

  /// Simple structure composed by a value and the lower and upper errors
  template <class T, class Enable = void> struct value_and_errors;

  /// \copydoc value_and_errors
  template <class T>
  struct value_and_errors<T,
                          std::enable_if_t<std::is_floating_point_v<T>, void>> {

    using value_type = T;

    /// Empty constructor
    value_and_errors() = default;
    /// Build the class with forwarded arguments
    template <class Value, class ErrorLower, class ErrorUpper>
    value_and_errors(Value &&value_, ErrorLower &&error_lower_,
                     ErrorUpper &&error_upper_)
        : value(value_), error_lower(error_lower_), error_upper(error_upper_) {}
    /// Build the class from a field constant expression
    template <class Value, class ErrorLower, class ErrorUpper>
    value_and_errors(fill<Value, ErrorLower, ErrorUpper> &&f)
        : value(std::get<0>(f)), error_lower(std::get<1>(f)),
          error_upper(std::get<2>(f)) {}

    /// Value
    value_type value;
    /// Lower error
    value_type error_lower;
    /// Upper error
    value_type error_upper;
    /// Calculate the squared error from the lower and upper errors
    value_type error_squared() const {
      return error_lower * error_lower + error_upper * error_upper;
    };
    /// Calculate the error from the lower and upper errors
    value_type error() const { return std::sqrt(error_squared()); }
  };

  /// Return the underlying value of an object (the same if the object is \ref
  /// std::optional)
  template <class T> constexpr auto const &access_value(T const &c) {
    return c;
  }

  /// \copydoc access_value
  template <class T>
  constexpr auto const &access_value(std::optional<T> const &opt) {
    return opt.value();
  }

  /// Type defining an accessor to a value/error field
  template <class F> struct get_t;

  /// Accessor to the value
  template <> struct get_t<value> {
    template <class T> constexpr auto const &operator()(T const &t) const {
      return access_value(t).value;
    }
  };

  /// Accessor to the error
  template <> struct get_t<error> {
    template <class T> constexpr auto const &operator()(T const &t) const {
      return access_value(t).error;
    }
  };

  /// Accessor to the lower error
  template <> struct get_t<error_lower> {
    template <class T> constexpr auto const &operator()(T const &t) const {
      return access_value(t).error_lower;
    }
  };

  /// Accessor to the upper error
  template <> struct get_t<error_upper> {
    template <class T> constexpr auto const &operator()(T const &t) const {
      return access_value(t).error_upper;
    }
  };

  /// Accessor to a value/error field
  template <class F> static constexpr get_t<F> get;

  /// Optional for \ref float type
  using float_opt = std::optional<float>;
  /// Optional for \ref double type
  using double_opt = std::optional<double>;
  /// Optional \ref value_and_errors for single-precision floating-point type
  using ve_float_opt = std::optional<value_and_errors<float>>;
  /// Optional \ref value_and_errors for double-precision floating-point type
  using ve_double_opt = std::optional<value_and_errors<double>>;

  /// Status code of a conversion to an arithmetic or \ref std::optional type
  enum conversion_status : int {
    success, ///< the conversion succeeded
    empty,   ///< the object is missing
    failed   ///<  the conversion failed
  };

  /// Internal utilitites for the \ref reactions::database namespace
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
  template <class Range, class T>
  conversion_status read_field(T &out, std::string const &s) {
    // std::from_chars is not correctly implemented in GCC, and
    // we can not work with std::string_view objects
    auto b = s.find_first_not_of(' ', Range::min);

    if (b >= Range::max)
      return empty;

    auto e = s.find_last_not_of(' ', Range::max) + 1;

    return detail::string_to_type(out, s.substr(b, e - b));
  }

  /// Read a field composed by value and errors in a line from a file
  template <class Ranges, class T>
  conversion_status read_field(value_and_errors<T> &out, std::string const &s) {
    // std::from_chars is not correctly implemented in GCC, and
    // we can not work with std::string_view objects
    static_assert(std::tuple_size_v<Ranges> == 3);

    auto b = s.find_first_not_of(' ', std::tuple_element_t<0, Ranges>::min);

    if (b >= std::tuple_element_t<2, Ranges>::max)
      return empty;

    auto value_sc = detail::string_to_type(
        out.value, s.substr(std::tuple_element_t<0, Ranges>::min,
                            std::tuple_element_t<0, Ranges>::max));
    auto error_lower_sc = detail::string_to_type(
        out.error_lower, s.substr(std::tuple_element_t<1, Ranges>::min,
                                  std::tuple_element_t<1, Ranges>::max));
    auto error_upper_sc = detail::string_to_type(
        out.error_upper, s.substr(std::tuple_element_t<2, Ranges>::min,
                                  std::tuple_element_t<2, Ranges>::max));

    if (value_sc == empty || error_lower_sc == empty || error_upper_sc == empty)
      return failed; // either all are defined or none
    else if (value_sc == failed || error_lower_sc == failed ||
             error_upper_sc == failed)
      return failed;
    else
      return success;
  }
} // namespace reactions::database
