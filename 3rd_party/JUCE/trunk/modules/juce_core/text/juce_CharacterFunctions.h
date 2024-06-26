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

#ifndef JUCE_CHARACTERFUNCTIONS_H_INCLUDED
#define JUCE_CHARACTERFUNCTIONS_H_INCLUDED


//==============================================================================
#if JUCE_WINDOWS && ! DOXYGEN
 #define JUCE_NATIVE_WCHAR_IS_UTF8      0
 #define JUCE_NATIVE_WCHAR_IS_UTF16     1
 #define JUCE_NATIVE_WCHAR_IS_UTF32     0
#else
 /** This macro will be set to 1 if the compiler's native wchar_t is an 8-bit type. */
 #define JUCE_NATIVE_WCHAR_IS_UTF8      0
 /** This macro will be set to 1 if the compiler's native wchar_t is a 16-bit type. */
 #define JUCE_NATIVE_WCHAR_IS_UTF16     0
 /** This macro will be set to 1 if the compiler's native wchar_t is a 32-bit type. */
 #define JUCE_NATIVE_WCHAR_IS_UTF32     1
#endif

#if JUCE_NATIVE_WCHAR_IS_UTF32 || DOXYGEN
 /** A platform-independent 32-bit unicode character type. */
 typedef wchar_t        juce_wchar;
#else
#ifdef LE_PATCHED_JUCE
 typedef wchar_t        juce_wchar;
#else
 typedef uint32         juce_wchar;
#endif // LE_PATCHED_JUCE
#endif

#ifndef DOXYGEN
 /** This macro is deprecated, but preserved for compatibility with old code. */
 #define JUCE_T(stringLiteral)   (L##stringLiteral)
#endif

#if JUCE_DEFINE_T_MACRO
 /** The 'T' macro is an alternative for using the "L" prefix in front of a string literal.

     This macro is deprecated, but available for compatibility with old code if you set
     JUCE_DEFINE_T_MACRO = 1. The fastest, most portable and best way to write your string
     literals is as standard char strings, using escaped utf-8 character sequences for extended
     characters, rather than trying to store them as wide-char strings.
 */
 #define T(stringLiteral)   JUCE_T(stringLiteral)
#endif

//==============================================================================
/**
    A collection of functions for manipulating characters and character strings.

    Most of these methods are designed for internal use by the String and CharPointer
    classes, but some of them may be useful to call directly.

    @see String, CharPointer_UTF8, CharPointer_UTF16, CharPointer_UTF32
*/
#ifdef LE_PATCHED_JUCE
    class CharPointer_ASCII;
    class CharPointer_UTF8 ;
    class CharPointer_UTF16;
    class CharPointer_UTF32;
#endif // LE_PATCHED_JUCE
class JUCE_API  CharacterFunctions
{
public:
    //==============================================================================
    /** Converts a character to upper-case. */
    static juce_wchar toUpperCase (juce_wchar character) noexcept;
    LE_PATCH( static char toUpperCase (char character) noexcept; )
    /** Converts a character to lower-case. */
    static juce_wchar toLowerCase (juce_wchar character) noexcept;
    LE_PATCH( static char toLowerCase (char character) noexcept; )

    /** Checks whether a unicode character is upper-case. */
    static bool isUpperCase (juce_wchar character) noexcept;
    LE_PATCH( static bool isUpperCase (char character) noexcept; )
    /** Checks whether a unicode character is lower-case. */
    static bool isLowerCase (juce_wchar character) noexcept;
    LE_PATCH( static bool isLowerCase (char character) noexcept; )

    /** Checks whether a character is whitespace. */
    static bool isWhitespace (char character) noexcept;
    /** Checks whether a character is whitespace. */
    static bool isWhitespace (juce_wchar character) noexcept;

    /** Checks whether a character is a digit. */
    static bool isDigit (char character) noexcept;
    /** Checks whether a character is a digit. */
    static bool isDigit (juce_wchar character) noexcept;

    /** Checks whether a character is alphabetic. */
    static bool isLetter (char character) noexcept;
    /** Checks whether a character is alphabetic. */
    static bool isLetter (juce_wchar character) noexcept;

