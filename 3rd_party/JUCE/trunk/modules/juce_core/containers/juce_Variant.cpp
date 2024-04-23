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

enum VariantStreamMarkers
{
    varMarker_Int       = 1,
    varMarker_BoolTrue  = 2,
    varMarker_BoolFalse = 3,
    varMarker_Double    = 4,
    varMarker_String    = 5,
    varMarker_Int64     = 6,
    varMarker_Array     = 7,
    varMarker_Binary    = 8
};

//==============================================================================
class LE_PATCH_NOVTABLE var::VariantType
{
public:
    VariantType() noexcept {}
    virtual ~VariantType() noexcept {}

    virtual int toInt (const ValueUnion&) const noexcept                        { return 0; }
    JUCE_ORIGINAL( virtual ) int64 toInt64 (const ValueUnion&) const noexcept                    { LE_PATCH_UNREACHABLE_CODE return 0; }
    virtual double toDouble (const ValueUnion&) const noexcept                  { return 0; }
    virtual String toString (const ValueUnion&) const                           { return String::empty; }
    virtual bool toBool (const ValueUnion&) const noexcept                      { return false; }
    JUCE_ORIGINAL( virtual ) ReferenceCountedObject* toObject (const ValueUnion&) const noexcept { LE_PATCH_UNREACHABLE_CODE return nullptr; }
    JUCE_ORIGINAL( virtual ) Array<var>* toArray (const ValueUnion&) const noexcept              { LE_PATCH_UNREACHABLE_CODE return 0; }
    JUCE_ORIGINAL( virtual ) MemoryBlock* toBinary (const ValueUnion&) const noexcept            { LE_PATCH_UNREACHABLE_CODE return nullptr; }

    virtual bool isVoid() const noexcept      { return false; }
    virtual bool isInt() const noexcept       { return false; }
    JUCE_ORIGINAL( virtual ) bool isInt64() const noexcept     { LE_PATCH_UNREACHABLE_CODE return false; }
    virtual bool isBool() const noexcept      { return false; }
    virtual bool isDouble() const noexcept    { return false; }
    virtual bool isString() const noexcept    { return false; }
    JUCE_ORIGINAL( virtual ) bool isObject() const noexcept    { LE_PATCH_UNREACHABLE_CODE return false; }
    JUCE_ORIGINAL( virtual ) bool isArray() const noexcept     { LE_PATCH_UNREACHABLE_CODE return false; }
    JUCE_ORIGINAL( virtual ) bool isBinary() const noexcept    { LE_PATCH_UNREACHABLE_CODE return false; }
    JUCE_ORIGINAL( virtual ) bool isMethod() const noexcept    { LE_PATCH_UNREACHABLE_CODE return false; }

    virtual void cleanUp (ValueUnion&) const noexcept {}
    virtual void createCopy (ValueUnion& dest, const ValueUnion& source) const LE_PATCH( noexcept )     { dest = source; }
    virtual bool equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) const noexcept = 0;
    JUCE_ORIGINAL( virtual void writeToStream (const ValueUnion& data, OutputStream& output) const = 0; )
};

//==============================================================================
class var::VariantType_Void  : public var::VariantType
{
public:
    VariantType_Void() noexcept {}
    static const VariantType_Void instance;

    bool isVoid() const noexcept    { return true; }
    bool equals (const ValueUnion&, const ValueUnion&, const VariantType& otherType) const noexcept { return otherType.isVoid(); }
    void writeToStream (const ValueUnion&, OutputStream& output) const   { output.writeCompressedInt (0); }
};

//==============================================================================
class var::VariantType_Int  : public var::VariantType
{
public:
    VariantType_Int() noexcept {}
    static const VariantType_Int instance;

    int toInt (const ValueUnion& data) const noexcept       { return data.intValue; };
    int64 toInt64 (const ValueUnion& data) const noexcept   { return (int64) data.intValue; };
    double toDouble (const ValueUnion& data) const noexcept { return (double) data.intValue; }
    String toString (const ValueUnion& data) const          { return String (data.intValue); }
    bool toBool (const ValueUnion& data) const noexcept     { return data.intValue != 0; }
    bool isInt() const noexcept                             { return true; }

