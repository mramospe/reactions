/* \file
\brief Definition of units
 */
#pragma once
#include "reactions/exceptions.hpp"
#include "reactions/pow_enum.hpp"

namespace reactions {

  /// Energy units
  REACTIONS_POW_ENUM_WITH_UNKNOWN(energy_units, eV, keV, MeV, GeV, TeV, PeV);

  /// Contain functions to determine the scale factors for the given units
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

    /// Proxy to use the template argument as a reference to determine scale
    /// factors
    template <energy_units U> struct reference {
      /// Determine the scale factor from a reference
      static constexpr auto scale_factor(energy_units u) {
        if (u == U)
          return 1.;
        else
          return scale_factor_for(U) / scale_factor_for(u);
      }
    };
  } // namespace units
} // namespace reactions
