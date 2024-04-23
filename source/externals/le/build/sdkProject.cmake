################################################################################
#
# createSDKProject() - shared logic for creating LE SDK targets
#
# Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
#
################################################################################


include( "${CMAKE_CURRENT_LIST_DIR}/utilities.cmake" )


# Implementation note:
# A workaround for the fact that the iOSUniversalBuild.sh script uses
# 'intermediate' configuration types (*-iphoneos and *-simulator) one of which
# ends up being passed to CPack.
#                                             (15.03.2016.) (Domagoj Saric)
if ( iOS )
    set( installConfigs )
else()
    set( installConfigs Release )
endif()

################################################################################
#   createPublicTarget()
################################################################################

set( leExternals "externals/le" )

    # helper: createPublicDevTarget
    set( devTargetSuffix "(dev)" CACHE INTERNAL "" FORCE )
    set( devLibSuffix    "debug" CACHE INTERNAL "" FORCE ) # If not stored in the cache CMake (2.8.12.2) somehow 'forgets' the variable when it gets to the Utility lib/target..
    function( createPublicDevTarget projectName projectLabel architecture sources )
        if ( LE_CREATE_PACKAGE_TARGET )
            set( devProjectName ${projectName}_${devLibSuffix} )

            add_library (        ${devProjectName} STATIC ${sources} )
            set_property( TARGET ${devProjectName} PROPERTY PROJECT_LABEL       "${projectLabel} ${devTargetSuffix}" )
            appendProperty( ${devProjectName} COMPILE_DEFINITIONS ${enableAsserts};LE_PUBLIC_BUILD )
           #set_property( TARGET ${devProjectName} PROPERTY ARCHIVE_OUTPUT_DIRECTORY_RELEASE "development"           )
           #set_property( TARGET ${devProjectName} PROPERTY ARCHIVE_OUTPUT_NAME_RELEASE      ${projectName}          )
            setupTargetForPlatform( ${devProjectName} ${architecture} )
            addPrefixHeader( ${devProjectName} "${PROJECT_SOURCE_DIR}/${leExternals}/build/leConfigurationAndODRHeader.h" )
            if ( LEB_SDK_USE_LTO )
                appendProperty( ${devProjectName} LINK_FLAGS_RELEASE ${ltoLinkerSwitch} )
                target_compile_options( ${devProjectName} PRIVATE $<$<NOT:$<CONFIG:Debug>>:${ltoCompilerSwitch}> )
                set_target_properties( ${devProjectName} PROPERTIES LINKER_LANGUAGE CXX )
            endif( LEB_SDK_USE_LTO )
            if ( MSVC )
                #...mrmlj...runtime crashes ocur with /MDd (at calls to __imp_* functions)...
                appendProperty( ${devProjectName} COMPILE_FLAGS "/MTd" )
                appendProperty( ${devProjectName} COMPILE_DEFINITIONS _ALLOW_ITERATOR_DEBUG_LEVEL_MISMATCH )
            endif()
            appendProperty( ${devProjectName} COMPILE_FLAGS          ${LEB_SDK_COMPILE_FLAGS}          )
            appendProperty( ${devProjectName} COMPILE_FLAGS_RELEASE "${LEB_SDK_COMPILE_FLAGS_RELEASE}" )
           #appendProperty( ${devProjectName} COMPILE_FLAGS_RELEASE "${debugSymbolsCompilerSwitch}"    )

            list( APPEND leSDKTargets ${devProjectName} )
        endif( LE_CREATE_PACKAGE_TARGET )
    endfunction( createPublicDevTarget )

