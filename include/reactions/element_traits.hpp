/*! \file
  \brief Define the types of elements that can go into a reaction or decay.
 */
#ifndef REACTIONS_ELEMENT_TYPES_HPP
#define REACTIONS_ELEMENT_TYPES_HPP

#include "reactions/database_pdg.hpp"
#include "reactions/pow_enum.hpp"

#include <functional>

namespace reactions {
  /*! \brief Kind of elements that can be processed
   *
   * This exposes the kind of elements that can be processed in the
   * package. Such elements are:
   * - **reactions::pdg**: for PDG elements
   * - **reactions::string**: for string elements
   * - **reactions::unknown**: when the type is unknown
   */
  REACTIONS_POW_ENUM_WITH_UNKNOWN(element_kind, pdg, string);
} // namespace reactions

/*! \brief Kinds of elements
 */
namespace reactions::element_traits {

  /// Define properties for a given element kind
  template <element_kind T> struct properties;

  /// Definition of the properties of a PDG element
  template <> struct properties<element_kind::pdg> {
    using type = database_pdg::element;
    static constexpr auto builder = [](std::string const &s) {
      return database_pdg::database::instance()(s);
    };
  };

  /// Definition of the properties of a string element
  template <> struct properties<element_kind::string> {
    using type = std::string;
    static constexpr auto builder = [](std::string const &s) { return s; };
  };

  /// Actual C++ type for the given kind of element
  template <element_kind T> using element_t = typename properties<T>::type;

  /// Default builder for a given kind of element
  template <element_kind T>
  static constexpr auto builder = properties<T>::builder;

  /// Default builder type for a given kind of element
  template <element_kind T> using builder_t = decltype(builder<T>);

  /// General builder type for a given kind of element
  template <class T>
  using builder_tpl_t = std::function<T(std::string const &)> const &;
} // namespace reactions::element_traits

#endif // REACTIONS_ELEMENT_TYPES_HPP
