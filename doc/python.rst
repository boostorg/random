============================
Boost.Random Python Bindings
============================

|(logo)|__

.. |(logo)| image:: ../../../../boost.png
   :alt: Boost

__ ../../../../index.htm

:Author: Daniel Wallin 
:Contact: daniel@boost-consulting.com 
:Organization: `Boost Consulting`_ 
:Copyright: Copyright Daniel Wallin 2006. Distributed
            under the Boost Software License, Version 1.0. 
            (See accompanying file LICENSE_1_0.txt or copy 
            at http://www.boost.org/LICENSE_1_0.txt)

.. _`Boost Consulting`: http://www.boost-consulting.com

Introduction
============

This is a Python binding for all random number generators and distributions
provided in |Boost.Random|.

.. |Boost.Random| replace:: `Boost.Random`_

Random Number Generators
========================

All RNG's provided in this Python binding produce double floating point
numbers in the open-closed range ``[0,1)``.

They also all implement this basic interface:

.. parsed-literal::

  class *random-number-generator*:
    def __init__(self)
    def seed(self)
    def reset(self)


minstd_rand0
---------------

:Documented at: http://www.boost.org/libs/random/random-generators.html#minstd_rand
:Additional members: 

  .. parsed-literal::

    def __init__(self, x0)
    def seed(self, x0)


minstd_rand
--------------

:Documented at: http://www.boost.org/libs/random/random-generators.html#minstd_rand
:Additional members: 

  .. parsed-literal::

    def __init__(self, x0)
    def seed(self, x0)


rand48
---------

:Documented at: http://www.boost.org/libs/random/random-generators.html#rand48
:Additional members: 

  .. parsed-literal::

    def __init__(self, x0)
    def seed(self, x0)

    def __init__(self, x0_64)
    def seed(self, x0_64)

  Note that to call the second set of overloads you need to explicitly name
  the argument:

  .. parsed-literal::

    rand48(1) *# Calls the first overload*
    rand48(x0_64 = 1) *# Calls the second overload*

ecuyer1988
-------------

:Documented at: http://www.boost.org/libs/random/random-generators.html#ecuyer1988
:Additional members: 

  .. parsed-literal::

    def __init__(self, x0, x1)
    def seed(self, x0, x1)


kreutzer1986
---------------

:Documented at: http://www.boost.org/libs/random/random-generators.html#kreutzer1986
:Additional members: 

  .. parsed-literal::

    def __init__(self, s)
    def seed(self, s)


hellekalek1995
-----------------

:Documented at: http://www.boost.org/libs/random/random-generators.html#hellekalek1995
:Additional members: 

  .. parsed-literal::

    def __init__(self, y0)
    def seed(self, y0)


mt11213b
-----------

:Documented at: http://www.boost.org/libs/random/random-generators.html#mt11213b
:Additional members: 

  .. parsed-literal::

    def __init__(self, x0)
    def seed(self, x0)


mt19937
----------

:Documented at: http://www.boost.org/libs/random/random-generators.html#mt19937
:Additional members: 

  .. parsed-literal::

    def __init__(self, x0)
    def seed(self, x0)


lagged_fibonacci607
----------------------

:Documented at: http://www.boost.org/libs/random/random-generators.html#lagged_fibonacci
:Additional members: 

  .. parsed-literal::

    def __init__(self, value)
    def seed(self, value)


lagged_fibonacci1279
-----------------------

:Documented at: http://www.boost.org/libs/random/random-generators.html#lagged_fibonacci
:Additional members: 

  .. parsed-literal::

    def __init__(self, value)
    def seed(self, value)


lagged_fibonacci2281
-----------------------

:Documented at: http://www.boost.org/libs/random/random-generators.html#lagged_fibonacci
:Additional members: 

  .. parsed-literal::

    def __init__(self, value)
    def seed(self, value)


lagged_fibonacci3217
-----------------------

:Documented at: http://www.boost.org/libs/random/random-generators.html#lagged_fibonacci
:Additional members: 

  .. parsed-literal::

    def __init__(self, value)
    def seed(self, value)


lagged_fibonacci4423
-----------------------

:Documented at: http://www.boost.org/libs/random/random-generators.html#lagged_fibonacci
:Additional members: 

  .. parsed-literal::

    def __init__(self, value)
    def seed(self, value)


lagged_fibonacci9689
-----------------------

:Documented at: http://www.boost.org/libs/random/random-generators.html#lagged_fibonacci
:Additional members: 

  .. parsed-literal::

    def __init__(self, value)
    def seed(self, value)


lagged_fibonacci19937
------------------------

:Documented at: http://www.boost.org/libs/random/random-generators.html#lagged_fibonacci
:Additional members: 

  .. parsed-literal::

    def __init__(self, value)
    def seed(self, value)


lagged_fibonacci23209
------------------------

:Documented at: http://www.boost.org/libs/random/random-generators.html#lagged_fibonacci
:Additional members: 

  .. parsed-literal::

    def __init__(self, value)
    def seed(self, value)


lagged_fibonacci44497
------------------------

:Documented at: http://www.boost.org/libs/random/random-generators.html#lagged_fibonacci
:Additional members: 

  .. parsed-literal::

    def __init__(self, value)
    def seed(self, value)


.. SPRNG classes

cmrg
-------

:Documented at: ...
:Additional members: 

  .. parsed-literal::

    def __init__(self, total_streams=1, stream_number=0, global_seed=0, parameter=0)
    def seed(self, total_streams=1, stream_number=0, global_seed=0, parameter=0)


lcg
------

:Documented at: ...
:Additional members: 

  .. parsed-literal::

    def __init__(self, total_streams=1, stream_number=0, global_seed=0, parameter=0)
    def seed(self, total_streams=1, stream_number=0, global_seed=0, parameter=0)

lcg64
--------

:Documented at: ...
:Additional members: 

  .. parsed-literal::

    def __init__(self, total_streams=1, stream_number=0, global_seed=0, parameter=0)
    def seed(self, total_streams=1, stream_number=0, global_seed=0, parameter=0)

lfg
------

:Documented at: ...
:Additional members: 

  .. parsed-literal::

    def __init__(self, total_streams=1, stream_number=0, global_seed=0, parameter=0)
    def seed(self, total_streams=1, stream_number=0, global_seed=0, parameter=0)

mlfg
-------

:Documented at: ...
:Additional members: 

  .. parsed-literal::

    def __init__(self, total_streams=1, stream_number=0, global_seed=0, parameter=0)
    def seed(self, total_streams=1, stream_number=0, global_seed=0, parameter=0)

pmlcg
--------

:Documented at: ...
:Additional members: 

  .. parsed-literal::

    def __init__(self, total_streams=1, stream_number=0, global_seed=0, parameter=0)
    def seed(self, total_streams=1, stream_number=0, global_seed=0, parameter=0)


Distributions
=============

All distributions implement this basic interface:

.. parsed-literal::

  class *distribution*:
    def reset(self)
    def __call__(self, rng)

Where ``rng`` is one of the above mentioned Random Number Generators.


uniform_int
-----------

:Documented at: http://www.boost.org/libs/random/random-distributions.html#uniform_int
:Additional members:

  .. parsed-literal::

    def __init__(self, min=0, max=9)
    def min(self)
    def max(self)


bernoulli_distribution
----------------------

:Documented at: http://www.boost.org/libs/random/random-distributions.html#bernoulli_distribution
:Additional members:

  .. parsed-literal::

    def __init__(self, p=0.5)
    def p(self)

geometric_distribution
----------------------

:Documented at: http://www.boost.org/libs/random/random-distributions.html#geometric_distribution
:Additional members:

  .. parsed-literal::

    def __init__(self, p=0.5)
    def p(self)

triangle_distribution
---------------------

:Documented at: http://www.boost.org/libs/random/random-distributions.html#triangle_distribution
:Additional members:

  .. parsed-literal::

    def __init__(self, a, b, c)
    def a(self)
    def b(self)
    def c(self)

exponential_distribution
------------------------

:Documented at: http://www.boost.org/libs/random/random-distributions.html#exponential_distribution
:Additional members:

  .. parsed-literal::

    def __init__(self, lambda\_)
    def lambda_(self)


normal_distribution
-------------------

:Documented at: http://www.boost.org/libs/random/random-distributions.html#normal_distribution
:Additional members:

  .. parsed-literal::

    def __init__(self, mean=0, sigma=1)
    def mean(self)
    def sigma(self)

lognormal_distribution
----------------------

:Documented at: http://www.boost.org/libs/random/random-distributions.html#lognormal_distribution
:Additional members:

  .. parsed-literal::

    def __init__(self, mean=1, sigma=1)
    def mean(self)
    def sigma(self)


Variate Generator
=================

``boost::variate_generator<>`` is exported for all distributions provided in
this library, where ``Engine`` is ``boost::buffered_uniform<>&``. The variate
generator classes are named according to the pattern ``<distribution>_variate``::

  uniform_int_variate
  bernoulli_distribution_variate
  geometric_distribution_variate
  ...

A convenience function ``variate_generator()`` is supplied that automatically
chooses the right variate-generator class depending on the distribution passed
to it::

  d = uniform_int(1,6)
  r = mt11213b()
  v = variate_generator(r, d)


Interfacing with C++
====================

The Random Number Generators are all derived from 
the abstract ``boost::buffered_uniform``. Exporting a function
that accepts any Random Number Generator can be done like this:

.. parsed-literal::

  void process(*boost::buffered_uniform<>*\ & rng)
  {
      *... use rng ...*
  }

  BOOST_PYTHON_MODULE(my_module)
  {
      def("process", process);
  }

