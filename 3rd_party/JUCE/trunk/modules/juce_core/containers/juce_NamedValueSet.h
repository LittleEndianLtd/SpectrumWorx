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

#ifndef JUCE_NAMEDVALUESET_H_INCLUDED
#define JUCE_NAMEDVALUESET_H_INCLUDED


//==============================================================================
/** Holds a set of named var objects.

    This can be used as a basic structure to hold a set of var object, which can
    be retrieved by using their identifier.
*/
class JUCE_API  NamedValueSet
{
public:
    /** Creates an empty set. */
    NamedValueSet() noexcept;

    /** Creates a copy of another set. */
    NamedValueSet (const NamedValueSet& other);

    /** Replaces this set with a copy of another set. */
    NamedValueSet& operator= (const NamedValueSet& other);

   #if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
    NamedValueSet (NamedValueSet&& other) noexcept;
    NamedValueSet& operator= (NamedValueSet&& other) noexcept;
   #endif

    /** Destructor. */
    ~NamedValueSet();

    bool operator== (const NamedValueSet& other) const;
    bool operator!= (const NamedValueSet& other) const;

    //==============================================================================
    /** Returns the total number of values that the set contains. */
    int size() const noexcept;

    /** Returns the value of a named item.
        If the name isn't found, this will return a void variant.
        @see getProperty
    */
    const var& operator[] (const Identifier name) const LE_PATCH( noexcept );

    /** Tries to return the named value, but if no such value is found, this will
        instead return the supplied default value.
    */
    var getWithDefault (const Identifier name, const var& defaultReturnValue) const;

    /** Changes or adds a named value.
        @returns    true if a value was changed or added; false if the
                    value was already set the the value passed-in.
    */
    bool set (const Identifier name, const var& newValue);

   #if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
    /** Changes or adds a named value.
        @returns    true if a value was changed or added; false if the
                    value was already set the the value passed-in.
    */
    bool set (const Identifier name, var&& newValue);
   #endif

    /** Returns true if the set contains an item with the specified name. */
    bool contains (const Identifier name) const;

    /** Removes a value from the set.
        @returns    true if a value was removed; false if there was no value
                    with the name that was given.
    */
    bool remove (const Identifier name);

    /** Returns the name of the value at a given index.
        The index must be between 0 and size() - 1.
    */
    const Identifier getName (int index) const;

    /** Returns the value of the item at a given index.
        The index must be between 0 and size() - 1.
    */
    const var& getValueAt (int index) const;

    /** Removes all values. */
    void clear();

    //==============================================================================
    /** Returns a pointer to the var that holds a named value, or null if there is
        no value with this name.

        Do not use this method unless you really need access to the internal var object
        for some reason - for normal reading and writing always prefer operator[]() and set().
    */
    var* getVarPointer (const Identifier name) const noexcept;

    //==============================================================================
    /** Sets properties to the values of all of an XML element's attributes. */
#ifndef LE_PATCHED_JUCE
    void setFromXmlAttributes (const XmlElement& xml);

    /** Sets attributes in an XML element corresponding to each of this object's
        properties.
    */
    void copyToXmlAttributes (XmlElement& xml) const;
#endif // LE_PATCHED_JUCE

private:
    //==============================================================================
    class NamedValue
    {
    public:
        NamedValue() noexcept;
        NamedValue (const NamedValue&);
        NamedValue (const Identifier name, const var& value);
        NamedValue& operator= (const NamedValue&);
       #if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
        NamedValue (NamedValue&&) noexcept;
        NamedValue (const Identifier name, var&& value);
        NamedValue& operator= (NamedValue&&) noexcept;
       #endif
        bool operator== (const NamedValue& other) const noexcept;

        LinkedListPointer<NamedValue> nextListItem;
        Identifier name;
        var value;

    private:
        JUCE_LEAK_DETECTOR (NamedValue)
    };

    friend class LinkedListPointer<NamedValue>;
    LinkedListPointer<NamedValue> values;

    friend class JSONFormatter;
};


#endif   // JUCE_NAMEDVALUESET_H_INCLUDED
