Reactions
=========

[![Build Status](https://travis-ci.org/mramospe/reactions.svg?branch=master)](https://travis-ci.org/mramospe/reactions)
[![Coverage](https://codecov.io/gh/mramospe/reactions/branch/master/graph/badge.svg)](https://codecov.io/gh/mramospe/reactions)
[![Documentation (Python)](https://img.shields.io/badge/python_documentation-link-blue.svg)](https://reactions.readthedocs.io/en/latest)
[![Documentation (C++)](https://img.shields.io/badge/c++_documentation-link-blue.svg)](https://reactions.readthedocs.io/en/latest/_static/cpp)

This package provides functionalities to define and handle reactions and decays.
Elements involved in these processes can be customized given a user-defined class and data-base.
A bulitin implementation of the [PDG](https://pdg.lbl.gov) database of particles and nuclei
is included.

The Reactions package allows to define a work with trees of processes among different
elements.
These processes can be either a reaction, where one or more reactants generate a set
of products; or a decay, where a single element generates a set of products.
This package is written in `C++` as a header-only library, and has a `python`
interface.
The usage is very similar in both of them.

This package is distributed as a header-only library for `C++`  and pip-installable package in python.
There is no need to install any external software.