    /** Checks whether a character is alphabetic or numeric. */
    static bool isLetterOrDigit (char character) noexcept;
    /** Checks whether a character is alphabetic or numeric. */
    static bool isLetterOrDigit (juce_wchar character) noexcept;

    /** Returns 0 to 16 for '0' to 'F", or -1 for characters that aren't a legal hex digit. */
    static int getHexDigitValue (juce_wchar digit) noexcept;

    //==============================================================================
    /** Parses a character string to read a floating-point number.
        Note that this will advance the pointer that is passed in, leaving it at
        the end of the number.
    */
    template <typename CharPointerType>
    static double readDoubleValue (CharPointerType& text) noexcept LE_PATCH( ; )
#ifndef LE_PATCHED_JUCE
    {
        double result[3] = { 0 }, accumulator[2] = { 0 };
        int exponentAdjustment[2] = { 0 }, exponentAccumulator[2] = { -1, -1 };
        int exponent = 0, decPointIndex = 0, digit = 0;
        int lastDigit = 0, numSignificantDigits = 0;
        bool isNegative = false, digitsFound = false;
        const int maxSignificantDigits = 15 + 2;

        text = text.findEndOfWhitespace();
        juce_wchar c = *text;

        switch (c)
        {
            case '-':   isNegative = true; // fall-through..
            case '+':   c = *++text;
        }

        switch (c)
        {
            case 'n':
            case 'N':
                if ((text[1] == 'a' || text[1] == 'A') && (text[2] == 'n' || text[2] == 'N'))
                    return std::numeric_limits<double>::quiet_NaN();
                break;

            case 'i':
            case 'I':
                if ((text[1] == 'n' || text[1] == 'N') && (text[2] == 'f' || text[2] == 'F'))
                    return std::numeric_limits<double>::infinity();
                break;
        }

        for (;;)
        {
            if (text.isDigit())
            {
                lastDigit = digit;
                digit = (int) text.getAndAdvance() - '0';
                digitsFound = true;

                if (decPointIndex != 0)
                    exponentAdjustment[1]++;

                if (numSignificantDigits == 0 && digit == 0)
                    continue;

                if (++numSignificantDigits > maxSignificantDigits)
                {
                    if (digit > 5)
                        ++accumulator [decPointIndex];
                    else if (digit == 5 && (lastDigit & 1) != 0)
                        ++accumulator [decPointIndex];

                    if (decPointIndex > 0)
                        exponentAdjustment[1]--;
                    else
                        exponentAdjustment[0]++;

                    while (text.isDigit())
                    {
                        ++text;
                        if (decPointIndex == 0)
                            exponentAdjustment[0]++;
                    }
                }
                else
                {
                    const double maxAccumulatorValue = (double) ((std::numeric_limits<unsigned int>::max() - 9) / 10);
                    if (accumulator [decPointIndex] > maxAccumulatorValue)
                    {
                        result [decPointIndex] = mulexp10 (result [decPointIndex], exponentAccumulator [decPointIndex])
                                                    + accumulator [decPointIndex];
                        accumulator [decPointIndex] = 0;
                        exponentAccumulator [decPointIndex] = 0;
                    }

                    accumulator [decPointIndex] = accumulator[decPointIndex] * 10 + digit;
                    exponentAccumulator [decPointIndex]++;
                }
            }
            else if (decPointIndex == 0 && *text == '.')
            {
                ++text;
                decPointIndex = 1;

                if (numSignificantDigits > maxSignificantDigits)
                {
                    while (text.isDigit())
                        ++text;
                    break;
                }
            }
            else
            {
                break;
            }
        }

        result[0] = mulexp10 (result[0], exponentAccumulator[0]) + accumulator[0];

        if (decPointIndex != 0)
            result[1] = mulexp10 (result[1], exponentAccumulator[1]) + accumulator[1];

        c = *text;
        if ((c == 'e' || c == 'E') && digitsFound)
        {
            bool negativeExponent = false;

            switch (*++text)
            {
                case '-':   negativeExponent = true; // fall-through..
                case '+':   ++text;
            }

            while (text.isDigit())
                exponent = (exponent * 10) + ((int) text.getAndAdvance() - '0');

            if (negativeExponent)
                exponent = -exponent;
        }

        double r = mulexp10 (result[0], exponent + exponentAdjustment[0]);
        if (decPointIndex != 0)
            r += mulexp10 (result[1], exponent - exponentAdjustment[1]);

        return isNegative ? -r : r;
    }
#endif // LE_PATCHED_JUCE

