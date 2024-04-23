################################################################################
#
# LE utility CMake functionality
#
# Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
#
################################################################################

cmake_minimum_required( VERSION 2.8.12 )


################################################################################
#   forwardToParentScope()
################################################################################

macro( forwardToParentScope variableName )
    set( ${variableName} "${${variableName}}" PARENT_SCOPE )
endmacro( forwardToParentScope )


################################################################################
#   setAndForwardToParentScope()
################################################################################

macro( setAndForwardToParentScope variableName )
    unset( setAndForwardToParentScope_local_temp_var )
    foreach( argument ${ARGN} )
        string( STRIP "${setAndForwardToParentScope_local_temp_var} ${argument}" setAndForwardToParentScope_local_temp_var_value )
        set( setAndForwardToParentScope_local_temp_var ${setAndForwardToParentScope_local_temp_var_value} )
    endforeach()
   #set( ${variableName} ${setAndForwardToParentScope_local_temp_var} CACHE STRING "" FORCE ) ...mrmlj...
    set( ${variableName} ${setAndForwardToParentScope_local_temp_var} )
    set( ${variableName} "${${variableName}}" PARENT_SCOPE )
    unset( setAndForwardToParentScope_local_temp_var       )
    unset( setAndForwardToParentScope_local_temp_var_value )
endmacro( setAndForwardToParentScope )


################################################################################
# LE_printVariable()
################################################################################

function( LE_printVariable variableName )
    message( STATUS "${variableName} = ${${variableName}}" )
endfunction()


################################################################################
# LE_capitaliseFirstLetter()
################################################################################

function( LE_capitaliseFirstLetter variableName )
    set( stringValue ${${variableName}} )
    string( SUBSTRING ${stringValue} 0 1               stringValueCapitalFirstLetter )
    string( TOUPPER   ${stringValueCapitalFirstLetter} stringValueCapitalFirstLetter )
    string( LENGTH    ${stringValue}                   stringValueSuffixLength       )
    math  ( EXPR      stringValueSuffixLength "${stringValueSuffixLength} - 1"       )
    string( SUBSTRING ${stringValue} 1 ${stringValueSuffixLength} stringValueSuffix )
    set   ( capitalizedStringValue ${stringValueCapitalFirstLetter}${stringValueSuffix} )
    set   ( ${variableName}        ${capitalizedStringValue} PARENT_SCOPE )
endfunction()


################################################################################
# LE_lowerFirstLetter()
################################################################################

function( LE_lowerFirstLetter variableName )
    set( stringValue ${${variableName}} )
    string( SUBSTRING ${stringValue} 0 1             stringValueLowerFirstLetter )
    string( TOLOWER   ${stringValueLowerFirstLetter} stringValueLowerFirstLetter )
    string( LENGTH    ${stringValue}                 stringValueSuffixLength     )
    math  ( EXPR      stringValueSuffixLength "${stringValueSuffixLength} - 1"   )
    string( SUBSTRING ${stringValue} 1 ${stringValueSuffixLength} stringValueSuffix )
    set   ( loweredStringValue ${stringValueLowerFirstLetter}${stringValueSuffix} )
    set   ( ${variableName}    ${loweredStringValue} PARENT_SCOPE )
endfunction()


################################################################################
# LE_configureFeatureOption()
################################################################################

function( LE_configureFeatureOption variableName )
    mark_as_advanced( ${variableName} )
    if ( ${variableName} )
        #add_definitions( -D${variableName}=1 )
        set_property(        DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${variableName}=1 )
        get_property( parent DIRECTORY        PROPERTY PARENT_DIRECTORY                      )
        while ( parent )
            set_property(        DIRECTORY ${parent} APPEND PROPERTY COMPILE_DEFINITIONS ${variableName}=1 )
            get_property( parent DIRECTORY ${parent}        PROPERTY PARENT_DIRECTORY                      )
        endwhile()
    endif()
endfunction()


################################################################################
# LE_makeTempPath()
################################################################################

function( LE_makeTempPath fileNameVariable )
    if    ( EXISTS $ENV{TEMP} )
        set( tempDir "$ENV{TEMP}" )
    elseif( EXISTS $ENV{TMP} )
        set( tempDir "$ENV{TMP}" )
    elseif( EXISTS $ENV{TMPDIR} )
        set( tempDir "$ENV{TMPDIR}" )
    else()
        set( tempDir "${PROJECT_BINARY_DIR}" )
    endif()
    set( fileName ${${fileNameVariable}} )
    set( ${fileNameVariable} "${tempDir}/${fileName}" PARENT_SCOPE )
endfunction()


################################################################################
# LE_addArchSubproject()
################################################################################

