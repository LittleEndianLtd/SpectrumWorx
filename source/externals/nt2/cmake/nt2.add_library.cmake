################################################################################
#         Copyright 2003 & onward LASMEA UMR 6602 CNRS/Univ. Clermont II
#         Copyright 2009 & onward LRI    UMR 8623 CNRS/Univ Paris Sud XI
#
#          Distributed under the Boost Software License, Version 1.0.
#                 See accompanying file LICENSE.txt or copy at
#                     http://www.boost.org/LICENSE_1_0.txt
################################################################################

if(NOT DEFINED NT2_ADD_LIBRARY_CMAKE_INCLUDED)
set(NT2_ADD_LIBRARY_CMAKE_INCLUDED 1)

include(nt2.add_library RESULT_VARIABLE FILE_NAME)
string(REGEX REPLACE ".cmake$" "" NT2_ADD_LIBRARY_DIR_NAME ${FILE_NAME})

# same as add_library, but will add the library for a particular build type
macro(nt2_add_library build_type target)
  if(CMAKE_CONFIGURATION_TYPES)
    add_library(${target} ${ARGN})
  else()
    set(OLD_BUILD_TYPE ${CMAKE_BUILD_TYPE})
    set(CMAKE_BUILD_TYPE ${build_type})
    set(ARGS ${target})
    foreach(arg ${ARGN})
      if(${arg} MATCHES "\\.(c|C|c\\+\\+|cc|cpp|cxx|m|M|mm|h|hh|h\\+\\+|hm|hpp|hxx|in|txx)$")
        if(NOT EXISTS ${arg})
          if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${arg})
            set(arg ${CMAKE_CURRENT_SOURCE_DIR}/${arg})
          elseif(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${arg})
            set(arg ${CMAKE_CURRENT_BINARY_DIR}/${arg})
          endif()
        endif()
      endif()
      list(APPEND ARGS ${arg})
    endforeach()

    add_subdirectory(${NT2_ADD_LIBRARY_DIR_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${target})
    set(CMAKE_BUILD_TYPE OLD_BUILD_TYPE)
  endif()
endmacro()

endif()
