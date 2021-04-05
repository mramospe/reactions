"""
Functions for testing
"""
import functools
import reactions


def restore_pdg_database(function):
    """
    Decorate a function and restore the PDG database path on exit, and clear the
    cache
    """
    @functools.wraps(function)
    def wrapper(*args, **kwargs):
        db = reactions.pdg_database.get_database_path()
        try:
            output = function(*args, **kwargs)
        finally:  # if the function fails we keep having a valid database
            reactions.pdg_database.set_database_path(db)
            reactions.pdg_database.clear_cache()
        return output
    return wrapper


def toggle_database_cache_status(db=None, clear_user_cache=False):
    """
    Yield the database two times with different cache status: one with it
    enabled and another one disabled.
    """
    db = db or reactions.pdg_database

    # here starts with the cache enabled
    db.enable_cache()
    yield db

    # here starts with the cache disabled
    if clear_user_cache:
        db.clear_cache()
    else:
        db.disable_cache()

    yield db

    # clear the cache at the end of the execution
    db.clear_cache()
