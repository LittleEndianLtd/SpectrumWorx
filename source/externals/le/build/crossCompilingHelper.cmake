################################################################################
#
# crossCompilingHelper.cmake
#
# Copyright (c) 2013 - 2016. Little Endian Ltd. All rights reserved.
#
################################################################################

include( ${CMAKE_SOURCE_DIR}/externals/le/build/utilities.cmake )


#...mrmlj...Ninja...cleanup&relocate...
# Implementation note: currently we perform Android builds only from Windows so
# setup Ninja only if ( WIN32 ).
#                                             (13.10.2015.) (Domagoj Saric)
if ( WIN32 )
    set( androidGenerator "Ninja" )
    #set( androidGenerator "Eclipse CDT4 - MinGW Makefiles" )
    find_file( ninjaPath ninja.exe C:/Windows NO_DEFAULT_PATH )
    if ( NOT ninjaPath )
        set( ninjaDownload ninja-win.zip )
        LE_makeTempPath( ninjaDownload )
        file( DOWNLOAD
            https://github.com/martine/ninja/releases/download/v1.6.0/ninja-win.zip
            ${ninjaDownload}
            STATUS success
            SHOW_PROGRESS
        )
        list( GET success 0 success )
        if ( NOT success EQUAL 0 )
            message( FATAL_ERROR "[LEB] Error downloading Ninja" )
        endif()
        execute_process( COMMAND ${CMAKE_COMMAND} -E tar xzf
            "${ninjaDownload}"
            WORKING_DIRECTORY "C:/Windows"
            RESULT_VARIABLE success
            OUTPUT_VARIABLE output
            ERROR_VARIABLE  output
        )
        if ( NOT success EQUAL 0 )
            message( FATAL_ERROR "[LEB] Error extracting Ninja to C:/Windows." )
        endif()
    endif( NOT ninjaPath )
    unset( ninjaPath CACHE )
endif( WIN32 )

