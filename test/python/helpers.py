"""
Functions for testing
"""
import functools
import reactions


def check_is_singleton(sgl):
    assert sgl() is sgl()


def restore_database(database):
    """
    Decorate a function and restore a database path on exit, and clear the
    cache
    """
    def wrapper(function):
        @functools.wraps(function)
        def _wrapper(*args, **kwargs):
            db = database.get_database_path()
            try:
                output = function(*args, **kwargs)
            finally:  # if the function fails we keep having a valid database
                database.set_database_path(db)
                database.clear_cache()
            return output
        return _wrapper
    return wrapper


def restore_nubase_database(function):
    """
    Decorate a function and restore the NuBase database path on exit, and clear the
    cache
    """
    return restore_database(reactions.nubase_database)(function)


def restore_pdg_database(function):
    """
    Decorate a function and restore the PDG database path on exit, and clear the
    cache
    """
    return restore_database(reactions.pdg_database)(function)


def toggle_database_cache_status(db, clear_user_cache=False):
    """
    Yield the database two times with different cache status: one with it
    enabled and another one disabled.
    """
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
