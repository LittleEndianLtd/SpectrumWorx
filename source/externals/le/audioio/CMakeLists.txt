################################################################################
#
# AudioIO build file
#
# Copyright (c) 2012 - 2016. Little Endian Ltd. All rights reserved.
#
################################################################################


cmake_minimum_required( VERSION 2.8.12 )


set( LE_AIO_PROJECT_NAME  "LE_AudioIO_SDK" CACHE INTERNAL "" FORCE )
set( LE_AIO_PROJECT_LABEL "Audio IO SDK"   CACHE INTERNAL "" FORCE )
set( LE_AIO_VERSION_MAJOR "2"              CACHE INTERNAL "" FORCE )
set( LE_AIO_VERSION_MINOR "2"              CACHE INTERNAL "" FORCE )
set( LE_AIO_VERSION_PATCH "0"              CACHE INTERNAL "" FORCE )

set( LE_AIO_PROJECT_DESCRIPTION "The easy audio input and output library. Copyright (C) 2013 - 2016. Little Endian. http://www.littleendian.com" CACHE INTERNAL "" FORCE )

set( leExternals "externals/le" )

include( "${CMAKE_SOURCE_DIR}/${leExternals}/build/3rdPartyLibs.cmake" )
include( "${CMAKE_SOURCE_DIR}/${leExternals}/build/buildOptions.cmake" )
include( "${CMAKE_SOURCE_DIR}/${leExternals}/build/sdkProject.cmake"   )

############################################################################
# Source files setup
############################################################################

set( Sources_Device
    device/blockingDevice.cpp
    device/device.hpp
    device/deviceImpl.hpp
    device/android.cpp
    device/ios.cpp
    device/osx.cpp
    device/rtAudio.cpp
    device/windows.cpp
    device/pimplForwarders.inl
)
source_group( "Device" FILES ${Sources_Device} )
set( Sources_File
    file/file.hpp
    file/fileAndroid.cpp
    file/fileApple.cpp
    file/fileWindows.cpp
    file/inputWaveFile.hpp
    file/outputWaveFile.hpp
    file/inputWaveFileImpl.hpp
    file/outputWaveFileImpl.hpp
    file/structures.hpp
    file/inputWaveFileImpl.cpp
    file/outputWaveFileImpl.cpp
    file/pimplForwarders.inl
)
source_group( "File" FILES ${Sources_File} )

set( Sources_Shared
    openSL.hpp
    openSL.cpp
    le_audioio_sdk.mk
)

set_source_files_properties( ${Sources_Device} openSL.cpp file/fileAndroid.cpp file/fileApple.cpp file/fileWindows.cpp PROPERTIES HEADER_FILE_ONLY true )
if ( ANDROID )
    set_source_files_properties( device/android.cpp   PROPERTIES HEADER_FILE_ONLY false )
    set_source_files_properties( file/fileAndroid.cpp PROPERTIES HEADER_FILE_ONLY false )
    set_source_files_properties( openSL.cpp           PROPERTIES HEADER_FILE_ONLY false )
elseif( iOS )
    set_source_files_properties( device/ios.cpp       PROPERTIES HEADER_FILE_ONLY false )
    set_source_files_properties( file/fileApple.cpp   PROPERTIES HEADER_FILE_ONLY false )
elseif( APPLE )
    set_source_files_properties( device/osx.cpp       PROPERTIES HEADER_FILE_ONLY false )
    set_source_files_properties( file/fileApple.cpp   PROPERTIES HEADER_FILE_ONLY false )
elseif( WIN32 )
    set_source_files_properties( device/rtAudio.cpp   PROPERTIES HEADER_FILE_ONLY false )
    set_source_files_properties( device/windows.cpp   PROPERTIES HEADER_FILE_ONLY false )
    set_source_files_properties( file/fileWindows.cpp PROPERTIES HEADER_FILE_ONLY false )
endif()
set_source_files_properties( device/blockingDevice.cpp PROPERTIES HEADER_FILE_ONLY false )

