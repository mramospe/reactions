"""
Setup script for the "reactions" package
"""

import importlib
import inspect
import os
import re
import subprocess
import tempfile
import warnings

from setuptools import Command, Extension, setup, find_packages

PWD = os.path.dirname(__file__)


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


def files_with_extension(ext, where):
    """
    Return all the files with the given extension in the package.
    """
    return [os.path.join(root, f) for root, _, files in os.walk(where) for f in filter(lambda s: s.endswith(f'.{ext}'), files)]


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
            python_files = files_with_extension('py', directory)
            python_proc = None if not python_files else subprocess.Popen(
                ['autopep8', '-i'] + python_files)

            # Format C files
            c_files = files_with_extension(
                'cpp', directory) + files_with_extension('hpp', directory)
            c_proc = None if not c_files else subprocess.Popen(
                ['clang-format', '-i'] + c_files)

            def killall(): return python_proc.kill() and c_proc.kill()

            # Wait for the processes to finish
            if python_proc is not None and python_proc.wait() != 0:
                killall()
                raise RuntimeError(
                    'Problems found while formatting python files')

            if c_proc is not None and c_proc.wait() != 0:
                killall()
                raise RuntimeError('Problems found while formatting C files')


class ApplyCopyrightCommand(DirectoryWorker):

    description = 'add the copyright to the files in the directories'

    def _check_add_copyright(self, path, preamble):
        """
        Check that the given file has the provided copyright and add it in case
        it is not present.
        """
        with open(path) as f:
            data = f.read()

        if not preamble in data:

            if data.startswith('#!'):  # is a script
                lines = data.splitlines(keepends=True)
                text = f'{lines[0]}{preamble}{"".join(lines[1:])}'
            else:
                text = f'{preamble}{data}'

            with open(path, 'wt') as f:
                f.write(text)

    def run(self):
        """
        Execution of the command action.
        """
        for directory in self.directories:
            python_files = files_with_extension('py', directory)
            c_files = files_with_extension(
                'cpp', directory) + files_with_extension('hpp', directory)

            lic = license_for_language('python')
            for pf in python_files:
                self._check_add_copyright(pf, lic)

            lic = license_for_language('cpp')
            for cf in c_files:
                self._check_add_copyright(cf, lic)


class CheckFormatCommand(DirectoryWorker):

    description = 'check the format of the files of a certain type in a given directory'

    def run(self):
        """
        Execution of the command action.
        """
        for directory in self.directories:
            python_files = files_with_extension('py', directory)
            c_files = files_with_extension(
                'cpp', directory) + files_with_extension('hpp', directory)

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
            python_files = files_with_extension('py', directory)

            process = subprocess.Popen(['pyflakes'] + python_files,
                                       stdout=subprocess.PIPE,
                                       stderr=subprocess.PIPE)

            try:
                check_format_process('pyflakes', directory, process)
            except RuntimeError:
                files = os.linesep.join(python_files)
                raise RuntimeError(
                    f'PyFlakes failed to process files:{os.linesep}{files}')


class CheckCopyrightCommand(DirectoryWorker):

    description = 'check that the copyright is present in the directory files'

    def _check_copyright(self, path, preamble):
        """
        Check that the given file has the provided copyright.
        """
        with open(path) as f:
            text = f.read()
            if not text.startswith(preamble):
                # it might be an executable (skip the first line)
                t = ''.join(text.splitlines(keepends=True)[1:])
                if not t.startswith(preamble):
                    raise RuntimeError(
                        f'Copyright not present in file "{path}"')

    def run(self):
        """
        Execution of the command action.
        """
        for directory in self.directories:

            python_files = files_with_extension('py', directory)
            c_files = files_with_extension(
                'cpp', directory) + files_with_extension('hpp', directory)

            lic = license_for_language('python')
            for pf in python_files:
                self._check_copyright(pf, lic)

            lic = license_for_language('cpp')
            for cf in c_files:
                self._check_copyright(cf, lic)


