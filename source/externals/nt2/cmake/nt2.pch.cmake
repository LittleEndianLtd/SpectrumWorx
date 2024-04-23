################################################################################
#         Copyright 2003 & onward LASMEA UMR 6602 CNRS/Univ. Clermont II
#         Copyright 2009 & onward LRI    UMR 8623 CNRS/Univ Paris Sud XI
#
#          Distributed under the Boost Software License, Version 1.0.
#                 See accompanying file LICENSE.txt or copy at
#                     http://www.boost.org/LICENSE_1_0.txt
################################################################################

set(NT2_WITH_PCH 0 CACHE BOOL "Whether to use precompiled headers on platforms that support it")

include(nt2.info)

macro(nt2_pch_file build_type out in)
  if(IS_ABSOLUTE ${in})
    set(arg ${in})
  elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${in})
    set(arg ${CMAKE_CURRENT_SOURCE_DIR}/${in})
  else()
    set(arg ${CMAKE_CURRENT_BINARY_DIR}/${in})
  endif()

  # Makefile generators do not support object dependencies outside of current directory
  if(CMAKE_GENERATOR MATCHES "Make")
    set(_deps ${NT2_PCH_TARGET}_${build_type}.pch)
    set(_object_deps ${arg})
  else()
    set(_deps)
    set(_object_deps ${arg} ${NT2_PCH_TARGET}_${build_type}.pch)
  endif()

  add_custom_command(OUTPUT ${out}
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different ${arg} ${out}
                     DEPENDS ${in} ${_deps}
                    )

  # Ideally, we should try to copy all properties
  get_property(defs SOURCE ${in} PROPERTY COMPILE_DEFINITIONS)

  set(path "${NT2_PCH_FILE}_${build_type}")
  set_source_files_properties(${out}
                              PROPERTIES OBJECT_DEPENDS "${_object_deps}"
                                         GENERATED ON
                                         COMPILE_FLAGS "-include \"${path}\" -Winvalid-pch"
                                         COMPILE_DEFINITIONS "${defs}"
                             )
endmacro()

macro(nt2_pch name)
  if(NT2_PCH_TARGET)
    message(FATAL_ERROR "[nt2.pch] precompiled header already set to ${NT2_PCH_TARGET}, cannot change to ${name}")
  endif()

  if( NT2_WITH_PCH
      AND NT2_COMPILER_GCC_LIKE
      AND CMAKE_GENERATOR MATCHES "Make" OR
         (CMAKE_GENERATOR MATCHES "Ninja" AND CMAKE_VERSION VERSION_EQUAL 2.8.10 OR CMAKE_VERSION VERSION_GREATER 2.8.10) # correct OBJECT_DEPENDS handling requires CMake 2.8.10
    )

    if(NOT TARGET pch)
      add_custom_target(pch ALL)
    endif()

    file(RELATIVE_PATH dir ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR})
    foreach(BUILD_TYPE Debug Release NT2Test NT2TestDebug NT2Bench)
      string(TOUPPER ${BUILD_TYPE} BUILD_TYPE_U)
      set(dirname "${dir}/${name}")
      set(dirname_ "${dir}/${name}/${BUILD_TYPE}")
      string(REPLACE "/" "_" pch_base ${dirname})
      string(REPLACE "/" "_" pch_base_ ${dirname_})
      string(REGEX REPLACE "\\.hpp$" "" rule ${pch_base})
      string(REGEX REPLACE "\\.hpp$" "_${BUILD_TYPE}" rule_ ${pch_base})

      file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${pch_base_} "#include <${name}>\n")
      set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${pch_base_} PROPERTIES LANGUAGE CXX)

      set(object_suffix .o)
      if(WIN32)
        set(object_suffix .obj)
      endif()

      add_library(${rule_}.pch STATIC ${CMAKE_CURRENT_BINARY_DIR}/${pch_base_})
      set_target_properties(${rule_}.pch PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_${BUILD_TYPE_U}} -x c++-header")
      add_custom_command(TARGET ${rule_}.pch POST_BUILD
                         COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${rule_}.pch.dir/${pch_base_}${object_suffix}
                                                          ${CMAKE_CURRENT_BINARY_DIR}/${pch_base_}.gch
                         COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${rule_}.pch.dir/${pch_base_}${object_suffix}
                         COMMAND ${CMAKE_COMMAND} -E touch  ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${rule_}.pch.dir/${pch_base_}${object_suffix}
                         COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}${rule_}.pch${CMAKE_STATIC_LIBRARY_SUFFIX}
                         COMMAND ${CMAKE_COMMAND} -E touch  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}${rule_}.pch${CMAKE_STATIC_LIBRARY_SUFFIX}
                        )
      add_dependencies(pch ${rule_}.pch)
      set(NT2_PCH_TARGET ${rule})
      set(NT2_PCH_FILE ${CMAKE_CURRENT_BINARY_DIR}/${pch_base})
    endforeach()
  endif()
endmacro()
