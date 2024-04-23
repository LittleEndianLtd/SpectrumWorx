################################################################################
#
# Utility functions for downloading and configuring 3rd party libraries.
#
# Copyright (c) 2014 - 2016. Little Endian Ltd. All rights reserved.
#
################################################################################

cmake_minimum_required( VERSION 2.8.12 )

include( "${CMAKE_CURRENT_LIST_DIR}/utilities.cmake" )

function( add3rdPartyLib name minimumVersion downloadVersion downloadURL downloadedFileName downloadHash versionSubDirectory )

    set( nameLower   ${name} )
    set( nameCapital ${name} )
    LE_lowerFirstLetter     ( nameLower   )
    LE_capitaliseFirstLetter( nameCapital )

    string( TOUPPER ${name}_root libRootVar )

    # http://www.cmake.org/Wiki/CMake:How_To_Find_Libraries
    # http://www.cmake.org/cmake/help/v2.8.12/cmake.html#command:find_package
    # https://github.com/maidsafe/MaidSafe/blob/master/cmake_modules/add_boost.cmake

    if ( NOT DEFINED LE_3rdParty_root )
        set( LE_3rdParty_root "$ENV{LEB_3rdParty_root}" CACHE FILE "root path for 3rd party libraries" )
    endif()
    if ( NOT LE_3rdParty_root )
        set( LE_3rdParty_root "${PROJECT_SOURCE_DIR}/3rdParty" )
    endif()

    # http://www.cmake.org/pipermail/cmake/2007-January/012519.html
    find_file( findModule "Find${nameCapital}.cmake" "${CMAKE_ROOT}/Modules" ${CMAKE_MODULE_PATH} )

    if ( EXISTS "${${name}_INCLUDE_DIR}" )
        # If there is an existing version check if it qualifies...
        string( REPLACE "." "_" versionWithUnderscores "${downloadVersion}" )
        if ( NOT ( ${name}_INCLUDE_DIR MATCHES ${downloadVersion} OR ${name}_INCLUDE_DIR MATCHES ${versionWithUnderscores} OR (versionSubDirectory AND ${name}_INCLUDE_DIR MATCHES "${versionSubDirectory}" ) ) )
            unset( ${name}_INCLUDE_DIR CACHE )
        endif()
    endif()

    if ( NOT EXISTS "${${name}_INCLUDE_DIR}" )
        unset( ${name}_INCLUDE_DIR CACHE )
        if ( findModule )
            find_package( ${name} ${minimumVersion} QUIET )
            if ( NOT BOOST_FOUND )
                unset( ${name}_INCLUDE_DIR CACHE )
            endif()
        endif()
    endif()
    if ( NOT EXISTS "${${name}_INCLUDE_DIR}" AND findModule )
        message( STATUS "[LEB] ${name} ${minimumVersion}+ not found in standard locations, searching in path pointed to by the LE_3rdParty_root CMake variable." )
        set( ENV{${libRootVar}} "${LE_3rdParty_root}/${name}" )
        set( ENV{${name}_DIR}   "${LE_3rdParty_root}/${name}" )
        find_package( ${name} ${minimumVersion} QUIET )
    endif()
    if ( NOT EXISTS ${${name}_INCLUDE_DIR} )
        set( unpackDirectory "${LE_3rdParty_root}/${name}" )
        if ( NOT versionSubDirectory AND NOT findModule )
            set( unpackDirectory  "${unpackDirectory}/${downloadVersion}" )
            set( installDirectory "${unpackDirectory}/${downloadVersion}" )
        elseif( versionSubDirectory )
            set( installDirectory "${unpackDirectory}/${versionSubDirectory}" )
        else()
            set( installDirectory "${unpackDirectory}" )
        endif()
        set( ${name}_INCLUDE_DIR  "${installDirectory}" CACHE FILE "Location of ${name} public headers" )
    endif()
    if ( NOT EXISTS ${${name}_INCLUDE_DIR} )
        LE_makeTempPath( downloadedFileName )
        if ( downloadHash )
            set( expectedHash "EXPECTED_HASH MD5=${downloadHash}" )
        endif()
        message( STATUS "[LEB] Downloading ${nameCapital}  ${downloadVersion} to ${downloadedFileName}" )
        file( DOWNLOAD
            ${downloadURL}
            ${downloadedFileName}
            STATUS success
            SHOW_PROGRESS
            ${expectedHash}
        )
        list( GET success 0 success )
        if ( NOT success EQUAL 0 )
            message( FATAL_ERROR "[LEB] Error downloading ${name}" )
        endif()
        message( STATUS "[LEB] Extracting ${nameCapital} ${downloadVersion} to ${installDirectory}" )
        file( MAKE_DIRECTORY "${unpackDirectory}" )
        execute_process( COMMAND ${CMAKE_COMMAND} -E tar xzf
            "${downloadedFileName}"
            WORKING_DIRECTORY "${unpackDirectory}"
            RESULT_VARIABLE success
            OUTPUT_VARIABLE output
            ERROR_VARIABLE  output
        )
        if ( NOT success EQUAL 0 )
            message( FATAL_ERROR "[LEB] Error extracting ${name} (${output})." )
        endif()

        if ( findModule )
            set( ${libRootVar} "${installDirectory}" )
            find_package( ${name} ${downloadVersion} EXACT REQUIRED )
        else()
            set( ${name}_INCLUDE_DIR "${installDirectory}" CACHE FILE "Location of ${name} public headers" )
        endif()
    endif()
    if ( NOT EXISTS ${${name}_INCLUDE_DIR} )
        message( FATAL_ERROR "[LEB] Failed to find or install ${name}." )
    endif()
    
    unset( findModule CACHE )

    include_directories( ${${name}_INCLUDE_DIR} )

endfunction()
