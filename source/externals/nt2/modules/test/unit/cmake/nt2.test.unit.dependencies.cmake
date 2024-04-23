################################################################################
#         Copyright 2003 & onward LASMEA UMR 6602 CNRS/Univ. Clermont II
#         Copyright 2009 & onward LRI    UMR 8623 CNRS/Univ Paris Sud XI
#
#          Distributed under the Boost Software License, Version 1.0.
#                 See accompanying file LICENSE.txt or copy at
#                     http://www.boost.org/LICENSE_1_0.txt
################################################################################
SET ( NT2_TEST.UNIT_DEPENDENCIES_EXTRA
      boost.dispatch
      boost.simd.base
      boost.simd.sdk
      core.base
      core.sdk
      sdk.config
      sdk.error
      sdk.functor
      sdk.meta
    )

SET ( NT2_TEST.UNIT_LIBRARIES optimized nt2_unit debug nt2_unit_d)
