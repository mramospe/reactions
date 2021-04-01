"""
Test composite classes
"""
import pytest
import reactions


def test_node_type():

    assert reactions.node_type(reactions.string_element('A')) == 'element'
    assert reactions.node_type(reactions.pdg_element('KS0')) == 'element'
    assert reactions.node_type(reactions.reaction()) == 'reaction'
    assert reactions.node_type(reactions.decay()) == 'decay'


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

    # test the "is_element" function
    assert reactions.is_element(el)

    assert not reactions.is_element(0.)

    # test the comparison operators
    assert reactions.pdg_element('pi+') == reactions.pdg_element(+211)
    assert reactions.pdg_element('pi+') != reactions.pdg_element('pi-')
    assert reactions.pdg_element(+211) != reactions.pdg_element(-211)

    # errors accessing elements
    with pytest.raises(reactions.LookupError):
        reactions.pdg_element('mu')

    # test LaTeX names
    assert reactions.pdg_element("KS0").latex_name == "K_{S}^{0}"
    assert reactions.pdg_element("K+").latex_name == "K^{+}"
    assert reactions.pdg_element("pi+").latex_name == "\\pi^{+}"
    assert reactions.pdg_element("pi-").latex_name == "\\pi^{-}"
    assert reactions.pdg_element("Lambda").latex_name == "\\Lambda"
    assert reactions.pdg_element("eta'(958)").latex_name == "\\eta^{'}(958)"
    assert reactions.pdg_element("a_0(980)0").latex_name == "a_{0}(980)^{0}"
    assert reactions.pdg_element("f'_2(1525)").latex_name == "f^{'}_{2}(1525)"
    assert reactions.pdg_element(
        "K_2(1430)*~0").latex_name == "\\bar{K}_{2}(1430)^{*0}"
    assert reactions.pdg_element(
        "D_s2(2573)*+").latex_name == "D_{s2}(2573)^{*+}"


def test_string_element():

    el = reactions.string_element('custom')
    assert el.name == 'custom'
    assert reactions.node_type(el) == 'element'
    assert reactions.string_element(
        'custom') == reactions.string_element('custom')
    assert reactions.string_element(
        'custom') != reactions.string_element('other')


def test_reaction():

    reac = reactions.reaction('pi+ -> mu+ nu_mu')

    # check sizes
    assert len(reac.reactants) == 1
    assert len(reac.products) == 2

    # check types
    assert reactions.node_type(reac) == 'reaction'
    for e in reac.reactants + reac.products:
        assert reactions.node_type(e) == 'element'

    # more complex reactions
    reactions.reaction('p+ p~- -> {pi+ -> mu+ nu_mu} pi-')

    # we can create empty reactions
    reac = reactions.reaction(kind='pdg')

    with pytest.raises(ValueError):
        reactions.reaction(kind='nothing')

    # we can append new elements
    reac.reactants.append(reactions.pdg_element('pi+'))
    reac.products.append(reactions.pdg_element('mu+'))
    reac.products.append(reactions.pdg_element('nu_mu'))

    # comparison operators
    assert reac == reac

    with pytest.raises(TypeError):
        # different element kinds
        reac != reactions.reaction('pi- -> mu- nu_mu~')

    reac != reactions.reaction('pi- -> mu- nu_mu~', kind='pdg')

    # order does not matter
    assert reac == reactions.reaction('pi+ -> nu_mu~ mu-', kind='pdg')
    reac = reactions.reaction('p+ p~- -> {pi+ -> mu+ nu_mu} pi-')
    assert reac == reactions.reaction('p~- p+ -> pi- {pi+ -> nu_mu mu+}')


def test_decay():

    reac = reactions.decay('pi+ -> mu+ nu_mu')

    # check products size
    assert len(reac.products) == 2

    # check types
    assert reactions.node_type(reac) == 'decay'
    for e in reac.products:
        assert reactions.node_type(e) == 'element'

    # more complex reactions
    reactions.decay('p+ -> {pi+ -> mu+ nu_mu} pi-')

    # we can create empty reactions
    reac = reactions.decay(kind='pdg')

    with pytest.raises(ValueError):
        reactions.decay(kind='nothing')

    # we can append new elements
    reac.head = reactions.pdg_element('pi+')
    reac.products.append(reactions.pdg_element('mu+'))
    reac.products.append(reactions.pdg_element('nu_mu'))

    # comparison operators
    assert reac == reac

    with pytest.raises(TypeError):
        # different element kinds
        reac != reactions.decay('pi- -> mu- nu_mu~')

    assert reac != reactions.decay('pi- -> mu- nu_mu~', kind='pdg')

    # order does not matter
    assert reac == reactions.decay('pi+ -> nu_mu~ mu-', kind='pdg')
    reac = reactions.decay('KS0 -> {pi+ -> mu+ nu_mu} pi-')
    assert reac == reactions.decay('KS0 -> pi- {pi+ -> nu_mu mu+}')


def test_syntax():

    for proc in reactions.reaction, reactions.decay:

        with pytest.raises(RuntimeError):
            proc('A ->')

        with pytest.raises(RuntimeError):
            proc('-> B C')

        with pytest.raises(RuntimeError):
            proc('{A -> B C')

        with pytest.raises(RuntimeError):
            proc('A -> B C}')

        with pytest.raises(RuntimeError):
            proc('A -> -> B C')

        with pytest.raises(RuntimeError):
            proc('A -> B {C')

    with pytest.raises(RuntimeError):
        reactions.decay('A B -> C D')

    reactions.reaction('A -> B {C D -> E} F')
    with pytest.raises(RuntimeError):
        reactions.decay('A -> B {C D -> E} F')
