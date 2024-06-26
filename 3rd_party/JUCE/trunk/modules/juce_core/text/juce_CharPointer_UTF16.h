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

#ifndef JUCE_CHARPOINTER_UTF16_H_INCLUDED
#define JUCE_CHARPOINTER_UTF16_H_INCLUDED

#ifdef LE_PATCHED_JUCE
    #include "juce_CharPointer_ASCII.h"
#endif // LE_PATCHED_JUCE

//==============================================================================
/**
    Wraps a pointer to a null-terminated UTF-16 character string, and provides
    various methods to operate on the data.
    @see CharPointer_UTF8, CharPointer_UTF32
*/
class CharPointer_UTF16
{
public:
   #if JUCE_NATIVE_WCHAR_IS_UTF16
    typedef wchar_t CharType;
   #else
    typedef int16 CharType;
   #endif

    inline explicit CharPointer_UTF16 (const CharType* const rawPointer) noexcept
        : data (const_cast <CharType*> (rawPointer))
    {
    }

    inline CharPointer_UTF16 (const CharPointer_UTF16& other) noexcept
        : data (other.data)
    {
    }

    inline CharPointer_UTF16 operator= (CharPointer_UTF16 other) noexcept
    {
        data = other.data;
        return *this;
    }

    inline CharPointer_UTF16 operator= (const CharType* text) noexcept
    {
        data = const_cast <CharType*> (text);
        return *this;
    }

    /** This is a pointer comparison, it doesn't compare the actual text. */
    inline bool operator== (CharPointer_UTF16 other) const noexcept     { return data == other.data; }
    inline bool operator!= (CharPointer_UTF16 other) const noexcept     { return data != other.data; }
    inline bool operator<= (CharPointer_UTF16 other) const noexcept     { return data <= other.data; }
    inline bool operator<  (CharPointer_UTF16 other) const noexcept     { return data <  other.data; }
    inline bool operator>= (CharPointer_UTF16 other) const noexcept     { return data >= other.data; }
    inline bool operator>  (CharPointer_UTF16 other) const noexcept     { return data >  other.data; }

    /** Returns the address that this pointer is pointing to. */
    inline CharType* getAddress() const noexcept        { return data; }

    /** Returns the address that this pointer is pointing to. */
    inline operator const CharType*() const noexcept    { return data; }

    /** Returns true if this pointer is pointing to a null character. */
    inline bool isEmpty() const noexcept                { return *data == 0; }

    /** Returns the unicode character that this pointer is pointing to. */
    juce_wchar operator*() const noexcept
    {
    #if defined( LE_PATCHED_JUCE ) && defined( _WIN32 )
        return *data;
    #else
        uint32 n = (uint32) (uint16) *data;

        if (n >= 0xd800 && n <= 0xdfff && ((uint32) (uint16) data[1]) >= 0xdc00)
            n = 0x10000 + (((n - 0xd800) << 10) | (((uint32) (uint16) data[1]) - 0xdc00));

        return (juce_wchar) n;
    #endif // LE_PATCHED_JUCE
    }

    /** Moves this pointer along to the next character in the string. */
    CharPointer_UTF16 operator++() noexcept
    {
    #if defined( LE_PATCHED_JUCE ) && defined( _WIN32 )
        ++data;
        return *this;
    #else
        const juce_wchar n = *data++;

        if (n >= 0xd800 && n <= 0xdfff && ((uint32) (uint16) *data) >= 0xdc00)
            ++data;

        return *this;
    #endif // LE_PATCHED_JUCE
    }

    /** Moves this pointer back to the previous character in the string. */
    CharPointer_UTF16 operator--() noexcept
    {
    #if defined( LE_PATCHED_JUCE ) && defined( _WIN32 )
        --data;
        return *this;
    #else
        const juce_wchar n = *--data;

        if (n >= 0xdc00 && n <= 0xdfff)
            --data;

        return *this;
    #endif // LE_PATCHED_JUCE
    }

