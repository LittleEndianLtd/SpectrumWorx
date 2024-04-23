################################################################################
#
# effectsList.cmake
#
# Copyright (c) 2012 - 2016. Little Endian Ltd. All rights reserved.
#
################################################################################

set( completeEffectList
  # syntax: "folder_name,C++ module name (omit the Impl suffix),effect implementation class name(omit the Impl suffix),group name"

  # Pitch (7)
  "pitch_shifter,pitchShifter,PitchShifter,Pitch"
  "pitch_follower,pitchFollower,PitchFollower,Pitch"
  "tune_worx,tuneWorx,TuneWorx,Pitch"
  "pitch_magnet,pitchMagnet,PitchMagnet,Pitch"
  "sumo_pitch,sumoPitch,SumoPitch,Pitch"
  "pitch_spring,pitchSpring,PitchSpring,Pitch"
  "octaver,octaver,Octaver,Pitch"

  # Timbre (8)
  "bandpass,bandpass,Bandpass,Timbre"
  "bandpass,bandpass,Bandstop,Timbre"
  "ah_ah,ahAh,AhAh,Timbre"
  "smoother,smoother,Smoother,Timbre"
  "sharper,sharper,Sharper,Timbre"
  "centroid_extractor,centroidExtractor,CentroidExtractor,Timbre"
  "tonal,tonal,Tonal,Timbre"
  "tonal,tonal,Atonal,Timbre"

  # Time (6)
  "freeze,freeze,Freeze,Time"
  "slicer,slicer,Slicer,Time"
  "wobbler,wobbler,Wobbler,Time"
  "reverser,reverser,Reverser,Time"
  "eximploder,exImploder,Imploder,Time"
  "eximploder,exImploder,Exploder,Time"

  # Space (3)
  "frecho,frecho,Frecho,Space"
  "frecho,frecho,Frevcho,Space"
  "freqverb,freqverb,Freqverb,Space"

  # Phase (4)
  "robotizer,robotizer,Robotizer,Phase"
  "whisperer,whisperer,Whisperer,Phase"
  "phasevolution,phasevolution,Phasevolution,Phase"
  "phlip,phlip,Phlip,Phase"

  # Loudness (5)
  "gain,gain,Gain,Loudness"
  "exaggerator,exaggerator,Exaggerator,Loudness"
  "denoiser,denoiser,Denoiser,Loudness"
  "quiet_boost,quietBoost,QuietBoost,Loudness"
  "freqnamics,freqnamics,Freqnamics,Loudness"

  # Combine (10)
  "talking_wind,talkingWind,TalkingWind,Combine"
  "convolver,convolver,Convolver,Combine"
  "ethereal,ethereal,Ethereal,Combine"
  "vaxateer,vaxateer,Vaxateer,Combine"
  "shapeless,shapeless,Shapeless,Combine"
  "colorifer,colorifer,Colorifer,Combine"
  "merger,merger,Merger,Combine"
  "blender,blender,Blender,Combine"
  "inserter,inserter,Inserter,Combine"
  "burrito,burrito,Burrito,Combine"

  # PV Domain (9)
  "phase_vocoder_analysis,phaseVocoderAnalysis,PhaseVocoderAnalysis,PVD"
  "pitch_shifter,pitchShifter,PVPitchShifter,PVD"
  "pitch_follower,pitchFollower,PitchFollowerPVD,PVD"
  "tune_worx,tuneWorx,TuneWorxPVD,PVD"
  "pitch_magnet,pitchMagnet,PitchMagnetPVD,PVD"
  "pitch_spring,pitchSpring,PitchSpringPVD,PVD"
  "eximploder,exImploder,PVImploder,PVD"
  "eximploder,exImploder,PVExploder,PVD"
  "phase_vocoder_synthesis,phaseVocoderSynthesis,PhaseVocoderSynthesis,PVD"

  # Misc (5)
  "armonizer,armonizer,Armonizer,Misc"
  "slew_limiter,slewLimiter,SlewLimiter,Misc"
  "shifter,shifter,Shifter,Misc"
  "swappah,swappah,Swappah,Misc"
  "quantizer,quantizer,Quantizer,Misc"

  #"synth,synth,Synth,Misc"
  #"talk_box,talkBox,TalkBox,Misc"
  #"vocoder,vocoder,Vocoder,Misc"
)