function( LE_addArchSubproject projectName tag generator )
    set( subProjectBinaryDir "${CMAKE_BINARY_DIR}/${tag}" ) #CACHE INTERNAL "Directory that contains the cache of the subproject" )
    message( STATUS "[LEB] Creating the separate * ${tag} * build tree in \"${subProjectBinaryDir}\":" )

    # Implementation note:
    # Allow the subproject to reuse the generated NT2 include hierarchy to speed
    # up the (build files generation) process.
    #                                         (25.09.2015.) (Domagoj Saric)
    if ( NT2_INCLUDE_DIR )
        set( reusableNT2 "-DNT2_INCLUDE_DIR=${NT2_INCLUDE_DIR}" )
    endif()

    execute_process( COMMAND "${CMAKE_COMMAND}" -E make_directory "${subProjectBinaryDir}" )
    execute_process(
        COMMAND "${CMAKE_COMMAND}"
            -G ${generator}
            -DLE_BUILD_SUBPROJECT_BUILD:BOOL=true
            -DLE_BUILD_SUBPROJECT_TAG:STRING=${tag}
            ${ARGN}
            "${reusableNT2}"
            "${CMAKE_SOURCE_DIR}"
        WORKING_DIRECTORY "${subProjectBinaryDir}"
        ERROR_VARIABLE    errorOutput
        OUTPUT_VARIABLE   standardOutput
    )
    message( STATUS ${standardOutput} )
    if ( errorOutput )
        message( FATAL_ERROR "[LEB] ${tag} build tree creation failure:\n${errorOutput}" )
    endif()
    if ( generator MATCHES Ninja )
        # Implementation note: limit/decrease the level of build parallelism to
        # match the number of cores (Ninja's N+2 logic only hogs the build
        # machine).
        # Merely appending -j ${cpuCount} to the cmake --build command (after a
        # -- delimiter) didn't work when executed from Visual Studio (CMake
        # complained that it does not recognize the -j option).
        #                                     (28.02.2014.) (Domagoj Saric)
        include( ProcessorCount )
        ProcessorCount( cpuCount )
        set( buildCommand ninja -C "${subProjectBinaryDir}" -j ${cpuCount} )
    else()
        set( buildCommand "${CMAKE_COMMAND}" --build "${subProjectBinaryDir}" --config "${CMAKE_CFG_INTDIR}" )
    endif()
    # Implementation note: For VS and projects which only generate a single
    # target, try to guess the vcxproj name and include it directly instead of
    # through the opaque CMake custom target (rethink this through: we might
    # end up 'guessing' and including a single .vcxproj even though many are
    # generated).
    #                                         (12.11.2015.) (Domagoj Saric)
    set( possibleVSSubproject "${subProjectBinaryDir}/${projectName}_${tag}.vcxproj" )
    if ( CMAKE_GENERATOR MATCHES Visual AND generator MATCHES Visual AND EXISTS "${possibleVSSubproject}" )
        include_external_msproject( ${projectName}_${tag} "${possibleVSSubproject}" PLATFORM Win32 )
    else()
        add_custom_target( ${projectName}_${tag} ALL
            COMMAND ${buildCommand}
            COMMENT "Building ${projectName} for ${tag}..."
            VERBATIM
        )
    endif()
    list( APPEND subProjectBinaryDirs ${subProjectBinaryDir} )
    forwardToParentScope( subProjectBinaryDirs )
endfunction()


################################################################################
#   createUnityBuildCPP()
################################################################################

function( createUnityBuildCPP sources )
    # Implementation note:
    # We use unity builds for static library projects to emulate "plain static
    # libs internally built with LTO" until MSVC and Clang offer such
    # functionality.
    # http://cheind.wordpress.com/2009/12/10/reducing-compilation-time-unity-builds
    #                                         (08.02.2012.) (Domagoj Saric)

    # Generate a unique filename for the unity build translation unit
    # Implementation note:
    # If the staticLibUnityBuildCPPFile variable is not first "created" without
    # PARENT_SCOPE it doesn't seem to be set/visible in this local scope.
    #                                         (30.08.2012.) (Domagoj Saric)
    set( staticLibUnityBuildCPPFile "${CMAKE_CURRENT_BINARY_DIR}/unityBuild.cpp"              )
    set( staticLibUnityBuildCPPFile "${CMAKE_CURRENT_BINARY_DIR}/unityBuild.cpp" PARENT_SCOPE )
    # Open the unity build file
    FILE( WRITE ${staticLibUnityBuildCPPFile} "// Unity Build generated by CMake\n" )
    # Add an include statement for each translation unit
    list( REMOVE_DUPLICATES sources )
    list( SORT              sources )
    foreach( source_file ${sources} )
        get_source_file_property( isHeaderOnly ${source_file} HEADER_FILE_ONLY )
        if ( NOT isHeaderOnly )
            string( FIND ${source_file} ".in"  inLocation           )
            string( FIND ${source_file} ".cpp" cppExtensionLocation )
            string( FIND ${source_file} ".mm"  mmExtensionLocation  )
            if ( ( cppExtensionLocation GREATER -1 OR mmExtensionLocation GREATER -1 ) AND inLocation EQUAL -1 )
                # get_filename_component( absolutePath "${source_file}" ABSOLUTE )
                if ( NOT IS_ABSOLUTE ${source_file} )
                    set( sourceAbsolutePath "${CMAKE_CURRENT_SOURCE_DIR}/${source_file}" )
                else()
                    set( sourceAbsolutePath "${source_file}" )
                endif()
                FILE( APPEND ${staticLibUnityBuildCPPFile} "#include \"${sourceAbsolutePath}\"\n" )
            endif()
        endif()
    endforeach( source_file )
    appendSourceFileFlags( "${staticLibUnityBuildCPPFile}" COMPILE_FLAGS "-DLE_UNITY_BUILD" )
endfunction()
