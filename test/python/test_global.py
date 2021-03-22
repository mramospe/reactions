"""
Test global features of the package
"""


def test_package():
    """
    Access the package from python
    """
    import reactions

    members = set(filter(lambda s: not s.startswith('_'), dir(reactions)))

    exposed_objects = set(['reactions'] + reactions.__all__)

    assert members == exposed_objects


def test_version():
    """
    Access the version
    """
    from reactions import __version__
    assert bool(__version__)
