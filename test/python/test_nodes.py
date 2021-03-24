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
    el = reactions.pdg_element(211)
    assert reactions.node_type(el) == 'element'

    # create a custom element
    reactions.pdg_element('gamma', 1, 22, 0.0, 0.00000000,
                          1.000000e+16, 'gamma', 22, 0.00000000)
    reactions.pdg_element(name='gamma', geant_id=1, pdg_id=22, charge=0.0, mass=0.00000000,
                          tau=1.000000e+16, evtgen_name='gamma', pythia_id=22, max_width=0.00000000)

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
