################################################################################
#         Copyright 2003 & onward LASMEA UMR 6602 CNRS/Univ. Clermont II
#         Copyright 2009 & onward LRI    UMR 8623 CNRS/Univ Paris Sud XI
#
#          Distributed under the Boost Software License, Version 1.0.
#                 See accompanying file LICENSE.txt or copy at
#                     http://www.boost.org/LICENSE_1_0.txt
################################################################################

if(NOT NT2_COMPILER_OPTIONS_INCLUDED)
set(NT2_COMPILER_OPTIONS_INCLUDED 1)

include(nt2.info)

# Remove /EHsc from CMAKE_CXX_FLAGS and re-add per configuration; useful to avoid 'overriding' warnings
# (/EHx options are used by MSVC and ICC)
if(CMAKE_CXX_FLAGS MATCHES "/EHsc")
  string(REPLACE " /EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  foreach(config Debug Release)
    string(TOUPPER ${config} config_U)
    set(CMAKE_CXX_FLAGS_${config_U} "/EHsc ${CMAKE_CXX_FLAGS_${config_U}}")
  endforeach()
endif()

# MSVC12 needs /FS if building in debug in parallel
if(MSVC AND (MSVC_VERSION EQUAL 1800 OR MSVC_VERSION GREATER 1800))
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /FS")
endif()

# Enable sanitizers for tests
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # Apple Clang seems to only support the undefined-trap/trap-on-error combo
  if(APPLE)
    set(NT2_FLAGS_TEST "${NT2_FLAGS_TEST} -DUSE_UBSAN -fsanitize=undefined-trap -fno-sanitize=float-cast-overflow,float-divide-by-zero,shift -fsanitize-undefined-trap-on-error")
  else()
    set(SANITIZE_FLAGS "-fsanitize=address,undefined-trap -fno-sanitize=float-cast-overflow,float-divide-by-zero,shift -fno-sanitize-recover")
    set(NT2_FLAGS_TEST "${NT2_FLAGS_TEST} -DUSE_UBSAN ${SANITIZE_FLAGS}")
    set(NT2_FLAGS_TEST_LINK "${NT2_FLAGS_TEST_LINK} ${SANITIZE_FLAGS}")
  endif()
elseif(CMAKE_COMPILER_IS_GNUXX)
  # Causes weird dependency on softfloat runtime on PowerPC
  if(NOT CMAKE_SYSTEM_PROCESSOR MATCHES "powerpc|ppc")
    set(NT2_FLAGS_TEST "${NT2_FLAGS_TEST} -DUSE_UBSAN -ftrapv")
  endif()
endif()

set(NT2_FLAGS_TEST "${NT2_FLAGS_TEST} -DBOOST_ENABLE_ASSERT_HANDLER -DNT2_ENABLE_WARNING_HANDLER")
set(NT2_FLAGS_BENCH "${CMAKE_CXX_FLAGS_RELEASE}")

# No debug symbols in tests because of excessive time and memory costs at compile time;
# use special debug targets instead
set(NT2_FLAGS_TESTDEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${NT2_FLAGS_TEST} -DBOOST_FORCEINLINE=inline")

# Implementation note:
# There is a bug in CMake 2.8.6+ (supposedly fixed but obviously not properly)
# that causes CMake to enter into an infinite loop at the end "Generate" if
# there is a compiler option other than "-g" that contains the "-g" substring so
# avoid this.
#                                             (21.11.2012.) (Domagoj Saric)

if(MSVC)
  # Reset base/default options as the CMake defaults are dubious
  set( CMAKE_CXX_FLAGS           "" CACHE STRING "" FORCE )
  set( CMAKE_EXE_LINKER_FLAGS    "" CACHE STRING "" FORCE )
  set( CMAKE_MODULE_LINKER_FLAGS "" CACHE STRING "" FORCE )
  set( CMAKE_SHARED_LINKER_FLAGS "" CACHE STRING "" FORCE )

  # Global MSVC settings: multithreaded + IEEE754 floating-point
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:precise /fp:except- /MP /W4")

  # MSVC12 needs /FS if building in debug in parallel
  if(MSVC AND (MSVC_VERSION EQUAL 1800 OR MSVC_VERSION GREATER 1800))
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /FS")
  endif()

  # /Ox is used instead of /O2 to enable easier switching between for-speed and
  # "intelligent for-size" builds and to get nicer looking/less confusing option
  # listings in Visual Studio project properties pages. For the same reason /O
  # options aren't grouped. Some options need to be explicitly specified even if
  # they are implied by /Ox because default .vcxproj settings override them.
  set(NT2_FLAGS_TEST "${NT2_FLAGS_TEST} /MDd /Ox /Ot /EHa /GF /Gm-")
  string(REPLACE "/EHsc" "/EHa" NT2_FLAGS_TESTDEBUG "${NT2_FLAGS_TESTDEBUG}")
  set(NT2_FLAGS_BENCH "/DNDEBUG /MD /D_SECURE_SCL=0 /GL /Ox /Ot /Oi /Oy /GF /Gm- /GS- /GR- /wd4530")

  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "/OPT:REF /OPT:ICF /LTCG /INCREMENTAL:NO" CACHE STRING "MSVC++ linker release options" FORCE)
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE    ${CMAKE_SHARED_LINKER_FLAGS_RELEASE}      CACHE STRING "MSVC++ linker release options" FORCE)
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # Use the latest language revision
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
  # Verify that NT2 does not rely on threadsafe statics
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-threadsafe-statics")
  # Strict aliasing disabled due to GCC bug #50800
  # -D_GLIBCXX_DEBUG=1 not used because of incompatibilities with libraries
  if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing -DBOOST_SIMD_NO_STRICT_ALIASING")
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcatch-undefined-behavior")
  endif()
  set(NT2_FLAGS_TEST "${NT2_FLAGS_TEST} -O3")
  set(NT2_FLAGS_TESTDEBUG "${NT2_FLAGS_TESTDEBUG} -O0 -g")
  if(NT2_ARCH_X86) # valgrind doesn't support SSE4 or AVX
     set(NT2_FLAGS_TESTDEBUG "${NT2_FLAGS_TESTDEBUG} -mno-sse4.1 -mno-sse4.2 -mno-avx")
  endif()
  set(NT2_FLAGS_BENCH "-DNDEBUG -O3 -fomit-frame-pointer -fno-exceptions -fno-rtti -flto -ffunction-sections -fdata-sections -Wl,--gc-sections,-flto")

  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "-flto" CACHE STRING "GCC/Clang linker release options" FORCE)
  if (APPLE)
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -dead_strip" CACHE STRING "GCC/Clang linker release options" FORCE)
  else()
    # OS X (10.8) ld does not seem to support --gc-sections
    # OS X (10.8) ld warns that -s is obsolete and ignored even though it is not
    # and makes a difference (not using it to avoid the linker warning)
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -s --gc-sections" CACHE STRING "GCC/Clang linker release options" FORCE)
  endif()
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE ${CMAKE_SHARED_LINKER_FLAGS_RELEASE} CACHE STRING "GCC/Clang linker release options" FORCE)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
  if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fp-model precise")
    set(NT2_FLAGS_TEST "${NT2_FLAGS_TEST} -O2")
    set(NT2_FLAGS_BENCH "${NT2_FLAGS_BENCH} -fno-exceptions")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:precise")
    set(NT2_FLAGS_TEST "${NT2_FLAGS_TEST} /O2 /EHa")
    string(REPLACE "/EHsc" "" NT2_FLAGS_BENCH "${NT2_FLAGS_BENCH}")
    string(REPLACE "/EHsc" "/EHa" NT2_FLAGS_TESTDEBUG "${NT2_FLAGS_TESTDEBUG}")
  endif()
