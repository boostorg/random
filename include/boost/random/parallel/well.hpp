/* 
 * Copyright Brigitte Surer and Matthias Troyer 2006
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
*
 */


#ifndef BOOST_RANDOM_PARALLEL_WELL_HPP
#define BOOST_RANDOM_PARALLEL_WELL_HPP

#include <boost/random/parallel/keyword.hpp>
#include <boost/random/parallel/detail/seed_macros.hpp>
#include <boost/config.hpp>
#include <boost/limits.hpp>
#include <boost/static_assert.hpp>
#include <boost/integer_traits.hpp>
#include <boost/cstdint.hpp>
#include <boost/random.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/parameter/macros.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/assert.hpp>

#include <iostream>
#include <stdexcept>
#include <cassert>


namespace boost { namespace random { namespace parallel {

template<int shift>
class mat0pos
{
    public: 
        template<class UIntType>
        static UIntType f(UIntType  v)
        {
            return (v^(v>>shift)); 
        }
};

template<int shift>
class mat0neg
{
    public: 
        template<class UIntType>
        static UIntType f(UIntType  v)
        {
            return v^(v <<(-(shift)));
        }
};

template<int shift>
class maF3neg
{
    public: 
        template<class UIntType>
        static UIntType f(UIntType  v)
        {
            return (v<<(-(shift)));
        }
};

template<int shift>
class maF4neg
{
    public: 
        template<class UIntType>
        static UIntType f(UIntType  v)
        {
              // always the same mask?
            return (v ^ ((v<<(-(shift))) & 0xda442d24U)); 
        }
};

class identity
{
    public: 
        template<class UIntType>
        static UIntType f(UIntType  v)
        {
            return v;        
        }
};

class zero
{
    public: 
        template<class UIntType>
        static UIntType f(UIntType  v)
        {
            return 0;        
        }
};

template<class UIntType, int statesize, UIntType val, class F1, class F2
       , class F3, class F4, class F5, class F6, class F7, class F8
       , int p1,  int p2,  int p3, UIntType mask, class RNG>
class well
{
    public:
        typedef UIntType result_type;

        BOOST_STATIC_CONSTANT(result_type, min_value = 0);
        BOOST_STATIC_CONSTANT(result_type, max_value);
        BOOST_STATIC_CONSTANT(bool, has_fixed_range = true);
	
	// forward seeding functions with iterator buffers to named versions
  BOOST_RANDOM_PARALLEL_ITERATOR_SEED()
  {
    unsigned int num = p[stream_number|0u];
    unsigned int total=p[total_streams|1u];
	BOOST_ASSERT(num < total);
  
	p[first] += num*statesize; 						  
	unsigned int j;						  			  
	for(j = 0; j < statesize && p[first] != p[last]; ++j, ++p[first])  
      state[j] = *p[first];
    if(p[first] == p[last] && j < statesize*total)			  
      throw std::invalid_argument("well::seed");			  
    state_i = 0;
  }

  
// forwarding named seeding functions
  BOOST_RANDOM_PARALLEL_SEED(well)
  {
    unsigned int stream = p[stream_number|0u];
    unsigned int num_stream=p[total_streams|1u];
    unsigned int s=p[global_seed|5489u];
    BOOST_ASSERT(stream < num_stream);

    
    //seeds the seeder, which in turn gives the seedvalue for the well-rng
	RNG seeder(s);
    for(unsigned int i = 0; i < stream; i++)
    	seeder();
    result_type value = seeder();
      
     // seed the generator
     UIntType seedmask = ~0u;
     state[0] = value & seedmask;
     for (int i = 1; i < statesize; i++) 
     {
     	state[i] = (1812433253UL * 
          (state[i-1] ^ (state[i-1] >> (statesize -2))) + i) & seedmask;
     }
     	state_i = 0;
    
  }

        
        // ==
        
        friend bool operator==(const well& x, const well& y)
        {
            for(int j = 0; j < statesize; ++j)
                if(x.state[j] != y.state[j])
                    return false;
            return true;
        }
            
