########################################
# MIT License
#
# Copyright (c) 2020 Miguel Ramos Pernas
########################################
# -*- coding: utf-8 -*-
#
# reactions documentation build configuration file, created by
# sphinx-quickstart on Fri Dec  8 18:24:26 2017.
#
# This file is execfile()d with the current directory set to its
# containing dir.
#
# Note that not all possible configuration values are present in this
# autogenerated file.
#
# All configuration values have a default; values that are commented out
# serve to show the default.

import os
import re
import reactions
import shutil
import subprocess
import sys
import tempfile
import time

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
sys.path.insert(0, os.path.dirname(os.path.abspath(reactions.__file__)))

# Generate the doxygen files for the C++ documentation. The Doxyfile in the
# root directory is copied to a temporary location and the version and output
# directories are modified according to the needs of Sphinx.
with tempfile.TemporaryDirectory() as tmpdir:

    # Install the C++ static files
    root = os.path.dirname(os.path.dirname(os.path.dirname(__file__)))

    ori_doxyfile = os.path.join(root, 'docs', 'Doxyfile')
    tmp_doxyfile = os.path.join(tmpdir, 'Doxyfile')

    shutil.copyfile(ori_doxyfile, tmp_doxyfile)

    auxiliar_dir = os.path.join(root, 'docs', 'source', 'auxiliar')

    intro_file = os.path.join(auxiliar_dir, 'CPP_INTRODUCTION.md')

    static_doc_dir = os.path.join(os.path.dirname(__file__), '_static')
    cpp_doc_dir = os.path.join(static_doc_dir, 'cpp')

    with open(tmp_doxyfile, 'at') as df:
        df.write(f'''
TAGFILES = {os.path.join(static_doc_dir, "cppreference-doxygen-web.tag.xml=http://en.cppreference.com/w/")}
INPUT = {intro_file} {os.path.join(root, "include")}
USE_MDFILE_AS_MAINPAGE = {intro_file}
PROJECT_NUMBER = {reactions.__version__}
OUTPUT_DIRECTORY = {cpp_doc_dir}
''')
    subprocess.check_call(['doxygen'], cwd=tmpdir)

    # Generate additional documents needed by the documentation, like tables, plots, ...
    auxiliar_tmp_dir = os.path.join(auxiliar_dir, 'tmp')

    os.makedirs(auxiliar_tmp_dir, exist_ok=True)

    # If this is executed by ReadTheDocs (READTHEDOCS_VERSION is set), the changelog
    # from the current version is used. This also works for the "stable" version. For
    # other builds, like "latest", no changelog is used. If we are running a local
    # build, the changelog is generated locally.
    tmp_changelog = os.path.join(tmpdir, 'changelog.md')

    version_tag = os.environ.get(
        'READTHEDOCS_VERSION', None)  # set by ReadTheDocs

    print(f'ReadTheDocs build version: {version_tag}')

    if version_tag and re.compile('^(v[0-9]*\.[0-9]*\.[0-9]|stable)$').match(version_tag):

        if version_tag != 'stable' and version_tag != reactions.__version__:
            raise RuntimeError(
                'Tag version is different from the package version')

        timeout = 3600  # 1 hour
        start = time.time()
        while time.time() - start < timeout:
            sc = subprocess.call(
                ['wget', f'https://github.com/mramospe/reactions/releases/download/v{reactions.__version__}/v{reactions.__version__}-full-changelog.md', '-O', tmp_changelog])
            if sc == 0:
                break
            else:
                time.sleep(30)  # wait 30 seconds

        if sc != 0:
            raise RuntimeError('Missing full catalog in tagged build')
        else:
            subprocess.check_call(['pandoc', tmp_changelog, '-o',
                                   os.path.join(auxiliar_tmp_dir, 'changelog.rst')])
    elif version_tag is None:
        # a local build
        subprocess.check_call(['bash', 'repository', 'changelog', '-o', tmp_changelog,
                               '--include-tags-regex', '^v[0-9]*\.[0-9]*\.[0-9]$', '--since-tag', 'v0.0.0'], cwd=root)
        subprocess.check_call(['pandoc', tmp_changelog, '-o',
                               os.path.join(auxiliar_tmp_dir, 'changelog.rst')])

    subprocess.check_call(['python', os.path.join(
        root, 'scripts', 'display-table.py'), 'pdg', '--output', os.path.join(static_doc_dir, 'pdg_table.pdf')])

    subprocess.check_call(['python', os.path.join(
        root, 'scripts', 'display-table.py'), 'nubase', '--output', os.path.join(static_doc_dir, 'nubase_table.pdf')])

