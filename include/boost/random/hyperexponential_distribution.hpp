/* boost random/hyperexponential_distribution.hpp header file
 *
 * Copyright Marco Guazzone 2014
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org for most recent version including documentation.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef BOOST_RANDOM_HYPEREXPONENTIAL_DISTRIBUTION_HPP
#define BOOST_RANDOM_HYPEREXPONENTIAL_DISTRIBUTION_HPP


#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/math/tools/precision.hpp>
#include <boost/random/detail/operators.hpp>
#include <boost/random/detail/vector_io.hpp>
#include <boost/random/discrete_distribution.hpp>
#include <boost/random/exponential_distribution.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <cassert>
#include <cstddef>
#ifndef BOOST_NO_CXX11_HDR_INITIALIZER_LIST
# include <initializer_list>
#endif // BOOST_NO_CXX11_HDR_INITIALIZER_LIST
#include <iostream>
#include <limits>
#include <vector>


namespace boost { namespace random {

namespace /*<unnamed>>*/ { namespace hyperexp_detail {

template <typename T>
std::vector<T>& normalize(std::vector<T>& v)
{
    const T sum = std::accumulate(v.begin(), v.end(), static_cast<T>(0));

    const typename std::vector<T>::iterator end = v.end();
    for (typename std::vector<T>::iterator it = v.begin();
         it != end;
         ++it)
    {
        *it /= sum;
    }

    return v;
}

template <typename T>
bool iszero(T x)
{
#ifdef FP_ZERO
    return (boost::math::fpclassify)(x) == FP_ZERO;
    // Alternatively, we could use std::fpclassify (but this is available from ISO C99)
    //return std::fpclassify(x) == FP_ZERO;
#else
    return ((x < 0) ? bool(-x < (std::numeric_limits<T>::min)())
                    : bool(+x < (std::numeric_limits<T>::min)()));
#endif // FP_ZERO
}

template <typename RealT>
bool check_probabilities(std::vector<RealT> const& probabilities)
{
    const std::size_t n = probabilities.size();
    RealT sum = 0;
    for (std::size_t i = 0; i < n; ++i)
    {
        if (probabilities[i] < 0
            || probabilities[i] > 1
            || !(boost::math::isfinite)(probabilities[i]))
        {
            return false;
        }
        sum += probabilities[i];
    }

    if (!iszero(sum-RealT(1)))
    {
        return false;
    }

    return true;
}

template <typename RealT>
bool check_rates(std::vector<RealT> const& rates)
{
    const std::size_t n = rates.size();
    for (std::size_t i = 0; i < n; ++i)
    {
        if (rates[i] <= 0
            || !(boost::math::isfinite)(rates[i]))
        {
            return false;
        }
    }
    return true;
}

template <typename RealT>
bool check_params(std::vector<RealT> const& probabilities, std::vector<RealT> const& rates)
{
    if (probabilities.size() != rates.size())
    {
        return false;
    }

    return check_probabilities(probabilities)
           && check_rates(rates);
}

}} // Namespace <unnamed>::hyperexp_detail


/**
 * The hyperexponential distribution is a real-valued continuous distribution
 * with two parameters, the <em>phase probability vector</em> \c probs and the
 * <em>rate vector</em> \c rates.
 *
 * A \f$k\f$-phase hyperexponential distribution is a mixture of \f$k\f$
 * exponential distributions.
 * For this reason, it is also referred to as <em>mixed exponential
 * distribution</em> or <em>parallel \f$k\f$-phase exponential
 * distribution</em>.
 *
 * A \f$k\f$-phase hyperexponential distribution has a probability density
 * function
 * \f[
 *  f(x) = \sum_{i=1}^k \alpha_i \lambda_i e^{-x\lambda_i}
 * \f]
 * where:
 * - \f$k\f$ is the <em>number of phases</em> and also the size of the input
 *   vector parameters,
 * - \f$\mathbf{\alpha}=(\alpha_1,\ldots,\alpha_k)\f$ is the <em>phase probability
 *   vector</em> parameter, and
 * - \f$\mathbf{\lambda}=(\lambda_1,\ldots,\lambda_k)\f$ is the <em>rate vector</em>
 *   parameter.
 * .
 *
 * References:
 * -# H.T. Papadopolous, C. Heavey and J. Browne, Queueing Theory in Manufacturing Systems Analysis and Design, Chapman & Hall/CRC, 1993, p. 35.
 * -# A. Feldmann and W. Whitt, Fitting mixtures of exponentials to long-tail distributions to analyze network performance models, Performance Evaluation 31(3-4):245, doi:10.1016/S0166-5316(97)00003-5, 1998.
 * -# Wikipedia, Hyperexponential Distribution, 2014, Online: http://en.wikipedia.org/wiki/Hyperexponential_distribution
 * -# Wolfram Mathematica, Hyperexponential Distribution, 2014, Online: http://reference.wolfram.com/language/ref/HyperexponentialDistribution.html
 * .
 */
