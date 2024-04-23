// switch.hpp
//
// Copyright (c) 2006-2007
// Steven Watanabe
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost/org/LICENSE_1_0.txt)
//
//
////////////////////////////////////////////////////////////////////////////////
//
// Implementation note:
//   This is a modification of the original boost switch library proposal. It
// has been slightly simplified by removing the switch_ variant that does not
// have the default case processing (and throws an exception in case the default
// case does actually get 'invoked'). This removed a lot of include
// declarations. It was also changed to take the functor parameter by reference
// (instead of by value).
//   The original proposal, discussion, documentation and source can be found
// at:
// http://archives.free.net.ph/message/20080105.174405.447700bc.en.html
// http://dancinghacker.com/switch
//                                            (08.05.2009.) (Domagoj)
//
////////////////////////////////////////////////////////////////////////////////
/// \todo
///   Replace this unofficial custom modified implementation with the official
/// library once it gets accepted and released.
///                                           (08.05.2009.) (Domagoj)
////////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_SWITCH_HPP_INCLUDED
#define BOOST_SWITCH_HPP_INCLUDED

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/preprocessor/config/limits.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/iteration/local.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/at.hpp>
#include <boost/type_traits/remove_reference.hpp>

#ifndef BOOST_SWITCH_LIMIT
    #define BOOST_SWITCH_LIMIT 50
#endif

#if BOOST_SWITCH_LIMIT > BOOST_PP_LIMIT_REPEAT
    #error BOOST_SWITCH_LIMIT exceeds Boost.Preprocessor limit
#endif
#if BOOST_SWITCH_LIMIT > BOOST_PP_LIMIT_ITERATION
    #error BOOST_SWITCH_LIMIT exceeds Boost.Preprocessor limit
#endif

namespace boost
{

namespace switch_detail
{

// N is the number of cases not including the default
template<int N>
struct switch_impl;

// specialize for 0 separately to avoid warnings
template<>
struct switch_impl<0> {
    template<class V, class Int, class F, class Default>
    static typename F::result_type
    apply(Int i, F &&, Default d BOOST_APPEND_EXPLICIT_TEMPLATE_TYPE(V)) {
        return(d(i));
    }
};

#define BOOST_SWITCH_CASE(z, n, data)                   \
    case boost::mpl::at_c<data, n>::type::value: {      \
        typename boost::mpl::at_c<data, n>::type arg;   \
        return(f(arg));                                 \
    }

#define BOOST_SWITCH_IMPL(z, n, data)                                   \
    template<>                                                          \
    struct switch_impl<n> {                                             \
        template<class V, class I, class F, class D>                    \
        static typename F::result_type                                  \
        apply(I i, F && f, D d BOOST_APPEND_EXPLICIT_TEMPLATE_TYPE(V)) { \
            switch(i) {                                                 \
                BOOST_PP_REPEAT_##z(n, BOOST_SWITCH_CASE, V)            \
                default: return(d(i));                                  \
            }                                                           \
        }                                                               \
    };

#define BOOST_PP_LOCAL_LIMITS (1, BOOST_SWITCH_LIMIT)
#define BOOST_PP_LOCAL_MACRO(n) BOOST_SWITCH_IMPL(1, n, ~)
#include BOOST_PP_LOCAL_ITERATE()

#undef BOOST_SWITCH_IMPL
#undef BOOST_SWITCH_CASE

}  // namespace switch_detail

////////////////////////////////////////////////////////////////////////////
///
/// \struct assert_no_default_case
///
/// \brief Helper functor for use with boost::switch_() which asserts that
/// the default case never gets 'traversed'.
///
////////////////////////////////////////////////////////////////////////////

template <typename result_type>
struct assert_no_default_case
{
    #ifdef _MSC_VER
        __declspec( noreturn )
    #endif
    result_type operator()( int ) const
    {
        BOOST_ASSERT( !"Default switch_ case executed!." );
        #if defined( _MSC_VER )
            __assume( false );
        #elif ( __clang_major__ >= 2 ) || ( ( ( __GNUC__ * 10 ) + __GNUC_MINOR__ ) >= 45 )
            __builtin_unreachable();
        #else // _MSC_VER
            return result_type();
        #endif
    }
};

template<class V, class N, class F, class D>
inline typename F::result_type
switch_(N n, F && f, D d BOOST_APPEND_EXPLICIT_TEMPLATE_TYPE(V))
{
    typedef switch_detail::switch_impl<boost::mpl::size<V>::value> impl;
    return(impl::template apply<V>(n, std::forward<F>(f), d));
}

}  // namespace boost
#endif
