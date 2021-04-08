"""
Test composite classes
"""
import pytest
import reactions


def test_reaction():

    reac = reactions.reaction('pi+ -> mu+ nu(mu)')

    # check sizes
    assert len(reac.reactants) == 1
    assert len(reac.products) == 2

    # check types
    assert reactions.node_type(reac) == 'reaction'
    for e in reac.reactants + reac.products:
        assert reactions.node_type(e) == 'element'

    # more complex reactions
    reactions.reaction('p+ p~- -> {pi+ -> mu+ nu(mu)} pi-')

    # we can create empty reactions
    reac = reactions.reaction(kind='pdg')

    with pytest.raises(ValueError):
        reactions.reaction(kind='nothing')

    # we can append new elements
    reac.reactants.append(reactions.pdg_element('pi+'))
    reac.products.append(reactions.pdg_element('mu+'))
    reac.products.append(reactions.pdg_element('nu(mu)'))

    # comparison operators
    assert reac == reac

    with pytest.raises(TypeError):
        # different element kinds
        reac != reactions.reaction('pi- -> mu- nu(mu)~')

    reac != reactions.reaction('pi- -> mu- nu(mu)~', kind='pdg')

    # order does not matter
    assert reac == reactions.reaction('pi+ -> nu(mu)~ mu-', kind='pdg')
    reac = reactions.reaction('p+ p~- -> {pi+ -> mu+ nu(mu)} pi-')
    assert reac == reactions.reaction('p~- p+ -> pi- {pi+ -> nu(mu) mu+}')


def test_decay():

    reac = reactions.decay('pi+ -> mu+ nu(mu)')

    # check products size
    assert len(reac.products) == 2

    # check types
    assert reactions.node_type(reac) == 'decay'
    for e in reac.products:
        assert reactions.node_type(e) == 'element'

    # more complex reactions
    reactions.decay('p+ -> {pi+ -> mu+ nu(mu)} pi-')

    # we can create empty reactions
    reac = reactions.decay(kind='pdg')

    with pytest.raises(ValueError):
        reactions.decay(kind='nothing')

    # we can append new elements
    reac.head = reactions.pdg_element('pi+')
    reac.products.append(reactions.pdg_element('mu+'))
    reac.products.append(reactions.pdg_element('nu(mu)'))

    # comparison operators
    assert reac == reac

    with pytest.raises(TypeError):
        # different element kinds
        reac != reactions.decay('pi- -> mu- nu(mu)~')

    assert reac != reactions.decay('pi- -> mu- nu(mu)~', kind='pdg')

    # order does not matter
    assert reac == reactions.decay('pi+ -> nu(mu)~ mu-', kind='pdg')
    reac = reactions.decay('K(S)0 -> {pi+ -> mu+ nu(mu)} pi-')
    assert reac == reactions.decay('K(S)0 -> pi- {pi+ -> nu(mu) mu+}')


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
