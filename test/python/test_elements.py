"""
Test element classes
"""
import pytest
import reactions


def test_pdg_element():

    # create an element from the database
    el = reactions.pdg_element('pi+')
    assert reactions.node_type(el) == 'element'
    assert el.name == 'pi+'
    assert el.pdg_id == 211

    el = reactions.pdg_element(211)
    assert reactions.node_type(el) == 'element'

    # check that empty values in C++ correspond to None in python
    assert reactions.pdg_element('H0').width is None
    assert reactions.pdg_element('H0').width_error_lower is None
    assert reactions.pdg_element('H0').width_error_upper is None
    assert reactions.pdg_element('H0').width_error is None

    # create a custom element
    reactions.pdg_element('gamma', 1, 0, (0., 0., 0.), (0., 1.e+16, 0.), True)
    reactions.pdg_element(name='gamma', pdg_id=1, three_charge=0, mass_and_errors=(
        0., 0., 0.), width_and_errors=(0., 0., 0.), is_self_cc=True)
    reactions.pdg_element(name='gamma', pdg_id=1, three_charge=0,
                          mass_and_errors=None, width_and_errors=(0., 0., 0.), is_self_cc=True)
    reactions.pdg_element(name='gamma', pdg_id=1, three_charge=0, mass_and_errors=(
        0., 0., 0.), width_and_errors=None, is_self_cc=True)
    reactions.pdg_element(name='gamma', pdg_id=1, three_charge=0,
                          mass_and_errors=None, width_and_errors=None, is_self_cc=True)

    # all these constructors must fail
    with pytest.raises(TypeError):
        reactions.pdg_element(0.0)

    with pytest.raises(RuntimeError):
        reactions.pdg_element('gamma', 1, 22)

    with pytest.raises(RuntimeError):
        reactions.pdg_element('gamma', 1, 22, pdg_id=22)

    with pytest.raises(RuntimeError):
        reactions.pdg_element(name='gamma', geant_id=1)

    # test the comparison operators
    assert reactions.pdg_element('pi+') == reactions.pdg_element(+211)
    assert reactions.pdg_element('pi+') != reactions.pdg_element('pi-')
    assert reactions.pdg_element(+211) != reactions.pdg_element(-211)

    # errors accessing elements
    with pytest.raises(reactions.LookupError):
        reactions.pdg_element('mu')

    # test LaTeX names
    assert reactions.pdg_element("K(S)0").latex_name == "K_{S}^{0}"
    assert reactions.pdg_element("K+").latex_name == "K^{+}"
    assert reactions.pdg_element("pi+").latex_name == "\\pi^{+}"
    assert reactions.pdg_element("pi-").latex_name == "\\pi^{-}"
    assert reactions.pdg_element("Lambda").latex_name == "\\Lambda"
    assert reactions.pdg_element("eta'(958)").latex_name == "\\eta^{'}(958)"
    assert reactions.pdg_element("a(0)(980)0").latex_name == "a_{0}(980)^{0}"
    assert reactions.pdg_element("f(2)'(1525)").latex_name == "f_{2}^{'}(1525)"
    assert reactions.pdg_element("Xi(c)'+").latex_name == "\\Xi_{c}^{'+}"
    assert reactions.pdg_element(
        "Delta(1950)~-").latex_name == "\\bar{\\Delta}(1950)^{-}"
    assert reactions.pdg_element(
        "K(2)*(1430)~0").latex_name == "\\bar{K}_{2}^{*}(1430)^{0}"
    assert reactions.pdg_element(
        "D(s2)*(2573)+").latex_name == "D_{s2}^{*}(2573)^{+}"

    for e in reactions.pdg_database.all_elements():
        # we must be able to compute all the LaTeX names
        e.latex_name


def test_string_element():

    el = reactions.string_element('custom')
    assert el.name == 'custom'
    assert reactions.node_type(el) == 'element'
    assert reactions.string_element(
        'custom') == reactions.string_element('custom')
    assert reactions.string_element(
        'custom') != reactions.string_element('other')