    bool equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) const noexcept
    {
        return otherType.toInt (otherData) == data.intValue;
    }

    void writeToStream (const ValueUnion& data, OutputStream& output) const
    {
        output.writeCompressedInt (5);
        output.writeByte (varMarker_Int);
        output.writeInt (data.intValue);
    }
};

//==============================================================================
class var::VariantType_Int64  : public var::VariantType
{
public:
    VariantType_Int64() noexcept { LE_PATCH_UNREACHABLE_CODE }
    static const VariantType_Int64 instance;

    int toInt (const ValueUnion& data) const noexcept       { return (int) data.int64Value; };
    int64 toInt64 (const ValueUnion& data) const noexcept   { return data.int64Value; };
    double toDouble (const ValueUnion& data) const noexcept { return (double) data.int64Value; }
    String toString (const ValueUnion& data) const          { return String (data.int64Value); }
    bool toBool (const ValueUnion& data) const noexcept     { return data.int64Value != 0; }
    bool isInt64() const noexcept                           { return true; }

    bool equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) const noexcept
    {
        return otherType.toInt64 (otherData) == data.int64Value;
    }

    void writeToStream (const ValueUnion& data, OutputStream& output) const
    {
        output.writeCompressedInt (9);
        output.writeByte (varMarker_Int64);
        output.writeInt64 (data.int64Value);
    }
};

//==============================================================================
class var::VariantType_Double   : public var::VariantType
{
public:
    VariantType_Double() noexcept {}
    static const VariantType_Double instance;

    int toInt (const ValueUnion& data) const noexcept       { return (int) data.doubleValue; };
    int64 toInt64 (const ValueUnion& data) const noexcept   { return (int64) data.doubleValue; };
    double toDouble (const ValueUnion& data) const noexcept { return data.doubleValue; }
    String toString (const ValueUnion& data) const          { return String (data.doubleValue); }
    bool toBool (const ValueUnion& data) const noexcept     { return data.doubleValue != 0; }
    bool isDouble() const noexcept                          { return true; }

    bool equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) const noexcept
    {
        return std::abs (otherType.toDouble (otherData) - data.doubleValue) < std::numeric_limits<double>::epsilon();
    }

    void writeToStream (const ValueUnion& data, OutputStream& output) const
    {
        output.writeCompressedInt (9);
        output.writeByte (varMarker_Double);
        output.writeDouble (data.doubleValue);
    }
};

//==============================================================================
class var::VariantType_Bool   : public var::VariantType
{
public:
    VariantType_Bool() noexcept {}
    static const VariantType_Bool instance;

    int toInt (const ValueUnion& data) const noexcept       { return data.boolValue ? 1 : 0; };
    int64 toInt64 (const ValueUnion& data) const noexcept   { return data.boolValue ? 1 : 0; };
    double toDouble (const ValueUnion& data) const noexcept { return data.boolValue ? 1.0 : 0.0; }
    String toString (const ValueUnion& data) const          { return String::charToString (data.boolValue ? (juce_wchar) '1' : (juce_wchar) '0'); }
    bool toBool (const ValueUnion& data) const noexcept     { return data.boolValue; }
    bool isBool() const noexcept                            { return true; }

    bool equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) const noexcept
    {
        return otherType.toBool (otherData) == data.boolValue;
    }

    void writeToStream (const ValueUnion& data, OutputStream& output) const
    {
        output.writeCompressedInt (1);
        output.writeByte (data.boolValue ? (char) varMarker_BoolTrue : (char) varMarker_BoolFalse);
    }
};

//==============================================================================
class var::VariantType_String   : public var::VariantType
{
public:
    VariantType_String() noexcept {}
    static const VariantType_String instance;

    void cleanUp (ValueUnion& data) const noexcept                       { getString (data)-> ~String(); }
    void createCopy (ValueUnion& dest, const ValueUnion& source) const  LE_PATCH( noexcept )  { new (dest.stringValue) String (*getString (source)); }

