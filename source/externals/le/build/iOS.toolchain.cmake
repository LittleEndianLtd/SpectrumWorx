################################################################################
#
# Little Endian iOS CMake tool chain file.
#
# Copyright (c) 2013 - 2016. Little Endian Ltd. All rights reserved.
#
# Adapted from bits&pieces gathered from various preexisting OSS solutions.
#
# http://www.cmake.org/Wiki/CMake_Cross_Compiling
#
# https://cmake.org/pipermail/cmake-developers/2014-September/023068.html [cmake-developers] iOS support
# https://github.com/Kitware/VTK/blob/master/CMake/ios.toolchain.xcode.cmake
# https://github.com/cristeab/ios-cmake
# https://github.com/plenluno/ios-cmake
# https://code.google.com/archive/p/ios-cmake
# https://llvm.org/svn/llvm-project/llvm/trunk/cmake/platforms/iOS.cmake
#
################################################################################


# Standard settings
set( CMAKE_SYSTEM_NAME    Darwin )
set( CMAKE_SYSTEM_VERSION 6      )
set( CMAKE_SYSTEM_PROCESSOR arm  )
set( UNIX  true )
set( APPLE true )
set( iOS   true )

# Skip the platform compiler checks for cross compiling (or not)...
set( CMAKE_CXX_COMPILER_WORKS false                       )
set( CMAKE_C_COMPILER_WORKS   ${CMAKE_CXX_COMPILER_WORKS} )
if ( NOT CMAKE_CXX_COMPILER_WORKS )
    # Make sure all executables are bundles otherwise try compiles will fail.
    set( CMAKE_MACOSX_BUNDLE                      true )
    set( CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer" CACHE STRING "how to sign executables" )
    set( MACOSX_BUNDLE_GUI_IDENTIFIER             "com.le.cmake-try-compile" )
endif()


# http://code.google.com/p/ios-cmake/source/browse/toolchain/iOS.cmake
# http://stackoverflow.com/questions/5010062/xcodebuild-simulator-or-device
set( CMAKE_XCODE_EFFECTIVE_PLATFORMS "-universal;-iphonesimulator;-iphoneos;" )
set( CMAKE_IOS_DEVELOPER_ROOT "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer" )

set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY libc++ )

set( iOSArchitectures "$(ARCHS_STANDARD)" ) # http://www.cocoanetics.com/2014/10/xcode-6-drops-armv7s

set( CMAKE_OSX_ARCHITECTURES                "${iOSArchitectures}" )
set( CMAKE_XCODE_ATTRIBUTE_ARCHS            "${iOSArchitectures}" )
set( CMAKE_XCODE_ATTRIBUTE_VALID_ARCHS      "${iOSArchitectures}" )
set( CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH NO                    )
set( CMAKE_XCODE_ATTRIBUTE_SDKROOT          iphoneos              ) # iphoneos "Latest iOS"


# set_xcode_property( TARGET XCODE_PROPERTY XCODE_VALUE )
#  A convenience macro for setting xcode specific properties on targets
#  example: set_xcode_property (myioslib IPHONEOS_DEPLOYMENT_TARGET "3.1")
macro( set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE )
	set_property( TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY} ${XCODE_VALUE} )
endmacro( set_xcode_property )


# find_host_package( PROGRAM ARGS )
#  A macro used to find executable programs on the host system, not within the
# iOS environment.
macro( find_host_package )
	set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
	set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER )
	set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER )
	set( iOS false )

	find_package(${ARGN})

	set( iOS true )
	set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY )
	set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
	set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
endmacro( find_host_package )