class RemoveCopyrightCommand(DirectoryWorker):

    description = 'remove the copyright from the files in the given directories'

    def _remove_copyright(self, path, preamble):
        """
        Remove the copyright if present in the given file.
        """
        with open(path) as f:
            text = f.read()

        if preamble in text:
            with open(path, 'wt') as f:
                f.write(text.replace(preamble, '', 1))

    def run(self):
        """
        Execution of the command action.
        """
        for directory in self.directories:

            python_files = files_with_extension('py', directory)
            c_files = files_with_extension(
                'cpp', directory) + files_with_extension('hpp', directory)

            lic = license_for_language('python')
            for pf in python_files:
                self._remove_copyright(pf, lic)

            lic = license_for_language('cpp')
            for cf in c_files:
                self._remove_copyright(cf, lic)


class CheckDocumentationCommand(Command):

    description = 'check that all the exposed functions have documentation about them and their arguments'

    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def _get_missing_descriptions(self, obj, from_class=False):
        """
        Get the names of the arguments that are not documented in the provided
        function. If *from_class* is provided, then the object is assumed to
        be a class member.
        """
        s = inspect.getfullargspec(obj)

        missing = []
        if s.args is not None and obj.__doc__:

            if from_class:
                args = s.args[1:]  # first argument is the class
            else:
                args = s.args

            for a in filter(lambda s: not f':param {s}:' in obj.__doc__, args):
                missing.append(a)

        return missing

    def run(self):
        """
        Execution of the command action.
        """
        module = importlib.import_module(
            'reactions')  # import the package even if it is not installed

        missing_args = {}

        for m in module.__all__:  # iterate over exposed objects

            o = getattr(module, m)

            if inspect.isfunction(o):  # check functions
                missing = self._get_missing_descriptions(o)
                if missing:
                    missing_args[m] = missing

            elif inspect.isclass(o):  # check classes

                for n, f in inspect.getmembers(o, predicate=inspect.isfunction):
                    missing = self._get_missing_descriptions(
                        f, from_class=True)
                    if missing:
                        missing_args[f'{o.__name__}.{n}'] = missing

                for n, f in inspect.getmembers(o, predicate=inspect.ismethod):
                    missing = self._get_missing_descriptions(
                        f, from_class=True)
                    if missing:
                        missing_args[f'{o.__name__}.{n}'] = missing

        if missing_args:
            output = os.linesep.join(
                f'- {m}: {s}' for m, s in sorted(missing_args.items()))
            raise RuntimeError(
                f'Missing descriptions in the following functions:{os.linesep}{output}')


with tempfile.TemporaryDirectory() as tmp_include_dir:

    # so the "include" statements work properly
    os.mkdir(os.path.join(tmp_include_dir, 'reactions'))

    # modify headers that need to be configured
    for input_filename in files_with_extension('in', os.path.join(PWD, 'include')):
        output_filename = os.path.join(
            tmp_include_dir, 'reactions', os.path.basename(input_filename[:-3]))
        with open(input_filename) as input_file, open(output_filename, 'wt') as output_file:
            # this is specified at the python level
            output_file.write(re.sub('@PDG_TABLE@', '', input_file.read()))

    # setup function
    setup(

        name='reactions',

        description='Package to define and handle reactions and decays',

        cmdclass={'apply_copyright': ApplyCopyrightCommand,
                  'apply_format': ApplyFormatCommand,
                  'check_copyright': CheckCopyrightCommand,
                  'check_format': CheckFormatCommand,
                  'check_pyflakes': CheckPyFlakesCommand,
                  'remove_copyright': RemoveCopyrightCommand,
                  'check_documentation': CheckDocumentationCommand},

        # Read the long description from the README
        long_description=open(os.path.join(PWD, 'README.md')).read(),

        # Keywords to search for the package
        keywords='hep high energy physics database',

        # Set the path to the python package
        packages=['reactions'],
        package_dir={'': 'python'},
        include_package_data=True,

        # Modules
        ext_modules=[Extension('reactions.reactions',
                               include_dirs=[tmp_include_dir, 'include'],
                               sources=[os.path.relpath(s, PWD) for s in files_with_extension(
                                   'cpp', os.path.join(PWD, 'src'))],
                               extra_compile_args=['-std=c++17'],
                               language='c++')],

        # Python version
        python_requires='>=3.6',

        tests_require=['pytest', 'pytest-runner'],
    )
