/*! \file
  The macro \ref REACTIONS_POW_ENUM_WITH_UNKNOWN is provided in this file, which
  defines a class containing an enumeration type, and several functions and
  constant objects to iterate over the enumeration values and convert/parse
  them to/from strings.
*/
#ifndef REACTIONS_POWENUM_HPP
#define REACTIONS_POWENUM_HPP

#include <cstdlib>
#include <string>
#include <utility>

/*! \brief Utilities to define smart enumeration types
 */
namespace reactions::pow_enum {

  /// String as a template of characters
  template <char... C> struct string_tpl {
    static constexpr char chars[sizeof...(C) + 1] = {C..., '\0'};
  };

  /// Helper struct to convert strings to templates of strings
  template <typename lambda_str_type> struct string_builder {
    template <std::size_t... indices> struct produce {
      typedef string_tpl<lambda_str_type{}.chars[indices]...> result;
    };
  };

  /// Functions to transform a template of characters into a string
  template <std::size_t count, template <std::size_t...> class meta_functor,
            std::size_t... indices>
  struct apply_range {
    typedef typename apply_range<count - 1, meta_functor, count - 1,
                                 indices...>::result result;
  };

  template <template <std::size_t...> class meta_functor,
            std::size_t... indices>
  struct apply_range<0, meta_functor, indices...> {
    typedef typename meta_functor<indices...>::result result;
  };

  /// Group of string templates
  template <class... S> struct string_group {
    static constexpr const char *chars[sizeof...(S) + 1] = {S::chars...,
                                                            nullptr};
  };

  /// Implementation of the function to parse a string
  template <class... Strings, char... C>
  constexpr auto parse_string_impl(string_group<Strings...>, string_tpl<C...>,
                                   string_tpl<>) {
    return string_group<Strings..., string_tpl<C...>>{};
  }

  /// Implementation of the function to parse a string
  template <class... Strings, char... Cp, char C0, char... C>
  constexpr auto parse_string_impl(string_group<Strings...>, string_tpl<Cp...>,
                                   string_tpl<C0, C...>) {
    if constexpr (C0 == ' ' || C0 == '\t')
      return parse_string_impl(string_group<Strings...>{}, string_tpl<Cp...>{},
                               string_tpl<C...>{});
    else if constexpr (C0 == ',')
      return parse_string_impl(string_group<Strings..., string_tpl<Cp...>>{},
                               string_tpl<>{}, string_tpl<C...>{});
    else
      return parse_string_impl(string_group<Strings...>{},
                               string_tpl<Cp..., C0>{}, string_tpl<C...>{});
  }

  /*! \brief Parse a string template and return it as an array of strings
   *
   * Commas act as separators, whilst whitespaces and tabulators are omitted
   */
  template <char... C> constexpr auto parse_string(string_tpl<C...> s) {
    return parse_string_impl(string_group<>{}, string_tpl<>{}, s);
  }
} // namespace reactions::pow_enum

/*!
 * This macro defines a new class with the given name, which itself defines an
 * enum and a series of elements to facilitate operations with them.
 * - enum_type: type of the enumeration
 * - size: number of elements defined
 * - list: elements as an array
 * - from_string: get the element associated to the given string (return unknown
 if not found)
 * - to_string: represent an element as a string
 */
#define REACTIONS_POW_ENUM_WITH_UNKNOWN(enum_name, ...)                        \
  enum enum_name : int { unknown = 0, __VA_ARGS__ };                           \
                                                                               \
  struct enum_name##_properties {                                              \
                                                                               \
    static constexpr std::size_t size =                                        \
        std::initializer_list{__VA_ARGS__}.size();                             \
                                                                               \
    static constexpr enum_name list[size] = {__VA_ARGS__};                     \
                                                                               \
    template <enum_name E, std::size_t I = 0>                                  \
    static constexpr auto index_impl() {                                       \
      static_assert(I < size);                                                 \
      if constexpr (list[I] == E)                                              \
        return I;                                                              \
      else                                                                     \
        return index_impl<E, I + 1>();                                         \
    }                                                                          \
                                                                               \
    template <enum_name E> static constexpr auto index() {                     \
      return index_impl<E>();                                                  \
    }                                                                          \
                                                                               \
    static enum_name from_string(const char *s) {                              \
      auto i = 0u;                                                             \
      for (; i < size; ++i)                                                    \
        if (strcmp(                                                            \
                [] {                                                           \
                  struct constexpr_string_type {                               \
                    const char *chars = #__VA_ARGS__;                          \
                  };                                                           \
                  return reactions::pow_enum::parse_string(                    \
                      reactions::pow_enum::apply_range<                        \
                          sizeof(#__VA_ARGS__) - 1,                            \
                          reactions::pow_enum::string_builder<                 \
                              constexpr_string_type>::produce>::result{});     \
                }()                                                            \
                    .chars[i],                                                 \
                s) == 0)                                                       \
          return list[i];                                                      \
      return unknown;                                                          \
    }                                                                          \
                                                                               \
    static enum_name from_string(std::string const &s) {                       \
      return from_string(s.c_str());                                           \
    }                                                                          \
                                                                               \
    static const char *to_c_string(enum_name e) {                              \
      auto i = 0u;                                                             \
      for (; i < size; ++i)                                                    \
        if (list[i] == e)                                                      \
          break;                                                               \
      return [] {                                                              \
        struct constexpr_string_type {                                         \
          const char *chars = #__VA_ARGS__;                                    \
        };                                                                     \
        return reactions::pow_enum::parse_string(                              \
            reactions::pow_enum::apply_range<                                  \
                sizeof(#__VA_ARGS__) - 1,                                      \
                reactions::pow_enum::string_builder<                           \
                    constexpr_string_type>::produce>::result{});               \
      }()                                                                      \
                 .chars[i];                                                    \
    }                                                                          \
    static std::string to_string(enum_name e) { return to_c_string(e); }       \
  }
// must use a comma

#endif // REACTIONS_POWENUM_HPP
