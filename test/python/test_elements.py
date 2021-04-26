"""
Test element classes
"""
import pytest
import reactions


def test_nubase_element():

    # create an element from the database
    el = reactions.nubase_element('1H')
    assert reactions.node_type(el) == 'element'
    assert el.name == '1H'
    assert el.nubase_id == 1001000

    el = reactions.nubase_element(1001000)
    assert reactions.node_type(el) == 'element'

    # check that empty values in C++ correspond to None in python
    assert reactions.nubase_element('1H').half_life is None
    assert reactions.nubase_element('1H').half_life_error is None
    assert reactions.nubase_element('1H').half_life_from_systematics is None

    # create a custom element
    n0 = reactions.nubase_element(
        '999Un', 999999000, 999, 999, (0., 0., False), False, None, True)
    n1 = reactions.nubase_element(name='999Un', nubase_id=999999000, atomic_number=999, mass_number=999, mass_excess_and_error_with_tag=(
        0., 0., False), half_life_and_error_with_tag=None, is_stable=True, is_ground_state=True)

    assert n0 == n1

    assert str(n0) == n0.__repr__()

    # all these constructors must fail
    with pytest.raises(TypeError):
        reactions.nubase_element(0.0)

    with pytest.raises(RuntimeError):
        reactions.nubase_element('gamma', 1, 22)

    with pytest.raises(RuntimeError):
        reactions.nubase_element('gamma', 1, 22, nubase_id=22)

    for e in reactions.nubase_database.all_elements():
        # we must be able to compute all the LaTeX names
        e.latex_name

    # test LaTeX names
    assert reactions.nubase_element("1H").latex_name == "\\ce{^{1}H}"
    assert reactions.nubase_element("1n").latex_name == "\\ce{^{1}n}"
    assert reactions.nubase_element("7Li(i)").latex_name == "\\ce{^{7}Li^{i}}"

    # errors accessing elements
    with pytest.raises(reactions.LookupError):
        reactions.nubase_element('H')


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
    g0 = reactions.pdg_element(name='gamma', pdg_id=1, three_charge=0, mass_and_errors=(
        0., 0., 0.), width_and_errors=None, is_self_cc=False)

    assert str(g0) == g0.__repr__()

    g1 = reactions.pdg_element(name='gamma', pdg_id=1, three_charge=0,
                               mass_and_errors=None, width_and_errors=None, is_self_cc=True)

    assert str(g1) == 'reactions.pdg_element(name="gamma", pdg_id=1, three_charge=0, mass_and_errors=None, width_and_errors=None, is_self_cc=True)'
    assert str(g1) == g1.__repr__()

    # all these constructors must fail
    with pytest.raises(TypeError):
        reactions.pdg_element(0.0)

    with pytest.raises(RuntimeError):
        reactions.pdg_element('gamma', 1, 22)

    with pytest.raises(RuntimeError):
        reactions.pdg_element('gamma', 1, 22, pdg_id=22)

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
    assert str(el) == 'reactions.string_element(name="custom")'
