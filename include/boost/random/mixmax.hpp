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
 
#ifndef BOOST_RANDOM_MIXMAX_HPP
#define BOOST_RANDOM_MIXMAX_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <array>

namespace boost {
namespace random {

template <typename T, T __min, T __max> class _Generator
// Boilerplate, it is required to be compatible with std::random interfaces, see example.cpp for how to use.
{
public:
    // Interfaces required by C++11 std::random and boost::random
    using result_type = T; //C++11 only
    static constexpr T min() {return __min;}
    static constexpr T max() {return __max;}
    void seed (std::uint64_t val = 1);
    std::uint64_t operator()();
    //std::ostream& operator<< (std::ostream&, const  _Generator&); // implementation cant be outside of class?
    void discard(std::uint64_t nsteps);    // boost::random wants this to fast-forward the generator by nsteps
};

/*
 Table of parameters for MIXMAX

 Figure of merit is entropy: best generator overall is N=240

 Vector size |                                                                                    period q
 N           |    SPECIAL                |   SPECIALMUL   |           MOD_MULSPEC               | log10(q)  |   entropy  | 
 ------------------------------------------------------------------------------------------------------------------------|
   8         |         0                 |     53         |                none                 |    129    |    220.4   | 
  17         |         0                 |     36         |                none                 |    294    |    374.3   | 
 240         |     271828282             |     32         |   fmodmulM61( 0, SPECIAL , (k) )    |   4389    |   8679.2   |

*/

//template <int Ndim=240, int SPECIALMUL=32, std::int64_t SPECIAL=271828282> // TEMPLATE
template <int Ndim, int SPECIALMUL, std::int64_t SPECIAL> // TEMPLATE
class mixmax_engine: public _Generator<std::uint64_t, 0, 0x1FFFFFFFFFFFFFFF> // does not work with any other values
{
private: // DATATYPES
    static const int N = Ndim;

    struct rng_state_st
    {
        std::array<std::uint64_t, N> V;
        std::uint64_t sumtot;
        int counter;
    };
    
    typedef struct rng_state_st rng_state_t;     // struct alias
    rng_state_t S;
    
public: // FUNCTIONS
    mixmax_engine();              // Constructor, unit vector as initial state
    mixmax_engine(std::uint64_t); // Constructor, one 64-bit seed
    mixmax_engine(uint32_t clusterID, uint32_t machineID, uint32_t runID, uint32_t  streamID );       // Constructor with four 32-bit seeds
    void seed(std::uint64_t seedval){seed_uniquestream( &S, 0, 0, (uint32_t)(seedval>>32), (uint32_t)seedval );} // seed with one 64-bit seed

    static constexpr           int rng_get_N()          {return N;}          // return internal parameters
    static constexpr long long int rng_get_SPECIAL()    {return SPECIAL;}
    static constexpr           int rng_get_SPECIALMUL() {return SPECIALMUL;}

    std::uint64_t get_next() ;
    double get_next_float();
    std::uint64_t operator()() {return get_next();}          // return one uint64 between min=0 and max=2^61-1
    double operator++(int unused) {return get_next_float();} // postfix ++ is increments state by one step, returns a double on [0,1]

    /* return a new generator with deterministically determined state, but statistically independent stream
       useful when you need to simulate a branching random process
     */
    mixmax_engine Branch();
    void BranchInplace();
    mixmax_engine& operator=(const mixmax_engine& other ); // simple copy

    void discard(std::uint64_t nsteps) {for(std::uint64_t j = 0; j < nsteps; ++j)  (*this)();} // required in boost
    
#ifndef BOOST_RANDOM_NO_STREAM_OPERATORS
    friend std::ostream& operator<< (std::ostream& ost, const mixmax_engine& me) // save the state of RNG to stream
    {
        int j;
        ost << "mixmax state, file version 1.0\n" ;
        ost << "N=" << me.rng_get_N() << "; V[N]={";
        for (j=0; (j< (me.rng_get_N()-1) ); j++) {
            ost <<  (std::uint64_t)me.S.V[j] << ", ";
        }
        ost << me.S.V[me.rng_get_N()-1] ;
        ost << "}; " ;
        ost << "counter=" << (std::uint64_t)me.S.counter << "; ";
        ost << "sumtot=" << (std::uint64_t)me.S.sumtot << ";\n";
        ost.flush();
        return ost;
    }
    
