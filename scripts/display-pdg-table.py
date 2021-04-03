#!/bin/env python
"""
Compile a LaTeX document with the information of the available PDG elements.
"""
import argparse
import math
import os
import reactions
import subprocess
import tempfile


if __name__ == '__main__':

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--tmp-dir', '-d', type=str, default=None,
                        help='Optional directory to write the temporary files')
    parser.add_argument('--compiler', '-c', type=str, default='pdflatex',
                        help='Command used to compiler the LaTeX source')
    parser.add_argument('--output', '-o', type=str, default='pdg_table.pdf',
                        help='Name of the output file. The LaTeX file will have the same name but with ".tex" extension')
    args = parser.parse_args()

    # Get the table of particles
    all_elements = reactions.pdg_database.all_elements()

    fields = ['name', 'latex_name', 'pdg_id', 'three_charge', 'mass',
              'mass_error_lower', 'mass_error_upper', 'width', 'width_error_lower', 'width_error_upper', 'is_self_cc']

    def format_float(v, prec=10):
        """ Format a float, using scientific notation if necessary """
        av = abs(v)
        if v == 0.:
            return '0'
        elif av > 100 or av < 0.01:
            exp = int(math.floor(math.log10(av)))
            s = f'{v / 10**exp:.{prec - 1}f}'
            e = f'\\times 10^{{{exp}}}'
            while s[-1] == '0':
                s = s[:-1]
            if s[-1] == '.':
                s = s[:-1]
            return f'\\({s} {e}\\)'
        else:
            s = f'{v:.{prec + 1}f}'
            while s[-1:] == '0':
                s = s[:-1]
            if s[-1] == '.':
                s = s[:-1]
            return s

    def format_error(v):
        """ Format a float representing an error """
        return format_float(v, prec=2)

    formatters = dict(
        name=lambda v: f'\\verb|{v}|',
        latex_name=lambda v: f'\\({v}\\)',
        pdg_id=lambda v: f'\\({v:+d}\\)',
        three_charge=lambda v: f'\\({v:+d}\\)',
        mass=format_float,
        mass_error_lower=format_error,
        mass_error_upper=format_error,
        width=format_float,
        width_error_lower=format_error,
        width_error_upper=format_error,
        is_self_cc=lambda v: 'true' if v else 'false',
    )

    titles = dict(
        name='Name',
        latex_name='LaTeX',
        pdg_id='ID',
        three_charge='3 x charge',
        mass='Mass',
        mass_error_lower='Mass lower error',
        mass_error_upper='Mass upper error',
        width='Width',
        width_error_lower='Width lower error',
        width_error_upper='Width upper error',
        is_self_cc='self charge-conj.',
    )

    base_name = os.path.splitext(os.path.basename(args.output))[0]
    latex_name = f'{base_name}.tex'
    pdf_name = f'{base_name}.pdf'

    if args.tmp_dir is None:
        tmpdir_ = tempfile.TemporaryDirectory()
        tmpdir = tmpdir_.name
    else:
        tmpdir = args.tmp_dir

    tmpfile = os.path.join(tmpdir, latex_name)

    with open(tmpfile, 'wt') as table_file:

        body = ' & '.join(
            f'\\textbf{{{titles[f]}}}' for f in fields) + f'\\\\{os.linesep}\\hline{os.linesep}'
        for el in all_elements:
            line = ' & '.join(' ' if getattr(el, f) is None else formatters[f](
                getattr(el, f)) for f in fields)
            body = f'{body}{line}\\\\{os.linesep}\\hline{os.linesep}'

        table_file.write(r'''\documentclass[12pt,a3paper]{{article}}
\usepackage{{amsmath}}
\usepackage{{pdflscape}}
\usepackage{{longtable}}
\usepackage[margin=0.5in]{{geometry}}
\pagenumbering{{gobble}}
\begin{{document}}
\centering
\renewcommand{{\arraystretch}}{{1.2}}
\begin{{landscape}}
\textbf{{\Huge{{Table of PDG elements}}}}\\
\normalsize{{Masses and widths are provided in \(\text{{GeV}}/c^2\)}}
\vspace{{0.75cm}}
\begin{{longtable}}{{{columns}}}
\hline
{body}
\end{{longtable}}
\end{{landscape}}
\end{{document}}
'''.format(columns='|c' * len(fields) + '|', body=body))

    # run the command twice to process the references
    subprocess.check_call([args.compiler, latex_name], cwd=tmpdir)
    subprocess.check_call([args.compiler, latex_name], cwd=tmpdir)

    if args.tmp_dir is None:
        # copy the PDF file from the temporary directory
        subprocess.check_call(
            ['cp', os.path.join(tmpdir, pdf_name), args.output])