function( createPublicTarget projectName projectLabel sources headers )

    # Implementation note:
    # Quick workaround for SDKs with (3rd party) sources incompatible with unity
    # builds.
    #                                         (25.02.2016.) (Domagoj Saric)
    set( unityBuildIncompatibleSources ${ARGN} )
    if ( LEB_SDK_USE_LTO )
        set( sources ${sources} ${unityBuildIncompatibleSources} )
    else( LEB_SDK_USE_LTO ) # use unity builds for builds that cannot use LTO
        createUnityBuildCPP( "${sources}" )
        set_source_files_properties( ${sources} PROPERTIES HEADER_FILE_ONLY true )

        if ( APPLE )
            set_source_files_properties( ${staticLibUnityBuildCPPFile} PROPERTIES COMPILE_FLAGS "-x objective-c++" ) #...mrmlj...melodify sdk, utility lib...
        endif()

        set( headers ${sources} ${headers}                                          )
        set( sources ${staticLibUnityBuildCPPFile} ${unityBuildIncompatibleSources} )
    endif( LEB_SDK_USE_LTO )

    include( ${PROJECT_SOURCE_DIR}/${leExternals}/build/buildOptions.cmake )

    foreach( architecture ${LEB_ARCHITECTURES} )

        string( REPLACE "," ";" architecture "${architecture}" )
        list( GET architecture 0 LE_TARGET_ARCHITECTURE )
        list( GET architecture 1 architectureSuffix     )
        if( ANDROID )
            unset( X86                  )
            unset( MIPS                 )
            unset( ARMEABI              )
            unset( ARMEABI_V6           )
            unset( ARMEABI_V7A          )
            unset( VFPV3                )
            unset( NEON                 )
            unset( ANDROID_CXX_FLAGS    )
            unset( ANDROID_LINKER_FLAGS )

            set( ANDROID_ABI ${LE_TARGET_ARCHITECTURE} CACHE STRING "The target ABI for Android." FORCE )

            include( ${PROJECT_SOURCE_DIR}/${leExternals}/build/android.toolchain.cmake )
        endif()

        #...mrmlj..."setup for architecture"...should be replaced by a function call...
        include( ${PROJECT_SOURCE_DIR}/${leExternals}/build/buildOptions.cmake )
        # Implementation note:
        #   The global options have to be set before the per-project ones (with
        # set_property), otherwise the per-project ones get overwritten.
        #                                     (27.07.2011.) (Domagoj Saric)
        LE_setFinalFlags()

        set( projectNameWithSuffix ${projectName}_${LEB_OS_SUFFIX} )
        if ( architectureSuffix )
            set( projectNameWithSuffix ${projectNameWithSuffix}_${architectureSuffix} )
        endif()

        message( STATUS "[LEB] Creating target \"${projectNameWithSuffix}\"" )
        add_library (        ${projectNameWithSuffix} STATIC ${sources} ${headers} )
        set_property( TARGET ${projectNameWithSuffix} PROPERTY PROJECT_LABEL       "${projectLabel} (${LE_TARGET_ARCHITECTURE})" )
        set_property( TARGET ${projectNameWithSuffix} PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Release>:${disableAsserts}>        )
        setupTargetForPlatform( ${projectNameWithSuffix} ${architecture} )
        addPrefixHeader( ${projectNameWithSuffix} "${PROJECT_SOURCE_DIR}/${leExternals}/build/leConfigurationAndODRHeader.h" )
        if ( LEB_SDK_USE_LTO )
            appendProperty( ${projectNameWithSuffix} LINK_FLAGS_RELEASE ${ltoLinkerSwitch} )
            target_compile_options( ${projectNameWithSuffix} PRIVATE $<$<NOT:$<CONFIG:Debug>>:${ltoCompilerSwitch}> )
            set_target_properties( ${projectNameWithSuffix} PROPERTIES LINKER_LANGUAGE CXX )
        endif( LEB_SDK_USE_LTO )

        appendProperty( ${projectNameWithSuffix} COMPILE_FLAGS         ${LEB_SDK_COMPILE_FLAGS}         )
        appendProperty( ${projectNameWithSuffix} COMPILE_FLAGS_RELEASE ${LEB_SDK_COMPILE_FLAGS_RELEASE} )
        # Implementation note:
        #   For now this is set/defined/configured in
        # leConfigurationAndODRHeader.h.
        #                                     (08.01.2015.) (Domagoj Saric)
       #appendProperty( ${projectNameWithSuffix} COMPILE_DEFINITIONS -DLE_IMPL_NAMESPACE_BEGIN\(\)="namespace LE { namespace {" -DLE_IMPL_NAMESPACE_END\(\)="} }" )

        list( APPEND leSDKTargets ${projectNameWithSuffix} ) # required for appendPropertyToAllArchitectures()
        createPublicDevTarget( ${projectNameWithSuffix} "${projectLabel} (${LE_TARGET_ARCHITECTURE})" ${LE_TARGET_ARCHITECTURE} "${sources}" )

        if( LE_CREATE_PACKAGE_TARGET )
            if( ANDROID )
                set( libPrefix    lib )
                set( libExtension a   )
                set( outputPath libs/${ANDROID_NDK_ABI_NAME} )
            elseif( APPLE )
                set( libPrefix    lib )
                set( libExtension a   )
                set( outputPath Release )
            elseif( WIN32 )
                set( libPrefix    ""  )
                set( libExtension lib )
                set( outputPath Release )
            endif()
            list( APPEND releaseBinaries     "${CMAKE_CURRENT_BINARY_DIR}/${outputPath}/${libPrefix}${projectNameWithSuffix}.${libExtension}"       )
            list( APPEND developmentBinaries "${CMAKE_CURRENT_BINARY_DIR}/${outputPath}/${libPrefix}${projectNameWithSuffix}_debug.${libExtension}" )
        endif()

    endforeach()

    forwardToParentScope( releaseBinaries     )
    forwardToParentScope( developmentBinaries )
    forwardToParentScope( leSDKTargets        )

