//==============================================================================
//         Copyright 2003 - 2011   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2011   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/core/settings/option.hpp>
#include <nt2/core/settings/settings.hpp>
#include <nt2/core/settings/shape.hpp>
#include "local_semantic.hpp"

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/basic.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/tests/type_expr.hpp>

NT2_TEST_CASE( shape_concept )
{
  using nt2::meta::option;
  using nt2::meta::match_option;

  NT2_TEST( (match_option< nt2::rectangular_, nt2::tag::shape_ >::value) );
}

NT2_TEST_CASE( single_shape_ )
{
  using nt2::rectangular_;
  using nt2::diagonal_;
  using nt2::meta::option;
  using boost::mpl::_;

  NT2_TEST_EXPR_TYPE( (rectangular_())
                      ,(option< _, nt2::tag::shape_, some_kind_>)
                      ,(rectangular_)
                      );
}

NT2_TEST_CASE( shape_default )
{
  using nt2::rectangular_;
  using nt2::diagonal_;
  using nt2::settings;
  using nt2::meta::option;

  NT2_TEST_TYPE_IS( (option < settings()
                            , nt2::tag::shape_
                            , some_kind_
                            >::type
                    )
                  , rectangular_
                  );

  NT2_TEST_TYPE_IS( (option < settings(int,void*)
                            , nt2::tag::shape_
                            , some_kind_
                            >::type
                    )
                  , rectangular_
                  );
}

NT2_TEST_CASE( single_settings_shape_ )
{
  using nt2::rectangular_;
  using nt2::diagonal_;
  using nt2::settings;
  using nt2::meta::option;
  using boost::mpl::_;

  NT2_TEST_TYPE_IS( (option < settings(rectangular_)
                            , nt2::tag::shape_
                            , some_kind_
                            >::type
                    )
                  , (rectangular_)
                  );
}

NT2_TEST_CASE( multi_settings_shape_ )
{
  using nt2::rectangular_;
  using nt2::diagonal_;
  using nt2::settings;
  using nt2::meta::option;
  using boost::mpl::_;

  NT2_TEST_TYPE_IS( (option < settings(rectangular_,diagonal_)
                            , nt2::tag::shape_
                            , some_kind_
                            >::type
                    )
                  , rectangular_
                  );

  NT2_TEST_TYPE_IS( (option < settings(diagonal_,rectangular_)
                            , nt2::tag::shape_
                            , some_kind_
                            >::type
                    )
                  , diagonal_
                  );
}

NT2_TEST_CASE( nested_settings_shape_ )
{
  using nt2::rectangular_;
  using nt2::diagonal_;
  using nt2::settings;
  using nt2::meta::option;
  using boost::mpl::_;

  typedef settings shadow(double,int);
  typedef settings option1(rectangular_,diagonal_);
  typedef settings option2(diagonal_,rectangular_);

  NT2_TEST_TYPE_IS( (option < settings(shadow,option1)
                            , nt2::tag::shape_
                            , some_kind_
                            >::type
                    )
                  , rectangular_
                  );

  NT2_TEST_TYPE_IS( (option < settings(shadow,option2)
                            , nt2::tag::shape_
                            , some_kind_
                            >::type
                    )
                  , (diagonal_)
                  );
}
