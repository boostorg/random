/* boost random/generalized_inverse_gaussian_distribution.hpp header file
 * 
 * Copyright Young Geun Kim 2025
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 * 
 * See http://www.boost.org for most recent version including documentation.
 * 
 * $Id$
 */

#ifndef BOOST_GENERALIZED_RANDOM_INVERSE_GAUSSIAN_DISTRIBUTION_HPP
#define BOOST_GENERALIZED_RANDOM_INVERSE_GAUSSIAN_DISTRIBUTION_HPP

#include <boost/config/no_tr1/cmath.hpp>
#include <istream>
#include <iosfwd>
#include <limits>
#include <boost/assert.hpp>
#include <boost/limits.hpp>
#include <boost/random/detail/config.hpp>
#include <boost/random/detail/operators.hpp>
#include <boost/random/uniform_01.hpp>

namespace boost {
namespace random {

/**
 * The generalized inverse gaussian distribution is a real-valued distribution with
 * three parameters p, a, and b. It produced values > 0.
 * 
 * It has
 * \f$\displaystyle p(x) = \frac{(a / b)^{p / 2}}{2 K_{p}(\sqrt{a b})} x^{p - 1} e^{-(a x + b / 2) / 2}\f$.
 * 
 * The algorithm used is from
 * 
 * @blockquote
 * "Random variate generation for the generalized inverse Gaussian distribution",
 * Luc Devroye,
 * Statistics and Computing,
 * Volume 24, 2014, Pages 236 - 246
 * @endblockquote
 */
template<class RealType = double>
class generalized_inverse_gaussian_distribution
{
public:
	typedef RealType result_type;
	typedef RealType input_type;

	class param_type {
	public:
		typedef generalized_inverse_gaussian_distribution distribution_type;

		/**
		 * Constructs a @c param_type object from the "p", "a", and "b"
		 * parameters.
		 *
		 * Requires:
			 * a > 0 && b >= 0 if p > 0,
			 * a > 0 && b > 0 if p == 0,
			 * a >= 0 && b > 0 if p < 0
		 */
		explicit param_type(RealType p_arg = RealType(1.0),
		                   	RealType a_arg = RealType(1.0),
							RealType b_arg = RealType(1.0))
		: _p(p_arg), _a(a_arg), _b(b_arg)
		{
			BOOST_ASSERT(
				(p_arg > RealType(0) && a_arg > RealType(0) && b_arg >= RealType(0)) ||
				(p_arg == RealType(0) && a_arg > RealType(0) && b_arg > RealType(0)) ||
				(p_arg < RealType(0) && a_arg >= RealType(0) && b_arg > RealType(0))
			);
		}

		/** Returns the "p" parameter of the distribution. */
		RealType p() const { return _p; }
		/** Returns the "a" parameter of the distribution. */
		RealType a() const { return _a; }
		/** Returns the "b" parameter of the distribution. */
		RealType b() const { return _b; }

		/** Writes a @c param_type to a @c std::ostream. */
		BOOST_RANDOM_DETAIL_OSTREAM_OPERATOR(os, param_type, parm)
		{
			os << parm._p << ' ' << parm._a << ' ' << parm._b;
			return os;
		}

		/** Reads a @c param_type from a @c std::istream. */
		BOOST_RANDOM_DETAIL_ISTREAM_OPERATOR(is, param_type, parm)
		{
			is >> parm._p >> std::ws >> parm._a >> std::ws >> parm._b;
			return is;
		}

		/** Returns true if the two sets of parameters are the same. */
		BOOST_RANDOM_DETAIL_EQUALITY_OPERATOR(param_type, lhs, rhs)
		{ return lhs._p == rhs._p && lhs._a == rhs._a && lhs._b == rhs._b; }

		/** Returns true if the two sets fo parameters are different. */
		BOOST_RANDOM_DETAIL_INEQUALITY_OPERATOR(param_type)

	private:
		RealType _p;
		RealType _a;
		RealType _b;
	};

#ifndef BOOST_NO_LIMITS_COMPILE_TIME_CONSTANTS
	BOOST_STATIC_ASSERT(!std::numeric_limits<RealType>::is_integer);
#endif

