# random
Boost.org random module - with MIXMAX RNG
Not in usable state yet!

The fork exists to develop and contribute the MIXMAX Random Number Generator to Boost.
It is the leading RNG, in terms of its theoretical guarantees regarding uniformness and statistical
equidistribution of its double-precision floating-point output.

If you are reading this because you want to use MIXMAX here and now, please go to http://mixmax.hepforge.org
and get the latest standalone distribution of mixmax. Its features are well-tested, mature and stable.
There are both C and C++ versions available there. The C++ interface is compatible with C++11
way of doing distributions by plugging into a RNG engine.

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
