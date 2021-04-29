#!/bin/env python
"""
Determine the size of the buffers used by the pdg_element::latex_name
function in C++.
"""
import argparse
import math
import os
import reactions

if __name__ == '__main__':

    parser = argparse.ArgumentParser(description=__doc__)
    parser.parse_args()  # no arguments

    for name, database in ('PDG', reactions.pdg_database), ('NuBase', reactions.nubase_database):

        all_elements = database.all_elements()

        superscript_chars = ('+', '-', '0', '*', '\'')

        ratio = max([len(e.latex_name)/len(e.name) for e in all_elements])

        if name == 'PDG':
            ss = max([sum(e.name.count(s) for s in superscript_chars)
                      for e in all_elements])
            additional = f'{os.linesep}- superscript size: {ss}'
        else:
            additional = ''

        print(
            f'Buffer sizes ({name}):{os.linesep}- latex/name ratio: {int(math.ceil(ratio))} ({ratio:.2f}){additional}')
