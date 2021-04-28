/*! \file
  \brief Utilities to work with element types
 */
#pragma once

#include "reactions/nubase.hpp"
#include "reactions/pdg.hpp"
#include "reactions/pow_enum.hpp"

#include <functional>
#include <string>

namespace reactions {
  /// Alias for an element based on a simple string
  using string_element = std::string;
} // namespace reactions

/*! \brief Handle the properties of elements
 */
namespace reactions::element_traits {

  /// Define properties for a given element kind
  template <class T> struct properties;

  /// Definition of the properties of a PDG element
  template <> struct properties<reactions::pdg_element> {
    using type = reactions::pdg_element;
    static constexpr auto builder = [](std::string const &s) {
      return reactions::pdg_database::instance()(s);
    };
  };

  /// Definition of the properties of a NuBase element
  template <> struct properties<reactions::nubase_element> {
    using type = reactions::nubase_element;
    static constexpr auto builder = [](std::string const &s) {
      return reactions::nubase_database::instance()(s);
    };
  };

  /// Definition of the properties of a string element
  template <> struct properties<reactions::string_element> {
    using type = reactions::string_element;
    static constexpr auto builder = [](std::string const &s) { return s; };
  };

  /// Actual `C++` type for the given kind of element
  template <class T> using element_t = typename properties<T>::type;

  /// Default builder for a given kind of element
  template <class T> static constexpr auto builder = properties<T>::builder;

  /// Default builder type for a given kind of element
  template <class T> using builder_t = decltype(builder<T>);

  /// General builder type for a given kind of element
  template <class T>
  using builder_tpl_t = std::function<T(std::string const &)> const &;
} // namespace reactions::element_traits
