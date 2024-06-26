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

#ifndef JUCE_SINGLETON_H_INCLUDED
#define JUCE_SINGLETON_H_INCLUDED


//==============================================================================
/**
    Macro to declare member variables and methods for a singleton class.

    To use this, add the line juce_DeclareSingleton (MyClass, doNotRecreateAfterDeletion)
    to the class's definition.

    Then put a macro juce_ImplementSingleton (MyClass) along with the class's
    implementation code.

    It's also a very good idea to also add the call clearSingletonInstance() in your class's
    destructor, in case it is deleted by other means than deleteInstance()

    Clients can then call the static method MyClass::getInstance() to get a pointer
    to the singleton, or MyClass::getInstanceWithoutCreating() which will return 0 if
    no instance currently exists.

    e.g. @code

        class MySingleton
        {
        public:
            MySingleton()
            {
            }

            ~MySingleton()
            {
                // this ensures that no dangling pointers are left when the
                // singleton is deleted.
                clearSingletonInstance();
            }

            juce_DeclareSingleton (MySingleton, false)
        };

        juce_ImplementSingleton (MySingleton)


        // example of usage:
        MySingleton* m = MySingleton::getInstance(); // creates the singleton if there isn't already one.

        ...

        MySingleton::deleteInstance(); // safely deletes the singleton (if it's been created).

    @endcode

    If doNotRecreateAfterDeletion = true, it won't allow the object to be created more
    than once during the process's lifetime - i.e. after you've created and deleted the
    object, getInstance() will refuse to create another one. This can be useful to stop
    objects being accidentally re-created during your app's shutdown code.

    If you know that your object will only be created and deleted by a single thread, you
    can use the slightly more efficient juce_DeclareSingleton_SingleThreaded() macro instead
    of this one.

    @see juce_ImplementSingleton, juce_DeclareSingleton_SingleThreaded
*/
#define juce_DeclareSingleton(classname, doNotRecreateAfterDeletion) \
\
    static classname* _singletonInstance;  \
    static juce::CriticalSection _singletonLock; \
\
    static classname* JUCE_CALLTYPE getInstance() \
    { \
        if (_singletonInstance == nullptr) \
        {\
            const juce::ScopedLock sl (_singletonLock); \
\
            if (_singletonInstance == nullptr) \
            { \
                static bool alreadyInside = false; \
                static bool createdOnceAlready = false; \
\
                const bool problem = alreadyInside || ((doNotRecreateAfterDeletion) && createdOnceAlready); \
                jassert (! problem); \
                if (! problem) \
                { \
                    createdOnceAlready = true; \
                    alreadyInside = true; \
                    classname* newObject = new classname();  /* (use a stack variable to avoid setting the newObject value before the class has finished its constructor) */ \
                    alreadyInside = false; \
\
                    _singletonInstance = newObject; \
                } \
            } \
        } \
\
        return _singletonInstance; \
    } \
\
    static inline classname* JUCE_CALLTYPE getInstanceWithoutCreating() noexcept\
    { \
        return _singletonInstance; \
    } \
\
    static void JUCE_CALLTYPE deleteInstance() \
    { \
        const juce::ScopedLock sl (_singletonLock); \
        if (_singletonInstance != nullptr) \
        { \
            classname* const old = _singletonInstance; \
            _singletonInstance = nullptr; \
            delete old; \
        } \
    } \
\
    void clearSingletonInstance() noexcept\
    { \
        if (_singletonInstance == this) \
            _singletonInstance = nullptr; \
    }


//==============================================================================
/** This is a counterpart to the juce_DeclareSingleton macro.

    After adding the juce_DeclareSingleton to the class definition, this macro has
    to be used in the cpp file.
*/
#ifdef LE_PATCHED_JUCE
#define juce_ImplementSingleton(classname) \
    LE_WEAK_SYMBOL classname* classname::_singletonInstance = nullptr; \
    LE_WEAK_SYMBOL juce::CriticalSection classname::_singletonLock;
#else
#define juce_ImplementSingleton(classname) \
\
    classname* classname::_singletonInstance = nullptr; \
    juce::CriticalSection classname::_singletonLock;
#endif // LE_PATCHED_JUCE