#...mrmlj...assumes location of effects...
set( leExternals "externals/le" )
include( "${CMAKE_CURRENT_LIST_DIR}/configuration.cmake" )
include( "${leExternals}/build/utilities.cmake"          )

macro( addToParentScope variable )
    set( ${variable} "${${variable}}" PARENT_SCOPE )
endmacro()

function( addSelectedEffects includedEffects )

    list( LENGTH includedEffects numberOfIncludedEffects )

    set( effectIndex 0 )
    foreach( effect ${completeEffectList} )
        string( REPLACE "," ";" effectElements ${effect} )
        list( GET effectElements 0 effectFolder )
        list( GET effectElements 1 effectModule )
        list( GET effectElements 2 effectName   )
        list( GET effectElements 3 effectGroup  )

        # Implementation note:
        #   We save basic information for all effects (even those not included
        # in the build/"edition" in order to enable (host project) compatibility
        # between different editions and to be able to show lists/menus filled
        # with all known effects (with only the actually included ones enabled/
        # selectable).
        #                                     (06.03.2012.) (Domagoj Saric)

        list( APPEND completeBaseHeaderList "${effectFolder}/${effectModule}.hpp" )
        list( APPEND groupList              "${effectGroup}"                      ) # ...mrmlj...only for plugin...

        list( FIND includedEffects ${effectName} foundEffectIndex )
        if ( ( foundEffectIndex GREATER -1 ) OR ( includedEffects STREQUAL all ) )
            set( effectIsIncluded TRUE )
        else()
            set( effectIsIncluded FALSE )
        endif()

        #...mrmlj...only for SDK...
        set( effectTypeNames          "${effectTypeNames}\t\"${effectName}\",\n" )
        #...mrmlj...prevent linker errors in the DoxyHelper in non-complete sdk builds...clean this up asap...
        if ( effectIsIncluded OR NOT LE_SDK_PROJECT_NAME )
            set( effectNames          "${effectNames}\tEffects::${effectName}::title,\n" )
        else()
            set( effectNames          "${effectNames}\t\"\",\n" )
        endif()
        set( effectIndexGroupMappings "${effectIndexGroupMappings}template <> struct Group<${effectIndex}> { typedef ModuleGroups::${effectGroup} type; };\n" )

        #http://stackoverflow.com/questions/275868/how-to-automatically-add-header-files-to-project
        #aux_source_directory( "${leExternals}/spectrumworx/effects/${effectFolder}" effectSources )
        set ( effectSources
            "${leExternals}/spectrumworx/effects/${effectFolder}/${effectModule}Impl.cpp"
            "${leExternals}/spectrumworx/effects/${effectFolder}/${effectModule}Impl.hpp"
            "${leExternals}/spectrumworx/effects/${effectFolder}/${effectModule}.hpp"
        )

        # Capitalize the first letter of the effect name for a nicer IDE display
        set( capitalizedEffectModuleName ${effectModule} )
        LE_capitaliseFirstLetter( capitalizedEffectModuleName )

        list( APPEND SOURCES_Externals__AllEffects ${effectSources} )

        if ( effectIsIncluded )
            list( APPEND SOURCES_Externals__IncludedEffects ${effectSources} )

            list( APPEND selectedBaseHeaderList  "${effectFolder}/${effectModule}.hpp"     )
            list( APPEND selectedImpllHeaderList "${effectFolder}/${effectModule}Impl.hpp" )

            set( indexToEffectImplMappings "${indexToEffectImplMappings}template <> struct ImplForIndex<${effectIndex}> { typedef ${effectName}Impl type; };\n" )
            # ...mrmlj...only for SDK...mrmlj..should include only selected effects...
            set( effectToIndexMappings     "${effectToIndexMappings}template <> struct IndexForEffect<${effectName}> { static unsigned const value = ${effectIndex}; };\n" )
            if ( validEffectIndices )
                set( validEffectIndices "${validEffectIndices}," )
            endif()
            set( validEffectIndices "${validEffectIndices} ${effectIndex}" )
            set( enabledEffects "${enabledEffects}\ttrue,\n" )

            # ...mrmlj...only for SDK...
            list( APPEND effectList "${effectName}" )
            if ( LE_SDK_BUILD )
                install(
                    FILES
                        "${leExternals}/spectrumworx/effects/${effectFolder}/${effectModule}.hpp"
                    DESTINATION    "include/le/spectrumworx/effects"
                    COMPONENT      SpectrumWorxSDKHeaders
                    CONFIGURATIONS ${installConfigs}
                )
            endif()
        else()
            # ...mrmlj...only for plugin...
            set( enabledEffects "${enabledEffects}\tfalse,\n" )
        endif()

        math( EXPR effectIndex "${effectIndex} + 1" )
    endforeach( effect )

    list( REMOVE_DUPLICATES completeBaseHeaderList  )
    list( REMOVE_DUPLICATES selectedBaseHeaderList  )
    list( REMOVE_DUPLICATES selectedImpllHeaderList )
    list( REMOVE_DUPLICATES groupList               ) # ...mrmlj...only for plugin...
    list( REMOVE_DUPLICATES effectList              ) # ...mrmlj...only for SDK...

    list( SORT completeBaseHeaderList  )
    list( SORT selectedBaseHeaderList  )
    list( SORT selectedImpllHeaderList )
    list( SORT effectList              )

    foreach( effectInclude ${selectedImpllHeaderList} )
      set( selectedImplIncludeList "${selectedImplIncludeList}#include \"le/spectrumworx/effects/${effectInclude}\"\n" )
    endforeach( effectInclude )

    foreach( effectInclude ${completeBaseHeaderList} )
      set( completeBaseIncludeList "${completeBaseIncludeList}#include \"le/spectrumworx/effects/${effectInclude}\"\n" )
    endforeach( effectInclude )

    # ...mrmlj...only for SDK...
    foreach( effect ${effectList} )
        if ( effectTypeList )
            set( effectTypeList "${effectTypeList},\n" )
        endif()
        set( effectTypeList   "${effectTypeList}   \t${effect}"                                  )
        set( effectCreateList "${effectCreateList} \ntemplate class Module<Effects::${effect}>;" )
    endforeach( effect )

    # ...mrmlj...only for plugin...
    foreach( group ${groupList} )
      if ( groupTypeList )
          set( groupTypeList "${groupTypeList},\n" )
      endif()
      set( groupTypeList "${groupTypeList}\tModuleGroups::${group}" )
    endforeach( group )

    # ...mrmlj...only for plugin...
    list( LENGTH completeEffectList numberOfEffects         )
    list( LENGTH effectList         numberOfIncludedEffects )
    list( LENGTH groupList          numberOfGroups          )

    if ( includedEffects STREQUAL all )
        add_definitions( -DLE_SW_FULL )
    endif()

    lel_effects_configure_sources()
    
    source_group( "Externals\\Effects" FILES ${SOURCES_Externals__AllEffects})

    addToParentScope( SOURCES_Externals__AllEffects      )
    addToParentScope( SOURCES_Externals__IncludedEffects )
    addToParentScope( completeBaseHeaderList             )
    addToParentScope( selectedBaseHeaderList             )
    addToParentScope( completeBaseIncludeList            )
    addToParentScope( selectedImplIncludeList            )
    addToParentScope( effectTypeList                     )
    addToParentScope( effectToIndexMappings              )
    addToParentScope( effectCreateList                   )
    addToParentScope( groupTypeList                      )
    addToParentScope( effectNames                        )
    addToParentScope( effectTypeNames                    )
    addToParentScope( enabledEffects                     )
    addToParentScope( validEffectIndices                 )
    addToParentScope( effectIndexGroupMappings           )
    addToParentScope( indexToEffectMappings              )
    addToParentScope( numberOfEffects                    )
    addToParentScope( numberOfIncludedEffects            )
    addToParentScope( numberOfGroups                     )

endfunction( addSelectedEffects )