    /** Returns the character that this pointer is currently pointing to, and then
        advances the pointer to point to the next character. */
    juce_wchar getAndAdvance() noexcept
    {
    #if defined( LE_PATCHED_JUCE ) && defined( _WIN32 )
        return *data++;
    #else
        uint32 n = (uint32) (uint16) *data++;

        if (n >= 0xd800 && n <= 0xdfff && ((uint32) (uint16) *data) >= 0xdc00)
            n = 0x10000 + ((((n - 0xd800) << 10) | (((uint32) (uint16) *data++) - 0xdc00)));

        return (juce_wchar) n;
    #endif // LE_PATCHED_JUCE
    }

    /** Moves this pointer along to the next character in the string. */
    CharPointer_UTF16 operator++ (int) noexcept
    {
        CharPointer_UTF16 temp (*this);
        ++*this;
        return temp;
    }

    /** Moves this pointer forwards by the specified number of characters. */
    void operator+= (int numToSkip) noexcept
    {
    #if defined( LE_PATCHED_JUCE ) && defined( _WIN32 )
        data += numToSkip;
    #else
        if (numToSkip < 0)
        {
            while (++numToSkip <= 0)
                --*this;
        }
        else
        {
            while (--numToSkip >= 0)
                ++*this;
        }
    #endif // LE_PATCHED_JUCE
    }

    /** Moves this pointer backwards by the specified number of characters. */
    void operator-= (int numToSkip) noexcept
    {
        operator+= (-numToSkip);
    }

    /** Returns the character at a given character index from the start of the string. */
    juce_wchar operator[] (const int characterIndex) const noexcept
    {
    #if defined( LE_PATCHED_JUCE ) && defined( _WIN32 )
        return data[ characterIndex ];
    #else
        CharPointer_UTF16 p (*this);
        p += characterIndex;
        return *p;
    #endif // LE_PATCHED_JUCE
    }

    /** Returns a pointer which is moved forwards from this one by the specified number of characters. */
    CharPointer_UTF16 operator+ (const int numToSkip) const noexcept
    {
        CharPointer_UTF16 p (*this);
        p += numToSkip;
        return p;
    }

    /** Returns a pointer which is moved backwards from this one by the specified number of characters. */
    CharPointer_UTF16 operator- (const int numToSkip) const noexcept
    {
        CharPointer_UTF16 p (*this);
        p += -numToSkip;
        return p;
    }

    /** Writes a unicode character to this string, and advances this pointer to point to the next position. */
    void write (juce_wchar charToWrite) noexcept
    {
    #ifdef LE_PATCHED_JUCE
        jassert(!(charToWrite >= 0x10000));
    #else
        if (charToWrite >= 0x10000)
        {
            charToWrite -= 0x10000;
            *data++ = (CharType) (0xd800 + (charToWrite >> 10));
            *data++ = (CharType) (0xdc00 + (charToWrite & 0x3ff));
        }
        else
    #endif // LE_PATCHED_JUCE
        {
            *data++ = (CharType) charToWrite;
        }
    }

    /** Writes a null character to this string (leaving the pointer's position unchanged). */
    inline void writeNull() const noexcept
    {
        *data = 0;
    }

    /** Returns the number of characters in this string. */
    size_t length() const noexcept
    {
    #if defined( LE_PATCHED_JUCE ) && defined( _WIN32 )
        return std::wcslen( data );
    #else
        const CharType* d = data;
        size_t count = 0;

        for (;;)
        {
            const int n = *d++;

            if (n >= 0xd800 && n <= 0xdfff)
            {
                if (*d++ == 0)
                    break;
            }
            else if (n == 0)
                break;

            ++count;
        }

        return count;
    #endif // LE_PATCHED_JUCE
    }

    /** Returns the number of characters in this string, or the given value, whichever is lower. */
    size_t lengthUpTo (const size_t maxCharsToCount) const noexcept
    {
    #if defined( LE_PATCHED_JUCE ) && defined( _WIN32 )
        return /*std*/::wcsnlen( data, maxCharsToCount );
    #else
        return CharacterFunctions::lengthUpTo (*this, maxCharsToCount);
    #endif // LE_PATCHED_JUCE
    }

