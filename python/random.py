from _random import *

class variate_generator:
    def __init__(self, rng, distribution):
        self.base = distribution_variate_map[distribution.__class__](rng, distribution)

    def __getattr__(self, key):
        return getattr(self.base, key)

