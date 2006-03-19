// Copyright Daniel Wallin 2006. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/preprocessor/iteration/local.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/stringize.hpp>

#include <boost/python.hpp>
#include <boost/bind/apply.hpp>

#include <boost/random/sprng.hpp>
#include <boost/random/buffered_uniform_01.hpp>
#include <boost/random/buffered_generator.hpp>

// Generators
#include <boost/random/linear_congruential.hpp>
#include <boost/random/mersenne_twister.hpp>

#include <boost/random/variate_generator.hpp>

#include <boost/random.hpp>

#include <boost/parameter/python.hpp>

#include <boost/utility/base_from_member.hpp>

using namespace boost::python;
namespace mpl = boost::mpl;

struct seed_fwd
{
#define BOOST_PP_LOCAL_MACRO(n) \
    template <class R, class T BOOST_PP_ENUM_TRAILING_PARAMS(n, class A)> \
    R operator()(boost::type<R>, T& self BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(n, A, const& a)) \
    { \
        return self.seed(BOOST_PP_ENUM_PARAMS(n, a)); \
    }

#define BOOST_PP_LOCAL_LIMITS (0, 4)
#include BOOST_PP_LOCAL_ITERATE()
};

struct sprng_visitor : def_visitor<sprng_visitor>
{
    typedef mpl::vector4<
        boost::random::tag::stream_number*
      , boost::random::tag::total_streams*
      , boost::random::tag::global_seed*
      , boost::random::tag::parameter*
    > keywords;

    template <class C>
    void visit(C& cl) const
    {
        typedef typename C::wrapped_type rng;

        namespace py = boost::parameter::python;

        cl
            .def(
                py::init<
                    keywords
                  , mpl::vector4<int,int,int,int> 
                >()
            )
            .def("seed",
                py::function<
                    seed_fwd
                  , keywords
                  , mpl::vector5<void,int,int,int,int> 
                >()
            );
    }
};

template <class Distribution>
struct variate_generator_class
  : class_<
        boost::variate_generator<
            boost::buffered_uniform_01<>&, Distribution
        >
    >
{
    typedef boost::variate_generator<
        boost::buffered_uniform_01<>&, Distribution
    > generator;

    typedef class_<generator> base;

    variate_generator_class(char const* name)
      : base(name, init<boost::buffered_uniform_01<>&, Distribution&>())
    {
        converter::registration const* r = converter::registry::query(type_id<Distribution>());
        assert(r);
        object distribution_class(handle<>(r->get_class_object()));

        dict current(scope().attr("__dict__"));

        if (!current.has_key("distribution_variate_map"))
            scope().attr("distribution_variate_map") = dict();

        scope().attr("distribution_variate_map")[distribution_class] = *this;

        this->def("__call__",
            make_function(
                boost::apply<typename generator::result_type>()
              , default_call_policies()
              , mpl::vector2<typename generator::result_type, generator&>()
            )
        );
    }
};

template <class Engine>
struct rng_wrapper
  : boost::base_from_member<Engine>
  , boost::basic_buffered_uniform_01<Engine&>
{
    typedef boost::base_from_member<Engine> member_base;
    typedef boost::basic_buffered_uniform_01<Engine&> buffered_base;

    rng_wrapper()
      : buffered_base(this->member)
    {}

#define BOOST_PP_LOCAL_MACRO(n) \
    template <BOOST_PP_ENUM_PARAMS(n, class A)> \
    rng_wrapper(BOOST_PP_ENUM_BINARY_PARAMS(n, A, a)) \
      : member_base(BOOST_PP_ENUM_PARAMS(n, a)) \
      , buffered_base(this->member) \
    {}

#define BOOST_PP_LOCAL_LIMITS (1, 5)
#include BOOST_PP_LOCAL_ITERATE()
    
    void seed()
    {
        this->member.seed();
        this->reset();
    }

#define BOOST_PP_LOCAL_MACRO(n) \
    template <BOOST_PP_ENUM_PARAMS(n, class A)> \
    void seed(BOOST_PP_ENUM_BINARY_PARAMS(n, A, a)) \
    { \
        this->member.seed(BOOST_PP_ENUM_PARAMS(n, a)); \
        this->reset(); \
    }

#define BOOST_PP_LOCAL_LIMITS (1, 5)
#include BOOST_PP_LOCAL_ITERATE()

};

