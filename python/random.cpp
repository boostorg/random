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
#include <boost/python/stl_iterator.hpp>
#include <boost/bind/apply.hpp>

//#include <boost/random/parallel.hpp>
#include <boost/random/buffered_uniform_01.hpp>
#include <boost/random/buffered_generator.hpp>

// Generators
#include <boost/random/linear_congruential.hpp>
#include <boost/random/mersenne_twister.hpp>

#include <boost/random/variate_generator.hpp>

#include <boost/random.hpp>

#include <boost/parameter/python.hpp>

#include <boost/utility/base_from_member.hpp>

#include <boost/random/multivariate_normal_distribution.hpp>

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
/*
struct sprng_visitor : def_visitor<sprng_visitor>
{
    typedef mpl::vector4<
        boost::random::random_tag::stream_number*
      , boost::random::random_tag::total_streams*
      , boost::random::random_tag::global_seed*
      , boost::random::random_tag::parameter*
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
*/
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

template <class Distribution>
struct distribution_class
  : class_<Distribution>
{
    static typename Distribution::result_type call(
        Distribution& d, boost::buffered_uniform_01<>& rng
    )
    {
        return d(rng);
    }

    template <class Init>
    distribution_class(char const* name, Init init)
      : class_<Distribution>(name, init)
    {
        this->def("reset", &Distribution::reset);
//        this->def("__call__", &call);
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

    rng_wrapper(rng_wrapper<Engine> const& other)
      : member_base(other.member)
      , buffered_base(this->member)
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

template <class R, class A0>
R(*unary_function(R(*f)(A0)))(A0)
{
    return f;
}

boost::multivariate_normal_distribution<>* 
  make_multivariate_normal_distribution(object const& c, object const& m)
{
    Py_ssize_t size = len(m);

    if (len(c) != size)
    {
        PyErr_SetString(PyExc_IndexError, "cholesky matrix must be square with the same size as the mean vector");
    }

    boost::multivariate_normal_distribution<>::matrix_type cholesky(size,size);
    boost::multivariate_normal_distribution<>::matrix_type::array_type::iterator out(
        cholesky.data().begin());

    for (stl_input_iterator<object> i(c); i != stl_input_iterator<object>(); ++i)
    {
        object inner(*i);

        if (len(inner) != size)
        {
            PyErr_SetString(PyExc_IndexError, "cholesky matrix must be square with the same size as the mean vector");
        }

        out = std::copy(stl_input_iterator<double>(inner), stl_input_iterator<double>(), out);
    }

    boost::multivariate_normal_distribution<>::vector_type mean(size);
    std::copy(stl_input_iterator<double>(m), stl_input_iterator<double>(), mean.begin());

    return new boost::multivariate_normal_distribution<>(cholesky, mean);
}

BOOST_PYTHON_MODULE(_boost_random)
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

    buffered_uniform_01_class<boost::minstd_rand0>("minstd_rand0")
        .def(init<boost::minstd_rand0::result_type>(arg("x0")))
        .def(
            "seed"
          , (void(rng_wrapper<boost::minstd_rand0>::*)(boost::minstd_rand0::result_type))
              &rng_wrapper<boost::minstd_rand0>::seed
          , arg("x0")
        );

    buffered_uniform_01_class<boost::minstd_rand>("minstd_rand")
        .def(init<boost::minstd_rand::result_type>(arg("x0")))
        .def(
            "seed"
          , (void(rng_wrapper<boost::minstd_rand>::*)(boost::minstd_rand::result_type))
              &rng_wrapper<boost::minstd_rand>::seed
          , arg("x0")
        );

    buffered_uniform_01_class<boost::ecuyer1988>("ecuyer1988")
        .def(init<boost::ecuyer1988::result_type, boost::ecuyer1988::result_type>( (arg("x0"), arg("x1")) ))
        .def(
            "seed"
          , (void(rng_wrapper<boost::ecuyer1988>::*)(boost::ecuyer1988::result_type, boost::ecuyer1988::result_type))
              &rng_wrapper<boost::ecuyer1988>::seed
          , (arg("x0"), arg("x1"))
        );

    buffered_uniform_01_class<boost::kreutzer1986>("kreutzer1986")
        .def(init<boost::kreutzer1986::result_type>(arg("s")))
        .def(
            "seed"
          , (void(rng_wrapper<boost::kreutzer1986>::*)(boost::kreutzer1986::result_type))
              &rng_wrapper<boost::kreutzer1986>::seed
          , arg("s")
        );

    buffered_uniform_01_class<boost::hellekalek1995>("hellekalek1995")
        .def(init<boost::hellekalek1995::result_type>(arg("y0")))
        .def(
            "seed"
          , (void(rng_wrapper<boost::hellekalek1995>::*)(boost::hellekalek1995::result_type))
              &rng_wrapper<boost::hellekalek1995>::seed
          , arg("y0")
        );

    buffered_uniform_01_class<boost::mt11213b>("mt11213b")
        .def(init<boost::mt11213b::result_type>(arg("value")))
        .def(
            "seed"
          , (void(rng_wrapper<boost::mt11213b>::*)(boost::mt11213b::result_type))
              &rng_wrapper<boost::mt11213b>::seed
          , arg("value")
        );

    buffered_uniform_01_class<boost::mt19937>("mt19937")
        .def(init<boost::mt19937::result_type>(arg("value")))
        .def(
            "seed"
          , (void(rng_wrapper<boost::mt19937>::*)(boost::mt19937::result_type))
              &rng_wrapper<boost::mt19937>::seed
          , arg("value")
        );

    buffered_uniform_01_class<boost::lagged_fibonacci607>("lagged_fibonacci607")
        .def(init<boost::uint32_t>(arg("value")))
        .def(
            "seed"
          , (void(rng_wrapper<boost::lagged_fibonacci607>::*)(boost::uint32_t))
                &rng_wrapper<boost::lagged_fibonacci607>::seed
          , arg("value")
        );

    buffered_uniform_01_class<boost::lagged_fibonacci1279>("lagged_fibonacci1279")
        .def(init<boost::uint32_t>(arg("value")))
        .def(
            "seed"
          , (void(rng_wrapper<boost::lagged_fibonacci1279>::*)(boost::uint32_t))
              &rng_wrapper<boost::lagged_fibonacci1279>::seed
          , arg("value")
        );

    buffered_uniform_01_class<boost::lagged_fibonacci2281>("lagged_fibonacci2281")
        .def(init<boost::uint32_t>(arg("value")))
        .def(
            "seed"
          , (void(rng_wrapper<boost::lagged_fibonacci2281>::*)(boost::uint32_t))
              &rng_wrapper<boost::lagged_fibonacci2281>::seed
          , arg("value")
        );

    buffered_uniform_01_class<boost::lagged_fibonacci3217>("lagged_fibonacci3217")
        .def(init<boost::uint32_t>(arg("value")))
        .def(
            "seed"
          , (void(rng_wrapper<boost::lagged_fibonacci3217>::*)(boost::uint32_t))
              &rng_wrapper<boost::lagged_fibonacci3217>::seed
          , arg("value")
        );

    buffered_uniform_01_class<boost::lagged_fibonacci4423>("lagged_fibonacci4423")
        .def(init<boost::uint32_t>(arg("value")))
        .def(
            "seed"
          , (void(rng_wrapper<boost::lagged_fibonacci4423>::*)(boost::uint32_t))
              &rng_wrapper<boost::lagged_fibonacci4423>::seed
          , arg("value")
        );

    buffered_uniform_01_class<boost::lagged_fibonacci9689>("lagged_fibonacci9689")
        .def(init<boost::uint32_t>(arg("value")))
        .def(
            "seed"
          , (void(rng_wrapper<boost::lagged_fibonacci9689>::*)(boost::uint32_t))
              &rng_wrapper<boost::lagged_fibonacci9689>::seed
          , arg("value")
        );

    buffered_uniform_01_class<boost::lagged_fibonacci19937>("lagged_fibonacci19937")
        .def(init<boost::uint32_t>(arg("value")))
        .def(
            "seed"
          , (void(rng_wrapper<boost::lagged_fibonacci19937>::*)(boost::uint32_t))
              &rng_wrapper<boost::lagged_fibonacci19937>::seed
          , arg("value")
        );

    buffered_uniform_01_class<boost::lagged_fibonacci23209>("lagged_fibonacci23209")
        .def(init<boost::uint32_t>(arg("value")))
        .def(
            "seed"
          , (void(rng_wrapper<boost::lagged_fibonacci23209>::*)(boost::uint32_t))
              &rng_wrapper<boost::lagged_fibonacci23209>::seed
          , arg("value")
        );

    buffered_uniform_01_class<boost::lagged_fibonacci44497>("lagged_fibonacci44497")
        .def(init<boost::uint32_t>(arg("value")))
        .def(
            "seed"
          , (void(rng_wrapper<boost::lagged_fibonacci44497>::*)(boost::uint32_t))
              &rng_wrapper<boost::lagged_fibonacci44497>::seed
          , arg("value")
        );


    distribution_class<boost::uniform_int<> >(
        "uniform_int"
      , init<int, int>( (arg("min")=0, arg("max")=9) )
    )
      .def("max", &boost::uniform_int<>::max)
      .def("min", &boost::uniform_int<>::min);

    distribution_class<boost::bernoulli_distribution<> >(
        "bernoulli_distribution"
      , init<double>(arg("p")=0.5)
    )
      .def("p", &boost::bernoulli_distribution<>::p);

    distribution_class<boost::geometric_distribution<> >(
        "geometric_distribution"
      , init<double>(arg("p")=0.5)
    )
      .def("p", &boost::geometric_distribution<>::p);

    distribution_class<boost::triangle_distribution<> >(
        "triangle_distribution"
      , init<double, double, double>( (arg("a"), arg("b"), arg("c")) )
    )
      .def("a", &boost::triangle_distribution<>::a)
      .def("b", &boost::triangle_distribution<>::b)
      .def("c", &boost::triangle_distribution<>::c);

    distribution_class<boost::exponential_distribution<> >(
        "exponential_distribution"
      , init<double>(arg("lambda_"))
    )
      .def("lambda_", &boost::exponential_distribution<>::lambda);

    distribution_class<boost::normal_distribution<> >(
        "normal_distribution"
      , init<double, double>( (arg("mean")=0.0, arg("sigma")=1.0) )
    )
      .def("mean", &boost::normal_distribution<>::mean)
      .def("sigma", &boost::normal_distribution<>::sigma);

    distribution_class<boost::lognormal_distribution<> >(
        "lognormal_distribution"
      , init<double, double>( (arg("mean")=0.0, arg("sigma")=1.0) )
    )
      .def("mean", &boost::lognormal_distribution<>::mean)
      .def("sigma", &boost::lognormal_distribution<>::sigma);

    distribution_class<boost::multivariate_normal_distribution<> >(
        "multivariate_normal_distribution"
      , no_init
    )
      .def("__init__", make_constructor(make_multivariate_normal_distribution));
//      .def("mean", &boost::multivariate_normal_distribution<>::mean);
//      .def("cholesky", &boost::multivariate_normal_distribution<>::cholesky);

    variate_generator_class<boost::uniform_int<> >("uniform_int_variate");
    variate_generator_class<boost::bernoulli_distribution<> >("bernoulli_distribution_variate");
    variate_generator_class<boost::geometric_distribution<> >("geometric_distribution_variate");
    variate_generator_class<boost::triangle_distribution<> >("triangle_distribution_variate");
    variate_generator_class<boost::exponential_distribution<> >("exponential_distribution_variate");
    variate_generator_class<boost::normal_distribution<> >("normal_distribution_variate");
    variate_generator_class<boost::lognormal_distribution<> >("lognormal_distribution_variate");
    variate_generator_class<boost::multivariate_normal_distribution<> >("multivariate_normal_distribution_variate");
/*
#define SPRNG_CLASSES \
    (cmrg)(lcg)(lcg64)(lfg)(mlfg)
    //(pmlcg)

#define MAKE_PYTHON_CLASS(r, _, rng) \
    buffered_uniform_01_class<boost::random::sprng::rng>(BOOST_PP_STRINGIZE(rng) "_01") \
        .def(sprng_visitor());

    BOOST_PP_SEQ_FOR_EACH(MAKE_PYTHON_CLASS, ~, SPRNG_CLASSES)

#undef MAKE_PYTHON_CLASS
#undef SPRNG_CLASSES

    typedef mpl::vector3<
        boost::random::random_tag::stream_number*
      , boost::random::random_tag::total_streams*
      , boost::random::random_tag::global_seed*
    > lcg64_keywords;
        
    buffered_uniform_01_class<boost::lcg64a>("lcg64_01")
            .def(
                boost::parameter::python::init<
                    lcg64_keywords
                  , mpl::vector3<int,int,int> 
                >()
            )
            .def("seed",
                boost::parameter::python::function<
                    seed_fwd
                  , lcg64_keywords
                  , mpl::vector4<void,int,int,int> 
                >()
            );*/
}

