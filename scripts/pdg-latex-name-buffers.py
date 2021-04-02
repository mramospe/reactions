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

    all_elements = reactions.pdg_database().all_elements()

    superscript_chars = ('+', '-', '0', '*', '\'')

    ratio = max([len(e.latex_name)/len(e.name) for e in all_elements])
    ss = max([sum(e.name.count(s) for s in superscript_chars)
              for e in all_elements])

    print(
        f'Buffer sizes:{os.linesep}- latex/name ratio: {int(math.ceil(ratio))} ({ratio:.2f}) {os.linesep}- superscript size: {ss}')
