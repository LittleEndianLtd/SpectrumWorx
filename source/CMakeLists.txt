################################################################################
#
# SpectrumWorx main build file
#
# Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
#
################################################################################


cmake_minimum_required( VERSION 3.5 )

# http://www.cmake.org/cmake/help/v3.0/policy/CMP0043.html
cmake_policy( SET CMP0043 OLD )

set( LE_SW_EDITION_SUFFIX         ""    CACHE STRING "SW project name suffix"              )
set( LE_SW_INCLUDED_EFFECTS       "all" CACHE STRING "list of included effects (or 'all')" )
set( LE_SW_FMOD                   false CACHE BOOL   "create FMOD Studio projects"         )
set( LE_SW_COMPILE_TIME_PROFILING false CACHE BOOL   "add compile time profiling targets"  )
mark_as_advanced( LE_SW_COMPILE_TIME_PROFILING )

set( LE_PROJECT_NAME       "SpectrumWorx"                   )
set( LE_PROJECT_LABEL      "SpectrumWorx"                   )

set( LE_FMOD_PROJECT_NAME  "fmod_littleendian_spectrumworx" )
set( LE_FMOD_PROJECT_LABEL "SpectrumWorx FMOD"              )

if ( LE_SW_FMOD AND FALSE ) #...mrmlj...in the initial development and testing phase we use all effects...
    include( fmod_lib/effectList.cmake )
    set( LE_SW_INCLUDED_EFFECTS ${fmodEffectList} CACHE STRING "Fixed set of 'essential' effects for FMOD" FORCE )
    set( LE_SW_EDITION_SUFFIX   "Essential"       CACHE STRING "SW project name suffix"                    FORCE )
endif()

if ( LE_SW_EDITION_SUFFIX )
    set( LE_PROJECT_NAME       "${LE_PROJECT_NAME}${LE_SW_EDITION_SUFFIX}"   )
    set( LE_PROJECT_LABEL      "${LE_PROJECT_LABEL} ${LE_SW_EDITION_SUFFIX}" )

    string( TOLOWER ${LE_SW_EDITION_SUFFIX} LE_SW_EDITION_SUFFIX_LOWERCASE )
    set( LE_FMOD_PROJECT_NAME  "${LE_FMOD_PROJECT_NAME}_${LE_SW_EDITION_SUFFIX_LOWERCASE}" )
    set( LE_FMOD_PROJECT_LABEL "${LE_FMOD_PROJECT_LABEL} ${LE_SW_EDITION_SUFFIX}" )

    set( editionString " ${LE_SW_EDITION_SUFFIX}" )
endif()

set( projectName ${LE_PROJECT_LABEL} )

project( ${LE_PROJECT_NAME} )

include( ${CMAKE_SOURCE_DIR}/core/configuration.cmake )

include( "$ENV{LEB_3rdParty_root}/juce/trunk/juce.cmake" )

############################################################################
# Source files setup
############################################################################

set( leExternals "externals/le" )

set( SOURCES_Sources
    spectrumWorx.cpp
    spectrumWorxVST24.cpp
)
if ( APPLE )
    list( APPEND SOURCES_Sources spectrumWorxAU.cpp )
endif()
if ( MSVC )
    list( APPEND SOURCES_Sources debugConsole.cpp )
endif()
source_group( "Sources" FILES ${SOURCES_Sources} )

set( SOURCES_Headers
    spectrumWorx.hpp
    spectrumWorxVST24.hpp
)
if ( APPLE )
    list( APPEND SOURCES_Headers spectrumWorxAU.hpp )
endif()
source_group( "Headers" FILES ${SOURCES_Headers} )

set( SOURCES_GUI
    gui/juceLexicalCast.cpp
    gui/gui.cpp
    gui/gui.hpp
    gui/editor/moduleMenuHolder.cpp
    gui/editor/moduleMenuHolder.hpp
    gui/editor/spectrumWorxEditor.cpp
    gui/editor/spectrumWorxEditor.hpp
    gui/editor/auxiliaryComponents.cpp
    gui/editor/auxiliaryComponents.hpp
    gui/modules/moduleUI.cpp
    gui/modules/moduleUI.hpp
    gui/modules/moduleControl.cpp
    gui/modules/moduleControl.hpp
    gui/preset_browser/presetBrowser.cpp
    gui/preset_browser/presetBrowser.hpp
)
if( APPLE )
    list( APPEND SOURCES_GUI gui/gui.mm )
endif()
source_group( "GUI" FILES ${SOURCES_GUI} )

set( SOURCES_ExternalAudio
    ${CMAKE_SOURCE_DIR}/external_audio/sample.cpp
    ${CMAKE_SOURCE_DIR}/external_audio/sample.hpp
)
if ( MSVC )
    list( APPEND SOURCES_ExternalAudio ${CMAKE_SOURCE_DIR}/external_audio/sampleWin.cpp )
elseif( APPLE )
    list( APPEND SOURCES_ExternalAudio ${CMAKE_SOURCE_DIR}/external_audio/sampleMac.cpp )
endif()
source_group( "ExternalAudio" FILES ${SOURCES_ExternalAudio} )

if ( MSVC )
    set(SOURCES_Externals__LEB
        ${SOURCES_Externals__LEB}
        ${leExternals}/build/precompiledHeaders.cpp
        ${leExternals}/build/precompiledHeaders.hpp
    )
endif()
source_group("Externals\\LEB" FILES ${SOURCES_Externals__LEB})

