################################################################################
#
# LE Utility project file
#
# Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
#
################################################################################


cmake_minimum_required( VERSION 3.1 )


set( LE_PROJECT_NAME  "LE_Utility" )
set( LE_PROJECT_LABEL "Utility"    )

set( leExternals "externals/le" )

include( "${PROJECT_SOURCE_DIR}/${leExternals}/build/buildOptions.cmake" )
include( "${PROJECT_SOURCE_DIR}/${leExternals}/build/sdkProject.cmake"   )

set( LE_UTILITY_MATLAB_INTEROP false CACHE BOOL "enable the Matlab module" )
LE_configureFeatureOption( LE_UTILITY_MATLAB_INTEROP )


############################################################################
# Source files setup
############################################################################

set( Headers
    abi.hpp
    assert.hpp
    buffers.hpp
    clear.hpp
    countof.hpp
    conditionVariable.hpp
    criticalSection.hpp
    demoLimiter.hpp
    entryPoint.hpp
    filesystem.hpp
    filesystemImpl.hpp
    filesystemImpl.inl
    intrinsics.hpp
    jni.hpp
    lexicalCast.hpp
    objc.hpp
    objcfwdhelpers.hpp
    parentFromMember.hpp
    pimpl.hpp
    pimplPrivate.hpp
    platformSpecifics.hpp
    profiler.hpp
    referenceCounter.hpp
    rvalueReferences.hpp
    sleep.hpp
    staticForEach.hpp
    switch.hpp
    tchar.hpp
    trace.hpp
    tracePrivate.hpp
    typeTraits.hpp
    windowsLite.hpp
    xml.hpp
)
set( Sources
    assertionHandler.cpp
    filesystem.cpp
    filesystemAndroid.cpp
    filesystemApple.cpp
    filesystemWindows.cpp
    jni.cpp
    lexicalCast.cpp
    objc.mm
    profiler.cpp
    sleep.cpp
    trace.cpp
)
if ( LE_UTILITY_XML )
    list( APPEND Sources xml.cpp )
endif()
if ( MSVC )
    set( Sources ${Sources} msvcUniversalbuild.cpp )
    if ( LE_UTILITY_MATLAB_INTEROP )
        list( APPEND Headers matlab.hpp )
        list( APPEND Sources matlab.cpp )
        #...mrmlj...this still needs to be manually added to/set in the project: PATH=$(ProgramW6432)\Matlab\bin\win64;
        set( MatlabExternDir "$ENV{ProgramW6432}/Matlab/extern" )
        file( TO_CMAKE_PATH "${MatlabExternDir}" MatlabExternDir )
        include_directories( AFTER "${MatlabExternDir}/include" )
        link_directories   (       "${MatlabExternDir}/lib/win64/microsoft" )
        add_definitions( -DLE_UTILITY_MATLAB_INTEROP_LIB_DIR="${MatlabExternDir}/lib/win64/microsoft/" )
    endif()
endif()
source_group( "Headers" FILES ${Headers} )
source_group( "Sources" FILES ${Sources} )

set_source_files_properties( filesystemAndroid.cpp filesystemApple.cpp filesystemWindows.cpp jni.cpp objc.mm PROPERTIES HEADER_FILE_ONLY true )
if ( ANDROID )
    set_source_files_properties( filesystemAndroid.cpp PROPERTIES HEADER_FILE_ONLY false )
    set_source_files_properties( jni.cpp               PROPERTIES HEADER_FILE_ONLY false )
