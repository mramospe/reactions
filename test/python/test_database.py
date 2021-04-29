"""
Test the properties of the database objects
"""
import pytest
import reactions

import helpers


def test_nubase_database():
    helpers.check_is_singleton(reactions.nubase_database_sgl)


def test_pdg_database():
    helpers.check_is_singleton(reactions.pdg_database_sgl)


def check_unique_elements(db, id_accessor):
    all_elements = db.all_elements()
    assert len({e.name for e in all_elements}) == len(all_elements)
    assert len({getattr(e, id_accessor)
                for e in all_elements}) == len(all_elements)


def test_nubase_unique_elements():
    check_unique_elements(reactions.nubase_database, 'nubase_id')


def test_pdg_unique_elements():
    check_unique_elements(reactions.pdg_database, 'pdg_id')


def check_getter_setter(db, db_sgl):
    first = db_sgl()
    second = db_sgl()
    db.set_database_path('some_path')
    assert first.get_database_path() == db.get_database_path()
    assert first.get_database_path() == second.get_database_path()
    first.set_database_path('another_path')
    assert first.get_database_path() == db.get_database_path()
    assert first.get_database_path() == second.get_database_path()
    # try to use a non-existing database
    with pytest.raises(reactions.DatabaseError):
        first('something')


@helpers.restore_nubase_database
def test_nubase_getter_setter():
    check_getter_setter(reactions.nubase_database,
                        reactions.nubase_database_sgl)


@helpers.restore_pdg_database
def test_pdg_getter_setter():
    check_getter_setter(reactions.pdg_database, reactions.pdg_database_sgl)


def check_cache(db):
    path = db.get_database_path()
    nels = len(db.all_elements())
    db.enable_cache()
    assert nels == len(db.all_elements())
    db.set_database_path(path)


@helpers.restore_nubase_database
def test_nubase_database_cache():
    check_cache(reactions.nubase_database)


@helpers.restore_pdg_database
def test_pdg_database_cache():
    check_cache(reactions.pdg_database)


@helpers.restore_nubase_database
def test_nubase_user_register():

    for db in helpers.toggle_database_cache_status(reactions.nubase_database, clear_user_cache=False):
        for el in db.all_elements():
            assert db(el.name) == db(el.nubase_id)

    for db in helpers.toggle_database_cache_status(reactions.nubase_database, clear_user_cache=True):

        n = reactions.nubase_element(name="999Un", nubase_id=999999000, atomic_number=999, mass_number=999,
                                     mass_excess_and_error_with_tag=None, is_stable=False, half_life_and_error_with_tag=None, is_ground_state=True)
        db.register_element(n)

        assert n.name == "999Un"

        reactions.nubase_element(name="998Un", nubase_id=999998000, atomic_number=999, mass_number=998,
                                 mass_excess_and_error_with_tag=None, is_stable=False, half_life_and_error_with_tag=None, is_ground_state=True)
        db.register_element("997Un", 999997000, 999, 997,
                            None, True, None, False)

        with pytest.raises(reactions.DatabaseError):  # existing name
            db.register_element("999Un", 999996000, 999, 999,
                                None, True, None, False)

        with pytest.raises(reactions.DatabaseError):  # existing NuBase ID
            db.register_element("996Un", 999999000, 999, 996,
                                None, True, None, False)

        for el in db.all_elements():
            assert db(el.name) == db(el.nubase_id)


@helpers.restore_pdg_database
def test_pdg_user_register():

    for db in helpers.toggle_database_cache_status(reactions.pdg_database, clear_user_cache=False):
        for el in db.all_elements():
            assert db(el.name) == db(el.pdg_id)

    for db in helpers.toggle_database_cache_status(reactions.pdg_database, clear_user_cache=True):

        z = reactions.pdg_element(name="Z0'", pdg_id=9999999, three_charge=0,
                                  mass_and_errors=None, width_and_errors=None, is_self_cc=True)
        db.register_element(z)

        assert z.name == "Z0'"

        db.register_element(name="Z0''", pdg_id=9999998, three_charge=0,
                            mass_and_errors=None, width_and_errors=None, is_self_cc=True)
        db.register_element("Z0'''", 99999997, 0, None, None, True)

        with pytest.raises(reactions.DatabaseError):  # existing name
            db.register_element("Z0", 99999996, 0, None, None, True)

        with pytest.raises(reactions.DatabaseError):  # existing PDG ID
            db.register_element("Z0''''", 1, 0, None, None, True)

        for el in db.all_elements():
            assert db(el.name) == db(el.pdg_id)


def test_pdg_charge_conjugate():
    for pa, cc in ('K(S)0', 'K(S)0'), ('pi+', 'pi-'), ('p', 'p~'), ('Lambda', 'Lambda~'):
        f, s = reactions.pdg_database(pa), reactions.pdg_database(cc)
        assert reactions.pdg_database.charge_conjugate(f) == s