#...mrmlj...toolchain files don't seem to be processed before project() is
#...mrmlj...called (and CMake seems to expect CMAKE_MAKE_PROGRAM to be set
#...mrmlj...before the toolchain file is processed) so we need to pre-setup
#...mrmlj...some variables for the toolchain beforehand...
#...mrmlj...so this file has to be included before calling project() in project
#...mrmlj...files that expect to be cross compiled...
# Implementation note:
#   The additional OR CMAKE_CROSSCOMPILING check is required because of our
# child-CMake-with-CMAKE_TOOLCHAIN_FILE bug workaround. See the below notes for
# more information...
#                                             (28.02.2014.) (Domagoj Saric)
if ( CMAKE_TOOLCHAIN_FILE OR CMAKE_CROSSCOMPILING )

    if ( WIN32 ) #...mrmlj...detect Android compilation (as opposed to iOS)...
        # Android NDK version
        # https://developer.android.com/tools/sdk/ndk/index.html
        set( androidNDKVersion r11c )
        # set( androidNDKPackageHash 8cd244fc799d0e6e59d65a59a8692588 )
        # add3rdPartyLib( Android/NDK #...mrmlj...no subdirectory support yet...
            # ${androidNDKVersion}
            # ${androidNDKVersion}
            # http://dl.google.com/android/ndk/android-ndk-r9d-windows-x86_64.zip
            # android-ndk-r9d-windows-x86_64.zip
            # ${androidNDKPackageHash}
            # ""
        # )
        set( ANDROID_NDK "$ENV{LEB_3rdParty_root}/Android/NDK/android-ndk-${androidNDKVersion}" )

        # Implementation note:
        #   The manual-inclusion-of-the-toolchain-file workaround, for an unknown
        # reason, requires the manual setting of CMAKE_HOST_SYSTEM_PROCESSOR on 64
        # bit Windows.
        #                                         (28.02.2014.) (Domagoj Saric)
        set( CMAKE_HOST_SYSTEM_PROCESSOR AMD64 )

        #...mrmlj...this does not actually work because the toolchain file does not
        #...mrmlj...support subsequent compiler changes...
        set( LE_BUILD_ANDROID_COMPILER Clang CACHE STRING "Compiler to use for Android builds." )
        mark_as_advanced( LE_BUILD_ANDROID_COMPILER )
        set_property( CACHE LE_BUILD_ANDROID_COMPILER PROPERTY STRINGS Clang GCC )
        
        set( ANDROID_STL_FORCE_FEATURES OFF CACHE BOOL "" FORCE )
        set( ANDROID_GOLD_LINKER        ON  CACHE BOOL "" FORCE )

        set( LIBRARY_OUTPUT_PATH_ROOT "${CMAKE_CURRENT_BINARY_DIR}" CACHE PATH "required by the Android toolchain file" )

        mark_as_advanced( ANDROID_ABI              )
        mark_as_advanced( ANDROID_NATIVE_API_LEVEL )
        mark_as_advanced( EXECUTABLE_OUTPUT_PATH   )
        mark_as_advanced( LIBRARY_OUTPUT_PATH      )
        mark_as_advanced( LIBRARY_OUTPUT_PATH_ROOT )

        if ( LE_BUILD_SUBPROJECT_BUILD )
                if ( LE_BUILD_ABI MATCHES arm-linux-androideabi )
                set( ANDROID_NATIVE_API_LEVEL 15 ) # ICS
            elseif ( LE_BUILD_ABI STREQUAL x86 )
                set( ANDROID_NATIVE_API_LEVEL 18 ) # JB
            else() # 64bit
                set( ANDROID_NATIVE_API_LEVEL 21 ) # L
            endif()
            set( CMAKE_ANDROID_API_MIN ${ANDROID_NATIVE_API_LEVEL} )
            message( STATUS "[LEB] Targeting Android API level ${ANDROID_NATIVE_API_LEVEL}." )

            unset( CMAKE_C_COMPILER               )
            unset( CMAKE_C_COMPILER         CACHE )
            unset( CMAKE_CXX_COMPILER             )
            unset( CMAKE_CXX_COMPILER       CACHE )
            unset( ANDROID_ABI                    )
            unset( ANDROID_ABI              CACHE )
            unset( ANDROID_NATIVE_API_LEVEL CACHE )

            set( ANDROID_COMPILER_VERSION 4.9 CACHE STRING "" FORCE ) # GCC version
            if ( LE_BUILD_ANDROID_COMPILER MATCHES Clang )
                set( ANDROID_TOOLCHAIN_NAME clang3.6                         )
                set( ANDROID_STL            c++_static CACHE STRING "" FORCE )
            else()
                set( ANDROID_TOOLCHAIN_NAME ${ANDROID_COMPILER_VERSION}                       )
                set( ANDROID_STL            gnustl_static               CACHE STRING "" FORCE )
            endif()

            set( ANDROID_TOOLCHAIN_NAME ${LE_BUILD_ABI}-${ANDROID_TOOLCHAIN_NAME} CACHE STRING "" FORCE )

            unset( releaseBinaries     CACHE ) #...retest/think whether this is necessary here...
            unset( developmentBinaries CACHE )
            # Implementation note:
            #   Because CMAKE_TOOLCHAIN_FILE seems broken when used through
            # execute_process() we work around this by manually including the
            # toolchain file.
            # http://www.cmake.org/Bug/view.php?id=14779
            #                                     (28.02.2014.) (Domagoj Saric)
            set( CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/externals/le/build/android.toolchain.cmake" )
            #...mrmlj...include( CMakeDetermineSystem.cmake )
            include( "${CMAKE_TOOLCHAIN_FILE}" )
        endif( LE_BUILD_SUBPROJECT_BUILD )
    endif( WIN32 ) # Android
endif( CMAKE_TOOLCHAIN_FILE OR CMAKE_CROSSCOMPILING )