    bool isString() const noexcept                          { return true; }
    int toInt (const ValueUnion& data) const noexcept       { return getString (data)->getIntValue(); };
    int64 toInt64 (const ValueUnion& data) const noexcept   { return getString (data)->getLargeIntValue(); };
    double toDouble (const ValueUnion& data) const noexcept { return getString (data)->getDoubleValue(); }
    String toString (const ValueUnion& data) const          { return *getString (data); }
    bool toBool (const ValueUnion& data) const noexcept     { return getString (data)->getIntValue() != 0
                                                                      || getString (data)->trim().equalsIgnoreCase ("true")
                                                                      || getString (data)->trim().equalsIgnoreCase ("yes"); }

    bool equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) const noexcept
    {
        return otherType.toString (otherData) == *getString (data);
    }

    void writeToStream (const ValueUnion& data, OutputStream& output) const
    {
        const String* const s = getString (data);
        const size_t len = s->getNumBytesAsUTF8() + 1;
        HeapBlock<char> temp (len);
        s->copyToUTF8 (temp, len);
        output.writeCompressedInt ((int) (len + 1));
        output.writeByte (varMarker_String);
        output.write (temp, len);
    }

private:
    static inline const String* getString (const ValueUnion& data) noexcept { return reinterpret_cast <const String*> (data.stringValue); }
    static inline String* getString (ValueUnion& data) noexcept             { return reinterpret_cast <String*> (data.stringValue); }
};

//==============================================================================
class var::VariantType_Object   : public var::VariantType
{
public:
    VariantType_Object() noexcept { LE_PATCH_UNREACHABLE_CODE }
    static const VariantType_Object instance;

    void cleanUp (ValueUnion& data) const noexcept                      { if (data.objectValue != nullptr) data.objectValue->decReferenceCount(); }

    void createCopy (ValueUnion& dest, const ValueUnion& source) const LE_PATCH( noexcept )
    {
        dest.objectValue = source.objectValue;
        if (dest.objectValue != nullptr)
            dest.objectValue->incReferenceCount();
    }

    String toString (const ValueUnion& data) const                            { return "Object 0x" + String::toHexString ((int) (pointer_sized_int) data.objectValue); }
    bool toBool (const ValueUnion& data) const noexcept                       { return data.objectValue != 0; }
    ReferenceCountedObject* toObject (const ValueUnion& data) const noexcept  { return data.objectValue; }
    bool isObject() const noexcept                                            { return true; }

    bool equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) const noexcept
    {
        return otherType.toObject (otherData) == data.objectValue;
    }

    void writeToStream (const ValueUnion&, OutputStream& output) const
    {
        jassertfalse; // Can't write an object to a stream!
        output.writeCompressedInt (0);
    }
};

//==============================================================================
class var::VariantType_Array   : public var::VariantType
{
public:
    VariantType_Array() noexcept { LE_PATCH_UNREACHABLE_CODE }
    static const VariantType_Array instance;

    void cleanUp (ValueUnion& data) const noexcept                      { delete data.arrayValue; }
    void createCopy (ValueUnion& dest, const ValueUnion& source) const LE_PATCH( noexcept ) { JUCE_ORIGINAL( dest.arrayValue = new Array<var> (*(source.arrayValue)); ) LE_PATCH_UNREACHABLE_CODE }

    String toString (const ValueUnion&) const                           { return "[Array]"; }
    bool isArray() const noexcept                                       { return true; }
    Array<var>* toArray (const ValueUnion& data) const noexcept         { return data.arrayValue; }

    bool equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) const noexcept
    {
        const Array<var>* const otherArray = otherType.toArray (otherData);
        return otherArray != nullptr && *otherArray == *(data.arrayValue);
    }

#ifndef LE_PATCHED_JUCE
    void writeToStream (const ValueUnion& data, OutputStream& output) const
    {
        MemoryOutputStream buffer (512);
        const int numItems = data.arrayValue->size();
        buffer.writeCompressedInt (numItems);

        for (int i = 0; i < numItems; ++i)
            data.arrayValue->getReference(i).writeToStream (buffer);

        output.writeCompressedInt (1 + (int) buffer.getDataSize());
        output.writeByte (varMarker_Array);
        output << buffer;
    }
