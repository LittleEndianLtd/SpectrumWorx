////////////////////////////////////////////////////////////////////////////////
///
/// \file channelStateDynamic.hpp
/// -----------------------------
///
/// Copyright (c) 2012 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef channelStateDynamic_hpp__44528C6F_047C_4814_BBB1_01EDE8DCAABF
#define channelStateDynamic_hpp__44528C6F_047C_4814_BBB1_01EDE8DCAABF
#pragma once
//------------------------------------------------------------------------------
#include "le/utility/platformSpecifics.hpp"

#include "boost/preprocessor/seq/seq.hpp"
#include "boost/preprocessor/seq/for_each.hpp"

#include <cstdint>
//------------------------------------------------------------------------------
namespace boost { template <class IteratorT> class iterator_range; }
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------

namespace Utility { std::uint32_t align( std::uint32_t storageBytes ); }

//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
namespace Engine
{
    struct StorageFactors;
    using Storage = boost::iterator_range<char * LE_RESTRICT>;
} //namespace Engine
//------------------------------------------------------------------------------
namespace Effects
{
//------------------------------------------------------------------------------

#define LE_CS_ADD_INDIVIDUAL_MEMBER( r, dummy, memberSequence ) \
    BOOST_PP_SEQ_HEAD( memberSequence ) BOOST_PP_SEQ_TAIL( memberSequence );

#define LE_CS_ENUMERATE_MEMBERS( members ) \
    BOOST_PP_SEQ_FOR_EACH( LE_CS_ADD_INDIVIDUAL_MEMBER, 0, members )


#define LE_CS_ACCUMULATE_INDIVIDUAL_MEMBER( r, dummy, memberSequence ) \
    + Utility::align( BOOST_PP_SEQ_HEAD( memberSequence )::requiredStorage( factors ) )

#define LE_CS_ACCUMULATE_MEMBERS( members ) \
    BOOST_PP_SEQ_FOR_EACH( LE_CS_ACCUMULATE_INDIVIDUAL_MEMBER, 0, members )


#define LE_CS_RESIZE_INDIVIDUAL_MEMBER( r, dummy, memberSequence ) \
    BOOST_PP_SEQ_TAIL( memberSequence ).resize( factors, storage );

#define LE_CS_RESIZE_MEMBERS( members ) \
    BOOST_PP_SEQ_FOR_EACH( LE_CS_RESIZE_INDIVIDUAL_MEMBER, 0, members )


#define LE_CS_RESET_INDIVIDUAL_MEMBER( r, dummy, memberSequence ) \
    BOOST_PP_SEQ_TAIL( memberSequence ).reset();

#define LE_CS_RESET_MEMBERS( members ) \
    BOOST_PP_SEQ_FOR_EACH( LE_CS_RESET_INDIVIDUAL_MEMBER, 0, members )


#define LE_NAMED_DYNAMIC_CHANNEL_STATE( name, members )                                  \
    struct name                                                                          \
    {                                                                                    \
        static std::uint32_t requiredStorage( Engine::StorageFactors const & factors )   \
        {                                                                                \
            return 0 LE_CS_ACCUMULATE_MEMBERS( members );                                \
        }                                                                                \
                                                                                         \
        void reset()                                                                     \
        {                                                                                \
            LE_CS_RESET_MEMBERS( members );                                              \
        }                                                                                \
                                                                                         \
        void resize( Engine::StorageFactors const & factors, Engine::Storage & storage ) \
        {                                                                                \
            LE_CS_RESIZE_MEMBERS( members );                                             \
        }                                                                                \
                                                                                         \
        LE_CS_ENUMERATE_MEMBERS( members )                                               \
    };


#define LE_DYNAMIC_CHANNEL_STATE( members ) LE_NAMED_DYNAMIC_CHANNEL_STATE( DynamicChannelState, members )


////////////////////////////////////////////////////////////////////////////////
/// \internal
/// \class CompoundChannelState
////////////////////////////////////////////////////////////////////////////////

LE_OPTIMIZE_FOR_SIZE_BEGIN()

template <typename... ChannelStates>
struct CompoundChannelState
    :
    ChannelStates...
{
private:
    // http://stackoverflow.com/questions/25680461/variadic-template-pack-expansion
    using expander = char[]; //...mrmlj...bad msvc12 codegen if int is used instead of char

public:
#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-value"
#endif // __clang__
    void LE_FASTCALL LE_COLD resize( Engine::StorageFactors const & factors, Engine::Storage & storage ) { expander { 0, (static_cast<ChannelStates &>( *this ).resize( factors, storage ), '\0')... }; }
    void LE_FASTCALL LE_COLD reset (                                                                   ) { expander { 0, (static_cast<ChannelStates &>( *this ).reset (                  ), '\0')... }; }

    static std::uint32_t LE_FASTCALL LE_COLD requiredStorage( Engine::StorageFactors const & factors )
    {
        std::uint32_t sum( 0 );
        expander { 0, (sum += Utility::align( ChannelStates::requiredStorage( factors ) ), '\0')... };
        return sum;
    }
#ifdef __clang__
    #pragma clang diagnostic pop
#endif // __clang__
}; // struct CompoundChannelState

LE_OPTIMIZE_FOR_SIZE_END()

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // channelStateDynamic_hpp
