# Place where the project version is specified
__version__ = '0.0.0.dev0'

try:

    from .reactions import (
        # types
        reaction, decay, string_element, pdg_element, pdg_database,
        # functions
        is_element, node_type, get_pdg_database, set_pdg_database,
        # errors
        DatabaseError, LookupError, SyntaxError
    )

    # Set the path to the database(s)
    import os
    set_pdg_database(os.path.abspath(os.path.join(
        os.path.dirname(__file__), 'data', 'pdg_mass_width_2020.txt')))
    del os

    # Variables exported
    __all__ = ['reaction', 'decay', 'string_element', 'pdg_element', 'pdg_database',
               'is_element', 'node_type', 'get_pdg_database', 'set_pdg_database',
               'DatabaseError', 'LookupError', 'SyntaxError']

except ModuleNotFoundError:
    # To allow setup.py to get the version
    pass
