################################################################################
#
# Compiler and linker options and utility functions shared across LE projects.
#
# Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
#
################################################################################

cmake_minimum_required( VERSION 3.1 )

include( "${CMAKE_CURRENT_LIST_DIR}/3rdPartyLibs.cmake" )
include( "${CMAKE_CURRENT_LIST_DIR}/utilities.cmake"    )


# Implementation note:
# iOS.toolchain.cmake might set this to prevent failures in CMake's compiler
# detection. We have to unset this to prevent all our targets being created as
# 'bundles'.
#                                             (15.03.2016.) (Domagoj Saric)
unset( CMAKE_MACOSX_BUNDLE )


################################################################################
################################################################################
#
# 3rd party libraries and tools version setup
#
################################################################################
################################################################################

# Boost
# http://boost.org
set( boostVersion     1.61.0 )
set( boostPackageHash bb1dad35ad069e8d7c8516209a51053c )
string( REPLACE "." "_" boostDirectoryName "boost_${boostVersion}" )
set( boostPackageName ${boostDirectoryName}.7z )
set( Boost_ADDITIONAL_VERSIONS ${Boost_ADDITIONAL_VERSIONS} "${boostVersion}" )
add3rdPartyLib( Boost
    ${boostVersion}
    ${boostVersion}
    "http://sourceforge.net/projects/boost/files/boost/${boostVersion}/${boostPackageName}/download"
    ${boostPackageName}
    ${boostPackageHash}
    ""
)

set( rapidXMLVersion 1.13 )

#...mrmlj...Android NDK setup in crossCompilingHelper.cmake

################################################################################



################################################################################
################################################################################
#
# Utility functions
#
################################################################################
################################################################################


################################################################################
#   appendProperty()
################################################################################
#http://www.kwwidgets.org/Bug/view.php?id=12342&nbn=2

function( appendProperty target property )
    # http://public.kitware.com/Bug/view.php?id=12342 Using set_property(...APPEND) for COMPILE_FLAGS result in semicolons in the build command line
    # http://www.cmake.org/pipermail/cmake/2010-November/040807.html set_property(SOURCE APPEND PROPERTY COMPILE_FLAGS)	generates list output
    list( LENGTH ARGN numberOfProperties )
    if ( NOT numberOfProperties )
        return()
    endif()
    set( additionalProperties ${ARGN} )
    if( property MATCHES FLAGS )
        if ( property STREQUAL COMPILE_FLAGS_RELEASE )
            separate_arguments( additionalProperties )
            #...mrmlj...mllvm options require special handling because
            #...mrmlj...target_compile_options() 'eats' all the -mllvm prefixes
            #...mrmlj...after the first one...
            #target_compile_options( ${target} PRIVATE $<$<CONFIG:Release>:${additionalProperties}> )
            string( REPLACE "-mllvm;" "-mllvm+" additionalProperties "${additionalProperties}" )
            foreach( flag ${additionalProperties} )
                if ( flag MATCHES -mllvm )
                    string( REPLACE "-mllvm+" "-mllvm " flag "${flag}" )
                    set( flag " ${flag}" ) #...mrmlj...!? see the below mrmlj for /arch:SSE
                    set_property( TARGET ${target} APPEND_STRING PROPERTY COMPILE_FLAGS ${flag} )
                else()
                    target_compile_options( ${target} PRIVATE $<$<CONFIG:Release>:${flag}> )
                endif()
            endforeach()
            return()
        endif()
        set( method APPEND_STRING )
        #...mrmlj...the explicit space before additionalProperties is a workaround for /arch:SSE getting appended without a space to the /Yu PCH parameter !?
        string( REPLACE ";" " " additionalProperties " ${additionalProperties}" )
    else()
        set( method APPEND )
    endif()
    set_property( TARGET ${target} ${method} PROPERTY ${property} ${additionalProperties} )
endfunction()


################################################################################
#   appendSourceFileFlags()
################################################################################

function( appendSourceFileFlags file flagType additionalFlags )
    string( REPLACE ";" " " additionalFlags "${additionalFlags} ${ARGN}" )
    set_property( SOURCE "${file}" APPEND_STRING PROPERTY ${flagType} ${additionalFlags} )
endfunction()


################################################################################
#   appendLinkDirectory()
################################################################################

function( appendLinkDirectory target targetFlags directory )
    appendProperty( ${target} LINK_FLAGS_${targetFlags} "${addLibpathSwitch}${directory}" )
endfunction()


################################################################################
#   addPCH()
################################################################################
# https://github.com/sakra/cotire
# http://gamesfromwithin.com/the-care-and-feeding-of-pre-compiled-headers
# http://stackoverflow.com/questions/148570/using-pre-compiled-headers-with-cmake
# http://cmake.3232098.n2.nabble.com/Adding-pre-compiled-header-support-easily-add-generated-source-td4589986.html

