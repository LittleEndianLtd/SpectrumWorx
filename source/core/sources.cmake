################################################################################
#
# sources.cmake
#
# Copyright (c) 2012 - 2015. Little Endian Ltd. All rights reserved.
#
################################################################################

set( leExternals "externals/le" )

set( SOURCES_Core
    core/automatedModuleChain.cpp
    core/automatedModuleChain.hpp
    core/parameterID.hpp
    core/spectrumWorxSharedImpl.hpp
    core/spectrumWorxSharedImpl.inl
    core/spectrumWorxCore.cpp
    core/spectrumWorxCore.hpp
)
source_group( "Core" FILES ${SOURCES_Core} )
set( SOURCES_Core_HostInterop
    core/host_interop/host2Plugin.cpp
    core/host_interop/host2Plugin.hpp
    core/host_interop/plugin2Host.cpp
    core/host_interop/plugin2Host.hpp
    core/host_interop/host2PluginImpl.inl
    core/host_interop/host2PluginImpl.hpp
    core/host_interop/plugin2HostImpl.inl
    core/host_interop/plugin2HostImpl.hpp
    core/host_interop/parameters.hpp
)
source_group( "Core\\HostInterop" FILES ${SOURCES_Core_HostInterop} )
set( SOURCES_Core_Modules
    core/modules/automatedModule.cpp
    core/modules/automatedModule.hpp
    core/modules/automatedModuleImpl.hpp
    core/modules/automatedModuleImpl.inl
    core/modules/moduleDSP.hpp
   #core/modules/moduleGUI.cpp
   #core/modules/moduleGUI.hpp
   #core/modules/moduleDSPAndGUI.cpp
   #core/modules/moduleDSPAndGUI.hpp
    core/modules/finalImplementations.hpp
    core/modules/factory.cpp
    core/modules/factory.hpp
)
source_group( "Core\\Modules" FILES ${SOURCES_Core_Modules} core/modules/moduleGUI.cpp core/modules/moduleGUI.hpp core/modules/moduleDSPAndGUI.cpp core/modules/moduleDSPAndGUI.hpp )
set( SOURCES_Core
    ${SOURCES_Core}
    ${SOURCES_Core_HostInterop}
    ${SOURCES_Core_Modules}
)

set( SOURCES_Configuration
    configuration/constants.hpp
    configuration/versionConfiguration.hpp
    configuration/versionConfiguration.hpp.in
)
source_group( "Configuration" FILES ${SOURCES_Configuration} )


set(SOURCES_Externals__Effects__Shared
    ${leExternals}/spectrumworx/effects/baseParameters.hpp
    ${leExternals}/spectrumworx/effects/baseParametersUIElements.cpp
    ${leExternals}/spectrumworx/effects/baseParametersUIElements.hpp
    ${leExternals}/spectrumworx/effects/channelStateDynamic.hpp
    ${leExternals}/spectrumworx/effects/channelStateStatic.hpp
    ${leExternals}/spectrumworx/effects/commonParameters.hpp
    ${leExternals}/spectrumworx/effects/commonParametersUIElements.cpp
    ${leExternals}/spectrumworx/effects/commonParametersUIElements.hpp
    ${leExternals}/spectrumworx/effects/effects.cpp
    ${leExternals}/spectrumworx/effects/effects.hpp
    ${leExternals}/spectrumworx/effects/historyBuffer.cpp
    ${leExternals}/spectrumworx/effects/historyBuffer.hpp
    ${leExternals}/spectrumworx/effects/indexRange.cpp
    ${leExternals}/spectrumworx/effects/indexRange.hpp
    ${leExternals}/spectrumworx/effects/parameters.hpp
    ${leExternals}/spectrumworx/effects/vibrato.cpp
    ${leExternals}/spectrumworx/effects/vibrato.hpp
    ${leExternals}/spectrumworx/effects/phase_vocoder/shared.cpp
    ${leExternals}/spectrumworx/effects/phase_vocoder/shared.hpp
)
source_group("Externals\\Effects\\_shared" FILES ${SOURCES_Externals__Effects__Shared})


set(SOURCES_Externals__Effects
    ${SOURCES_Externals__Effects__Shared}
    ${SOURCES_Effects_Configuration}
    ${SOURCES_Externals__AllEffects}
)

