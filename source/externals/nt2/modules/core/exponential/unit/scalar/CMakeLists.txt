##############################################################################
###   Copyright 2003 and onward LASMEA UMR 6602 CNRS/U.B.P Clermont-Ferrand
###   Copyright 2009 and onward LRI    UMR 8623 CNRS/Univ Paris Sud XI
###
###          Distributed under the Boost Software License, Version 1.0
###                 See accompanying file LICENSE.txt or copy at
###                     http://www.boost.org/LICENSE_1_0.txt
##############################################################################

SET( SOURCES
  cbrt.cpp
  exp.cpp
  expm1.cpp
  expx2.cpp
  exp10.cpp
  exp2.cpp
  log.cpp
  log1p.cpp
  log10.cpp
  log2.cpp
  logspace_sub.cpp
  logspace_add.cpp
  nthroot.cpp
  pow.cpp
  pow_abs.cpp
  pow2.cpp
  realpow.cpp
  reallog.cpp
  realsqrt.cpp
  sqrt1pm1.cpp
# List of scalar test files for toolbox exponential
   )

nt2_module_add_tests(core.exponential.scalar.unit ${SOURCES})
