/* boost random/mixmax.hpp header file
 *
 * Copyright Kostas Savvidis 2008-2019
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org for most recent version including documentation.
 *
 * $Id$
 *
 * Revision history
 *  2019-04-23 created
 */

 /*  MIXMAX generator
 *
 *  G.K.Savvidy and N.G.Ter-Arutyunian,
 *  On the Monte Carlo simulation of physical systems,
 *  J.Comput.Phys. 97, 566 (1991);
 *  Preprint EPI-865-16-86, Yerevan, Jan. 1986
 *  http://dx.doi.org/10.1016/0021-9991(91)90015-D
 *
 *  K.Savvidy
 *  The MIXMAX random number generator
 *  Comp. Phys. Commun. 196 (2015), pp 161–165
 *  http://dx.doi.org/10.1016/j.cpc.2015.06.003
 *
 *  K.Savvidy and G.Savvidy
 *  Spectrum and Entropy of C-systems. MIXMAX random number generator
 *  Chaos, Solitons & Fractals, Volume 91, (2016) pp. 33–38
 *  http://dx.doi.org/10.1016/j.chaos.2016.05.003
 *
 */

#ifndef BOOST_RANDOM_MIXMAX_HPP
#define BOOST_RANDOM_MIXMAX_HPP

#include <sstream>
#include <array>

#include <boost/random/detail/seed.hpp>
#include <boost/random/detail/seed_impl.hpp>

namespace boost {
namespace random {

    /**
     * Instantiations of class template mixmax_engine model,
     * \pseudo_random_number_generator . It uses the algorithms from:
     *
     *  @blockquote
     *  G.K.Savvidy and N.G.Ter-Arutyunian,
     *  On the Monte Carlo simulation of physical systems,
     *  J.Comput.Phys. 97, 566 (1991);
     *  Preprint EPI-865-16-86, Yerevan, Jan. 1986
     *  http://dx.doi.org/10.1016/0021-9991(91)90015-D
     *
     *  K.Savvidy
     *  The MIXMAX random number generator
     *  Comp. Phys. Commun. 196 (2015), pp 161–165
     *  http://dx.doi.org/10.1016/j.cpc.2015.06.003
     *
     *  K.Savvidy and G.Savvidy
     *  Spectrum and Entropy of C-systems. MIXMAX random number generator
     *  Chaos, Solitons & Fractals, Volume 91, (2016) pp. 33–38
     *  http://dx.doi.org/10.1016/j.chaos.2016.05.003
     *  @endblockquote
     *
     * The generator crucially depends on the choice of the
     * parameters. The valid sets of parameters are from the published papers above.
     *
     */

template <int Ndim, int SPECIALMUL, std::int64_t SPECIAL> // MIXMAX TEMPLATE PARAMETERS
class mixmax_engine
{
    public:
    // Interfaces required by C++11 std::random and boost::random
        typedef std::uint64_t result_type ;
        BOOST_STATIC_CONSTANT(std::uint64_t,mixmax_min=0);
        BOOST_STATIC_CONSTANT(std::uint64_t,mixmax_max=((1ULL<<61)-1));
        BOOST_STATIC_CONSTEXPR result_type min() {return mixmax_min;}
        BOOST_STATIC_CONSTEXPR result_type max() {return mixmax_max;}
        //void seed (std::uint64_t val = 1);
        //std::uint64_t operator()();
        static const bool has_fixed_range = false;
        BOOST_STATIC_CONSTANT(int,N=Ndim);     ///< The main internal parameter, size of the defining MIXMAX matrix
    // CONSTRUCTORS:
        mixmax_engine();                       ///< Constructor, unit vector as initial state, acted on by A^2^512
        mixmax_engine(std::uint64_t);          ///< Constructor, one 64-bit seed
        mixmax_engine(uint32_t clusterID, uint32_t machineID, uint32_t runID, uint32_t  streamID );  ///< Constructor, four 32-bit seeds for 128-bit seeding flexibility
        void seed(std::uint64_t seedval=default_seed){seed_uniquestream( &S, 0, 0, (uint32_t)(seedval>>32), (uint32_t)seedval );} ///< seed with one 64-bit seed

private: // DATATYPES
    struct rng_state_st
    {
        std::array<std::uint64_t, Ndim> V;
        std::uint64_t sumtot;
        int counter;
    };
    
    typedef struct rng_state_st rng_state_t;     // struct alias
    rng_state_t S;
    
