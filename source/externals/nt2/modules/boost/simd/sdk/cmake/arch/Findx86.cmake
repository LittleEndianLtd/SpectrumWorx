################################################################################
#         Copyright 2003 & onward LASMEA UMR 6602 CNRS/Univ. Clermont II
#         Copyright 2009 & onward LRI    UMR 8623 CNRS/Univ Paris Sud XI
#
#          Distributed under the Boost Software License, Version 1.0.
#                 See accompanying file LICENSE.txt or copy at
#                     http://www.boost.org/LICENSE_1_0.txt
################################################################################

include(CheckCXXCompilerFlag)

################################################################################
# Force scalar computation to use SIMD unit
################################################################################
macro(nt2_simd_set_fpmath ext)
  string(TOLOWER ${ext} ext_l)
  set(ext_u ${ext})

  if(NT2_COMPILER_GCC)

      if(ext_u MATCHES "AVX|FMA4|XOP|FMA3|AVX2")
        set(ext_u AVX)
      else()
        set(ext_u SSE)
      endif()
      string(TOLOWER ${ext_u} ext_l)

      check_cxx_compiler_flag("-mfpmath=${ext_l}" HAS_GCC_MFPMATH_${ext_u})
      if(HAS_GCC_MFPMATH_${ext_u})
        set(NT2_SIMD_FLAGS "${NT2_SIMD_FLAGS} -mfpmath=${ext_l}")
      endif()

      if(ext_l STREQUAL avx)
        check_cxx_compiler_flag("-fabi-version=4" HAS_GCC_FABI_VERSION_4)
        if(HAS_GCC_FABI_VERSION_4)
          set(NT2_SIMD_FLAGS "${NT2_SIMD_FLAGS} -fabi-version=4")
        endif()
      endif()

  elseif(NT2_COMPILER_MSVC)

      if(ext_u MATCHES "AVX|FMA4|XOP|FMA3|AVX2")
        set(ext_u AVX)
      elseif(NOT ext_u STREQUAL SSE)
        set(ext_u SSE2)
      else()
        set(ext_u SSE)
      endif()

    check_cxx_compiler_flag("/arch:${ext_u}" HAS_MSVC_ARCH_${ext_u})
    if(HAS_MSVC_ARCH_${ext_u})
      set(NT2_SIMD_FLAGS "${NT2_SIMD_FLAGS} /arch:${ext_u}")
    endif()
  endif()

endmacro()

################################################################################
# Detect best SIMD capability on host machine
################################################################################
macro(nt2_simd_cpuid_check ext)

  # Check for ext availability
  string(TOLOWER ${ext} ext_l)
  string(REPLACE "_" "." ext_l ${ext_l})

  nt2_module_tool(is_supported ${ext_l} RESULT_VARIABLE RESULT_VAR OUTPUT_QUIET)
  if(RESULT_VAR EQUAL 0)
    set(NT2_HAS_${ext}_SUPPORT 1)
  else()
    set(NT2_HAS_${ext}_SUPPORT 0)
  endif()

  if(${ext} STREQUAL FMA3)
    set(ext_l fma)
  endif()

  if(NT2_HAS_${ext}_SUPPORT)
    check_cxx_compiler_flag("-m${ext_l}" HAS_GCC_${ext})
    if(HAS_GCC_${ext})
      set(NT2_SIMD_FLAGS "-m${ext_l}")
    else()
      # find a way to test if compiler really supports it?
      set(NT2_SIMD_FLAGS "-DBOOST_SIMD_HAS_${ext}_SUPPORT")
    endif()

    if(${ext} STREQUAL avx2)
      check_cxx_compiler_flag("-mfma" HAS_GCC_FMA)
      if(HAS_GCC_FMA)
        set(NT2_SIMD_FLAGS "${NT2_SIMD_FLAGS} -mfma")
      else()
        # find a way to test if compiler really supports it?
        set(NT2_SIMD_FLAGS "${NT2_SIMD_FLAGS} -DBOOST_SIMD_HAS_FMA3_SUPPORT")
      endif()
    endif()
  endif()

  if(NT2_HAS_${ext}_SUPPORT)
    message(STATUS "[boost.simd.sdk] ${ext} available")
    set(NT2_SIMD_EXT ${ext_l} PARENT_SCOPE)
    nt2_simd_set_fpmath(${ext})
    set(NT2_SIMD_FLAGS ${NT2_SIMD_FLAGS} PARENT_SCOPE)
    return()
  else()
    message(STATUS "[boost.simd.sdk] ${ext} not available")
  endif()

endmacro()

function(nt2_simd_cpuid_find)
  nt2_simd_cpuid_check(AVX2)
  nt2_simd_cpuid_check(FMA3)
  nt2_simd_cpuid_check(XOP)
  nt2_simd_cpuid_check(FMA4)
  nt2_simd_cpuid_check(AVX)
  nt2_simd_cpuid_check(SSE4_2)
  nt2_simd_cpuid_check(SSE4_1)
  nt2_simd_cpuid_check(SSSE3)
  nt2_simd_cpuid_check(SSE4A)
  nt2_simd_cpuid_check(SSE3)
  nt2_simd_cpuid_check(SSE2)
  nt2_simd_cpuid_check(SSE)

  set(NT2_SIMD_FLAGS " " PARENT_SCOPE)

endfunction()

nt2_simd_cpuid_find()