# -- General configuration ------------------------------------------------

# If your documentation needs a minimal Sphinx version, state it here.
#
# needs_sphinx = '1.0'

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'sphinx.ext.intersphinx',
    'sphinx.ext.coverage',
    'sphinx.ext.viewcode',
    'sphinx_automodapi.automodapi',
    'sphinx_automodapi.smart_resolver',
    'sphinx_rtd_theme',
    'nbsphinx',
    'sphinx.ext.napoleon'
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# The suffix(es) of source filenames.
# You can specify multiple suffix as a list of string:
#
# source_suffix = ['.rst', '.md']
source_suffix = ['.rst', '.ipynb']

# The master toctree document.
master_doc = 'index'

# General information about the project.
project = u'Reactions'
copyright = u'2021, Miguel Ramos Pernas'
author = u'Miguel Ramos Pernas'

# The version info for the project you're documenting, acts as replacement for
# |version| and |release|, also used in various other places throughout the
# built documents.
#
# The short X.Y version.
version = reactions.__version__
# The full version, including alpha/beta/rc tags.
release = reactions.__version__

# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
#
# This is also used if you do content translation via gettext catalogs.
# Usually you set "language" from the command line for these cases.
language = None

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This patterns also effect to html_static_path and html_extra_path
exclude_patterns = ['notebooks/.ipynb_checkpoints/*']

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = 'sphinx'

# If true, `todo` and `todoList` produce output, else they produce nothing.
todo_include_todos = False


# -- Options for HTML output ----------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'

# Theme options are theme-specific and customize the look and feel of a theme
# further.  For a list of options available for each theme, see the
# documentation.
#
# html_theme_options = {}

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

# Custom sidebar templates, must be a dictionary that maps document names
# to template names.
#
# This is required for the alabaster theme
# refs: http://alabaster.readthedocs.io/en/latest/installation.html#sidebars
html_sidebars = {
    '**': [
        'about.html',
        'navigation.html',
        'relations.html',  # needs 'show_related': True theme option to display
        'searchbox.html',
        'donate.html',
    ]
}


# -- Options for auto-documentation ---------------------------------------

autoclass_content = 'init'


# -- Options for HTMLHelp output ------------------------------------------

# Output file base name for HTML help builder.
htmlhelp_basename = 'reactionsdoc'


# -- Options for LaTeX output ---------------------------------------------

latex_elements = {
    # The paper size ('letterpaper' or 'a4paper').
    #
    # 'papersize': 'letterpaper',

    # The font size ('10pt', '11pt' or '12pt').
    #
    # 'pointsize': '10pt',

    # Additional stuff for the LaTeX preamble.
    #
    # 'preamble': '',

    # Latex figure (float) alignment
    #
    # 'figure_align': 'htbp',
}

# Grouping the document tree into LaTeX files. List of tuples
# (source start file, target name, title,
#  author, documentclass [howto, manual, or own class]).
latex_documents = [
    (master_doc, 'reactions.tex', u'reactions Documentation',
     u'Miguel Ramos Pernas', 'manual'),
]


# -- Options for manual page output ---------------------------------------

# One entry per manual page. List of tuples
# (source start file, name, description, authors, manual section).
man_pages = [
    (master_doc, 'reactions', u'Reactions Documentation',
     [author], 1)
]


# -- Options for Texinfo output -------------------------------------------

# Grouping the document tree into Texinfo files. List of tuples
# (source start file, target name, title, author,
#  dir menu entry, description, category)
texinfo_documents = [
    (master_doc, 'reactions', u'Reactions Documentation',
     author, 'reactions', 'One line description of project.',
     'Miscellaneous'),
]


# Example configuration for intersphinx: refer to the Python standard library.
intersphinx_mapping = {
    'https://docs.python.org/': None,
}
