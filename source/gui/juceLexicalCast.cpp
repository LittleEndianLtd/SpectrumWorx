////////////////////////////////////////////////////////////////////////////////
///
/// juceLexicalCast.cpp
/// -------------------
///
/// Copyright (c) 2012 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "le/utility/lexicalCast.hpp"

#include "juce/beginIncludes.hpp"
    #include "juce/juce_core/text/juce_CharacterFunctions.h"
#include "juce/endIncludes.hpp"
//------------------------------------------------------------------------------
namespace juce
{
//------------------------------------------------------------------------------
namespace NumberToStringConverters
{
    #ifdef _MSC_VER
        #define noexcept throw()
    #endif
    char * numberToString( char * t,  int64       const n ) noexcept { return t + LE_INT_SPRINTFA( t, "%lld", n ) ; }
    char * numberToString( char * t, uint64       const n ) noexcept { return t + LE_INT_SPRINTFA( t, "%llu", n ) ; }
    char * numberToString( char * t,          int const n ) noexcept { return t + LE::Utility::lexical_cast( n, t ); }
    char * numberToString( char * t, unsigned int const n ) noexcept { return t + LE::Utility::lexical_cast( n, t ); }

    char * doubleToString( char * /*const*/ buffer, int const numChars, double const n, int const numDecPlaces, std::size_t & len ) noexcept
    {
        len = LE::Utility::lexical_cast( n, static_cast<unsigned char>( numDecPlaces ), buffer );
        BOOST_ASSERT( len < unsigned( numChars ) ); (void)numChars;
        return buffer;
    }
} // namespace NumberToStringConverters

template <>
int CharacterFunctions::getIntValue<int, CharPointer_ASCII>( CharPointer_ASCII const string ) noexcept
{
    return LE::Utility::lexical_cast<int>( string );
};

double CharacterFunctions::readDoubleValue( CharPointer_ASCII & text ) noexcept
{
    char const * pValueString( text );
    double const result( LE::Utility::lexical_cast<double>( pValueString ) );
    //...mrmlj...quick-fix...our lexical_cast takes the pointer by value...
    //text = pValueString;
    text = text.findTerminatingNull();
    return result;
}

double CharacterFunctions::readDoubleValue( CharPointer_UTF8 & text ) noexcept
{
    return readDoubleValue( reinterpret_cast<CharPointer_ASCII &>( text ) );
}

double CharacterFunctions::readDoubleValue( CharPointer_UTF16 & text ) noexcept
{
    char buffer[ 64 ];
    BOOST_ASSERT( text.length() < numElementsInArray( buffer ) );
    CharPointer_ASCII ascii( buffer );
    copyAll( ascii, text );
    double const result( readDoubleValue( ascii ) );
    text += static_cast<unsigned int>( ascii.getAddress() - buffer );
    return result;
}

#ifdef __GNUC__
double CharacterFunctions::readDoubleValue( CharPointer_UTF32 & text ) noexcept
{
    CharPointer_UTF32::CharType * pEnd;
    double const result( /*std*/::wcstod( text.getAddress(), &pEnd ) );
    text = pEnd;
    return result;
}
#endif // __GNUC__
//------------------------------------------------------------------------------
} // namespace juce
//------------------------------------------------------------------------------