#...mrmlj...clean this up...use Utility as a separate library
set(SOURCES_Externals__Utility
    ${leExternals}/utility/assertionHandler.cpp
    ${leExternals}/utility/buffers.hpp
    ${leExternals}/utility/clear.hpp
    ${leExternals}/utility/countof.hpp
    ${leExternals}/utility/criticalSection.hpp
    ${leExternals}/utility/lexicalCast.cpp
    ${leExternals}/utility/lexicalCast.hpp
    ${leExternals}/utility/filesystem.cpp
    ${leExternals}/utility/filesystem.hpp
    ${leExternals}/utility/parentFromMember.hpp
    ${leExternals}/utility/platformSpecifics.hpp
    ${leExternals}/utility/staticForEach.hpp
    ${leExternals}/utility/switch.hpp
    ${leExternals}/utility/tchar.hpp
    ${leExternals}/utility/trace.cpp
    ${leExternals}/utility/trace.hpp
    ${leExternals}/utility/typeTraits.hpp
    ${leExternals}/utility/xml.cpp
    ${leExternals}/utility/xml.hpp
    externals/boost/filesystem/directory_iterator.hpp
    externals/boost/mmap/mapped_view/mapped_view.hpp
)
if( APPLE )
    set( SOURCES_Externals__Utility
        ${SOURCES_Externals__Utility}
        ${leExternals}/utility/filesystemApple.cpp
        ${leExternals}/utility/objc.mm
    )
    set_source_files_properties( "${leExternals}/utility/filesystemApple.cpp" PROPERTIES COMPILE_FLAGS "-x objective-c++" )
    set_source_files_properties( "${leExternals}/utility/trace.cpp"           PROPERTIES COMPILE_FLAGS "-x objective-c++" )
elseif( WIN32 )
    set( SOURCES_Externals__Utility ${SOURCES_Externals__Utility} ${leExternals}/utility/filesystemWindows.cpp )
endif()
source_group("Externals\\Utility" FILES ${SOURCES_Externals__Utility})


set(SOURCES_Externals__Engine
    ${leExternals}/spectrumworx/engine/automatableParameters.hpp
    ${leExternals}/spectrumworx/engine/automatableParameters.cpp
    ${leExternals}/spectrumworx/engine/buffers.hpp
    ${leExternals}/spectrumworx/engine/channelBuffers.hpp
    ${leExternals}/spectrumworx/engine/channelBuffers.cpp
    ${leExternals}/spectrumworx/engine/channelData.hpp
    ${leExternals}/spectrumworx/engine/channelData.cpp
    ${leExternals}/spectrumworx/engine/channelDataAmPh.hpp
    ${leExternals}/spectrumworx/engine/channelDataAmPh.cpp
    ${leExternals}/spectrumworx/engine/channelDataReIm.hpp
    ${leExternals}/spectrumworx/engine/channelDataReIm.cpp
    ${leExternals}/spectrumworx/engine/channelData_fwd.hpp
    ${leExternals}/spectrumworx/engine/module.hpp
    ${leExternals}/spectrumworx/engine/module.cpp
    ${leExternals}/spectrumworx/engine/moduleBase.hpp
    ${leExternals}/spectrumworx/engine/moduleParameters.hpp
    ${leExternals}/spectrumworx/engine/moduleParameters.cpp
    ${leExternals}/spectrumworx/engine/moduleImpl.hpp
    ${leExternals}/spectrumworx/engine/moduleChainImpl.hpp
    ${leExternals}/spectrumworx/engine/moduleChainImpl.cpp
    ${leExternals}/spectrumworx/engine/moduleNode.hpp
    ${leExternals}/spectrumworx/engine/parameters.hpp
    ${leExternals}/spectrumworx/engine/parameters.cpp
    ${leExternals}/spectrumworx/engine/processor.hpp
    ${leExternals}/spectrumworx/engine/processor.cpp
    ${leExternals}/spectrumworx/engine/setup.hpp
    ${leExternals}/spectrumworx/engine/setup.cpp
)
source_group("Externals\\Engine" FILES ${SOURCES_Externals__Engine})

set(SOURCES_Externals__LEB
    ${leExternals}/build/leConfigurationAndODRHeader.h
)