    #ifdef LE_PATCHED_JUCE
        static double readDoubleValue( CharPointer_ASCII & text ) noexcept;
        static double readDoubleValue( CharPointer_UTF8  & text ) noexcept;
        static double readDoubleValue( CharPointer_UTF16 & text ) noexcept;
        #ifdef __GNUC__
            static double readDoubleValue( CharPointer_UTF32 & text ) noexcept;
        #endif // __GNUC__
    #endif // LE_PATCHED_JUCE

    /** Parses a character string, to read a floating-point value. */
    template <typename CharPointerType>
    static double getDoubleValue (CharPointerType text) noexcept
    {
        return readDoubleValue (text);
    }

    //==============================================================================
    /** Parses a character string, to read an integer value. */
    template <typename IntType, typename CharPointerType>
    static IntType getIntValue (const CharPointerType text) noexcept LE_PATCH( ; )
#ifndef LE_PATCHED_JUCE
    {
        IntType v = 0;
        CharPointerType s (text.findEndOfWhitespace());

        const bool isNeg = *s == '-';
        if (isNeg)
            ++s;

        for (;;)
        {
            const juce_wchar c = s.getAndAdvance();

            if (c >= '0' && c <= '9')
                v = v * 10 + (IntType) (c - '0');
            else
                break;
        }

        return isNeg ? -v : v;
    }
#endif // LE_PATCHED_JUCE

    //==============================================================================
    /** Counts the number of characters in a given string, stopping if the count exceeds
        a specified limit. */
    template <typename CharPointerType>
    static size_t lengthUpTo (CharPointerType text, const size_t maxCharsToCount) noexcept
    {
        size_t len = 0;

        while (len < maxCharsToCount && text.getAndAdvance() != 0)
            ++len;

        return len;
    }

    /** Counts the number of characters in a given string, stopping if the count exceeds
        a specified end-pointer. */
    template <typename CharPointerType>
    static size_t lengthUpTo (CharPointerType start, const CharPointerType end) noexcept
    {
        size_t len = 0;

        while (start < end && start.getAndAdvance() != 0)
            ++len;

        return len;
    }

    /** Copies null-terminated characters from one string to another. */
    template <typename DestCharPointerType, typename SrcCharPointerType>
    static void copyAll (DestCharPointerType& dest, SrcCharPointerType src) noexcept
    {
        for (;;)
        {
            const juce_wchar c = src.getAndAdvance();

            if (c == 0)
                break;

            dest.write (c);
        }

        dest.writeNull();
    }

    /** Copies characters from one string to another, up to a null terminator
        or a given byte size limit. */
    template <typename DestCharPointerType, typename SrcCharPointerType>
    static size_t copyWithDestByteLimit (DestCharPointerType& dest, SrcCharPointerType src, size_t maxBytesToWrite) noexcept
    {
        typename DestCharPointerType::CharType const* const startAddress = dest.getAddress();
        ssize_t maxBytes = (ssize_t) maxBytesToWrite;
        maxBytes -= sizeof (typename DestCharPointerType::CharType); // (allow for a terminating null)

        for (;;)
        {
            const juce_wchar c = src.getAndAdvance();
            const size_t bytesNeeded = DestCharPointerType::getBytesRequiredFor (c);

            maxBytes -= bytesNeeded;
            if (c == 0 || maxBytes < 0)
                break;

            dest.write (c);
        }

        dest.writeNull();

        return (size_t) getAddressDifference (dest.getAddress(), startAddress)
                 + sizeof (typename DestCharPointerType::CharType);
    }

    /** Copies characters from one string to another, up to a null terminator
        or a given maximum number of characters. */
    template <typename DestCharPointerType, typename SrcCharPointerType>
    static void copyWithCharLimit (DestCharPointerType& dest, SrcCharPointerType src, int maxChars) noexcept
    {
        while (--maxChars > 0)
        {
            const juce_wchar c = src.getAndAdvance();
            if (c == 0)
                break;

            dest.write (c);
        }

        dest.writeNull();
    }

