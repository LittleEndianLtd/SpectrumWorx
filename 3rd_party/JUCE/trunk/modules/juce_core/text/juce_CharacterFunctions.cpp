/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

//==============================================================================
#if JUCE_MSVC
 #pragma warning (push)
 #pragma warning (disable: 4514 4996)
#endif

juce_wchar CharacterFunctions::toUpperCase (const juce_wchar character) noexcept
{
#if defined( LE_PATCHED_JUCE ) && defined( _WIN32 )
    return reinterpret_cast<wchar_t>( ::CharUpperW( reinterpret_cast<LPWSTR>( static_cast<std::size_t>( character ) ) ) );
#else
    return towupper ((wchar_t) character);
#endif // LE_PATCHED_JUCE
}

juce_wchar CharacterFunctions::toLowerCase (const juce_wchar character) noexcept
{
#if defined( LE_PATCHED_JUCE ) && defined( _WIN32 )
    return reinterpret_cast<wchar_t>( ::CharLowerW( reinterpret_cast<LPWSTR>( static_cast<std::size_t>( character ) ) ) );
#else
    return towlower ((wchar_t) character);
#endif // LE_PATCHED_JUCE
}

bool CharacterFunctions::isUpperCase (const juce_wchar character) noexcept
{
   #if JUCE_WINDOWS
    return iswupper ((wchar_t) character) != 0;
   #else
    return toLowerCase (character) != character;
   #endif
}

bool CharacterFunctions::isLowerCase (const juce_wchar character) noexcept
{
   #if JUCE_WINDOWS
    return iswlower ((wchar_t) character) != 0;
   #else
    return toUpperCase (character) != character;
   #endif
}

#if defined( LE_PATCHED_JUCE )
char CharacterFunctions::toUpperCase (const char character) noexcept
{
#if defined( _WIN32 )
    return reinterpret_cast<char>( ::CharUpperA( reinterpret_cast<LPSTR>( static_cast<std::size_t>( character ) ) ) );
#else
    return toupper (character);
#endif // _WIN32
}

char CharacterFunctions::toLowerCase (const char character) noexcept
{
#if defined( _WIN32 )
    return reinterpret_cast<char>( ::CharLowerA( reinterpret_cast<LPSTR>( static_cast<std::size_t>( character ) ) ) );
#else
    return tolower (character);
#endif // _WIN32
}
#endif //LE_PATCHED_JUCE

#if JUCE_MSVC
 #pragma warning (pop)
#endif

//==============================================================================
bool CharacterFunctions::isWhitespace (const char character) noexcept
{
    return character == ' ' || (character <= 13 && character >= 9);
}

bool CharacterFunctions::isWhitespace (const juce_wchar character) noexcept
{
    return iswspace ((wchar_t) character) != 0;
}

bool CharacterFunctions::isDigit (const char character) noexcept
{
    return (character >= '0' && character <= '9');
}

bool CharacterFunctions::isDigit (const juce_wchar character) noexcept
{
#if defined( LE_PATCHED_JUCE ) && defined( _WIN32 )
    return ::IsCharAlphaNumericW( character ) && !::IsCharAlphaW( character );
#else
    return iswdigit ((wchar_t) character) != 0;
#endif // LE_PATCHED_JUCE
}

bool CharacterFunctions::isLetter (const char character) noexcept
{
    return (character >= 'a' && character <= 'z')
        || (character >= 'A' && character <= 'Z');
}

bool CharacterFunctions::isLetter (const juce_wchar character) noexcept
{
#if defined( LE_PATCHED_JUCE ) && defined( _WIN32 )
    return ::IsCharAlphaW( character ) != 0;
#else
    return iswalpha ((wchar_t) character) != 0;
#endif //LE_PATCHED_JUCE
}

bool CharacterFunctions::isLetterOrDigit (const char character) noexcept
{
    return (character >= 'a' && character <= 'z')
        || (character >= 'A' && character <= 'Z')
        || (character >= '0' && character <= '9');
}

bool CharacterFunctions::isLetterOrDigit (const juce_wchar character) noexcept
{
#if defined( LE_PATCHED_JUCE ) && defined( _WIN32 )
    return ::IsCharAlphaNumericW( character ) != 0;
#else
    return iswalnum ((wchar_t) character) != 0;
#endif //LE_PATCHED_JUCE
}

int CharacterFunctions::getHexDigitValue (const juce_wchar digit) noexcept
{
    unsigned int d = (unsigned int) digit - '0';
    if (d < (unsigned int) 10)
        return (int) d;

    d += (unsigned int) ('0' - 'a');
    if (d < (unsigned int) 6)
        return (int) d + 10;

    d += (unsigned int) ('a' - 'A');
    if (d < (unsigned int) 6)
        return (int) d + 10;

    return -1;
}

#ifndef LE_PATCHED_JUCE
double CharacterFunctions::mulexp10 (const double value, int exponent) noexcept
{
    if (exponent == 0)
        return value;

    if (value == 0)
        return 0;

    const bool negative = (exponent < 0);
    if (negative)
        exponent = -exponent;

    double result = 1.0, power = 10.0;
    for (int bit = 1; exponent != 0; bit <<= 1)
    {
        if ((exponent & bit) != 0)
        {
            exponent ^= bit;
            result *= power;
            if (exponent == 0)
                break;
        }
        power *= power;
    }

    return negative ? (value / result) : (value * result);
}
#endif // LE_PATCHED_JUCE
