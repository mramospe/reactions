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

  template <char... C, std::size_t... I>
  bool match_token_impl(token<C...>, std::index_sequence<I...>,
                        std::string::const_iterator const &it) {
    return ((*(it + I) == C) && ...);
  }

  /// Check if the iterator at the current position matches the token
  template <class Token>
  bool match_token(std::string::const_iterator const &it) {
    return match_token_impl(Token{}, std::make_index_sequence<Token::size>(),
                            it);
  }
} // namespace reactions::tokens
