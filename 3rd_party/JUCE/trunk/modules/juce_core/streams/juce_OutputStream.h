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

#ifndef JUCE_OUTPUTSTREAM_H_INCLUDED
#define JUCE_OUTPUTSTREAM_H_INCLUDED


//==============================================================================
/**
    The base class for streams that write data to some kind of destination.

    Input and output streams are used throughout the library - subclasses can override
    some or all of the virtual functions to implement their behaviour.

    @see InputStream, MemoryOutputStream, FileOutputStream
*/
class JUCE_API  OutputStream
{
protected:
    //==============================================================================
    OutputStream();

public:
    /** Destructor.

        Some subclasses might want to do things like call flush() during their
        destructors.
    */
    virtual ~OutputStream() LE_PATCH( noexcept );

    //==============================================================================
    /** If the stream is using a buffer, this will ensure it gets written
        out to the destination. */
    virtual void flush() LE_PATCH( noexcept ) = 0;

    /** Tries to move the stream's output position.

        Not all streams will be able to seek to a new position - this will return
        false if it fails to work.

        @see getPosition
    */
    virtual bool setPosition (int64 newPosition) LE_PATCH( noexcept ) = 0;

    /** Returns the stream's current position.

        @see setPosition
    */
    virtual int64 getPosition() LE_PATCH( noexcept ) = 0;

    //==============================================================================
    /** Writes a block of data to the stream.

        When creating a subclass of OutputStream, this is the only write method
        that needs to be overloaded - the base class has methods for writing other
        types of data which use this to do the work.

        @param dataToWrite      the target buffer to receive the data. This must not be null.
        @param numberOfBytes    the number of bytes to write.
        @returns false if the write operation fails for some reason
    */
    virtual bool write (const void* dataToWrite,
                        size_t numberOfBytes) LE_PATCH( noexcept ) = 0;

    //==============================================================================
    /** Writes a single byte to the stream.
        @returns false if the write operation fails for some reason
        @see InputStream::readByte
    */
    virtual bool writeByte (char byte) LE_PATCH( noexcept );

    /** Writes a boolean to the stream as a single byte.
        This is encoded as a binary byte (not as text) with a value of 1 or 0.
        @returns false if the write operation fails for some reason
        @see InputStream::readBool
    */
    virtual bool writeBool (bool boolValue) LE_PATCH( noexcept );

    /** Writes a 16-bit integer to the stream in a little-endian byte order.
        This will write two bytes to the stream: (value & 0xff), then (value >> 8).
        @returns false if the write operation fails for some reason
        @see InputStream::readShort
    */
    virtual bool writeShort (short value) LE_PATCH( noexcept );

    /** Writes a 16-bit integer to the stream in a big-endian byte order.
        This will write two bytes to the stream: (value >> 8), then (value & 0xff).
        @returns false if the write operation fails for some reason
        @see InputStream::readShortBigEndian
    */
    virtual bool writeShortBigEndian (short value) LE_PATCH( noexcept );

    /** Writes a 32-bit integer to the stream in a little-endian byte order.
        @returns false if the write operation fails for some reason
        @see InputStream::readInt
    */
    virtual bool writeInt (int value) LE_PATCH( noexcept );

    /** Writes a 32-bit integer to the stream in a big-endian byte order.
        @returns false if the write operation fails for some reason
        @see InputStream::readIntBigEndian
    */
    virtual bool writeIntBigEndian (int value) LE_PATCH( noexcept );

    /** Writes a 64-bit integer to the stream in a little-endian byte order.
        @returns false if the write operation fails for some reason
        @see InputStream::readInt64
    */
    virtual bool writeInt64 (int64 value) LE_PATCH( noexcept );

    /** Writes a 64-bit integer to the stream in a big-endian byte order.
        @returns false if the write operation fails for some reason
        @see InputStream::readInt64BigEndian
    */
    virtual bool writeInt64BigEndian (int64 value) LE_PATCH( noexcept );

    /** Writes a 32-bit floating point value to the stream in a binary format.
        The binary 32-bit encoding of the float is written as a little-endian int.
        @returns false if the write operation fails for some reason
        @see InputStream::readFloat
    */
    virtual bool writeFloat (float value) LE_PATCH( noexcept );

    /** Writes a 32-bit floating point value to the stream in a binary format.
        The binary 32-bit encoding of the float is written as a big-endian int.
        @returns false if the write operation fails for some reason
        @see InputStream::readFloatBigEndian
    */
    virtual bool writeFloatBigEndian (float value) LE_PATCH( noexcept );

