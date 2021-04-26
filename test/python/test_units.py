"""
Test the systems of units of the databases
"""
import pytest
import reactions

import helpers


def test_nubase_system_of_units_sgl():
    helpers.check_is_singleton(reactions.nubase_system_of_units_sgl)


def test_pdg_system_of_units_sgl():
    helpers.check_is_singleton(reactions.pdg_system_of_units_sgl)


def test_nubase_system_of_units():
    # energy units
    mass_excess_prev = reactions.nubase_database('1H').mass_excess
    assert reactions.nubase_system_of_units.get_energy_units() == 'keV'
    reactions.nubase_system_of_units.set_energy_units('eV')
    assert reactions.nubase_system_of_units.get_energy_units() == 'eV'
    assert abs(mass_excess_prev -
               reactions.nubase_database('1H').mass_excess * 1e-3) < 1e-12
    with pytest.raises(reactions.ValueError):
        reactions.nubase_system_of_units.set_energy_units('unknown')
    with pytest.raises(reactions.ValueError):
        reactions.nubase_system_of_units.set_energy_units('different')
    # time units
    half_life_prev = reactions.nubase_database('1n').half_life
    assert reactions.nubase_system_of_units.get_time_units() == 'sec'
    reactions.nubase_system_of_units.set_time_units('ms')
    assert reactions.nubase_system_of_units.get_time_units() == 'ms'
    assert abs(half_life_prev -
               reactions.nubase_database('1n').half_life * 1e-3) < 1e-12
    with pytest.raises(reactions.ValueError):
        reactions.nubase_system_of_units.set_time_units('unknown')
    with pytest.raises(reactions.ValueError):
        reactions.nubase_system_of_units.set_time_units('different')


def test_pdg_system_of_units():
    mass_prev = reactions.pdg_database('K(S)0').mass
    assert reactions.pdg_system_of_units.get_energy_units() == 'GeV'
    reactions.pdg_system_of_units.set_energy_units('MeV')
    assert reactions.pdg_system_of_units.get_energy_units() == 'MeV'
    assert abs(mass_prev - reactions.pdg_database('K(S)0').mass * 1e-3) < 1e-12
    with pytest.raises(reactions.ValueError):
        reactions.pdg_system_of_units.set_energy_units('unknown')
    with pytest.raises(reactions.ValueError):
        reactions.pdg_system_of_units.set_energy_units('different')