	/**
	 * Constructs an @c generalized_inverse_gaussian_distribution from its "alpha" and "beta" parameters.
	 *
	 * Requires: alpha > 0, beta > 0
	 */
	explicit generalized_inverse_gaussian_distribution(RealType p_arg = RealType(1.0),
                       								   RealType a_arg = RealType(1.0),
													   RealType b_arg = RealType(1.0))
    : _p(p_arg), _a(a_arg), _b(b_arg)
	{
		BOOST_ASSERT(
			(p_arg > RealType(0) && a_arg > RealType(0) && b_arg >= RealType(0)) ||
			(p_arg == RealType(0) && a_arg > RealType(0) && b_arg > RealType(0)) ||
			(p_arg < RealType(0) && a_arg >= RealType(0) && b_arg > RealType(0))
		);
		init();
	}
	/** Constructs an @c generalized_inverse_gaussian_distribution from its parameters. */
	explicit generalized_inverse_gaussian_distribution(const param_type& parm)
	: _p(parm.p()), _a(parm.a()), _b(parm.b())
	{
		init();
	}

	/**
	 * Returns a random variate distributed according to the
	 * generalized inverse gaussian distribution.
	 */
	template<class URNG>
	RealType operator()(URNG& urng) const
	{
#ifndef BOOST_NO_STDC_NAMESPACE
		using std::sqrt;
		using std::log;
		using std::min;
		using std::exp;
#endif
		RealType t, s, t_deriv, s_deriv, eta, zeta, theta, xi, u, v, w, p, q, r, cand;
		RealType log_concave = -psi(result_type(1));
		if (log_concave >= result_type(.5) && log_concave <= result_type(2)) {
			t = result_type(1);
		} else if (log_concave > result_type(2)) {
			t = sqrt(result_type(2) / (_alpha + _abs_p));
		} else if (log_concave < result_type(.5)) {
			t = log(result_type(4) / (_alpha + result_type(2) * _abs_p));
		}
		log_concave = -psi(result_type(-1));
		if (log_concave >= result_type(.5) && log_concave <= result_type(2)) {
			s = result_type(1);
		} else if (log_concave > result_type(2)) {
			s = sqrt(result_type(4) / (_alpha * cosh(1) + _abs_p));
		} else if (log_concave < result_type(.5)) {
			s = min(result_type(1) / _abs_p, log(result_type(1) + result_type(1) / _alpha + sqrt(result_type(1) / (_alpha * _alpha) + result_type(2) / _alpha)));
		}
		eta = -psi(t);
		zeta = -psi_deriv(t);
		theta = -psi(-s);
		xi = psi_deriv(-s);
		p = result_type(1) / xi;
		r = result_type(1) / zeta;
		t_deriv = t - r * eta;
		s_deriv = s - p * theta;
		q = t_deriv + s_deriv;
		do
		{
			u = uniform_01<RealType>()(urng);
			v = uniform_01<RealType>()(urng);
			w = uniform_01<RealType>()(urng);
			if (u < q / (p + q + r)) {
				cand = -s_deriv + q * v;
			} else if (u < (q + r) / (p + q + r)) {
				cand = t_deriv - r * log(v);
			} else {
				cand = -s_deriv + p * log(v);
			}
		} while (w * chi(cand, s, t, s_deriv, t_deriv, eta, zeta, theta, xi) > exp(psi(cand)));
		cand = (_abs_p / _omega + sqrt(result_type(1) + _abs_p * _abs_p / (_omega * _omega))) * exp(cand);
		return _p > 0 ? cand * sqrt(_b / _a) : sqrt(_b / _a) / cand;
	}

	/**
	 * Returns a random variate distributed accordint to the beta
	 * distribution with parameters specified by @c param.
	 */
	template<class URNG>
	result_type operator()(URNG& urng, const param_type& parm) const
	{
		return generalized_inverse_gaussian_distribution(parm)(urng);
	}

	/** Returns the "p" parameter of the distribution. */
	RealType p() const { return _p; }
	/** Returns the "a" parameter of the distribution. */
	RealType a() const { return _a; }
	/** Returns the "b" parameter of the distribution. */
	RealType b() const { return _b; }

