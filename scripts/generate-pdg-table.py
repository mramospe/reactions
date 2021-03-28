#!/bin/env python
"""
Expand the PDG particle table at https://pdg.lbl.gov/2020/html/computer_read.html so
particles can be identified by the name (i.e. there are no multiple entries sharing
the same name). This is achieved by including the charge in the name, and processing
the charge-conjugate of the particles.
"""
import argparse
import datetime
import os
import re
import tempfile

from urllib import request

ONLINE_PDG_TABLE = 'https://pdg.lbl.gov/2020/mcdata/mass_width_2020.mcd'

# These values correspond to how the data is written in the PDG table
PID_FIELDS = [slice(i * 8, (i + 1) * 8) for i in range(4)]  # -, 0, +, ++
MASS_FIELD = slice(33, 51)
MASS_ERROR_UPPER_FIELD = slice(52, 60)
MASS_ERROR_LOWER_FIELD = slice(61, 69)
WIDTH_FIELD = slice(70, 88)
WIDTH_ERROR_UPPER_FIELD = slice(89, 97)
WIDTH_ERROR_LOWER_FIELD = slice(98, 106)
NAME_CHARGE_FIELD = slice(107, 128)

# Size of the fields for the output table. They have been modified with respect to
# the PDG table in order to fit decrease the file size and boost the reading.
NAME_SIZE = 16  # size reserved for strings
ID_SIZE = 10  # size reserved for the particle ID
THREE_CHARGE_SIZE = 2  # size reserved for three times the charge
VALUE_SIZE = 16  # size reserved for values
ERROR_SIZE = 9  # size reserved for errors
IS_SELF_CC_SIZE = 1  # size for the self-conjugate flag

# Lengths of the data fields
LENGTHS = (NAME_SIZE, ID_SIZE, THREE_CHARGE_SIZE, VALUE_SIZE, ERROR_SIZE,
           ERROR_SIZE, VALUE_SIZE, ERROR_SIZE, ERROR_SIZE, IS_SELF_CC_SIZE)


def format_name(n):
    """ Format the name """
    return f'{n:>{NAME_SIZE}}'


def format_id(i):
    """ Format the PDG ID """
    return f'{i:>{ID_SIZE}}'


def format_three_charge(c):
    """ Format the charge """
    return f'{c:>{THREE_CHARGE_SIZE}}'


def parse_value(v):
    """ Format a value (mass or width) """
    if v.strip():
        return f'{float(v):>{VALUE_SIZE}.{VALUE_SIZE - 7}e}'
    else:
        return VALUE_SIZE * ' '


def parse_error(e):
    """ Format an error """
    if e.strip():
        return f'{abs(float(e)):>{ERROR_SIZE}.{ERROR_SIZE - 7}e}'
    else:
        return ERROR_SIZE * ' '


def parse_is_self_cc(b):
    """ Format the self-conjugate flag """
    return f'{int(bool(b)):>{IS_SELF_CC_SIZE}}'


CHARGE_FROM_PDG_CHARGE = {'0': 0,
                          '+1/3': +1./3,
                          '-1/3': -1./3,
                          '+2/3': +2./3,
                          '-2/3': -2./3,
                          '+': +1,
                          '++': +2,
                          '-': -1,
                          '--': -2}


