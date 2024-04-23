//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2 container relative_size"

#include <nt2/table.hpp>
#include <nt2/include/functions/extent.hpp>
#include <nt2/include/functions/of_size.hpp>
#include <nt2/include/functions/relative_size.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/basic.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>

template<class A0, class I0>
typename boost::proto::result_of::
make_expr< nt2::tag::function_
         , A0&
         , typename nt2::meta::
           call< nt2::tag::function_index_
                 ( typename nt2::meta::
                   call< nt2::tag::aggregate_
                         ( I0 const&
                         )
                       >::type
                 , typename nt2::make_size<1>::type
                 , boost::dispatch::meta::as_< typename A0::indexes_type >
                 )
              >::type
         >::type
at_expr(A0& a0, I0 const& i0)
{
  return boost::proto::make_expr<nt2::tag::function_>
         ( boost::ref(a0)
         , nt2::function_index( nt2::aggregate(i0)
                              , typename nt2::make_size<1>::type(a0.extent())
                              , boost::dispatch::meta::as_< typename A0::indexes_type >()
                              )
         );
}

template<class A0, class I0, class I1>
typename boost::proto::result_of::
make_expr< nt2::tag::function_
         , A0&
         , typename nt2::meta::
           call< nt2::tag::function_index_
                 ( typename nt2::meta::
                   call< nt2::tag::aggregate_
                         ( I0 const&
                         , I1 const&
                         )
                       >::type
                 , typename nt2::make_size<1>::type
                 , boost::dispatch::meta::as_< typename A0::indexes_type >
                 )
              >::type
        >::type
at_expr(A0& a0, I0 const& i0, I1 const& i1)
{
  return boost::proto::make_expr<nt2::tag::function_>
         ( boost::ref(a0)
         , nt2::function_index( nt2::aggregate(i0, i1)
                              , typename nt2::make_size<1>::type(a0.extent())
                              , boost::dispatch::meta::as_< typename A0::indexes_type >()
                              )
         );
}

template<class A0, class I0, class I1, class I2>
typename boost::proto::result_of::
make_expr< nt2::tag::function_
         , A0&
         , typename nt2::meta::
           call< nt2::tag::function_index_
                 ( typename nt2::meta::
                   call< nt2::tag::aggregate_
                         ( I0 const&
                         , I1 const&
                         , I2 const&
                         )
                       >::type
                 , typename nt2::make_size<1>::type
                 , boost::dispatch::meta::as_< typename A0::indexes_type >
                 )
              >::type
        >::type
at_expr(A0& a0, I0 const& i0, I1 const& i1, I2 const& i2)
{
  return boost::proto::make_expr<nt2::tag::function_>
         ( boost::ref(a0)
         , nt2::function_index( nt2::aggregate(i0, i1, i2)
                              , typename nt2::make_size<1>::type(a0.extent())
                              , boost::dispatch::meta::as_< typename A0::indexes_type >()
                              )
         );
}

template<class A0, class I0, class I1, class I2, class I3>
typename boost::proto::result_of::
make_expr< nt2::tag::function_
         , A0&
         , typename nt2::meta::
           call< nt2::tag::function_index_
                 ( typename nt2::meta::
                   call< nt2::tag::aggregate_
                         ( I0 const&
                         , I1 const&
                         , I2 const&
                         , I3 const&
                         )
                       >::type
                 , typename nt2::make_size<1>::type
                 , boost::dispatch::meta::as_< typename A0::indexes_type >
                 )
              >::type
        >::type
at_expr(A0& a0, I0 const& i0, I1 const& i1, I2 const& i2, I3 const& i3)
{
  return boost::proto::make_expr<nt2::tag::function_>
         ( boost::ref(a0)
         , nt2::function_index( nt2::aggregate(i0, i1, i2, i3)
                              , typename nt2::make_size<1>::type(a0.extent())
                              , boost::dispatch::meta::as_< typename A0::indexes_type >()
                              )
         );
}

NT2_TEST_CASE( integral_subscript )
{
  using nt2::_;
  using nt2::table;
  using nt2::of_size;
  typedef double T;

  table<T> a;

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<0>(boost::proto::child_c<1>(a(4,_,_,_)))
                  , 5u, 1u
                  )
                , 1u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<1>(boost::proto::child_c<1>(a(_,3,_,_)))
                  , 5u, 1
                  )
                , 1u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<2>(boost::proto::child_c<1>(a(_,_,2,_)))
                  , 5u, 1
                  )
                , 1u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<3>(boost::proto::child_c<1>(at_expr(a,_,_,_,1)))
                  , 5u, 1
                  )
                , 1u
                );
}

