#pragma once
#include <tuple>

namespace reactions::utils {

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
  template <class Tuple, class E> struct tuple_index {
    static constexpr auto value =
        detail::tuple_index<0, Tuple, E, std::tuple_element_t<0, Tuple>>::value;
  };

  /// \copydoc tuple_index
  template <class Tuple, class E>
  static constexpr auto tuple_index_v = tuple_index<Tuple, E>::value;
} // namespace reactions::utils
