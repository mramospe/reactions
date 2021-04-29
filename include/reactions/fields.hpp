/*! \file
 * \brief Common operations on fields
 */
#pragma once

#include <algorithm>
#include <cmath>
#include <optional>
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

/*!\brief Utilities to handle database objects and their elements.

  Databases return elements with *fields*. The template argument name *Field*
  designates an object with the following properties:

  - **title**: a string representing the name of the field.

  - **value_type**: a type representing the object that is stored (can be a
  \ref std::optional).

  - **units_reference_type**: a resolved type of the \ref fields::reference
  template, containing the units of the field and the associated units in the
  database. If the field has no units, \ref units::none must be used.

  - **range_type**: a resolved type of the \ref fields::range template,
  indicating the position of the field in the database (can be a set of ranges).

  On the other hand, the members of a field can be accessed using the \ref
  fields::get function, using as a template argument the structure
  (*Subfield*) that identifies the member.

  The access to a value of a field (or a member of a field) is handled by
  \ref fields::accessor, which automatically handles the system of units
  and the access to members of a field. If at some point the associated
  type is a \ref std::optional, the value is accessed without checking its
  validity.
 */
namespace reactions::fields {

  /// Define a \ref std::tuple with the types of the fields provided
  template <class Tuple> struct underlying_types;

  /// \copydoc underlying_types
  template <class... Field> struct underlying_types<std::tuple<Field...>> {
    using type = std::tuple<typename Field::value_type...>;
  };

  /// \copydoc underlying_types
  template <class Fields>
  using underlying_types_t = typename underlying_types<Fields>::type;

  /// Check if a type represents an optional
  template <class> struct is_optional : std::false_type {};

  /// Check if a type represents an optional
  template <class T> struct is_optional<std::optional<T>> : std::true_type {};

  /// Alias to check if a type represents an optional
  template <class T> constexpr auto is_optional_v = is_optional<T>::value;

  /// Contain the information whether a field is optional or not
  template <class Field, class Enable = void>
  struct is_optional_field : std::false_type {};

  /// \copydoc is_optional_field
  template <class Field>
  struct is_optional_field<
      Field, std::enable_if_t<is_optional_v<typename Field::value_type>>>
      : std::true_type {};

  /// \copydoc is_optional_field
  template <class Field>
  constexpr auto is_optional_field_v = is_optional_field<Field>::value;

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
  /// Field for a tag
  struct tag {};

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

  /// Simple structure composed by a value and its error
  template <class T, class Enable = void> struct value_and_error;

  /// \copydoc value_and_error
  template <class T>
  struct value_and_error<T,
                         std::enable_if_t<std::is_floating_point_v<T>, void>> {

    using value_type = T;

    /// Empty constructor
    value_and_error(){};

    /// Build the class with forwarded arguments
    template <class Value, class Error>
    value_and_error(Value value_, Error error_)
        : value(value_), error(error_) {}
    /// Build the class from a field constant expression
    template <class Value, class Error>
    value_and_error(fill<Value, Error> &&f)
        : value(std::get<0>(f)), error(std::get<1>(f)) {}

    /// Value
    value_type value;
    /// Error
    value_type error;
  };

  /// Represent a value, its error and a identifier tag
  template <class ValueType, class TagType>
  struct value_and_error_with_tag : value_and_error<ValueType> {

    using base_type = value_and_error<ValueType>;

    /// Empty constructor
    value_and_error_with_tag() : base_type{} {};

    /// Build the class with forwarded arguments
    template <class Value, class Error>
    value_and_error_with_tag(Value value_, Error error_, TagType tag_)
        : base_type{value_, error_}, tag(tag_) {}
    /// Build the class from a field constant expression
    template <class Value, class Error>
    value_and_error_with_tag(fill<Value, Error, TagType> &&f)
        : base_type{std::get<0>(f), std::get<1>(f)}, tag(std::get<2>(f)) {}

    /// Internal tag
    TagType tag;
  };

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
    value_and_errors(Value value_, ErrorLower error_lower_,
                     ErrorUpper error_upper_)
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
  template <class Field> struct get_t;

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

  /// Accessor to the tag
  template <> struct get_t<tag> {
    template <class T> constexpr auto const &operator()(T const &t) const {
      return access_value(t).tag;
    }
  };

  /// Determine a returned type
  template <class T, class Subfield, class... S> struct type_of {
    using type = typename type_of<Subfield, S...>::type;
  };

  /// \copydoc type_of
  template <class T, class Subfield> struct type_of<T, Subfield> {
    using type =
        std::enable_if_t<std::is_default_constructible_v<T>,
                         std::decay_t<decltype(get_t<Subfield>{}(T{}))>>;
  };

  /// \copydoc type_of
  template <class T, class... Subfield>
  using type_of_t = typename type_of<T, Subfield...>::type;

  /// Multiplication of the value and error by a constant
  template <class T>
  value_and_error<T> operator*(value_and_error<T> const &vae, T f) {
    return {f * vae.value, f * vae.error};
  }

  /// Multiplication of the value and error by a constant
  template <class T>
  value_and_error<T> operator*(T f, value_and_error<T> const &vae) {
    return vae * f;
  }

  /// Multiplication of the value and error by a constant
  template <class V, class T>
  value_and_error_with_tag<V, T>
  operator*(value_and_error_with_tag<V, T> const &vae, V f) {
    return {f * vae.value, f * vae.error, vae.tag};
  }

