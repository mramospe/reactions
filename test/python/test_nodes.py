"""
Test nodes
"""
import reactions


def test_node_type():
    assert reactions.node_type(reactions.string_element('A')) == 'element'
    assert reactions.node_type(reactions.pdg_element('K(S)0')) == 'element'
    assert reactions.node_type(reactions.reaction()) == 'reaction'
    assert reactions.node_type(reactions.decay()) == 'decay'


def test_is_element():
    assert reactions.is_element(reactions.string_element('A'))
    assert reactions.is_element(reactions.pdg_element('K(S)0'))
    assert not reactions.is_element(0.)
