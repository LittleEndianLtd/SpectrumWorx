################################################################################
#
# LEL.Engine configuration.cmake
#
# Copyright (c) 2014 - 2015. Little Endian Ltd. All rights reserved.
#
################################################################################

set( LE_SW_ENGINE_SIDE_CHANNEL true CACHE BOOL "include side chaining functionality" )
#...mrmlj...legacy LE_SW_DISABLE_SIDE_CHANNEL variable...
#LE_configureFeatureOption( LE_SW_ENGINE_SIDE_CHANNEL )
mark_as_advanced( LE_SW_ENGINE_SIDE_CHANNEL )
if ( NOT LE_SW_ENGINE_SIDE_CHANNEL )
    add_definitions( -DLE_SW_DISABLE_SIDE_CHANNEL )
endif()

set( LE_SW_ENGINE_INPUT_MODE full CACHE STRING "I/O mode configurability" )
set_property( CACHE LE_SW_ENGINE_INPUT_MODE PROPERTY STRINGS full read-only disabled )
mark_as_advanced( LE_SW_ENGINE_INPUT_MODE )
if ( LE_SW_ENGINE_INPUT_MODE STREQUAL full )
    add_definitions( -DLE_SW_ENGINE_INPUT_MODE=2 )
elseif()
    add_definitions( -DLE_SW_ENGINE_INPUT_MODE=1 )
else()
    add_definitions( -DLE_SW_ENGINE_INPUT_MODE=0 )
endif()

set( LE_SW_ENGINE_WINDOW_PRESUM false CACHE BOOL "enable \"window presum\" capability" )
LE_configureFeatureOption( LE_SW_ENGINE_WINDOW_PRESUM )
