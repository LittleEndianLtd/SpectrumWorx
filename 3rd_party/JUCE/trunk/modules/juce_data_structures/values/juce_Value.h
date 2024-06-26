/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_VALUE_H_INCLUDED
#define JUCE_VALUE_H_INCLUDED


//==============================================================================
/**
    Represents a shared variant value.

    A Value object contains a reference to a var object, and can get and set its value.
    Listeners can be attached to be told when the value is changed.

    The Value class is a wrapper around a shared, reference-counted underlying data
    object - this means that multiple Value objects can all refer to the same piece of
    data, allowing all of them to be notified when any of them changes it.

    When you create a Value with its default constructor, it acts as a wrapper around a
    simple var object, but by creating a Value that refers to a custom subclass of ValueSource,
    you can map the Value onto any kind of underlying data.
*/
class JUCE_API  Value
{
public:
    //==============================================================================
    /** Creates an empty Value, containing a void var. */
    Value();

    /** Creates a Value that refers to the same value as another one.

        Note that this doesn't make a copy of the other value - both this and the other
        Value will share the same underlying value, so that when either one alters it, both
        will see it change.
    */
    Value (const Value& other);

    /** Creates a Value that is set to the specified value. */
    explicit Value (const var& initialValue);

   #if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
    Value (Value&&) noexcept;
    Value& operator= (Value&&) noexcept;
   #endif

    /** Destructor. */
    ~Value();

    //==============================================================================
    /** Returns the current value. */
    var LE_PATCH( const & ) getValue() const LE_PATCH( noexcept );

    /** Returns the current value. */
    operator var LE_PATCH( const & ) () const LE_PATCH( noexcept );

    /** Returns the value as a string.

        This is alternative to writing things like "myValue.getValue().toString()".
    */
    String toString() const;

    /** Sets the current value.

        You can also use operator= to set the value.

        If there are any listeners registered, they will be notified of the
        change asynchronously.
    */
    void setValue (const var& newValue);

    /** Sets the current value.

        This is the same as calling setValue().

        If there are any listeners registered, they will be notified of the
        change asynchronously.
    */
    Value& operator= (const var& newValue);

    /** Makes this object refer to the same underlying ValueSource as another one.

        Once this object has been connected to another one, changing either one
        will update the other.

        Existing listeners will still be registered after you call this method, and
        they'll continue to receive messages when the new value changes.
    */
    void referTo (const Value& valueToReferTo);

    /** Returns true if this value and the other one are references to the same value.
    */
    bool refersToSameSourceAs (const Value& other) const;

    /** Compares two values.
        This is a compare-by-value comparison, so is effectively the same as
        saying (this->getValue() == other.getValue()).
    */
    bool operator== (const Value& other) const;

    /** Compares two values.
        This is a compare-by-value comparison, so is effectively the same as
        saying (this->getValue() != other.getValue()).
    */
    bool operator!= (const Value& other) const;

    //==============================================================================
    /** Receives callbacks when a Value object changes.
        @see Value::addListener
    */
    class JUCE_API LE_PATCH_NOVTABLE Listener
    {
    JUCE_ORIGINAL( public: ) LE_PATCH( protected: )
        JUCE_ORIGINAL( Listener()          {} )
        JUCE_ORIGINAL( virtual ) ~Listener() {} LE_PATCH( public: )

        /** Called when a Value object is changed.

            Note that the Value object passed as a parameter may not be exactly the same
            object that you registered the listener with - it might be a copy that refers
            to the same underlying ValueSource. To find out, you can call Value::refersToSameSourceAs().
        */
        virtual void valueChanged (Value& value) LE_PATCH( noexcept ) = 0;
    };

    /** Adds a listener to receive callbacks when the value changes.

        The listener is added to this specific Value object, and not to the shared
        object that it refers to. When this object is deleted, all the listeners will
        be lost, even if other references to the same Value still exist. So when you're
        adding a listener, make sure that you add it to a ValueTree instance that will last
        for as long as you need the listener. In general, you'd never want to add a listener
        to a local stack-based ValueTree, but more likely to one that's a member variable.

        @see removeListener
    */
    void addListener (Listener* listener);

    /** Removes a listener that was previously added with addListener(). */
    void removeListener (Listener* listener);


    //==============================================================================
    /**
        Used internally by the Value class as the base class for its shared value objects.

        The Value class is essentially a reference-counted pointer to a shared instance
        of a ValueSource object. If you're feeling adventurous, you can create your own custom
        ValueSource classes to allow Value objects to represent your own custom data items.
    */
    class JUCE_API  ValueSource   : public SingleThreadedReferenceCountedObject
    {
    public:
        ValueSource() LE_PATCH( {} ) JUCE_ORIGINAL( ; )
        virtual ~ValueSource() LE_PATCH( noexcept );

        /** Returns the current value of this object. */
        virtual var LE_PATCH( const & ) getValue() const LE_PATCH( noexcept ) = 0;

        /** Changes the current value.
            This must also trigger a change message if the value actually changes.
        */
        virtual void setValue (const var& newValue) LE_PATCH( noexcept ) = 0;

        /** Delivers a change message to all the listeners that are registered with
            this value.

            If dispatchSynchronously is true, the method will call all the listeners
            before returning; otherwise it'll dispatch a message and make the call later.
        */
        void sendChangeMessage (bool dispatchSynchronously);

    protected:
        //==============================================================================
        friend class Value;
        SortedSet <Value*> valuesWithListeners;
        ReferenceCountedObjectPtr<ReferenceCountedObject> asyncUpdater;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValueSource)
    };


    //==============================================================================
    /** Creates a Value object that uses this valueSource object as its underlying data. */
    explicit Value (ValueSource* valueSource);

    /** Returns the ValueSource that this value is referring to. */
    ValueSource& getValueSource() noexcept          { return *value; }


private:
    //==============================================================================
    friend class ValueSource;
    ReferenceCountedObjectPtr <ValueSource> value;
    ListenerList <Listener> listeners;

    void callListeners();

    // This is disallowed to avoid confusion about whether it should
    // do a by-value or by-reference copy.
    Value& operator= (const Value&);
};

/** Writes a Value to an OutputStream as a UTF8 string. */
OutputStream& JUCE_CALLTYPE operator<< (OutputStream&, const Value&);

/** This typedef is just for compatibility with old code - newer code should use the Value::Listener class directly. */
typedef Value::Listener ValueListener;

#endif   // JUCE_VALUE_H_INCLUDED