if ( APPLE )
    set_source_files_properties( inputWaveFileImpl.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++ -fno-objc-exceptions" )
elseif( WIN32 )
    #...mrmlj...RtAudio.cpp does not explicitly include objbase.h and includes <algorithm> too late for WASAPI...
    add_definitions( -D__WINDOWS_WASAPI__ -D__WINDOWS_ASIO__ -DWIN32 -DLEB_INCLUDE_FULL_WINDOWS_HEADERS -DNOMINMAX )
    addForceInclude( algorithm )
endif()

if ( LE_SDK_DEMO )
    add_definitions( -DLE_SDK_DEMO_BUILD )
endif()


include_directories(
    BEFORE
    ${PROJECT_SOURCE_DIR}
    "$ENV{LEB_3rdParty_root}/boost/${boostVersion}"
    "${CMAKE_SOURCE_DIR}/externals"
)
if ( WIN32 ) # RtAudio targets...
    set( rtAudioVersion rtaudio-4.1.2 )
    add3rdPartyLib( RtAudio
        4.1.2
        4.1.2
        http://www.music.mcgill.ca/~gary/rtaudio/release/rtaudio-4.1.2.tar.gz
        rtaudio-4.1.2.tar.gz
        ""
        rtaudio-4.1.2
    )

    include_directories( BEFORE
        "${RtAudio_INCLUDE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/device/patchedRtAudio" #...mrmlj...CMake actually adds them in reverse order...
    )
    #...mrmlj...RtAudio and ASIO cause weird compilation errors in pimplPrivate.hpp in unity builds (it seems due to the inclusion of windows.h)
    set( Sources_Externals_ASIO
        "${RtAudio_INCLUDE_DIR}/include/asio.cpp"
        "${RtAudio_INCLUDE_DIR}/include/asiodrivers.cpp"
        "${RtAudio_INCLUDE_DIR}/include/asiolist.cpp"
    )
    set( Sources_Externals_RTAudio
        #...mrmlj...RtAudio w/o iostream usage...
        "${CMAKE_CURRENT_SOURCE_DIR}/device/patchedRtAudio/RtAudio.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/device/patchedRtAudio/RtAudio.h"
    )

    source_group( "Externals\\RtAudio" FILES ${Sources_Externals_RTAudio} ${Sources_Externals_ASIO} )

    set( LEB_SDK_COMPILE_FLAGS ${exceptionsOnSwitch} )
else()
    set( LEB_SDK_COMPILE_FLAGS ${exceptionsOffSwitch} )
endif()

set( LEB_SDK_COMPILE_FLAGS_RELEASE "${sizeOptimizationSwitch} ${vectorizeOnSwitch} ${rttiOffSwitch}" )
if ( iOS )
    set( armCompilerSwitches "${armCompilerSwitches} -mthumb" )
endif()

if ( LE_CREATE_PACKAGE_TARGET )
    set( LE_SDK_DOCUMENTATION_SOURCES
        "${LE_SDK_DOCUMENTATION_SOURCES}"
        "${CMAKE_CURRENT_SOURCE_DIR}/documentation"
        "${CMAKE_CURRENT_SOURCE_DIR}/device/device.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/file/file.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/file/inputWaveFile.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/file/outputWaveFile.hpp"
        #...mrmlj...Utility's CMakeLists.txt gets included too late...
        "${CMAKE_SOURCE_DIR}/${leExternals}/utility/filesystem.hpp"
        "${CMAKE_SOURCE_DIR}/${leExternals}/utility/jni.hpp"
        CACHE INTERNAL "" FORCE
    )

    set( LE_SDK_DOCUMENTATION_EXTRA_IMAGE_PATHS
        "${LE_SDK_DOCUMENTATION_EXTRA_IMAGE_PATHS}"
        "${CMAKE_CURRENT_SOURCE_DIR}/documentation"
        CACHE INTERNAL "" FORCE
    )
endif ( LE_CREATE_PACKAGE_TARGET ) 

if ( standaloneAudioIO )
    set( LE_DOXYGEN_ENABLED_SECTIONS "${LE_DOXYGEN_ENABLED_SECTIONS} LE_STANDALONE_AUDIOIO" )
    createSDKProject(
        "${LE_AIO_PROJECT_NAME}"
        "${LE_AIO_PROJECT_LABEL}"
        "${LE_AIO_PROJECT_DESCRIPTION}"
        "${LE_AIO_VERSION_MAJOR}"
        "${LE_AIO_VERSION_MINOR}"
        "${LE_AIO_VERSION_PATCH}"
        "${Sources_Device};${Sources_File};${Sources_Shared}"
        "${Sources_Externals_ASIO};${Sources_Externals_RTAudio}"
    )
else()
    createPublicTarget(
        ${LE_AIO_PROJECT_NAME}
        ${LE_AIO_PROJECT_LABEL}
        "${Sources_Device};${Sources_File};${Sources_Shared}"
        ""
        "${Sources_Externals_ASIO};${Sources_Externals_RTAudio}"
    )
endif()

if ( CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER MATCHES clang )
    set_property( DIRECTORY APPEND PROPERTY COMPILE_FLAGS -Wno-unused-private-field )
    set_property( GLOBAL    APPEND PROPERTY COMPILE_FLAGS -Wno-unused-private-field )
endif()


if ( LE_SDK_BUILD )
    if ( LE_CREATE_PACKAGE_TARGET )
        if ( ANDROID AND DEFINED LE_SDK_ANDROID_MK_DIRECTORY )
            install(
                FILES          "${CMAKE_CURRENT_SOURCE_DIR}/le_audioio_sdk.mk"
                DESTINATION    ${LE_SDK_ANDROID_MK_DIRECTORY}
                COMPONENT      audioio
                CONFIGURATIONS ${installConfigs}
            )
            install(
                FILES          "${CMAKE_CURRENT_SOURCE_DIR}/build.gradle"
                DESTINATION    ${LE_SDK_ANDROID_GRADLE_DIRECTORY}/audioio
                COMPONENT      audioio
                CONFIGURATIONS ${installConfigs}
            )
        endif()

        install(
            FILES
                "${CMAKE_CURRENT_SOURCE_DIR}/device/device.hpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/file/file.hpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/file/inputWaveFile.hpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/file/outputWaveFile.hpp"
            DESTINATION    "include/le/audioio"
            COMPONENT      audioio
            CONFIGURATIONS ${installConfigs}
        )
        
        if ( WIN32 )
            # Boost.Function for targets w/o Blocks or w/o a usable std::function (remove after switching to VS2015)
            install(
                FILES
                    "${Boost_INCLUDE_DIR}/boost/assert.hpp"
                    "${Boost_INCLUDE_DIR}/boost/config.hpp"
                    "${Boost_INCLUDE_DIR}/boost/cstdint.hpp"
                    "${Boost_INCLUDE_DIR}/boost/current_function.hpp"
                    "${Boost_INCLUDE_DIR}/boost/function.hpp"
                    "${Boost_INCLUDE_DIR}/boost/function_equal.hpp"
                    "${Boost_INCLUDE_DIR}/boost/get_pointer.hpp"
                    "${Boost_INCLUDE_DIR}/boost/integer.hpp"
                    "${Boost_INCLUDE_DIR}/boost/integer_fwd.hpp"
                    "${Boost_INCLUDE_DIR}/boost/integer_traits.hpp"
                    "${Boost_INCLUDE_DIR}/boost/limits.hpp"
                    "${Boost_INCLUDE_DIR}/boost/mem_fn.hpp"
                    "${Boost_INCLUDE_DIR}/boost/ref.hpp"
                    "${Boost_INCLUDE_DIR}/boost/static_assert.hpp"
                    "${Boost_INCLUDE_DIR}/boost/throw_exception.hpp"
                    "${Boost_INCLUDE_DIR}/boost/version.hpp"
                DESTINATION    "include/boost"
                COMPONENT      Boost
                CONFIGURATIONS ${installConfigs}
            )
            install(
                FILES
                    "${Boost_INCLUDE_DIR}/boost/detail/no_exceptions_support.hpp"
                    "${Boost_INCLUDE_DIR}/boost/detail/sp_typeinfo.hpp"
                    "${Boost_INCLUDE_DIR}/boost/detail/workaround.hpp"
                DESTINATION    "include/boost/detail"
                COMPONENT      Boost
                CONFIGURATIONS ${installConfigs}
            )
            install(
                DIRECTORY
                    "${Boost_INCLUDE_DIR}/boost/bind"
                    "${Boost_INCLUDE_DIR}/boost/config"
                    "${Boost_INCLUDE_DIR}/boost/core"
                    "${Boost_INCLUDE_DIR}/boost/exception"
                    "${Boost_INCLUDE_DIR}/boost/function"
                    "${Boost_INCLUDE_DIR}/boost/move"
                    "${Boost_INCLUDE_DIR}/boost/mpl"
                    "${Boost_INCLUDE_DIR}/boost/preprocessor"
                    "${Boost_INCLUDE_DIR}/boost/type_traits"
                    "${Boost_INCLUDE_DIR}/boost/utility"
                DESTINATION    "include/boost"
                COMPONENT      Boost
                CONFIGURATIONS ${installConfigs}
                # remove at least some of the unnecessary Boost.MPL baggage
                REGEX   "preprocessed/bcc" EXCLUDE # a bcc.hpp header is required so a simple bcc* pattern cannot be used
                PATTERN "dmc"              EXCLUDE
                PATTERN "msvc60"           EXCLUDE
                PATTERN "msvc70"           EXCLUDE
                PATTERN "msvc60"           EXCLUDE
                PATTERN "mwcw"             EXCLUDE
                PATTERN "no_ctps"          EXCLUDE
                PATTERN "no_ttp"           EXCLUDE
            )
        endif( WIN32 )

        if ( standaloneAudioIO ) #...mrmlj...CPack not included yet otherwise...
            include( CPack )
            cpack_add_component(
                audioio
                DISPLAY_NAME "LE.AudioIO"
                GROUP ${projectName}
            )
        endif()

        #...mrmlj...internal sdkProject.cmake knowledge...
        forwardToParentScope( releaseBinaries     )
        forwardToParentScope( developmentBinaries )
    endif()
endif()
