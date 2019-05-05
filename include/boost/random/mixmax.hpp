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

#include <iosfwd>
#include <istream>
#include <stdexcept>
#include <boost/config.hpp>
#include <boost/cstdint.hpp>
#include <boost/integer/integer_mask.hpp>
// #include <boost/random/detail/config.hpp>
// #include <boost/random/detail/ptr_helper.hpp>
// #include <boost/random/detail/seed.hpp>
// #include <boost/random/detail/seed_impl.hpp>
// #include <boost/random/detail/generator_seed_seq.hpp>
// #include <boost/random/detail/polynomial.hpp>
// 
// #include <boost/random/detail/disable_warnings.hpp>
#include <array>

namespace boost {
namespace random {

template <typename T, T __min, T __max> class _Generator
// Boilerplate code, it is required to be compatible with std::random interfaces, see example.cpp for how to use.
{
public:
    // Interface C++11 std::random
    using result_type = T; //C++11 only
    static constexpr T min() {return __min;}
    static constexpr T max() {return __max;}
    void seed (std::uint64_t val = 1);
    std::uint64_t operator()();
};

typedef uint32_t myID_t;
typedef uint64_t myuint;


constexpr int Ndim = 17; // turn off for TEMPLATE use

constexpr int BITS=61;
constexpr myuint M61=2305843009213693951ULL;
constexpr myuint MERSBASE=M61;
constexpr double INV_MERSBASE=(0.43368086899420177360298E-18);


/*
 Table of parameters for MIXMAX

 Figure of merit is entropy: best generator overall is N=240

 Vector size |                                                                                    period q
 N           |    SPECIAL                |   SPECIALMUL   |           MOD_MULSPEC               | log10(q)  |   entropy  | 
 ------------------------------------------------------------------------------------------------------------------------|
   8         |         0                 |     53         |                none                 |    129    |    220.4   | 
  17         |         0                 |     36         |                none                 |    294    |    374.3   | 
 240         | 487013230256099140        |     51         |   fmodmulM61( 0, SPECIAL , (k) )    |   4389    |   8679.2   |

*/

// template <int Ndim=240> // TEMPLATE
//class mixmax_engine: public _Generator<std::uint64_t, 0, 0x1FFFFFFFFFFFFFFF> // does not work with any other values
class mixmax_engine: public _Generator<std::uint32_t, 0, 0xFFFFFFFF> // does not work with any other values
{
static const int N = Ndim;
    static constexpr long long int SPECIAL   = ((N==17)? 0 : ((N==240)? 487013230256099140ULL:-1) ); // etc...
    static constexpr long long int SPECIALMUL= ((N==17)? 36: ((N==240)? 51                   : 0) ); // etc...
    // Note the potential for confusion...

struct rng_state_st
{
    std::array<myuint, N> V;
    myuint sumtot;
    int counter;
};
    
typedef struct rng_state_st rng_state_t;     // struct alias

rng_state_t S;
    
public:
	//using T = result_type;								  // should it be double?
    static constexpr int rng_get_N() {return N;}
    static constexpr long long int rng_get_SPECIAL()    {return SPECIAL;}
    static constexpr int rng_get_SPECIALMUL() {return SPECIALMUL;}
    void seed_uniquestream( rng_state_t* Xin, myID_t clusterID, myID_t machineID, myID_t runID, myID_t  streamID );
    void print_state();
// void read_state(const char filename[] );
    myuint get_next() ;
    double get_next_float();

   // int iterate();
    mixmax_engine Branch();
    void BranchInplace();
    
    mixmax_engine(myID_t clusterID, myID_t machineID, myID_t runID, myID_t  streamID );	   // Constructor with four 32-bit seeds
    void seed(uint64_t seedval){seed_uniquestream( &S, 0, 0, (myID_t)(seedval>>32), (myID_t)seedval );} // seed with one 64-bit seed
    mixmax_engine(); // Constructor, no seeds
    