  /// Multiplication of the value and error by a constant
  template <class V, class T>
  value_and_error_with_tag<V, T>
  operator*(V f, value_and_error_with_tag<V, T> const &vae) {
    return vae * f;
  }

  /// Multiplication of the value and errors by a constant
  template <class T>
  value_and_errors<T> operator*(value_and_errors<T> const &vae, T f) {
    return {f * vae.value, f * vae.error_lower, f * vae.error_upper};
  }

  /// Multiplication of the value and errors by a constant
  template <class T>
  value_and_errors<T> operator*(T f, value_and_errors<T> const &vae) {
    return vae * f;
  }

  /// Accessor to a value/error field
  template <class Field> static constexpr get_t<Field> get;

  /// Optional for \ref float type
  using float_opt = std::optional<float>;
  /// Optional for \ref double type
  using double_opt = std::optional<double>;
  /// Optional \ref value_and_error for single-precision floating-point type
  using ve_float_opt = std::optional<value_and_error<float>>;
  /// Optional \ref value_and_error for double-precision floating-point type
  using ve_double_opt = std::optional<value_and_error<double>>;
  /// Optional \ref value_and_error_with_tag for single-precision floating-point
  /// type
  template <class TagType>
  using vet_float_opt = std::optional<value_and_error_with_tag<float, TagType>>;
  /// Optional \ref value_and_error_with_tag for double-precision floating-point
  /// type
  template <class TagType>
  using vet_double_opt =
      std::optional<value_and_error_with_tag<double, TagType>>;
  /// Optional \ref value_and_errors for single-precision floating-point type
  using ves_float_opt = std::optional<value_and_errors<float>>;
  /// Optional \ref value_and_errors for double-precision floating-point type
  using ves_double_opt = std::optional<value_and_errors<double>>;

  /// Status code of a conversion to an arithmetic or \ref std::optional type
  enum conversion_status : int {
    success, ///< the conversion succeeded
    empty,   ///< the object is missing
    failed   ///<  the conversion failed
  };

  /// Internal utilitites for the \ref reactions::fields namespace
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

  /// Read a field composed by value and an error in a line from a file
  template <class Ranges, class T>
  conversion_status read_field(value_and_error<T> &out, std::string const &s) {
    // std::from_chars is not correctly implemented in GCC, and
    // we can not work with std::string_view objects
    static_assert(std::tuple_size_v<Ranges> == 3);

    auto b = s.find_first_not_of(' ', std::tuple_element_t<0, Ranges>::min);

    if (b >= std::tuple_element_t<1, Ranges>::max)
      return empty;

    auto value_sc = read_field<std::tuple_element_t<0, Ranges>>(out.value, s);
    auto error_sc = read_field<std::tuple_element_t<1, Ranges>>(out.error, s);

    if (value_sc == empty || error_sc == empty)
      return failed; // either all are defined or none
    else if (value_sc == failed || error_sc == failed)
      return failed;
    else
      return success;
  }

  /// Read a field composed by value and an error in a line from a file
  template <class Ranges, class ValueType, class TagType>
  conversion_status
  read_field(value_and_error_with_tag<ValueType, TagType> &out,
             std::string const &s) {
    // std::from_chars is not correctly implemented in GCC, and
    // we can not work with std::string_view objects
    static_assert(std::tuple_size_v<Ranges> == 3);

    auto b = s.find_first_not_of(' ', std::tuple_element_t<0, Ranges>::min);

    if (b >= std::tuple_element_t<2, Ranges>::max)
      return empty;

    auto value_sc = read_field<std::tuple_element_t<0, Ranges>>(out.value, s);
    auto error_sc = read_field<std::tuple_element_t<1, Ranges>>(out.error, s);
    auto tag_sc = read_field<std::tuple_element_t<2, Ranges>>(out.tag, s);

    if (value_sc == empty || error_sc == empty || tag_sc == empty)
      return failed; // either all are defined or none
    else if (value_sc == failed || error_sc == failed || tag_sc == failed)
      return failed;
    else
      return success;
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

    auto value_sc = read_field<std::tuple_element_t<0, Ranges>>(out.value, s);
    auto error_lower_sc =
        read_field<std::tuple_element_t<1, Ranges>>(out.error_lower, s);
    auto error_upper_sc =
        read_field<std::tuple_element_t<2, Ranges>>(out.error_upper, s);

    if (value_sc == empty || error_lower_sc == empty || error_upper_sc == empty)
      return failed; // either all are defined or none
    else if (value_sc == failed || error_lower_sc == failed ||
             error_upper_sc == failed)
      return failed;
    else
      return success;
  }

  /// Access the subtype of a set of fields
  template <class Field, class... Subfield> struct field_member_type {
    using type =
        type_of_t<remove_optional_t<typename Field::value_type>, Subfield...>;
  };

  /// \copydoc field_member_type
  template <class Field> struct field_member_type<Field> {
    using type = remove_optional_t<typename Field::value_type>;
  };

  /// \copydoc field_member_type
  template <class Field, class... Subfield>
  using field_member_type_t =
      typename field_member_type<Field, Subfield...>::type;

  /// Convert the given object to a string
  template <class T> std::string to_string(T const &v) {
    return std::to_string(v);
  }

  /// \copydoc to_string
  template <> std::string to_string(std::string const &v) { return v; }
} // namespace reactions::fields