    /** Returns the number of characters in this string, or up to the given end pointer, whichever is lower. */
    size_t lengthUpTo (const CharPointer_UTF16 end) const noexcept
    {
        return CharacterFunctions::lengthUpTo (*this, end);
    }

    /** Returns the number of bytes that are used to represent this string.
        This includes the terminating null character.
    */
    size_t sizeInBytes() const noexcept
    {
    #if defined( LE_PATCHED_JUCE ) && defined( _WIN32 )
        return sizeof (CharType) * ( length() + 1 );
    #else
        return sizeof (CharType) * (findNullIndex (data) + 1);
    #endif // LE_PATCHED_JUCE
    }

    /** Returns the number of bytes that would be needed to represent the given
        unicode character in this encoding format.
    */
    static size_t getBytesRequiredFor (const juce_wchar charToWrite) noexcept
    {
    #if defined( LE_PATCHED_JUCE ) && defined( _WIN32 )
        jassert(!(charToWrite >= 0x10000));charToWrite;
        return sizeof (CharType);
    #else
        return (charToWrite >= 0x10000) ? (sizeof (CharType) * 2) : sizeof (CharType);
    #endif // LE_PATCHED_JUCE
    }

    /** Returns the number of bytes that would be needed to represent the given
        string in this encoding format.
        The value returned does NOT include the terminating null character.
    */
    template <class CharPointer>
    static size_t getBytesRequiredFor (CharPointer text) noexcept
    {
        size_t count = 0;
        juce_wchar n;

        while ((n = text.getAndAdvance()) != 0)
            count += getBytesRequiredFor (n);

        return count;
    }

    #if defined( LE_PATCHED_JUCE ) && defined( _WIN32 )
    static std::size_t getBytesRequiredFor( CharPointer_UTF16 const text ) noexcept
    {
        return std::wcslen( text );
    }
    #endif // LE_PATCHED_JUCE
    /** Returns a pointer to the null character that terminates this string. */
    CharPointer_UTF16 findTerminatingNull() const noexcept
    {
    #if defined( LE_PATCHED_JUCE ) && defined( _WIN32 )
        wchar_t const * const pNull( data + std::wcslen( data ) );
        jassert( *pNull == 0 );
        return CharPointer_UTF16( pNull );
    #else
        const CharType* t = data;

        while (*t != 0)
            ++t;

        return CharPointer_UTF16 (t);
    #endif // LE_PATCHED_JUCE
    }

    /** Copies a source string to this pointer, advancing this pointer as it goes. */
    template <typename CharPointer>
    void writeAll (const CharPointer src) noexcept
    {
        CharacterFunctions::copyAll (*this, src);
    }

    /** Copies a source string to this pointer, advancing this pointer as it goes. */
    void writeAll (const CharPointer_UTF16 src) noexcept
    {
        const CharType* s = src.data;

        while ((*data = *s) != 0)
        {
            ++data;
            ++s;
        }
    }

    /** Copies a source string to this pointer, advancing this pointer as it goes.
        The maxDestBytes parameter specifies the maximum number of bytes that can be written
        to the destination buffer before stopping.
    */
    template <typename CharPointer>
    size_t writeWithDestByteLimit (const CharPointer src, const size_t maxDestBytes) noexcept
    {
        return CharacterFunctions::copyWithDestByteLimit (*this, src, maxDestBytes);
    }

    /** Copies a source string to this pointer, advancing this pointer as it goes.
        The maxChars parameter specifies the maximum number of characters that can be
        written to the destination buffer before stopping (including the terminating null).
    */
    template <typename CharPointer>
    void writeWithCharLimit (const CharPointer src, const int maxChars) noexcept
    {
        CharacterFunctions::copyWithCharLimit (*this, src, maxChars);
    }

    /** Compares this string with another one. */
    template <typename CharPointer>
    int compare (const CharPointer other) const noexcept
    {
        return CharacterFunctions::compare (*this, other);
    }

#if defined( LE_PATCHED_JUCE ) && defined( _WIN32 )
    int compare ( CharPointer_UTF16 const other) const noexcept
    {
        return std::wcscmp( data, other.data );
    }
#endif // LE_PATCHED_JUCE

