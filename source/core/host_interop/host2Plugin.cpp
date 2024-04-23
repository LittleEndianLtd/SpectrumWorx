////////////////////////////////////////////////////////////////////////////////
///
/// host2Plugin.cpp
/// ---------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "host2Plugin.hpp"

#include "le/utility/trace.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------

#pragma warning( push )
#pragma warning( disable : 4127 ) // Conditional expression is constant.

LE_OPTIMIZE_FOR_SIZE_BEGIN()

//...mrmlj...orphan...
template <typename Char>
LE_NOTHROWNOALIAS LE_COLD char * LE_FASTCALL copyToBuffer( Char const * const string, boost::iterator_range<char *> const & buffer )
{
    //std::strncpy( buffer.begin(), string, buffer.size() - 1 );
    Char const * LE_RESTRICT       pSourceCharacter     ( string             );
    char       * LE_RESTRICT       pDestinationCharacter( buffer.begin()     );
    char const * LE_RESTRICT const pDestinationEnd      ( buffer.end  () - 1 );

    bool const same( !std::is_same<Char, char>::value && ( static_cast<void const *>( string ) == static_cast<void const *>( pDestinationCharacter ) ) );
    if ( same )
    {
        BOOST_ASSERT( *pDestinationCharacter == '\0' );
        return pDestinationCharacter;
    }
    if ( string == nullptr )
    {
        *pDestinationCharacter = '\0';
        return pDestinationCharacter;
    }

    while ( pDestinationCharacter != pDestinationEnd )
    {
        Char const sourceCharacter( *pSourceCharacter );
        if ( sourceCharacter == '\0' )
            break;
        *pDestinationCharacter = static_cast<char>( sourceCharacter );
        BOOST_ASSERT( *pDestinationCharacter == sourceCharacter );
        ++pSourceCharacter;
        ++pDestinationCharacter;
    }
    *pDestinationCharacter = '\0';

    LE_TRACE_IF
    (
        std::char_traits<Char>::length( string ) >= unsigned( buffer.size() ),
        "\tSW: source string (\"%s\") does not fit into destination buffer (%d characters).",
        std::is_same<Char, char>::value ? reinterpret_cast<char const *>( string ) : buffer.begin(), buffer.size() //...mrmlj...
    );

    return pDestinationCharacter;
}

#pragma warning( pop )

template LE_NOTHROWNOALIAS char * LE_FASTCALL copyToBuffer<char   >( char    const *, boost::iterator_range<char *> const & );
#ifdef _WIN32
template LE_NOTHROWNOALIAS char * LE_FASTCALL copyToBuffer<wchar_t>( wchar_t const *, boost::iterator_range<char *> const & );
#endif // _WIN32

LE_OPTIMIZE_FOR_SIZE_END()

//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
