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

class SharedValueSourceUpdater  : public ReferenceCountedObject,
                                  private AsyncUpdater
{
public:
    SharedValueSourceUpdater() : LE_PATCH( masterReference( *this ), ) sourcesBeingIterated (nullptr) {}
    ~SharedValueSourceUpdater()  { masterReference.clear(); }

    void update (Value::ValueSource* const source)
    {
        sourcesNeedingAnUpdate.add (source);

        if (sourcesBeingIterated == nullptr)
            triggerAsyncUpdate();
    }

    void valueDeleted (Value::ValueSource* const source)
    {
        sourcesNeedingAnUpdate.removeValue (source);

        if (sourcesBeingIterated != nullptr)
            sourcesBeingIterated->removeValue (source);
    }

    WeakReference<SharedValueSourceUpdater>::Master masterReference;

private:
    typedef SortedSet<Value::ValueSource*> SourceSet;
    SourceSet sourcesNeedingAnUpdate;
    SourceSet* sourcesBeingIterated;

    void handleAsyncUpdate() override
    {
        const ReferenceCountedObjectPtr<SharedValueSourceUpdater> localRef (this);

        {
            const ScopedValueSetter<SourceSet*> inside (sourcesBeingIterated, nullptr, nullptr);
            int maxLoops = 10;

            while (sourcesNeedingAnUpdate.size() > 0)
            {
                if (--maxLoops == 0)
                {
                    triggerAsyncUpdate();
                    break;
                }

                SourceSet sources;
                sources.swapWith (sourcesNeedingAnUpdate);
                sourcesBeingIterated = &sources;

                for (int i = sources.size(); --i >= 0;)
                    if (i < sources.size())
                        sources.getUnchecked(i)->sendChangeMessage (true);
            }
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SharedValueSourceUpdater)
};

LE_PATCH( LE_WEAK_SYMBOL ) JUCE_ORIGINAL( static ) WeakReference<SharedValueSourceUpdater> sharedUpdater;

#ifndef LE_PATCHED_JUCE
Value::ValueSource::ValueSource()
{
}
#endif // LE_PATCHED_JUCE

Value::ValueSource::~ValueSource() LE_PATCH( noexcept )
{
    if (asyncUpdater != nullptr)
        static_cast <SharedValueSourceUpdater*> (asyncUpdater.get())->valueDeleted (this);
}

void Value::ValueSource::sendChangeMessage (const bool synchronous)
{
    const int numListeners = valuesWithListeners.size();

    if (numListeners > 0)
    {
        if (synchronous)
        {
            const ReferenceCountedObjectPtr<ValueSource> localRef (this);
            asyncUpdater = nullptr;

            for (int i = numListeners; --i >= 0;)
                if (Value* const v = valuesWithListeners[i])
                    v->callListeners();
        }
        else
        {
            SharedValueSourceUpdater* updater = static_cast <SharedValueSourceUpdater*> (asyncUpdater.get());

            if (updater == nullptr)
            {
                if (sharedUpdater == nullptr)
                {
                    asyncUpdater = updater = new SharedValueSourceUpdater();
                    sharedUpdater = updater;
                }
                else
                {
                    asyncUpdater = updater = sharedUpdater.get();
                }
            }

            updater->update (this);
        }
    }
}

//==============================================================================
class SimpleValueSource  : public Value::ValueSource
{
public:
    SimpleValueSource()
    {
    }

    SimpleValueSource (const var& initialValue)
        : value (initialValue)
    {
    }

    var LE_PATCH( const & ) getValue() const LE_PATCH( noexcept )
    {
        return value;
    }

    void setValue (const var& newValue) LE_PATCH( noexcept )
    {
        if (! newValue.equalsWithSameType (value))
        {
            value = newValue;
            sendChangeMessage (false);
        }
    }

private:
    var value;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleValueSource)
};


//==============================================================================
Value::Value()
    : value (new SimpleValueSource())
{
}

Value::Value (ValueSource* const v)
    : value (v)
{
    jassert (v != nullptr);
}

Value::Value (const var& initialValue)
    : value (new SimpleValueSource (initialValue))
{
}

Value::Value (const Value& other)
    : value (other.value)
{
}

Value& Value::operator= (const Value& other)
{
    value = other.value;
    return *this;
}

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
Value::Value (Value&& other) noexcept
    : value (static_cast <ReferenceCountedObjectPtr <ValueSource>&&> (other.value))
{
}

Value& Value::operator= (Value&& other) noexcept
{
    value = static_cast <ReferenceCountedObjectPtr <ValueSource>&&> (other.value);
    return *this;
}
#endif

Value::~Value()
{
    if (listeners.size() > 0)
        value->valuesWithListeners.removeValue (this);
}

//==============================================================================
var LE_PATCH( const & ) Value::getValue() const LE_PATCH( noexcept )
{
    return value->getValue();
}

Value::operator var LE_PATCH( const & ) () const LE_PATCH( noexcept )
{
    return getValue();
}

void Value::setValue (const var& newValue)
{
    value->setValue (newValue);
}

String Value::toString() const
{
    return value->getValue().toString();
}

Value& Value::operator= (const var& newValue)
{
    value->setValue (newValue);
    return *this;
}

void Value::referTo (const Value& valueToReferTo)
{
    if (valueToReferTo.value != value)
    {
        if (listeners.size() > 0)
        {
            value->valuesWithListeners.removeValue (this);
            valueToReferTo.value->valuesWithListeners.add (this);
        }

        value = valueToReferTo.value;
        callListeners();
    }
}

bool Value::refersToSameSourceAs (const Value& other) const
{
    return value == other.value;
}

bool Value::operator== (const Value& other) const
{
    return value == other.value || value->getValue() == other.getValue();
}

bool Value::operator!= (const Value& other) const
{
    return value != other.value && value->getValue() != other.getValue();
}

//==============================================================================
void Value::addListener (ValueListener* const listener)
{
    if (listener != nullptr)
    {
        if (listeners.size() == 0)
            value->valuesWithListeners.add (this);

        listeners.add (listener);
    }
}

void Value::removeListener (ValueListener* const listener)
{
    listeners.remove (listener);

    if (listeners.size() == 0)
        value->valuesWithListeners.removeValue (this);
}

void Value::callListeners()
{
    if (listeners.size() > 0)
    {
        Value v (*this); // (create a copy in case this gets deleted by a callback)
        listeners.call (&ValueListener::valueChanged, v);
    }
}

#ifndef LE_PATCHED_JUCE
OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const Value& value)
{
    return stream << value.toString();
}
#endif // LE_PATCHED_JUCE