    mixmax_engine& operator=(const mixmax_engine& other );
    
inline std::uint64_t operator()()
    {
        return get_next();
    }
    
private:
    myuint MOD_MULSPEC(myuint k);
    void seed_vielbein(rng_state_t* X, unsigned int i); // seeds with the i-th unit vector, i = 0..N-1,  for testing only
    myuint iterate_raw_vec(myuint* Y, myuint sumtotOld);
    myuint apply_bigskip(myuint* Vout, myuint* Vin, myID_t clusterID, myID_t machineID, myID_t runID, myID_t  streamID );
    myuint modadd(myuint foo, myuint bar);
    myuint fmodmulM61(myuint cum, myuint s, myuint a);
#if defined(__x86_64__)
    inline myuint mod128(__uint128_t s);
#endif
};


#define ARRAY_INDEX_OUT_OF_BOUNDS   0xFF01
#define SEED_WAS_ZERO               0xFF02
#define ERROR_READING_STATE_FILE    0xFF03
#define ERROR_READING_STATE_COUNTER       0xFF04
#define ERROR_READING_STATE_CHECKSUM      0xFF05

#define MOD_PAYNE(k) ((((k)) & MERSBASE) + (((k)) >> BITS) )
#define MOD_MERSENNE(k) MOD_PAYNE(k)

#define PREF 
#define POST
//#define PREF template <int Ndim> // TEMPLATE
//#define POST <Ndim>              // TEMPLATE

PREF myuint mixmax_engine POST::MOD_MULSPEC(myuint k){
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
            break;
    }
}

PREF mixmax_engine POST ::mixmax_engine()
// constructor, with no params, fast and seeds with a unit vector
{
    seed_vielbein(&S,0);
}

PREF mixmax_engine POST ::mixmax_engine(myID_t clusterID, myID_t machineID, myID_t runID, myID_t  streamID)
// constructor, no need to allocate, just seed
{
    seed_uniquestream( &S, clusterID,  machineID,  runID,  streamID );
}

#define MULWU(k) (( (k)<<(SPECIALMUL) & M61) | ( (k) >> (BITS-SPECIALMUL))  )

PREF myuint mixmax_engine POST ::iterate_raw_vec(myuint* Y, myuint sumtotOld){
    // operates with a raw vector, uses known sum of elements of Y
    int i;
    
    myuint temp2 = Y[1];
    
    
    myuint  tempP, tempV;
    Y[0] = ( tempV = sumtotOld);
    myuint sumtot = Y[0], ovflow = 0; // will keep a running sum of all new elements
    tempP = 0;              // will keep a partial sum of all old elements
    for (i=1; i<N; i++){
        if (SPECIALMUL!=0){
            myuint tempPO = MULWU(tempP);
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

PREF myuint mixmax_engine POST ::get_next() {
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

PREF double mixmax_engine POST ::get_next_float()				// Returns a random double with all 53 bits random, in the range (0,1]
{    /* cast to signed int trick suggested by Andrzej Görlich     */
    int64_t Z=(int64_t)get_next();
    double F;
#if defined(__GNUC__) && (__GNUC__ < 5) && (!defined(__ICC)) && defined(__x86_64__) && defined(__SSE2_MATH__) && defined(USE_INLINE_ASM)
    //#warning Using the inline assembler
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

PREF void mixmax_engine POST ::seed_vielbein(rng_state_t* X, unsigned int index)
{
    //rng_state_t S=&X;
    int i;
    if (index<N){
        for (i=0; i < N; i++){
            X->V[i] = 0;
        }
        X->V[index] = 1;
    }else{
        //fprintf(stderr, "Out of bounds index, is not ( 0 <= index < N  )\n");
        std::cerr << "MIXMAX ERROR: " << ARRAY_INDEX_OUT_OF_BOUNDS << "Out of bounds index, is not ( 0 <= index < N  )\n";
        std::terminate();
    }
    X->counter = N;  // set the counter to N if iteration should happen right away
    X->sumtot = 1;
}


PREF void mixmax_engine POST ::seed_uniquestream( rng_state_t* Xin, myID_t clusterID, myID_t machineID, myID_t runID, myID_t  streamID ){
    seed_vielbein(Xin,0);
    //print_state();
    Xin->sumtot = apply_bigskip(Xin->V.data(), Xin->V.data(),  clusterID,  machineID,  runID,   streamID );
    //   if (Xin->fh==NULL){Xin->fh=stdout;} // if the filehandle is not yet set, make it stdout
    std::cerr << "seeding with: " << clusterID << ", " << machineID << ", " <<   runID <<  ", " <<  streamID << "\n";
    Xin->counter = 1;
}


PREF myuint mixmax_engine POST ::apply_bigskip( myuint* Vout, myuint* Vin, myID_t clusterID, myID_t machineID, myID_t runID, myID_t  streamID ){
    /*
     makes a derived state vector, Vout, from the mother state vector Vin
     by skipping a large number of steps, determined by the given seeding ID's
     
     it is mathematically guaranteed that the substreams derived in this way from the SAME (!!!) Vin will not collide provided
     1) at least one bit of ID is different
     2) less than 10^100 numbers are drawn from the stream
     (this is good enough : a single CPU will not exceed this in the lifetime of the universe, 10^19 sec,
     even if it had a clock cycle of Planch time, 10^44 Hz )
     
     Caution: never apply this to a derived vector, just choose some mother vector Vin, for example the unit vector by seed_vielbein(X,0),
     and use it in all your runs, just change runID to get completely nonoverlapping streams of random numbers on a different day.
     
     clusterID and machineID are provided for the benefit of large organizations who wish to ensure that a simulation
     which is running in parallel on a large number of  clusters and machines will have non-colliding source of random numbers.
     
     did i repeat it enough times? the non-collision guarantee is absolute, not probabilistic
     
     */
    
    
//     const	myuint skipMat240[128][240] =
// #include "mixmax_skip_N240.c"
//     ;
    const	myuint skipMat17[128][17] =
#include "boost/random/detail/mixmax_skip_N17.icc"
    ;
//    const	myuint skipMat8[128][8] =
//#include "mixmax_skip_N8.c"
    ;
    
    const myuint* skipMat[128];
    switch (N) {
//         case 240:
//             for (int i=0; i<128; i++) { skipMat[i] = skipMat240[i];}
//             break;
        case 17:
            for (int i=0; i<128; i++) { skipMat[i] = skipMat17[i];}
            break;
//        case 8:
//            for (int i=0; i<128; i++) { skipMat[i] = skipMat8[i];}
//            break;
            
        default:
            exit(-1);
            break;
    }
    
    myID_t IDvec[4] = {streamID, runID, machineID, clusterID};
    int r,i,j,  IDindex;
    myID_t id;
    myuint Y[N], cum[N];
    myuint coeff;
    myuint* rowPtr;
    myuint sumtot=0;
    
    
    for (i=0; i<N; i++) { Y[i] = Vin[i]; sumtot = modadd( sumtot, Vin[i]); } ;
    for (IDindex=0; IDindex<4; IDindex++) { // go from lower order to higher order ID
        id=IDvec[IDindex];
        //printf("now doing ID at level %d, with ID = %d\n", IDindex, id);
        r = 0;
        while (id){
            if (id & 1) {
                rowPtr = (myuint*)skipMat[r + IDindex*8*sizeof(myID_t)];
                //printf("free coeff for row %d is %llu\n", r, rowPtr[0]);
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
PREF inline myuint mixmax_engine POST ::mod128(__uint128_t s){
    myuint s1;
    s1 = ( (  ((myuint)s)&MERSBASE )    + (  ((myuint)(s>>64)) * 8 )  + ( ((myuint)s) >>BITS) );
    return	MOD_MERSENNE(s1);
}

PREF inline myuint mixmax_engine POST ::fmodmulM61(myuint cum, myuint a, myuint b){
    __uint128_t temp;
    temp = (__uint128_t)a*(__uint128_t)b + cum;
    return mod128(temp);
}

#else // on all other platforms, including 32-bit linux, PPC and PPC64, ARM and all Windows
#define MASK32 0xFFFFFFFFULL

PREF inline myuint mixmax_engine POST ::fmodmulM61(myuint cum, myuint s, myuint a)
{
    register myuint o,ph,pl,ah,al;
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

PREF myuint mixmax_engine POST ::modadd(myuint foo, myuint bar){
#if (defined(__x86_64__) || defined(__i386__)) &&  defined(__GNUC__) && defined(USE_INLINE_ASM)
    //#warning Using assembler routine in modadd
    myuint out;
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

PREF void mixmax_engine POST ::print_state(){ // (std::ostream& ost){
    int j;
    fprintf(stdout, "mixmax state, file version 1.0\n" );
    fprintf(stdout, "N=%u; V[N]={", rng_get_N() );
    for (j=0; (j< (rng_get_N()-1) ); j++) {
        fprintf(stdout, "%llu, ", S.V[j] );
    }
    fprintf(stdout, "%llu", S.V[rng_get_N()-1] );
    fprintf(stdout, "}; " );
    fprintf(stdout, "counter=%u; ", S.counter );
    fprintf(stdout, "sumtot=%llu;\n", S.sumtot );
}

PREF mixmax_engine POST mixmax_engine POST ::Branch(){
    S.sumtot = iterate_raw_vec(S.V.data(), S.sumtot); S.counter = N-1;
    mixmax_engine tmp=*this;
    tmp.BranchInplace();
    return tmp;
}

PREF mixmax_engine POST & mixmax_engine POST ::operator=(const mixmax_engine& other ){
    S.V = other.S.V;
    S.sumtot = other.S.sumtot;
    S.counter = other.S.counter;
    return *this;
}

PREF void mixmax_engine POST ::BranchInplace(){
    // Dont forget to iterate the mother, when branching the daughter, or else will have collisions!
    // a 64-bit LCG from Knuth line 26, is used to mangle a vector component
    constexpr myuint MULT64=6364136223846793005ULL;
    myuint tmp=S.V[1];
    S.V[1] *= MULT64; S.V[1] &= MERSBASE;
    S.sumtot = modadd( S.sumtot , S.V[1] - tmp + MERSBASE);
    S.sumtot = iterate_raw_vec(S.V.data(), S.sumtot);// printf("iterating!\n");
    S.counter = 1;
}

//template class mixmax_engine<240>;// TEMPLATE
//template class mixmax_engine<17>;// TEMPLATE
typedef mixmax_engine mixmax;
}
}

#endif // BOOST_RANDOM_MIXMAX_HPP
