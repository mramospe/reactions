#!/bin/env python
"""
Process the NuBase table of elements to write it to a format that
becomes faster to read.
"""
import argparse
import datetime
import os
import tempfile

from urllib import request

ONLINE_NUBASE_TABLE = 'http://amdc.impcas.ac.cn/masstables/Ame2020/nubase_1.mas20'

MASS_NUMBER = slice(0, 3)
ATOMIC_NUMBER = slice(4, 7)
NAME = slice(11, 16)
ISOMER = slice(16, 17)
MASS_EXCESS = slice(18, 31)
MASS_EXCESS_ERROR = slice(31, 42)
HALF_LIFE = slice(69, 78)
HALF_LIFE_UNIT = slice(78, 80)
HALF_LIFE_ERROR = slice(81, 88)

NAME_SIZE = 8  # size reserved for strings
VALUE_SIZE = 16  # size reserved for values
ERROR_SIZE = 9  # size reserved for errors
BOOL_SIZE = 1  # size for the self-conjugate flag
AZ_SIZE = 3  # size reserved for the mass and atomic numbers
ID_SIZE = 3 * AZ_SIZE  # size reserved for the particle ID

time_units = {
    'ys': 1e-24,
    'zs': 1e-21,
    'as': 1e-18,
    'fs': 1e-15,
    'ps': 1e-12,
    'ns': 1e-9,
    'us': 1e-6,
    'ms': 1e-3,
    's': 1,  # reference
}
time_units['m'] = 60
time_units['h'] = 60 * time_units['m']
time_units['d'] = 24 * time_units['h']
time_units['y'] = 365 * 24 * time_units['h']
time_units['ky'] = 1e3 * time_units['y']
time_units['My'] = 1e6 * time_units['y']
time_units['Gy'] = 1e9 * time_units['y']
time_units['Ty'] = 1e12 * time_units['y']
time_units['Py'] = 1e15 * time_units['y']
time_units['Ey'] = 1e18 * time_units['y']
time_units['Zy'] = 1e21 * time_units['y']
time_units['Yy'] = 1e24 * time_units['y']


def parse_name(name):
    return f'{name:>{NAME_SIZE}}'


def parse_id(nid):
    return f'{int(nid):>{ID_SIZE}}'


def parse_value(value):
    if value:
        return f'{float(value):>{VALUE_SIZE}.{VALUE_SIZE - 7}e}'
    else:
        return f'{value:>{VALUE_SIZE}}'


def parse_error(error):
    if error:
        return f'{float(error):>{ERROR_SIZE}.1e}'
    else:
        return f'{error:>{ERROR_SIZE}}'


def parse_bool(value):
    return f'{int(value):>{BOOL_SIZE}}'


def parse_az(value):
    return f'{int(value):>{AZ_SIZE}}'


FIELDS = [
    ('name', parse_name),
    ('nid', parse_id),
    ('atomic_number', parse_az),
    ('mass_number', parse_az),
    ('mass_excess', parse_value),
    ('mass_excess_error', parse_error),
    ('mass_excess_from_systematics', parse_bool),
    ('is_stable', parse_bool),
    ('half_life', parse_value),
    ('half_life_error', parse_error),
    ('half_life_from_systematics', parse_bool),
    ('is_ground_state', parse_bool),
]


