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

#ifndef JUCE_VARIANT_H_INCLUDED
#define JUCE_VARIANT_H_INCLUDED


//==============================================================================
/**
    A variant class, that can be used to hold a range of primitive values.

    A var object can hold a range of simple primitive values, strings, or
    any kind of ReferenceCountedObject. The var class is intended to act like
    the kind of values used in dynamic scripting languages.

    You can save/load var objects either in a small, proprietary binary format
    using writeToStream()/readFromStream(), or as JSON by using the JSON class.

    @see JSON, DynamicObject
*/
class JUCE_API  var
{
public:
    //==============================================================================
    typedef const var (DynamicObject::*MethodFunction) (const var* arguments, int numArguments);
    typedef Identifier identifier;

    //==============================================================================
    /** Creates a void variant. */
    var() noexcept;

    /** Destructor. */
    ~var() noexcept;

    /** A static var object that can be used where you need an empty variant object. */
    static const var null;

    var (const var& valueToCopy);
    var (int value) noexcept;
    JUCE_ORIGINAL( var (int64 value) noexcept; )
    var (bool value) noexcept;
    var (double value) noexcept;
    var (const char* value);
    var (const wchar_t* value);
    var (const String& value);
#ifndef LE_PATCHED_JUCE
    var (const Array<var>& value);
    var (ReferenceCountedObject* object);
    var (MethodFunction method) noexcept;
    var (const void* binaryData, size_t dataSize);
    var (const MemoryBlock& binaryData);
#endif // LE_PATCHED_JUCE

    var& operator= (const var& valueToCopy);
    var& operator= (int value);
    JUCE_ORIGINAL( var& operator= (int64 value); )
    var& operator= (bool value);
    var& operator= (double value);
    var& operator= (const char* value);
    var& operator= (const wchar_t* value);
    var& operator= (const String& value);
#ifndef LE_PATCHED_JUCE
    var& operator= (const Array<var>& value);
    var& operator= (ReferenceCountedObject* object);
    var& operator= (MethodFunction method);
#endif // LE_PATCHED_JUCE
   #if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
    var (var&& other) noexcept;
    var (String&& value);
    JUCE_ORIGINAL( var (MemoryBlock&& binaryData); )
    var& operator= (var&& other) noexcept;
    var& operator= (String&& value);
   #endif

    void swapWith (var& other) noexcept;

    //==============================================================================
    operator int() const noexcept;
    JUCE_ORIGINAL( operator int64() const noexcept; )
    operator bool() const noexcept;
    operator float() const noexcept;
    operator double() const noexcept;
    operator String() const;
    String toString() const;

    /** If this variant holds an array, this provides access to it.
        NOTE: Beware when you use this - the array pointer is only valid for the lifetime
        of the variant that returned it, so be very careful not to call this method on temporary
        var objects that are the return-value of a function, and which may go out of scope before
        you use the array!
    */
    Array<var>* getArray() const noexcept;

    /** If this variant holds a memory block, this provides access to it.
        NOTE: Beware when you use this - the MemoryBlock pointer is only valid for the lifetime
        of the variant that returned it, so be very careful not to call this method on temporary
        var objects that are the return-value of a function, and which may go out of scope before
        you use the MemoryBlock!
    */
    MemoryBlock* getBinaryData() const noexcept;

    ReferenceCountedObject* getObject() const noexcept;
    DynamicObject* getDynamicObject() const noexcept;

    //==============================================================================
    bool isVoid() const noexcept;
    bool isInt() const noexcept;
    bool isInt64() const noexcept;
    bool isBool() const noexcept;
    bool isDouble() const noexcept;
    bool isString() const noexcept;
    bool isObject() const noexcept;
    bool isArray() const noexcept;
    bool isBinaryData() const noexcept;
    bool isMethod() const noexcept;

    /** Returns true if this var has the same value as the one supplied.
        Note that this ignores the type, so a string var "123" and an integer var with the
        value 123 are considered to be equal.
        @see equalsWithSameType
    */
    bool equals (const var& other) const noexcept;

    /** Returns true if this var has the same value and type as the one supplied.
        This differs from equals() because e.g. "123" and 123 will be considered different.
        @see equals
    */
    bool equalsWithSameType (const var& other) const noexcept;

    //==============================================================================
    /** If the var is an array, this returns the number of elements.
        If the var isn't actually an array, this will return 0.
    */
    int size() const;

    /** If the var is an array, this can be used to return one of its elements.
        To call this method, you must make sure that the var is actually an array, and
        that the index is a valid number. If these conditions aren't met, behaviour is
        undefined.
        For more control over the array's contents, you can call getArray() and manipulate
        it directly as an Array\<var\>.
    */
    JUCE_ORIGINAL( const var& operator[] (int arrayIndex) const; )

    /** If the var is an array, this can be used to return one of its elements.
        To call this method, you must make sure that the var is actually an array, and
        that the index is a valid number. If these conditions aren't met, behaviour is
        undefined.
        For more control over the array's contents, you can call getArray() and manipulate
        it directly as an Array\<var\>.
    */
    JUCE_ORIGINAL( var& operator[] (int arrayIndex); )

