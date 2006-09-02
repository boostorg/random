from _boost_random import *

class UnknownVariateGenerator(Exception):
    def __init__(self, engine, dist):
        self.engine = engine
        self.dist = dist

    def __str__(self):
        return 'No variate_generator combination for "%s" and "%s"' \
            % (self.engine.__name__, self.dist.__name__)

def variate_generator(engine, dist):
    try:
        return distribution_variate_map[dist.__class__](engine, distribution)
    except KeyError:
        raise UnknownVariateGenerator(engine.__class__, dist.__class__)

