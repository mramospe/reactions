#!/bin/env python
"""
Build the CMake project depending on Reactions and check that the version of
the python and CMake parts coincide.

Since the python versions can contain development flags (e.g. v1.0.2dev0)
only version, revision and patch numbers are checked.
"""
import os
import re
import subprocess
import sys

if __name__ == '__main__':

    pwd = os.path.dirname(os.path.abspath(__file__))
    root = os.path.dirname(os.path.dirname(
        os.path.dirname(os.path.abspath(__file__))))
    env = dict(os.environ)

    env['CMAKE_PREFIX_PATH'] = ':'.join([
        env.get('CMAKE_PREFIX_PATH', ''),
        os.path.join(root, 'build')
    ])

    build_dir = os.path.join(pwd, 'build')

    print('Create build directory')
    os.makedirs(build_dir, exist_ok=True)

    print('Invoking cmake')
    cmake_output = subprocess.check_output(['cmake', '..', f'-DCMAKE_CXX_COMPILER={env["CXX"]}'],
                                           cwd=build_dir,
                                           env=env,
                                           stderr=subprocess.STDOUT,
                                           encoding='utf-8')

    # check the CMake version
    sys.path.append(os.path.join(root, 'python'))
    import reactions

    version_line_finder = re.compile('^-- Found.*')
    version_finder = re.compile('[0-9]*\.[0-9]*\.[0-9]*')

    for line in cmake_output.split(os.linesep):
        if version_line_finder.match(line):
            cmake_version = version_finder.search(line)
            if not cmake_version:
                raise LookupError('Unable to extract version for CMake')
            if not reactions.__version__.startswith(cmake_version.group()):
                raise RuntimeError(
                    f'Python and CMake versions differ: python={reactions.__version__}, cmake={cmake_version.group()}')
            break

    print('Invoking make')
    subprocess.check_call(['make'], cwd=build_dir, env=env,
                          stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    print('Run application')
    subprocess.check_call(['./main'], cwd=build_dir, env=env)
    print('Success!')