if __name__ == '__main__':

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('output_table', type=str, help='Output table')
    parser.add_argument('--input-table', '-i', type=str, default=None,
                        help='NuBase and database table. If it is not provided the table is downloaded from the internet')
    parser.add_argument('--overwrite', '-w', action='store_true',
                        help='If set, overwrite the output file, if existing')
    args = parser.parse_args()

    if not args.overwrite and os.path.exists(args.output_table):
        if os.path.isfile(args.output_table):
            raise OSError(
                f'Path "{args.output_table}" already refers to a file. Provide "--overwrite" or "-w" to overwrite the file.')
        else:
            raise OSError(
                f'Path "{args.output_table}" already exists and is not a file')

    if args.input_table is None:
        tmpdir = tempfile.TemporaryDirectory()
        input_table = os.path.join(tmpdir.name, 'nubase_table.txt')
        request.urlretrieve(ONLINE_NUBASE_TABLE, input_table)
    else:
        input_table, input_pdg_table = args.input_table

    with open(input_table) as input_file, open(args.output_table, 'wt') as output_file:

        output_file.write(f'''*
* Reactions particle table for NuBase elements, generated on {datetime.datetime.today().date()}
*
* This table has been generated from the NuBase table of masses and widths at
* {ONLINE_NUBASE_TABLE}. Masses and
* lifetimes referring to limits have been excluded.
*
''')

        # Add additional stable particles
        for i, p in enumerate(('gamma', 'e-', 'e+')):
            if '+' in p:
                atomic_number = +1
            elif '-' in p:
                atomic_number = -1
            else:
                atomic_number = 0

            config = dict(
                name=p,
                nid=i,
                mass_number=0,
                atomic_number=atomic_number,
                mass_excess=0.,
                mass_excess_error=0.,
                mass_excess_from_systematics=False,
                half_life=0.,
                half_life_error=0.,
                half_life_from_systematics=False,
                is_ground_state=True,
                is_stable=True,
            )
            output_file.write(
                ' '.join(frmt(config[f]) for f, frmt in FIELDS) + os.linesep)

        for line in filter(lambda s: not s.startswith('#'), input_file.readlines()):

            base_name = line[NAME].strip()
            isomer = line[ISOMER].strip()

            mass_excess = line[MASS_EXCESS].strip()
            mass_excess_error = line[MASS_EXCESS_ERROR].strip()

            if mass_excess.endswith('#'):
                mass_excess = mass_excess[:-1]
                mass_excess_error = mass_excess_error[:-1]
                mass_excess_from_systematics = True
            else:
                mass_excess_from_systematics = False

            half_life = line[HALF_LIFE].strip()
            half_life_units = line[HALF_LIFE_UNIT].strip()
            half_life_error = line[HALF_LIFE_ERROR].strip()

            if half_life == 'stbl':
                is_stable = True
                half_life, half_life_error = '', ''
                half_life_from_systematics = False
            elif half_life == 'p-unst':
                is_stable = False
                half_life, half_life_error = '', ''
                half_life_from_systematics = False
            else:
                is_stable = False

                if half_life.endswith('#'):
                    half_life_from_systematics = True
                    half_life = half_life[:-1]
                    half_life_error = half_life_error[:-1]
                else:
                    half_life_from_systematics = False

                try:
                    half_life = float(half_life)
                    half_life_error = float(half_life_error)
                except:
                    half_life, half_life_error = '', ''
                    half_life_from_systematics = False

            if half_life:
                half_life = float(half_life) * time_units[half_life_units]
                half_life_error = float(
                    half_life_error) * time_units[half_life_units]

            mass_number = line[MASS_NUMBER].strip()
            atomic_number = line[ATOMIC_NUMBER].strip()

            nid = f'{mass_number:0<{AZ_SIZE}}{atomic_number:0<{AZ_SIZE}}{ord(isomer or chr(0)):0<{AZ_SIZE}}'

            config = dict(
                name=f'{base_name}({isomer})' if isomer else base_name,
                nid=nid,
                mass_number=mass_number,
                atomic_number=atomic_number,
                is_stable=is_stable,
                half_life=half_life,
                half_life_error=half_life_error,
                half_life_from_systematics=half_life_from_systematics,
                mass_excess=mass_excess,
                mass_excess_error=mass_excess_error,
                mass_excess_from_systematics=mass_excess_from_systematics,
                is_ground_state=not isomer,
            )

            output_file.write(
                ' '.join(frmt(config[f]) for f, frmt in FIELDS) + os.linesep)