//==============================================================================
/**
    Macro to declare member variables and methods for a singleton class.

    This is exactly the same as juce_DeclareSingleton, but doesn't use a critical
    section to make access to it thread-safe. If you know that your object will
    only ever be created or deleted by a single thread, then this is a
    more efficient version to use.

    If doNotRecreateAfterDeletion = true, it won't allow the object to be created more
    than once during the process's lifetime - i.e. after you've created and deleted the
    object, getInstance() will refuse to create another one. This can be useful to stop
    objects being accidentally re-created during your app's shutdown code.

    See the documentation for juce_DeclareSingleton for more information about
    how to use it, the only difference being that you have to use
    juce_ImplementSingleton_SingleThreaded instead of juce_ImplementSingleton.

    @see juce_ImplementSingleton_SingleThreaded, juce_DeclareSingleton, juce_DeclareSingleton_SingleThreaded_Minimal
*/
#define juce_DeclareSingleton_SingleThreaded(classname, doNotRecreateAfterDeletion) \
\
    static classname* _singletonInstance;  \
\
    static classname* getInstance() \
    { \
        if (_singletonInstance == nullptr) \
        { \
            static bool alreadyInside = false; \
            static bool createdOnceAlready = false; \
\
            const bool problem = alreadyInside || ((doNotRecreateAfterDeletion) && createdOnceAlready); \
            jassert (! problem); \
            if (! problem) \
            { \
                createdOnceAlready = true; \
                alreadyInside = true; \
                classname* newObject = new classname();  /* (use a stack variable to avoid setting the newObject value before the class has finished its constructor) */ \
                alreadyInside = false; \
\
                _singletonInstance = newObject; \
            } \
        } \
\
        return _singletonInstance; \
    } \
\
    static inline classname* getInstanceWithoutCreating() noexcept\
    { \
        return _singletonInstance; \
    } \
\
    static void deleteInstance() \
    { \
        if (_singletonInstance != nullptr) \
        { \
            classname* const old = _singletonInstance; \
            _singletonInstance = nullptr; \
            delete old; \
        } \
    } \
\
    void clearSingletonInstance() noexcept\
    { \
        if (_singletonInstance == this) \
            _singletonInstance = nullptr; \
    }

//==============================================================================
/**
    Macro to declare member variables and methods for a singleton class.

    This is like juce_DeclareSingleton_SingleThreaded, but doesn't do any checking
    for recursion or repeated instantiation. It's intended for use as a lightweight
    version of a singleton, where you're using it in very straightforward
    circumstances and don't need the extra checking.

    Juce use the normal juce_ImplementSingleton_SingleThreaded as the counterpart
    to this declaration, as you would with juce_DeclareSingleton_SingleThreaded.

    See the documentation for juce_DeclareSingleton for more information about
    how to use it, the only difference being that you have to use
    juce_ImplementSingleton_SingleThreaded instead of juce_ImplementSingleton.

    @see juce_ImplementSingleton_SingleThreaded, juce_DeclareSingleton
*/
#define juce_DeclareSingleton_SingleThreaded_Minimal(classname) \
\
    static classname* _singletonInstance;  \
\
    static classname* getInstance() \
    { \
        if (_singletonInstance == nullptr) \
            _singletonInstance = new classname(); \
\
        return _singletonInstance; \
    } \
\
    static inline classname* getInstanceWithoutCreating() noexcept\
    { \
        return _singletonInstance; \
    } \
\
    static void deleteInstance() \
    { \
        if (_singletonInstance != nullptr) \
        { \
            classname* const old = _singletonInstance; \
            _singletonInstance = nullptr; \
            delete old; \
        } \
    } \
\
    void clearSingletonInstance() noexcept\
    { \
        if (_singletonInstance == this) \
            _singletonInstance = nullptr; \
    }


//==============================================================================
/** This is a counterpart to the juce_DeclareSingleton_SingleThreaded macro.

    After adding juce_DeclareSingleton_SingleThreaded or juce_DeclareSingleton_SingleThreaded_Minimal
    to the class definition, this macro has to be used somewhere in the cpp file.
*/
#ifdef LE_PATCHED_JUCE
    #define juce_ImplementSingleton_SingleThreaded(classname) \
        LE_WEAK_SYMBOL classname* classname::_singletonInstance = nullptr;
#else
#define juce_ImplementSingleton_SingleThreaded(classname) \
\
    classname* classname::_singletonInstance = nullptr;
#endif // LE_PATCHED_JUCE



#endif   // JUCE_SINGLETON_H_INCLUDED