function( addPCH target pchFile )
    if ( MSVC )
        appendProperty( "${target}" COMPILE_FLAGS "/FI\"${pchFile}.hpp\"" "/Yu\"${pchFile}.hpp\"" )
        appendSourceFileFlags( "${pchFile}.cpp" COMPILE_FLAGS "/Yc\"${pchFile}.hpp\"" )
    elseif ( APPLE )
        set_property( TARGET ${target} PROPERTY XCODE_ATTRIBUTE_GCC_PREFIX_HEADER            "${pchFile}.hpp" )
        set_property( TARGET ${target} PROPERTY XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES"            )
    elseif( CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES Clang )
        # http://clang.llvm.org/docs/UsersManual.html#precompiled-headers
        # http://clang.llvm.org/docs/PCHInternals.html#using-precompiled-headers-with-clang
        set_source_files_properties( ${pchFile}.cpp PROPERTIES HEADER_FILE_ONLY true  )
        set_source_files_properties( ${pchFile}.hpp PROPERTIES HEADER_FILE_ONLY false )
        appendSourceFileFlags( "${pchFile}.hpp" COMPILE_FLAGS -x c++-header -o "${PROJECT_SOURCE_DIR}/${pchFile}.hpp.pch" )
        #appendProperty( "${target}" COMPILE_FLAGS -include-pch \"${PROJECT_SOURCE_DIR}/${pchFile}.hpp.pch\" )
        #get_target_property( flags ${target} COMPILE_FLAGS )
        #add_custom_target( ${target}_PCH
        #    COMMENT           "Generating precompiled headers"
        #    COMMAND           "${CMAKE_CXX_COMPILER}" ${flags} -x c++-header "${pchFile}.hpp" -o "${pchFile}.hpp.pch"
        #    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        #    SOURCES           "${pchFile}.hpp"
        #)
        #add_dependencies( ${target} ${target}_PCH )
        #target_compile_options( ${target} PRIVATE "-include \"${pchFile}.hpp\"" )
    else()
        message( FATAL_ERROR "Toolchain does not support PCH" )
    endif()
endfunction()


################################################################################
#   addForceInclude()
################################################################################

function( addForceInclude header )
    if ( MSVC )
        set( compilerOption "/FI\"${header}\"" )
    elseif ( CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES Clang )
        set( compilerOption "-include \"${header}\"" )
    endif()
    add_compile_options( ${compilerOption} )
endfunction()


################################################################################
#   addPrefixHeader()
################################################################################

function( addPrefixHeader target prefixHeaderFile )
    if ( MSVC )
        appendProperty( ${target} COMPILE_FLAGS "/FI\"${prefixHeaderFile}\"" )
    elseif ( APPLE )
        set_property( TARGET ${target} PROPERTY XCODE_ATTRIBUTE_GCC_PREFIX_HEADER            "${prefixHeaderFile}" )
        set_property( TARGET ${target} PROPERTY XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "NO"                  )
    else()
        #addForceInclude( "${prefixHeaderFile}" ) #...mrmlj...
        appendProperty( ${target} COMPILE_FLAGS "-include \"${prefixHeaderFile}\"" )
    endif()
endfunction()


################################################################################
#   Strict aliasing and fastmath
################################################################################

if ( MSVC )
    set( strictAliasing NO   )
    set( fastMath       full )
elseif ( LEB_NOT_USING_NT2 OR NOT ( ANDROID OR iOS ) )
    set( strictAliasing YES  )
    set( fastMath       full )
else()
    # Implementation note:
    # The FFT and/or parts of NT2 miscompile with Clang (3.6) and 'SIMD
    # emulation' (x86 and ARM) if the full fast-math option is enabled.
    # https://llvm.org/svn/llvm-project/cfe/trunk/test/Driver/fast-math.c
    # http://llvm.org/docs/LangRef.html#fast-math-flags
    #                                         (25.08.2011.) (Domagoj Saric)
    set( strictAliasing NO      )
    set( fastMath       partial )
endif()
set( LE_BUILD_STRICT_ALIASING ${strictAliasing} CACHE BOOL   "" )
set( LE_BUILD_FAST_MATH       ${fastMath}       CACHE STRING "" )
set_property( CACHE LE_BUILD_FAST_MATH PROPERTY STRINGS disabled partial full )
mark_as_advanced( LE_BUILD_STRICT_ALIASING )
mark_as_advanced( LE_BUILD_FAST_MATH       )
unset( strictAliasing )
unset( fastMath       )


################################################################################
#   addNT2()
################################################################################

function( addNT2 )
    # Implementation note:
    #   Clang and GCC autodetect SIMD support from the command line options
    # (e.g.-msse3) but MSVC cannot as we only use /arch:SSE). The definitions
    # also have to be added before NT2 is included so that they are seen by the
    # NT2 auxiliary projects (boost_simd and nt2).
    # https://connect.microsoft.com/VisualStudio/feedback/details/805105/msvc-add-other-all-architectures-to-the-arch-compiler-option
    #                                         (25.08.2011.) (Domagoj Saric)
    if ( MSVC )
        add_definitions(
            -DBOOST_SIMD_MEMORY_NO_BUILTINS
            -DBOOST_SIMD_BRANCH_FREE_IF_ELSE
            -DBOOST_SIMD_HAS_SSE_SUPPORT
            -DBOOST_SIMD_HAS_SSE2_SUPPORT
           #-DBOOST_SIMD_HAS_SSE3_SUPPORT
           #-DBOOST_SIMD_HAS_SSSE3_SUPPORT
           #-DBOOST_SIMD_HAS_SSE4_1_SUPPORT
           #-DBOOST_SIMD_HAS_SSE4_2_SUPPORT
        )
        if ( NOT CMAKE_GENERATOR MATCHES 64 )
            add_definitions( -DBOOST_SIMD_HAS_MMX_SUPPORT )
        endif()
    endif( MSVC )
    add_definitions(
        -DLE_HAS_NT2
        -DBOOST_SIMD_NO_DENORMALS
        -DBOOST_SIMD_NO_INFINITIES
        -DBOOST_SIMD_NO_NANS
        -DBOOST_SIMD_NO_MINUSZERO
        -DNT2_DISABLE_ERROR
    )
    if ( NOT LE_BUILD_STRICT_ALIASING )
        add_definitions( -DBOOST_SIMD_NO_STRICT_ALIASING )
    endif()

    set( NT2_SOURCE_ROOT "${PROJECT_SOURCE_DIR}/externals/nt2" )

    #...mrmlj...for hardcoded set(Boost_ADDITIONAL_VERSIONS "1.48") in nt2.boost.cmake causes failure to find boost when the official findboost.cmake does not include the currently installed versions...
    set( Boost_FOUND true )

    #...mrmlj...warning in nt2.simd.cmake...
    # http://www.cmake.org/cmake/help/v3.1/policy/CMP0054.html
    cmake_policy( SET CMP0054 OLD )

    # Implementation note:
    #   Skip the building of the is_supported tool because we don't need SIMD
    # level support information at CMake-time and the tool created problems on
    # OS X.
    #                                         (17.02.2012.) (Domagoj Saric)
    set( NT2_SIMD_FLAGS "" CACHE STRING "" )
    list( APPEND CMAKE_MODULE_PATH "${NT2_SOURCE_ROOT}/cmake" )
    # Implementation note:
    # Skip the slow NT2 inclusion process if we can reuse the NT2_INCLUDE_DIR
    # from a super-project.
    #                                         (25.09.2015.) (Domagoj Saric)
    if ( NOT NT2_INCLUDE_DIR )
        find_package( NT2 COMPONENTS boost.simd core.base core.exponential core.signal core.trigonometric )

        forwardToParentScope( NT2_INCLUDE_DIR )

        mark_as_advanced( NT2_SIMD_EXT    )
        mark_as_advanced( NT2_SIMD_FLAGS  )
        mark_as_advanced( Boost_DIR       )
        mark_as_advanced( WAVE_EXECUTABLE )
    endif()

    include_directories( ${NT2_INCLUDE_DIR} )
endfunction()


################################################################################
#   setupTargetForPlatform()
################################################################################

function( setupTargetForPlatform projectName architecture )
    if ( ANDROID )
        appendProperty( ${projectName} COMPILE_FLAGS ${ANDROID_CXX_FLAGS} )
        if ( X86 OR X86_64 )
            appendProperty( ${projectName} COMPILE_FLAGS ${x86CompilerSwitches} )
            appendProperty( ${projectName} LINK_FLAGS   "${x86CompilerSwitches} ${x86LinkerSwitches}" ) #...mrmlj...need to pass compiler options to the 'linker' for LTO builds...
        else()
            appendProperty( ${projectName} COMPILE_FLAGS ${armCompilerSwitches} )
            appendProperty( ${projectName} LINK_FLAGS   "${armCompilerSwitches} ${armLinkerSwitches}" )
        endif()
        if ( CMAKE_CXX_COMPILER_ID MATCHES Clang )
            appendProperty( ${projectName} LINK_FLAGS -Qunused-arguments ) #...mrmlj...silence the 'unused command line argument' warning for -mllvm options...
        endif()
        set_property( TARGET ${projectName} PROPERTY ARCHIVE_OUTPUT_DIRECTORY libs/${ANDROID_NDK_ABI_NAME} )
        target_include_directories( ${projectName} SYSTEM PRIVATE ${ANDROID_INCLUDE_DIRECTORIES} )
       #target_link_directories   ( ${projectName} ${ANDROID_LINK_DIRECTORIES} )
        foreach( libdir ${ANDROID_LINK_DIRECTORIES} )
            appendProperty( ${projectName} LINK_FLAGS -L${libdir} )
        endforeach()
    elseif ( APPLE )
        # http://www.cmake.org/pipermail/cmake/2011-October/046737.html
        # http://stackoverflow.com/questions/1211854/xcode-conditional-build-settings-based-on-architecture-device-arm-vs-simulat
        # http://www.macresearch.org/how_to_properly_use_sse3_and_ssse3_and_future_intel_vector_extensions_0
        #...mrmlj...the PER_ARCH approach does not work...find out where this was seen and/or remove completely...
        # set_property( TARGET ${projectName}
            # PROPERTY XCODE_ATTRIBUTE_PER_ARCH_CFLAGS_i386   "${XCODE_ATTRIBUTE_CFLAGS_i386}"
            # PROPERTY XCODE_ATTRIBUTE_PER_ARCH_CFLAGS_x86_64 "${XCODE_ATTRIBUTE_CFLAGS_x86_64}"
            # PROPERTY XCODE_ATTRIBUTE_PER_ARCH_CFLAGS_armv7  "${XCODE_ATTRIBUTE_PER_ARCH_CFLAGS_armv7}"
            # PROPERTY XCODE_ATTRIBUTE_PER_ARCH_CFLAGS_armv7s "${XCODE_ATTRIBUTE_PER_ARCH_CFLAGS_armv7s}"
        # )

        if ( iOS )
            get_target_property( isAnApp ${projectName} MACOSX_BUNDLE )
            if ( NOT isAnApp )
                # Generate a 'universal'/'fat' (i.e. simulator+device) build:
                # http://red-glasses.com/index.php/tutorials/xcode4-make-a-library-in-one-file-that-works-on-both-device-and-simulator
                # http://atastypixel.com/blog/an-xcode-4-template-to-create-universal-static-libraries
                # http://stackoverflow.com/questions/3520977/build-fat-static-library-device-simulator-using-xcode-and-sdk-4
                # https://gist.github.com/3705459
                # https://github.com/michaeltyson/iOS-Universal-Library-Template
                # https://github.com/kstenerud/iOS-Universal-Framework
                add_custom_command(
                    TARGET ${projectName}
                    POST_BUILD
                    # Implementation note:
                    #   We have to override the TARGET_NAME variable (generated
                    # by Xcode and used by the iOSUniversalBuild.sh script) as
                    # well as the script's working directory when this function
                    # is used on a subproject (e.g. AudioIO from the
                    # SpectrumWorx SDK).
                    #                         (04.12.2012.) (Domagoj Saric)
                    COMMAND export TARGET_NAME=${projectName} && "${PROJECT_SOURCE_DIR}/externals/le/build/iOSUniversalBuild.sh"
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    VERBATIM
                )
            endif()
            appendProperty( ${projectName} COMPILE_FLAGS "${armCompilerSwitches}" )

            # http://cmake.3232098.n2.nabble.com/Different-settings-for-different-configurations-in-Xcode-td6908021.html
            # http://www.cmake.org/pipermail/cmake/2011-October/046756.html Separating SET_TARGET_PROPERTIES() for different configurations
            # http://public.kitware.com/Bug/view.php?id=8915 Missing feature to set Xcode specific build settings
            set_property( TARGET ${projectName} PROPERTY XCODE_ATTRIBUTE_OTHER_CFLAGS[arch=armv7]          "${XCODE_ATTRIBUTE_CFLAGS_armv7}  $(OTHER_CFLAGS)"         )
            set_property( TARGET ${projectName} PROPERTY XCODE_ATTRIBUTE_OTHER_CFLAGS[arch=armv7s]         "${XCODE_ATTRIBUTE_CFLAGS_armv7s} $(OTHER_CFLAGS)"         )
            set_property( TARGET ${projectName} PROPERTY XCODE_ATTRIBUTE_OTHER_CPLUSPLUSFLAGS[arch=armv7]  "${XCODE_ATTRIBUTE_CFLAGS_armv7}  $(OTHER_CPLUSPLUSFLAGS)" )
            set_property( TARGET ${projectName} PROPERTY XCODE_ATTRIBUTE_OTHER_CPLUSPLUSFLAGS[arch=armv7s] "${XCODE_ATTRIBUTE_CFLAGS_armv7s} $(OTHER_CPLUSPLUSFLAGS)" )
            set_property( TARGET ${projectName} PROPERTY XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET        "${architecture}"                                          )

            # Implementation note:
            #   It is necessary to reset this to Xcode defaults (from what CMake
            # sets it to) in order for the iOSUniversalBuild script to work.
            #                                 (14.06.2011.) (Domagoj Saric)
            set_target_properties( ${projectName} PROPERTIES
                XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR
                "$(BUILD_DIR)/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)"
            )
        else() # OSX
            appendProperty( ${projectName} COMPILE_FLAGS ${x86CompilerSwitches} )

            # http://www.cmake.org/cmake/help/v3.0/prop_tgt/MACOSX_RPATH.html
            set_property( TARGET ${projectName} PROPERTY MACOSX_RPATH false )
            set_property( TARGET ${projectName} PROPERTY XCODE_ATTRIBUTE_OTHER_CFLAGS[arch=i386]           "${XCODE_ATTRIBUTE_CFLAGS_i386}   $(OTHER_CFLAGS)"         )
            set_property( TARGET ${projectName} PROPERTY XCODE_ATTRIBUTE_OTHER_CFLAGS[arch=x86_64]         "${XCODE_ATTRIBUTE_CFLAGS_x86_64} $(OTHER_CFLAGS)"         )
            #...mrmlj...kill the duplication...
            set_property( TARGET ${projectName} PROPERTY XCODE_ATTRIBUTE_OTHER_CPLUSPLUSFLAGS[arch=i386]   "${XCODE_ATTRIBUTE_CFLAGS_i386}   $(OTHER_CPLUSPLUSFLAGS)" )
            set_property( TARGET ${projectName} PROPERTY XCODE_ATTRIBUTE_OTHER_CPLUSPLUSFLAGS[arch=x86_64] "${XCODE_ATTRIBUTE_CFLAGS_x86_64} $(OTHER_CPLUSPLUSFLAGS)" )

            if ( LE_SDK_BUILD )
                set_property( TARGET ${projectName} PROPERTY XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY libc++ )
            else()
                # Implementation note:
                #   It seems that a lot of SW users are still using 64 bit SL...
                #                             (30.09.2013.) (Domagoj Saric)
                #...mrmlj...however current code does not compile with the old libstdc++ from SL so libc++/10.7 has to be used...
                #set_property( TARGET ${projectName} PROPERTY XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET[arch=x86_64] 10.7   )
                #set_property( TARGET ${projectName} PROPERTY XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY[arch=x86_64]        libc++ )
                set_property( TARGET ${projectName} PROPERTY XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET 10.7   )
                set_property( TARGET ${projectName} PROPERTY XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY        libc++ )
            endif()
        endif()
    elseif ( MSVC )
        appendProperty( "${projectName}" COMPILE_FLAGS ${x86CompilerSwitches} )
        # C4530: C++ exception handler used, but unwind semantics are not enabled.
        appendProperty( "${projectName}" COMPILE_FLAGS /wd4530 )
        #...mrmlj...quick fix for this dubious warning in NT2 FFT code that only
        #...mrmlj...seems to appear in unity builds and is impossible to silence
        #...mrmlj...with pragmas...
        appendProperty( "${projectName}" COMPILE_FLAGS /wd4512 )
        appendProperty( "${projectName}" LINK_FLAGS    ${x86LinkerSwitches} )
    endif()
endfunction( setupTargetForPlatform )

################################################################################




################################################################################
################################################################################
#
# Global build variables and options setup
#
################################################################################
################################################################################
#...mrmlj...there is still a fair amount of spaghetti here...
#...mrmlj...- global/fixed/static option setting should be separated from
#...mrmlj...  dynamic/per-project-setting (e.g. in mutli-arch builds)
#...mrmlj...- os options selection should be decoupled from compiler options
#...mrmlj...  selection
#...mrmlj...- individual os and compiler logic should be extracted into separate
#...mrmlj.... dedicated files
################################################################################


################################################################################
# Define the build type or valid configuration types for IDE generators
# http://mail.kde.org/pipermail/kde-buildsystem/2008-November/005112.html
################################################################################

if ( DEFINED CMAKE_CONFIGURATION_TYPES )
    # We currently need only the following configuration types:
    set( CMAKE_CONFIGURATION_TYPES Debug Release CACHE STRING "Supported configuartion types" FORCE )
elseif ( NOT CMAKE_BUILD_TYPE )
    set( CMAKE_BUILD_TYPE Release CACHE STRING "Target configuration" FORCE )
endif()


################################################################################
#
# OS and architecture specific variables and utility functions
#
################################################################################

################################################################################
# List of CPU architectures and OS suffix strings
################################################################################
if ( ANDROID )
    set( LEB_ABIs
        aarch64-linux-android
        arm-linux-androideabi
        x86_64
        x86
    )
    if ( LE_BUILD_ABI STREQUAL aarch64-linux-android )
        set( LEB_ARCHITECTURES "arm64-v8a,ARMv8a_64" CACHE INTERNAL "" FORCE )
    elseif ( LE_BUILD_ABI STREQUAL arm-linux-androideabi )
        set( LEB_ARCHITECTURES
           #"armeabi,ARMv5TE_soft-float" ... obsolete
           #"armeabi-v6 with VFP,ARMv6_VFP2" ... obsolete
            "armeabi-v7a,ARMv7a_VFP3-D16"
           #"armeabi-v7a with VFPV3,ARMv7a_VFP3-D32" ... rare
            "armeabi-v7a with NEON,ARMv7a_NEON"
            CACHE INTERNAL "" FORCE
        )
    elseif ( LE_BUILD_ABI STREQUAL x86_64 )
        set( LEB_ARCHITECTURES "x86_64,x86-64_SSE4.2" CACHE INTERNAL "" FORCE )
    elseif ( LE_BUILD_ABI STREQUAL x86 )
        # Implementation note:
        # This piece of official documentation
        # http://developer.android.com/ndk/guides/cpu-features.html seems to
        # imply that the x86 ABI guarantees only SSE3 (and there seem to be a
        # number of people that try to run Android on older hardware). However,
        # the first official Android x86 devices are Atom based which supports
        # SSSE3 and the release notes for NDK r10 explicitly mention upgrading
        # the minimal ABI to SSSE3 (and Google seems to have 'silently' made
        # SSSE3 a requirement for ICS).
        # http://developer.android.com/ndk/downloads/revision_history.html @ r10
        # https://groups.google.com/forum/#!topic/android-x86/NH_Ny1hdpPg
        # https://groups.google.com/forum/#!topic/android-x86/KXiCtW4sZg8
        # http://ftp.software-sources.co.il/Developing%20Android%20Applications%20for%20Intel%20Atom%20Based%20Platforms-final.pdf
        #                                     (28.02.2014.) (Domagoj Saric)
        set( LEB_ARCHITECTURES "x86,x86-32_SSSE3" CACHE INTERNAL "" FORCE )
    endif()
    set( LEB_OS_SUFFIX Android CACHE INTERNAL "" )
elseif( iOS )
    # Implementation note:
    #   Using target names with parentheses (e.g. with a
    # "5_(simulator+armv7+armv7s)" suffix) isn't yet possible because of
    # the following bug http://public.kitware.com/Bug/view.php?id=14521.
    #                                        (29.10.2013.) (Domagoj Saric)
    set( LEB_ARCHITECTURES
        "5.1.1,"
        CACHE INTERNAL "" FORCE
    )
    set( LEB_OS_SUFFIX iOS CACHE INTERNAL "" FORCE )
elseif( APPLE )
    #...mrmlj...perhaps we can default to sse4.1 for 64bit builds...investigate...
    # http://www.cnet.com/news/older-64-bit-macs-out-of-the-picture-for-mountain-lion
    # http://www.everymac.com/mac-answers/snow-leopard-mac-os-x-faq/mac-os-x-snow-leopard-64-bit-macs-64-bit-efi-boot-in-64-bit-mode.html
    # http://en.wikipedia.org/wiki/List_of_Macintosh_models_grouped_by_CPU_type
    set( LEB_ARCHITECTURES
        "sse3,x86-32_SSE3+x86-64_SSSE3"
        "sse4.1,x86-32_SSE4.1+x86-64_SSE4.1"
        CACHE INTERNAL "" FORCE
    )
    set( LEB_OS_SUFFIX OSX CACHE INTERNAL "" FORCE )
elseif ( WIN32 )
    #...mrmlj...ABI abstraction to be rethought & finished...see createSDKProjectAux() in sdkProject.cmake...
    set( LEB_ABIs
        Win32
        Win64
    )
    # Implementation note:
    # MSVC has no options of its own to enable SSE3/4 codegen and so it makes no
    # sense to generate such builds if NT2 is not used (which can be made to use
    # SSE3+ regardless of compiler options).
    #                                         (10.03.2016.) (Domagoj Saric)
    if ( CMAKE_GENERATOR MATCHES 64 )
        if ( LEB_NOT_USING_NT2 )
            set( LEB_ARCHITECTURES "sse2,x86-64_SSE2" CACHE INTERNAL "" )
        else()
            set( LEB_ARCHITECTURES
                "sse3,x86-64_SSE3"
                "sse4.1,x86-64_SSE4.1"
                CACHE INTERNAL ""
            )
        endif()
        set( LEB_OS_SUFFIX Win64 CACHE INTERNAL "" )
    else()
        if ( LEB_NOT_USING_NT2 )
            set( LEB_ARCHITECTURES "sse2,x86-32_SSE2" CACHE INTERNAL "" )
        else()
            set( LEB_ARCHITECTURES
                "sse2,x86-32_SSE2"
                "sse4.1,x86-32_SSE4.1"
                CACHE INTERNAL ""
            )
        endif()
        set( LEB_OS_SUFFIX Win32 CACHE INTERNAL "" )
    endif()
endif() # OS


################################################################################
# LE_parseArchitectureString()
################################################################################

function( LE_parseArchitectureString architectureString nameVariable suffixVariable )
    string( REPLACE "," ";" architecture "${architectureString}" )
    list( GET architecture 0 name   )
    list( GET architecture 1 suffix )
    set( ${nameVariable}   ${name}   PARENT_SCOPE )
    set( ${suffixVariable} ${suffix} PARENT_SCOPE )
endfunction()


################################################################################
# Create a combo-box for the LE_TARGET_ARCHITECTURE variable
################################################################################
#...mrmlj...partial duplication with sdkProject.cmake
if ( NOT LE_BUILD_BATCH_BUILD ) #...mrmlj...
    if ( NOT LE_TARGET_ARCHITECTURE OR NOT LEB_ARCHITECTURES MATCHES "${LE_TARGET_ARCHITECTURE}" )
        foreach( architecture ${LEB_ARCHITECTURES} )
            LE_parseArchitectureString( ${architecture} name suffix )
            list( APPEND architectureList       ${name}   )
            list( APPEND architectureSuffixList ${suffix} )
        endforeach()
        # Define defaults:
        list( GET architectureList       0 targetArchitecture       )
        list( GET architectureSuffixList 0 targetArchitectureSuffix )
        set( LE_TARGET_ARCHITECTURE        ${targetArchitecture}       CACHE STRING   "CPU architecture to build for." FORCE )
        set( LE_TARGET_ARCHITECTURE_SUFFIX ${targetArchitectureSuffix} CACHE INTERNAL ""                               FORCE )
        mark_as_advanced( LE_TARGET_ARCHITECTURE )
        set_property( CACHE LE_TARGET_ARCHITECTURE PROPERTY STRINGS ${architectureList} )
    endif()
endif()


################################################################################
# Define the build type or valid configuration types for IDE generators
################################################################################

# Implementation note:
#   Clear any default CMake compiler flags.
#                                             (30.01.2014.) (Domagoj Saric)
function( LE_clearDefaultFlags )
    unset( CMAKE_C_FLAGS           )
    unset( CMAKE_C_FLAGS_DEBUG     )
    unset( CMAKE_C_FLAGS_RELEASE   )
    unset( CMAKE_CXX_FLAGS         )
    unset( CMAKE_CXX_FLAGS_DEBUG   )
    unset( CMAKE_CXX_FLAGS_RELEASE )

    unset( CMAKE_C_FLAGS           CACHE )
    unset( CMAKE_C_FLAGS_DEBUG     CACHE )
    unset( CMAKE_C_FLAGS_RELEASE   CACHE )
    unset( CMAKE_CXX_FLAGS         CACHE )
    unset( CMAKE_CXX_FLAGS_DEBUG   CACHE )
    unset( CMAKE_CXX_FLAGS_RELEASE CACHE )

    set( CMAKE_EXE_LINKER_FLAGS            "" CACHE STRING "" FORCE )
    set( CMAKE_EXE_LINKER_FLAGS_DEBUG      "" CACHE STRING "" FORCE )
    set( CMAKE_EXE_LINKER_FLAGS_RELEASE    "" CACHE STRING "" FORCE )
    set( CMAKE_MODULE_LINKER_FLAGS         "" CACHE STRING "" FORCE )
    set( CMAKE_MODULE_LINKER_FLAGS_DEBUG   "" CACHE STRING "" FORCE )
    set( CMAKE_MODULE_LINKER_FLAGS_RELEASE "" CACHE STRING "" FORCE )
    set( CMAKE_SHARED_LINKER_FLAGS         "" CACHE STRING "" FORCE )
    set( CMAKE_SHARED_LINKER_FLAGS_DEBUG   "" CACHE STRING "" FORCE )
    set( CMAKE_SHARED_LINKER_FLAGS_RELEASE "" CACHE STRING "" FORCE )
    set( CMAKE_STATIC_LINKER_FLAGS         "" CACHE STRING "" FORCE )
    set( CMAKE_STATIC_LINKER_FLAGS_DEBUG   "" CACHE STRING "" FORCE )
    set( CMAKE_STATIC_LINKER_FLAGS_RELEASE "" CACHE STRING "" FORCE )
endfunction()


################################################################################
#
# Compiler specific configuration
#
################################################################################

if ( MSVC )
    if ( NOT WIN32 )
        message( SEND_ERROR "LEB: unexpected configuration - MSVC detected but not Win32." )
    endif()

    set( addLibpathSwitch           /LIBPATH:            )
    set( ltoCompilerSwitch          /GL                  )
    set( ltoLinkerSwitch            /LTCG                ) #...mrmlj...:STATUS does not work for the librarian/static libs...
    set( sizeOptimizationSwitch     /Os                  )
    set( speedOptimizationSwitch    /Ot                  )
    set( rttiOnSwitch               /GR                  )
    set( rttiOffSwitch              /GR-                 )
    set( exceptionsOnSwitch         /EHsc                )
    set( exceptionsOffSwitch        ""                   )
    set( debugSymbolsCompilerSwitch /Zi                  )
    set( debugSymbolsLinkerSwitch   /DEBUG               )
    set( unicodeOnSwitch            /DUNICODE /D_UNICODE )
    # Implementation note:
    #   Explicitly defining _SBCS as a workaround for CMake defining _MBCS by
    # default otherwise.
    #                                         (15.07.2013.) (Domagoj Saric)
    set( unicodeOffSwitch           /D_SBCS              )
    # https://connect.microsoft.com/VisualStudio/feedback/details/804810/msvc-add-explicit-compiler-option-to-completely-disable-the-autovectorizer
    set( vectorizeOnSwitch  "/Qvec-report:1" )
    set( vectorizeOffSwitch "/d2Qvec-"       )

    unset( x86CompilerSwitches )
    unset( x86LinkerSwitches   )
    if ( CMAKE_GENERATOR MATCHES 64 )
        set( x86LinkerSwitches "/MACHINE:X64" )
        if ( LE_TARGET_ARCHITECTURE STREQUAL sse3 )
            set( x86CompilerSwitches -DBOOST_SIMD_HAS_SSE3_SUPPORT )
        elseif ( LE_TARGET_ARCHITECTURE STREQUAL sse4.1 )
            set( x86CompilerSwitches "-DBOOST_SIMD_HAS_SSE3_SUPPORT -DBOOST_SIMD_HAS_SSSE3_SUPPORT -DBOOST_SIMD_HAS_SSE4_1_SUPPORT" )
        elseif( NOT LE_TARGET_ARCHITECTURE STREQUAL sse2 )
            if ( DEFINED LE_TARGET_ARCHITECTURE ) #...mrmlj...when buildOptions has to be included before the architecture is set...cleanup...
                message( FATAL_ERROR "Unknown Win64 architecture (${LE_TARGET_ARCHITECTURE})" )
            endif()
        endif()
    else()
        set( x86LinkerSwitches "/MACHINE:X86" )
        if ( LE_TARGET_ARCHITECTURE STREQUAL sse2 )
            if ( LE_SDK_BUILD )
                set( x86CompilerSwitches "/arch:SSE2" )
            else()
                # Implementation note:
                # For our own public binaries (SW plugin) we specify only SSE1
                # because of inherent code-size problems with MSVC and static
                # linking.
                #                             (28.07.2014.) (Domagoj Saric)
                set( x86CompilerSwitches "/arch:SSE" )
            endif()
        elseif ( LE_TARGET_ARCHITECTURE STREQUAL sse4.1 )
            set( x86CompilerSwitches "/arch:SSE2 -DBOOST_SIMD_HAS_SSE3_SUPPORT -DBOOST_SIMD_HAS_SSSE3_SUPPORT -DBOOST_SIMD_HAS_SSE4_1_SUPPORT" )
        elseif( DEFINED LE_TARGET_ARCHITECTURE ) #...mrmlj...when buildOptions has to be included before the architecture is set...cleanup...
            message( FATAL_ERROR "Unknown Win32 architecture (${LE_TARGET_ARCHITECTURE})" )
        endif()
    endif()
    # http://msdn.microsoft.com/en-us/library/ms235330.aspx
    set( LEB_sharedLinkerSwitches "/DYNAMICBASE:NO /INCREMENTAL:NO /LARGEADDRESSAWARE:NO /MANIFESTUAC:NO" ) #noenv.obj works only with static linking # MSVC12 chokes on nochkclr.obj, noarg.obj DoxyHelper requires argv/argc...

    add_definitions(
        -D_WIN32_WINNT=_WIN32_WINNT_VISTA
        -D_ALLOW_MSC_VER_MISMATCH
    )

    set( LEB_cSpecificFlags   "" CACHE INTERNAL "" FORCE )
    set( LEB_cppSpecificFlags "" CACHE INTERNAL "" FORCE )
    set( LEB_C_FLAGS
        #/BE (EDG frontend)
        #/BE /dE--parse_templates (EDG frontend early instantiation)
        "/MP /Oi /nologo /errorReport:prompt /W4 /WX- /wd4373 /wd4481"
    )

    if ( MSVC_VERSION GREATER 1800 )
        set( LEB_C_FLAGS "${LEB_C_FLAGS} /Zc:threadSafeInit-" )
    endif()

    # Implementation note:
    #   Full debug-mode runtime checks (/GS /RTCsuc) make the plugin too slow to
    # be usable when NT2 is used. Unfortunately pragmas that selectively turn
    # off those checks don't seem to work with MSVC10:
    # http://connect.microsoft.com/VisualStudio/feedback/details/668672/gs-cannot-be-disabled-with-pragma
    # and since CMake still does not support per-configuration COMPILE_FLAGS
    # properties on sources we have to disable the most ofending checks (GS and
    # RTCc) and enable inlining in order to get bearable debug performance.
    #                                         (25.08.2011.) (Domagoj Saric)
    set( LEB_C_FLAGS_DEBUG          "/MDd /Od /Ob2 /Gm- /GS- /RTCsu /fp:precise /EHa /DDEBUG /D_DEBUG" )
    set( LEB_C_FLAGS_MINSIZEREL     "/MD"  )
    set( LEB_C_FLAGS_RELWITHDEBINFO "/MDd" )
    set( LEB_C_FLAGS_RELEASE        "/MT"  )
    set( LEB_sharedCReleaseFlags
        "/Bt /Ox /Ob2 /Oy /GF /Gm- /GS- /Gy /fp:fast /fp:except- /Qfast_transcendentals /wd4714 /wd4723" CACHE INTERNAL "" FORCE
    )
    # http://blogs.msdn.com/b/vcblog/archive/2013/08/28/ability-to-debug-optimized-code-optimized-debugging.aspx
    # https://randomascii.wordpress.com/2013/09/11/debugging-optimized-codenew-in-visual-studio-2012
    # https://blogs.msdn.microsoft.com/visualstudio/2016/03/03/visual-studio-2015-update-2-rc @ C4883
    set( LEB_sharedCReleaseFlags "${LEB_sharedCReleaseFlags} /Gw /Qpar-" )

    # http://blogs.msdn.com/b/vcblog/archive/2013/10/30/the-visual-c-linker-best-practices-developer-iteration.aspx
    set( LEB_SHARED_LINKER_FLAGS_RELEASE "/OPT:REF /OPT:ICF=3" ) #/VERBOSE:UNUSEDLIBS"

elseif ( CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES Clang ) #...mrmlj...clean up this GCC-Clang overlap

    set( addLibpathSwitch           -L              )
    set( ltoCompilerSwitch          -flto           )
    set( ltoLinkerSwitch            -flto           )
    set( rttiOnSwitch               -frtti          )
    set( rttiOffSwitch              -fno-rtti       )
    set( exceptionsOnSwitch         -fexceptions    )
    set( exceptionsOffSwitch        -fno-exceptions )
    set( debugSymbolsCompilerSwitch -g              )
    set( debugSymbolsLinkerSwitch   -g              )
    if ( CMAKE_CXX_COMPILER_ID MATCHES Clang )
        # http://llvm.org/docs/Vectorizers.html
        # http://llvm.org/devmtg/2012-04-12/Slides/Hal_Finkel.pdf
        # http://polly.llvm.org/example_load_Polly_into_clang.html -mllvm -polly -mllvm -polly-vectorizer=polly
        # http://clang.llvm.org/docs/UsersManual.html#options-to-emit-optimization-reports
        set( vectorizeOnSwitch "-fvectorize -fslp-vectorize -fslp-vectorize-aggressive" )
        # Implementation note:
        # Disable optimiser pass reports for iOS builds as they get run by the
        # iOSUniversalBuild.sh script and then Xcode interprets this optimiser
        # output from the child-build process as errors.
        #                                     (07.10.2015.) (Domagoj Saric)
        if ( NOT iOS )
            set( vectorizeOnSwitch "${vectorizeOnSwitch} -Rpass=loop-.*" ) #-Rpass-missed=loop-vectorize -Rpass-analysis=loop-vectorize -mllvm -bb-vectorize-aligned-only
        endif()
        set( vectorizeOffSwitch "-fno-vectorize" )
    else()
        set( vectorizeOnSwitch  "-ftree-vectorize -ftree-vectorizer-verbose=1 -ftree-slp-vectorize -fvect-cost-model=cheap" )
        set( vectorizeOffSwitch "-fno-tree-vectorize" )
    endif()
    set( speedOptimizationSwitch "-O3" ) #-funroll-loops
    set( sizeOptimizationSwitch  "-Os" )

    if ( CMAKE_CXX_COMPILER_ID MATCHES Clang )
        # Implementation note:
        # appendProperty() (actually CMake/target_compile_options) badly handles
        # multiple mllvm options. As a workaround the unroll-allow-partial
        # option was moved to the LEB_sharedCReleaseFlags variable.
        #                                     (30.05.2014.) (Domagoj Saric)
        # http://reviews.llvm.org/rL200898 Set default of inlinecold-threshold to 225.
        # http://lists.llvm.org/pipermail/llvm-dev/2014-July/074374.html
        #...mrmlj...with clang3.6 and inline-threshold below ~175 the binaries start to grow!?
        #set( speedOptimizationSwitch "${speedOptimizationSwitch} -mllvm -inline-threshold=125 -mllvm -inlinehint-threshold=150 -mllvm -inlinecold-threshold=75" )
        #set( sizeOptimizationSwitch  "${sizeOptimizationSwitch}  -mllvm -inline-threshold=100 -mllvm -inlinehint-threshold=115 -mllvm -inlinecold-threshold=50" )
    else()
        set( speedOptimizationSwitch "${speedOptimizationSwitch} -finline-limit=200 -ftracer" )
        set( sizeOptimizationSwitch  "${sizeOptimizationSwitch}  -finline-functions -falign-functions -fprefetch-loop-arrays -freorder-blocks -finline-limit=100 -finline-small-functions -fpartial-inlining -findirect-inlining -falign-jumps -falign-loops -fpredictive-commoning -fgcse-after-reload -ftree-partial-pre" )
    endif()

    # http://gcc.gnu.org/projects/cxx1y.html
    set( LEB_cSpecificFlags   "-std=gnu11"   CACHE INTERNAL "" FORCE )
    set( LEB_cppSpecificFlags "-std=gnu++1z" CACHE INTERNAL "" FORCE )

    set( LEB_sharedCReleaseFlags "-fmerge-all-constants -fno-stack-protector -fno-math-errno" CACHE INTERNAL "" FORCE ) #-D_FORTIFY_SOURCE=0 -Wsuggest-attribute

    set( LEB_C_FLAGS
        "-fstrict-enums -fvisibility=hidden -fvisibility-inlines-hidden -fno-threadsafe-statics -fno-common -fno-ident -Wreorder -Wall -Wstrict-aliasing -Wno-unused-function -Wno-multichar -Wno-unknown-pragmas -Wno-delete-non-virtual-dtor -Wno-unused-local-typedefs"
    )
    if ( LE_BUILD_STRICT_ALIASING )
        set( LEB_C_FLAGS "${LEB_C_FLAGS} -fstrict-aliasing"    ) #-Wundefined-reinterpret-cast" )
    else()
        set( LEB_C_FLAGS "${LEB_C_FLAGS} -fno-strict-aliasing" ) #-Wno-undefined-reinterpret-cast" )
    endif()

    if ( LE_BUILD_FAST_MATH STREQUAL full )
        set( LEB_C_FLAGS "${LEB_C_FLAGS} -ffast-math -ffp-contract=fast" )
        if ( NOT CMAKE_CXX_COMPILER_ID MATCHES Clang ) # http://lists.llvm.org/pipermail/cfe-commits/Week-of-Mon-20150601/130099.html
            set( LEB_C_FLAGS "${LEB_C_FLAGS} -mrecip" )
        endif()
    elseif( LE_BUILD_FAST_MATH STREQUAL partial )
        if ( CMAKE_CXX_COMPILER_ID MATCHES Clang )
            #-menable-no-infs -menable-no-nans
            set( LEB_C_FLAGS "${LEB_C_FLAGS} -fno-signed-zeros -fno-trapping-math -freciprocal-math -ffinite-math-only -fno-honor-infinities -fno-honor-nans -fno-associative-math" )
        else() # GCC
            set( LEB_C_FLAGS "${LEB_C_FLAGS} -ffast-math -fno-finite-math-only" )
        endif()
    else()    
        set( LEB_C_FLAGS "${LEB_C_FLAGS} -fno-fast-math -ffp-contract=off" )
    endif()

    if ( CMAKE_CXX_COMPILER_ID MATCHES Clang )
        set( LEB_C_FLAGS
            #-Wno-local-type-template-args
            #...mrmlj...-fdelayed-template-parsing still broken...causes linker errors...
            # https://www.mail-archive.com/cfe-commits@cs.uiuc.edu/msg33374.html
            "${LEB_C_FLAGS} -Wheader-guard"
        )
        # http://clang.llvm.org/docs/UsersManual.html#controlling-code-generation
        # http://stackoverflow.com/questions/20678801/clang-mac-os-x-maveric-not-supporting-fsanitize-undefined
        set( LEB_C_FLAGS_DEBUG "${LEB_C_FLAGS_DEBUG} -fsanitize-undefined-trap-on-error" ) # -fsanitize=address-full,integer,thread,memory
        set( LEB_sharedCReleaseFlags "${LEB_sharedCReleaseFlags} -mllvm -unroll-allow-partial" CACHE INTERNAL "" FORCE )
    else() # GCC
        set( LEB_C_FLAGS       "${LEB_C_FLAGS} -fpermissive"                       ) #...mrmlj...History typedef in Loudness...
        set( LEB_C_FLAGS       "${LEB_C_FLAGS} -Wno-return-type"                   ) #...mrmlj...dubious warnings with GCC 4.9 NDKr10b...
        set( LEB_C_FLAGS       "${LEB_C_FLAGS} -Wno-enum-compare"                  )
        set( LEB_C_FLAGS_DEBUG "${LEB_C_FLAGS_DEBUG} -fbounds-check -fstack-check" )
        set( LEB_sharedCReleaseFlags #-fwhole-program...mrmlj...requires explicit visibility=default attributes or does not work at all...
            "${LEB_sharedCReleaseFlags} -freorder-functions -fira-loop-pressure -fivopts -fgraphite -fgraphite-identity -floop-block -floop-flatten -floop-interchange -floop-strip-mine -floop-parallelize-all -ftree-loop-linear -fira-hoist-pressure -free -fdelete-null-pointer-checks -fsched-spec-load -fsched2-use-superblocks -fsched-stalled-insns -fmodulo-sched -fmodulo-sched-allow-regmoves -fsched-pressure -fgcse-sm -fgcse-las -fipa-pta -fweb -frename-registers -fbranch-target-load-optimize2 -fno-enforce-eh-specs -fnothrow-opt -freorder-blocks-and-partition"
            CACHE INTERNAL "" FORCE #x86 -mno-align-stringops -minline-all-stringops -mno-align-long-strings -mtls-direct-seg-refs -momit-leaf-frame-pointer
        )
    endif()

    #...mrmlj...
    if ( ANDROID )
        set( LE_ANDROID_PROFILING false CACHE BOOL "enable profiling on Andorid" )
        mark_as_advanced( LE_ANDROID_PROFILING )
    endif()
    if ( LE_ANDROID_PROFILING )
        set( LEB_sharedCReleaseFlags "${LEB_sharedCReleaseFlags} -ftree-vectorizer-verbose=2 -g -pg" CACHE INTERNAL "" FORCE )
    else()
        set( LEB_sharedCReleaseFlags "${LEB_sharedCReleaseFlags} -fomit-frame-pointer -ffunction-sections -fdata-sections" CACHE INTERNAL "" FORCE )
        if ( CMAKE_COMPILER_IS_GNUCXX )
            #set( LEB_sharedCReleaseFlags "${LEB_sharedCReleaseFlags} -fsection-anchors" CACHE INTERNAL "" FORCE )
        endif()
    endif()

    set( LEB_SHARED_LINKER_FLAGS_DEBUG "-funwind-tables" )
    # OS X (10.8-11) ld does not seem to support --gc-sections
    # OS X (10.8-11) ld warns that -s is obsolete and ignored even though it is
    # seems it isn't (makes a difference)
    set( LEB_SHARED_LINKER_FLAGS_RELEASE "-s" )

    if ( APPLE )

        set( LEB_C_FLAGS
            #...mrmlj...-fobjc-direct-dispatch does not seem to work with Xcode 5.1
            # http://stackoverflow.com/questions/2794434/what-exactly-does-gcc-fobjc-direct-dispatch-option-do
            "${LEB_C_FLAGS} -fconstant-cfstrings -fobjc-call-cxx-cdtors -D__NO_MATH_INLINES"
        )

        #...mrmlj...Clang cries unknown option for -fno-objc-exceptions when not
        #...mrmlj...compiling ObjC sources...
        #set( LEB_sharedCReleaseFlags
        #    "${LEB_sharedCReleaseFlags} -fno-objc-exceptions" CACHE INTERNAL "" FORCE
        #)

        set( LEB_SHARED_LINKER_FLAGS_RELEASE
            "${LEB_SHARED_LINKER_FLAGS_RELEASE} -dead_strip -dead_strip_dylibs"
        )

        set( CMAKE_XCODE_ATTRIBUTE_GCC_C_LANGUAGE_STANDARD   gnu11   CACHE STRING "" FORCE )
        set( CMAKE_XCODE_ATTRIBUTE_GCC_CXX_LANGUAGE_STANDARD GNU++14 CACHE STRING "" FORCE )
        set( CMAKE_XCODE_ATTRIBUTE_GCC_C++_LANGUAGE_STANDARD GNU++14 CACHE STRING "" FORCE )

       #set( CMAKE_XCODE_ATTRIBUTE_GCC_AUTO_VECTORIZATION         YES CACHE STRING "" FORCE )
        set( CMAKE_XCODE_ATTRIBUTE_GCC_ENABLE_SSE3_EXTENSIONS     YES CACHE STRING "" FORCE )
        set( CMAKE_XCODE_ATTRIBUTE_GCC_SYMBOLS_PRIVATE_EXTERN     YES CACHE STRING "" FORCE )
        set( CMAKE_XCODE_ATTRIBUTE_GCC_INLINES_ARE_PRIVATE_EXTERN YES CACHE STRING "" FORCE )
        set( CMAKE_XCODE_ATTRIBUTE_GCC_THREADSAFE_STATICS         NO  CACHE STRING "" FORCE )
        set( CMAKE_XCODE_ATTRIBUTE_GCC_FAST_MATH                  YES CACHE STRING "" FORCE )
        set( CMAKE_XCODE_ATTRIBUTE_GCC_STRICT_ALIASING            ${LE_BUILD_STRICT_ALIASING} CACHE STRING "" FORCE )
        set( CMAKE_XCODE_ATTRIBUTE_STRIP_INSTALLED_PRODUCT        YES CACHE STRING "" FORCE )
        set( CMAKE_XCODE_ATTRIBUTE_DEAD_CODE_STRIPPING            YES CACHE STRING "" FORCE )

        set( CMAKE_XCODE_ATTRIBUTE_PRECOMPS_INCLUDE_HEADERS_FROM_BUILT_PRODUCTS_DIR NO CACHE STRING "" FORCE )

        if ( iOS )
            # http://pandorawiki.org/Floating_Point_Optimization#NFP_.2F_VFP_to_ARM_Transfers
            # https://groups.google.com/forum/?fromgroups#!topic/android-ndk/KW0JNEpZ5wg
            # https://developer.apple.com/library/ios/documentation/Xcode/Conceptual/iPhoneOSABIReference/Articles/ARMv7FunctionCallingConventions.html
            # http://infocenter.arm.com/help/topic/com.arm.doc.ihi0055b/IHI0055B_aapcs64.pdf ARM64
            set( armCompilerSwitches "-mfpu=neon" ) #-fsingle-precision-constant -mfloat-abi=softfp
            set( sizeOptimizationSwitch "${sizeOptimizationSwitch} -mthumb" )
            set( XCODE_ATTRIBUTE_CFLAGS_armv7  "-mcpu=cortex-a8 -mtune=cortex-a9"  ) 
            set( XCODE_ATTRIBUTE_CFLAGS_armv7s "                -mtune=cortex-a15" ) # http://www.anandtech.com/show/6292/iphone-5-a6-not-a15-custom-core
        else()
            # https://developer.apple.com/library/mac/#documentation/Darwin/Conceptual/64bitPorting/building/building.html
            # https://gcc.gnu.org/onlinedocs/gcc/i386-and-x86-64-Options.html
            set( x86CompilerSwitches "-mmmx -mfpmath=sse -mcx16" ) #clang unsupported -msahf

            if ( LE_TARGET_ARCHITECTURE STREQUAL sse3 )
                set( XCODE_ATTRIBUTE_CFLAGS_i386   "-msse3  -march=prescott -mtune=core2"  )
                set( XCODE_ATTRIBUTE_CFLAGS_x86_64 "-mssse3 -march=core2    -mtune=corei7" )
            elseif ( LE_TARGET_ARCHITECTURE STREQUAL sse4.1 )
                set( XCODE_ATTRIBUTE_CFLAGS_i386   "-msse4.1 -march=core2 -mtune=core2"  )
                set( XCODE_ATTRIBUTE_CFLAGS_x86_64 "-msse4.1 -march=core2 -mtune=corei7" )
            elseif( DEFINED LE_TARGET_ARCHITECTURE ) #...mrmlj...when buildOptions has to be included before the architecture is set...cleanup...
                message( FATAL_ERROR "Unknown OSX architecture (${LE_TARGET_ARCHITECTURE})" )
            endif()
            # https://github.com/mxcl/homebrew/issues/12946
            # http://lists.apple.com/archives/xcode-users/2012/Jun/msg00056.html
            # http://stackoverflow.com/questions/3544035/what-is-the-difference-between-fpic-and-fpic-gcc-parameters
            # http://docs.oracle.com/cd/E19082-01/819-0690/chapter4-29405/index.html
            set( XCODE_ATTRIBUTE_CFLAGS_x86_64 "${XCODE_ATTRIBUTE_CFLAGS_x86_64} -fpic" )
            # Implementation note:
            #   Dubious linker errors happen with Clang when Boost.MMAP is not
            # used as header only. Maybe this is related to the following bug:
            # http://llvm.org/bugs/show_bug.cgi?id=10113.
            #                                 (25.08.2011.) (Domagoj Saric)
            add_definitions( -DBOOST_MMAP_HEADER_ONLY )

            set( CMAKE_OSX_ARCHITECTURES           "$(ARCHS_STANDARD_32_64_BIT)" CACHE STRING "OSX architectures"     FORCE )
            set( CMAKE_XCODE_ATTRIBUTE_VALID_ARCHS "$(ARCHS_STANDARD_32_64_BIT)" CACHE STRING "OSX architectures"     FORCE )
            set( CMAKE_OSX_SYSROOT                 "macosx"                      CACHE STRING "OSX Base SDK"          FORCE ) #"Latest Mac OS X"
            set( CMAKE_OSX_SYSROOT_DEFAULT         ${CMAKE_OSX_SYSROOT}          CACHE STRING "OSX Base SDK default"  FORCE )
            if ( LE_SDK_BUILD )
                set( CMAKE_OSX_DEPLOYMENT_TARGET   "10.7"                        CACHE STRING "OSX deployment target" FORCE )
            else() #...mrmlj...in 2013 a lot of people still seemed to be using SL...however current code does not compile with the old libstdc++ from SL so libc++/10.7 has to be used...
                set( CMAKE_OSX_DEPLOYMENT_TARGET   "10.7"                        CACHE STRING "OSX deployment target" FORCE )
            endif()
        endif()

    else() #...ANDROID...

        #...mrmlj...add...%ANDROID_NDK%/toolchains/arm-linux-androideabi-4.8/prebuilt/windows-x86_64/bin/arm-linux-androideabi-strip --strip-unneeded  ./libs/armeabi-v7a/lib*.so
        #...mrmlj...-nodefaultlibs -lgcc -lc -lstdc++ -lc++_static -lm -ldl
        set( LEB_SHARED_LINKER_FLAGS_RELEASE "${LEB_SHARED_LINKER_FLAGS_RELEASE} ${ANDROID_LINKER_FLAGS} -lc++_static -lm --sysroot=${ANDROID_SYSROOT} -Wl,--icf=all -Wl,--gc-sections -s" ) # -fuse-ld=gold -fuse-ld=bfd -fuse-ld=mcld -Wl,-s
        if ( CMAKE_CXX_COMPILER_ID MATCHES Clang )
            set( LEB_SHARED_LINKER_FLAGS_RELEASE "-gcc-toolchain ${ANDROID_TOOLCHAIN_ROOT} -target ${ANDROID_LLVM_TRIPLE} ${LEB_SHARED_LINKER_FLAGS_RELEASE}" )
        endif()

        unset( armCompilerSwitches )
        unset( x86CompilerSwitches )
        if ( X86 OR X86_64 )
            if ( X86_64 )
                set( x86CompilerSwitches "-m64 -march=atom -msse4.2 -mpopcnt" ) # assume Silvermont arch
            else()
                set( x86CompilerSwitches "-m32 -march=atom -mmmx -mssse3 -mcx16" )
            endif()
            if ( CMAKE_CXX_COMPILER_ID MATCHES GNU )
                set( x86CompilerSwitches "${x86CompilerSwitches} -mfpmath=both -mno-ieee-fp" )
            endif()
        else()
            if ( ARM64_V8A )
                set( armCompilerSwitches "-march=armv8-a" )
            elseif ( ARMEABI_V7A )
                set( armCompilerSwitches "-march=armv7-a" )
                set( sizeOptimizationSwitch "${sizeOptimizationSwitch} -mthumb" )
                if ( NEON )
                    set( armCompilerSwitches "${armCompilerSwitches} -mtune=cortex-a8" )
                elseif ( VFPV3 )
                endif()
                #...mrmlj...r9b+...
                #...mrmlj...tests/device/hard-float/jni/Android.mk
                #...mrmlj...__NDK_FPABI__@usr/include/sys/cdefs.h
                #...mrmlj...https://code.google.com/p/android/issues/detail?id=61784
                #...mrmlj...http://blog.alexrp.com/2014/02/18/android-hard-float-support
                #...mrmlj...gnustl has a hard build...
                #...mrmlj...http://gcc.gnu.org/onlinedocs/gcc/Function-Attributes.html pcs aapcs
                #set( ANDROID_CXX_FLAGS "${ANDROID_CXX_FLAGS} -mfloat-abi=hard" )
            elseif ( ARMEABI_V6 )
                set( armCompilerSwitches "-marm -march=armv6k -mtune=arm1136j-s" )
            elseif ( ARMEABI    )
            elseif( DEFINED LE_TARGET_ARCHITECTURE ) #...mrmlj...when buildOptions has to be included before the architecture is set...cleanup...
                message( FATAL_ERROR "Unknown Android architecture (${LE_TARGET_ARCHITECTURE})" )
            endif()
            set( armCompilerSwitches "-marm ${armCompilerSwitches}" ) #...mrmlj...-fsingle-precision-constant
            # http://gcc.gnu.org/onlinedocs/libstdc++/manual/bk01pt01ch01s02.html
            # http://stackoverflow.com/questions/3214168/linking-statically-with-glibc-and-libstdc
            #set( CMAKE_SHARED_LINKER_FLAGS "--fix-cortex-a8" ) clang++.exe: error: unsupported option '--fix-cortex-a8' (ndk r9d)
        endif( X86 OR X86_64 )
        if ( CMAKE_CXX_COMPILER_ID MATCHES Clang )
            set( ANDROID_CXX_FLAGS "-target ${ANDROID_LLVM_TRIPLE} -v ${ANDROID_CXX_FLAGS}" ) #-stdlib=libc++
        endif()

        # "What's the difference between gnustl_static & stlport_static?"
        # https://groups.google.com/forum/#!topic/android-ndk/OWl_orR0DRQ
        if ( CMAKE_CXX_COMPILER_ID MATCHES Clang )
            set( stlPath     "${ANDROID_NDK}/sources/cxx-stl/llvm-libc++" )
            set( stlIncludes "libcxx/include" )
        else()
            set( stlPath "${ANDROID_NDK}/sources/cxx-stl/gnu-libstdc++/${ANDROID_COMPILER_VERSION}" )
        endif()
        include_directories(
            BEFORE
            SYSTEM
            "${ANDROID_NDK}/sources/android/support/include"
            "${stlPath}/${stlIncludes}"
        )

        link_directories(
            "${ANDROID_SYSROOT}/usr/lib"
            "${stlPath}/libs/${ANDROID_NDK_ABI_NAME}/thumb"
            "${stlPath}/libs/${ANDROID_NDK_ABI_NAME}"
        )
    endif() # APPLE | ANDROID

    set( LEB_C_FLAGS_DEBUG
        "-O0 ${debugSymbolsCompilerSwitch} ${rttiOnSwitch} -DDEBUG=1 -D_DEBUG=1 -fstack-protector-all ${LEB_C_FLAGS_DEBUG}"
    )
else()
    message( FATAL_ERROR "Unknown compiler CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} CMAKE_CXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}." )
endif()


################################################################################
#   LE_setFinalFlags()
################################################################################

function( LE_setFinalFlags )
    set( LEB_C_FLAGS_DEBUG   "${LEB_C_FLAGS_DEBUG}   ${debugSymbolsCompilerSwitch}" )
    set( LEB_C_FLAGS_RELEASE "${LEB_C_FLAGS_RELEASE} ${LEB_sharedCReleaseFlags}"    )

    set( CMAKE_CXX_FLAGS         "${LEB_cppSpecificFlags} ${LEB_C_FLAGS}"        CACHE STRING "C++ shared"  FORCE )
    set( CMAKE_CXX_FLAGS_DEBUG                            ${LEB_C_FLAGS_DEBUG}   CACHE STRING "C++ debug"   FORCE )
    set( CMAKE_CXX_FLAGS_RELEASE                          ${LEB_C_FLAGS_RELEASE} CACHE STRING "C++ release" FORCE )
    set( CMAKE_C_FLAGS           "${LEB_cSpecificFlags}   ${LEB_C_FLAGS}"        CACHE STRING "C shared"    FORCE )
    set( CMAKE_C_FLAGS_DEBUG                              ${LEB_C_FLAGS_DEBUG}   CACHE STRING "C debug"     FORCE )
    set( CMAKE_C_FLAGS_RELEASE                            ${LEB_C_FLAGS_RELEASE} CACHE STRING "C release"   FORCE )

    #...mrmlj...the quotes are necessary (at least with msvc10) as the flags seem to be duplicated (concatenated with a semicolon, which causes the linker to fail with an error)..!?
    set( CMAKE_EXE_LINKER_FLAGS            "${LEB_SHARED_LINKER_FLAGS}         ${LEB_sharedLinkerSwitches}" CACHE STRING "link exe"            FORCE )
    set( CMAKE_EXE_LINKER_FLAGS_DEBUG      "${LEB_SHARED_LINKER_FLAGS_DEBUG}   ${debugSymbolsLinkerSwitch}" CACHE STRING "link exe debug"      FORCE )
    set( CMAKE_EXE_LINKER_FLAGS_RELEASE    "${LEB_SHARED_LINKER_FLAGS_RELEASE}                            " CACHE STRING "link exe release"    FORCE )
    set( CMAKE_MODULE_LINKER_FLAGS         "${LEB_SHARED_LINKER_FLAGS}         ${LEB_sharedLinkerSwitches}" CACHE STRING "link module"         FORCE )
    set( CMAKE_MODULE_LINKER_FLAGS_DEBUG   "${LEB_SHARED_LINKER_FLAGS_DEBUG}   ${debugSymbolsLinkerSwitch}" CACHE STRING "link module debug"   FORCE )
    set( CMAKE_MODULE_LINKER_FLAGS_RELEASE "${LEB_SHARED_LINKER_FLAGS_RELEASE}                            " CACHE STRING "link module release" FORCE )
    set( CMAKE_SHARED_LINKER_FLAGS         "${LEB_SHARED_LINKER_FLAGS}         ${LEB_sharedLinkerSwitches}" CACHE STRING ""                    FORCE )
    set( CMAKE_SHARED_LINKER_FLAGS_DEBUG   "${LEB_SHARED_LINKER_FLAGS_DEBUG}   ${debugSymbolsLinkerSwitch}" CACHE STRING ""                    FORCE )
    set( CMAKE_SHARED_LINKER_FLAGS_RELEASE "${LEB_SHARED_LINKER_FLAGS_RELEASE}                            " CACHE STRING ""                    FORCE )
    set( CMAKE_STATIC_LINKER_FLAGS         "${LEB_STATIC_LINKER_FLAGS}                                    " CACHE STRING ""                    FORCE )
    set( CMAKE_STATIC_LINKER_FLAGS_DEBUG   "${LEB_STATIC_LINKER_FLAGS_DEBUG}   ${debugSymbolsLinkerSwitch}" CACHE STRING ""                    FORCE )
    set( CMAKE_STATIC_LINKER_FLAGS_RELEASE "${LEB_STATIC_LINKER_FLAGS_RELEASE}                            " CACHE STRING ""                    FORCE )
endfunction()


################################################################################
# Misc...
################################################################################

set( enableAsserts BOOST_ENABLE_ASSERT_HANDLER LE_CHECKED_BUILD=1 )
set( disableAsserts NDEBUG BOOST_DISABLE_ASSERTS )
if ( APPLE )
    set( disableAsserts ${disableAsserts} NS_BLOCK_ASSERTIONS=1 )
endif()
#...mrmlj...GLOBAL does not work but DIRECTORY does...
# http://www.cmake.org/cmake/help/v3.0/policy/CMP0043.html
set_property( DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
    $<$<CONFIG:Debug>:${enableAsserts}>
    $<$<CONFIG:RelWithDebInfo>:${enableAsserts}>
    $<$<CONFIG:MinSizeRel>:${disableAsserts}>
    #...mrmlj...left for projects to decide for themselves...required for the
    #...mrmlj...LE_CREATE_PACKAGE_TARGET functionality (building "dev" targets with
    #...mrmlj...asserts under the Release configuration)...
    #$<$<CONFIG:Release       >:${disableAsserts}>
)


add_definitions(
    -DBOOST_EXCEPTION_DISABLE
    -DBOOST_MPL_LIMIT_STRING_SIZE=8
    -DLE_CONFIGURATION_3rd_PARTY_ROOT="$ENV{LEB_3rdParty_root}"
)

if ( MSVC )
    if ( CMAKE_SIZEOF_VOID_P MATCHES 8 )
        set( archLibDir "/x64" )
    else()
        set( archLibDir "/Win32" )
    endif()
elseif ( APPLE )
    set( archLibDir "/" ) # universal binaries
endif()

################################################################################