template<class RealT = double>
class hyperexponential_distribution
{
    public: typedef RealT result_type;
    public: typedef RealT input_type;


    public: class param_type
    {
        public: typedef hyperexponential_distribution distribution_type;

        /**
         * Constructs a @c param_type with the default parameters
         * of the distribution.
         */
        public: param_type()
        : probs_(1, 1),
          rates_(1, 1)
        {
        }

        /**
         * Constructs a @c param_type from the "probs" and "rates" parameters
         * of the distribution.
         */
        public: template <typename ProbIterT, typename RateIterT>
                param_type(ProbIterT prob_first, ProbIterT prob_last,
                           RateIterT rate_first, RateIterT rate_last)
        : probs_(prob_first, prob_last),
          rates_(rate_first, rate_last)
        {
            hyperexp_detail::normalize(probs_);

            assert( hyperexp_detail::check_params(probs_, rates_) );
        }

        /**
         * Constructs a @c param_type from the "probs" and "rates" parameters
         * of the distribution.
         */
        public: template <typename ProbRangeT, typename RateRangeT>
                param_type(ProbRangeT const& prob_range, RateRangeT const& rate_range)
        : probs_(boost::begin(prob_range), boost::end(prob_range)),
          rates_(boost::begin(rate_range), boost::end(rate_range))
        {
            hyperexp_detail::normalize(probs_);

            assert( hyperexp_detail::check_params(probs_, rates_) );
        }

#ifndef BOOST_NO_CXX11_HDR_INITIALIZER_LIST
        public: template <typename ArgT>
                param_type(std::initializer_list<ArgT> l1, std::initializer_list<ArgT> l2)
        : probs_(l1.begin(), l1.end()),
          rates_(l2.begin(), l2.end())
        {
            assert( hyperexp_detail::check_params(dd_.probabilities(), rates_) );
        }
#endif

        /** Returns the "probs" parameter of the distribtuion. */
        public: std::vector<RealT> probabilities() const
        {
            return probs_;
        }

        /** Returns the "rates" parameter of the distribution. */
        public: std::vector<RealT> rates() const
        {
            return rates_;
        }

        /** Writes a @c param_type to a @c std::ostream. */
        public: BOOST_RANDOM_DETAIL_OSTREAM_OPERATOR(os, param_type, parm)
        {
            detail::print_vector(os, parm.probs_);
            os << ' ';
            detail::print_vector(os, parm.rates_);

            return os;
        }

        /** Reads a @c param_type from a @c std::istream. */
        public: BOOST_RANDOM_DETAIL_ISTREAM_OPERATOR(is, param_type, parm)
        {
            std::vector<RealT> tmp;

            detail::read_vector(is, tmp);
            if (!is)
            {
                return is;
            }
            parm.probs_.swap(tmp);
            hyperexp_detail::normalize(parm.probs_);

            is >> std::ws;

            tmp.clear();
            detail::read_vector(is, tmp);
            if (!is)
            {
                return is;
            }
            parm.rates_.swap(tmp);

            //pre: vector size conformance
            assert(parm.probs_.size() == parm.rates_.size());

            return is;
        }

        /** Returns true if the two sets of parameters are the same. */
        public: BOOST_RANDOM_DETAIL_EQUALITY_OPERATOR(param_type, lhs, rhs)
        {
            return lhs.probs_ == rhs.probs_
                   && lhs.rates_ == rhs.rates_;
        }
        
        /** Returns true if the two sets of parameters are the different. */
        public: BOOST_RANDOM_DETAIL_INEQUALITY_OPERATOR(param_type)


        private: std::vector<RealT> probs_;
        private: std::vector<RealT> rates_;
    }; // param_type


    /**
     * Constructs a @c hyperexponential_distribution with default parameters.
     */
    public: hyperexponential_distribution()
    : dd_(std::vector<RealT>(1, 1)),
      rates_(1, 1)
    {
        // empty
    }

    /**
     * Constructs a @c hyperexponential_distribution from its "probs" and "rates" parameters.
     */
    public: template <typename ProbIterT, typename RateIterT>
            hyperexponential_distribution(ProbIterT prob_first, ProbIterT prob_last, RateIterT rate_first, RateIterT rate_last)
    : dd_(prob_first, prob_last),
      rates_(rate_first, rate_last)
    {
        assert( hyperexp_detail::check_params(dd_.probabilities(), rates_) );
    }

