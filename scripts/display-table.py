#!/bin/env python
"""
Compile a LaTeX document with the information of the available elements.
"""
import argparse
import math
import os
import reactions
import subprocess
import tempfile


if __name__ == '__main__':

    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(help='Type of element to process')
    pdg_parser = subparsers.add_parser('pdg', description='PDG elements')
    pdg_parser.set_defaults(database=reactions.pdg_database, title='Table of PDG elements', subtitle=r'Masses and widths are provided in \(\text{GeV}/c^2\)',
                            fields=['name', 'latex_name', 'pdg_id', 'three_charge', 'mass',
                                    'mass_error_lower', 'mass_error_upper', 'width', 'width_error_lower', 'width_error_upper', 'is_self_cc'])
    nubase_parser = subparsers.add_parser(
        'nubase', description='NuBase elements')
    nubase_parser.set_defaults(database=reactions.nubase_database, title='Table of NuBase elements', subtitle=r'Mass excesses are provided in \(\text{keV}/c^2\), half-lifes in seconds',
                               fields=['name', 'latex_name', 'nubase_id', 'atomic_number', 'mass_number', 'mass_excess', 'mass_excess_error', 'mass_excess_from_systematics', 'is_stable', 'half_life', 'half_life_error', 'half_life_from_systematics', 'is_ground_state'])

    for p in pdg_parser, nubase_parser:
        p.add_argument('--tmp-dir', '-d', type=str, default=None,
                       help='Optional directory to write the temporary files')
        p.add_argument('--compiler', '-c', type=str, default='pdflatex',
                       help='Command used to compiler the LaTeX source')
        p.add_argument('--output', '-o', type=str, default='table.pdf',
                       help='Name of the output file. The LaTeX file will have the same name but with ".tex" extension')

    args = parser.parse_args()

    # Get the table of particles
    all_elements = args.database.all_elements()

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
        # common
        name=lambda v: f'\\verb|{v}|',
        latex_name=lambda v: f'\\({v}\\)',
        # PDG element
        pdg_id=lambda v: f'\\({v:+d}\\)',
        three_charge=lambda v: f'\\({v:+d}\\)',
        mass=format_float,
        mass_error_lower=format_error,
        mass_error_upper=format_error,
        width=format_float,
        width_error_lower=format_error,
        width_error_upper=format_error,
        is_self_cc=lambda v: 'true' if v else 'false',
        # NuBase element
        nubase_id=lambda v: f'\\({v:d}\\)',
        atomic_number=lambda v: f'\\({v:+d}\\)',
        mass_number=lambda v: f'\\({v:d}\\)',
        mass_excess=format_float,
        mass_excess_error=format_error,
        mass_excess_from_systematics=lambda v: 'true' if v else 'false',
        is_stable=lambda v: 'true' if v else 'false',
        half_life=format_float,
        half_life_error=format_error,
        half_life_from_systematics=lambda v: 'true' if v else 'false',
        is_ground_state=lambda v: 'true' if v else 'false',
    )

    titles = dict(
        # common
        name='Name',
        latex_name='LaTeX',
        # PDG element
        pdg_id='ID',
        three_charge='3 x charge',
        mass='Mass',
        mass_error_lower='Mass lower error',
        mass_error_upper='Mass upper error',
        width='Width',
        width_error_lower='Width lower error',
        width_error_upper='Width upper error',
        is_self_cc='self charge-conj.',
        # NuBase element
        nubase_id='ID',
        atomic_number='Z',
        mass_number='A',
        mass_excess='Mass excess',
        mass_excess_error='Mass excesss error',
        mass_excess_from_systematics='Mass excess from syst.',
        is_stable='Stable',
        half_life='Half-life',
        half_life_error='Half-life error',
        half_life_from_systematics='Half-life from syst.',
        is_ground_state='Ground state',
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
            f'\\textbf{{{titles[f]}}}' for f in args.fields) + f'\\\\{os.linesep}\\hline{os.linesep}'
        for el in all_elements:
            line = ' & '.join(' ' if getattr(el, f) is None else formatters[f](
                getattr(el, f)) for f in args.fields)
            body = f'{body}{line}\\\\{os.linesep}\\hline{os.linesep}'

        table_file.write(r'''\documentclass[12pt,a3paper]{{article}}
\usepackage{{amsmath}}
\usepackage{{pdflscape}}
\usepackage{{longtable}}
\usepackage[version=4]{{mhchem}}
\usepackage[margin=0.5in]{{geometry}}
\pagenumbering{{gobble}}
\begin{{document}}
\centering
\renewcommand{{\arraystretch}}{{1.2}}
\begin{{landscape}}
\textbf{{\Huge{{{title}}}}}\\
\normalsize{{{subtitle}}}
\vspace{{0.75cm}}
\begin{{longtable}}{{{columns}}}
\hline
{body}
\end{{longtable}}
\end{{landscape}}
\end{{document}}
'''.format(columns='|c' * len(args.fields) + '|', body=body, title=args.title, subtitle=args.subtitle))

    # run the command twice to process the references
    subprocess.check_call([args.compiler, latex_name], cwd=tmpdir)
    subprocess.check_call([args.compiler, latex_name], cwd=tmpdir)

    if args.tmp_dir is None:
        # copy the PDF file from the temporary directory
        subprocess.check_call(
            ['cp', os.path.join(tmpdir, pdf_name), args.output])
