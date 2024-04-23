################################################################################
#
# SpectrumWorx configuration.cmake
#
# Copyright (c) 2012 - 2015. Little Endian Ltd. All rights reserved.
#
################################################################################

set( leExternals "${PROJECT_SOURCE_DIR}/externals/le" )

include( "${leExternals}/build/buildOptions.cmake" )


############################################################################
# Version configuration
############################################################################

set( versionMajor       "3" )
set( versionMinor       "0" )
set( versionPatch       "0" )
set( versionDescription "development" )

set( versionString "${versionMajor}.${versionMinor}.${versionPatch}" )
if ( versionDescription )
    set( fullVersionString "${versionString} ${versionDescription}" )
else()
    set( fullVersionString "${versionString}" )
endif()

if ( versionDescription )
    set( retailBuild 0 )
else()
    set( retailBuild 1 )
endif()

set( versionUpgradeEnabled 0 )

configure_file(
    "${PROJECT_SOURCE_DIR}/configuration/versionConfiguration.hpp.in"
    "${PROJECT_SOURCE_DIR}/configuration/versionConfiguration.hpp"
)


################################################################################
# Features configuration
################################################################################

set( LE_SW_GUI                    true  CACHE BOOL "GUI interface"                              )
set( LE_SW_SEPARATED_DSP_GUI      false CACHE BOOL "separated DSP and GUI instances"            )
set( LE_SW_PRESETS                true  CACHE BOOL "persistent, filesystem based presets"       )
set( LE_SW_PROGRAMS               true  CACHE BOOL "temporary in-memory presets (VST programs)" )
set( LE_SW_AUTHORISATION_REQUIRED true  CACHE BOOL "licence based copy-protection system"       )

LE_configureFeatureOption( LE_SW_GUI                    )
LE_configureFeatureOption( LE_SW_SEPARATED_DSP_GUI      )
LE_configureFeatureOption( LE_SW_PRESETS                )
LE_configureFeatureOption( LE_SW_PROGRAMS               )
LE_configureFeatureOption( LE_SW_AUTHORISATION_REQUIRED )

include( ${leExternals}/spectrumworx/engine/configuration.cmake )


################################################################################
# Source files setup
################################################################################

# Implementation note: SW plugin uses the old/'simple' version of TuneWorx.
#                                             (07.06.2016.) (Domagoj Saric)
add_definitions( -DLE_SIMPLE_TUNEWORX )

include( "${leExternals}/spectrumworx/effects/configuration/effectsList.cmake" )

addSelectedEffects( "${LE_SW_INCLUDED_EFFECTS}" )

# Implementation note:
#   Has to be included after the SOURCES_Externals__Effects variable has been
# filled.
#                                             (27.02.2012.) (Domagoj Saric)
include( ${PROJECT_SOURCE_DIR}/core/sources.cmake )


############################################################################
# Packaging/installation configuration
############################################################################

set ( sharedPermissions OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ WORLD_WRITE )

#...mrmlj...cleanup these intermingled CMake/install and CPack directives...
set( CPACK_TOPLEVEL_TAG "Little Endian" )

set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "The ultimate sound mangler!" )
set( CPACK_PACKAGE_VENDOR              "Little Endian Ltd."          )
set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "Will be installed..."        )

set( CPACK_PACKAGE_VERSION_MAJOR     ${versionMajor} )
set( CPACK_PACKAGE_VERSION_MINOR     ${versionMinor} )
set( CPACK_PACKAGE_VERSION_PATCH     ${versionPatch} )
set( CPACK_PACKAGE_VERSION           ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH} )
set( CPACK_PACKAGE_INSTALL_DIRECTORY ${projectName}  )
#...zzz...CPack "working directory", command line -B option
#set( CPACK_PACKAGE_DIRECTORY         ${projectName}  )

set( CPACK_PACKAGE_FILE_NAME "${projectName}-${versionMajor}.${versionMinor}.${versionPatch}-" )
if ( versionDescription )
    set( CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}${versionDescription}-" )
endif()


set( CPACK_PACKAGE_RELOCATABLE ON )

set( CPACK_COMPONENT_UNSPECIFIED_HIDDEN OFF )
#set( CPACK_COMPONENT_UNSPECIFIED_REQUIRED TRUE )

#CPACK_INSTALLED_DIRECTORIES
#CPACK_REMOVE_TOPLEVEL_DIRECTORY
#CMAKE_INSTALL_COMPONENT
#CPACK_ABSOLUTE_DESTINATION_FILES
#CPACK_ADD_REMOVE
#CPACK_PACKAGEMAKER_CHOICES


############################################################################


include_directories(
    BEFORE #...mrmlj...it seems that the directories get added in the reverse order...
    "$ENV{LEB_3rdParty_root}/boost/${boostVersion}"
    "${PROJECT_SOURCE_DIR}/externals"
    ${PROJECT_SOURCE_DIR}
)


add_definitions(
    -DLEB_PRECOMPILE_OS_HEADERS
    -DBOOST_SWITCH_LIMIT=60
    -DFUSION_MAX_VECTOR_SIZE=20
    -DLE_PARAMETERS_NO_CLASSIC_ACCESSORS
)


#...mrmlj...see the note in buildOptions.cmake...
set_property( DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Release>:${disableAsserts}> )
