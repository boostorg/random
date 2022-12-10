/* 
 * Copyright Matt Borland 2022.
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org for most recent version including documentation.
 *
 * $Id$
 */

#ifndef BOOST_RANDOM_DETAIL_XOSHIRO_BASE
#define BOOST_RANDOM_DETAIL_XOSHIRO_BASE

#include <cstdint>
#include <array>
#include <type_traits>
#include <utility>
#include <stdexcept>
#include <string>
#include <limits>
#include <initializer_list>
#include <boost/random/splitmix64.hpp>

namespace boost { namespace random { namespace detail {

// N is the number of words (e.g. for xoshiro 256 N=4)
template <std::size_t N>
class xoshiro
{
protected:
    std::array<std::uint64_t, N> state_;

    inline std::uint64_t concatenate(std::uint32_t word1, std::uint32_t word2)
    {
        return static_cast<std::uint64_t>(word1) << 32 | word2;
    }

    template <typename T>
    inline T rotl(T x, int s) noexcept
    {
        constexpr auto n = std::numeric_limits<T>::digits;
        auto r = s % n;

        if (r == 0)
        {
            return 0;
        }
        else if (r > 0)
        {
            return (x << r) | (x >> (n - r));
        }

        return rotr(x, -r);
    }

    template <typename T>
    inline T rotr(T x, int s) noexcept
    {
        constexpr auto n = std::numeric_limits<T>::digits;
        auto r = s % n;

        if (r == 0)
        {
            return 0;
        }
        else if (r > 0)
        {
            return (x >> r) | (x << (n - r));
        }

        return rotl(x, -r);
    }

public:
    using result_type = std::uint64_t;
    using seed_type = std::uint64_t;

    static constexpr bool has_fixed_range {false};

    void seed(const std::uint64_t state = 0)
    {
        splitmix64 gen(state);
        for (auto& i : state_)
        {
            i = gen();
        }
    }

    template <typename T, typename std::enable_if<std::is_convertible<T, std::uint64_t>::value, bool>::type = true>
    void seed(T value = 0)
    {
        seed(static_cast<std::uint64_t>(value));
    }

    template <typename Sseq, 
              typename std::enable_if<!std::is_convertible<Sseq, std::uint64_t>::value, bool>::type = true>
    void seed(Sseq& seq)
    {
        for (auto& i : state_)
        {
            std::array<std::uint32_t, 2> seeds;
            seq.generate(seeds.begin(), seeds.end());
            
            i = concatenate(seeds[0], seeds[1]);
        }
    }

    template <typename FIter>
    void seed(FIter first, FIter last)
    {
        if (std::distance(first, last) < N)
        {
            throw std::logic_error("Not enough values were provided to seed the engine");
        }

        for (auto& i : state_)
        {
            i = *first++;
        }
    }

    explicit xoshiro(std::uint64_t state = 0)
    {
        seed(state);
    }

    template <typename Sseq, typename std::enable_if<!std::is_convertible<Sseq, xoshiro>::value, bool>::type = true>
    explicit xoshiro(Sseq& seq)
    {
        seed(seq);
    }

    template <typename FIter>
    xoshiro(FIter first, FIter last)
    {
        seed(first, last);
    }

    template <typename... Args, typename Integer = typename std::common_type<Args...>::type,
              typename std::enable_if<std::is_convertible<Integer, std::uint64_t>::value, bool>::type = true>
    xoshiro(Args&& ...args)
    {
        std::initializer_list<Integer> list {std::forward<Args>(args)...};
        seed(list.begin(), list.end());
    }

    xoshiro(const xoshiro& other) = default;
    xoshiro& operator=(const xoshiro& other) = default;

    // The following functions are pure virtual as each generator has it's own method
    virtual inline result_type next() noexcept = 0;
    virtual inline void jump() noexcept = 0;
    virtual inline void long_jump() noexcept = 0;

    inline result_type operator()() noexcept
    {
        return next();
    }

    inline void discard(unsigned long long val)
    {
        for (unsigned long long i {}; i < val; ++i)
        {
            next();
        }
    }

    inline friend bool operator==(const xoshiro& lhs, const xoshiro& rhs) noexcept
    {
        if (lhs.state_ == rhs.state_)
        {
            return true;
        }

        return false;
    }

    inline friend bool operator!=(const xoshiro& lhs, const xoshiro& rhs) noexcept
    {
        return !(lhs == rhs);
    }

    template <typename CharT, typename Traits>
    inline friend std::basic_ostream<CharT,Traits>& operator<<(std::basic_ostream<CharT,Traits>& ost, 
                                                               const xoshiro& e)
    {
        for (std::size_t i {}; i < N; ++i)
        {
            std::string sstate = std::to_string(e.state_[i]);
            for (const auto i : sstate)
            {
                ost << i;
            }

            ost << ',';
        }

        return ost;
    }

    template <typename CharT, typename Traits>
    friend std::basic_istream<CharT,Traits>& operator>>(std::basic_istream<CharT,Traits>& ist, 
                                                        xoshiro& e)
    {
        for (std::size_t i {}; i < N; ++i)
        {
            std::string sstate;
            CharT val;
            ist >> val >> std::ws;
            while (val != ',')
            {
                sstate.push_back(val);
                ist >> val >> std::ws;
            }

            e.state_[i] = std::strtoull(sstate.c_str(), nullptr, 10);
        }

        return ist;
    }

    template <typename FIter>
    inline void generate(FIter first, FIter last)
    {
        while (first != last)
        {
            *first++ = next();
        }
    }

    static constexpr result_type (max)() noexcept
    {
        return (std::numeric_limits<std::uint64_t>::max)();
    }

    static constexpr result_type (min)() noexcept
    {
        return (std::numeric_limits<std::uint64_t>::min)();
    }

    inline std::array<std::uint64_t, N> state()
    {
        return state_;
    }
};

}}} // Namespaces

#endif