endfunction( createPublicTarget )


################################################################################
#   createSDKProject()
################################################################################
# Implementation note:
#   The straight approach of simply defining createSDKProject as a function does
# not work correctly when createSDKProject is called from within another
# function. In such cases key CPack configuration variables don't seem to
# "reach" CPack. For this reason a wrapper macro is used that "re-forwards" the
# key variables one more level (up the calling stack).
#                                             (30.08.2012.) (Domagoj Saric)
# Todo:
#   Move project-independent/global variables out of these function/macro calls
# to circumvent this problem (e.g. like installConfigs).
#                                             (15.03.2016.) (Domagoj Saric)
################################################################################

macro( createSDKProject
    projectName
    projectLabel
    projectDescription
    versionMajor
    versionMinor
    versionPatch
    sources
)

    createSDKProjectAux(
        "${projectName}"
        "${projectLabel}"
        "${projectDescription}"
        "${versionMajor}"
        "${versionMinor}"
        "${versionPatch}"
        "${sources}"
        ${ARGN}
    )

    if( LE_CREATE_PACKAGE_TARGET AND NOT LE_BUILD_SUBPROJECT_BUILD )
        #...mrmlj...certainly required...
        forwardToParentScope( CMAKE_INSTALL_PREFIX           )
        forwardToParentScope( CPACK_INSTALL_PREFIX           )
        forwardToParentScope( CPACK_PACKAGING_INSTALL_PREFIX )
        forwardToParentScope( CPACK_PACKAGE_DEFAULT_LOCATION )
        forwardToParentScope( CPACK_PACKAGE_FILE_NAME        )

        #...mrmlj...possibly required...
        forwardToParentScope( CPACK_PACKAGE_DESCRIPTION_SUMMARY )
        forwardToParentScope( CPACK_PACKAGE_VENDOR              )

        forwardToParentScope( CPACK_PACKAGE_VERSION_MAJOR      )
        forwardToParentScope( CPACK_PACKAGE_VERSION_MINOR      )
        forwardToParentScope( CPACK_PACKAGE_VERSION_PATCH      )
        forwardToParentScope( CPACK_PACKAGE_INSTALL_DIRECTORY  )
        forwardToParentScope( CPACK_PACKAGE_DIRECTORY          )
        forwardToParentScope( CPACK_TOPLEVEL_TAG               )
        forwardToParentScope( CPACK_INCLUDE_TOPLEVEL_DIRECTORY )
        forwardToParentScope( CPACK_OSX_PACKAGE_VERSION        )

        forwardToParentScope( CPACK_MONOLITHIC_INSTALL            )
        forwardToParentScope( CPACK_ARCHIVE_COMPONENT_INSTALL     )
        forwardToParentScope( CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE )
        forwardToParentScope( CPACK_COMPONENTS_IGNORE_GROUPS      )

        forwardToParentScope( CPACK_COMPONENT_UNSPECIFIED_HIDDEN   )
        forwardToParentScope( CPACK_COMPONENT_UNSPECIFIED_REQUIRED )

    endif( LE_CREATE_PACKAGE_TARGET AND NOT LE_BUILD_SUBPROJECT_BUILD )

endmacro( createSDKProject )


################################################################################
# createSDKProjectAux()
################################################################################

