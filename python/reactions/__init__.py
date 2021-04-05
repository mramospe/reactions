# Place where the project version is specified
__version__ = '0.0.0.dev0'

try:

    from .reactions import (
        # types
        node, reaction, decay, string_element, pdg_element, pdg_database,
        # functions
        is_element, node_type,
        # errors
        DatabaseError, LookupError, SyntaxError, InternalError
    )

    pdg_database = pdg_database()  # overwrite it

    # Set the path to the database(s)
    import os
    pdg_database.set_database_path(os.path.abspath(os.path.join(
        os.path.dirname(__file__), 'data', 'pdg_mass_width_2020.txt')))
    del os

    # Variables exported
    __all__ = ['node', 'reaction', 'decay', 'string_element', 'pdg_element', 'pdg_database',
               'is_element', 'node_type',
               'DatabaseError', 'LookupError', 'SyntaxError', 'InternalError']

except ModuleNotFoundError:
    # To allow setup.py to get the version
    pass