else()
  set(NT2_FLAGS_TEST "${NT2_FLAGS_TEST}")
  set(NT2_FLAGS_TESTDEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${NT2_FLAGS_TESTDEBUG}")
  set(NT2_FLAGS_BENCH "${NT2_FLAGS_BENCH} ${CMAKE_CXX_FLAGS_RELEASE}")
endif()

set(CMAKE_C_FLAGS_NT2TEST ${NT2_FLAGS_TEST})
set(CMAKE_C_FLAGS_NT2TESTDEBUG ${NT2_FLAGS_TESTDEBUG})
set(CMAKE_C_FLAGS_NT2BENCH ${NT2_FLAGS_BENCH})
set(CMAKE_C_FLAGS_NT2DEBUGEMPTY)
set(CMAKE_CXX_FLAGS_NT2TEST ${NT2_FLAGS_TEST})
set(CMAKE_CXX_FLAGS_NT2TESTDEBUG ${NT2_FLAGS_TESTDEBUG})
set(CMAKE_CXX_FLAGS_NT2BENCH ${NT2_FLAGS_BENCH})
set(CMAKE_CXX_FLAGS_NT2DEBUGEMPTY)
set(CMAKE_EXE_LINKER_FLAGS_NT2TEST "${NT2_FLAGS_TEST_LINK} ${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
set(CMAKE_EXE_LINKER_FLAGS_NT2TESTDEBUG "${NT2_FLAGS_TEST_LINK} ${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
set(CMAKE_EXE_LINKER_FLAGS_NT2BENCH ${CMAKE_EXE_LINKER_FLAGS_RELEASE})
set(CMAKE_EXE_LINKER_FLAGS_NT2DEBUGEMPTY)
set(CMAKE_SHARED_LINKER_FLAGS_NT2TEST "${NT2_FLAGS_TEST_LINK} ${CMAKE_SHARED_LINKER_FLAGS_RELEASE}")
set(CMAKE_SHARED_LINKER_FLAGS_NT2TESTDEBUG "${NT2_FLAGS_TEST_LINK} ${CMAKE_SHARED_LINKER_FLAGS_DEBUG}")
set(CMAKE_SHARED_LINKER_FLAGS_NT2BENCH ${CMAKE_SHARED_LINKER_FLAGS_RELEASE})
set(CMAKE_SHARED_LINKER_FLAGS_NT2DEBUGEMPTY)
set(CMAKE_SHARED_LINKER_FLAGS_NT2DEBUGEMPTY ${CMAKE_SHARED_LINKER_FLAGS})
if(MSVC)
  set(CMAKE_EXE_LINKER_FLAGS_NT2BENCH "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
  set(CMAKE_SHARED_LINKER_FLAGS_NT2BENCH "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG")
endif()
set_property(GLOBAL APPEND PROPERTY DEBUG_CONFIGURATIONS Debug NT2Test NT2TestDebug NT2DebugEmpty)

include(options/nt2.extra.warnings)

if(CMAKE_CXX_FLAGS MATCHES "[^ ]")
  message(STATUS "[nt2] Global flags: ${CMAKE_CXX_FLAGS}")
endif()

# Make sure no build type is selected
set(CMAKE_BUILD_TYPE "")

message(STATUS "[nt2] Debug flags: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "[nt2] Release flags: ${CMAKE_CXX_FLAGS_RELEASE}")

message(STATUS "[nt2] Test flags: ${CMAKE_CXX_FLAGS_NT2TEST}")
message(STATUS "[nt2] Test debug flags: ${CMAKE_CXX_FLAGS_NT2TESTDEBUG}")
message(STATUS "[nt2] Bench flags: ${CMAKE_CXX_FLAGS_NT2BENCH}")

endif()
