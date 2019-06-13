# random
Boost.org random module - with MIXMAX RNG

The fork exists to develop and contribute the MIXMAX Random Number Generator to Boost.
It is the leading RNG, in terms of its theoretical guarantees regarding uniformness and statistical
equidistribution of its double-precision floating-point output.

If you are reading this because you want to use the cutting edge capabilities of
MIXMAX, you need to decide whether to use the boost version whose featured are equivalent 
to those required by the ISO-2011 C++ standard 
or to use the standalone distribution of MIXMAX which has some additional powerful features.
The latest standalone distribution is at http://mixmax.hepforge.org. Its features are well-tested, mature and stable.
There are both C and C++ versions available there. The C++ interface is compatible with C++11
way of doing distributions by plugging into a RNG engine, and also provides a native method for producing uniform random numbers on \[0,1). I am available on a consulting basis to integrate further cutting-edge features into your Monte-Carlo application typical of particle/nuclear physics and chemistry, such as gaussian distributed numbers, unit 3d random-direction vectors, and AVX vectorized implementations of the above. Typically, this is capable of giving significant boosts to your final application.

The MIXMAX RNG was proposed in the paper by my parents in 1986:
   G.K.Savvidy and N.G.Ter-Arutyunian,
   "On the Monte Carlo simulation of physical systems",
   J.Comput.Phys. 97, 566 (1991);
   Preprint EPI-865-16-86, Yerevan, Jan. 1986, 
   http://dx.doi.org/10.1016/0021-9991(91)90015-D

This implementation is based on further evolution of the theory and algorithms in my paper:
   K.Savvidy, 
   "The MIXMAX random number generator", 
   Comp. Phys. Commun. 196 (2015), pp 161–165, 
   http://dx.doi.org/10.1016/j.cpc.2015.06.003

and
   K.Savvidy and G.Savvidy, 
   "Spectrum and Entropy of C-systems. MIXMAX random number generator", 
   Chaos, Solitons & Fractals, Volume 91, (2016) pp. 33–38, 
   http://dx.doi.org/10.1016/j.chaos.2016.05.003