function( createSDKProjectAux
    projectName
    projectLabel
    projectDescription
    versionMajor
    versionMinor
    versionPatch
    sources
)
    if ( leSDKTargets )
        message( FATAL_ERROR "An SDK project has already been created?" )
    endif()
    
    if ( LE_SDK_DEMO )
        add_definitions( -DLE_SDK_DEMO_BUILD )
    endif()

    ############################################################################
    # Setup targets
    ############################################################################

    # Implementation note:
    # Because CMake lacks support for "One build, multiple compilers and
    # packages" 
    #                                         (25.02.2016.) (Domagoj Saric)
    # http://stackoverflow.com/questions/9542971/using-cmake-with-multiple-compilers-for-the-same-language
    # http://cmake.3232098.n2.nabble.com/One-build-multiple-compilers-and-packages-td7585262.html
    # http://www.kitware.com/media/html/BuildingExternalProjectsWithCMake2.8.html
    # http://www.cmake.org/Wiki/CMake_FAQ#I_change_CMAKE_C_COMPILER_in_the_GUI_but_it_changes_back_on_the_next_configure_step._Why.3F

    if ( LE_BUILD_SUBPROJECT_BUILD OR NOT DEFINED LEB_ABIs )
        if( LE_BUILD_SUBPROJECT_TAG )
            #ignore for sdk silence warning
            #set( projectName "${projectName}_${LE_BUILD_SUBPROJECT_TAG}" )
        endif()
        createPublicTarget( "${projectName}" "${projectLabel}" "${sources}" "" ${ARGN} )
    elseif ( ANDROID )
        file( TO_NATIVE_PATH "${PROJECT_SOURCE_DIR}/${leExternals}/build/android.toolchain.cmake" androidToolchainFile )
        set( androidToolchainFileOptions -DCMAKE_TOOLCHAIN_FILE="${androidToolchainFile}" )

        foreach( abi ${LEB_ABIs} )
            set( extraForwardedVariables
                -DLE_SDK_PROJECT_NAME=${projectName}
                -DLE_SDK_PROJECT_LABEL="${projectLabel}"
                -DLE_CREATE_PACKAGE_TARGET:BOOL=${LE_CREATE_PACKAGE_TARGET}
                -DLE_BUILD_ABI:STRING=${abi}
                -DLEB_NOT_USING_NT2=${LEB_NOT_USING_NT2}
               #-DCMAKE_BUILD_TYPE:STRING=Release #${CMAKE_BUILD_TYPE}
                -DCMAKE_TOOLCHAIN_FILE:FILEPATH="${CMAKE_TOOLCHAIN_FILE}"
            )
            if ( DEFINED LE_SDK_DEMO )
                list( APPEND extraForwardedVariables -DLE_SDK_DEMO=${LE_SDK_DEMO} )
            endif()
            LE_addArchSubproject( ${projectName} ${abi} "${CMAKE_GENERATOR}" "${extraForwardedVariables}" "${androidToolchainFileOptions}" "-DLE_BUILD_ABI=${abi}" )
        endforeach()
    elseif( MSVC )
        if ( NOT CMAKE_GENERATOR MATCHES 64 )
            message( FATAL_ERROR "Please use a 64 bit Visual Studio generator (for the top level project)." )
        endif()
        # Todo: unify/abstract multiple ABI handling (Android & Windows).
        #                                     (12.11.2015.) (Domagoj Saric)
        createPublicTarget( "${projectName}" "${projectLabel}" "${sources}" "" ${ARGN} )
        string( REPLACE " Win64" "" x86Generator ${CMAKE_GENERATOR} )
        # not ${projectName}
        LE_addArchSubproject( Win32 build "${x86Generator}" )
    else()
        message( SEND_ERROR "LEB: unexpected generator-platform-ABI combination." )
    endif()

    list( GET leSDKTargets 0 defaultArchSDKTarget )
    forwardToParentScope( defaultArchSDKTarget )


    ############################################################################
    # Subproject install setup
    ############################################################################

    if ( LE_CREATE_PACKAGE_TARGET )

        ########################################################################
        # Setup system variables (for cross compiling)
        ########################################################################

        unset( releaseBinaries     CACHE )
        unset( developmentBinaries CACHE )
        if ( ANDROID )
            setAndForwardToParentScope( CPACK_CMAKE_GENERATOR "${androidGenerator}" )
            setAndForwardToParentScope( CPACK_SYSTEM_NAME     "Android"             )
        elseif( iOS )
            setAndForwardToParentScope( CMAKE_SYSTEM_NAME iOS                  )
            setAndForwardToParentScope( CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME} )
        elseif( APPLE )
            setAndForwardToParentScope( CPACK_SYSTEM_NAME OSX )
        else()
            setAndForwardToParentScope( CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}" )
        endif()

        ########################################################################
        # Setup CPack and CMake install staging variables
        ########################################################################

        # https://cmake.org/Wiki/CMake:CPackConfiguration
        # http://www.cmake.org/Wiki/CMake:Packaging_With_CPack
        # http://www.cmake.org/Wiki/CMake:CPackPackageGenerators
        # http://www.cmake.org/cmake/help/v2.8.12/cpack.html
        #
        # http://stackoverflow.com/questions/6712000/cmake-cpack-package-installation-path-nightmare

        setAndForwardToParentScope( CPACK_PACKAGE_DESCRIPTION_SUMMARY ${projectDescription} )
        setAndForwardToParentScope( CPACK_PACKAGE_VENDOR              "Little Endian Ltd."  )

        set( CPACK_PACKAGE_FILE_NAME "${projectName}_${versionMajor}.${versionMinor}.${versionPatch}" )
        setAndForwardToParentScope( CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}_${CPACK_SYSTEM_NAME}" )
        if ( LE_SDK_DEMO )
            setAndForwardToParentScope( CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}_demo" )
        endif()

        file( RELATIVE_PATH CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR} ${CMAKE_BINARY_DIR} )
        if ( NOT EXISTS "${PROJECT_SOURCE_DIR}/${CMAKE_INSTALL_PREFIX}" )
            set( CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR} )
            setAndForwardToParentScope( CPACK_SET_DESTDIR true )
        endif()

        setAndForwardToParentScope( CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/install" )
        setAndForwardToParentScope( CPACK_INSTALL_PREFIX "${CPACK_PACKAGE_FILE_NAME}/"     )
       #setAndForwardToParentScope( CPACK_PACKAGING_INSTALL_PREFIX ${CPACK_INSTALL_PREFIX} )
       #setAndForwardToParentScope( CPACK_PACKAGE_DEFAULT_LOCATION "/"                     )

        setAndForwardToParentScope( CPACK_PACKAGE_VERSION_MAJOR     ${versionMajor} )
        setAndForwardToParentScope( CPACK_PACKAGE_VERSION_MINOR     ${versionMinor} )
        setAndForwardToParentScope( CPACK_PACKAGE_VERSION_PATCH     ${versionPatch} )
       #setAndForwardToParentScope( CPACK_PACKAGE_INSTALL_DIRECTORY ${projectName}  )
        #...zzz...CPack "working directory", command line -B option
       #setAndForwardToParentScope( CPACK_PACKAGE_DIRECTORY         ${projectName}  )
        setAndForwardToParentScope( CPACK_TOPLEVEL_TAG "Little Endian"    )
        setAndForwardToParentScope( CPACK_INCLUDE_TOPLEVEL_DIRECTORY YES  )
        setAndForwardToParentScope( CPACK_OSX_PACKAGE_VERSION        10.7 )
        # http://www.cmake.org/Wiki/CMake:Component_Install_With_CPack#Controlling_packaging_of_component-aware_generators
        if ( APPLE )
            set( cpackArchive ZIP ) #...mrmlj...TBZ2 has a much better compression ratio and is natively supported by OSX but isn't used yet because.....
        else()
            set( cpackArchive ZIP )
        endif()
        setAndForwardToParentScope( CPACK_GENERATOR          ${cpackArchive} )
        setAndForwardToParentScope( CPACK_MONOLITHIC_INSTALL ON              )
        if ( NOT CPACK_MONOLITHIC_INSTALL )
            setAndForwardToParentScope( CPACK_ARCHIVE_COMPONENT_INSTALL     ON  )
            setAndForwardToParentScope( CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE ON  )
            setAndForwardToParentScope( CPACK_COMPONENTS_IGNORE_GROUPS      OFF )
        endif( NOT CPACK_MONOLITHIC_INSTALL )

        setAndForwardToParentScope( CPACK_COMPONENT_UNSPECIFIED_HIDDEN   TRUE )
        setAndForwardToParentScope( CPACK_COMPONENT_UNSPECIFIED_REQUIRED TRUE )


        ########################################################################
        # Setup Doxygen
        ########################################################################

        if ( NOT DOXYGEN_FOUND AND NOT LE_BUILD_SUBPROJECT_BUILD ) # skip multiple creations
            find_package( Doxygen )
            if ( DOXYGEN_FOUND )

                set( LE_DOXYGEN_CLANG_ASSISTED_PARSING NO CACHE BOOL "Use Clang for parsing sources when generating the documentation (more accurate but slower)" )
                mark_as_advanced( LE_DOXYGEN_CLANG_ASSISTED_PARSING )
                if ( LE_DOXYGEN_CLANG_ASSISTED_PARSING )
                    set( LE_DOXYGEN_CLANG_ASSISTED_PARSING YES )
                else()
                    set( LE_DOXYGEN_CLANG_ASSISTED_PARSING NO  )
                endif()

                if ( LE_DOXYGEN_CLANG_ASSISTED_PARSING )
                    # http://clang-developers.42468.n3.nabble.com/invalid-token-paste-td4025526.html
                    # http://lists.boost.org/boost-users/2007/12/32559.php
                    # http://clang.llvm.org/docs/PCHInternals.html
                    set( LE_DOXYGEN_CLANG_OPTIONS "-fms-extensions -fdelayed-template-parsing -fmsc-version=1600 -Wno-ignored-attributes -Wno-multichar -DLE_HAS_NT2 -DBOOST_DISPATCH_NO_RESTRICT -D\"BOOST_PP_CONFIG_FLAGS()\"=1 -DBOOST_MPL_LIMIT_STRING_SIZE=8 -D__GCCXML__ -D__GNUC__=4 -D__GNUC_MINOR__=2 -D__GNUC_PATCH__=1 -DDOXYGEN_ONLY -DLE_SDK_BUILD -include le/build/leConfigurationAndODRHeader.h" )
                endif()

                set( LE_SDK_DOXYGEN_SHOW_NAMESPACES  YES )
                set( LE_SDK_DOXYGEN_HIDE_SCOPE_NAMES YES )

                # Add all include directories to the Doxygen search path if none
                # were specified explicitly
                if ( NOT DEFINED LE_DOXYGEN_INCLUDE_PATHS )
                    #...mrmlj...reinvestigate whether this is necssary (can slow down the doxygen pass quite considerably)...
                    #get_directory_property( LE_DOXYGEN_INCLUDE_PATHS DIRECTORY ${PROJECT_SOURCE_DIR} INCLUDE_DIRECTORIES )
                    #string( REPLACE ";" "\" \"" LE_DOXYGEN_INCLUDE_PATHS "\"${LE_DOXYGEN_INCLUDE_PATHS}\"" )
                endif()

                # Implementation note:
                # CMake (up to 3.0) seems to have problems with variables
                # containing newlines and double/escaped backslashes in the
                # cache ("Parse error in cache file" errors get reported). For
                # this reason the LE_SDK_DOCUMENTATION_SOURCES and similar
                # variables is first filled as a standard CMake list and then
                # only here converted to a Doxygen-friendly multi-line list with
                # backslashes.
                # http://www.cmake.org/pipermail/cmake/2013-July/055232.html
                #                             (15.07.2014.) (Domagoj Saric)
                string( REPLACE ";" " \\\n\t" LE_SDK_DOCUMENTATION_SOURCES           "${LE_SDK_DOCUMENTATION_SOURCES}"           )
                string( REPLACE ";" " \\\n\t" LE_SDK_DOCUMENTATION_EXTRA_IMAGE_PATHS "${LE_SDK_DOCUMENTATION_EXTRA_IMAGE_PATHS}" )

                set( LE_SDK_INSTALL_STAGE_PATH "${PROJECT_SOURCE_DIR}/${CMAKE_INSTALL_PREFIX}" )

                configure_file(
                    "${PROJECT_SOURCE_DIR}/../doc/template/doxyfile.in"
                    "${PROJECT_BINARY_DIR}/doxyfile"
                )
                # Implementation note:
                # First remove the intermediate documentation directory to
                # ensure clean documentation generation (w/o leftovers from a
                # previous build).
                #                             (27.01.2014.) (Domagoj Saric)
                add_custom_target( Documentation
                    COMMAND           "${CMAKE_COMMAND}" -E remove_directory "${LE_SDK_INSTALL_STAGE_PATH}/doc"
                    COMMAND           "${CMAKE_COMMAND}" -E make_directory   "${LE_SDK_INSTALL_STAGE_PATH}/doc/html"
                    COMMAND           "${DOXYGEN_EXECUTABLE}" "${PROJECT_BINARY_DIR}/doxyfile"
                    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/../doc"
                    COMMENT           "Doxygen documentation"
                    SOURCES           "${PROJECT_SOURCE_DIR}/../doc/main_page.doxy"
                                      "${PROJECT_BINARY_DIR}/doxyfile"
                                      "${PROJECT_SOURCE_DIR}/../doc/tutorial_page.doxy"
                )
                if ( LE_CREATE_PACKAGE_TARGET )
                    set_property( TARGET Documentation PROPERTY EXCLUDE_FROM_ALL false )
                endif ( LE_CREATE_PACKAGE_TARGET )

            elseif()
               message( WARNING "Unable to find Doxygen" )
            endif()
        endif() # DOXYGEN_FOUND

        if ( ANDROID OR WIN32 ) #...mrmlj...make this generic...
            # Implementation note:
            #   As a mechanism for passing the list of (paths to) libraries
            # generated by subprojects we use the CMake cache as a persistent
            # variable memory. Environment variables aren't a suitable solution
            # because the child CMake process gets its own environment.
            # http://www.cmake.org/Wiki/CMake_FAQ#How_can_I_get_or_set_environment_variables.3F
            #                                 (28.02.2014.) (Domagoj Saric)
            if ( LE_BUILD_SUBPROJECT_BUILD )
                set( releaseBinaries     ${releaseBinaries}     CACHE STRING "LE subproject binaries (internal)" FORCE )
                set( developmentBinaries ${developmentBinaries} CACHE STRING "LE subproject binaries (internal)" FORCE )
            else()
                foreach( subProjectBinaryDir ${subProjectBinaryDirs} )
                    unset( childProject_releaseBinaries     CACHE )
                    unset( childProject_developmentBinaries CACHE )
                    load_cache( ${subProjectBinaryDir} READ_WITH_PREFIX childProject_ releaseBinaries developmentBinaries )
                    list( APPEND releaseBinaries     ${childProject_releaseBinaries}     )
                    list( APPEND developmentBinaries ${childProject_developmentBinaries} )
                endforeach()
            endif()
        endif()

        if ( NOT LE_BUILD_SUBPROJECT_BUILD )

            #...mrmlj...strip static libs...
            #http://stackoverflow.com/questions/1506346/with-gcc-how-do-i-export-only-certain-functions-in-a-static-library
            #http://stackoverflow.com/questions/393980/restricting-symbols-in-a-linux-static-library

            install(
                FILES          ${releaseBinaries}
                DESTINATION    "libs/release"
                COMPONENT      Libraries
                CONFIGURATIONS ${installConfigs}
            )

            # Redirect "development"/"public debug" builds into the libs/
            # development subdirectory and remove the "_debug" suffix from the
            # filename (creating them that way directly, e.g. with
            # ARCHIVE_OUTPUT_DIRECTORY, turned out too difficult as it requires
            # special handling for Android and iOS platforms):
            #install(
            #    FILES          ${developmentBinaries}
            #    DESTINATION    "libs/development"
            #    COMPONENT      ${projectName}Library
            #    CONFIGURATIONS ${installConfigs}
            #)
            foreach( lib ${developmentBinaries} )
                get_filename_component( libFileName ${lib} NAME )
                string( REPLACE _debug "" libFileName ${libFileName} )
                install(
                    FILES "${lib}"
                    DESTINATION libs/development
                    COMPONENT Libraries
                    RENAME ${libFileName}
                    CONFIGURATIONS ${installConfigs}
                )    
            endforeach()

            if ( TARGET Documentation )
                install(
                    DIRECTORY      "${LE_SDK_INSTALL_STAGE_PATH}/doc"
                    DESTINATION    "./"
                    COMPONENT      Documentation
                    CONFIGURATIONS ${installConfigs}
                )
                install(
                    FILES          "${PROJECT_SOURCE_DIR}/../doc/readme.txt"
                    DESTINATION    "./doc"
                    COMPONENT      Documentation
                    CONFIGURATIONS ${installConfigs}
                )
                install(
                    FILES          "${PROJECT_SOURCE_DIR}/../doc/template/documentation.html"
                    DESTINATION    "./doc"
                    COMPONENT      Documentation
                    CONFIGURATIONS ${installConfigs}
                )
            endif()

            if ( NOT CPACK_MONOLITHIC_INSTALL )
                include( CPack )
                
                setAndForwardToParentScope( CPACK_COMPONENTS_ALL Libraries Headers Documentation )

                cpack_add_component_group( ${projectName} )

                cpack_add_component(
                    Libraries
                    DISPLAY_NAME Libraries
                    GROUP ${projectName}
                )

                cpack_add_component(
                    Headers
                    DISPLAY_NAME Headers
                    GROUP ${projectName}
                )

                cpack_add_component(
                    Documentation
                    DISPLAY_NAME Documentation
                    GROUP ${projectName}
                )
            endif( NOT CPACK_MONOLITHIC_INSTALL )

        endif ( NOT LE_BUILD_SUBPROJECT_BUILD )
    endif( LE_CREATE_PACKAGE_TARGET )

endfunction( createSDKProjectAux )


################################################################################
#   appendPropertyToAllArchitectures()
################################################################################
# http://www.cmake.org/pipermail/cmake/2010-September/039388.html
#...mrmlj...might not work for "development" targets...
function( appendPropertyToAllArchitectures target property value )
    if ( NOT value )
        return()
    endif()
    foreach( existingTarget ${leSDKTargets} )
        if ( existingTarget MATCHES ${target} )
            appendProperty( ${existingTarget} ${property} ${value} )
        endif()
    endforeach()
endfunction()


################################################################################
#   setupSDKTool()
################################################################################

function( setupSDKTool targetName )

    include( "${CMAKE_SOURCE_DIR}/${leExternals}/build/buildOptions.cmake" ) #...mrmlj...for disableAsserts

    LE_setFinalFlags()
    appendProperty( ${targetName} COMPILE_FLAGS         ${exceptionsOnSwitch} ${rttiOffSwitch}                               )
    appendProperty( ${targetName} COMPILE_FLAGS_RELEASE ${speedOptimizationSwitch} ${vectorizeOnSwitch} ${ltoCompilerSwitch} )
    appendProperty( ${targetName} LINK_FLAGS_RELEASE    ${ltoLinkerSwitch}                                                   )

    set_property( TARGET ${targetName} PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Release>:${disableAsserts}> )

    addPrefixHeader       ( ${targetName} "${PROJECT_SOURCE_DIR}/externals/le/build/leConfigurationAndODRHeader.h" )
    setupTargetForPlatform( ${targetName} ${LE_TARGET_ARCHITECTURE} )

    if ( NOT setupSDKToolSkipSDK ) #...mrmlj...quick-hack for tools which do not use the SDK but the SDK uses them (e.g. resourceEmbedder in Melodify) to prevent circular dependencies...
        add_dependencies     ( ${targetName} ${defaultArchSDKTarget} )
        target_link_libraries( ${targetName} ${defaultArchSDKTarget} )
    endif()

    set( utilityLibTarget LE_Utility_${LEB_OS_SUFFIX}_${LE_TARGET_ARCHITECTURE_SUFFIX} )
    if ( TARGET ${utilityLibTarget} )
        add_dependencies     ( ${targetName} ${utilityLibTarget} )
        target_link_libraries( ${targetName} ${utilityLibTarget} )
    endif()

    if ( APPLE )
        set_target_properties(
            ${targetName} PROPERTIES
            OSX_ARCHITECTURES                        x86_64
            XCODE_ATTRIBUTE_ARCHS                    x86_64
            XCODE_ATTRIBUTE_VALID_ARCHS              x86_64
            XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH         YES
            XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET 10.11
            XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY        libc++
        )
        target_link_libraries( ${targetName} "-framework Accelerate -framework CoreServices -framework Foundation" )
        if ( iOS )
            set_target_properties( ${targetName} PROPERTIES
               #RESOURCE                             "${iOSResources}"
                XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY   "iPhone Developer"
                XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS "YES"
                XCODE_ATTRIBUTE_INSTALL_PATH         "/Applications"
                MACOSX_BUNDLE_GUI_IDENTIFIER         "com.littleendian.${targetName}"
                #...mrmlj...http://www.cmake.org/Wiki/CMake:OSX_InterfaceBuilderFiles
                #MACOSX_BUNDLE_NSMAIN_NIB_FILE        "guiController"
                #MACOSX_BUNDLE_INFO_STRING
                #MACOSX_BUNDLE_ICON_FILE              "Icon~iphone.png"
                #MACOSX_BUNDLE_LONG_VERSION_STRING
                #MACOSX_BUNDLE_BUNDLE_NAME
                #MACOSX_BUNDLE_SHORT_VERSION_STRING
                #MACOSX_BUNDLE_BUNDLE_VERSION
                #MACOSX_BUNDLE_COPYRIGHT
            )
        endif()
    endif()
endfunction()
