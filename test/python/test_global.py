"""
Test global features of the package
"""


def test_package():
    """
    Access the package from python
    """
    import reactions
    members = set(filter(lambda s: not s.startswith('_'), dir(reactions)))
    assert members == set(reactions.__all__)


def test_version():
    """
    Access the version
    """
    from reactions import __version__
    assert bool(__version__)