#endif // LE_PATCHED_JUCE
};

//==============================================================================
class var::VariantType_Binary   : public var::VariantType
{
public:
    VariantType_Binary() noexcept { LE_PATCH_UNREACHABLE_CODE }

    static const VariantType_Binary instance;

    void cleanUp (ValueUnion& data) const noexcept                      { delete data.binaryValue; }
    void createCopy (ValueUnion& dest, const ValueUnion& source) const LE_PATCH( noexcept )  { dest.binaryValue = new MemoryBlock (*source.binaryValue); }

    String toString (const ValueUnion& data) const                      { return data.binaryValue->toBase64Encoding(); }
    bool isBinary() const noexcept                                      { return true; }
    MemoryBlock* toBinary (const ValueUnion& data) const noexcept       { return data.binaryValue; }

    bool equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) const noexcept
    {
        const MemoryBlock* const otherBlock = otherType.toBinary (otherData);
        return otherBlock != nullptr && *otherBlock == *data.binaryValue;
    }

#ifndef LE_PATCHED_JUCE
    void writeToStream (const ValueUnion& data, OutputStream& output) const
    {
        output.writeCompressedInt (1 + (int) data.binaryValue->getSize());
        output.writeByte (varMarker_Binary);
        output << *data.binaryValue;
    }
#endif // LE_PATCHED_JUCE
};

//==============================================================================
class var::VariantType_Method   : public var::VariantType
{
public:
    VariantType_Method() noexcept { LE_PATCH_UNREACHABLE_CODE }
    static const VariantType_Method instance;

    String toString (const ValueUnion&) const               { return "Method"; }
    bool toBool (const ValueUnion& data) const noexcept     { return data.methodValue != nullptr; }
    bool isMethod() const noexcept                          { return true; }

    bool equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) const noexcept
    {
        return otherType.isMethod() && otherData.methodValue == data.methodValue;
    }

    void writeToStream (const ValueUnion&, OutputStream& output) const
    {
        jassertfalse; // Can't write a method to a stream!
        output.writeCompressedInt (0);
    }
};

//==============================================================================
LE_PATCH( LE_WEAK_SYMBOL ) const var::VariantType_Void    var::VariantType_Void::instance;
LE_PATCH( LE_WEAK_SYMBOL ) const var::VariantType_Int     var::VariantType_Int::instance;
#ifndef LE_PATCHED_JUCE
LE_PATCH( LE_WEAK_SYMBOL ) const var::VariantType_Int64   var::VariantType_Int64::instance;
#endif // LE_PATCHED_JUCE
LE_PATCH( LE_WEAK_SYMBOL ) const var::VariantType_Bool    var::VariantType_Bool::instance;
LE_PATCH( LE_WEAK_SYMBOL ) const var::VariantType_Double  var::VariantType_Double::instance;
LE_PATCH( LE_WEAK_SYMBOL ) const var::VariantType_String  var::VariantType_String::instance;
#ifndef LE_PATCHED_JUCE
LE_PATCH( LE_WEAK_SYMBOL ) const var::VariantType_Object  var::VariantType_Object::instance;
LE_PATCH( LE_WEAK_SYMBOL ) const var::VariantType_Array   var::VariantType_Array::instance;
LE_PATCH( LE_WEAK_SYMBOL ) const var::VariantType_Binary  var::VariantType_Binary::instance;
LE_PATCH( LE_WEAK_SYMBOL ) const var::VariantType_Method  var::VariantType_Method::instance;
#endif // LE_PATCHED_JUCE


//==============================================================================
var::var() noexcept : type (&VariantType_Void::instance)
{
}

var::~var() noexcept
{
    type->cleanUp (value);
}

LE_PATCH( LE_WEAK_SYMBOL ) const var var::null;

//==============================================================================
var::var (const var& valueToCopy)  : type (valueToCopy.type)
{
    type->createCopy (value, valueToCopy.value);
}