    friend std::istream& operator>> (std::istream &in, mixmax_engine& me){
        // will throw an exception if format is not right
        std::array<std::uint64_t, N> vec;
        std::uint64_t sum=0, sumtmp=0, counter=0;
        std::string line;
        std::string token;
        //in.ignore(150,'='); // up to N=
        try{
            if(std::getline(in, line)){
                std::istringstream iss(line);
                iss.ignore(150,'='); // up to N=
                std::getline(iss, token, ';');
                int i=std::stoi(token);
                if(i!=Ndim) { std::cerr << "ERROR: Wrong dimension of the MIXMAX RNG state on input, "<<i<<" vs "<<Ndim<<"\n"; }else{std::cerr <<"Dim ok "<< i << "\n";}
                iss.ignore(15,'{'); // up to V[N]={
                for(int j=0;j<Ndim-1;j++) {
                    std::getline(iss, token, ','); if (!iss.fail() ) { vec[j]=std::stoull(token);sum=me.modadd(sum,vec[j]);std::cerr <<vec[j]<<"; ";}
                }
                std::getline(iss, token, '}');
                if (!iss.fail() ) { vec[Ndim-1]=std::stoull(token);std::cerr <<vec[Ndim-1]<<"; ";}else{std::cerr << "ERROR: last elem empty\n";}
                iss.ignore(10,'='); std::getline(iss, token, ';'); counter=std::stoi(token);
                iss.ignore(10,'='); std::getline(iss, token, ';'); sumtmp=std::stoull(token);
            }else{std::cerr << "ERROR: nothing to read\n";}
            }catch (const std::exception& e) {
                std::cerr << "ERROR: Exception caught in MIXMAX RNG while reading state: " << e.what() << "\nfrom string='"<<token<<"'\n"; //
                in.setstate(std::ios::failbit);
                throw;
            }
        sum=me.modadd(sum,MERSBASE-vec[0]);
        me.S.V=vec; me.S.counter = counter; me.S.sumtot=sumtmp;
        if (sum == sumtmp && counter>0 && counter<N){
            me.S.V=vec; me.S.counter = counter; me.S.sumtot=sumtmp;
        }else{
            std::cerr << "ERROR: incorrect checksum or out of range counter while reading  MIXMAX RNG state.\n"
                        <<counter<<" "<<sumtmp<<" "<<sum<<"\n";
            in.setstate(std::ios::failbit);
        }
        return in;
    }
#endif
    
    
private:
    static constexpr int BITS=61;
    static constexpr std::uint64_t M61=2305843009213693951ULL;
    static constexpr std::uint64_t MERSBASE=M61;
    static constexpr double INV_MERSBASE=(0.43368086899420177360298E-18);
    std::uint64_t MOD_MERSENNE(std::uint64_t k) {return ((((k)) & MERSBASE) + (((k)) >> BITS) );}
    std::uint64_t MOD_MULSPEC(std::uint64_t k);
    std::uint64_t MULWU(std::uint64_t k);
    void seed_vielbein(rng_state_t* X, unsigned int i); // seeds with the i-th unit vector, i = 0..N-1,  for testing only
    void seed_uniquestream( rng_state_t* Xin, uint32_t clusterID, uint32_t machineID, uint32_t runID, uint32_t  streamID );
    std::uint64_t iterate_raw_vec(std::uint64_t* Y, std::uint64_t sumtotOld);
    std::uint64_t apply_bigskip(std::uint64_t* Vout, std::uint64_t* Vin, uint32_t clusterID, uint32_t machineID, uint32_t runID, uint32_t  streamID );
    std::uint64_t modadd(std::uint64_t foo, std::uint64_t bar);
    std::uint64_t fmodmulM61(std::uint64_t cum, std::uint64_t s, std::uint64_t a);
#if defined(__x86_64__)
    inline std::uint64_t mod128(__uint128_t s);
#endif
};

#define MIXMAX_PREF template <int Ndim, int SPECIALMUL, std::int64_t SPECIAL> // TEMPLATE
#define MIXMAX_POST              <Ndim,     SPECIALMUL,              SPECIAL> // TEMPLATE

MIXMAX_PREF std::uint64_t mixmax_engine MIXMAX_POST::MOD_MULSPEC(std::uint64_t k){
    switch (N) {
        case 17:
            return 0;
            break;
        case 8:
            return 0;
            break;
        case 240:
            return fmodmulM61( 0, SPECIAL , (k) );
            break;
        default:
            std::cerr << "MIXMAX ERROR: " << "Disallowed value of parameter N\n";
            std::terminate();
    }
}

MIXMAX_PREF mixmax_engine MIXMAX_POST ::mixmax_engine()
// constructor, with no params, fast and seeds with a unit vector
{
    seed_vielbein(&S,0);
}

MIXMAX_PREF mixmax_engine MIXMAX_POST ::mixmax_engine(std::uint64_t seedval)
// constructor, one uint64_t seed
{
    seed_uniquestream( &S, 0,  0,  (uint32_t)(seedval>>32), (uint32_t)seedval );
}

MIXMAX_PREF mixmax_engine MIXMAX_POST ::mixmax_engine(uint32_t clusterID, uint32_t machineID, uint32_t runID, uint32_t  streamID)
// constructor, no need to allocate, just seed
{
    seed_uniquestream( &S, clusterID,  machineID,  runID,  streamID );
}

MIXMAX_PREF uint64_t mixmax_engine MIXMAX_POST ::MULWU (uint64_t k){ return (( (k)<<(SPECIALMUL) & M61) ^ ( (k) >> (BITS-SPECIALMUL))  )  ;}

MIXMAX_PREF std::uint64_t mixmax_engine MIXMAX_POST ::iterate_raw_vec(std::uint64_t* Y, std::uint64_t sumtotOld){
    // operates with a raw vector, uses known sum of elements of Y
    int i;
    
    std::uint64_t temp2 = Y[1];
    std::uint64_t  tempP, tempV;
    Y[0] = ( tempV = sumtotOld);
    std::uint64_t sumtot = Y[0], ovflow = 0; // will keep a running sum of all new elements
    tempP = 0;                               // will keep a partial sum of all old elements
    for (i=1; i<N; i++){
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
        temp2 = MOD_MULSPEC(temp2);
        Y[2] = modadd( Y[2] , temp2 );
        sumtot += temp2; if (sumtot < temp2) {ovflow++;}
    }
    return MOD_MERSENNE(MOD_MERSENNE(sumtot) + (ovflow <<3 ));
}

MIXMAX_PREF std::uint64_t mixmax_engine MIXMAX_POST ::get_next() {
    int i;
    i=S.counter;
    
    if (i<=(N-1) ){
        S.counter++;
        return S.V[i];
    }else{
        S.sumtot = iterate_raw_vec(S.V.data(), S.sumtot);
        S.counter=2;
        return S.V[1];
    }
}

MIXMAX_PREF double mixmax_engine MIXMAX_POST ::get_next_float()				// Returns a random double with all 53 bits random, in the range (0,1]
{    /* cast to signed int trick suggested by Andrzej Görlich     */
    int64_t Z=(int64_t)get_next();
    double F;
#if defined(__GNUC__) && (__GNUC__ < 5) && (!defined(__ICC)) && defined(__x86_64__) && defined(__SSE2_MATH__) && defined(USE_INLINE_ASM)
#warning Using inline assembler to zero xmm register
    /* using SSE inline assemly to zero the xmm register, just before int64 -> double conversion,
     not necessary in GCC-5 or better, but huge penalty on earlier compilers
     */
    __asm__  __volatile__("pxor %0, %0; "
                          :"=x"(F)
                          );
#endif
    F=Z;
    return F*INV_MERSBASE;
}

MIXMAX_PREF void mixmax_engine MIXMAX_POST ::seed_vielbein(rng_state_t* X, unsigned int index)
{
    int i;
    if (index<N){
        for (i=0; i < N; i++){
            X->V[i] = 0;
        }
        X->V[index] = 1;
    }else{
        std::cerr << "MIXMAX ERROR: " << "Out of bounds index, is not ( 0 <= index < N  )\n";
        std::terminate();
    }
    X->counter = N;  // set the counter to N if iteration should happen right away
    X->sumtot = 1;
}


MIXMAX_PREF void mixmax_engine MIXMAX_POST ::seed_uniquestream( rng_state_t* Xin, uint32_t clusterID, uint32_t machineID, uint32_t runID, uint32_t  streamID ){
    seed_vielbein(Xin,0);
    Xin->sumtot = apply_bigskip(Xin->V.data(), Xin->V.data(),  clusterID,  machineID,  runID,   streamID );
    std::cerr << "seeding mixmax with: {" << clusterID << ", " << machineID << ", " <<   runID <<  ", " <<  streamID << "}\n";
    Xin->counter = 1;
}


MIXMAX_PREF std::uint64_t mixmax_engine MIXMAX_POST ::apply_bigskip( std::uint64_t* Vout, std::uint64_t* Vin, uint32_t clusterID, uint32_t machineID, uint32_t runID, uint32_t  streamID ){
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
    
    
    const	std::uint64_t skipMat240[128][240] =
#include "boost/random/detail/mixmax_skip_N240.icc"
    ;
    const	std::uint64_t skipMat17[128][17] =
#include "boost/random/detail/mixmax_skip_N17.icc"
    ;
//    const	std::uint64_t skipMat8[128][8] =
//#include "mixmax_skip_N8.c"
    ;
    
    const std::uint64_t* skipMat[128];
    switch (N) {
        case 240:
            for (int i=0; i<128; i++) { skipMat[i] = skipMat240[i];}
            break;
        case 17:
            for (int i=0; i<128; i++) { skipMat[i] = skipMat17[i];}
            break;
//        case 8:
//            for (int i=0; i<128; i++) { skipMat[i] = skipMat8[i];}
//            break;
            
        default:
        	std::cerr << "disallowed value of N\n";
            std::terminate();
    }
    
    uint32_t IDvec[4] = {streamID, runID, machineID, clusterID};
    int r,i,j,  IDindex;
    uint32_t id;
    std::uint64_t Y[N], cum[N];
    std::uint64_t coeff;
    std::uint64_t* rowPtr;
    std::uint64_t sumtot=0;
    
    
    for (i=0; i<N; i++) { Y[i] = Vin[i]; sumtot = modadd( sumtot, Vin[i]); } ;
    for (IDindex=0; IDindex<4; IDindex++) { // go from lower order to higher order ID
        id=IDvec[IDindex];
        //printf("now doing ID at level %d, with ID = %d\n", IDindex, id);
        r = 0;
        while (id){
            if (id & 1) {
                rowPtr = (std::uint64_t*)skipMat[r + IDindex*8*sizeof(uint32_t)];
                for (i=0; i<N; i++){ cum[i] = 0; }
                for (j=0; j<N; j++){              // j is lag, enumerates terms of the poly
                    // for zero lag Y is already given
                    coeff = rowPtr[j]; // same coeff for all i
                    for (i =0; i<N; i++){
                        cum[i] =  fmodmulM61( cum[i], coeff ,  Y[i] ) ;
                    }
                    sumtot = iterate_raw_vec(Y, sumtot);
                }
                sumtot=0;
                for (i=0; i<N; i++){ Y[i] = cum[i]; sumtot = modadd( sumtot, cum[i]); } ;
            }
            id = (id >> 1); r++; // bring up the r-th bit in the ID
        }
    }
    sumtot=0;
    for (i=0; i<N; i++){ Vout[i] = Y[i]; sumtot = modadd( sumtot, Y[i]); } ;  // returns sumtot, and copy the vector over to Vout
    return (sumtot) ;
}

#if defined(__x86_64__)
MIXMAX_PREF inline std::uint64_t mixmax_engine MIXMAX_POST ::mod128(__uint128_t s){
    std::uint64_t s1;
    s1 = ( (  ((std::uint64_t)s)&MERSBASE )    + (  ((std::uint64_t)(s>>64)) * 8 )  + ( ((std::uint64_t)s) >>BITS) );
    return	MOD_MERSENNE(s1);
}

MIXMAX_PREF inline std::uint64_t mixmax_engine MIXMAX_POST ::fmodmulM61(std::uint64_t cum, std::uint64_t a, std::uint64_t b){
    __uint128_t temp;
    temp = (__uint128_t)a*(__uint128_t)b + cum;
    return mod128(temp);
}

#else // on all other platforms, including 32-bit linux, PPC and PPC64, ARM and all Windows
const std::uint64_t MASK32=0xFFFFFFFFULL;

MIXMAX_PREF inline std::uint64_t mixmax_engine MIXMAX_POST ::fmodmulM61(std::uint64_t cum, std::uint64_t s, std::uint64_t a)
{
    register std::uint64_t o,ph,pl,ah,al;
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

MIXMAX_PREF std::uint64_t mixmax_engine MIXMAX_POST ::modadd(std::uint64_t foo, std::uint64_t bar){
#if (defined(__x86_64__) || defined(__i386__)) &&  defined(__GNUC__)  && (!defined(__ICC)) && !defined(__clang__) && defined(USE_INLINE_ASM)
#warning Using assembler routine in modadd
    std::uint64_t out;
    /* Assembler trick suggested by Andrzej Görlich     */
    __asm__ ("addq %2, %0; "
             "btrq $61, %0; "
             "adcq $0, %0; "
             :"=r"(out)
             :"0"(foo), "r"(bar)
             );
    return out;
#else
    return MOD_MERSENNE(foo+bar);
#endif
}

MIXMAX_PREF mixmax_engine MIXMAX_POST mixmax_engine MIXMAX_POST ::Branch(){
    S.sumtot = iterate_raw_vec(S.V.data(), S.sumtot); S.counter = N-1;
    mixmax_engine tmp=*this;
    tmp.BranchInplace();
    return tmp;
}

MIXMAX_PREF mixmax_engine MIXMAX_POST & mixmax_engine MIXMAX_POST ::operator=(const mixmax_engine& other ){
    S.V = other.S.V;
    S.sumtot = other.S.sumtot;
    S.counter = other.S.counter;
    return *this;
}

MIXMAX_PREF void mixmax_engine MIXMAX_POST ::BranchInplace(){
    // Dont forget to iterate the mother, when branching the daughter, or else will have collisions!
    // a 64-bit LCG from Knuth line 26, is used to mangle a vector component
    constexpr std::uint64_t MULT64=6364136223846793005ULL;
    std::uint64_t tmp=S.V[1];
    S.V[1] *= MULT64; S.V[1] &= MERSBASE;
    S.sumtot = modadd( S.sumtot , S.V[1] - tmp + MERSBASE);
    S.sumtot = iterate_raw_vec(S.V.data(), S.sumtot);
    S.counter = 1;
}

template class mixmax_engine<240,32,271828282>;    // TEMPLATE instantiation
template class mixmax_engine<17,36,0>;             // TEMPLATE instantiation
typedef mixmax_engine<17,36,0>          mixmaxN17;
typedef mixmax_engine<240,32,271828282> mixmax;
}
}

#undef MIXMAX_PREF
#undef MIXMAX_POST

#endif // BOOST_RANDOM_MIXMAX_HPP
