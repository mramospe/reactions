"""
Test the properties of the database objects
"""
import os
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
    reactions.set_pdg_database('some_path')
    assert first.get_database() == reactions.get_pdg_database()
    assert first.get_database() == second.get_database()
    first.set_database('another_path')
    assert first.get_database() == reactions.get_pdg_database()
    assert first.get_database() == second.get_database()

    # try to use a non-existing database
    with pytest.raises(reactions.DatabaseError):
        reactions.pdg_element('mu+')


@helpers.restore_pdg_database
def test_pdg_database_cache():
    db = reactions.pdg_database()
    path = db.get_database()
    db.enable_cache()
    db.set_database(path)