    /** Writes a 64-bit floating point value to the stream in a binary format.
        The eight raw bytes of the double value are written out as a little-endian 64-bit int.
        @returns false if the write operation fails for some reason
        @see InputStream::readDouble
    */
    virtual bool writeDouble (double value) LE_PATCH( noexcept );

    /** Writes a 64-bit floating point value to the stream in a binary format.
        The eight raw bytes of the double value are written out as a big-endian 64-bit int.
        @see InputStream::readDoubleBigEndian
        @returns false if the write operation fails for some reason
    */
    virtual bool writeDoubleBigEndian (double value) LE_PATCH( noexcept );

    /** Writes a byte to the output stream a given number of times.
        @returns false if the write operation fails for some reason
    */
    virtual bool writeRepeatedByte (uint8 byte, size_t numTimesToRepeat) LE_PATCH( noexcept );

    /** Writes a condensed binary encoding of a 32-bit integer.

        If you're storing a lot of integers which are unlikely to have very large values,
        this can save a lot of space, because values under 0xff will only take up 2 bytes,
        under 0xffff only 3 bytes, etc.

        The format used is: number of significant bytes + up to 4 bytes in little-endian order.

        @returns false if the write operation fails for some reason
        @see InputStream::readCompressedInt
    */
    virtual bool writeCompressedInt (int value) LE_PATCH( noexcept );

    /** Stores a string in the stream in a binary format.

        This isn't the method to use if you're trying to append text to the end of a
        text-file! It's intended for storing a string so that it can be retrieved later
        by InputStream::readString().

        It writes the string to the stream as UTF8, including the null termination character.

        For appending text to a file, instead use writeText, or operator<<

        @returns false if the write operation fails for some reason
        @see InputStream::readString, writeText, operator<<
    */
    virtual bool writeString (const String& text) LE_PATCH( noexcept );

    /** Writes a string of text to the stream.

        It can either write the text as UTF-8 or UTF-16, and can also add the UTF-16 byte-order-mark
        bytes (0xff, 0xfe) to indicate the endianness (these should only be used at the start
        of a file).

        The method also replaces '\\n' characters in the text with '\\r\\n'.
        @returns false if the write operation fails for some reason
    */
    virtual bool writeText (const String& text,
                            bool asUTF16,
                            bool writeUTF16ByteOrderMark) LE_PATCH( noexcept );

    /** Reads data from an input stream and writes it to this stream.

        @param source               the stream to read from
        @param maxNumBytesToWrite   the number of bytes to read from the stream (if this is
                                    less than zero, it will keep reading until the input
                                    is exhausted)
        @returns the number of bytes written
    */
    virtual int writeFromInputStream (InputStream& source, int64 maxNumBytesToWrite);

    //==============================================================================
    /** Sets the string that will be written to the stream when the writeNewLine()
        method is called.
        By default this will be set the the value of NewLine::getDefault().
    */
    void setNewLineString (const String& newLineString);

    /** Returns the current new-line string that was set by setNewLineString(). */
    const String& getNewLineString() const noexcept         { return newLineString; }

private:
    //==============================================================================
    String newLineString;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OutputStream)
};

//==============================================================================
/** Writes a number to a stream as 8-bit characters in the default system encoding. */
JUCE_API OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, int number);

/** Writes a number to a stream as 8-bit characters in the default system encoding. */
JUCE_API OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, int64 number);

/** Writes a number to a stream as 8-bit characters in the default system encoding. */
JUCE_API OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, double number);

/** Writes a character to a stream. */
JUCE_API OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, char character);

/** Writes a null-terminated text string to a stream. */
JUCE_API OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const char* text);

/** Writes a block of data from a MemoryBlock to a stream. */
JUCE_API OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const MemoryBlock& data);

/** Writes the contents of a file to a stream. */
JUCE_API OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const File& fileToRead);

/** Writes the complete contents of an input stream to an output stream. */
JUCE_API OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, InputStream& streamToRead);

/** Writes a new-line to a stream.
    You can use the predefined symbol 'newLine' to invoke this, e.g.
    @code
    myOutputStream << "Hello World" << newLine << newLine;
    @endcode
    @see OutputStream::setNewLineString
*/
JUCE_API OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const NewLine&);


#endif   // JUCE_OUTPUTSTREAM_H_INCLUDED
