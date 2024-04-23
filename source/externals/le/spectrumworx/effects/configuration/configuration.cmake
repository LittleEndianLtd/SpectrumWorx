################################################################################
#
# LEL.Effects configuration.cmake
#
# Copyright (c) 2012 - 2016. Little Endian Ltd. All rights reserved.
#
################################################################################

set( LE_SW_PV_TSS false CACHE BOOL "enable phase vocoder transient/steady state separation" )
mark_as_advanced( LE_SW_PV_TSS )
if ( LE_SW_PV_TSS )
    add_definitions( -DLE_PV_USE_TSS )
endif()

#...mrmlj...assumes location of effects...
set( effectConfigurationPath externals/le/spectrumworx/effects/configuration )

set( SOURCES_Effects_Configuration
    ${effectConfigurationPath}/allEffectImpls.hpp
    ${effectConfigurationPath}/allEffectImpls.hpp.in
    ${effectConfigurationPath}/allEffects.hpp
    ${effectConfigurationPath}/allEffects.hpp.in
    ${effectConfigurationPath}/configuration.cmake
    ${effectConfigurationPath}/effectsList.cmake
    ${effectConfigurationPath}/effectNames.cpp
    ${effectConfigurationPath}/effectNames.cpp.in
    ${effectConfigurationPath}/effectNames.hpp
    ${effectConfigurationPath}/effectTypeNames.cpp
    ${effectConfigurationPath}/effectTypeNames.cpp.in
    ${effectConfigurationPath}/effectTypeNames.hpp
    ${effectConfigurationPath}/effectGroups.hpp
    ${effectConfigurationPath}/effectIndexToGroupMapping.hpp
    ${effectConfigurationPath}/effectIndexToGroupMapping.hpp.in
    ${effectConfigurationPath}/constants.hpp
    ${effectConfigurationPath}/constants.hpp.in
    ${effectConfigurationPath}/includedEffects.cpp
    ${effectConfigurationPath}/includedEffects.cpp.in
    ${effectConfigurationPath}/includedEffects.hpp
    ${effectConfigurationPath}/includedEffects.hpp.in
    ${effectConfigurationPath}/indexToEffectImplMapping.hpp
    ${effectConfigurationPath}/indexToEffectImplMapping.hpp.in
)
#...mrmlj...assumes location of effects...
source_group( "Externals\\Effects\\Configuration" FILES ${SOURCES_Effects_Configuration} )

function( lel_effects_configure_sources )

    set( effectConfigurationPath "${CMAKE_SOURCE_DIR}/${effectConfigurationPath}" )

    configure_file(
        "${effectConfigurationPath}/allEffects.hpp.in"
        "${effectConfigurationPath}/allEffects.hpp"
    )

    configure_file(
        "${effectConfigurationPath}/allEffectImpls.hpp.in"
        "${effectConfigurationPath}/allEffectImpls.hpp"
    )

    configure_file(
        "${effectConfigurationPath}/constants.hpp.in"
        "${effectConfigurationPath}/constants.hpp"
    )

    configure_file(
        "${effectConfigurationPath}/effectIndexToGroupMapping.hpp.in"
        "${effectConfigurationPath}/effectIndexToGroupMapping.hpp"
    )

    configure_file(
        "${effectConfigurationPath}/includedEffects.hpp.in"
        "${effectConfigurationPath}/includedEffects.hpp"
    )

    configure_file(
        "${effectConfigurationPath}/includedEffects.cpp.in"
        "${effectConfigurationPath}/includedEffects.cpp"
    )

    configure_file(
        "${effectConfigurationPath}/indexToEffectImplMapping.hpp.in"
        "${effectConfigurationPath}/indexToEffectImplMapping.hpp"
    )

    configure_file(
        "${effectConfigurationPath}/effectNames.cpp.in"
        "${effectConfigurationPath}/effectNames.cpp"
    )

    configure_file(
        "${effectConfigurationPath}/effectTypeNames.cpp.in"
        "${effectConfigurationPath}/effectTypeNames.cpp"
    )

endfunction( lel_effects_configure_sources )