    inline std::uint64_t get_next();    // output a random integer on [0,2^61-1]
    
public: // SEEDING FUNCTIONS
    template<class It> mixmax_engine(It& first, It last) { seed(first,last); }
    BOOST_RANDOM_DETAIL_SEED_SEQ_CONSTRUCTOR(mixmax_engine,  SeedSeq, seq){ seed(seq); }
    
    /** Sets the state of the generator using values from an iterator range. */
    template<class It>
    void seed(It& first, It last)
    {
        uint32_t v[4];
        detail::fill_array_int<32>(first, last, v);
            seed_uniquestream( &S, v[0], v[1], v[2], v[3]);
    }
    /** Sets the state of the generator using values from a seed_seq. */
    BOOST_RANDOM_DETAIL_SEED_SEQ_SEED(mixmax_engine, SeeqSeq, seq)
    {
        uint32_t v[4];
        detail::seed_array_int<32>(seq, v);
        seed_uniquestream( &S, v[0], v[1], v[2], v[3]);
    }

    std::uint64_t operator()() {return get_next();}          ///< return one uint64 between min=0 and max=2^61-1
    
    /** Fills a range with random values */
    template<class Iter>
    void generate(Iter first, Iter last)
    { detail::generate_from_int(*this, first, last); }
    
    mixmax_engine& operator=(const mixmax_engine& other ); ///< simple copy

    void discard(std::uint64_t nsteps) {for(std::uint64_t j = 0; j < nsteps; ++j)  (*this)();} ///< discard n steps, required in boost::random
    
#ifndef BOOST_RANDOM_NO_STREAM_OPERATORS
    /** save the state of the RNG to a stream */
    template<class CharT, class Traits>
    friend std::basic_ostream<CharT,Traits>&
    operator<< (std::basic_ostream<CharT,Traits>& ost, const mixmax_engine& me)
    {
        int j;
        ost << Ndim << " ";
        ost << me.S.counter << " ";
        ost << me.S.sumtot << " ";
        for (j=0; (j< (Ndim-1) ); j++) {
            ost <<  (std::uint64_t)me.S.V[j] << " ";
        }
        ost << me.S.V[Ndim-1] << "\n";
        ost.flush();
        return ost;
    }
    
    /** read the state of the RNG from a stream */
    template<class CharT, class Traits>
    friend std::basic_istream<CharT,Traits>&
    operator>> (std::basic_istream<CharT,Traits> &in, mixmax_engine& me){
        // will set std::ios::failbit and throw an exception if format is not right
        std::array<std::uint64_t, Ndim> vec;
        std::uint64_t sum=0, sumtmp=0, counter=0;
        std::basic_string<CharT> line;
        std::basic_string<CharT> token;
        CharT xxxchar;
        try{
            if(std::getline( in, line)){
                std::basic_istringstream<CharT> iss(line);
                getline(iss, token, xxxchar=' ');
                int i=std::stoi(token);
                if(i!=Ndim) {
                    throw;
                    // std::cout << "ERROR: Wrong dimension of the MIXMAX RNG state on input, "<<i<<" vs "<<Ndim<<"\n";
                    }
                std::getline(iss, token, xxxchar=' '); counter=std::stoull(token);
                std::getline(iss, token, xxxchar=' '); sumtmp=std::stoull(token);
                for(int j=0;j<Ndim-1;j++) {
                    std::getline(iss, token, xxxchar=' ');
                    if (!iss.fail() ) { vec[j]=std::stoull(token); sum=me.MOD_MERSENNE(sum+vec[j]);}
                }
                std::getline(iss, token);
                if (!iss.fail() ) { vec[Ndim-1]=std::stoull(token); sum=me.MOD_MERSENNE(sum+vec[Ndim-1]);
                }else{throw;}
            }else{throw;}
            }catch (const std::exception& e) {
                in.setstate(std::ios::failbit);
                throw;
            }
        if (sum == sumtmp && counter>0 && counter<Ndim){
            me.S.V=vec; me.S.counter = counter; me.S.sumtot=sumtmp;
        }else{
//            std::cout << "ERROR: incorrect checksum or out of range counter while reading  MIXMAX RNG state.\n"
//                        <<counter<<" "<<sumtmp<<" vs "<<sum<<"\n";
            in.setstate(std::ios::failbit);
            throw;
        }
        return in;
    }
#endif
    