var::var (const int v) noexcept       : type (&VariantType_Int::instance)    { value.intValue = v; }
JUCE_ORIGINAL( var::var (const int64 v) noexcept     : type (&VariantType_Int64::instance)  { value.int64Value = v; } )
var::var (const bool v) noexcept      : type (&VariantType_Bool::instance)   { value.boolValue = v; }
var::var (const double v) noexcept    : type (&VariantType_Double::instance) { value.doubleValue = v; }
JUCE_ORIGINAL( var::var (MethodFunction m) noexcept  : type (&VariantType_Method::instance) { value.methodValue = m; } )
JUCE_ORIGINAL( var::var (const Array<var>& v)        : type (&VariantType_Array::instance)  { value.arrayValue = new Array<var> (v); } )
var::var (const String& v)            : type (&VariantType_String::instance) { new (value.stringValue) String (v); }
var::var (const char* const v)        : type (&VariantType_String::instance) { new (value.stringValue) String (v); }
var::var (const wchar_t* const v)     : type (&VariantType_String::instance) { new (value.stringValue) String (v); }
JUCE_ORIGINAL( var::var (const void* v, size_t sz)   : type (&VariantType_Binary::instance) { value.binaryValue = new MemoryBlock (v, sz); } )
JUCE_ORIGINAL( var::var (const MemoryBlock& v)       : type (&VariantType_Binary::instance) { value.binaryValue = new MemoryBlock (v); } )

#ifndef LE_PATCHED_JUCE
var::var (ReferenceCountedObject* const object)  : type (&VariantType_Object::instance)
{
    value.objectValue = object;

    if (object != nullptr)
        object->incReferenceCount();
}
#endif // LE_PATCHED_JUCE

//==============================================================================
bool var::isVoid() const noexcept       { return type->isVoid(); }
bool var::isInt() const noexcept        { return type->isInt(); }
bool var::isInt64() const noexcept      { return type->isInt64(); }
bool var::isBool() const noexcept       { return type->isBool(); }
bool var::isDouble() const noexcept     { return type->isDouble(); }
bool var::isString() const noexcept     { return type->isString(); }
bool var::isObject() const noexcept     { return type->isObject(); }
bool var::isArray() const noexcept      { return type->isArray(); }
bool var::isBinaryData() const noexcept { return type->isBinary(); }
bool var::isMethod() const noexcept     { return type->isMethod(); }

var::operator int() const noexcept                      { return type->toInt (value); }
JUCE_ORIGINAL( var::operator int64() const noexcept                    { return type->toInt64 (value); } )
var::operator bool() const noexcept                     { return type->toBool (value); }
var::operator float() const noexcept                    { return (float) type->toDouble (value); }
var::operator double() const noexcept                   { return type->toDouble (value); }
String var::toString() const                            { return type->toString (value); }
var::operator String() const                            { return type->toString (value); }
JUCE_ORIGINAL( ReferenceCountedObject* var::getObject() const noexcept { return type->toObject (value); } )
JUCE_ORIGINAL( Array<var>* var::getArray() const noexcept              { return type->toArray (value); } )
MemoryBlock* var::getBinaryData() const noexcept        { return type->toBinary (value); }
JUCE_ORIGINAL( DynamicObject* var::getDynamicObject() const noexcept   { return dynamic_cast <DynamicObject*> (getObject()); } )

//==============================================================================
void var::swapWith (var& other) noexcept
{
    std::swap (type, other.type);
    std::swap (value, other.value);
}

