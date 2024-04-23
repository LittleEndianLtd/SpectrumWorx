////////////////////////////////////////////////////////////////////////////////
///
/// \file properties.hpp
/// --------------------
///
/// Copyright (c) 2013 - 2015. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef properties_hpp__E6002456_AF36_4B69_9762_7CE0B91F29AA
#define properties_hpp__E6002456_AF36_4B69_9762_7CE0B91F29AA
#pragma once
//------------------------------------------------------------------------------
#include "plugin.hpp"

#include "le/utility/buffers.hpp"
#include "le/utility/cstdint.hpp"
#include "le/utility/platformSpecifics.hpp"
#include "le/utility/trace.hpp"
#include "le/utility/typeTraits.hpp"

#include "boost/assert.hpp"
#include "boost/concept_check.hpp"
#include "boost/preprocessor/seq/for_each.hpp"
#include "boost/range/iterator_range_core.hpp"

#include <array>
#include <cstddef>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Plugins
{
//------------------------------------------------------------------------------

namespace Detail
{
    enum CanDo { No = false, Yes = true, RuntimeQuery };

    template <typename T> struct Array;

    template <AudioUnitPropertyID> struct PropertyType;
    template <AudioUnitPropertyID> struct PropertyValueCount { template <class Impl> static unsigned int get( Impl const &, ::AudioUnitScope, ::AudioUnitElement ) { return 1; } };

    template <class Impl, AudioUnitPropertyID PropertyID>
    struct PropertyHandler
    {
        typedef void const * value_type;

        static CanDo          const readable = No;
        static CanDo          const writable = No;
        static AudioUnitScope const scope0   = -1;
        static AudioUnitScope const scope1   = -1;

        template <typename T> static OSStatus get( Impl const &, AudioUnitScope, AudioUnitElement, T const & ) { return kAudio_UnimplementedError; }
        template <typename T> static OSStatus set( Impl const &, AudioUnitScope, AudioUnitElement, T const & ) { return kAudio_UnimplementedError; }
    };


    template <typename Source, typename Target> struct CopyConstness                       { typedef Target       type; };
    template <typename Source, typename Target> struct CopyConstness<Source const, Target> { typedef Target const type; };

    template <typename PropertyType>
    struct PropertyTypeTraits
    {
        static bool const array = false;

        typedef PropertyType               value_type;
        typedef PropertyType const   const_value_type;
        typedef PropertyType       &       param_type;
        typedef PropertyType const & const_param_type;

        static param_type make( void * const pOutData, UInt32 & size )
        {
            LE_ASSUME( pOutData );
            LE_TRACE_IF( ( size != sizeof( value_type ) ),"AU: Invalid property buffer size (required %lu, provided %u)", sizeof( value_type ), static_cast<unsigned int>( size ) );
            LE_ASSUME( size >= sizeof( value_type ) );
            size = sizeof( value_type );
            //BOOST_ASSERT_MSG( !pDataSize || *pDataSize == sizeof( value_type ), "Incorrect AU property value buffer size." );
            return *static_cast<value_type *>( pOutData );
        }

        static const_param_type make( void const * const pInData, UInt32 const size )
        {
            LE_ASSUME( pInData );
            LE_ASSUME( size == sizeof( value_type ) );
            return *static_cast<const_value_type *>( pInData );
        }
    }; // struct PropertyTypeTraits


    template <typename PropertyType>
    struct PropertyTypeTraits<Array<PropertyType>>
    {
        static bool const array = true;

        typedef PropertyType                                                value_type;
        typedef PropertyType const                                    const_value_type;
        typedef boost::iterator_range<      value_type * LE_RESTRICT>       param_type;
        typedef boost::iterator_range<const_value_type * LE_RESTRICT> const_param_type;

        static param_type make( void * const pOutData, UInt32 & size )
        {
            LE_ASSUME( pOutData );
            LE_ASSUME( size % sizeof( value_type ) == 0 );
            //BOOST_ASSERT_MSG( !pDataSize || *pDataSize == sizeof( value_type ), "Incorrect AU property value buffer size." );
            value_type * const pBegin( static_cast<value_type *>( pOutData ) );
            value_type * const pEnd  ( pBegin + size / sizeof( value_type )  );
            return boost::make_iterator_range( pBegin, pEnd );
        }

        static const_param_type make( void const * const pInData, UInt32 const size )
        {
            LE_ASSUME( pInData );
            LE_ASSUME( size % sizeof( value_type ) == 0 );
            const_value_type * const pBegin( static_cast<const_value_type *>( pInData ) );
            const_value_type * const pEnd  ( pBegin + size / sizeof( value_type )       );
            return boost::make_iterator_range( pBegin, pEnd );
        }
    }; // struct PropertyTypeTraits<Array<PropertyType>>

    template <>
    struct PropertyTypeTraits<::HostCallbackInfo>
    {
        using PropertyType = ::HostCallbackInfo;

        static bool constexpr array = false;

        using       value_type = PropertyType        ;
        using const_value_type = PropertyType const  ;
        using       param_type = PropertyType       &;
        using const_param_type = PropertyType const &;

        static param_type make( void * const pOutData, UInt32 & size ) //...mrmlj...copy pasted non specialized version...
        {
            LE_ASSUME( pOutData );
            LE_TRACE_IF( ( size != sizeof( value_type ) ),"AU: Invalid property buffer size (required %lu, provided %u)", sizeof( value_type ), static_cast<unsigned int>( size ) );
            LE_ASSUME( size >= sizeof( value_type ) );
            size = sizeof( value_type );
            //BOOST_ASSERT_MSG( !pDataSize || *pDataSize == sizeof( value_type ), "Incorrect AU property value buffer size." );
            return *static_cast<value_type *>( pOutData );
        }

        static const_param_type make( void const * const pInData, UInt32 const size )
        {
            LE_ASSUME( pInData );
            LE_TRACE_IF( ( size > sizeof( value_type ) ), "Storage provided for HostCallbackInfo larger than sizeof( HostCallbackInfo )" );
            if ( BOOST_LIKELY( size >= sizeof( value_type ) ) )
                return *static_cast<const_value_type *>( pInData );
            else
            {
                static value_type value;
                LE_TRACE( "Storage provided for HostCallbackInfo smaller than sizeof HostCallbackInfo (%u < %u)", size, sizeof( value ) );
                std::memcpy( &value, pInData, size );
                std::memset( reinterpret_cast<char *>( &value ) + size, 0, sizeof( value ) - size );
                return value;
            }
        }
    }; // struct PropertyTypeTraits<::HostCallbackInfo>

    template <>
    struct PropertyTypeTraits<::AUHostVersionIdentifier> //...mrmlj...for StudioOne 2.6.3
    {
        using PropertyType = ::AUHostVersionIdentifier;

        static bool constexpr array = false;

        using       value_type = PropertyType        ;
        using const_value_type = PropertyType const  ;
        using       param_type = PropertyType       &;
        using const_param_type = PropertyType const &;

        static param_type make( void * const pOutData, UInt32 & size ) //...mrmlj...copy pasted non specialized version...
        {
            LE_ASSUME( pOutData );
            LE_TRACE_IF( ( size != sizeof( value_type ) ),"AU: Invalid property buffer size (required %lu, provided %u)", sizeof( value_type ), static_cast<unsigned int>( size ) );
            LE_ASSUME( size >= sizeof( value_type ) );
            size = sizeof( value_type );
            //BOOST_ASSERT_MSG( !pDataSize || *pDataSize == sizeof( value_type ), "Incorrect AU property value buffer size." );
            return *static_cast<value_type *>( pOutData );
        }

        static const_param_type make( void const * const pInData, UInt32 const size )
        {
            LE_ASSUME( pInData );
            LE_TRACE_IF( ( size > sizeof( value_type ) ), "Storage provided for AUHostVersionIdentifier larger than sizeof( AUHostVersionIdentifier )" );
            if ( BOOST_LIKELY( size >= sizeof( value_type ) ) )
                return *static_cast<const_value_type *>( pInData );
            else
            {
                static value_type value;
                LE_TRACE( "Storage provided for AUHostVersionIdentifier smaller than sizeof AUHostVersionIdentifier (%u < %u)", size, sizeof( value ) );
                std::memcpy( &value, pInData, size );
                std::memset( reinterpret_cast<char *>( &value ) + size, 0, sizeof( value ) - size );
                return value;
            }
        }
    }; // struct PropertyTypeTraits<::AUHostVersionIdentifier>


    template <class Impl, AudioUnitPropertyID> struct PropertyHandler;
    template <            AudioUnitPropertyID> struct PropertyTraits ;

    template <AudioUnitPropertyID PropertyID>
    struct PropertyChangeDetector
    {
        template <class Impl>
        PropertyChangeDetector( Impl & impl, AudioUnitScope const scope, AudioUnitElement const element )
        {
            BOOST_VERIFY(( PropertyHandler<Impl, PropertyID>::get( impl, scope, element, currentValue ) == noErr ));
        }
        template <class Impl>
        bool changed( Impl & impl, AudioUnitScope const scope, AudioUnitElement const element )
        {
            typedef typename PropertyTraits<PropertyID>::value_type value_type;
            value_type newValue;
            BOOST_VERIFY(( PropertyHandler<Impl, PropertyID>::get( impl, scope, element, newValue ) == noErr ));
            // http://boost.2283326.n4.nabble.com/TypeTraits-A-patch-for-clang-s-intrinsics-was-type-traits-is-enum-on-scoped-enums-doesn-t-works-as-e-td3781550.html
            static_assert( /*std::*//*is_pod*/__has_trivial_assign( value_type ), "memcmp safe only on PODs" );
            //return newValue != currentValue;
            return std::memcmp( &newValue, &currentValue, sizeof( newValue ) ) != 0;
        }

        typename PropertyTraits<PropertyID>::value_type /*const*/ currentValue;
    }; // struct PropertyChangeDetector

#ifdef _DEBUG
    #define LE_AU_PROPERTY_DEBUGGER() mutable char const * pPropertyName
    #define LE_AU_PROPERTY_DEBUGGER_INIT( functor ) functor.pPropertyName = nullptr
    #define LE_AU_PROPERTY_DEBUGGER_SET_PROPERTY( functor, propertyID ) functor.pPropertyName = PropertyTraits<propertyID>::name()
#else
    #define LE_AU_PROPERTY_DEBUGGER()
    #define LE_AU_PROPERTY_DEBUGGER_INIT( functor )
    #define LE_AU_PROPERTY_DEBUGGER_SET_PROPERTY( functor, propertyID )
#endif // _DEBUG

    /// \note AUBase::GetStreamFormat() note: "global stream description is an
    /// alias for that of output 0".
    ///                                       (08.02.2013.) (Domagoj Saric)
    template <AudioUnitPropertyID propertyID> AudioUnitScope adjustScope( AudioUnitScope const scope ) { return scope; }
    template <> inline AudioUnitScope adjustScope<kAudioUnitProperty_StreamFormat>( AudioUnitScope const scope ) { return ( scope == kAudioUnitScope_Global ) ? kAudioUnitScope_Output : scope; }
    template <> inline AudioUnitScope adjustScope<kAudioUnitProperty_SampleRate  >( AudioUnitScope const scope ) { return adjustScope<kAudioUnitProperty_StreamFormat>( scope ); }


    #define LE_AU_PROPERTIES_SELECT_FUNCTION_CASE_AUX( propertyID )                                        \
        case propertyID:                                                                                   \
            LE_AU_PROPERTY_DEBUGGER_SET_PROPERTY( functor, propertyID );                                   \
            return functor. template apply<propertyID>( impl, adjustScope<propertyID>( scope ), element );

    #define LE_AU_PROPERTIES_SELECT_FUNCTION_CASE( r, data, propertyInfoTuple )  \
        LE_AU_PROPERTIES_SELECT_FUNCTION_CASE_AUX( BOOST_PP_TUPLE_ELEM( 7, 0, propertyInfoTuple ) )

    #define LE_AU_PROPERTIES_SELECT_FUNCTION( propertiesSequence )                                                                                                                             \
        template <class Impl, class Functor>                                                                                                                                                   \
        OSStatus LE_FORCEINLINE LE_NOTHROW handleProperty( Impl & impl, AudioUnitPropertyID const propertyID, AudioUnitScope const scope, AudioUnitElement const element, Functor & functor )  \
        {                                                                                                                                                                                      \
            LE_AU_PROPERTY_DEBUGGER_INIT( functor );                                                                                                                                           \
            switch ( propertyID ) { BOOST_PP_SEQ_FOR_EACH( LE_AU_PROPERTIES_SELECT_FUNCTION_CASE, 0, propertiesSequence ) }                                                                    \
            return kAudio_UnimplementedError;                                                                                                                                                  \
        }

    #define LE_AU_PROPERTIES_TRAITS( r, data, propertyInfoTuple )                                                                               \
        template <> struct PropertyTraits<BOOST_PP_TUPLE_ELEM( 7, 0, propertyInfoTuple )>                                                       \
        {                                                                                                                                       \
            using TypeTraits = PropertyTypeTraits<BOOST_PP_TUPLE_ELEM( 7, 1, propertyInfoTuple )>;                                              \
            using       value_type = typename TypeTraits::      value_type;                                                                     \
            using       param_type = typename TypeTraits::      param_type;                                                                     \
            using const_param_type = typename TypeTraits::const_param_type;                                                                     \
                                                                                                                                                \
            static CanDo          constexpr readable    = BOOST_PP_TUPLE_ELEM( 7, 3, propertyInfoTuple );                                       \
            static CanDo          constexpr writable    = BOOST_PP_TUPLE_ELEM( 7, 2, propertyInfoTuple );                                       \
            static AudioUnitScope constexpr scope0      = BOOST_PP_TUPLE_ELEM( 7, 4, propertyInfoTuple );                                       \
            static AudioUnitScope constexpr scope1      = BOOST_PP_TUPLE_ELEM( 7, 5, propertyInfoTuple );                                       \
            static bool           constexpr implemented = BOOST_PP_TUPLE_ELEM( 7, 6, propertyInfoTuple );                                       \
                                                                                                                                                \
            static char const * name() { return std::strchr( BOOST_PP_STRINGIZE( BOOST_PP_TUPLE_ELEM( 7, 0, propertyInfoTuple ) ), '_' ) + 1; } \
        };


    #define LE_AU_PROPERTIES( propertiesSequence ) \
        BOOST_PP_SEQ_FOR_EACH( LE_AU_PROPERTIES_TRAITS, 0, propertiesSequence ) \
        LE_AU_PROPERTIES_SELECT_FUNCTION( propertiesSequence )

    // http://developer.apple.com/library/mac/#documentation/AudioUnit/Reference/AudioUnitPropertiesReference/Reference/reference.html
    // http://developer.apple.com/library/mac/#documentation/musicaudio/reference/CoreAudioDataTypesRef/Reference/reference.html
    // http://developer.apple.com/documentation/MusicAudio/Conceptual/AudioUnitProgrammingGuide/TheAudioUnit/TheAudioUnit.html#//apple_ref/doc/uid/TP40003278-CH12-SW10
    // https://developer.apple.com/library/mac/qa/qa1684/_index.html Audio Unit Properties and Core Foundation Data Types
    LE_AU_PROPERTIES
    (   // 0: ID                                          1: type                               2: w           3: r  4: scope0               5: scope1             6:implemented
        (( kAudioUnitProperty_ClassInfo                 , ::CFPropertyListRef                   , Yes         , Yes, kAudioUnitScope_Global, kAudioUnitScope_Part  , 1 ))
        (( kAudioUnitProperty_MakeConnection            , ::AudioUnitConnection                 , Yes         , No , kAudioUnitScope_Input , -1                    , 1 ))
        (( kAudioUnitProperty_SampleRate                , ::Float64                             , Yes         , Yes, kAudioUnitScope_Input , kAudioUnitScope_Output, 1 ))
        (( kAudioUnitProperty_ParameterList             , Array<::AudioUnitParameterID>         , No          , Yes, -1                    , -1                    , 1 ))
        (( kAudioUnitProperty_ParameterInfo             , ::AudioUnitParameterInfo              , No          , Yes, -1                    , -1                    , 1 ))
        (( kAudioUnitProperty_CPULoad                   , ::Float64                             , No          , Yes, kAudioUnitScope_Global, -1                    , 0 ))
        (( kAudioUnitProperty_StreamFormat              , ::AudioStreamBasicDescription         , Yes         , Yes, kAudioUnitScope_Input , kAudioUnitScope_Output, 1 ))
        (( kAudioUnitProperty_ElementCount              , ::UInt32                              , RuntimeQuery, Yes, -1                    , -1                    , 1 ))
        (( kAudioUnitProperty_Latency                   , ::Float64                             , No          , Yes, kAudioUnitScope_Global, -1                    , 1 ))
        (( kAudioUnitProperty_SupportedNumChannels      , Array<::AUChannelInfo>                , No          , Yes, kAudioUnitScope_Global, -1                    , 1 ))
        (( kAudioUnitProperty_MaximumFramesPerSlice     , ::UInt32                              , Yes         , Yes, kAudioUnitScope_Global, -1                    , 1 ))
        (( kAudioUnitProperty_ParameterValueStrings     , ::CFArrayRef                          , No          , Yes, kAudioUnitScope_Global, -1                    , 1 ))
        (( kAudioUnitProperty_AudioChannelLayout        , ::AudioChannelLayout                  , Yes         , Yes, kAudioUnitScope_Input , kAudioUnitScope_Output, 0 ))
        (( kAudioUnitProperty_TailTime                  , ::Float64                             , No          , Yes, kAudioUnitScope_Global, -1                    , 1 ))
        (( kAudioUnitProperty_BypassEffect              , ::UInt32                              , Yes         , Yes, kAudioUnitScope_Global, -1                    , 0 ))
        (( kAudioUnitProperty_LastRenderError           , ::OSStatus                            , No          , Yes, kAudioUnitScope_Global, -1                    , 1 ))
        (( kAudioUnitProperty_SetRenderCallback         , ::AURenderCallbackStruct              , Yes         , No , kAudioUnitScope_Input , -1                    , 1 ))
        (( kAudioUnitProperty_FactoryPresets            , ::CFArrayRef /*AUPreset*/             , No          , Yes, kAudioUnitScope_Global, -1                    , 0 ))
        (( kAudioUnitProperty_RenderQuality             , ::UInt32                              , Yes         , Yes, kAudioUnitScope_Global, -1                    , 0 ))
        (( kAudioUnitProperty_InPlaceProcessing         , ::UInt32                              , Yes         , Yes, kAudioUnitScope_Global, -1                    , 1 ))
        (( kAudioUnitProperty_ElementName               , ::CFStringRef                         , Yes         , Yes, -1                    , -1                    , 1 ))
        (( kAudioUnitProperty_SupportedChannelLayoutTags, Array<::AudioChannelLayoutTag>        , No          , Yes, kAudioUnitScope_Input , kAudioUnitScope_Output, 0 ))
        (( kAudioUnitProperty_PresentPreset             , ::AUPreset                            , Yes         , Yes, kAudioUnitScope_Global, kAudioUnitScope_Part  , 1 ))
        (( kAudioUnitProperty_ShouldAllocateBuffer      , ::UInt32                              , Yes         , Yes, kAudioUnitScope_Input , kAudioUnitScope_Output, 1 ))
      //(( kAudioUnitProperty_ParameterHistoryInfo      , ::AudioUnitConnection                 , Yes         , Yes, kAudioUnitScope_Global, -1                    , 0 )) Lion
    //#if !TARGET_OS_IPHONE
      //(( kAudioUnitProperty_FastDispatch              , void *                                , No          , Yes, kAudioUnitScope_Global, -1                    , 0 ))
        (( kAudioUnitProperty_SetExternalBuffer         , ::AudioUnitExternalBuffer             , Yes         , No , kAudioUnitScope_Global, -1                    , 0 ))
      //(( kAudioUnitProperty_GetUIComponentList        , Array<::AudioComponentDescription>    , Yes         , No , -1                    , -1                    , 0 ))
        (( kAudioUnitProperty_ContextName               , ::CFStringRef                         , Yes         , Yes, kAudioUnitScope_Global, -1                    , 0 ))
        (( kAudioUnitProperty_HostCallbacks             , ::HostCallbackInfo                    , Yes         , Yes, kAudioUnitScope_Global, -1                    , 1 )) // not readable according to documentation but yes according to OSX10.6 auval and AUBase.cpp
        (( kAudioUnitProperty_CocoaUI                   , ::AudioUnitCocoaViewInfo              , No          , Yes, kAudioUnitScope_Global, -1                    , 1 ))
        (( kAudioUnitProperty_ParameterIDName           , ::AudioUnitParameterNameInfo          , No          , Yes, -1                    , -1                    , 1 )) //...mrmlj...enabled only for testing host behaviour...
        (( kAudioUnitProperty_ParameterClumpName        , ::AudioUnitParameterNameInfo          , No          , Yes, -1                    , -1                    , 0 ))
        (( kAudioUnitProperty_ParameterStringFromValue  , ::AudioUnitParameterStringFromValue   , No          , Yes, -1                    , -1                    , 1 ))
        (( kAudioUnitProperty_OfflineRender             , ::UInt32                              , Yes         , Yes, kAudioUnitScope_Global, -1                    , 0 ))
        (( kAudioUnitProperty_ParameterValueFromString  , ::AudioUnitParameterValueFromString   , No          , Yes, -1                    , -1                    , 0 ))
        (( kAudioUnitProperty_IconLocation              , ::CFURLRef                            , No          , Yes, kAudioUnitScope_Global, -1                    , 0 ))
        (( kAudioUnitProperty_PresentationLatency       , ::Float64                             , Yes         , No , kAudioUnitScope_Input , kAudioUnitScope_Output, 0 ))
        (( kAudioUnitProperty_DependentParameters       , Array<::AUDependentParameter>         , No          , Yes, -1                    , -1                    , 1 ))
        (( kAudioUnitProperty_AUHostIdentifier          , ::AUHostVersionIdentifier             , Yes         , No , kAudioUnitScope_Global, -1                    , 1 ))
        (( kAudioUnitProperty_MIDIOutputCallbackInfo    , ::CFArrayRef                          , No          , Yes, kAudioUnitScope_Global, -1                    , 0 ))
        (( kAudioUnitProperty_MIDIOutputCallback        , ::AUMIDIOutputCallbackStruct          , Yes         , No , kAudioUnitScope_Global, -1                    , 0 ))
        (( kAudioUnitProperty_InputSamplesInOutput      , ::AUInputSamplesInOutputCallbackStruct, Yes         , Yes, kAudioUnitScope_Global, -1                    , 0 ))
        (( kAudioUnitProperty_ClassInfoFromDocument     , ::CFDictionaryRef                     , Yes         , Yes, kAudioUnitScope_Global, -1                    , 0 ))
        (( kAudioUnitProperty_FrequencyResponse         , ::AudioUnitFrequencyResponseBin       , No          , Yes, kAudioUnitScope_Global, -1                    , 0 ))

        (( Detail::auViewPropertyID                     , ObjC::NSView *                        , Yes         , No , -1                    , -1                    , 1 ))
    //#endif
    )


    ////////////////////////////////////////////////////////////////////////////
    ///
    /// Property accessor functors
    ///
    ////////////////////////////////////////////////////////////////////////////

    static void verifyScope( AudioUnitScope const specifiedScope, AudioUnitScope const allowedScope1, AudioUnitScope const allowedScope2 )
    {
        BOOST_ASSERT_MSG
        (
            ( specifiedScope == allowedScope1 ) ||
            ( specifiedScope == allowedScope2 ) ||
            ( allowedScope1  == -1            ),
            "Host specified an invalid property scope."
        );
        boost::ignore_unused_variable_warning( specifiedScope );
        boost::ignore_unused_variable_warning( allowedScope1  );
        boost::ignore_unused_variable_warning( allowedScope2  );
    }

    template <AudioUnitScope allowedScope1, AudioUnitScope allowedScope2>
    static OSStatus checkScope( AudioUnitScope const specifiedScope )
    {
        if ( ( allowedScope1 == -1 ) && ( allowedScope2 == -1 ) )
            return noErr;
        if ( specifiedScope == allowedScope1 )
            return noErr;
        if ( ( allowedScope2 != -1 ) && ( specifiedScope == allowedScope2) )
            return noErr;
        return kAudioUnitErr_InvalidScope;
    }

    struct PropertyInfoGetter
    {
        using result_type = OSStatus;

        template <unsigned PropertyID, class Impl>
        result_type apply( Impl const & impl, ::AudioUnitScope const scope, ::AudioUnitElement const element )
        {
            using namespace Detail;

            using Handler    = Detail::PropertyTraits<PropertyID>;
            using value_type = typename Handler::value_type;

            //LE_ASSUME( element == 0 || element == 1 );
            //verifyScope( scope, Handler::scope0, Handler::scope1 );
            OSStatus const scopeResult( checkScope<Handler::scope0, Handler::scope1>( scope ) );
            if ( scopeResult != noErr )
            {
                LE_TRACE( "AU: Invalid scope (" LE_OSX_INT_FORMAT( u ) ") for property (%s).", scope, Handler::name() );
                return scopeResult;
            }

            dataSize = PropertyValueCount<PropertyID>::get( impl, scope, element ) * sizeof( value_type );
            writable = Handler::writable;
            //...mrmlj..."runtime query" for Logic (tries to set output bus count)...
            if ( PropertyID == kAudioUnitProperty_ElementCount && scope != kAudioUnitScope_Input )
                writable = false;

            return Handler::implemented ? noErr : kAudio_UnimplementedError;
        }

        // "Some properties that have read/write access when an audio unit is
        // uninitialized become read-only when the audio unit is initialized."
        // https://developer.apple.com/library/prerelease/mac/documentation/AudioUnit/Reference/AUComponentServicesReference/index.html#//apple_ref/c/func/AudioUnitGetPropertyInfo
        UInt32 dataSize;
        bool   writable;
        LE_AU_PROPERTY_DEBUGGER();
    }; // struct PropertyInfoGetter


    struct PropertyGetter
    {
        using result_type = ::OSStatus;

        template <unsigned PropertyID, class Impl>
        result_type apply( Impl const & impl, ::AudioUnitScope const scope, ::AudioUnitElement const element ) const
        {
            using namespace Detail;

            using Handler    = Detail::PropertyTraits<PropertyID>;
            using value_type = typename Handler::value_type;

            //LE_ASSUME( element == 0 || element == 1 );
            //verifyScope( scope, Handler::scope0, Handler::scope1 );
            OSStatus const scopeResult( checkScope<Handler::scope0, Handler::scope1>( scope ) );
            if ( scopeResult != noErr )
                return LE_TRACE_RETURN( scopeResult, "\tSW AU: Invalid scope (" LE_OSX_INT_FORMAT( u ) ") for property (%s).", scope, Handler::name() );

            LE_ASSUME(( Handler::readable ));

            return apply<PropertyID>( impl, scope, element, std::integral_constant<bool, Handler::implemented && Handler::readable>() );
        }

        template <unsigned PropertyID, class Impl>
        result_type apply( Impl const & impl, ::AudioUnitScope const scope, ::AudioUnitElement const element, std::true_type ) const
        {
            using namespace Detail;
            using Handler    = PropertyTraits<PropertyID>  ;
            using value_type = typename Handler::value_type;
            if ( dataSize < sizeof( value_type ) )
                return LE_TRACE_RETURN( kAudioUnitErr_InvalidParameter, "\tSW AU: Too small property (%s) buffer size (required %lu, provided %u).", Handler::name(), sizeof( value_type ), static_cast<unsigned int>( dataSize ) );
            if ( Handler::TypeTraits::array )
            {
                UInt32 const minimumSize( PropertyValueCount<PropertyID>::get( impl, scope, element ) * sizeof( value_type ) );
                LE_ASSUME( dataSize >= minimumSize );
                dataSize = std::min( dataSize, minimumSize );
            }

            return PropertyHandler<Impl, PropertyID>::get( impl, scope, element, Handler::TypeTraits::make( pOutData, dataSize ) );
        }

        template <unsigned PropertyID, class Impl>
        result_type apply( Impl const &, ::AudioUnitScope, ::AudioUnitElement, std::false_type ) const { return kAudio_UnimplementedError; }

                void   * const pOutData;
        mutable UInt32         dataSize;
        LE_AU_PROPERTY_DEBUGGER();
    }; // struct PropertyGetter


    struct PropertySetter
    {
        using result_type = OSStatus;

        template <unsigned PropertyID, class Impl>
        result_type apply( Impl & impl, AudioUnitScope const scope, AudioUnitElement const element ) const
        {
            using namespace Detail;

            typedef typename PropertyTraits<PropertyID>::value_type value_type;

            LE_ASSUME(( PropertyTraits<PropertyID>::writable != No ));

            return apply<PropertyID>( impl, scope, element, std::integral_constant<bool, PropertyTraits<PropertyID>::implemented && PropertyTraits<PropertyID>::writable != No>() );
        }

        template <unsigned PropertyID, class Impl>
        result_type apply( Impl & impl, AudioUnitScope const scope, AudioUnitElement const element, std::true_type ) const
        {
            using namespace Detail;

            PropertyChangeDetector<PropertyID> changeDetector( impl, scope, element );

            result_type const result( PropertyHandler<Impl, PropertyID>::set( impl, scope, element, PropertyTraits<PropertyID>::TypeTraits::make( pOutData, dataSize ) ) );

            changed = changeDetector.changed( impl, scope, element );

            return result;
        }

        template <unsigned PropertyID, class Impl>
        result_type apply( Impl const &, AudioUnitScope, AudioUnitElement, std::false_type ) const { return kAudio_UnimplementedError; }

                void   const * const pOutData;
                UInt32         const dataSize;
        mutable bool                 changed ;
        LE_AU_PROPERTY_DEBUGGER();
    }; // struct PropertySetter


    ////////////////////////////////////////////////////////////////////////////
    ///
    /// Property handler implementations
    ///
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // Format
    ////////////////////////////////////////////////////////////////////////////

    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_SampleRate>
    {
        static OSStatus get( Impl const & impl, AudioUnitScope, AudioUnitElement, ::Float64       & value ) { value = impl.getSampleRate(); return noErr;          }
        static OSStatus set( Impl       & impl, AudioUnitScope, AudioUnitElement, ::Float64 const & value ) { return makeErrorCode( impl.setSampleRate( value ) ); }
    };

    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_StreamFormat>
    {
        static OSStatus get( Impl const & impl, AudioUnitScope const scope, AudioUnitElement const element, ::AudioStreamBasicDescription & format )
        {
            //...mrmlj...SW HARDCODE...
            BOOST_ASSERT_MSG( element == 0                   || element == 1, "Invalid element" );
            BOOST_ASSERT_MSG( scope == kAudioUnitScope_Input || element == 0, "Invalid element" );
            boost::ignore_unused_variable_warning( element );

            format.mFormatID        = kAudioFormatLinearPCM;
            format.mFormatFlags     = kAudioFormatFlagIsFloat | kAudioFormatFlagIsNonInterleaved | kAudioFormatFlagIsPacked; // kAudioFormatFlagIsBigEndian
            format.mBytesPerPacket  = sizeof( float );
            format.mFramesPerPacket = 1;
            format.mBytesPerFrame   = sizeof( float );
            format.mBitsPerChannel  = sizeof( float ) * 8;
            format.mReserved        = 0;

            format.mSampleRate      = impl.getSampleRate();

            std::uint8_t const * LE_RESTRICT pSourceNumberOfChannels;
            if ( scope == kAudioUnitScope_Input )
            {
                auto & hostProxy( impl.host() );
                if ( element == 0 ) { pSourceNumberOfChannels = &hostProxy.inputChannels_; /*BOOST_ASSERT( *pSourceNumberOfChannels == impl.numberOfInputChannels() );*/ }
                else                { pSourceNumberOfChannels = &hostProxy.sideChannels_ ; /*BOOST_ASSERT( *pSourceNumberOfChannels == impl.numberOfSideChannels () );*/ }
            }
            else
            {
                LE_ASSUME( scope   == kAudioUnitScope_Output );
                LE_ASSUME( element == 0                      );
                pSourceNumberOfChannels = &impl.host().outputChannels_;
                //BOOST_ASSERT( *pSourceNumberOfChannels == impl.numberOfOutputChannels() );
            }

            // http://forum.cockos.com/showthread.php?p=1065630
            //...mrmlj...auval asks for the bus format before calling initialise...
            //...mrmlj...if the plugin was not yet initialised...
            format.mChannelsPerFrame = *pSourceNumberOfChannels;
            return noErr;
        }

        static OSStatus set( Impl & impl, AudioUnitScope const scope, AudioUnitElement const element, ::AudioStreamBasicDescription const & format )
        {
            //...mrmlj...SW HARDCODE...
            BOOST_ASSERT_MSG( element == 0                     || element == 1, "Invalid element" );
            BOOST_ASSERT_MSG( scope   == kAudioUnitScope_Input || element == 0, "Invalid element" );
            boost::ignore_unused_variable_warning( element );

            BOOST_ASSERT_MSG
            (
                ( format.mFormatID        == kAudioFormatLinearPCM                                                                     ) &&
                ( format.mFormatFlags     == ( kAudioFormatFlagIsFloat | kAudioFormatFlagIsNonInterleaved | kAudioFormatFlagIsPacked ) ) &&
                ( format.mBytesPerPacket  == sizeof( float )                                                                           ) &&
                ( format.mFramesPerPacket == 1                                                                                         ) &&
                ( format.mBytesPerFrame   == sizeof( float )                                                                           ) &&
                ( format.mBitsPerChannel  == sizeof( float ) * 8                                                                       ) &&
                ( format.mReserved        == 0 || /*Audacity 2.0.3*/ format.mReserved == 4                                             ),
                "Invalid or unsupported AudioStreamBasicDescription"
            );

            std::uint8_t * LE_RESTRICT pTargetNumberOfChannels;
            if ( scope == kAudioUnitScope_Input )
            {
                auto & hostProxy( impl.host() );
                if ( element == 0 ) pTargetNumberOfChannels = &hostProxy.inputChannels_;
                else                pTargetNumberOfChannels = &hostProxy.sideChannels_ ;
            }
            else
            {
                LE_ASSUME( scope   == kAudioUnitScope_Output );
                LE_ASSUME( element == 0                      );
                pTargetNumberOfChannels = &impl.outputChannels_;
            }

            auto const numberOfChannels( static_cast<std::uint_fast8_t>( format.mChannelsPerFrame ) );

            bool const formatChanged
            (
                ( format.mSampleRate != impl.getSampleRate()     ) ||
                ( numberOfChannels   != *pTargetNumberOfChannels )
            );
            if ( !formatChanged )
                return noErr;

            if ( impl.initialised_ )
                return LE_TRACE_RETURN
                (
                    kAudioUnitErr_PropertyNotWritable /*kAudioUnitErr_Initialized*/,
                    "\tSW AU: host trying to change stream format while initialised."
                );

            BOOST_ASSERT_MSG
            (
                impl.inputConnections_[ element ].type() != AUPluginBase::RenderDelegate::Connection,
                "Cannot change input format while connected."
            );

            /// \note Channel configuration changes are not passed directly to
            /// the plugin because the AU protocol supports only separate format
            /// changes for input and output and some plugins (e.g. SW) accept
            /// only specific IO channel count combinations. For this reason
            /// channel count/IO mode setting is deffered to initialisation
            /// time.
            ///                               (27.08.2014.) (Domagoj Saric)
            *pTargetNumberOfChannels = numberOfChannels;

            return makeErrorCode( impl.setSampleRate( format.mSampleRate ) );
        }
    }; // kAudioUnitProperty_StreamFormat

    template<> struct PropertyChangeDetector<kAudioUnitProperty_StreamFormat>
    {
        template <class Impl>
        PropertyChangeDetector( Impl const & impl, AudioUnitScope, AudioUnitElement )
            :
            sampleRate          ( impl.getSampleRate        () ),
            numberOfChannels    ( impl.numberOfInputChannels() ),
            numberOfSideChannels( impl.numberOfSideChannels () )
        {}

        template <class Impl>
        bool changed( Impl const & impl, AudioUnitScope, AudioUnitElement )
        {
            return
                ( sampleRate           != impl.getSampleRate        () ) ||
                ( numberOfChannels     != impl.numberOfInputChannels() ) ||
                ( numberOfSideChannels != impl.numberOfSideChannels () );
        }

        float             const sampleRate          ;
        std::uint_fast8_t const numberOfChannels    ;
        std::uint_fast8_t const numberOfSideChannels;
    }; // PropertyChangeDetector<kAudioUnitProperty_StreamFormat>


    ////////////////////////////////////////////////////////////////////////////
    // IO modes
    ////////////////////////////////////////////////////////////////////////////

    // Sidechaining:
    // - http://lists.apple.com/archives/coreaudio-api/2005/May/msg00211.html
    // - http://lists.apple.com/archives/coreaudio-api/2008/Oct/msg00165.html
    // - http://lists.apple.com/archives/coreaudio-api/2012/Jan/msg00046.html
    // - http://www.mailinglistarchive.com/html/coreaudio-api@lists.apple.com/2012-02/msg00249.html
    // - http://osdir.com/ml/coreaudio-api/2010-01/msg00082.html
    // - http://www.kvraudio.com/forum/viewtopic.php?t=312717
    // - http://www.soundonsound.com/sos/apr05/articles/logicnotes.htm

    // http://lists.apple.com/archives/coreaudio-api/2008/Sep/msg00109.html

    template <> struct PropertyValueCount<kAudioUnitProperty_SupportedNumChannels> { template <class Impl> static unsigned int get( Impl const &, ::AudioUnitScope, ::AudioUnitElement ) { return 1; } };
    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_SupportedNumChannels>
    {
        static OSStatus get( Impl const & impl, AudioUnitScope, AudioUnitElement const bus, boost::iterator_range<::AUChannelInfo *> const channelConfigurations )
        {
            BOOST_ASSERT_MSG( channelConfigurations.size() == 1, "Incorrect AUChannelInfo array size." );
            channelConfigurations.front().inChannels  =                -1    ;
            channelConfigurations.front().outChannels = ( bus == 0 ) ? -1 : 0;
            return noErr;
        }
    }; // kAudioUnitProperty_SupportedNumChannels


    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_ElementCount>
    {
        static OSStatus get( Impl const & impl, ::AudioUnitScope const scope, ::AudioUnitElement, ::UInt32 & value )
        {
            auto & hostProxy( impl.host() );
            //...mrmlj...SW HARDCODE...
            switch ( scope )
            {
                case kAudioUnitScope_Input : value = hostProxy.sideChannels_ ? 2 : 1; break;
                case kAudioUnitScope_Output: value =                               1; break;
                default:                     value =                               0; break;
                case kAudioUnitScope_Global: //...mrmlj...StudioOne 2.6.3...LE_UNREACHABLE_CODE();
                                             LE_TRACE( "AU: host asked for Global scope element count" );
                                             value = 1; break;
            }
            return noErr;
        }

        static OSStatus set( Impl & impl, ::AudioUnitScope const scope, ::AudioUnitElement, ::UInt32 const value )
        {
            //...mrmlj...SW HARDCODE...
            if ( scope != kAudioUnitScope_Input )
                return kAudioUnitErr_InvalidScope;
            if ( value > 2 ) // Ableton Live 8.3.4
                return kAudioUnitErr_InvalidPropertyValue;
            auto & hostProxy( impl.host() );
            hostProxy.sideChannels_ = ( value == 2 ) ? hostProxy.inputChannels_ : 0;
            return impl.checkSideChannelBusStateChange();
        }
    }; // kAudioUnitProperty_ElementCount


    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_ElementName>
    {
        static OSStatus get( Impl const & impl, ::AudioUnitScope const scope, ::AudioUnitElement const bus, ::CFStringRef & name )
        {
            BOOST_ASSERT_MSG
            (
                scope == kAudioUnitScope_Input || ( scope == kAudioUnitScope_Output && bus == 0 ),
                "Invalid scope."
            );
            switch ( bus )
            {
                case 0: name = CFSTR( "main" ); break;
                case 1: name = CFSTR( "side" ); break;
                LE_DEFAULT_CASE_UNREACHABLE();
            }
            return noErr;
        }
        static OSStatus set( Impl & impl, ::AudioUnitScope, ::AudioUnitElement const bus, ::CFStringRef /*name*/ )
        {
            return kAudio_UnimplementedError;
        }
    }; // kAudioUnitProperty_ElementName


    ////////////////////////////////////////////////////////////////////////////
    // ...
    ////////////////////////////////////////////////////////////////////////////

    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_Latency>
    {
        static OSStatus get( Impl const & impl, AudioUnitScope, AudioUnitElement, ::Float64 & value ) { value = impl.engineSetup().latencyInMilliseconds() / 1000; return noErr; }
    }; // kAudioUnitProperty_Latency

    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_TailTime>
    {
        static OSStatus get( Impl const & impl, AudioUnitScope, AudioUnitElement, ::Float64 & value ) { value = Impl::maxTailSize; return noErr; }
    }; // kAudioUnitProperty_TailTime

    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_MaximumFramesPerSlice>
    {
        static OSStatus get( Impl const & impl, AudioUnitScope, AudioUnitElement, ::UInt32       & value ) { value = impl.processBlockSize(); return noErr; }
        static OSStatus set( Impl       & impl, AudioUnitScope, AudioUnitElement, ::UInt32 const   value ) { BOOST_ASSERT( !impl.initialised_ ); return makeErrorCode( impl.setBlockSize( value ) ); }
    }; // kAudioUnitProperty_MaximumFramesPerSlice

    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_LastRenderError>
    {
        static OSStatus get( Impl const & impl, AudioUnitScope, AudioUnitElement, ::OSStatus & value ) { value = impl.lastRenderError_; return noErr; }
    }; // kAudioUnitProperty_LastRenderError


    ////////////////////////////////////////////////////////////////////////////
    // Callbacks and connections
    ////////////////////////////////////////////////////////////////////////////

    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_HostCallbacks>
    {
        // Write only property but get required for change detection/notification...
        static OSStatus get( Impl const & impl, AudioUnitScope, AudioUnitElement, ::HostCallbackInfo       & hostCallbacks ) { hostCallbacks = impl.host().callbacks(); return noErr; }
        static OSStatus set( Impl       & impl, AudioUnitScope, AudioUnitElement, ::HostCallbackInfo const & hostCallbacks ) { impl.host().callbacks() = hostCallbacks; return noErr; }
    }; // kAudioUnitProperty_HostCallbacks


    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_SetRenderCallback>
    {
        static OSStatus set( Impl & impl, ::AudioUnitScope, ::AudioUnitElement const bus, ::AURenderCallbackStruct const & callback )
        {
            AUPluginBase::RenderDelegate & delegate( impl.inputConnections_[ bus ] );
            bool const connectionOverwritten( delegate.type() == AUPluginBase::RenderDelegate::Connection );
            delegate = std::make_pair( callback, bus );
            if ( connectionOverwritten )
                impl.propertyChanged( kAudioUnitProperty_MakeConnection, kAudioUnitScope_Input, bus );
            return impl.checkSideChannelBusStateChange();
        }
    }; // kAudioUnitProperty_SetRenderCallback

    template <>
    struct PropertyChangeDetector<kAudioUnitProperty_SetRenderCallback>
    {
        PropertyChangeDetector( AUPluginBase const & plugin, AudioUnitScope, AudioUnitElement const bus ) : currentDelegate( plugin.inputConnections_[ bus ] ) {}
        bool changed( AUPluginBase const & plugin, AudioUnitScope, AudioUnitElement const bus ) const
        {
            return std::memcmp( &currentDelegate, &plugin.inputConnections_[ bus ], sizeof( currentDelegate ) ) == 0;
        }
        AUPluginBase::RenderDelegate const currentDelegate;
    }; // PropertyChangeDetector<kAudioUnitProperty_SetRenderCallback>


    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_MakeConnection>
    {
        static ::OSStatus set( Impl & impl, ::AudioUnitScope, ::AudioUnitElement const bus, ::AudioUnitConnection const & connection )
        {
            BOOST_ASSERT_MSG( connection.destInputNumber == bus, "Inconsistent connection input bus." );

            if ( connection.sourceAudioUnit )
            {
                ::AudioStreamBasicDescription format;
                ::UInt32 size( sizeof( format ) );
                ::OSStatus const getFormatResult
                (
                    ::AudioUnitGetProperty
                    (
                        connection.sourceAudioUnit,
                        kAudioUnitProperty_StreamFormat,
                        kAudioUnitScope_Output,
                        connection.sourceOutputNumber,
                        &format,
                        &size
                    )
                );
                if ( getFormatResult != noErr )
                    return getFormatResult;
                BOOST_ASSERT_MSG( size == sizeof( format ), "Unexpected result size." );

                ::OSStatus const setFormatResult
                (
                    PropertyHandler<Impl, kAudioUnitProperty_StreamFormat>::set( impl, kAudioUnitScope_Input, bus, format )
                );
                if ( setFormatResult != noErr )
                    return setFormatResult;

            #ifdef _DEBUG
                ::UInt32 framesPerSlice;
                size = sizeof( framesPerSlice );
                ::OSStatus const getFramesPerSliceResult
                (
                    ::AudioUnitGetProperty
                    (
                        connection.sourceAudioUnit,
                        kAudioUnitProperty_MaximumFramesPerSlice,
                        kAudioUnitScope_Global,
                        connection.sourceOutputNumber,
                        &framesPerSlice,
                        &size
                    )
                );
                BOOST_ASSERT_MSG( getFramesPerSliceResult == noErr         , "Unexpected error."                                  );
                BOOST_ASSERT_MSG( size == sizeof( framesPerSlice )         , "Unexpected result size."                            );
                BOOST_ASSERT_MSG( framesPerSlice >= impl.processBlockSize(), "Connecting AU has a too small maximum buffer size." );
            #endif // _DEBUG
            }

            AUPluginBase::RenderDelegate & delegate( impl.inputConnections_[ bus ] );
            bool const callbackOverwritten( delegate.type() == AUPluginBase::RenderDelegate::Callback );

            AURenderCallbackStruct callback;
            ::UInt32 size( sizeof( callback.inputProc ) );
            OSStatus const getFastRenderResult
            (
                ::AudioUnitGetProperty
                (
                    connection.sourceAudioUnit,
                    kAudioUnitProperty_FastDispatch,
                    kAudioUnitScope_Global,
                    kAudioUnitRenderSelect,
                    &callback.inputProc,
                    &size
                )
            );
            BOOST_ASSERT_MSG( size == sizeof( callback.inputProc ), "Unexpected result size." );
            if ( getFastRenderResult == noErr )
            {
                callback.inputProcRefCon = ::GetComponentInstanceStorage( connection.sourceAudioUnit );
                BOOST_ASSERT_MSG( callback.inputProcRefCon, "Null connecting AU instance." );
                delegate = std::make_pair( callback, connection.sourceOutputNumber );
            }
            else
            {
                BOOST_ASSERT_MSG( ( getFastRenderResult == kAudioUnitErr_InvalidProperty ) || !connection.sourceAudioUnit, "Unexpected error code." );
                delegate = connection;
            }

            if ( callbackOverwritten )
                impl.propertyChanged( kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, bus );
            return impl.checkSideChannelBusStateChange();
        }
    }; // kAudioUnitProperty_MakeConnection

    template <>
    struct PropertyChangeDetector<kAudioUnitProperty_MakeConnection> : PropertyChangeDetector<kAudioUnitProperty_SetRenderCallback>
    {
        using PropertyChangeDetector<kAudioUnitProperty_SetRenderCallback>::PropertyChangeDetector;
    };


    ////////////////////////////////////////////////////////////////////////////
    // Buffering policies
    ////////////////////////////////////////////////////////////////////////////

    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_ShouldAllocateBuffer>
    {
        static OSStatus get( Impl const & impl, AudioUnitScope const scope, AudioUnitElement const element, ::UInt32       & value )
        {
            BOOST_ASSERT( !impl.initialised_ );
            BOOST_ASSERT_MSG( element == 0 || scope == kAudioUnitScope_Input, "Invalid element." );
            value =
                ( scope == kAudioUnitScope_Output )
                    ? impl.shouldAllocateOutputBuffer_
                    : impl.inputConnections_[ element ].getShouldAllocateBuffer();
            return noErr;
        }
        static OSStatus set( Impl       & impl, AudioUnitScope const scope, AudioUnitElement const element, ::UInt32 const & value )
        {
            BOOST_ASSERT_MSG( element == 0 || scope == kAudioUnitScope_Input, "Invalid element." );
            bool const should( value != 0 );
            if ( scope == kAudioUnitScope_Output ) impl.shouldAllocateOutputBuffer_ = should;
            else                                   impl.inputConnections_[ element ].setShouldAllocateBuffer( should );
            return noErr;
        }
    }; // kAudioUnitProperty_ShouldAllocateBuffer

    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_InPlaceProcessing>
    {   // http://osdir.com/ml/coreaudio-api/2010-02/msg00042.html
        static OSStatus get( Impl const & impl, AudioUnitScope, AudioUnitElement, ::UInt32       & value ) { value = impl.inPlaceProcessing_         ; return noErr; }
        static OSStatus set( Impl       & impl, AudioUnitScope, AudioUnitElement, ::UInt32 const & value ) { impl.inPlaceProcessing_ = ( value != 0 ); return noErr; }
    }; // kAudioUnitProperty_InPlaceProcessing


    ////////////////////////////////////////////////////////////////////////////
    ///
    /// Preset/state properties
    ///
    ////////////////////////////////////////////////////////////////////////////
    /// https://www.ableton.com/en/articles/au-preset-handling
    ////////////////////////////////////////////////////////////////////////////

    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_PresentPreset>
    {
        static OSStatus get( Impl const & impl, AudioUnitScope, AudioUnitElement, ::AUPreset & preset )
        {
            preset.presetNumber = -1;
            preset.presetName   = ::CFStringCreateWithCString( nullptr, impl.currentProgramName(), kCFStringEncodingUTF8 );
            return noErr;
        }
        static OSStatus set( Impl & impl, AudioUnitScope, AudioUnitElement, ::AUPreset const & preset )
        {
            BOOST_ASSERT( preset.presetName   != nullptr );
            BOOST_ASSERT( preset.presetNumber == -1      );
            char const * const pNewPresetName( ::CFStringGetCStringPtr( preset.presetName, kCFStringEncodingUTF8 ) ); // kCFStringEncodingMacRoman kCFStringEncodingASCII kCFStringEncodingNonLossyASCII
            LE_TRACE_IF( !pNewPresetName, "\tSW AU: preset name string not UTF8 encoded." );
            impl.setProgramName( pNewPresetName );
            return noErr;
        }
    }; // kAudioUnitProperty_PresentPreset

    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_FactoryPresets>
    {
        static OSStatus get( Impl const & impl, AudioUnitScope, AudioUnitElement, ::CFArrayRef & presets ) { LE_UNREACHABLE_CODE(); presets = nullptr; return kAudioUnitErr_InvalidProperty; }
    }; // kAudioUnitProperty_FactoryPresets


    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_ClassInfo>
    {
        static OSStatus get( Impl const & impl, AudioUnitScope, AudioUnitElement, ::CFPropertyListRef & preset )
        {
            ::CFMutableDictionaryRef const dictionary( ::CFDictionaryCreateMutable( nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks ) );
            if ( !dictionary )
                return kAudio_MemFullError;

            /// \note Audio Unit identification (version, FourCCs: "type",
            /// "subtype", "manufacturer code"):
            /// http://developer.apple.com/documentation/MusicAudio/Conceptual/AudioUnitProgrammingGuide/AudioUnitDevelopmentFundamentals/AudioUnitDevelopmentFundamentals.html#//apple_ref/doc/uid/TP40003278-CH7-SW1
            ///                               (27.02.2013.) (Domagoj Saric)
            ::AudioComponentDescription description;
            description.componentType         = kAudioUnitType_Effect;
            description.componentSubType      = 'SW00'; // kAudioUnitSubType_AUFilter
            description.componentManufacturer = 'LE00';
            if
            (
                !addToDictionary( dictionary, CFSTR( kAUPresetVersionKey      ), Impl::version                     ) ||
                !addToDictionary( dictionary, CFSTR( kAUPresetTypeKey         ), description.componentType         ) ||
                !addToDictionary( dictionary, CFSTR( kAUPresetSubtypeKey      ), description.componentSubType      ) ||
                !addToDictionary( dictionary, CFSTR( kAUPresetManufacturerKey ), description.componentManufacturer )
            )
            {
                ::CFRelease( dictionary );
                return kAudio_MemFullError;
            }

            ::CFStringRef const currentPresetName( ::CFStringCreateWithCString( nullptr, impl.currentProgramName(), kCFStringEncodingUTF8 ) );
            ::CFDictionarySetValue( dictionary, CFSTR( kAUPresetNameKey ), currentPresetName );
            ::CFRelease( currentPresetName );

            BOOST_ASSERT( impl.getProgram() == 0 );
            unsigned int const maximumPresetSize( impl.maximumProgramSize() );
            ::CFMutableDataRef const cfPresetData( ::CFDataCreateMutable( 0, maximumPresetSize ) );
            if ( !cfPresetData )
            {
                ::CFRelease( dictionary );
                return kAudio_MemFullError;
            }
            unsigned int const actualPresetSize( impl.saveProgramState( 0, ::CFDataGetMutableBytePtr( cfPresetData ), maximumPresetSize ) );
            BOOST_ASSERT( actualPresetSize <= maximumPresetSize );
            ::CFDataSetLength     ( cfPresetData, actualPresetSize );
            ::CFDictionarySetValue( dictionary, CFSTR( "LE.Plugin.State" ), cfPresetData );
            ::CFRelease           ( cfPresetData );
            preset = dictionary;
            return noErr;
        }

        static OSStatus set( Impl & impl, AudioUnitScope, AudioUnitElement, ::CFPropertyListRef const & preset )
        {
            BOOST_ASSERT_MSG( ::CFGetTypeID( preset ) == CFDictionaryGetTypeID(), "kAudioUnitErr_InvalidPropertyValue" );
            ::CFDictionaryRef const dictionary( static_cast<::CFDictionaryRef>( preset                                                           ) );
            ::CFDataRef       const presetData( static_cast<::CFDataRef      >( ::CFDictionaryGetValue( dictionary, CFSTR( "LE.Plugin.State" ) ) ) );
            ::CFStringRef     const presetName( static_cast<::CFStringRef    >( ::CFDictionaryGetValue( dictionary, CFSTR( kAUPresetNameKey  ) ) ) );
            BOOST_ASSERT_MSG( presetData, "Invalid AU preset" );
            void         const * const pPresetData( ::CFDataGetBytePtr( presetData ) );
            unsigned int         const presetSize ( ::CFDataGetLength ( presetData ) );
            BOOST_ASSERT_MSG( pPresetData, "Invalid AU preset" );
            BOOST_ASSERT_MSG( presetSize , "Invalid AU preset" );
            char const * pPresetName;
            static ::CFStringBuiltInEncodings constexpr encodings[] = { kCFStringEncodingMacRoman, kCFStringEncodingUTF8, kCFStringEncodingASCII, kCFStringEncodingNonLossyASCII, kCFStringEncodingWindowsLatin1, kCFStringEncodingISOLatin1 };
            for ( auto const encoding : encodings )
            {
                pPresetName = ::CFStringGetCStringPtr( presetName, encoding );
                if ( presetName )
                    break;
            }
            char conversionBuffer[ 256 ];
            if ( !pPresetName )
            {
                BOOST_VERIFY( ::CFStringGetCString( presetName, conversionBuffer, sizeof( conversionBuffer ), kCFStringEncodingUTF8 ) );
                pPresetName = conversionBuffer;
            }
            BOOST_ASSERT( impl.getProgram() == 0 );
            OSStatus const result( Detail::makeErrorCode( impl.loadProgramState( 0, pPresetName, pPresetData, presetSize ) ) );
            if ( result == noErr )
                impl.propertyChanged( kAudioUnitProperty_PresentPreset, kAudioUnitScope_Global, 0 );
            return result;
        }

    private:
        static bool addToDictionary( ::CFMutableDictionaryRef const dictionary, ::CFStringRef const key, ::OSType const value )
        {
            ::CFNumberRef const number( ::CFNumberCreate( 0, kCFNumberLongType, &value ) );
            if ( !number ) return false;
            ::CFDictionarySetValue( dictionary, key, number );
            ::CFRelease( number );
            return true;
        }
    }; // kAudioUnitProperty_ClassInfo

    template <>
    struct PropertyChangeDetector<kAudioUnitProperty_ClassInfo>
    {
        PropertyChangeDetector( AUPluginBase const &, AudioUnitScope, AudioUnitElement ) {}
        static bool changed( AUPluginBase const &, AudioUnitScope, AudioUnitElement ) { return false; }
    }; // PropertyChangeDetector<kAudioUnitProperty_ClassInfo>


    ////////////////////////////////////////////////////////////////////////////
    ///
    /// GUI properties
    ///
    ////////////////////////////////////////////////////////////////////////////

    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_CocoaUI>
    {
        static OSStatus get( Impl const &, AudioUnitScope, AudioUnitElement, ::AudioUnitCocoaViewInfo & viewInfo )
        {
            //BOOST_ASSERT( viewInfo.mCocoaAUViewBundleLocation == nullptr );
            //BOOST_ASSERT( viewInfo.mCocoaAUViewClass[ 0 ]     == nullptr );

            viewInfo.mCocoaAUViewBundleLocation = Detail::auBundlePath               ();
            viewInfo.mCocoaAUViewClass[ 0 ]     = Detail::auCocoaViewFactoryClassName();
            return noErr;
        }
    }; // kAudioUnitProperty_CocoaUI


    template <class Impl>
    struct PropertyHandler<Impl, Detail::auViewPropertyID>
    {
        static OSStatus set( Impl & impl, AudioUnitScope, AudioUnitElement, ObjC::NSView * const pParentView )
        {
            if ( pParentView )
                return Detail::makeErrorCode( impl.createGUI( pParentView ) );
            else
            {
                impl.destroyGUI();
                return noErr;
            }
        }
    }; // Detail::auViewPropertyID
    template <>
    struct PropertyChangeDetector<auViewPropertyID>
    {
        PropertyChangeDetector( AUPluginBase const &, AudioUnitScope, AudioUnitElement ) {}
        static bool changed( AUPluginBase const &, AudioUnitScope, AudioUnitElement ) { return false; }
    }; // PropertyChangeDetector<auViewPropertyID>


    ////////////////////////////////////////////////////////////////////////////
    ///
    /// Parameter properties
    ///
    ////////////////////////////////////////////////////////////////////////////

    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_ParameterList>
    {
        /// \note Dynamic parameter list support:
        /// http://web.archiveorange.com/archive/v/q7bubJgO6HIqbgmu16wC
        /// http://www.mailinglistarchive.com/html/coreaudio-api@lists.apple.com/2009-08/msg00133.html
        ///                                   (27.02.2013.) (Domagoj Saric)
        static OSStatus get( Impl const & impl, ::AudioUnitScope const scope, ::AudioUnitElement, boost::iterator_range<::AudioUnitParameterID *> const parameters )
        {
            LE_ASSUME( scope == kAudioUnitScope_Global );
            Impl::getParameterIDs( reinterpret_cast<boost::iterator_range<ParameterID *> const &>( parameters ), ( impl.host().canTryDynamicParameterList() && impl.staticParameterListReported_ ) ? &impl.program() : nullptr );
            return noErr;
        }
    }; // kAudioUnitProperty_ParameterList
    template <>
    struct PropertyValueCount<kAudioUnitProperty_ParameterList>
    {
        template <class Impl>
        static unsigned int get( Impl const & impl, ::AudioUnitScope const scope, ::AudioUnitElement )
        {
            return ( scope == kAudioUnitScope_Global ) ? Impl::numberOfParameters( ( impl.host().canTryDynamicParameterList() && impl.staticParameterListReported_ ) ? &impl.program() : nullptr ) : 0;
        }
    };

    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_DependentParameters>
    {
        // http://lists.apple.com/archives/coreaudio-api/2005/Oct/msg00161.html
        // http://www.mailinglistarchive.com/html/coreaudio-api@lists.apple.com/2007-02/msg00102.html
        static OSStatus get( Impl const & impl, ::AudioUnitScope const scope, ::AudioUnitElement const parameterID, boost::iterator_range<::AUDependentParameter *> const dependentParameters )
        {
            LE_ASSUME( scope == kAudioUnitScope_Global );
            Impl::getDependentParameters( reinterpret_cast<ParameterID const &>( parameterID ), dependentParameters, ( impl.host().canTryDynamicParameterList() && impl.staticParameterListReported_ ) ? &impl : nullptr );
            return noErr;
        }
    }; // kAudioUnitProperty_DependentParameters
    template <>
    struct PropertyValueCount<kAudioUnitProperty_DependentParameters>
    {
        template <class Impl>
        static unsigned int get( Impl const & impl, ::AudioUnitScope const scope, ::AudioUnitElement const parameterID )
        {
            LE_ASSUME( scope == kAudioUnitScope_Global );
            return Impl::numberOfDependentParameters( reinterpret_cast<ParameterID const &>( parameterID ), ( impl.host().canTryDynamicParameterList() && impl.staticParameterListReported_ ) ? &impl : nullptr );
        }
    };

    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_ParameterInfo>
    {
        static OSStatus get( Impl const & impl, ::AudioUnitScope const scope, ::AudioUnitElement const parameterID, ::AudioUnitParameterInfo & parameterInfo )
        {
            LE_ASSUME( scope == kAudioUnitScope_Global );
            BOOST_VERIFY( Impl::getParameterProperties( ParameterID{ parameterID }, static_cast<AUPluginBase::ParameterInformation &>( parameterInfo ), ( impl.host().canTryDynamicParameterList() && impl.staticParameterListReported_ ) ? &impl.program() : nullptr ) );
            impl.staticParameterListReported_ = true;
            return noErr;
        }
    }; // kAudioUnitProperty_ParameterInfo

    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_ParameterIDName> // short name
    {
        static OSStatus get( Impl const &, ::AudioUnitScope const scope, ::AudioUnitElement const parameterID, ::AudioUnitParameterNameInfo & /*parameterNameInfo*/ )
        {
            LE_ASSUME( scope == kAudioUnitScope_Global );
            LE_TRACE( "\tSW AU: host requested short name for parameter %u.", parameterID );
            return kAudio_UnimplementedError;
        }
    }; // kAudioUnitProperty_ParameterIDName

    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_ParameterClumpName> // clump = group
    {
        static OSStatus get( Impl const &, ::AudioUnitScope const scope, ::AudioUnitElement const parameterID, ::AudioUnitParameterNameInfo & parameterClumpNameInfo )
        {
            LE_ASSUME( scope == kAudioUnitScope_Global );
            // http://www.mailinglistarchive.com/coreaudio-api@lists.apple.com/msg00783.html
            //::UInt32 const clumpID( parameterClumpNameInfo.inID );
            return kAudio_UnimplementedError;
        }
    }; // kAudioUnitProperty_ParameterClumpName

    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_ParameterStringFromValue>
    {
        static OSStatus get( Impl const & impl, ::AudioUnitScope const scope, ::AudioUnitElement const parameterID, ::AudioUnitParameterStringFromValue & stringFromValue )
        {
            LE_ASSUME( scope       == kAudioUnitScope_Global                    );
            LE_ASSUME( parameterID == stringFromValue.inParamID || !parameterID );
            std::array<char, 64> buffer;
            impl.getParameterDisplay( ParameterID{ stringFromValue.inParamID }, buffer, stringFromValue.inValue );
            stringFromValue.outString = ::CFStringCreateWithCString( nullptr, buffer.begin(), kCFStringEncodingASCII );
            if ( !stringFromValue.outString )
                return kAudio_MemFullError;
            return noErr;
        }
    }; // kAudioUnitProperty_ParameterStringFromValue

    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_ParameterValueFromString>
    {
        static OSStatus get( Impl const &, ::AudioUnitScope const scope, ::AudioUnitElement const parameterID, ::AudioUnitParameterValueFromString & valueFromString )
        {
            LE_ASSUME( scope == kAudioUnitScope_Global );
            return kAudio_UnimplementedError;
        }
    }; // kAudioUnitProperty_ParameterStringFromValue

    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_ParameterValueStrings>
    {
        static OSStatus get( Impl const &, ::AudioUnitScope const scope, ::AudioUnitElement const parameterID, ::CFArrayRef & parameterValueStrings )
        {
            LE_ASSUME( scope == kAudioUnitScope_Global );
            return kAudio_UnimplementedError;
        }
    }; // kAudioUnitProperty_ParameterValueStrings


    ////////////////////////////////////////////////////////////////////////////
    ///
    /// Host information
    ///
    ////////////////////////////////////////////////////////////////////////////

    template <class Impl>
    struct PropertyHandler<Impl, kAudioUnitProperty_AUHostIdentifier>
    {
        static OSStatus set( Impl & impl, ::AudioUnitScope const scope, ::AudioUnitElement const parameterID, ::AUHostVersionIdentifier const & hostIdentifier )
        {
            LE_ASSUME( scope == kAudioUnitScope_Global );
            // http://forum.cockos.com/showthread.php?t=98416
            if
            (
              //( CFStringCompare( hostIdentifier.hostName, CFSTR( "REAPER" ), 0 )          == kCFCompareEqualTo ) || //...mrmlj...Reaper does not seem to support this with AUs...
                ( CFStringCompare( hostIdentifier.hostName, CFSTR( "Live"   ), 0 )          == kCFCompareEqualTo ) ||
              //( CFStringFind   ( hostIdentifier.hostName, CFSTR( "logic"  ), 0 ).location != kCFNotFound       ) || //...does not work...probably have to wait until it registers all the parameter listeners...
                false
            )
            {
                //...ugh...mrmlj...
                impl.canTryDynamicParameterList_  = true;
                impl.staticParameterListReported_ = true;
            }
            return noErr;
        }
    }; // kAudioUnitProperty_AUHostIdentifier
    template <>
    struct PropertyChangeDetector<kAudioUnitProperty_AUHostIdentifier>
    {
        PropertyChangeDetector( AUPluginBase const &, AudioUnitScope, AudioUnitElement ) {}
        static bool changed( AUPluginBase const &, AudioUnitScope, AudioUnitElement ) { return false; }
    }; // PropertyChangeDetector<kAudioUnitProperty_AUHostIdentifier>
} // namespace Detail

#undef LE_AU_PROPERTY_DEBUGGER
#undef LE_AU_PROPERTY_DEBUGGER_INIT
#undef LE_AU_PROPERTY_DEBUGGER_SET_PROPERTY

//------------------------------------------------------------------------------
} // namespace Plugins
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#endif // properties_hpp
