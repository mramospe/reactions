# Place where the project version is specified
__version__ = '0.0.0.dev2'

try:

    from .capi import (
        # types
        node, reaction, decay, string_element, pdg_element, pdg_database_sgl, pdg_system_of_units_sgl,
        # functions
        is_element, node_type,
        # errors
        DatabaseError, LookupError, SyntaxError, InternalError, ValueError
    )

    pdg_database = pdg_database_sgl()  # overwrite it
    pdg_system_of_units = pdg_system_of_units_sgl()  # overwrite it

    # Set the path to the database(s)
    import os
    pdg_database.set_database_path(os.path.abspath(os.path.join(
        os.path.dirname(__file__), 'data', 'pdg_mass_width_2020.txt')))
    del os

    # Variables exported
    __all__ = ['node', 'reaction', 'decay', 'string_element', 'pdg_element', 'pdg_database', 'pdg_system_of_units', 'pdg_database_sgl', 'pdg_system_of_units_sgl',
               'is_element', 'node_type',
               'DatabaseError', 'LookupError', 'SyntaxError', 'InternalError', 'ValueError']

except ModuleNotFoundError:
    # To allow setup.py to get the version
    pass