NT2_TEST_CASE( begin_subscript )
{
  using nt2::_;
  using nt2::begin_;
  using nt2::table;
  using nt2::of_size;
  typedef double T;

  table<T> a;

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<0>(boost::proto::child_c<0>(boost::proto::child_c<1>(a(begin_,_,_,_))))
                  , 5u, 1u
                  )
                , 1u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<1>(boost::proto::child_c<0>(boost::proto::child_c<1>(a(_,begin_,_,_))))
                  , 5u, 1u
                  )
                , 1u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<2>(boost::proto::child_c<0>(boost::proto::child_c<1>(a(_,_,begin_,_))))
                  , 5u, 1u
                  )
                , 1u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<3>(boost::proto::child_c<0>(boost::proto::child_c<1>(at_expr(a,_,_,_,begin_))))
                  , 5u, 1u
                  )
                , 1u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<0>(boost::proto::child_c<0>(boost::proto::child_c<1>(a(1+begin_,_,_,_))))
                  , 5u, 1u
                  )
                , 1u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<1>(boost::proto::child_c<0>(boost::proto::child_c<1>(a(_,1+begin_,_,_))))
                  , 5u, 1u
                  )
                , 1u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<2>(boost::proto::child_c<0>(boost::proto::child_c<1>(a(_,_,1+begin_,_))))
                  , 5u, 1u
                  )
                , 1u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<3>(boost::proto::child_c<0>(boost::proto::child_c<1>(at_expr(a,_,_,_,1+begin_))))
                  , 5u, 1u
                  )
                , 1u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<0>(boost::proto::child_c<0>(boost::proto::child_c<1>(a(begin_+1,_,_,_))))
                  , 5u, 1u
                  )
                , 1u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<1>(boost::proto::child_c<0>(boost::proto::child_c<1>(a(_,begin_+1,_,_))))
                  , 5u, 1u
                  )
                , 1u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<2>(boost::proto::child_c<0>(boost::proto::child_c<1>(a(_,_,begin_+1,_))))
                  , 5u, 1u
                  )
                , 1u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<3>(boost::proto::child_c<0>(boost::proto::child_c<1>(at_expr(a,_,_,_,begin_+1))))
                  , 5u, 1u
                  )
                , 1u
                );
}

NT2_TEST_CASE( end_subscript )
{
  using nt2::_;
  using nt2::end_;
  using nt2::table;
  using nt2::of_size;
  typedef double T;

  table<T> a;

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<0>(boost::proto::child_c<0>(boost::proto::child_c<1>(a(end_,_,_,_))))
                  , 5u, 1u
                  )
                , 1u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<1>(boost::proto::child_c<0>(boost::proto::child_c<1>(a(_,end_,_,_))))
                  , 5u, 1u
                  )
                , 1u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<2>(boost::proto::child_c<0>(boost::proto::child_c<1>(a(_,_,end_,_))))
                  , 5u, 1u
                  )
                , 1u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<3>(boost::proto::child_c<0>(boost::proto::child_c<1>(at_expr(a,_,_,_,end_))))
                  , 5u, 1u
                  )
                , 1u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<0>(boost::proto::child_c<0>(boost::proto::child_c<1>(a(end_-1,_,_,_))))
                  , 5u, 1u
                  )
                , 1u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<1>(boost::proto::child_c<0>(boost::proto::child_c<1>(a(_,end_-1,_,_))))
                  , 5u, 1u
                  )
                , 1u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<2>(boost::proto::child_c<0>(boost::proto::child_c<1>(a(_,_,end_-1,_))))
                  , 5u, 1u
                  )
                , 1u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<3>(boost::proto::child_c<0>(boost::proto::child_c<1>(at_expr(a,_,_,_,end_-1))))
                  , 5u, 1u
                  )
                , 1u
                );
}

NT2_TEST_CASE( colon_subscript )
{
  using nt2::_;
  using nt2::table;
  using nt2::of_size;
  typedef double T;

  table<T> a;

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<0>(boost::proto::child_c<0>(boost::proto::child_c<1>(at_expr(a,_,1,1,1))))
                  , 5u, 1u
                  )
                , 5u
                );


  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<1>(boost::proto::child_c<0>(boost::proto::child_c<1>(a(1,_,1,1))))
                  , 5u, 1u
                  )
                , 5u
                );


  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<2>(boost::proto::child_c<0>(boost::proto::child_c<1>(a(1,1,_,1))))
                  , 5u, 1u
                  )
                , 5u
                );


  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<3>(boost::proto::child_c<0>(boost::proto::child_c<1>(a(1,1,1,_))))
                  , 5u, 1u
                  )
                , 5u
                );
}