    /** Compares two null-terminated character strings. */
    template <typename CharPointerType1, typename CharPointerType2>
    static int compare (CharPointerType1 s1, CharPointerType2 s2) noexcept
    {
        for (;;)
        {
            const int c1 = (int) s1.getAndAdvance();
            const int c2 = (int) s2.getAndAdvance();
            const int diff = c1 - c2;

            if (diff != 0)  return diff < 0 ? -1 : 1;
            if (c1 == 0)    break;
        }

        return 0;
    }

    /** Compares two null-terminated character strings, up to a given number of characters. */
    template <typename CharPointerType1, typename CharPointerType2>
    static int compareUpTo (CharPointerType1 s1, CharPointerType2 s2, int maxChars) noexcept
    {
        while (--maxChars >= 0)
        {
            const int c1 = (int) s1.getAndAdvance();
            const int c2 = (int) s2.getAndAdvance();
            const int diff = c1 - c2;

            if (diff != 0)  return diff < 0 ? -1 : 1;
            if (c1 == 0)    break;
        }

        return 0;
    }

    /** Compares two null-terminated character strings, using a case-independant match. */
    template <typename CharPointerType1, typename CharPointerType2>
    static int compareIgnoreCase (CharPointerType1 s1, CharPointerType2 s2) noexcept
    {
        for (;;)
        {
            const int c1 = (int) s1.toUpperCase(); ++s1;
            const int c2 = (int) s2.toUpperCase(); ++s2;
            const int diff = c1 - c2;

            if (diff != 0)  return diff < 0 ? -1 : 1;
            if (c1 == 0)    break;
        }

        return 0;
    }

    /** Compares two null-terminated character strings, using a case-independent match. */
    template <typename CharPointerType1, typename CharPointerType2>
    static int compareIgnoreCaseUpTo (CharPointerType1 s1, CharPointerType2 s2, int maxChars) noexcept
    {
        while (--maxChars >= 0)
        {
            const int c1 = (int) s1.toUpperCase(); ++s1;
            const int c2 = (int) s2.toUpperCase(); ++s2;
            const int diff = c1 - c2;

            if (diff != 0)  return diff < 0 ? -1 : 1;
            if (c1 == 0)    break;
        }

        return 0;
    }

    /** Finds the character index of a given substring in another string.
        Returns -1 if the substring is not found.
    */
    template <typename CharPointerType1, typename CharPointerType2>
    static int indexOf (CharPointerType1 textToSearch, const CharPointerType2 substringToLookFor) noexcept
    {
        int index = 0;
        const int substringLength = (int) substringToLookFor.length();

        for (;;)
        {
            if (textToSearch.compareUpTo (substringToLookFor, substringLength) == 0)
                return index;

            if (textToSearch.getAndAdvance() == 0)
                return -1;

            ++index;
        }
    }

    /** Returns a pointer to the first occurrence of a substring in a string.
        If the substring is not found, this will return a pointer to the string's
        null terminator.
    */
    template <typename CharPointerType1, typename CharPointerType2>
    static CharPointerType1 find (CharPointerType1 textToSearch, const CharPointerType2 substringToLookFor) noexcept
    {
        const int substringLength = (int) substringToLookFor.length();

        while (textToSearch.compareUpTo (substringToLookFor, substringLength) != 0
                 && ! textToSearch.isEmpty())
            ++textToSearch;

        return textToSearch;
    }

    /** Finds the character index of a given substring in another string, using
        a case-independent match.
        Returns -1 if the substring is not found.
    */
    template <typename CharPointerType1, typename CharPointerType2>
    static int indexOfIgnoreCase (CharPointerType1 haystack, const CharPointerType2 needle) noexcept
    {
        int index = 0;
        const int needleLength = (int) needle.length();

        for (;;)
        {
            if (haystack.compareIgnoreCaseUpTo (needle, needleLength) == 0)
                return index;

            if (haystack.getAndAdvance() == 0)
                return -1;

            ++index;
        }
    }