    /** Compares this string with another one, up to a specified number of characters. */
    template <typename CharPointer>
    int compareUpTo (const CharPointer other, const int maxChars) const noexcept
    {
        return CharacterFunctions::compareUpTo (*this, other, maxChars);
    }

    /** Compares this string with another one. */
    template <typename CharPointer>
    int compareIgnoreCase (const CharPointer other) const noexcept
    {
        return CharacterFunctions::compareIgnoreCase (*this, other);
    }

    /** Compares this string with another one, up to a specified number of characters. */
    template <typename CharPointer>
    int compareIgnoreCaseUpTo (const CharPointer other, const int maxChars) const noexcept
    {
        return CharacterFunctions::compareIgnoreCaseUpTo (*this, other, maxChars);
    }

   #if JUCE_WINDOWS && ! DOXYGEN
#ifndef LE_PATCHED_JUCE
    int compareIgnoreCase (const CharPointer_UTF16 other) const noexcept
    {
        return _wcsicmp (data, other.data);
    }

    int compareIgnoreCaseUpTo (const CharPointer_UTF16 other, int maxChars) const noexcept
    {
        return _wcsnicmp (data, other.data, (size_t) maxChars);
    }
#endif // LE_PATCHED_JUCE

    int indexOf (const CharPointer_UTF16 stringToFind) const noexcept
    {
        const CharType* const t = wcsstr (data, stringToFind.getAddress());
        return t == nullptr ? -1 : (int) (t - data);
    }
   #endif

    /** Returns the character index of a substring, or -1 if it isn't found. */
    template <typename CharPointer>
    int indexOf (const CharPointer stringToFind) const noexcept
    {
        return CharacterFunctions::indexOf (*this, stringToFind);
    }

    /** Returns the character index of a unicode character, or -1 if it isn't found. */
    int indexOf (const juce_wchar charToFind) const noexcept
    {
        #if defined( LE_PATCHED_JUCE ) && defined( _WIN32 )
            CharType const * const pChar( /*std*/::wcschr( data, charToFind ) );
            return pChar ? static_cast<int>( pChar - data ) : -1;
        #else
            return CharacterFunctions::indexOfChar (*this, charToFind);
        #endif // LE_PATCHED_JUCE
    }

    /** Returns the character index of a unicode character, or -1 if it isn't found. */
    int indexOf (const juce_wchar charToFind, const bool ignoreCase) const noexcept
    {
        return ignoreCase ? CharacterFunctions::indexOfCharIgnoreCase (*this, charToFind)
                          JUCE_ORIGINAL( : CharacterFunctions::indexOfChar (*this, charToFind); )
                          LE_PATCH(      : this->indexOf( charToFind ); )
    }

    /** Returns true if the first character of this string is whitespace. */
    bool isWhitespace() const noexcept      { return CharacterFunctions::isWhitespace (operator*()) != 0; }
    /** Returns true if the first character of this string is a digit. */
    bool isDigit() const noexcept           { return CharacterFunctions::isDigit (operator*()) != 0; }
    /** Returns true if the first character of this string is a letter. */
    bool isLetter() const noexcept          { return CharacterFunctions::isLetter (operator*()) != 0; }
    /** Returns true if the first character of this string is a letter or digit. */
    bool isLetterOrDigit() const noexcept   { return CharacterFunctions::isLetterOrDigit (operator*()) != 0; }
    /** Returns true if the first character of this string is upper-case. */
    bool isUpperCase() const noexcept       { return CharacterFunctions::isUpperCase (operator*()) != 0; }
    /** Returns true if the first character of this string is lower-case. */
    bool isLowerCase() const noexcept       { return CharacterFunctions::isLowerCase (operator*()) != 0; }

    /** Returns an upper-case version of the first character of this string. */
    juce_wchar toUpperCase() const noexcept { return CharacterFunctions::toUpperCase (operator*()); }
    /** Returns a lower-case version of the first character of this string. */
    juce_wchar toLowerCase() const noexcept { return CharacterFunctions::toLowerCase (operator*()); }