if __name__ == '__main__':

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('output_table', type=str, help='Output table')
    parser.add_argument('--input-table', '-i', type=str, default=None,
                        help='PDG database table. If it is not provided the table is downloaded form the internet')
    parser.add_argument('--overwrite', '-w', action='store_true',
                        help='If set, overwrite the output file, if existing')
    args = parser.parse_args()

    # Order of the fields in the output table, together with the size reserved for them
    # Nte that the lower and upper errors are inverted with respect to the PDG
    FIELDS = (
        ('name', format_name),
        ('id', format_id),
        ('three_charge', format_three_charge),
        ('mass', parse_value),
        ('mass_error_lower', parse_error),
        ('mass_error_upper', parse_error),
        ('width', parse_value),
        ('width_error_lower', parse_error),
        ('width_error_upper', parse_error),
        ('is_self_cc', parse_is_self_cc),
    )

    # commented line
    commented_line = re.compile('^w*\*')

    # components in parentheses
    paranthesized = re.compile('\(([A-z]*|[0-9]*|[0-9][A-Z]|[a-z][0-9])\)')

    # get the base name
    re_base_name = re.compile('^[A-z]*/?[A-z]*')

    # check if a base name is an uppercase character
    re_uppercase_character = re.compile('^[A-Z]$')

    # check if a base name is an uppercase string
    re_uppercase_string = re.compile('^[A-Z]{2,}$')

    # neutrinos need an special treatment
    re_neutrino = re.compile('^nu$')

    if not args.overwrite and os.path.exists(args.output_table):
        if os.path.isfile(args.output_table):
            raise OSError(
                f'Path "{args.output_table}" already refers to a file. Provide "--overwrite" or "-w" to overwrite the file.')
        else:
            raise OSError(
                f'Path "{args.output_table}" already exists and is not a file')

    if args.input_table is None:
        tmpdir = tempfile.TemporaryDirectory()
        input_table = os.path.join(tmpdir.name, 'pdg_table.txt')
        request.urlretrieve(ONLINE_PDG_TABLE, input_table)
    else:
        input_table = args.input_table

    # store the particle names to check when we must add charges to the name
    pdg_particle_names = []
    with open(input_table) as input_file:
        for line in filter(lambda s: not commented_line.match(s), input_file):
            pdg_particle_names.append(
                line[NAME_CHARGE_FIELD].split()[0].strip())

    new_particle_names = []  # to check that there are unique names
    with open(input_table) as input_file, open(args.output_table, 'wt') as output_file:

        output_file.write(f'''*
* Reactions particle table for PDG elements, generated on {datetime.datetime.today().date()}
*
* This table has been generated from the PDG table of masses and widths at https://pdg.lbl.gov/2020/html/computer_read.html,
* where particles have been expanded by charge so they can be efficiently read by name.#
*
''')

        def parse_int(s): return None if not s else int(s)

        for line in filter(lambda s: not commented_line.match(s), input_file):

            # extract the information
            pids = tuple(parse_int(line[s].strip())
                         for s in PID_FIELDS)  # -, 0, +, ++

            config = dict(
                mass=line[MASS_FIELD],
                mass_error_upper=line[MASS_ERROR_UPPER_FIELD],
                mass_error_lower=line[MASS_ERROR_LOWER_FIELD],
                width=line[WIDTH_FIELD],
                width_error_upper=line[WIDTH_ERROR_UPPER_FIELD],
                width_error_lower=line[WIDTH_ERROR_LOWER_FIELD],
            )
            name, charge = (s.strip() for s in line[107:128].split())

            # determine the parts of the name
            base_name = re_base_name.match(name).group()
            primes = "'" * name.count("'")
            excited = '*' if '*' in name else ''

            sub_super_scripts = paranthesized.findall(name)

            # define the base of the name
            if len(sub_super_scripts) == 0:
                extra = ''
            elif len(sub_super_scripts) == 1:
                if re.compile('\d+').match(*sub_super_scripts):
                    extra = '({0})'.format(*sub_super_scripts)
                else:
                    f, = sub_super_scripts
                    if re_uppercase_character.match(f):
                        base_name, extra = f'{base_name}{f}', ''
                    else:
                        extra = f'_{f}'
            elif len(sub_super_scripts) == 2:
                f, s = sub_super_scripts
                if re_uppercase_character.match(f):
                    extra = f'{f}({s})'
                else:
                    extra = f'_{f}({s})'
            else:
                raise RuntimeError(
                    'Can not process more than three paranthesized values')

            core_name = f'{base_name}{primes}{extra}{excited}'

            charges = charge.split(',')

            def write_element_(config, name, three_charge, pid):
                """"""
                config = dict(config)
                config['name'] = name
                config['three_charge'] = f'{three_charge:+d}'
                config['id'] = pid
                output_file.write(
                    ' '.join(f(config[s]) for s, f in FIELDS) + os.linesep)
                new_particle_names.append(config['name'])

            def write_particle_and_antiparticle_(config, core_name, pid, three_charge, particle_format, antiparticle_format):
                """"""
                for i, (t, p, f) in enumerate([(+three_charge, +pid, particle_format), (-three_charge, -pid, antiparticle_format)]):
                    c = int(round(t / 3.))
                    if c == 0:
                        charge = 0
                    else:
                        charge = abs(c) * '+' if c > 0 else abs(c) * '-'
                    write_element_(config, f.format(
                        core_name=core_name, charge=charge), t, p)

            def category_from_pid_(pid):
                """"""
                if pid % 10000 > 999:
                    return 'baryon'
                elif pid % 1000 > 99:
                    return 'meson'
                else:
                    return 'fundamental'

            def is_self_cc_meson(pid):
                q1, q2 = pid % 100 // 10, pid % 1000 // 100
                if q1 == q2:
                    return True
                else:
                    return False

            if len(charges) == 1:
                # we can use "charge"
                three_charge = int(round(3 * CHARGE_FROM_PDG_CHARGE[charge]))

                pid = pids[0]

                category = category_from_pid_(pid)

                # change latter for baryons
                config['is_self_cc'] = (three_charge == 0)

                # particle
                if abs(three_charge) in (1, 2):  # is a quark
                    write_particle_and_antiparticle_(
                        config, core_name, pid, three_charge, particle_format='{core_name}', antiparticle_format='{core_name}~')
                elif three_charge != 0:  # charged
                    if category == 'baryon':
                        write_particle_and_antiparticle_(
                            config, core_name, pid, three_charge, particle_format='{core_name}{charge}', antiparticle_format='{core_name}~{charge}')
                    else:  # meson or fundamental
                        write_particle_and_antiparticle_(
                            config, core_name, pid, three_charge, particle_format='{core_name}{charge}', antiparticle_format='{core_name}{charge}')
                else:  # neutral
                    if category == 'baryon':

                        config['is_self_cc'] = False

                        write_particle_and_antiparticle_(
                            config, core_name, pid, three_charge, particle_format='{core_name}{charge}', antiparticle_format='{core_name}~{charge}')

                    elif category == 'meson':

                        # KS0, KL0, ...
                        if re_uppercase_string.match(base_name):
                            config['is_self_cc'] = True
                        else:
                            config['is_self_cc'] = is_self_cc_meson(pid)

                        # K, D, B, ...
                        if re_uppercase_character.match(base_name):
                            write_particle_and_antiparticle_(
                                config, core_name, pid, three_charge, particle_format='{core_name}{charge}', antiparticle_format='{core_name}~{charge}')
                        # pi, KS, KL, ...
                        elif pdg_particle_names.count(base_name) > 1 or re_uppercase_string.match(base_name):
                            write_element_(
                                config, f'{core_name}{charge}', three_charge, pid)
                        else:  # eta, phi, ...
                            write_element_(config, core_name,
                                           three_charge, pid)

                    else:  # fundamental

                        # H0, Z0, ...
                        if re_uppercase_character.match(base_name):
                            write_element_(
                                config, f'{core_name}{charge}', three_charge, pid)
                        elif re_neutrino.match(base_name):  # nu_e, nu_mu, nu_tau
                            config['is_self_cc'] = False
                            write_particle_and_antiparticle_(
                                config, core_name, pid, three_charge, particle_format=core_name, antiparticle_format='{core_name}~')
                        else:  # g, gamma, ...
                            write_element_(config, core_name,
                                           three_charge, pid)

            else:
                for charge, pid in zip(charges, pids):

                    three_charge = int(
                        round(3 * CHARGE_FROM_PDG_CHARGE[charge]))

                    if category == 'baryon':

                        config['is_self_cc'] = False
                        write_particle_and_antiparticle_(
                            config, core_name, pid, three_charge, particle_format='{core_name}{charge}', antiparticle_format='{core_name}~{charge}')

                    elif category == 'meson':

                        config['is_self_cc'] = is_self_cc_meson(pid)

                        if three_charge == 0:
                            write_element_(
                                config, f'{core_name}{charge}', three_charge, pid)
                        else:
                            write_particle_and_antiparticle_(
                                config, core_name, pid, three_charge, particle_format='{core_name}{charge}', antiparticle_format='{core_name}{charge}')
                    else:
                        raise RuntimeError(
                            'Unable to process fundamental particles with several associated PIDs')

    # check that particles are unique
    if len(new_particle_names) != len(set(new_particle_names)):
        repeated = [p.strip() for p in set(new_particle_names)
                    if new_particle_names.count(p) > 1]
        raise RuntimeError(
            f'Particles do not have unique names: {repeated}. Output table is invalid.')

    # reopen the table we just created to verify the length of the strings written in it
    line_size = len(LENGTHS) + sum(LENGTHS)  # account for the additional \n
    with open(args.output_table) as table:
        for line in filter(lambda s: not commented_line.match(s), table):
            assert len(line) == line_size
