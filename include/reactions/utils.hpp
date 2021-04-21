#pragma once
#include <tuple>

/// General utilities
namespace reactions::utils {

  /// Internal utilities for the \ref reactions::utils namespace
  namespace detail {
    /// Index corresponding to a given element
    template <std::size_t I, class Tuple, class E0, class E1>
    struct tuple_index {
      static constexpr auto value =
          tuple_index<I + 1, Tuple, E0,
                      std::tuple_element_t<I + 1, Tuple>>::value;
    };

    /// \copydoc tuple_index
    template <std::size_t I, class Tuple, class E>
    struct tuple_index<I, Tuple, E, E> {
      static constexpr auto value = I;
    };
  } // namespace detail

  /// \copydoc detail::tuple_index
  template <class E, class Tuple> struct tuple_index {
    static constexpr auto value =
        detail::tuple_index<0, Tuple, E, std::tuple_element_t<0, Tuple>>::value;
  };

  /// \copydoc tuple_index
  template <class E, class Tuple>
  static constexpr auto tuple_index_v = tuple_index<E, Tuple>::value;

  /// Check whether a type is a specialization of a template
  template <class Type, template <class...> class Template>
  struct is_template_specialization : std::false_type {};

  /// \copydoc is_template_specialization
  template <template <class...> class Template, class... T>
  struct is_template_specialization<Template<T...>, Template> : std::true_type {
  };

  template <class Type, template <class...> class Template>
  static constexpr auto is_template_specialization_v =
      is_template_specialization<Type, Template>::value;
} // namespace reactions::utils