var& var::operator= (const var& v)               { type->cleanUp (value); type = v.type; type->createCopy (value, v.value); return *this; }
var& var::operator= (const int v)                { type->cleanUp (value); type = &VariantType_Int::instance; value.intValue = v; return *this; }
JUCE_ORIGINAL( var& var::operator= (const int64 v)              { type->cleanUp (value); type = &VariantType_Int64::instance; value.int64Value = v; return *this; } )
var& var::operator= (const bool v)               { type->cleanUp (value); type = &VariantType_Bool::instance; value.boolValue = v; return *this; }
var& var::operator= (const double v)             { type->cleanUp (value); type = &VariantType_Double::instance; value.doubleValue = v; return *this; }
var& var::operator= (const char* const v)        { type->cleanUp (value); type = &VariantType_String::instance; new (value.stringValue) String (v); return *this; }
var& var::operator= (const wchar_t* const v)     { type->cleanUp (value); type = &VariantType_String::instance; new (value.stringValue) String (v); return *this; }
var& var::operator= (const String& v)            { type->cleanUp (value); type = &VariantType_String::instance; new (value.stringValue) String (v); return *this; }
#ifndef LE_PATCHED_JUCE
var& var::operator= (const Array<var>& v)        { var v2 (v); swapWith (v2); return *this; }
var& var::operator= (ReferenceCountedObject* v)  { var v2 (v); swapWith (v2); return *this; }
var& var::operator= (MethodFunction v)           { var v2 (v); swapWith (v2); return *this; }
#endif // LE_PATCHED_JUCE

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
var::var (var&& other) noexcept
    : type (other.type),
      value (other.value)
{
    other.type = &VariantType_Void::instance;
}

var& var::operator= (var&& other) noexcept
{
    swapWith (other);
    return *this;
}

var::var (String&& v)  : type (&VariantType_String::instance)
{
    new (value.stringValue) String (static_cast<String&&> (v));
}

#ifndef LE_PATCHED_JUCE
var::var (MemoryBlock&& v)  : type (&VariantType_Binary::instance)
{
    value.binaryValue = new MemoryBlock (static_cast<MemoryBlock&&> (v));
}
#endif // LE_PATCHED_JUCE

var& var::operator= (String&& v)
{
    type->cleanUp (value);
    type = &VariantType_String::instance;
    new (value.stringValue) String (static_cast<String&&> (v));
    return *this;
}
#endif

//==============================================================================
bool var::equals (const var& other) const noexcept
{
    return type->equals (value, other.value, *other.type);
}

bool var::equalsWithSameType (const var& other) const noexcept
{
    return type == other.type && equals (other);
}

bool operator== (const var& v1, const var& v2) noexcept     { return v1.equals (v2); }
bool operator!= (const var& v1, const var& v2) noexcept     { return ! v1.equals (v2); }
bool operator== (const var& v1, const String& v2)           { return v1.toString() == v2; }
bool operator!= (const var& v1, const String& v2)           { return v1.toString() != v2; }
bool operator== (const var& v1, const char* const v2)       { return v1.toString() == v2; }
bool operator!= (const var& v1, const char* const v2)       { return v1.toString() != v2; }


//==============================================================================
#ifndef LE_PATCHED_JUCE
var var::operator[] (const Identifier propertyName) const
{
    if (DynamicObject* const o = getDynamicObject())
        return o->getProperty (propertyName);

    return var::null;
}

var var::operator[] (const char* const propertyName) const
{
    return operator[] (Identifier (propertyName));
}

var var::getProperty (const Identifier propertyName, const var& defaultReturnValue) const
{
    if (DynamicObject* const o = getDynamicObject())
        return o->getProperties().getWithDefault (propertyName, defaultReturnValue);

    return defaultReturnValue;
}

var var::invoke (const Identifier method, const var* arguments, int numArguments) const
{
    if (DynamicObject* const o = getDynamicObject())
        return o->invokeMethod (method, arguments, numArguments);

    return var::null;
}

var var::invokeMethod (DynamicObject* const target, const var* const arguments, const int numArguments) const
{
    jassert (target != nullptr);

    if (isMethod())
        return (target->*(value.methodValue)) (arguments, numArguments);

    return var::null;
}

var var::call (const Identifier method) const
{
    return invoke (method, nullptr, 0);
}

var var::call (const Identifier method, const var& arg1) const
{
    return invoke (method, &arg1, 1);
}

var var::call (const Identifier method, const var& arg1, const var& arg2) const
{
    var args[] = { arg1, arg2 };
    return invoke (method, args, 2);
}

var var::call (const Identifier method, const var& arg1, const var& arg2, const var& arg3)
{
    var args[] = { arg1, arg2, arg3 };
    return invoke (method, args, 3);
}