set(SOURCES_Externals__Math
    ${leExternals}/math/constants.hpp
    ${leExternals}/math/conversion.cpp
    ${leExternals}/math/conversion.hpp
    ${leExternals}/math/math.cpp
    ${leExternals}/math/math.hpp
    ${leExternals}/math/vector.cpp
    ${leExternals}/math/vector.hpp
    ${leExternals}/math/windows.cpp
    ${leExternals}/math/windows.hpp
)
source_group("Externals\\Math" FILES ${SOURCES_Externals__Math})

    set(SOURCES_Externals__Math__DFT
        ${leExternals}/math/dft/domainConversion.cpp
        ${leExternals}/math/dft/domainConversion.hpp
        ${leExternals}/math/dft/fft.cpp
        ${leExternals}/math/dft/fft.hpp
    )
    source_group("Externals\\Math\\DFT" FILES ${SOURCES_Externals__Math__DFT})


set(SOURCES_Externals__Parameters
    ${leExternals}/parameters/conversion.hpp
    ${leExternals}/parameters/implDetails.cpp
    ${leExternals}/parameters/factoryMacro.hpp
    ${leExternals}/parameters/fusionAdaptors.hpp
    ${leExternals}/parameters/lfo.hpp
    ${leExternals}/parameters/lfoImpl.cpp
    ${leExternals}/parameters/lfoImpl.hpp
    ${leExternals}/parameters/parameter.hpp
    ${leExternals}/parameters/parametersUtilities.hpp
    ${leExternals}/parameters/printer.hpp
    ${leExternals}/parameters/printer_fwd.hpp
    ${leExternals}/parameters/runtimeInformation.hpp
    ${leExternals}/parameters/uiElements.hpp
)
source_group("Externals\\Parameters" FILES ${SOURCES_Externals__Parameters})

    set(SOURCES_Externals__Parameters__Boolean
        ${leExternals}/parameters/boolean/conversion.hpp
        ${leExternals}/parameters/boolean/parameter.hpp
        ${leExternals}/parameters/boolean/printer.hpp
        ${leExternals}/parameters/boolean/tag.hpp
    )
    source_group("Externals\\Parameters\\Boolean" FILES ${SOURCES_Externals__Parameters__Boolean})

    set(SOURCES_Externals__Parameters__Dynamic
        ${leExternals}/parameters/dynamic/conversion.hpp
        ${leExternals}/parameters/dynamic/parameter.hpp
        ${leExternals}/parameters/dynamic/printer.hpp
        ${leExternals}/parameters/dynamic/tag.hpp
    )
    source_group("Externals\\Parameters\\Dynamic" FILES ${SOURCES_Externals__Parameters__Dynamic})

    set(SOURCES_Externals__Parameters__Enumerated
        ${leExternals}/parameters/enumerated/conversion.hpp
        ${leExternals}/parameters/enumerated/parameter.hpp
        ${leExternals}/parameters/enumerated/printer.hpp
        ${leExternals}/parameters/enumerated/tag.hpp
    )
    source_group("Externals\\Parameters\\Enumerated" FILES ${SOURCES_Externals__Parameters__Enumerated})

    set(SOURCES_Externals__Parameters__Linear
        ${leExternals}/parameters/linear/conversion.hpp
        ${leExternals}/parameters/linear/parameter.hpp
        ${leExternals}/parameters/linear/printer.hpp
        ${leExternals}/parameters/linear/tag.hpp
    )
    source_group("Externals\\Parameters\\Linear" FILES ${SOURCES_Externals__Parameters__Linear})

    set(SOURCES_Externals__Parameters__PowerOfTwo
        ${leExternals}/parameters/powerOfTwo/conversion.hpp
        ${leExternals}/parameters/powerOfTwo/parameter.hpp
        ${leExternals}/parameters/powerOfTwo/printer.hpp
        ${leExternals}/parameters/powerOfTwo/tag.hpp
    )
    source_group("Externals\\Parameters\\PowerOfTwo" FILES ${SOURCES_Externals__Parameters__PowerOfTwo})

    set(SOURCES_Externals__Parameters__Symmetric
        ${leExternals}/parameters/symmetric/conversion.hpp
        ${leExternals}/parameters/symmetric/parameter.hpp
        ${leExternals}/parameters/symmetric/printer.hpp
        ${leExternals}/parameters/symmetric/tag.hpp
    )
    source_group("Externals\\Parameters\\Symmetric" FILES ${SOURCES_Externals__Parameters__Symmetric})

    set(SOURCES_Externals__Parameters__Trigger
        ${leExternals}/parameters/trigger/conversion.hpp
        ${leExternals}/parameters/trigger/parameter.cpp
        ${leExternals}/parameters/trigger/parameter.hpp
        ${leExternals}/parameters/trigger/printer.hpp
        ${leExternals}/parameters/trigger/tag.hpp
    )
    source_group("Externals\\Parameters\\Trigger" FILES ${SOURCES_Externals__Parameters__Trigger})

