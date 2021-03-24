"""
Process a sets of reactions and/or decays, checking its syntax
"""
from . import reaction, decay

import argparse
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--reactions', nargs='+', type=str,
                    help='Set of reactions to check')
parser.add_argument('--decays', nargs='+', type=str,
                    help='Set of decays to check')
parser.add_argument('--kind', type=str, choices=('string', 'pdg'),
                    default='string', help='Type of the elements')
args = parser.parse_args()

if args.reactions:
    for r in args.reactions:
        reaction(r, kind=args.kind)

if args.decays:
    for d in args.decays:
        decay(d, kind=args.kind)
