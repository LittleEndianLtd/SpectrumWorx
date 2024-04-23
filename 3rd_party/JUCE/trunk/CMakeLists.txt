################################################################################
#
# LE customized JUCE library build
# http://www.juce.com
# http://www.rawmaterialsoftware.com
#
# Copyright (c) 2013 - 2016. Little Endian Ltd. All rights reserved.
#
################################################################################


cmake_minimum_required( VERSION 2.8.12 )

set( LE_PROJECT_NAME  "juce${compilerSuffix}" )
set( LE_PROJECT_LABEL "JUCE"                  )

project( ${LE_PROJECT_NAME} CXX C )

include( "${CMAKE_SOURCE_DIR}/externals/LEB/buildOptions.cmake" )


############################################################################
# Source files setup
############################################################################

if ( APPLE )
    set( sourceExtension mm )
    set( libPrefix lib ) #...mrmlj...
else()
    set( sourceExtension cpp )
    set( libPrefix "" )
endif()

set( headers_le
    AppConfig.h
    beginIncludes.hpp
    endIncludes.hpp
    juce.cmake
)
source_group( "LE" FILES ${headers_le} )
    
include_directories(
    BEFORE
    "${CMAKE_CURRENT_SOURCE_DIR}/externals"
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( LE_JUCESources
    modules/juce_core/juce_core.${sourceExtension}
    modules/juce_data_structures/juce_data_structures.${sourceExtension}
    modules/juce_events/juce_events.${sourceExtension}
    modules/juce_graphics/juce_graphics.${sourceExtension}
    modules/juce_gui_basics/juce_gui_basics.${sourceExtension}
    ${headers_le}
)

include( juce.cmake )

add_library( ${LE_PROJECT_NAME} STATIC ${LE_JUCESources} )
set_property( TARGET ${LE_PROJECT_NAME} PROPERTY PROJECT_LABEL "${LE_PROJECT_LABEL}"                                              )
set_property( TARGET ${LE_PROJECT_NAME} PROPERTY OUTPUT_NAME   "${libPrefix}${LE_PROJECT_NAME}_${LEB_OS_SUFFIX}${compilerSuffix}" )
if ( MSVC )
    add_library( ${LE_PROJECT_NAME}_MD STATIC ${LE_JUCESources} )
    set_property( TARGET ${LE_PROJECT_NAME}_MD PROPERTY PROJECT_LABEL "${LE_PROJECT_LABEL} (MD)"                                            )
    set_property( TARGET ${LE_PROJECT_NAME}_MD PROPERTY OUTPUT_NAME   "${libPrefix}${LE_PROJECT_NAME}_${LEB_OS_SUFFIX}${compilerSuffix}_MD" )
endif()

#addPCH( "${LE_PROJECT_NAME}" externals/LEB/precompiledHeaders )


set_property( DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Release>:${disableAsserts}> )
# Implementation note:
#   Disabling warnings with set_property or add_compile_options or
# target_compile_options does not seem to work here with MSVC10.
#                                             (28.08.2013.) (Domagoj Saric)
if ( MSVC )
    set( LEB_cppSpecificFlags "${LEB_cSpecificFlags} /wd4100 /wd4355 /wd4702" CACHE INTERNAL "" FORCE )
endif()
set( LEB_cSpecificFlags      "${LEB_cSpecificFlags} ${debugSymbolsCompilerSwitch}"                       CACHE INTERNAL "" FORCE )
set( LEB_cppSpecificFlags    "${LEB_cppSpecificFlags} ${exceptionsOnSwitch} ${rttiOnSwitch}"             CACHE INTERNAL "" FORCE )
set( LEB_sharedCReleaseFlags "${LEB_sharedCReleaseFlags} ${sizeOptimizationSwitch} ${ltoCompilerSwitch}" CACHE INTERNAL "" FORCE )
LE_setFinalFlags()
appendProperty( ${LE_PROJECT_NAME} LINK_FLAGS_RELEASE ${ltoLinkerSwitch} )
addPrefixHeader( ${LE_PROJECT_NAME} "${PROJECT_SOURCE_DIR}/externals/LEB/leConfigurationAndODRHeader.h" )
if ( MSVC )
    appendProperty( ${LE_PROJECT_NAME}_MD LINK_FLAGS_RELEASE ${ltoLinkerSwitch} )
    target_compile_options( ${LE_PROJECT_NAME}_MD PRIVATE "$<$<CONFIG:Debug>:/MDd>$<$<NOT:$<CONFIG:Debug>>:/MD>" )
    addPrefixHeader( ${LE_PROJECT_NAME}_MD "${PROJECT_SOURCE_DIR}/externals/LEB/leConfigurationAndODRHeader.h" )
    if ( NOT CMAKE_GENERATOR MATCHES 64 )
        appendProperty( ${LE_PROJECT_NAME}_MD COMPILE_FLAGS /arch:SSE2 )
    endif()
endif()

setupTargetForPlatform( ${LE_PROJECT_NAME} ${LE_TARGET_ARCHITECTURE} )

if ( MSVC )
    add_definitions    ( -DJUCE_STRING_UTF_TYPE=16 ${unicodeOnSwitch} )
    add_compile_options( /wd4100 /wd4355 /wd4702 ) #...mrmlj...see above...does not work!?

    # Implementation note:
    # http://www.cmake.org/pipermail/cmake/2009-February/026858.html
    # http://www.mail-archive.com/cmake@cmake.org/msg14083.html
    #                                        (18.10.2010.) (Domagoj Saric)
    #set_target_properties( ${LE_PROJECT_NAME} PROPERTIES PREFIX "../" )
elseif ( APPLE )
    add_definitions( -DJUCE_STRING_UTF_TYPE=8 )
    appendProperty( ${LE_PROJECT_NAME} COMPILE_FLAGS "${x86CompilerSwitches}" )
    set( CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT       dwarf-with-dsym )
    set( CMAKE_XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS yes             )

    set_target_properties( ${LE_PROJECT_NAME} PROPERTIES PREFIX "" )
endif()


set( CMAKE_INSTALL_PREFIX "$ENV{LEB_3rdParty_root}/juce/trunk" )

install(
    DIRECTORY   modules/juce_core
                modules/juce_events
                modules/juce_data_structures
                modules/juce_graphics
                modules/juce_gui_basics
    DESTINATION juce
    COMPONENT   Headers
    FILES_MATCHING PATTERN "*.h*"
)

install(
    FILES          AppConfig.h
                   beginIncludes.hpp
                   endIncludes.hpp
    DESTINATION    juce
    COMPONENT      Headers
    #PERMISSIONS    ${sharedPermissions}
    #CONFIGURATIONS Debug Release
)

install(
    FILES          juce.cmake
    DESTINATION    .
    COMPONENT      Build
)

# http://www.cmake.org/pipermail/cmake/2010-May/036979.html
install(
    TARGETS     ${LE_PROJECT_NAME}
    DESTINATION lib/\${CMAKE_INSTALL_CONFIG_NAME}
    COMPONENT   Library
)
if ( MSVC )
    install(
        TARGETS     ${LE_PROJECT_NAME}_MD
        DESTINATION lib/\${CMAKE_INSTALL_CONFIG_NAME}
        COMPONENT   Library
    )
endif()