NT2_TEST_CASE( colon_k_end_subscript )
{
  using nt2::_;
  using nt2::end_;
  using nt2::table;
  using nt2::of_size;
  typedef double T;

  table<T> a;

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<0>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>( at_expr(a, _(1,end_) ,1,1,1)) ))
                  , 5u, 1u
                  )
                , 5u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<1>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>(a(1,_(1,end_),1,1))))
                  , 5u, 1u
                  )
                , 5u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<2>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>(a(1,1,_(1,end_),1))))
                  , 5u, 1u
                  )
                , 5u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<3>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>(a(1,1,1,_(1,end_)))))
                  , 5u, 1u
                  )
                , 5u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<0>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>( at_expr(a, _(1,end_-2) ,1,1,1)) ))
                  , 5u, 1u
                  )
                , 3u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<1>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>(a(1,_(1,end_-2),1,1))))
                  , 5u, 1u
                  )
                , 3u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<2>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>(a(1,1,_(1,end_-2),1))))
                  , 5u, 1u
                  )
                , 3u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<3>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>(a(1,1,1,_(1,end_-2)))))
                  , 5u, 1u
                  )
                , 3u
                );
}

NT2_TEST_CASE( colon_begin_k_subscript )
{
  using nt2::_;
  using nt2::begin_;
  using nt2::table;
  using nt2::of_size;
  typedef double T;

  table<T> a;

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<0>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>( at_expr(a, _(begin_,4) ,1,1,1)) ))
                  , 5u, 1u
                  )
                , 4u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<1>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>(a(1,_(begin_,4),1,1))))
                  , 5u, 1u
                  )
                , 4u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<2>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>(a(1,1,_(begin_,4),1))))
                  , 5u, 1u
                  )
                , 4u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<3>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>(a(1,1,1,_(begin_,4)))))
                  , 5u, 1u
                  )
                , 4u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<0>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>( at_expr(a, _(begin_+1,5) ,1,1,1)) ))
                  , 5u, 1u
                  )
                , 4u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<1>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>(a(1,_(begin_+1,5),1,1))))
                  , 5u, 1u
                  )
                , 4u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<2>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>(a(1,1,_(begin_+1,5),1))))
                  , 5u, 1u
                  )
                , 4u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<3>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>(a(1,1,1,_(begin_+1,5)))))
                  , 5u, 1u
                  )
                , 4u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<0>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>( at_expr(a, _(1+begin_,5) ,1,1,1)) ))
                  , 5u, 1u
                  )
                , 4u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<1>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>(a(1,_(1+begin_,5),1,1))))
                  , 5u, 1u
                  )
                , 4u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<2>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>(a(1,1,_(1+begin_,5),1))))
                  , 5u, 1u
                  )
                , 4u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<3>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>(a(1,1,1,_(1+begin_,5)))))
                  , 5u, 1u
                  )
                , 4u
                );
}

NT2_TEST_CASE( colon_begin_end_subscript )
{
  using nt2::_;
  using nt2::end_;
  using nt2::begin_;
  using nt2::table;
  using nt2::of_size;
  typedef double T;

  table<T> a;

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<0>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>( at_expr(a, _(begin_+1,end_-1) ,1,1,1)) ))
                  , 5u, 1u
                  )
                , 3u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<1>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>(a(1,_(begin_+1,end_-1),1,1))))
                  , 5u, 1u
                  )
                , 3u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<2>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>(a(1,1,_(begin_+1,end_-1),1))))
                  , 5u, 1u
                  )
                , 3u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<3>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>(a(1,1,1,_(begin_+1,end_-1)))))
                  , 5u, 1u
                  )
                , 3u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<3>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>(a(1,1,1,_(end_,begin_)))))
                  , 5u, 1u
                  )
                , 0u
                );
}

NT2_TEST_CASE( colon_begin_stride_end_subscript )
{
  using nt2::_;
  using nt2::end_;
  using nt2::begin_;
  using nt2::table;
  using nt2::of_size;
  typedef double T;

  table<T> a;

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<0>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>( a( _(1,2,end_) ,1,1,1)) ))
                  , 5u, 1u
                  )
                , 3u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<0>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>( a( _(begin_,2,5) ,1,1,1)) ))
                  , 5u, 1u
                  )
                , 3u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<0>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>( a( _(begin_,2,end_) ,1,1,1)) ))
                  , 5u, 1u
                  )
                , 3u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<0>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>( a( _(end_,-1,begin_) ,1,1,1)) ))
                  , 5u, 1u
                  )
                , 5u
                );

  NT2_TEST_EQUAL( nt2::relative_size
                  ( boost::proto::child_c<0>(boost::proto::child_c<0>(boost::proto
                    ::child_c<1>( a( _(end_,-2,begin_) ,1,1,1)) ))
                  , 5u, 1u
                  )
                , 3u
                );
}