    friend bool operator==(const mixmax_engine & x,
                           const mixmax_engine & y){return x.S.counter==y.S.counter&& x.S.sumtot==y.S.sumtot && x.S.V[1]==y.S.V[1] ;}; // && x.counter == y.counter;
    friend bool operator!=(const mixmax_engine & x,
                           const mixmax_engine & y){return !(x.S.counter==y.S.counter&& x.S.sumtot==y.S.sumtot && x.S.V[1]==y.S.V[1]) ;}; // && x.counter == y.counter

    
private:
BOOST_STATIC_CONSTANT(int, BITS=61);
BOOST_STATIC_CONSTANT(std::uint64_t, M61=2305843009213693951ULL);
BOOST_STATIC_CONSTANT(std::uint64_t, default_seed=1);
    inline std::uint64_t MOD_MERSENNE(std::uint64_t k) {return ((((k)) & M61) + (((k)) >> BITS) );}
    inline std::uint64_t MULWU(std::uint64_t k);
    inline void seed_vielbein(rng_state_t* X, unsigned int i); // seeds with the i-th unit vector, i = 0..Ndim-1,  for testing only
    inline void seed_uniquestream( rng_state_t* Xin, uint32_t clusterID, uint32_t machineID, uint32_t runID, uint32_t  streamID );
    inline std::uint64_t iterate_raw_vec(std::uint64_t* Y, std::uint64_t sumtotOld);
    inline std::uint64_t apply_bigskip(std::uint64_t* Vout, std::uint64_t* Vin, uint32_t clusterID, uint32_t machineID, uint32_t runID, uint32_t  streamID );
    inline std::uint64_t modadd(std::uint64_t foo, std::uint64_t bar);
    inline std::uint64_t fmodmulM61(std::uint64_t cum, std::uint64_t s, std::uint64_t a);
#if defined(BOOST_HAS_INT128)
    inline std::uint64_t mod128(__uint128_t s);
#endif
};

template <int Ndim, int SPECIALMUL, std::int64_t SPECIAL> mixmax_engine  <Ndim, SPECIALMUL, SPECIAL> ::mixmax_engine()
///< constructor, with no params, seeds with seed=0,  random numbers are as good as from any other seed
{
    seed_uniquestream( &S, 0,  0, 0, default_seed);
}

template <int Ndim, int SPECIALMUL, std::int64_t SPECIAL> mixmax_engine  <Ndim, SPECIALMUL, SPECIAL> ::mixmax_engine(std::uint64_t seedval)
///< constructor, one uint64_t seed, random numbers are statistically independent from any two distinct seeds, e.g. consecutive seeds are ok
{
    seed_uniquestream( &S, 0,  0,  (uint32_t)(seedval>>32), (uint32_t)seedval );
}

template <int Ndim, int SPECIALMUL, std::int64_t SPECIAL> mixmax_engine  <Ndim, SPECIALMUL, SPECIAL> ::mixmax_engine(uint32_t clusterID, uint32_t machineID, uint32_t runID, uint32_t  streamID)
// constructor, four 32-bit seeds for 128-bit seeding flexibility
{
    seed_uniquestream( &S, clusterID,  machineID,  runID,  streamID );
}

template <int Ndim, int SPECIALMUL, std::int64_t SPECIAL> uint64_t mixmax_engine  <Ndim, SPECIALMUL, SPECIAL> ::MULWU (uint64_t k){ return (( (k)<<(SPECIALMUL) & M61) ^ ( (k) >> (BITS-SPECIALMUL))  )  ;}

template <int Ndim, int SPECIALMUL, std::int64_t SPECIAL> std::uint64_t mixmax_engine  <Ndim, SPECIALMUL, SPECIAL> ::iterate_raw_vec(std::uint64_t* Y, std::uint64_t sumtotOld){
    // operates with a raw vector, uses known sum of elements of Y
    int i;
    
    std::uint64_t temp2 = Y[1];
    std::uint64_t  tempP, tempV;
    Y[0] = ( tempV = sumtotOld);
    std::uint64_t sumtot = Y[0], ovflow = 0; // will keep a running sum of all new elements
    tempP = 0;                               // will keep a partial sum of all old elements
    for (i=1; i<Ndim; i++){
        if (SPECIALMUL!=0){
            std::uint64_t tempPO = MULWU(tempP);
            tempP = modadd(tempP, Y[i]);
            tempV = MOD_MERSENNE(tempV+tempP+tempPO); // new Y[i] = old Y[i] + old partial * m
        }else{
            tempP = modadd(tempP , Y[i]);
            tempV = modadd(tempV , tempP);
        }
        
        Y[i] = tempV;
        sumtot += tempV; if (sumtot < tempV) {ovflow++;}
    }
    if ( SPECIAL !=0 ){
        Y[2] = modadd( Y[2] , temp2 );
        sumtot += temp2; if (sumtot < temp2) {ovflow++;}
    }
    return MOD_MERSENNE(MOD_MERSENNE(sumtot) + (ovflow <<3 ));
}

template <int Ndim, int SPECIALMUL, std::int64_t SPECIAL> inline std::uint64_t mixmax_engine  <Ndim, SPECIALMUL, SPECIAL> ::get_next() {
    int i;
    i=S.counter;
    
    if (i<=(Ndim-1) ){
        S.counter++;
        return S.V[i];
    }else{
        S.sumtot = iterate_raw_vec(S.V.data(), S.sumtot);
        S.counter=2;
        return S.V[1];
    }
}

template <int Ndim, int SPECIALMUL, std::int64_t SPECIAL> void mixmax_engine  <Ndim, SPECIALMUL, SPECIAL> ::seed_vielbein(rng_state_t* X, unsigned int index)
{
    int i;
    if (index<Ndim){
        for (i=0; i < Ndim; i++){
            X->V[i] = 0;
        }
        X->V[index] = 1;
    }else{
        std::terminate();
    }
    X->counter = Ndim;  // set the counter to Ndim if iteration should happen right away
    X->sumtot = 1;
}


template <int Ndim, int SPECIALMUL, std::int64_t SPECIAL> void mixmax_engine  <Ndim, SPECIALMUL, SPECIAL> ::seed_uniquestream( rng_state_t* Xin, uint32_t clusterID, uint32_t machineID, uint32_t runID, uint32_t  streamID ){
    seed_vielbein(Xin,0);
    Xin->sumtot = apply_bigskip(Xin->V.data(), Xin->V.data(),  clusterID,  machineID,  runID,   streamID );
    //std::cout << "seeding mixmax with: {" << clusterID << ", " << machineID << ", " <<   runID <<  ", " <<  streamID << "}\n";
    Xin->counter = 1;
}


template <int Ndim, int SPECIALMUL, std::int64_t SPECIAL> std::uint64_t mixmax_engine  <Ndim, SPECIALMUL, SPECIAL> ::apply_bigskip( std::uint64_t* Vout, std::uint64_t* Vin, uint32_t clusterID, uint32_t machineID, uint32_t runID, uint32_t  streamID ){
    /*
     makes a derived state vector, Vout, from the mother state vector Vin
     by skipping a large number of steps, determined by the given seeding ID's
     
     it is mathematically guaranteed that the substreams derived in this way from the SAME (!!!) Vin will not collide provided
     1) at least one bit of ID is different
     2) less than 10^100 numbers are drawn from the stream
     (this is good enough : a single CPU will not exceed this in the lifetime of the universe, 10^19 sec,
     even if it had a clock cycle of Planck time, 10^44 Hz )
     
     Caution: never apply this to a derived vector, just choose some mother vector Vin, for example the unit vector by seed_vielbein(X,0),
     and use it in all your runs, just change runID to get completely nonoverlapping streams of random numbers on a different day.
     
     clusterID and machineID are provided for the benefit of large organizations who wish to ensure that a simulation
     which is running in parallel on a large number of  clusters and machines will have non-colliding source of random numbers.
     
     did i repeat it enough times? the non-collision guarantee is absolute, not probabilistic
     
     */
    
    
    const	std::uint64_t skipMat17[128][17] =
#include "boost/random/detail/mixmax_skip_N17.ipp"
    ;
    
    const std::uint64_t* skipMat[128];
    switch (Ndim) {
        case 17:
            for (int i=0; i<128; i++) { skipMat[i] = skipMat17[i];}
            break;
        default:
        	//std::cout << "disallowed value of Ndim\n";
            std::terminate();
    }
    
    uint32_t IDvec[4] = {streamID, runID, machineID, clusterID};
    int r,i,j,  IDindex;
    uint32_t id;
    std::uint64_t Y[Ndim], cum[Ndim];
    std::uint64_t coeff;
    std::uint64_t* rowPtr;
    std::uint64_t sumtot=0;
    
    
    for (i=0; i<Ndim; i++) { Y[i] = Vin[i]; sumtot = modadd( sumtot, Vin[i]); } ;
    for (IDindex=0; IDindex<4; IDindex++) { // go from lower order to higher order ID
        id=IDvec[IDindex];
        //printf("now doing ID at level %d, with ID = %d\n", IDindex, id);
        r = 0;
        while (id){
            if (id & 1) {
                rowPtr = (std::uint64_t*)skipMat[r + IDindex*8*sizeof(uint32_t)];
                for (i=0; i<Ndim; i++){ cum[i] = 0; }
                for (j=0; j<Ndim; j++){              // j is lag, enumerates terms of the poly
                    // for zero lag Y is already given
                    coeff = rowPtr[j]; // same coeff for all i
                    for (i =0; i<Ndim; i++){
                        cum[i] =  fmodmulM61( cum[i], coeff ,  Y[i] ) ;
                    }
                    sumtot = iterate_raw_vec(Y, sumtot);
                }
                sumtot=0;
                for (i=0; i<Ndim; i++){ Y[i] = cum[i]; sumtot = modadd( sumtot, cum[i]); } ;
            }
            id = (id >> 1); r++; // bring up the r-th bit in the ID
        }
    }
    sumtot=0;
    for (i=0; i<Ndim; i++){ Vout[i] = Y[i]; sumtot = modadd( sumtot, Y[i]); } ;  // returns sumtot, and copy the vector over to Vout
    return (sumtot) ;
}

#if defined(BOOST_HAS_INT128)
template <int Ndim, int SPECIALMUL, std::int64_t SPECIAL> inline std::uint64_t mixmax_engine  <Ndim, SPECIALMUL, SPECIAL> ::mod128(__uint128_t s){
    std::uint64_t s1;
    s1 = ( (  ((std::uint64_t)s)&M61 )    + (  ((std::uint64_t)(s>>64)) * 8 )  + ( ((std::uint64_t)s) >>BITS) );
    return	MOD_MERSENNE(s1);
}

template <int Ndim, int SPECIALMUL, std::int64_t SPECIAL> inline std::uint64_t mixmax_engine  <Ndim, SPECIALMUL, SPECIAL> ::fmodmulM61(std::uint64_t cum, std::uint64_t a, std::uint64_t b){
    __uint128_t temp;
    temp = (__uint128_t)a*(__uint128_t)b + cum;
    return mod128(temp);
}

#else // on all other platforms, including 32-bit linux, PPC and PPC64, ARM and all Windows
const std::uint64_t MASK32=0xFFFFFFFFULL;

template <int Ndim, int SPECIALMUL, std::int64_t SPECIAL> inline std::uint64_t mixmax_engine  <Ndim, SPECIALMUL, SPECIAL> ::fmodmulM61(std::uint64_t cum, std::uint64_t s, std::uint64_t a)
{
    std::uint64_t o,ph,pl,ah,al;
    o=(s)*a;
    ph = ((s)>>32);
    pl = (s) & MASK32;
    ah = a>>32;
    al = a & MASK32;
    o = (o & M61) + ((ph*ah)<<3) + ((ah*pl+al*ph + ((al*pl)>>32))>>29) ;
    o += cum;
    o = (o & M61) + ((o>>61));
    return o;
}
#endif

template <int Ndim, int SPECIALMUL, std::int64_t SPECIAL> std::uint64_t mixmax_engine  <Ndim, SPECIALMUL, SPECIAL> ::modadd(std::uint64_t foo, std::uint64_t bar){
    return MOD_MERSENNE(foo+bar);
}

template <int Ndim, int SPECIALMUL, std::int64_t SPECIAL> mixmax_engine  <Ndim, SPECIALMUL, SPECIAL> & mixmax_engine  <Ndim, SPECIALMUL, SPECIAL> ::operator=(const mixmax_engine& other ){
    S.V = other.S.V;
    S.sumtot = other.S.sumtot;
    S.counter = other.S.counter;
    return *this;
}

    /* @copydoc boost::random::detail::mixmax_engine_doc */
    /** Instantiation with a valid parameter set. */
typedef mixmax_engine<17,36,0>          mixmax;
}// namespace random
}// namespace boost

#endif // BOOST_RANDOM_MIXMAX_HPP
