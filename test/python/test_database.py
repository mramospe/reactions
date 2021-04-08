"""
Test the properties of the database objects
"""
import pytest
import reactions

import helpers


def test_pdg_database():
    from reactions.capi import pdg_database_sgl
    a = pdg_database_sgl()
    b = pdg_database_sgl()
    assert a is b


def test_pdg_unique_elements():
    all_elements = reactions.pdg_database.all_elements()
    assert len({e.name for e in all_elements}) == len(all_elements)
    assert len({e.pdg_id for e in all_elements}) == len(all_elements)


@helpers.restore_pdg_database
def test_pdg_getter_setter():
    from reactions.capi import pdg_database_sgl
    first = pdg_database_sgl()
    second = pdg_database_sgl()
    reactions.pdg_database.set_database_path('some_path')
    assert first.get_database_path() == reactions.pdg_database.get_database_path()
    assert first.get_database_path() == second.get_database_path()
    first.set_database_path('another_path')
    assert first.get_database_path() == reactions.pdg_database.get_database_path()
    assert first.get_database_path() == second.get_database_path()

    # try to use a non-existing database
    with pytest.raises(reactions.DatabaseError):
        reactions.pdg_element('mu+')


@helpers.restore_pdg_database
def test_pdg_database_cache():
    db = reactions.pdg_database
    path = db.get_database_path()
    nels = len(db.all_elements())
    db.enable_cache()
    assert nels == len(db.all_elements())
    db.set_database_path(path)


@helpers.restore_pdg_database
def test_pdg_user_register():

    for db in helpers.toggle_database_cache_status(clear_user_cache=False):
        for el in db.all_elements():
            assert db(el.name) == db(el.pdg_id)

    for db in helpers.toggle_database_cache_status(clear_user_cache=True):

        z = reactions.pdg_element(name="Z0'", pdg_id=9999999, three_charge=0,
                                  mass_and_errors=None, width_and_errors=None, is_self_cc=True)
        db.register_element(z)

        db.register_element(name="Z0''", pdg_id=9999998, three_charge=0,
                            mass_and_errors=None, width_and_errors=None, is_self_cc=True)
        db.register_element("Z0'''", 99999997, 0, None, None, True)

        with pytest.raises(reactions.DatabaseError):  # existing name
            db.register_element("Z0", 99999996, 0, None, None, True)

        with pytest.raises(reactions.DatabaseError):  # existing PDG ID
            db.register_element("Z0''''", 1, 0, None, None, True)

        for el in db.all_elements():
            assert db(el.name) == db(el.pdg_id)


def test_charge_conjugate():
    for pa, cc in ('K(S)0', 'K(S)0'), ('pi+', 'pi-'), ('p', 'p~'), ('Lambda', 'Lambda~'):
        f, s = reactions.pdg_database(pa), reactions.pdg_database(cc)
        assert reactions.pdg_database.charge_conjugate(f) == s
