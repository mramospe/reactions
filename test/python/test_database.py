"""
Test the properties of the database objects
"""
import pytest
import reactions

import helpers


def test_pdg_database():
    a = reactions.pdg_database()
    b = reactions.pdg_database()
    assert a is b


@helpers.restore_pdg_database
def test_pdg_getter_setter():
    first = reactions.pdg_database()
    second = reactions.pdg_database()
    reactions.set_pdg_database_path('some_path')
    assert first.get_database_path() == reactions.get_pdg_database_path()
    assert first.get_database_path() == second.get_database_path()
    first.set_database_path('another_path')
    assert first.get_database_path() == reactions.get_pdg_database_path()
    assert first.get_database_path() == second.get_database_path()

    # try to use a non-existing database
    with pytest.raises(reactions.DatabaseError):
        reactions.pdg_element('mu+')


@helpers.restore_pdg_database
def test_pdg_database_cache():
    db = reactions.pdg_database()
    path = db.get_database_path()
    db.enable_cache()
    db.set_database_path(path)


@helpers.restore_pdg_database
def test_pdg_user_register():

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
