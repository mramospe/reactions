"""
Process a sets of reactions and/or decays, checking its syntax
"""
from . import pdg_database, reaction, decay

import argparse


def check_syntax(reactions, decays, kind):
    """ Check the syntax of the given decays and reactions """
    if reactions:
        for r in reactions:
            reaction(r, kind=kind)

    if decays:
        for d in decays:
            decay(d, kind=kind)


def print_table(kind):
    """ Print the table of particles """
    if kind == 'pdg':
        with open(pdg_database.get_database_path()) as db:
            print(db.read())
    else:
        raise ValueError(f'Unknown kind "{kind}"')


parser = argparse.ArgumentParser(description=__doc__)
subparsers = parser.add_subparsers(help='Command to run')

p_check_syntax = subparsers.add_parser(
    check_syntax.__name__.replace('_', '-'), help=check_syntax.__doc__)
p_check_syntax.set_defaults(function=check_syntax)
p_check_syntax.add_argument('--reactions', nargs='+', type=str,
                            help='Set of reactions to check')
p_check_syntax.add_argument('--decays', nargs='+', type=str,
                            help='Set of decays to check')
p_check_syntax.add_argument('--kind', type=str, choices=('string', 'pdg'),
                            default='string', help='Type of the elements')

p_print_table = subparsers.add_parser(
    print_table.__name__.replace('_', '-'), help=print_table.__doc__)
p_print_table.set_defaults(function=print_table)
p_print_table.add_argument('--kind', type=str, choices=('pdg',),
                           default='pdg', help='Element kind of the table to print')

args = parser.parse_args()

options = vars(args)

function = options.pop('function')

function(**options)
