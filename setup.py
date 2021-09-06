"""
Setup script for the "reactions" package
"""

import os
import re
import subprocess
import tempfile
import warnings

from setuptools import Command, Extension, setup

PWD = os.path.abspath(os.path.dirname(__file__))


def check_format_process(name, directory, process, compare=None):
    """
    Check the return code of a process. If "compare" is given, it is used to
    compare the output with it. By default it just checks that there is no output.
    """
    stdout, stderr = process.communicate()

    if process.returncode < 0:
        raise RuntimeError(
            f'Call to {name} exited with error {abs(process.returncode)}; the message is:{os.linesep}{stderr}')

    if compare is None:
        if len(stdout):
            raise RuntimeError(
                f'Found problems for files in directory "{directory}"')
    else:
        if stdout.decode() != compare:
            raise RuntimeError(
                f'Found problems for files in directory "{directory}"')


def files_with_extension(where, *exts):
    """
    Return all the files with the given extension in the package.
    """
    return [os.path.join(root, f) for root, _, files in os.walk(where) for f in filter(lambda s: any(s.endswith(f'.{e}') for e in exts), files)]


def python_files_in(d):
    """ Python files in the given directory """
    return files_with_extension(d, 'py')


def cpp_files_in(d):
    """ C++ files in the given directory """
    return files_with_extension(d, 'hpp', 'cpp', 'hpp.in')


def license_for_language(language):
    """
    Create the license string for the given language.
    """
    with open(os.path.join(PWD, 'LICENSE.txt')) as f:
        lines = f.read().split(os.linesep)[:3]  # take the first paragraphs

    if language == 'python':

        ml = max(map(len, lines)) + 2  # for the extra # and the whitespace

        text = os.linesep.join(f'# {l}' if l else '#' for l in lines)

        return ml * '#' + f'{os.linesep}{text}{os.linesep}' + ml * '#' + os.linesep

    elif language == 'cpp':

        ml = max(map(len, lines)) + 1

        text = os.linesep.join(f' * {l}' if l else ' *' for l in lines)

        return '/*' + ml * '*' + f'{os.linesep}{text}{os.linesep} ' + ml * '*' + f'*/{os.linesep}'

    else:
        raise ValueError(f'Unknown programming language "{language}"')


class DirectoryWorker(Command):

    user_options = [
        ('regex=', 'r', 'regular expression defining the directories to process'),
    ]

    def initialize_options(self):
        """
        Running at the begining of the configuration.
        """
        self.regex = None
        self.directories = None

    def finalize_options(self):
        """
        Running at the end of the configuration.
        """
        if self.regex is None:
            raise Exception('Parameter --regex is missing')

        m = re.compile(self.regex)

        self.directories = list(os.path.join(PWD, d) for d in filter(
            lambda s: os.path.isdir(s) and (m.match(s) is not None), os.listdir(PWD)))

        if len(self.directories) == 0:
            warnings.warn('Empty list of directories', RuntimeWarning)


class ApplyFormatCommand(DirectoryWorker):

    description = 'apply the format to the files in the given directories'

    def run(self):
        """
        Execution of the command action.
        """
        for directory in self.directories:

            # Format the python files
            python_files = python_files_in(directory)
            python_proc = None if not python_files else subprocess.Popen(
                ['autopep8', '-i'] + python_files)

            # Format C files
            c_files = cpp_files_in(directory)
            c_proc = None if not c_files else subprocess.Popen(
                ['clang-format', '-i'] + c_files)

            def killall():
                if python_proc is not None:
                    python_proc.kill()
                if c_proc is not None:
                    c_proc.kill()

            # Wait for the processes to finish
            if python_proc is not None and python_proc.wait() != 0:
                killall()
                raise RuntimeError(
                    'Problems found while formatting python files')

            if c_proc is not None and c_proc.wait() != 0:
                killall()
                raise RuntimeError('Problems found while formatting C files')


class CheckFormatCommand(DirectoryWorker):

    description = 'check the format of the files of a certain type in a given directory'

    def run(self):
        """
        Execution of the command action.
        """
        for directory in self.directories:
            python_files = python_files_in(directory)
            c_files = cpp_files_in(directory)

            # Check python files
            process = subprocess.Popen(['autopep8', '--diff'] + python_files,
                                       stdout=subprocess.PIPE,
                                       stderr=subprocess.PIPE)

            check_format_process('autopep8', directory, process)

            # Check the C files
            for fl in c_files:
                process = subprocess.Popen(['clang-format', fl],
                                           stdout=subprocess.PIPE,
                                           stderr=subprocess.PIPE)
                with open(fl) as f:
                    check_format_process(
                        'clang-format', directory, process, compare=f.read())


class CheckPyFlakesCommand(DirectoryWorker):

    description = 'run pyflakes in order to detect unused objects and errors in the code'

    def run(self):
        """
        Execution of the command action.
        """
        for directory in self.directories:
            python_files = python_files_in(directory)

            process = subprocess.Popen(['pyflakes'] + python_files,
                                       stdout=subprocess.PIPE,
                                       stderr=subprocess.PIPE)

            try:
                check_format_process('pyflakes', directory, process)
            except RuntimeError:
                files = os.linesep.join(python_files)
                raise RuntimeError(
                    f'PyFlakes failed to process files:{os.linesep}{files}')


with tempfile.TemporaryDirectory() as tmp_include_dir:

    # so the "include" statements work properly
    os.mkdir(os.path.join(tmp_include_dir, 'reactions'))

    # modify headers that need to be configured
    for input_filename in files_with_extension(os.path.join(PWD, 'include'), 'hpp.in'):
        output_filename = os.path.join(
            tmp_include_dir, 'reactions', os.path.basename(input_filename[:-3]))
        with open(input_filename) as input_file, open(output_filename, 'wt') as output_file:
            # this is specified at the python level
            output_file.write(re.sub('@[A-Z]*_TABLE@', '', input_file.read()))

    # setup function
    setup(

        name='reactions',

        description='Package to define and handle reactions and decays',

        cmdclass={'apply_format': ApplyFormatCommand,
                  'check_format': CheckFormatCommand,
                  'check_pyflakes': CheckPyFlakesCommand},

        # Read the long description from the README
        long_description=open(os.path.join(PWD, 'README.md')).read(),

        # Keywords to search for the package
        keywords='hep high energy physics database',

        # Set the path to the python package
        packages=['reactions'],
        package_dir={'': 'python'},
        include_package_data=True,

        # Modules
        ext_modules=[Extension('reactions.capi',
                               include_dirs=[tmp_include_dir,
                                             'include', os.path.join(PWD, 'python', 'src')],
                               sources=[os.path.relpath(s, PWD) for s in files_with_extension(
                                   os.path.join(PWD, 'python', 'src'), 'cpp')],
                               extra_compile_args=['-std=c++17'],
                               language='c++')],

        # Python version
        python_requires='>=3.6',

        tests_require=['pytest', 'pytest-runner'],
    )