	/** Returns the smallest value that the distribution can produce. */
	RealType min BOOST_PREVENT_MACRO_SUBSTITUTION () const
	{ return RealType(0.0); }
	/** Returns the largest value that the distribution can produce. */
	RealType max BOOST_PREVENT_MACRO_SUBSTITUTION () const
	{ return (std::numeric_limits<RealType>::infinity)(); }

	/** Returns the parameters of the distribution. */
	param_type param() const { return param_type(_p, _a, _b); }
	/** Sets the parameters of the distribution. */
	void param(const param_type& parm)
	{
		_p = parm.p();
		_a = parm.a();
		_b = parm.b();
		init();
	}

	/**
	 * Effects: Subsequent uses of the distribution do not depend
	 * on values produced by any engine prior to invoking reset.
	 */
	void reset() { }

	/** Writes an @c generalized_inverse_gaussian_distribution to a @c std::ostream. */
	BOOST_RANDOM_DETAIL_OSTREAM_OPERATOR(os, generalized_inverse_gaussian_distribution, wd)
	{
		os << wd.param();
		return os;
	}

	/** Reads an @c generalized_inverse_gaussian_distribution from a @c std::istream. */
	BOOST_RANDOM_DETAIL_ISTREAM_OPERATOR(is, generalized_inverse_gaussian_distribution, wd)
	{
		param_type parm;
		if(is >> parm) {
			wd.param(parm);
		}
		return is;
	}

	/**
	 * Returns true if the two instances of @c generalized_inverse_gaussian_distribution will
	 * return identical sequences of values given equal generators.
	 */
	BOOST_RANDOM_DETAIL_EQUALITY_OPERATOR(generalized_inverse_gaussian_distribution, lhs, rhs)
	{ return lhs._p == rhs._p && lhs._a == rhs._a && lhs._b == rhs._b; }

	/**
	 * Returns true if the two instances of @c generalized_inverse_gaussian_distribution will
	 * return different sequences of values given equal generators.
	 */
	BOOST_RANDOM_DETAIL_INEQUALITY_OPERATOR(generalized_inverse_gaussian_distribution)

private:
	RealType _p;
	RealType _a;
	RealType _b;
	// some data precomputed from the parameters
	RealType _abs_p;
	RealType _omega;
	RealType _alpha;

	/// \cond hide_private_members
	void init()
    {
#ifndef BOOST_NO_STDC_NAMESPACE
		using std::abs;
#endif
        _abs_p = abs(_p);
		_omega = sqrt(_a * _b); // two-parameter representation (p, omega)
		_alpha = sqrt(_omega * _omega + _abs_p * _abs_p) - _abs_p;
    }

	static result_type psi(const RealType& x)
	{
#ifndef BOOST_NO_STDC_NAMESPACE
		using std::sinh;
		using std::exp;
#endif
		return -_alpha * (cosh(x) - result_type(1)) - _abs_p * (exp(x) - x - result_type(1));
	}

	static result_type psi_deriv(const RealType& x)
	{
#ifndef BOOST_NO_STDC_NAMESPACE
		using std::sinh;
		using std::exp;
#endif
		return -_alpha * sinh(x) - _abs_p * (exp(x) - result_type(1));
	}

	static result_type chi(const RealType& x, 
						   const RealType& s, const RealType& t,
						   const RealType& s_deriv, const RealType& t_deriv,
						   const RealType& eta, const RealType& zeta, const RealType& theta, const RealType& xi)
	{
#ifndef BOOST_NO_STDC_NAMESPACE
		using std::exp;
#endif
		if (x >= -s_deriv && x <= t_deriv) {
			return result_type(1);
		} else if (x > t_deriv) {
			return exp(-eta - zeta * (x - t));
		}
		return exp(-theta + xi * (x + s));
	}
	/// \endcond
};

} // namespace random

using random::generalized_inverse_gaussian_distribution;

} // namespace boost

#endif // BOOST_GENERALIZED_RANDOM_INVERSE_GAUSSIAN_DISTRIBUTION_HPP