var var::call (const Identifier method, const var& arg1, const var& arg2, const var& arg3, const var& arg4) const
{
    var args[] = { arg1, arg2, arg3, arg4 };
    return invoke (method, args, 4);
}

var var::call (const Identifier method, const var& arg1, const var& arg2, const var& arg3, const var& arg4, const var& arg5) const
{
    var args[] = { arg1, arg2, arg3, arg4, arg5 };
    return invoke (method, args, 5);
}

//==============================================================================
int var::size() const
{
    if (const Array<var>* const array = getArray())
        return array->size();

    return 0;
}

const var& var::operator[] (int arrayIndex) const
{
    const Array<var>* const array = getArray();

    // When using this method, the var must actually be an array, and the index
    // must be in-range!
    jassert (array != nullptr && isPositiveAndBelow (arrayIndex, array->size()));

    return array->getReference (arrayIndex);
}

var& var::operator[] (int arrayIndex)
{
    const Array<var>* const array = getArray();

    // When using this method, the var must actually be an array, and the index
    // must be in-range!
    jassert (array != nullptr && isPositiveAndBelow (arrayIndex, array->size()));

    return array->getReference (arrayIndex);
}

Array<var>* var::convertToArray()
{
    Array<var>* array = getArray();

    if (array == nullptr)
    {
        const Array<var> tempVar;
        var v (tempVar);
        array = v.value.arrayValue;

        if (! isVoid())
            array->add (*this);

        swapWith (v);
    }

    return array;
}
#endif // LE_PATCHED_JUCE

#ifndef LE_PATCHED_JUCE
void var::append (const var& n)
{
    convertToArray()->add (n);
}

void var::remove (const int index)
{
    if (Array<var>* const array = getArray())
        array->remove (index);
}

void var::insert (const int index, const var& n)
{
    convertToArray()->insert (index, n);
}

void var::resize (const int numArrayElementsWanted)
{
    convertToArray()->resize (numArrayElementsWanted);
}

int var::indexOf (const var& n) const
{
    if (const Array<var>* const array = getArray())
        return array->indexOf (n);

    return -1;
}
#endif // LE_PATCHED_JUCE

//==============================================================================
#ifndef LE_PATCHED_JUCE
void var::writeToStream (OutputStream& output) const
{
    type->writeToStream (value, output);
}
#endif // LE_PATCHED_JUCE

var var::readFromStream (InputStream& input)
{
    const int numBytes = input.readCompressedInt();

    if (numBytes > 0)
    {
        switch (input.readByte())
        {
            case varMarker_Int:         return var (input.readInt());
            case varMarker_Int64:       LE_PATCH_UNREACHABLE_CODE JUCE_ORIGINAL( return var (input.readInt64()); )
            case varMarker_BoolTrue:    LE_PATCH_UNREACHABLE_CODE JUCE_ORIGINAL( return var (true); )
            case varMarker_BoolFalse:   LE_PATCH_UNREACHABLE_CODE JUCE_ORIGINAL( return var (false); )
            case varMarker_Double:      return var (input.readDouble());
            case varMarker_String:
            {
                MemoryOutputStream mo;
                mo.writeFromInputStream (input, numBytes - 1);
                return var (mo.toUTF8());
            }

            case varMarker_Binary:
            #ifdef LE_PATCHED_JUCE
                LE_PATCH_UNREACHABLE_CODE
            #else
            {
                MemoryBlock mb ((size_t) numBytes - 1);

                if (numBytes > 1)
                {
                    const int numRead = input.read (mb.getData(), numBytes - 1);
                    mb.setSize ((size_t) numRead);
                }

                return var (mb);
            }
            #endif // LE_PATCHED_JUCE

            case varMarker_Array:
            #ifdef LE_PATCHED_JUCE
                LE_PATCH_UNREACHABLE_CODE
            #else
            {
                var v;
                Array<var>* const destArray = v.convertToArray();

                for (int i = input.readCompressedInt(); --i >= 0;)
                    destArray->add (readFromStream (input));

                return v;
            }
            #endif // LE_PATCHED_JUCE

            default:
                input.skipNextBytes (numBytes - 1); break;
        }
    }

    return var::null;
}