        // !=
        
        friend bool operator!=(const well& x, const well& y)
        { return !(x == y); }
        
        //ostream
        
        template<class CharT, class Traits>
        friend std::basic_ostream<CharT,Traits>&
        operator<<(std::basic_ostream<CharT,Traits>& os,  const well& w)
        {
            for(int j = 0; j < statesize; j++)
              os << w.state[j] << " ";
            os << w.state_i;
            return os;
        }
        
        //istream
        
        template<class CharT, class Traits>
        friend std::basic_istream<CharT,Traits>&
        operator>>(std::basic_istream<CharT,Traits>& is, well& w)	
        {
            for(int j = 0; j < statesize; j++)
              is >> w.state[j] >> std::ws;
            is >> w.state_i;
            return is;
        }
        
        result_type min BOOST_PREVENT_MACRO_SUBSTITUTION () const
        {
            return min_value;
        }
        
        result_type max BOOST_PREVENT_MACRO_SUBSTITUTION () const
        {
            result_type res = 0;
            for(int i = 0; i < 32; ++i)
            res |= (1u << i);
            return res;
        }
        
        static bool validation(result_type value)
        {
            return(val == value);
        }
        
        result_type operator()()
        {
            result_type z0 = state[(state_i + (statesize - 1) )& mask];
            result_type z1 = F1::f(state[state_i])^F2::f(state[(state_i + p1) & mask]);
            result_type z2 = F3::f(state[(state_i + p2)& mask])^F4::f(state[(state_i + p3) & mask]);
            result_type z3 = z1^z2;
            state[state_i] = z3;
            result_type z4 = F5::f(z0)^F6::f(z1)^F7::f(z2)^F8::f(z3);
            state[(state_i + (statesize - 1) )& mask] = z4;
            state_i = (state_i + (statesize - 1) ) & mask;
            return state[state_i];
        }
                
    private:
        result_type state[statesize];
        UIntType state_i;
};

#ifndef BOOST_NO_INCLASS_MEMBER_INITIALIZATION
template<class UIntType, int statesize, UIntType val, class F1, class F2, class F3, class F4, class F5, class F6, class F7, class F8, int p1,  int p2,  int p3, UIntType mask, class RNG>
const bool well<UIntType, statesize, val, F1, F2, F3, F4, F5, F6, F7, F8, p1, p2, p3, mask, RNG>::has_fixed_range;

template<class UIntType, int statesize, UIntType val, class F1, class F2, class F3, class F4, class F5, class F6, class F7, class F8, int p1,  int p2,  int p3, UIntType mask, class RNG>
const UIntType well<UIntType, statesize, val, F1, F2, F3, F4, F5, F6, F7, F8, p1, p2, p3, mask, RNG>::min_value;

template<class UIntType, int statesize, UIntType val, class F1, class F2, class F3, class F4, class F5, class F6, class F7, class F8, int p1,  int p2,  int p3, UIntType mask, class RNG>
const UIntType well<UIntType, statesize, val, F1, F2, F3, F4, F5, F6, F7, F8, p1, p2, p3, mask, RNG>::max_value;
#endif

typedef well<uint32_t,16,178010050,mat0neg<-16>, mat0neg<-15>, mat0pos<11>, 
            zero, mat0neg<-2>, mat0neg<-18>, maF3neg<-28>, maF4neg<-5>,
            13,9,1, 0x0000000fU, boost::mt19937 > well512a;

typedef well<uint32_t,32,1573116597,identity, mat0pos<8>, mat0neg<-19>, 
             mat0neg<-14>, mat0neg<-11>, mat0neg<-7>, mat0neg<-13>, zero,
             3,24,10, 0x0000001fU,  boost::mt19937 > well1024a;

} } // end namespace random::parallel
        
using random::parallel::well512a;
using random::parallel::well1024a;


} // end namespace boost


#endif /*BOOST_RANDOM_PARALLEL_WELL_HPP*/