    /** Appends an element to the var, converting it to an array if it isn't already one.
        If the var isn't an array, it will be converted to one, and if its value was non-void,
        this value will be kept as the first element of the new array. The parameter value
        will then be appended to it.
        For more control over the array's contents, you can call getArray() and manipulate
        it directly as an Array\<var\>.
    */
    JUCE_ORIGINAL( void append (const var& valueToAppend); )

    /** Inserts an element to the var, converting it to an array if it isn't already one.
        If the var isn't an array, it will be converted to one, and if its value was non-void,
        this value will be kept as the first element of the new array. The parameter value
        will then be inserted into it.
        For more control over the array's contents, you can call getArray() and manipulate
        it directly as an Array\<var\>.
    */
    JUCE_ORIGINAL( void insert (int index, const var& value); )

    /** If the var is an array, this removes one of its elements.
        If the index is out-of-range or the var isn't an array, nothing will be done.
        For more control over the array's contents, you can call getArray() and manipulate
        it directly as an Array\<var\>.
    */
    JUCE_ORIGINAL( void remove (int index); )

    /** Treating the var as an array, this resizes it to contain the specified number of elements.
        If the var isn't an array, it will be converted to one, and if its value was non-void,
        this value will be kept as the first element of the new array before resizing.
        For more control over the array's contents, you can call getArray() and manipulate
        it directly as an Array\<var\>.
    */
    JUCE_ORIGINAL( void resize (int numArrayElementsWanted); )

    /** If the var is an array, this searches it for the first occurrence of the specified value,
        and returns its index.
        If the var isn't an array, or if the value isn't found, this returns -1.
    */
    JUCE_ORIGINAL( int indexOf (const var& value) const; )

    //==============================================================================
    /** If this variant is an object, this returns one of its properties. */
    var operator[] (const Identifier propertyName) const;
    /** If this variant is an object, this returns one of its properties. */
    var operator[] (const char* propertyName) const;
    /** If this variant is an object, this returns one of its properties, or a default
        fallback value if the property is not set. */
    var getProperty (const Identifier propertyName, const var& defaultReturnValue) const;

    /** If this variant is an object, this invokes one of its methods with no arguments. */
    var call (const Identifier method) const;
    /** If this variant is an object, this invokes one of its methods with one argument. */
    var call (const Identifier method, const var& arg1) const;
    /** If this variant is an object, this invokes one of its methods with 2 arguments. */
    var call (const Identifier method, const var& arg1, const var& arg2) const;
    /** If this variant is an object, this invokes one of its methods with 3 arguments. */
    var call (const Identifier method, const var& arg1, const var& arg2, const var& arg3);
    /** If this variant is an object, this invokes one of its methods with 4 arguments. */
    var call (const Identifier method, const var& arg1, const var& arg2, const var& arg3, const var& arg4) const;
    /** If this variant is an object, this invokes one of its methods with 5 arguments. */
    var call (const Identifier method, const var& arg1, const var& arg2, const var& arg3, const var& arg4, const var& arg5) const;
    /** If this variant is an object, this invokes one of its methods with a list of arguments. */
    var invoke (const Identifier method, const var* arguments, int numArguments) const;

    //==============================================================================
    /** Writes a binary representation of this value to a stream.
        The data can be read back later using readFromStream().
        @see JSON
    */
    JUCE_ORIGINAL( void writeToStream (OutputStream& output) const; )

    /** Reads back a stored binary representation of a value.
        The data in the stream must have been written using writeToStream(), or this
        will have unpredictable results.
        @see JSON
    */
    static var readFromStream (InputStream& input);

private:
    //==============================================================================
    class VariantType;         friend class VariantType;
    class VariantType_Void;    friend class VariantType_Void;
    class VariantType_Int;     friend class VariantType_Int;
    class VariantType_Int64;   friend class VariantType_Int64;
    class VariantType_Double;  friend class VariantType_Double;
    class VariantType_Bool;    friend class VariantType_Bool;
    class VariantType_String;  friend class VariantType_String;
    class VariantType_Object;  friend class VariantType_Object;
    class VariantType_Array;   friend class VariantType_Array;
    class VariantType_Binary;  friend class VariantType_Binary;
    class VariantType_Method;  friend class VariantType_Method;

    union ValueUnion
    {
        int intValue;
        int64 int64Value;
        bool boolValue;
        double doubleValue;
        char stringValue [sizeof (String)];
        ReferenceCountedObject* objectValue;
        Array<var>* arrayValue;
        MemoryBlock* binaryValue;
        MethodFunction methodValue;
    };

    const VariantType* type;
    LE_PATCH( public: ) ValueUnion value; LE_PATCH( private: )

#ifndef LE_PATCHED_JUCE
    Array<var>* convertToArray();
    friend class DynamicObject;
    var invokeMethod (DynamicObject*, const var*, int) const;
#endif // LE_PATCHED_JUCE
};

/** Compares the values of two var objects, using the var::equals() comparison. */
bool operator== (const var& v1, const var& v2) noexcept;
/** Compares the values of two var objects, using the var::equals() comparison. */
bool operator!= (const var& v1, const var& v2) noexcept;
bool operator== (const var& v1, const String& v2);
bool operator!= (const var& v1, const String& v2);
bool operator== (const var& v1, const char* v2);
bool operator!= (const var& v1, const char* v2);


#endif   // JUCE_VARIANT_H_INCLUDED