# Implementation note: adding/'integrating' the Licenser SDK 'manually'/directly
# (as a first step) rather than building it as a separate project/library and
# linking with it (similarly to how this project currently uses the Utility
# library. To be cleaned up...
#                                         (25.04.2016.) (Domagoj Saric)
add_definitions( -DLE_SDK_NO_AUTO_LINK )
set(SOURCES_Externals__License
    ${leExternals}/licenser/cryptography/base64.cpp
    ${leExternals}/licenser/cryptography/hashing.cpp
    ${leExternals}/licenser/cryptography/key.cpp
    ${leExternals}/licenser/cryptography/signing.cpp
    ${leExternals}/licenser/cryptography/signing.hpp
    ${leExternals}/licenser/license/predefinedItems.hpp
    ${leExternals}/license_key/le_key_01_modulus.cpp
    ${leExternals}/license_key/le_key_01_modulus.hpp
)
source_group("Externals\\License" FILES ${SOURCES_Externals__License})

#source_group("Externals\\Plugins" FILES ${SOURCES_Externals__Plugins})
    set( SOURCES_Externals__Plugins__VST_24
      ${leExternals}/plugins/vst/2.4/plugin.cpp
      ${leExternals}/plugins/vst/2.4/plugin.hpp
      ${leExternals}/plugins/vst/2.4/plugin.inl
    )
    source_group("Externals\\Plugins\\VST\\2.4" FILES ${SOURCES_Externals__Plugins__VST_24})
    if ( APPLE )
        set( SOURCES_Externals__Plugins__AU
          ${leExternals}/plugins/au/gui.mm   #...mrmlj...Xcode fails to debug plugin.cpp if this source is named plugin.mm...
          ${leExternals}/plugins/au/plugin.cpp
          ${leExternals}/plugins/au/plugin.hpp
          ${leExternals}/plugins/au/plugin.inl
          ${leExternals}/plugins/au/properties.hpp
          ${leExternals}/plugins/au/resources.r
          ${leExternals}/plugins/au/resources.rsrc
        )
        source_group("Externals\\Plugins\\AU" FILES ${SOURCES_Externals__Plugins__AU})
    endif()


set( SOURCES_Externals
    ${SOURCES_Externals_Core}
    ${SOURCES_Externals__LEB}
    ${SOURCES_Externals__License}
    ${SOURCES_Externals__Plugins__VST_24}
)
if ( APPLE )
    list( APPEND SOURCES_Externals ${SOURCES_Externals__Plugins__AU} )
endif()

set( SOURCES
    ${SOURCES_Configuration}
    ${SOURCES_ExternalAudio}
    ${SOURCES_Externals}
    ${SOURCES_GUI}
    ${SOURCES_Core}
    ${SOURCES_Sources}
    ${SOURCES_Headers}
    #...mrmlj...
    core/modules/moduleDSPAndGUI.cpp
    core/modules/moduleDSPAndGUI.hpp
)


if ( APPLE )
    add_library( "${LE_PROJECT_NAME}" SHARED ${SOURCES} )
    # Note: For debugging with the auval tool straight from Xcode set:
    # - executable: arch
    # - arguments: (-i386) auvaltool (-comp) -v aufx SW00 LE00
    # http://developer.apple.com/library/mac/#technotes/tn2204/_index.html (Audio Unit Validation Using the auval Tool)
    #                                         (15.02.2013.) (Domagoj Saric)
else()
    add_library( "${LE_PROJECT_NAME}" MODULE ${SOURCES} )
endif()

set_property( TARGET "${LE_PROJECT_NAME}" PROPERTY PROJECT_LABEL "${LE_PROJECT_LABEL}" )

include_directories(
    AFTER
    "$ENV{LEB_3rdParty_root}/RapidXML/1.13"
)
add_definitions( -DLEB_PRECOMPILE_RapidXML )


if ( LE_SW_FMOD )
    set( fmodAdditionalSources
        core/modules/moduleGUI.cpp
        core/modules/moduleGUI.hpp
        debugConsole.cpp
        "fmod_lib/spectrumWorxEditorFMOD.cpp"
        ${SOURCES_GUI}
    )
    set( fmodStudioGUI true )
    include( fmod_lib/fmod_lib.cmake ) #add_subdirectory( fmod_lib ) ...mrmlj...see the note in fmod_lib.cmake
    unset( fmodStudioGUI )
endif()


# Implementation note:
#   Unfortunately Mac still requires RTTI because the
# juce::ComponentPeer::findCurrentTextInputTarget() gets called for all keyboard
# events and it relies on dynamic_cast which (with Clang/GCC) crashes for
# objects w/o RTTI records (e.g. when a SW control is active and a key is
# pressed it will try to dynamic_cast it to a juce::TextInputTarget).
#                                             (22.12.2011.) (Domagoj Saric)
if ( MSVC )
    set( releaseRTTISwitch ${rttiOffSwitch} )
else()
    set( releaseRTTISwitch ${rttiOnSwitch}  )
endif()
# Implementation note:
#   CMake does not (yet) support setting COMPILE_FLAGS_<CONFIG> on a per
# target basis so generator expressions have to be used instead.
# http://www.cmake.org/cmake/help/git-master/manual/cmake-generator-expressions.7.html#output-expressions
# http://public.kitware.com/Bug/view.php?id=6493 "configuration dependent COMPILE_FLAGS for SET_TARGET_PROPERTIES"
# http://public.kitware.com/Bug/view.php?id=6269 "Some CMake commands should support Debug/Release configurations"
#                                             (20.08.2013.) (Domagoj Saric)
# Implementation note:
#   The global variables have to be set before the per-project ones (with
# set_property), otherwise the per-project ones get overwritten.
#                                             (27.07.2011.) (Domagoj Saric)
if ( MSVC )
    set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /VERSION:${versionMajor}.${versionMinor}" )
