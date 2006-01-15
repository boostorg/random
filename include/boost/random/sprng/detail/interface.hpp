/* 
 * Copyright Matthias Troyer 2005
 * Distributed under the Boost Software License, Version 1.0.) (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
*
 */

#include <boost/preprocessor/cat.hpp>

#ifndef BOOST_SPRNG_GENERATOR
#error Please set BOOST_SPRNG_GENERATOR to he name of one of the SPRNG generators before including this header file
#else

// This header file declares the C API functions for the SPRNG generators

extern "C" {

int BOOST_PP_CAT(BOOST_SPRNG_GENERATOR, _get_rn_int) (int *igenptr);
float  BOOST_PP_CAT(BOOST_SPRNG_GENERATOR, _get_rn_flt) (int *igenptr);
double  BOOST_PP_CAT(BOOST_SPRNG_GENERATOR, _get_rn_dbl) (int *igenptr);
int *  BOOST_PP_CAT(BOOST_SPRNG_GENERATOR, _init_rng) (int rng_type,  int gennum, int total_gen,  int seed, int mult);
int  BOOST_PP_CAT(BOOST_SPRNG_GENERATOR, _spawn_rng) (int *igenptr, int nspawned, int ***newgens, int checkid);
int  BOOST_PP_CAT(BOOST_SPRNG_GENERATOR, _get_seed_rng) (int *genptr);
int  BOOST_PP_CAT(BOOST_SPRNG_GENERATOR, _free_rng) (int *genptr);
int  BOOST_PP_CAT(BOOST_SPRNG_GENERATOR, _pack_rng) ( int *genptr, char **buffer);
int * BOOST_PP_CAT(BOOST_SPRNG_GENERATOR, _unpack_rng) ( char const *packed);
int  BOOST_PP_CAT(BOOST_SPRNG_GENERATOR, _print_rng) ( int *igen);

}

#endif 