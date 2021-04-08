"""
Test the systems of units of the databases
"""
import pytest
import reactions


def test_pdg_system_of_units_sgl():
    from reactions.capi import pdg_system_of_units_sgl
    assert pdg_system_of_units_sgl() is pdg_system_of_units_sgl()


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