set(SOURCES_Externals__Parameters
    ${SOURCES_Externals__Parameters__Boolean}
    ${SOURCES_Externals__Parameters__Dynamic}
    ${SOURCES_Externals__Parameters__Enumerated}
    ${SOURCES_Externals__Parameters__Linear}
    ${SOURCES_Externals__Parameters__PowerOfTwo}
    ${SOURCES_Externals__Parameters__Symmetric}
    ${SOURCES_Externals__Parameters__Trigger}
    ${SOURCES_Externals__Parameters}
)

set( SOURCES_Externals__Plugins
    ${leExternals}/plugins/plugin.hpp
)
source_group("Externals\\Plugins" FILES ${SOURCES_Externals__Plugins})

    set( SOURCES_Externals__Plugins__FMOD
      ${leExternals}/plugins/fmod/plugin.cpp
      ${leExternals}/plugins/fmod/plugin.hpp
      ${leExternals}/plugins/fmod/plugin.inl
      ${leExternals}/plugins/fmod/tag.hpp
    )
    set( SOURCES_Externals__Plugins__FMOD_GUI
      ${leExternals}/plugins/fmod/gui.cpp
      ${leExternals}/plugins/fmod/gui.hpp
    )
    source_group("Externals\\Plugins\\FMOD" FILES ${SOURCES_Externals__Plugins__FMOD} ${SOURCES_Externals__Plugins__FMOD_GUI})
    if ( APPLE )
        appendSourceFileFlags( "${leExternals}/plugins/fmod/gui.cpp" COMPILE_FLAGS -x objective-c++ )
    endif()
    
    set( SOURCES_Externals__Plugins__Unity
      ${leExternals}/plugins/unity/plugin.cpp
      ${leExternals}/plugins/unity/plugin.hpp
      ${leExternals}/plugins/unity/plugin.inl
      ${leExternals}/plugins/unity/tag.hpp
    )
    source_group("Externals\\Plugins\\Unity" FILES ${SOURCES_Externals__Plugins__Unity})

set(SOURCES_Externals__SDK
    ${leExternals}/spectrumworx/engine/configuration.hpp
)
source_group("Externals\\SDK" FILES ${SOURCES_Externals__SDK})

set(SOURCES_Externals__Analysis
    ${leExternals}/analysis/musical_scales/musicalScales.cpp
    ${leExternals}/analysis/musical_scales/musicalScales.hpp
    ${leExternals}/analysis/peak_detector/peakDetector.cpp
    ${leExternals}/analysis/peak_detector/peakDetector.hpp
    ${leExternals}/analysis/pitch_detector/pitchDetector.cpp
    ${leExternals}/analysis/pitch_detector/pitchDetector.hpp
)
source_group("Externals\\Analysis" FILES ${SOURCES_Externals__Analysis})

if ( LE_SW_PRESETS )
    set(SOURCES_Externals__Presets
        ${leExternals}/spectrumworx/presets.hpp
        ${leExternals}/spectrumworx/presets.cpp
    )
    source_group("Externals\\Presets" FILES ${SOURCES_Externals__Presets})
endif()

set( SOURCES_Externals_Core
  ${SOURCES_Externals__Analysis}
  ${SOURCES_Externals__Utility}
  ${SOURCES_Externals__Effects}
  ${SOURCES_Externals__Engine}
  ${SOURCES_Externals__Math}
  ${SOURCES_Externals__Math__DFT}
  ${SOURCES_Externals__Parameters}
  ${SOURCES_Externals__Plugins}
  ${SOURCES_Externals__Presets}
  ${SOURCES_Externals__SDK}
)