elseif( APPLE )
    set_source_files_properties( filesystemApple.cpp PROPERTIES HEADER_FILE_ONLY false )
    set_source_files_properties( filesystemApple.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++" )
    set_source_files_properties( trace.cpp           PROPERTIES COMPILE_FLAGS "-x objective-c++" )
    set_source_files_properties( objc.mm             PROPERTIES HEADER_FILE_ONLY false )
elseif( WIN32 )
    set_source_files_properties( filesystemWindows.cpp PROPERTIES HEADER_FILE_ONLY false )
endif()

if ( ANDROID )
    include_directories(
        AFTER
        "${ANDROID_NDK}/sources/android/native_app_glue"
        "${ANDROID_NDK}/sources/android/ndk_helper"
    )
elseif ( WIN32 )
    add_definitions( -D_WIN32_WINNT=_WIN32_WINNT_VISTA )
endif()


include_directories(
    BEFORE
    "${CMAKE_SOURCE_DIR}/externals"
    "$ENV{LEB_3rdParty_root}/boost/${boostVersion}"
)
if ( LE_UTILITY_XML )
    include_directories( "$ENV{LEB_3rdParty_root}/RapidXML/${rapidXMLVersion}" )
endif()

set( LEB_SDK_COMPILE_FLAGS         "" )
set( LEB_SDK_COMPILE_FLAGS_RELEASE "${sizeOptimizationSwitch} ${vectorizeOffSwitch} ${rttiOffSwitch} ${exceptionsOffSwitch}" )
if ( ANDROID OR iOS )
    set( armCompilerSwitches "${armCompilerSwitches} -mthumb" )
endif()

set( LE_SDK_DOCUMENTATION_SOURCES
    "${LE_SDK_DOCUMENTATION_SOURCES}"
    "${CMAKE_CURRENT_SOURCE_DIR}/filesystem.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/jni.hpp"
    CACHE INTERNAL "" FORCE
)
if ( LE_UTILITY_XML )
    set( LE_SDK_DOCUMENTATION_SOURCES
        "${LE_SDK_DOCUMENTATION_SOURCES}"
        "${CMAKE_CURRENT_SOURCE_DIR}/xml.hpp"
        CACHE INTERNAL "" FORCE
    )
endif()

createPublicTarget( ${LE_PROJECT_NAME} ${LE_PROJECT_LABEL} "${Sources}" "${Headers}" )

if ( LE_SDK_BUILD )
    if ( LE_CREATE_PACKAGE_TARGET )
        if ( ANDROID )
            if ( DEFINED LE_SDK_ANDROID_MK_DIRECTORY )
                install(
                    FILES          "${CMAKE_CURRENT_SOURCE_DIR}/le_utility.mk"
                    DESTINATION    ${LE_SDK_ANDROID_MK_DIRECTORY}
                    COMPONENT      Utility
                    CONFIGURATIONS ${installConfigs}
                )
            endif()
            if ( DEFINED LE_SDK_ANDROID_GRADLE_DIRECTORY )
                install(
                    FILES          "${CMAKE_CURRENT_SOURCE_DIR}/build.gradle"
                                   "${CMAKE_CURRENT_SOURCE_DIR}/utility.gradle"
                    DESTINATION    ${LE_SDK_ANDROID_GRADLE_DIRECTORY}/utility
                    COMPONENT      Utility
                    CONFIGURATIONS ${installConfigs}
                )
            endif()
            install(
                FILES          "jni.hpp"
                DESTINATION    "include/le/utility"
                COMPONENT      Utility
                CONFIGURATIONS ${installConfigs}
            )
        elseif( APPLE )
            install(
                FILES          "objc.hpp"
                DESTINATION    "include/le/utility"
                COMPONENT      Utility
                CONFIGURATIONS ${installConfigs}
            )
        endif()

        install(
            FILES
                "abi.hpp"
                "assert.hpp"
                "filesystem.hpp"
                "pimpl.hpp"
                "profiler.hpp"
                "sleep.hpp"
                "trace.hpp"
                "xml.hpp"
            DESTINATION    "include/le/utility"
            COMPONENT      Utility
            CONFIGURATIONS ${installConfigs}
        )

        if ( false ) #...mrmlj...CPack not included yet...
            cpack_add_component(
                Utility
                DISPLAY_NAME "LE.Utility"
                GROUP ${projectName}
            )
        endif()

        #...mrmlj...internal sdkProject.cmake knowledge...
        forwardToParentScope( releaseBinaries     )
        forwardToParentScope( developmentBinaries )
    endif()
endif()