template <class Rng>
struct buffered_uniform_01_class
  : class_<
        rng_wrapper<Rng>, bases<boost::buffered_uniform_01<> > 
    >
{
    typedef class_<
        rng_wrapper<Rng>, bases<boost::buffered_uniform_01<> > 
    > base;

    buffered_uniform_01_class(char const* name)
      : base(name)
    {
        this->def("seed", (void(rng_wrapper<Rng>::*)())&rng_wrapper<Rng>::seed);
    }
};

BOOST_PYTHON_MODULE(_random)
{
    typedef boost::buffered_uniform_01<boost::mt11213b> rng;

    class_<boost::buffered_generator<double>, boost::noncopyable>("buffered_generator", no_init)
        .def("__call__",
            make_function(
                boost::apply<boost::buffered_generator<double>::result_type>()
              , default_call_policies()
              , mpl::vector2<boost::buffered_generator<double>::result_type, boost::buffered_generator<double>&>()
            )
        );

    class_<
        boost::buffered_uniform_01<>, bases<boost::buffered_generator<double> >, boost::noncopyable
    >("buffered_uniform_01", no_init);

#define RNG_CLASSES \
    (minstd_rand0)(minstd_rand) \
    (rand48) \
    (ecuyer1988) \
    (kreutzer1986) \
    (hellekalek1995) \
    (mt11213b)(mt19937) \
    (lagged_fibonacci607)(lagged_fibonacci1279)(lagged_fibonacci2281) \
    (lagged_fibonacci3217)(lagged_fibonacci4423)(lagged_fibonacci9689) \
    (lagged_fibonacci19937)(lagged_fibonacci23209)(lagged_fibonacci44497)

#define MAKE_PYTHON_CLASS(r, _, rng) \
    buffered_uniform_01_class<boost::rng>(BOOST_PP_STRINGIZE(rng) "_01");

    BOOST_PP_SEQ_FOR_EACH(MAKE_PYTHON_CLASS, ~, RNG_CLASSES)

#undef MAKE_PYTHON_CLASS
#undef RNG_CLASSES

    class_<boost::uniform_int<> >("uniform_int", init<optional<int,int> >());
    class_<boost::bernoulli_distribution<> >("bernoulli_distribution", init<optional<double> >());
    class_<boost::geometric_distribution<> >("geometric_distribution", init<optional<double> >());
    class_<boost::triangle_distribution<> >("triangle_distribution", init<optional<double,double,double> >());
    class_<boost::exponential_distribution<> >("exponential_distribution", init<optional<double> >());
    class_<boost::normal_distribution<> >("normal_distribution", init<optional<double,double> >());
    class_<boost::lognormal_distribution<> >("lognormal_distribution", init<optional<double,double> >());

    variate_generator_class<boost::uniform_int<> >("uniform_int_variate");
    variate_generator_class<boost::bernoulli_distribution<> >("bernoulli_distribution_variate");
    variate_generator_class<boost::geometric_distribution<> >("geometric_distribution_variate");
    variate_generator_class<boost::triangle_distribution<> >("triangle_distribution_variate");
    variate_generator_class<boost::exponential_distribution<> >("exponential_distribution_variate");
    variate_generator_class<boost::normal_distribution<> >("normal_distribution_variate");
    variate_generator_class<boost::lognormal_distribution<> >("lognormal_distribution_variate");

#define SPRNG_CLASSES \
    (cmrg)(lcg)(lcg64)(lfg)(mlfg)
    //(pmlcg)

#define MAKE_PYTHON_CLASS(r, _, rng) \
    buffered_uniform_01_class<boost::random::sprng::rng>(BOOST_PP_STRINGIZE(rng) "_01") \
        .def(sprng_visitor());

    BOOST_PP_SEQ_FOR_EACH(MAKE_PYTHON_CLASS, ~, SPRNG_CLASSES)

#undef MAKE_PYTHON_CLASS
#undef SPRNG_CLASSES

}

