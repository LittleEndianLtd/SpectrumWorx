################################################################################
#         Copyright 2003 & onward LASMEA UMR 6602 CNRS/Univ. Clermont II
#         Copyright 2009 & onward LRI    UMR 8623 CNRS/Univ Paris Sud XI
#
#          Distributed under the Boost Software License, Version 1.0.
#                 See accompanying file LICENSE.txt or copy at
#                     http://www.boost.org/LICENSE_1_0.txt
################################################################################

# only available since 2.8.3
include(CMakeParseArguments OPTIONAL RESULT_VARIABLE CMakeParseArguments_FOUND)
if(WAVE_EXECUTABLE AND NOT EXISTS WAVE_EXECUTABLE)
  unset(WAVE_EXECUTABLE CACHE)
endif()
find_program(WAVE_EXECUTABLE wave $ENV{BOOST_ROOT}/dist/bin)

macro(nt2_preprocess target)
  if(NOT WAVE_EXECUTABLE MATCHES "NOTFOUND$" AND (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX) AND CMakeParseArguments_FOUND AND (PROJECT_NAME MATCHES "^NT2"))
    set(NT2_PREPROCESS_ENABLED 1)

    get_directory_property(INCLUDES INCLUDE_DIRECTORIES)

    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/tmpfile "")
    execute_process(COMMAND ${CMAKE_C_COMPILER} -v -x c++ ${CMAKE_CURRENT_BINARY_DIR}/tmpfile -fsyntax-only
                    ERROR_VARIABLE COMPILER_VERSION_INFO
                   )
    file(REMOVE tmpfile)

    string(REGEX REPLACE "^.*#include <...>[^\n]*((\n [^\n]+)*)\n[^ ].*$" "\\1" INCLUDE_SYSTEM_DIRECTORIES ${COMPILER_VERSION_INFO} )
    string(REGEX REPLACE "^\n " "" INCLUDE_SYSTEM_DIRECTORIES ${INCLUDE_SYSTEM_DIRECTORIES} )
    string(REPLACE "\n " ";" INCLUDE_SYSTEM_DIRECTORIES ${INCLUDE_SYSTEM_DIRECTORIES} )

    set(INCLUDE_DIRECTORIES)
    foreach(INCLUDE ${INCLUDES})
      list(APPEND INCLUDE_DIRECTORIES "-S${INCLUDE}")
    endforeach()
    foreach(INCLUDE ${INCLUDE_SYSTEM_DIRECTORIES})
      list(APPEND INCLUDE_DIRECTORIES "-S${INCLUDE}")
    endforeach()

    cmake_parse_arguments(ARG "" "" "DEPENDS;OPTIONS" ${ARGN})

    add_custom_target(${target})
    set_property(TARGET ${target} PROPERTY FOLDER preprocess)

    set(limits -D__CHAR_BIT__=8 -D__SCHAR_MAX__=127 -D__SHRT_MAX__=32767 -D__INT_MAX__=2147483647 -D__LONG_LONG_MAX__=9223372036854775807LL)
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
      list(APPEND limits -D__LONG_MAX__=2147483647L)
    else()
      list(APPEND limits -D__LONG_MAX__=9223372036854775807L)
    endif()

    set(prev 0)
    set(depends)
    if(ARG_DEPENDS)
      set(depends DEPENDS ${ARG_DEPENDS})
    endif()
    foreach(src ${ARG_UNPARSED_ARGUMENTS})
      math(EXPR n "${prev} + 1")
      add_custom_target(${target}.${n}
                        COMMAND ${WAVE_EXECUTABLE} --variadics --long_long --timer ${limits} ${ARG_OPTIONS} ${INCLUDE_DIRECTORIES} -o - ${src}
                        ${depends}
                        WORKING_DIRECTORY ${NT2_BINARY_DIR}/include
                        COMMENT "wave ${src}"
                       )
      set_property(TARGET ${target}.${n} PROPERTY FOLDER preprocess)
      add_dependencies(${target} ${target}.${n})
      set(prev ${n})
    endforeach()

    # Create target "preprocess" if it doesn't already exist, and make it depend on target
    if(NOT TARGET preprocess)
      add_custom_target(preprocess)
      set_property(TARGET preprocess PROPERTY FOLDER preprocess)
    endif()
    add_dependencies(preprocess ${target})
  endif()
endmacro()