    /** Parses this string as a 32-bit integer. */
    int getIntValue32() const noexcept
    {
       #if JUCE_WINDOWS
        #ifdef LE_PATCHED_JUCE
            char buffer[ 64 ];
            jassert( this->length() < numElementsInArray( buffer ) );
            CharPointer_ASCII ascii( buffer );
            CharacterFunctions::copyAll( ascii, *this );
            return CharacterFunctions::getIntValue<int>( ascii );
        #else
            return _wtoi (data);
        #endif // LE_PATCHED_JUCE
       #else
        return CharacterFunctions::getIntValue <int, CharPointer_UTF16> (*this);
       #endif
    }

    /** Parses this string as a 64-bit integer. */
#ifndef LE_PATCHED_JUCE
    int64 getIntValue64() const noexcept
    {
       #if JUCE_WINDOWS
        return _wtoi64 (data);
       #else
        return CharacterFunctions::getIntValue <int64, CharPointer_UTF16> (*this);
       #endif
    }
#endif // LE_PATCHED_JUCE

    /** Parses this string as a floating point double. */
    double getDoubleValue() const noexcept  { return CharacterFunctions::getDoubleValue (*this); }

    /** Returns the first non-whitespace character in the string. */
    CharPointer_UTF16 findEndOfWhitespace() const noexcept   { return CharacterFunctions::findEndOfWhitespace (*this); }

    /** Returns true if the given unicode character can be represented in this encoding. */
    static bool canRepresent (juce_wchar character) noexcept
    {
    #if defined( LE_PATCHED_JUCE ) && defined( _WIN32 )
        jassert(((unsigned int) character) < (unsigned int) 0x10ffff
            && (((unsigned int) character) < 0xd800 || ((unsigned int) character) > 0xdfff));
        character;
        return true;
    #else
        return ((unsigned int) character) < (unsigned int) 0x10ffff
                 && (((unsigned int) character) < 0xd800 || ((unsigned int) character) > 0xdfff);
    #endif // LE_PATCHED_JUCE
    }

    /** Returns true if this data contains a valid string in this encoding. */
    static bool isValidString (const CharType* dataToTest, int maxBytesToRead)
    {
    #if defined( LE_PATCHED_JUCE ) && defined( _WIN32 ) && defined( NDEBUG )
        dataToTest; maxBytesToRead;
        return true;
    #else
        maxBytesToRead /= sizeof (CharType);

        while (--maxBytesToRead >= 0 && *dataToTest != 0)
        {
            const uint32 n = (uint32) (uint16) *dataToTest++;

            if (n >= 0xd800)
            {
                if (n > 0x10ffff)
                    return false;

                if (n <= 0xdfff)
                {
                    if (n > 0xdc00)
                        return false;

                    const uint32 nextChar = (uint32) (uint16) *dataToTest++;

                    if (nextChar < 0xdc00 || nextChar > 0xdfff)
                        return false;
                }
            }
        }

        return true;
    #endif // LE_PATCHED_JUCE
    }

    /** Atomically swaps this pointer for a new value, returning the previous value. */
    CharPointer_UTF16 atomicSwap (const CharPointer_UTF16 newValue)
    {
        return CharPointer_UTF16 (reinterpret_cast <Atomic<CharType*>&> (data).exchange (newValue.data));
    }

    /** These values are the byte-order-mark (BOM) values for a UTF-16 stream. */
    enum
    {
        byteOrderMarkBE1 = 0xfe,
        byteOrderMarkBE2 = 0xff,
        byteOrderMarkLE1 = 0xff,
        byteOrderMarkLE2 = 0xfe
    };

private:
    CharType* data;

    static unsigned int findNullIndex (const CharType* const t) noexcept
    {
        unsigned int n = 0;

        while (t[n] != 0)
            ++n;

        return n;
    }
};


#endif   // JUCE_CHARPOINTER_UTF16_H_INCLUDED