    /** Finds the character index of a given character in another string.
        Returns -1 if the character is not found.
    */
    template <typename Type>
    static int indexOfChar (Type text, const juce_wchar charToFind) noexcept
    {
        int i = 0;

        while (! text.isEmpty())
        {
            if (text.getAndAdvance() == charToFind)
                return i;

            ++i;
        }

        return -1;
    }

    /** Finds the character index of a given character in another string, using
        a case-independent match.
        Returns -1 if the character is not found.
    */
    template <typename Type>
    static int indexOfCharIgnoreCase (Type text, juce_wchar charToFind) noexcept
    {
        charToFind = CharacterFunctions::toLowerCase (charToFind);
        int i = 0;

        while (! text.isEmpty())
        {
            if (text.toLowerCase() == charToFind)
                return i;

            ++text;
            ++i;
        }

        return -1;
    }

    /** Returns a pointer to the first non-whitespace character in a string.
        If the string contains only whitespace, this will return a pointer
        to its null terminator.
    */
    template <typename Type>
    static Type findEndOfWhitespace (Type text) noexcept
    {
        while (text.isWhitespace())
            ++text;

        return text;
    }

    /** Returns a pointer to the first character in the string which is found in
        the breakCharacters string.
    */
    template <typename Type>
    static Type findEndOfToken (Type text, const Type breakCharacters, const Type quoteCharacters)
    {
        juce_wchar currentQuoteChar = 0;

        while (! text.isEmpty())
        {
            const juce_wchar c = text.getAndAdvance();

            if (currentQuoteChar == 0 && breakCharacters.indexOf (c) >= 0)
            {
                --text;
                break;
            }

            if (quoteCharacters.indexOf (c) >= 0)
            {
                if (currentQuoteChar == 0)
                    currentQuoteChar = c;
                else if (currentQuoteChar == c)
                    currentQuoteChar = 0;
            }
        }

        return text;
    }

private:
    static double mulexp10 (const double value, int exponent) noexcept;
};


#ifdef LE_PATCHED_JUCE

