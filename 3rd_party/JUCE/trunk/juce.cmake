################################################################################
#
# LEB JUCE module
#
# Copyright (c) 2013 - 2016. Little Endian Ltd. All rights reserved.
#
################################################################################

#...mrmlj...avoid including buildOptions only for osSuffix...
#include( "${PROJECT_SOURCE_DIR}/externals/LEB/buildOptions.cmake" )
if ( NOT LEB_OS_SUFFIX )
    message( FATAL_ERROR "Include buildOptions.cmake before juce.cmake" )
endif()

include_directories(
    AFTER
    "$ENV{LEB_3rdParty_root}/juce/trunk"
)

set( juceLibsPath "$ENV{LEB_3rdParty_root}/juce/trunk/lib" )

if ( APPLE )
    link_directories( "${juceLibsPath}/$(CONFIGURATION)" ) #...mrmlj...CMAKE_CFG_INTDIR !?
else()
    link_directories( "${juceLibsPath}/${CMAKE_CFG_INTDIR}" )
endif()

# http://cmake.3232098.n2.nabble.com/MSVC-VERSION-for-VC11-td7345691.html
if ( MSVC_VERSION )
    set( compilerSuffix "_msvc${MSVC_VERSION}" )
endif()


function( addJUCE target )

    appendProperty( ${target} COMPILE_DEFINITIONS LEB_PRECOMPILE_JUCE )

    set( juceLibName "juce_${LEB_OS_SUFFIX}${compilerSuffix}${LE_JUCE_LIB_EXTRA_SUFFIX}" )
    if ( WIN32 )
        appendProperty( ${target} COMPILE_DEFINITIONS UNICODE _UNICODE )
        target_link_libraries( ${target}
            ${juceLibName}
            shlwapi
            version
            winmm
        )
    elseif ( APPLE )
        target_link_libraries( ${target}
            ${juceLibName}
            "-framework Carbon"
            "-framework Cocoa"
            "-framework QuartzCore"
        )
    endif()

endfunction()
