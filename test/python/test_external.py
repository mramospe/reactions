import particle
import reactions


def test_particle():
    for e in reactions.pdg_database.all_elements():
        assert(particle.Particle.from_pdgid(e.pdg_id).name == e.name)
