"""
Functions for testing
"""
import functools
import reactions


def restore_pdg_database(function):
    """ Decorate a function and restore the PDG database path on exit
    """
    @functools.wraps(function)
    def wrapper(*args, **kwargs):
        db = reactions.get_pdg_database()
        output = function(*args, **kwargs)
        reactions.set_pdg_database(db)
    return wrapper