template <> inline size_t CharacterFunctions::lengthUpTo( char    const * const text, const size_t maxCharsToCount ) noexcept { return /*std*/::strnlen( text, maxCharsToCount ); }
template <> inline size_t CharacterFunctions::lengthUpTo( wchar_t const * const text, const size_t maxCharsToCount ) noexcept { return /*std*/::wcsnlen( text, maxCharsToCount ); }
template <> inline size_t CharacterFunctions::lengthUpTo( char    const * const start, const char    * const end   ) noexcept { return lengthUpTo( start, end - start ); }
template <> inline size_t CharacterFunctions::lengthUpTo( wchar_t const * const start, const wchar_t * const end   ) noexcept { return lengthUpTo( start, end - start ); }
template <> inline void   CharacterFunctions::copyAll( char    * & dest, char    const * const src ) noexcept { std::strcpy( dest, src ); dest += std::strlen( src ); }
template <> inline void   CharacterFunctions::copyAll( wchar_t * & dest, wchar_t const * const src ) noexcept { std::wcscpy( dest, src ); dest += std::wcslen( src ); }
template <> inline void   CharacterFunctions::copyWithCharLimit    ( char    * & dest, char    const * const src, int const maxChars ) noexcept { std::strncpy( dest, src, maxChars - 1 ); dest[ maxChars - 1 ] = '\0'; dest += std::strlen( dest ); }
template <> inline void   CharacterFunctions::copyWithCharLimit    ( wchar_t * & dest, wchar_t const * const src, int const maxChars ) noexcept { std::wcsncpy( dest, src, maxChars - 1 ); dest[ maxChars - 1 ] = '\0'; dest += std::wcslen( dest ); }
template <> inline size_t CharacterFunctions::copyWithDestByteLimit( char    * & dest, char    const * const src, size_t const maxBytesToWrite ) noexcept { char    const * const destStart( dest ); copyWithCharLimit( dest, src, static_cast<unsigned int>( maxBytesToWrite / sizeof( *src ) ) ); return ( dest - destStart + 1 ) * sizeof( *src ); }
template <> inline size_t CharacterFunctions::copyWithDestByteLimit( wchar_t * & dest, wchar_t const * const src, size_t const maxBytesToWrite ) noexcept { wchar_t const * const destStart( dest ); copyWithCharLimit( dest, src, static_cast<unsigned int>( maxBytesToWrite / sizeof( *src ) ) ); return ( dest - destStart + 1 ) * sizeof( *src ); }
template <> inline int    CharacterFunctions::compare( char    const * const s1, char    const * const s2 ) noexcept { return std::strcmp( s1, s2 ); }
template <> inline int    CharacterFunctions::compare( wchar_t const * const s1, wchar_t const * const s2 ) noexcept { return std::wcscmp( s1, s2 ); }
template <> inline int    CharacterFunctions::compareUpTo( char    const * const s1, char    const * const s2, int const maxChars ) noexcept { return std::strncmp( s1, s2, maxChars ); }
template <> inline int    CharacterFunctions::compareUpTo( wchar_t const * const s1, wchar_t const * const s2, int const maxChars ) noexcept { return std::wcsncmp( s1, s2, maxChars ); }
#ifdef _WIN32
template <> inline int    CharacterFunctions::compareIgnoreCase( char    const * const s1, char    const * const s2 ) noexcept { return ::CompareStringA( LOCALE_USER_DEFAULT, LINGUISTIC_IGNORECASE, s1, -1, s2, -1 ); }
template <> inline int    CharacterFunctions::compareIgnoreCase( wchar_t const * const s1, wchar_t const * const s2 ) noexcept { return ::CompareStringW( LOCALE_USER_DEFAULT, LINGUISTIC_IGNORECASE, s1, -1, s2, -1 ); }
#else
template <> inline int    CharacterFunctions::compareIgnoreCase( char    const * const s1, char    const * const s2 ) noexcept { return /*std*/::strcasecmp( s1, s2 ); }
template <> inline int    CharacterFunctions::compareIgnoreCase( wchar_t const * const s1, wchar_t const * const s2 ) noexcept { return /*std*/::wcscasecmp( s1, s2 ); }
#endif // _WIN32
#ifdef _WIN32
template <> inline int    CharacterFunctions::compareIgnoreCaseUpTo( char    const * const s1, char    const * const s2, int const maxChars ) noexcept { return ::CompareStringA( LOCALE_USER_DEFAULT, LINGUISTIC_IGNORECASE, s1, static_cast<unsigned int>( lengthUpTo( s1, maxChars ) ), s2, static_cast<unsigned int>( lengthUpTo( s1, maxChars ) ) ); }
template <> inline int    CharacterFunctions::compareIgnoreCaseUpTo( wchar_t const * const s1, wchar_t const * const s2, int const maxChars ) noexcept { return ::CompareStringW( LOCALE_USER_DEFAULT, LINGUISTIC_IGNORECASE, s1, static_cast<unsigned int>( lengthUpTo( s1, maxChars ) ), s2, static_cast<unsigned int>( lengthUpTo( s1, maxChars ) ) ); }
#else
template <> inline int    CharacterFunctions::compareIgnoreCaseUpTo( char    const * const s1, char    const * const s2, int const maxChars ) noexcept { return /*std*/::strncasecmp( s1, s2, maxChars ); }
template <> inline int    CharacterFunctions::compareIgnoreCaseUpTo( wchar_t const * const s1, wchar_t const * const s2, int const maxChars ) noexcept { return /*std*/::wcsncasecmp( s1, s2, maxChars ); }
#endif // _WIN32
//template <> inline int          CharacterFunctions::indexOf( char const * const textToSearch, char const * const substringToLookFor) noexcept;
//template <> inline char const * CharacterFunctions::find (char const * const textToSearch, char const * const substringToLookFor) noexcept;
//template <> inline int          CharacterFunctions::indexOfIgnoreCase (char const * const haystack, char const * const needle) noexcept;
//template <> inline int          CharacterFunctions::indexOfChar (char const * const text, const juce_wchar charToFind) noexcept;
//template <> inline int          CharacterFunctions::indexOfCharIgnoreCase (char const * const text, juce_wchar charToFind) noexcept;

template <> int CharacterFunctions::getIntValue<int, CharPointer_ASCII>( CharPointer_ASCII ) noexcept;
#endif // LE_PATCHED_JUCE

#endif   // JUCE_CHARACTERFUNCTIONS_H_INCLUDED
