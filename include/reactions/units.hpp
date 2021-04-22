/* \file
\brief Definition of units
 */
#pragma once
#include "reactions/database.hpp"
#include "reactions/exceptions.hpp"
#include "reactions/pow_enum.hpp"

namespace reactions {

  /// Energy units
  REACTIONS_POW_ENUM_WITH_UNKNOWN(energy_units, eV, keV, MeV, GeV, TeV, PeV);

  /*! \brief Contain functions to determine the scale factors for the given
     units Units are handled by multiplying values returned by the elements of
     the reactions by a factor that depends on the unit used as a reference.

     Units are handled by arguments passed as a *SystemOfUnits* template
     argument, which must be instances defining a member function of the form:

     \code{.cpp}
     template<class Units>
     Units const& units() const {
       // return the appropriate units here
       ...
     }
     \endcode

     implementing the unit to return for each kind of system of units.

     Units are defined as `C++` enumerations, whose types are:

     - **energy_units**: units of energy
   */
  namespace units {

    /// Represent the abscence of units
    struct none {};

    /// Compute the scale factor for the given unit
    auto scale_factor_for(energy_units u) {
      switch (u) {
      case (energy_units::eV):
        return 1.;
      case (energy_units::keV):
        return 1e3;
      case (energy_units::MeV):
        return 1e6;
      case (energy_units::GeV):
        return 1e9;
      case (energy_units::TeV):
        return 1e12;
      case (energy_units::PeV):
        return 1e12;
      case (energy_units::unknown):
        break;
      }

      throw internal_error(
          (std::string{
               "Attempt to compute a scale factor of an unknown unit: \""} +
           energy_units_properties::to_string(u) + '"')
              .c_str());
    }

    /// Use the template argument as a reference to determine scale factors
    template <class Units, Units U> struct reference;

    /// \copydoc reference
    template <energy_units U> struct reference<energy_units, U> {
      using units_type = energy_units;
      /// Determine the scale factor from a reference
      static constexpr auto scale_factor(energy_units u) {
        if (u == U)
          return 1.;
        else
          return scale_factor_for(U) / scale_factor_for(u);
      }
    };

    /// Functor to process the units of a field
    template <class F, class Enable = void> struct process_units_t;

    /// \copydoc process_units_t
    template <class F>
    struct process_units_t<
        F, std::enable_if_t<
               std::is_same_v<typename F::units_reference_type, units::none>,
               void>> {
      template <class SystemOfUnits, class T>
      constexpr auto const &operator()(SystemOfUnits const &,
                                       T const &value) const {
        if constexpr (database::is_optional_v<std::decay_t<T>>)
          return value.value();
        else
          return value;
      }
    };

    /// \copydoc process_units_t
    template <class F>
    struct process_units_t<
        F, std::enable_if_t<
               !std::is_same_v<typename F::units_reference_type, units::none>,
               void>> {
      template <class SystemOfUnits, class T>
      constexpr auto operator()(SystemOfUnits const &sou, T &&value) const {
        using R = typename F::units_reference_type;
        using U = typename R::units_type;
        if constexpr (database::is_optional_v<std::decay_t<T>>)
          return R::scale_factor(sou.template units<U>()) * value.value();
        else
          return R::scale_factor(sou.template units<U>()) * value;
      }
    };

    /// \copydoc process_units_t
    template <class F> static constexpr process_units_t<F> process_units;

    /// Determine the type to be returned after processing the units of a field
    template <class F, class T, class Enable = void>
    struct return_type_after_parsing_units {
      using type = std::decay_t<database::remove_optional_t<T>>;
    };

    /// \copydoc return_type_after_parsing_units
    template <class F, class T>
    struct return_type_after_parsing_units<
        F, T,
        std::enable_if_t<
            std::is_same_v<typename F::units_reference_type, units::none>>> {
      using type = std::decay_t<database::remove_optional_t<T>> const &;
    };

    /// \copydoc return_type_after_parsing_units
    template <class F, class T>
    using return_type_after_parsing_units_t =
        typename return_type_after_parsing_units<F, T>::type;

    /// Return type of the accessor of a field
    template <class F, class... S> struct return_type {
      using type = return_type_after_parsing_units_t<
          F, database::field_member_type_t<typename F::value_type, S...>>;
    };

    /// \copydoc return_type
    template <class F, class... S>
    using return_type_t = typename return_type<F, S...>::type;

    /// Object to access fields
    template <class F, class... S> struct accessor_t;

    /// Static object to access the fields of a tuple-like object
    template <class F, class... S>
    static constexpr accessor_t<F, S...> accessor;

    /// \copydoc accessor_t
    template <class F> struct accessor_t<F> {
      /// Access a value and process the units
      template <class SystemOfUnits, class T>
      constexpr return_type_after_parsing_units_t<F, T>
      operator()(SystemOfUnits const &sou, T const &f) const {
        return reactions::units::process_units<F>(sou, f);
      }
    };

    /// \copydoc accessor_t
    template <class F, class S0, class... S> struct accessor_t<F, S0, S...> {
      /// \copydoc accessor_t::operator()
      template <class SystemOfUnits, class T>
      constexpr return_type_t<F, S0, S...> operator()(SystemOfUnits const &sou,
                                                      T const &f) const {

        if constexpr (database::is_optional_v<T>)
          return accessor<F, S...>(sou, database::get<S0>(f.value()));
        else
          return accessor<F, S...>(sou, database::get<S0>(f));
      }
    };
  } // namespace units
} // namespace reactions
