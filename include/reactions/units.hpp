/* \file
\brief Definition of units
 */
#pragma once
#include "reactions/exceptions.hpp"
#include "reactions/pow_enum.hpp"
#include <string>
#include <type_traits>

namespace reactions {

  /// Energy units
  REACTIONS_POW_ENUM_WITH_UNKNOWN(energy_units, eV, keV, MeV, GeV, TeV, PeV);
  /// Time units
  REACTIONS_POW_ENUM_WITH_UNKNOWN(time_units, ys, zs, as, fs, ps, ns, us, ms,
                                  sec, min, hour, day, year, ky, My, Gy, Ty, Py,
                                  Ey, Zy, Yy);

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
      case (energy_units::unknown_energy_units):
        break;
      }

      throw internal_error(
          (std::string{
               "Attempt to compute a scale factor of an unknown unit: \""} +
           energy_units_properties::to_string(u) + '"')
              .c_str());
    }

    /// Compute the scale factor for the given unit
    auto scale_factor_for(time_units u) {

      switch (u) {
      case (time_units::ys):
        return 1e-24;
      case (time_units::zs):
        return 1e-21;
      case (time_units::as):
        return 1e-18;
      case (time_units::fs):
        return 1e-15;
      case (time_units::ps):
        return 1e-12;
      case (time_units::ns):
        return 1e-9;
      case (time_units::us):
        return 1e-6;
      case (time_units::ms):
        return 1e-3;
      case (time_units::sec):
        return 1.;
      case (time_units::min):
        return 60. * scale_factor_for(time_units::sec);
      case (time_units::hour):
        return 60. * scale_factor_for(time_units::min);
      case (time_units::day):
        return 24. * scale_factor_for(time_units::hour);
      case (time_units::year):
        return 365. * scale_factor_for(time_units::day);
      case (time_units::ky):
        return 1e3 * scale_factor_for(time_units::year);
      case (time_units::My):
        return 1e6 * scale_factor_for(time_units::year);
      case (time_units::Gy):
        return 1e9 * scale_factor_for(time_units::year);
      case (time_units::Ty):
        return 1e12 * scale_factor_for(time_units::year);
      case (time_units::Py):
        return 1e15 * scale_factor_for(time_units::year);
      case (time_units::Ey):
        return 1e18 * scale_factor_for(time_units::year);
      case (time_units::Zy):
        return 1e21 * scale_factor_for(time_units::year);
      case (time_units::Yy):
        return 1e24 * scale_factor_for(time_units::year);
      case (time_units::unknown_time_units):
        break;
      }

      throw internal_error(
          (std::string{
               "Attempt to compute a scale factor of an unknown unit: \""} +
           time_units_properties::to_string(u) + '"')
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

    /// \copydoc reference
    template <time_units U> struct reference<time_units, U> {
      using units_type = time_units;
      /// Determine the scale factor from a reference
      static constexpr auto scale_factor(time_units u) {
        if (u == U)
          return 1.;
        else
          return scale_factor_for(U) / scale_factor_for(u);
      }
    };

    /// Whether a type can be affected by units
    template <class T> struct is_type_affected_by_units {
      static constexpr auto value =
          (std::is_class_v<std::decay_t<fields::remove_optional_t<T>>> ||
           std::is_floating_point_v<
               std::decay_t<fields::remove_optional_t<T>>>);
    };

    /// \copydoc is_type_affected_by_units
    template <class T>
    static constexpr auto is_type_affected_by_units_v =
        is_type_affected_by_units<T>::value;

    /// Whether a subfield must be modified due to the presence of units
    template <class Field, class... Subfield> struct has_units {
      static constexpr auto value =
          !std::is_same_v<typename Field::units_reference_type, units::none> &&
          is_type_affected_by_units_v<
              fields::field_member_type_t<Field, Subfield...>>;
    };

    /// \copydoc has_units
    template <class Field> struct has_units<Field> {
      static constexpr auto value =
          !std::is_same_v<typename Field::units_reference_type, units::none> &&
          is_type_affected_by_units_v<typename Field::value_type>;
    };

    /// \copydoc has_units
    template <class Field, class... Subfield>
    static constexpr auto has_units_v = has_units<Field, Subfield...>::value;

    /// The return type after taking into account the possible units
    template <class Field, class... Subfield> struct return_type {
      using type = std::conditional_t<
          has_units_v<Field, Subfield...>,
          fields::field_member_type_t<Field, Subfield...>,
          fields::field_member_type_t<Field, Subfield...> const &>;
    };

    /// \copydoc return_type
    template <class Field, class... Subfield>
    using return_type_t = typename return_type<Field, Subfield...>::type;

    /// Object to access fields
    template <class Field, class... Subfield> struct accessor_t;

    /// Static object to access the fields of a tuple-like object
    template <class Field, class... Subfield>
    static constexpr accessor_t<Field, Subfield...> accessor;

    /// \copydoc accessor_t
    template <class Field> struct accessor_t<Field> {
      /// Access a value and process the units
      template <class T> constexpr auto const &operator()(T const &f) const {
        if constexpr (fields::is_optional_v<T>)
          return f.value();
        else
          return f;
      }
    };

    /// \copydoc accessor_t
    template <class Field, class Subfield, class... S>
    struct accessor_t<Field, Subfield, S...> {
      /// \copydoc accessor_t::operator()
      template <class T>
      constexpr fields::field_member_type_t<Field, Subfield, S...> const &
      operator()(T const &f) const {

        if constexpr (fields::is_optional_v<T>)
          return accessor<Field, S...>(fields::get<Subfield>(f.value()));
        else
          return accessor<Field, S...>(fields::get<Subfield>(f));
      }
    };

    /// Object to access fields and process the units
    template <class Field, class... Subfield> struct accessor_with_units_t {
      /// Access the value of the subfield and process the units
      template <class SystemOfUnits, class T>
      constexpr return_type_t<Field, Subfield...>
      operator()(SystemOfUnits const &sou, T const &f) const {
        using R = typename Field::units_reference_type;
        using U = typename R::units_type;
        return R::scale_factor(sou.template units<U>()) *
               accessor<Field, Subfield...>(f);
      }
    };

    /// Static object to access the fields of a tuple-like object
    template <class Field, class... Subfield>
    static constexpr accessor_with_units_t<Field, Subfield...>
        accessor_with_units;

  } // namespace units
} // namespace reactions