    /**
     * Constructs a @c hyperexponential_distribution from its "probs" and "rates" parameters.
     */
    public: template <typename ProbRangeT, typename RateRangeT>
            hyperexponential_distribution(ProbRangeT prob_range, RateRangeT const& rate_range)
    : dd_(prob_range),
      rates_(boost::begin(rate_range), boost::end(rate_range))
    {
        assert( hyperexp_detail::check_params(dd_.probabilities(), rates_) );
    }

    /** Constructs a @c hyperexponential_distribution from its parameters. */
    public: explicit hyperexponential_distribution(param_type const& parm)
    //: dd_(typename boost::random::discrete_distribution<int,RealT>::param_type(parm.probabilities())),
    : dd_(parm.probabilities()),
      rates_(parm.rates())
    {
        assert( hyperexp_detail::check_params(dd_.probabilities(), rates_) );
    }

#ifndef BOOST_NO_CXX11_HDR_INITIALIZER_LIST
    public: template <typename ArgT>
            hyperexponential_distribution(std::initializer_list<ArgT> const& l1, std::initializer_list<ArgT> const& l2)
    : dd_(l1.begin(), l1.end()),
      rates_(l2.begin(), l2.end())
    {
        assert( hyperexp_detail::check_params(dd_.probabilities(), rates_) );
    }
#endif

    /**
     * Returns a random variate distributed according to the
     * hyper-exponential distribution.
     */
    public: template<class URNG>
            RealT operator()(URNG& urng) const
    {
        const int i = dd_(urng);

        return boost::random::exponential_distribution<RealT>(rates_[i])(urng);
    }

    /**
     * Returns a random variate distributed accordint to the hyper-exponential
     * distribution with parameters specified by @c param.
     */
    public: template<class URNG>
            RealT operator()(URNG& urng, const param_type& parm) const
    {
        return hyperexponential_distribution(parm)(urng);
    }

    /** Returns the number of phases of the distribution. */
    public: std::size_t num_phases() const
    {
        return rates_.size();
    }

    /** Returns the "probs" parameter of the distribution. */
    public: std::vector<RealT> probabilities() const
    {
        return dd_.probabilities();
    }

    /** Returns the "rates" parameter of the distribution. */
    public: std::vector<RealT> rates() const
    {
        return rates_;
    }

    /** Returns the smallest value that the distribution can produce. */
    public: RealT min BOOST_PREVENT_MACRO_SUBSTITUTION () const
    {
        return 0;
    }

    /** Returns the largest value that the distribution can produce. */
    public: RealT max BOOST_PREVENT_MACRO_SUBSTITUTION () const
    {
        return std::numeric_limits<RealT>::infinity();
    }

    /** Returns the parameters of the distribution. */
    public: param_type param() const
    {
        std::vector<RealT> probs = dd_.probabilities();

        return param_type(probs.begin(), probs.end(), rates_.begin(), rates_.end());
    }

    /** Sets the parameters of the distribution. */
    public: void param(param_type const& parm)
    {
        dd_.param(typename boost::random::discrete_distribution<int,RealT>::param_type(parm.probabilities()));
        rates_ = parm.rates();
    }

    /**
     * Effects: Subsequent uses of the distribution do not depend
     * on values produced by any engine prior to invoking reset.
     */
    public: void reset()
    {
        // empty
    }

    /** Writes an @c hyperexponential_distribution to a @c std::ostream. */
    public: BOOST_RANDOM_DETAIL_OSTREAM_OPERATOR(os, hyperexponential_distribution, hd)
    {
        os << hd.param();
        return os;
    }

    /** Reads an @c hyperexponential_distribution from a @c std::istream. */
    public: BOOST_RANDOM_DETAIL_ISTREAM_OPERATOR(is, hyperexponential_distribution, hd)
    {
        param_type parm;
        if(is >> parm)
        {
            hd.param(parm);
        }
        return is;
    }

    /**
     * Returns true if the two instances of @c hyperexponential_distribution will
     * return identical sequences of values given equal generators.
     */
    public: BOOST_RANDOM_DETAIL_EQUALITY_OPERATOR(hyperexponential_distribution, lhs, rhs)
    {
        return lhs.dd_ == rhs.dd_
               && lhs.rates_ == rhs.rates_;
    }
    
    /**
     * Returns true if the two instances of @c hyperexponential_distribution will
     * return different sequences of values given equal generators.
     */
    public: BOOST_RANDOM_DETAIL_INEQUALITY_OPERATOR(hyperexponential_distribution)


    private: boost::random::discrete_distribution<int,RealT> dd_;
    //private: std::vector<RealT> probs_;
    private: std::vector<RealT> rates_;
}; // hyperexponential_distribution

}} // namespace boost::random


#endif // BOOST_RANDOM_HYPEREXPONENTIAL_DISTRIBUTION_HPP