endif()
set( LEB_sharedCReleaseFlags "${LEB_sharedCReleaseFlags} ${sizeOptimizationSwitch} ${vectorizeOnSwitch} ${releaseRTTISwitch}" CACHE INTERNAL "" FORCE )
LE_setFinalFlags()
appendProperty( ${LE_PROJECT_NAME} COMPILE_FLAGS      ${exceptionsOnSwitch} )
appendProperty( ${LE_PROJECT_NAME} LINK_FLAGS_RELEASE ${ltoLinkerSwitch}    )
target_compile_options( ${LE_PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:${rttiOnSwitch}> $<$<NOT:$<CONFIG:Debug>>:${ltoCompilerSwitch}> )

setupTargetForPlatform( ${LE_PROJECT_NAME} ${LE_TARGET_ARCHITECTURE} )

addJUCE( "${LE_PROJECT_NAME}" )
addNT2()
addPCH( "${LE_PROJECT_NAME}" ${leExternals}/build/precompiledHeaders )

if ( MSVC )

    if ( NOT CMAKE_GENERATOR MATCHES 64 )
        #...mrmlj...CMake still does not support COMPILE_FLAGS_RELEASE...
        set_source_files_properties( "${leExternals}/math/vector.cpp"  PROPERTIES COMPILE_FLAGS "/Y- /arch:SSE2 /Ot" )
        set_source_files_properties( "${leExternals}/math/dft/fft.cpp" PROPERTIES COMPILE_FLAGS "/Y- /arch:SSE2 /Ot" )
    endif()

    # Implementation note:
    # Enable debug information in release builds of development versions.
    #                                         (30.08.2011.) (Domagoj Saric)
    if ( versionDescription )
        appendProperty( ${LE_PROJECT_NAME} COMPILE_FLAGS      /Zi    )
        appendProperty( ${LE_PROJECT_NAME} LINK_FLAGS_RELEASE /DEBUG )
    endif()

    # Implementation note:
    # http://www.cmake.org/pipermail/cmake/2009-February/026858.html
    # http://www.mail-archive.com/cmake@cmake.org/msg14083.html
    #                                        (18.10.2010.) (Domagoj Saric)
    set_target_properties( ${LE_PROJECT_NAME} PROPERTIES PREFIX "../" )

    # http://forum.cockos.com/showthread.php?t=61288
    # http://www.kvraudio.com/forum/viewtopic.php?t=319194
    set( outputPath "$ENV{VST_PATH}" )
    if ( outputPath )
        file( TO_CMAKE_PATH "${outputPath}" outputPath )
    else()
        message( WARNING "VST_PATH environment variable not set" )
        set( outputPath "Common Files/VST2" )
    endif()
    if ( CMAKE_GENERATOR MATCHES 64 )
        string( REPLACE " (x86)" "" outputPath ${outputPath} )
    endif()
    set( outputPath "${outputPath}/${LE_PROJECT_NAME}" )

    #...mrmlj...testing/win installer development in progress...
    set( sharedDirectory "Little Endian/SpectrumWorx" )
    set( vstDirectory    "${outputPath}"              )

    set( defaultSWBinaryPath "${vstDirectory}" )

elseif ( APPLE )

    appendProperty( ${LE_PROJECT_NAME} COMPILE_FLAGS "${x86CompilerSwitches}" )

    set( CMAKE_XCODE_ATTRIBUTE_MACH_O_TYPE       "mh_dylib" CACHE STRING "" FORCE )
    set( CMAKE_XCODE_ATTRIBUTE_EXECUTABLE_PREFIX ""         CACHE STRING "" FORCE )
    set( CMAKE_SHARED_LIBRARY_PREFIX             ""         CACHE STRING "" FORCE )
    set_target_properties( ${LE_PROJECT_NAME} PROPERTIES PREFIX "" )

    set( vstDirectory    "Library/Audio/Plug-Ins/VST"                             )
    set( auDirectory     "Library/Audio/Plug-Ins/Components"                      )
    set( sharedDirectory "Library/Application Support/Little Endian/SpectrumWorx" )

    set( defaultSWBinaryPath /${sharedDirectory} )

    set( LE_SW_DEFAULT_SHARED_DIRECTORY  "/Library/Application Support/Little Endian/SpectrumWorx" )
    set( LE_SW_DEFAULT_PRESETS_DIRECTORY "${LE_SW_DEFAULT_SHARED_DIRECTORY}/Presets"               )

    # Compile the 'old style'/pre-Lion AU Carbon resources.
    # http://www.cmake.org/Wiki/CMake_FAQ#How_can_I_apply_resources_on_Mac_OS_X_automatically.3F
    # http://accountingplusplus.blogspot.com/2010/12/cmake-build-os-x-bundle.html
    # http://lists.apple.com/archives/coreaudio-api/2007/Sep/msg00120.html
    set( auComponentManagerResourceFile         "${CMAKE_SOURCE_DIR}/${leExternals}/plugins/au/resources.r"    )
    set( auComponentManagerCompiledResourceFile "${CMAKE_SOURCE_DIR}/${leExternals}/plugins/au/resources.rsrc" )

    configure_file(
        "${auComponentManagerResourceFile}.in"
        "${auComponentManagerResourceFile}"
        @ONLY
    )
    configure_file(
        "${CMAKE_SOURCE_DIR}/../installer/resources/OSX/SpectrumWorx.component/Contents/Info.plist.in"
        "${CMAKE_SOURCE_DIR}/../installer/resources/OSX/SpectrumWorx.component/Contents/Info.plist"
        @ONLY
    )
    configure_file(
        "${CMAKE_SOURCE_DIR}/../installer/resources/OSX/SpectrumWorx.vst/Contents/Info.plist.in"
        "${CMAKE_SOURCE_DIR}/../installer/resources/OSX/SpectrumWorx.vst/Contents/Info.plist"
        @ONLY
    )

    add_custom_command(
        OUTPUT  "${auComponentManagerCompiledResourceFile}"
        COMMAND Rez "${auComponentManagerResourceFile}" -d SystemSevenOrLater=1 -useDF -script Roman -d i386_YES -d x86_64_YES -isysroot $(SDKROOT) -o "${auComponentManagerCompiledResourceFile}"
        COMMAND mkdir -p "/${auDirectory}/${LE_PROJECT_NAME}.component/Contents/Resources"
        COMMAND cp "${auComponentManagerCompiledResourceFile}" "/${auDirectory}/${LE_PROJECT_NAME}.component/Contents/Resources/${LE_PROJECT_NAME}.rsrc"
        DEPENDS "${auComponentManagerResourceFile}"
    )

    # Create symlinks to/for the current build configuration
    add_custom_command(
        TARGET "${LE_PROJECT_NAME}"
        POST_BUILD
        COMMAND mkdir -p "/${auDirectory}/${LE_PROJECT_NAME}.component/Contents/MacOS"
        COMMAND mkdir -p "/${vstDirectory}/${LE_PROJECT_NAME}.vst/Contents/MacOS"
        COMMAND ln -s -f "/${sharedDirectory}/${CMAKE_CFG_INTDIR}/${LE_PROJECT_NAME}.dylib" "/${auDirectory}/${LE_PROJECT_NAME}.component/Contents/MacOS/${LE_PROJECT_NAME}"
        COMMAND ln -s -f "/${sharedDirectory}/${CMAKE_CFG_INTDIR}/${LE_PROJECT_NAME}.dylib" "/${vstDirectory}/${LE_PROJECT_NAME}.vst/Contents/MacOS/${LE_PROJECT_NAME}"
    )

endif()

set_target_properties( ${LE_PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${defaultSWBinaryPath}" )


if ( APPLE )
    target_link_libraries( ${LE_PROJECT_NAME}
        "-framework Accelerate"
        "-framework AudioToolbox"
        "-framework AudioUnit"
        "-framework CoreFoundation"
    )
endif()


if ( LE_SW_COMPILE_TIME_PROFILING )
    list( APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/externals/boost/profile_templates2" )
    include( "externals/boost/profile_templates2/profiler.cmake" )
    appendProperty( template.profiler COMPILE_FLAGS ${exceptionsOnSwitch} )
    template_profile( CompileTimeProfileSpectrumWorx SpectrumWorx.cpp               )
    template_profile( CompileTimeProfileNT2          ${leExternals}/math/vector.cpp )
endif()


################################################################################
#
# Installation and packaging
#
# http://www.cmake.org/Wiki/CMake:CPackConfiguration
#
# http://en.wikipedia.org/wiki/List_of_installation_software
#
################################################################################

if( LE_SW_FMOD )
    set( sharedDirectory "resources" )
endif()

install(
    DIRECTORY             "${CMAKE_SOURCE_DIR}/../installer/ProgramFolder/"
    DESTINATION           "${sharedDirectory}"
    COMPONENT             SupportFiles
    FILE_PERMISSIONS      ${sharedPermissions}
    DIRECTORY_PERMISSIONS ${sharedPermissions} OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE
    CONFIGURATIONS        Debug Release
    PATTERN               "*.svn"     EXCLUDE
    PATTERN               ".DS_store" EXCLUDE
    PATTERN               "*.pk*"     EXCLUDE
    PATTERN               "*tmp*"     EXCLUDE
)


#set( CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/../doc/readme.txt" )
set( CPACK_RESOURCE_FILE_LICENSE    "${CMAKE_SOURCE_DIR}/../installer/ProgramFolder/Licences/EULA.txt"   )
set( CPACK_RESOURCE_FILE_README     "${CMAKE_SOURCE_DIR}/../installer/ProgramFolder/ReadMe.rtf"          )
set( CPACK_RESOURCE_FILE_WELCOME    "${CMAKE_SOURCE_DIR}/../installer/resources/OSX/MacInstallIntro.txt" )


if ( LE_SW_FMOD )

    set( CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}${LEB_OS_SUFFIX}-FMOD_Studio" )

else() # non-FMOD builds

if ( WIN32 )
    set( CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}${LEB_OS_SUFFIX}-" )
endif()
set( CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}Setup" )

#set( CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF )
#set( CPACK_PACKAGE_EXECUTABLES ${LE_PROJECT_NAME}.vst;${LE_PROJECT_NAME} )

# Note: Place the configured paths file into the binary dir so that CMake
# configure runs from different machines (w/ different OSes) over a network from
# the same source location do not overwrite each other's paths file.
#                                             (11.06.2015.) (Domagoj Saric)
set ( pathsFile "${CMAKE_BINARY_DIR}/${LE_PROJECT_NAME}.paths" )

if ( APPLE )
    #...mrmlj...install( TARGET ) does not yet work because CPack assumes that
    #...mrmlj...bundles have a .app extension...
    install(
        FILES          "${defaultSWBinaryPath}/Release/${LE_PROJECT_NAME}.dylib"
        DESTINATION    "${sharedDirectory}"
        COMPONENT      PluginBinary
        PERMISSIONS    OWNER_READ GROUP_READ WORLD_READ OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE
        CONFIGURATIONS Release
    )

    install(
        DIRECTORY             "${CMAKE_SOURCE_DIR}/../installer/resources/OSX/${LE_PROJECT_NAME}.vst"
        DESTINATION           "${vstDirectory}"
        COMPONENT             VST24PluginBundle
        FILE_PERMISSIONS      ${sharedPermissions}
        DIRECTORY_PERMISSIONS ${sharedPermissions} OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE
        CONFIGURATIONS        Debug Release
        PATTERN               "*.svn"     EXCLUDE
        PATTERN               ".DS_store" EXCLUDE
    )

    install(
        DIRECTORY             "${CMAKE_SOURCE_DIR}/../installer/resources/OSX/${LE_PROJECT_NAME}.component"
        DESTINATION           "${auDirectory}"
        COMPONENT             AUComponent
        FILE_PERMISSIONS      ${sharedPermissions}
        DIRECTORY_PERMISSIONS ${sharedPermissions} OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE
        PATTERN               "*.svn"     EXCLUDE
        PATTERN               ".DS_store" EXCLUDE
    )

    install(
        FILES                 "${leExternals}/plugins/au/resources.rsrc"
        DESTINATION           "${auDirectory}/${LE_PROJECT_NAME}.component/Contents/Resources"
        RENAME                "${LE_PROJECT_NAME}.rsrc"
        COMPONENT             AUComponent
        PERMISSIONS           OWNER_READ GROUP_READ WORLD_READ
        CONFIGURATIONS        Debug Release
    )

    # VST & AU bundle/plist info:
    # - http://www.australsounds.com/2011/04/vst-development-in-xcode.html
    # - http://developer.apple.com/library/mac/#technotes/tn2276/_index.html#//apple_ref/doc/uid/DTS40011031
    # - http://developer.apple.com/library/mac/#documentation/General/Reference/InfoPlistKeyReference/Articles/CocoaKeys.html
    # - http://developer.apple.com/library/ios/#documentation/general/Reference/InfoPlistKeyReference/Articles/CoreFoundationKeys.html#//apple_ref/doc/uid/TP40009249-SW1
    # - http://revolution.byu.edu/helps/file-creatorcodes.php
    # - http://developer.apple.com/library/mac/#documentation/CoreFoundation/Conceptual/CFBundles/Introduction/Introduction.html
    install(
        FILES                 "${CMAKE_SOURCE_DIR}/../installer/resources/OSX/PkgInfo"
        DESTINATION           "${vstDirectory}/${LE_PROJECT_NAME}.vst/Contents"
        COMPONENT             VST24PluginBundle
        PERMISSIONS           OWNER_READ GROUP_READ WORLD_READ
        CONFIGURATIONS        Debug Release
    )
    install(
        FILES                 "${CMAKE_SOURCE_DIR}/../installer/resources/OSX/PkgInfo"
        DESTINATION           "${auDirectory}/${LE_PROJECT_NAME}.component/Contents"
        COMPONENT             AUComponent
        PERMISSIONS           OWNER_READ GROUP_READ WORLD_READ
        CONFIGURATIONS        Debug Release
    )
    install(
        FILES                 "${pathsFile}"
        DESTINATION           "${vstDirectory}/${LE_PROJECT_NAME}.vst/Contents/MacOS"
        COMPONENT             VST24PluginBundle
        PERMISSIONS           ${sharedPermissions}
        CONFIGURATIONS        Debug Release
    )
    install(
        FILES                 "${pathsFile}"
        DESTINATION           "${auDirectory}/${LE_PROJECT_NAME}.component/Contents/MacOS"
        COMPONENT             AUComponent
        PERMISSIONS           ${sharedPermissions}
        CONFIGURATIONS        Debug Release
    )

    # http://www.vtk.org/Wiki/CMake:CPackConfiguration
    #
    # "PackageMaker Howto"             http://s.sudre.free.fr/Stuff/PackageMaker_Howto.html
    # "Distributing with PackageMaker" http://www.mactech.com/articles/mactech/Vol.22/22.12/2212PackageMaker/index.html
    #...mrmlj...PackageMaker is deprecated/no longer provided by Apple (last
    #...mrmlj...available in 2012 Auxiliary tools) and CMake is still not
    #...mrmlj...updated...
    #https://public.kitware.com/Bug/view.php?id=13662
    #https://developer.apple.com/downloads/?name=auxiliary
    #http://stackoverflow.com/questions/11487596/making-os-x-installer-packages-like-a-pro-xcode4-developer-id-mountain-lion-re
    #
    # http://stackoverflow.com/questions/6712000/cmake-cpack-package-installation-path-nightmare
    # http://www.cmake.org/pipermail/cmake/2008-July/022985.html
    # http://public.kitware.com/Bug/view.php?id=14433

    set( CMAKE_INSTALL_PREFIX           "/" )
    set( CPACK_INSTALL_PREFIX           "/" )
    set( CPACK_PACKAGING_INSTALL_PREFIX "/" )
    set( CPACK_PACKAGE_DEFAULT_LOCATION "/" )

    set( CPACK_PACKAGE_ICON   "${CMAKE_SOURCE_DIR}/../installer/resources/OSX/SpectrumWorx.icns" )
    set( CPACK_APPLE_GUI_ICON "${CMAKE_SOURCE_DIR}/../installer/resources/OSX/SpectrumWorx.icns" )
    set( CPACK_OSX_PACKAGE_VERSION 10.6 )
    set( CPACK_GENERATOR PackageMaker )
    set( CPACK_STRIP_FILES TRUE )
    # http://www.cmake.org/pipermail/cmake/2008-July/022958.html
    # http://www.cmake.org/pipermail/cmake/2010-February/035171.html
    set( CPACK_SET_DESTDIR TRUE )
    set( CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE ON )
    #...mrmlj...cpack bug...scripts don't work in multi component installers...
    #...mrmlj...http://public.kitware.com/Bug/view.php?id=12375 marked as fixed but it still does not seem to work...
    #set( CPACK_COMPONENTS_ALL SupportFiles PluginBinary AUComponent VST24PluginBundle )
    #set( CPACK_ALL_INSTALL_TYPES Full AU VST24 )
    set( CPACK_MONOLITHIC_INSTALL ON )

    # Symlinks:
    # http://www.cmake.org/pipermail/cmake/2010-April/036504.html
    # http://lists.kde.org/?l=kde-commits&m=116948038808442&w=2
    set( CPACK_POSTUPGRADE_SCRIPT "${CMAKE_SOURCE_DIR}/../installer/resources/OSX/make_symlinks.sh" )
    set( CPACK_POSTFLIGHT_SCRIPT  "${CMAKE_SOURCE_DIR}/../installer/resources/OSX/make_symlinks.sh" )
    set( CPACK_POSTINSTALL_SCRIPT "${CMAKE_SOURCE_DIR}/../installer/resources/OSX/make_symlinks.sh" )

    list( APPEND CPACK_INSTALL_COMMANDS
        "strip -S -x \\\"${defaultSWBinaryPath}/Release/${LE_PROJECT_NAME}.dylib\\\""
    )

    #list( APPEND CPACK_INSTALL_COMMANDS
	#"ln -s Library/Audio/Plug-Ins/VST/${LE_PROJECT_NAME}.vst Library/Audio/Plug-Ins/Components/${LE_PROJECT_NAME}.component/Contents/Resources/${LE_PROJECT_NAME}.vst"
    #)

else() # Windows
    # WiX
    #  http://wixtoolset.org
    #  http://www.joyofsetup.com
    #  http://github.com/dblock/msiext
    #
    # Tutorials:
    #  http://wix.tramontana.co.hu
    #  http://www.merlinia.com/mdt/WiXTutorial4.msl
    #  http://blogs.technet.com/b/alexshev (From MSI to Wix)
    #  http://www.dizzymonkeydesign.com/blog/misc/adding-and-customizing-dlgs-in-wix-3
    #  http://stackoverflow.com/questions/471424/wix-tricks-and-tips
    #  http://kentie.net/article/wixtipstricks
    #  http://www.packtpub.com/sites/default/files/3722-chapter-1-getting-started%20.pdf
    #
    # IDEs:
    #  http://www.icsharpcode.net/opensource/sd
    #  http://wixedit.sourceforge.net
    #
    # speedups:
    #  - "consider only having select files (EXE,DLL,OCX...) as keyfiles and the
    #    rest as companion files"
    #  - disable rollback http://msdn.microsoft.com/en-us/library/windows/desktop/aa372899%28v=vs.85%29.aspx
    #  - MSIFASTINSTALL http://msdn.microsoft.com/en-us/library/windows/desktop/dd408005%28v=vs.85%29.aspx
    #
    # install size calculation problems:
    #  http://www.mail-archive.com/wix-users@lists.sourceforge.net/msg15656.html
    #  http://blogs.msdn.com/b/oldnewthing/archive/2004/07/09/178342.aspx
    #
    # Alternatives
    #  http://nsis.sourceforge.net
    #  http://www.installsite.org/pages/en/tt_nonmsi.htm
    #
    # WiX + CPack
    # http://www.cmake.org/cmake/help/v3.0/module/CPackWIX.html
    # http://annealingtechnologies.blogspot.com/2010/02/wix-and-cpack-integration.html

    if ( CMAKE_GENERATOR MATCHES 64 )
        set( programFiles "$ENV{ProgramW6432}" )
    else()
        set( programFiles "$ENV{PROGRAMFILES}" )
    endif()
    file( TO_CMAKE_PATH ${programFiles} programFiles )

    set( CPACK_GENERATOR WIX )
    #set( CPACK_GENERATOR NSIS )
    set( CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE ON )
    set( CPACK_COMPONENTS_ALL SupportFiles VST24PluginBundle )
    set( CPACK_WIX_COMPONENT_INSTALL  ON )
    set( CPACK_NSIS_COMPONENT_INSTALL ON )
    #set( CPACK_MONOLITHIC_INSTALL OFF ) #...mrmlj...even setting it seems to disable component install...
    set( CPACK_NSIS_MODIFY_PATH ON )
    set( CMAKE_INSTALL_PREFIX "${programFiles}" )
    if ( CPACK_GENERATOR STREQUAL WIX ) #...mrmlj...
        file( RELATIVE_PATH vstDirectory "${CMAKE_INSTALL_PREFIX}" "${vstDirectory}" )
    else()
        set( CPACK_INSTALL_PREFIX           "${programFiles}" )
        set( CPACK_PACKAGING_INSTALL_PREFIX "${programFiles}" )
        set( CPACK_PACKAGE_DEFAULT_LOCATION "${programFiles}" )
        set( CPACK_SET_DESTDIR TRUE )
    endif()
    file( TO_CMAKE_PATH "$ENV{TEMP}/${CPACK_PACKAGE_FILE_NAME}" CPACK_PACKAGE_DIRECTORY )
    if ( CMAKE_GENERATOR MATCHES 64 )
        set( LE_CPACK_WIX_START_MENU_GUID 3B80AC4B-C894-4215-A05D-98D4F17FD358 )
        set( CPACK_WIX_UPGRADE_GUID       4CE617E5-202B-4A65-85A7-43B7668EF759 )
        set( CPACK_WIX_PRODUCT_GUID_2.9.0 43B34092-FA2E-43FF-83E2-75226C4D77D1 )
        set( CPACK_WIX_PRODUCT_GUID_2.9.1 7A5B91CC-695F-4630-B939-01366FE0899A )
        set( CPACK_WIX_PRODUCT_GUID_2.9.2 20F76FD0-B63B-4C48-AAC1-45CA0B21C3C9 )
        set( CPACK_WIX_PRODUCT_GUID_2.9.3 BCDA631A-FAAC-4E47-98F0-10594F56B23E )
        set( CPACK_WIX_PRODUCT_GUID_2.9.4 C7E0623D-B6C4-40A6-92D5-E63469495592 )

        set( CPACK_WIX_PRODUCT_GUID_3.0.0 2CCE732F-BED2-4E80-B281-7FDC2E19DC2A )
    else()
        set( LE_CPACK_WIX_START_MENU_GUID 6C5B5843-603C-4640-9D1A-934DF6559040 )
        set( CPACK_WIX_UPGRADE_GUID       AFFC54F0-D507-4D9C-A201-D31FBE27766C )
        set( CPACK_WIX_PRODUCT_GUID_2.9.0 CBF561D0-6309-4090-9C1F-B8212EE422B1 )
        set( CPACK_WIX_PRODUCT_GUID_2.9.1 A252B981-F20C-439E-A668-8D1C81F80B88 )
        set( CPACK_WIX_PRODUCT_GUID_2.9.2 EE81206F-9457-4E98-B9DC-BAF3B0F0B859 )
        set( CPACK_WIX_PRODUCT_GUID_2.9.3 D5623E52-BDE8-45B3-AC1D-C6E92C164551 )
        set( CPACK_WIX_PRODUCT_GUID_2.9.4 40E88E6C-A3CD-40B2-84E1-22CE6E988A7B )

        set( CPACK_WIX_PRODUCT_GUID_3.0.0 5EF53381-1E72-4089-829A-B4D482CFE543 )
    endif()
    set( CPACK_WIX_PRODUCT_GUID ${CPACK_WIX_PRODUCT_GUID${versionMajor}.${versionMinor}.${versionPatch}} )

    set( CPACK_WIX_PROGRAM_MENU_FOLDER "LittleEndian/SpectrumWorx" )
    set( CPACK_WIX_PRODUCT_ICON        "${CMAKE_SOURCE_DIR}/../installer/resources/Windows/SpectrumWorx_64x64.ico" )
    set( CPACK_WIX_UI_BANNER           "${CMAKE_SOURCE_DIR}/../installer/resources/Windows/upper_right_icon.jpg"   )
    set( CPACK_WIX_UI_DIALOG           "${CMAKE_SOURCE_DIR}/../installer/resources/Windows/left_side_image.jpg"    )
    #set( CPACK_WIX_TEMPLATE )

    set( CPACK_PACKAGE_INSTALL_REGISTRY_KEY "LittleEndian/SpectrumWorx" )

    set( LE_CPACK_WIX_WORK_DIR "${CPACK_PACKAGE_DIRECTORY}/_CPack_Packages/${CPACK_TOPLEVEL_TAG}/WIX" )
    configure_file(
        "${CMAKE_SOURCE_DIR}/../installer/resources/Windows/files.wxs.in"
        "${CMAKE_SOURCE_DIR}/../installer/resources/Windows/files.${LEB_OS_SUFFIX}.wxs"
        @ONLY
    )

    file( TO_CMAKE_PATH "${defaultSWBinaryPath}/Release/${LE_PROJECT_NAME}.dll" dllSourcePath )
    install(
        FILES          "${dllSourcePath}"
        DESTINATION    "${vstDirectory}"
        COMPONENT      VST24PluginBundle
        PERMISSIONS    OWNER_READ GROUP_READ WORLD_READ OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE
        CONFIGURATIONS Release
    )
    # install(
        # TARGETS ${LE_PROJECT_NAME}
        # CONFIGURATIONS Release
        # COMPONENT VST24PluginBundle
        # DESTINATION "${vstDirectory}"
    # )
    install(
        FILES          "${pathsFile}"
        DESTINATION    "${vstDirectory}"
        COMPONENT      VST24PluginBundle
        PERMISSIONS    ${sharedPermissions}
        CONFIGURATIONS Debug Release
    )

    set( LE_SW_DEFAULT_SHARED_DIRECTORY  "${programFiles}\\${sharedDirectory}"        )
    set( LE_SW_DEFAULT_PRESETS_DIRECTORY "${LE_SW_DEFAULT_SHARED_DIRECTORY}\\Presets" )

endif() # OS

endif( LE_SW_FMOD )

configure_file(
    "${CMAKE_SOURCE_DIR}/../installer/PluginsFolder/SpectrumWorx.paths.in"
    "${pathsFile}"
    NEWLINE_STYLE UNIX
    @ONLY
)
#...mrmlj...configure_file with NEWLINE_STYLE appends a newline to the end of the file...cmMakeFile.cxx line 3521...
file( APPEND "${pathsFile}" "${LE_SW_DEFAULT_PRESETS_DIRECTORY}" )


# Make sure our custom installer templates (CPack.distribution.dist.in,
# WIX.template.in) get used.
list( INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/../installer/resources" )

if ( APPLE )
    set( pluginBinaryComponent PluginBinary )
endif()

#...mrmlj...poor WiX support workarounds...
if ( CPACK_GENERATOR STREQUAL WIX )
    #...mrmlj...c/p from CPackWIX.cmake for wix binaries paths...
    if ( NOT CPACK_WIX_ROOT )
      file( TO_CMAKE_PATH "$ENV{WIX}" CPACK_WIX_ROOT )
    endif()
    find_program( CPACK_WIX_CANDLE_EXECUTABLE candle PATHS "${CPACK_WIX_ROOT}/bin" )
    if( NOT CPACK_WIX_CANDLE_EXECUTABLE )
      message( FATAL_ERROR "Could not find the WiX candle executable." )
    endif()
    find_program( CPACK_WIX_LIGHT_EXECUTABLE light PATHS "${CPACK_WIX_ROOT}/bin" )
    if ( NOT CPACK_WIX_LIGHT_EXECUTABLE )
      message( FATAL_ERROR "Could not find the WiX light executable." )
    endif()

    if ( CMAKE_GENERATOR MATCHES 64 )
        set( wixArch x64 )
    else()
        set( wixArch x86 )
    endif()
    set( resourcesDirectory "${CMAKE_SOURCE_DIR}/../installer/resources/Windows"                   )
    set( packageDirectory   "${CPACK_PACKAGE_DIRECTORY}/_CPack_Packages/${CPACK_TOPLEVEL_TAG}/WIX" )
    add_custom_target( PackageLECustom
        COMMAND ${CPACK_WIX_CANDLE_EXECUTABLE} -nologo -arch ${wixArch} -out directories.wixobj "${resourcesDirectory}/directories.wxs"            -ext WixUIExtension
        COMMAND ${CPACK_WIX_CANDLE_EXECUTABLE} -nologo -arch ${wixArch} -out files.wixobj       "${resourcesDirectory}/files.${LEB_OS_SUFFIX}.wxs" -ext WixUIExtension
        COMMAND ${CPACK_WIX_CANDLE_EXECUTABLE} -nologo -arch ${wixArch} -out features.wixobj    "${resourcesDirectory}/features.wxs"               -ext WixUIExtension
        COMMAND ${CPACK_WIX_CANDLE_EXECUTABLE} -nologo -arch ${wixArch} -out main.wixobj        "${packageDirectory}/main.wxs"                     -ext WixUIExtension
        COMMAND ${CPACK_WIX_LIGHT_EXECUTABLE}  -nologo -dcl:high -spdb  -out "${CMAKE_BINARY_DIR}/${CPACK_PACKAGE_FILE_NAME}.msi" "directories.wixobj" "files.wixobj" "features.wixobj" "main.wixobj" -ext WixUIExtension
        DEPENDS SpectrumWorx
        WORKING_DIRECTORY ${resourcesDirectory}
        SOURCES "${resourcesDirectory}/../WIX.template.in" #"${packageDirectory}/main.wxs"
    )
endif()

include( CPack )

cpack_add_component(
    SupportFiles
    DISPLAY_NAME "Support files"
    DESCRIPTION "Documents, licences, presets, skins..."
    #REQUIRED
    #[GROUP group]
    #[DEPENDS comp1 comp2 ... ]
    #[INSTALL_TYPES type1 type2 ... ]
    #[DOWNLOADED]
    #[ARCHIVE_FILE filename])
)

if ( LE_SW_FMOD )
    cpack_add_component(
        FMODPluginLibraries
        DISPLAY_NAME "FMOD plugin binaries and libraries"
        DESCRIPTION "SpectrumWorx FMOD Studio wrapper"
        DEPENDS SupportFiles ${pluginBinaryComponent}
    )
else()
    cpack_add_component(
        VST24PluginBundle
        DISPLAY_NAME "VST 2.4 plugin"
        DESCRIPTION "SpectrumWorx VST 2.4 wrapper"
        #REQUIRED
        #[GROUP group]
        DEPENDS SupportFiles ${pluginBinaryComponent}
        #[INSTALL_TYPES type1 type2 ... ]
        #[DOWNLOADED]
        #[ARCHIVE_FILE filename])
    )

    if ( APPLE )
        cpack_add_component(
            AUComponent
            DISPLAY_NAME "Audio Unit component"
            DESCRIPTION "SpectrumWorx AU wrapper"
            DEPENDS SupportFiles ${pluginBinaryComponent}
        )

        cpack_add_component(
            ${pluginBinaryComponent}
            DISPLAY_NAME "Plugin binary"
            DESCRIPTION "The actual SpectrumWorx binary"
            DEPENDS SupportFiles
        )
    endif( APPLE )
endif( LE_SW_FMOD )
