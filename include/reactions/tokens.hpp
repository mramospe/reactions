/*! \file
  \brief Tokens allowed in a reaction or decay.
 */
#pragma once

#include <cstdlib>
#include <string>
#include <utility>

/*! \brief Syntax tokens
 */
namespace reactions::tokens {

  /// Template to define new tokens
  template <char... C> struct token {
    static const size_t size = sizeof...(C);
  };

  /// Defines the separation of two elements
  using space = token<' '>;
  /// Defines a reaction
  using arrow = token<'-', '>'>;
  /// Defines the begining of a reaction
  using left_bra = token<'{'>;
  /// Defines the end of a reaction
  using right_bra = token<'}'>;

  /// Check if the iterator at the current position matches the token
  template <char... C, std::size_t... I>
  bool match_token_impl(token<C...>, std::index_sequence<I...>,
                        std::string::const_iterator const &it) {
    return ((*(it + I) == C) && ...);
  }

  /// \copydoc match_token_impl
  template <class Token>
  bool match_token(std::string::const_iterator const &it) {
    return match_token_impl(Token{}, std::make_index_sequence<Token::size>(),
                            it);
  }

  /// Check if the given character matches any of the template arguments
  template <char... C> constexpr bool match_any(char c) {
    return ((C == c) || ...);
  }

  /// Check if the given character is within the range defined by the template
  /// arguments
  template <char C0, char C1> constexpr bool match_range(char c) {
    return (c >= C0) && (c <= C1);
  }
} // namespace reactions::tokens
