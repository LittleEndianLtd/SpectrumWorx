################################################################################
#         Copyright 2003 & onward LASMEA UMR 6602 CNRS/Univ. Clermont II
#         Copyright 2009 & onward LRI    UMR 8623 CNRS/Univ Paris Sud XI
#
#          Distributed under the Boost Software License, Version 1.0.
#                 See accompanying file LICENSE.txt or copy at
#                     http://www.boost.org/LICENSE_1_0.txt
################################################################################

################################################################################
# Check for ARM NEON availability
################################################################################

nt2_module_tool(is_supported neon RESULT_VARIABLE RESULT_VAR OUTPUT_QUIET)
if(RUN_RESULT_VAR EQUAL 0)
  set(NT2_HAS_NEON_SUPPORT 1)
else()
  set(NT2_HAS_NEON_SUPPORT 0)
endif()

if(NT2_HAS_NEON_SUPPORT)
  message(STATUS "[boost.simd.sdk] ARM NEON available")
  set(NT2_SIMD_EXT neon)

  # Find the proper options to compile
  check_cxx_compiler_flag("-mfpu=neon" HAS_GCC_NEON)

  if(HAS_GCC_NEON)
    set(NT2_SIMD_FLAGS "-mfpu=neon")
  else()
    set(NT2_SIMD_FLAGS "-DBOOST_SIMD_HAS_NEON_SUPPORT")
  endif()

else()
  message(STATUS "[boost.simd.sdk] ARM NEON not available")
  set(NT2_SIMD_FLAGS " ")
endif()
