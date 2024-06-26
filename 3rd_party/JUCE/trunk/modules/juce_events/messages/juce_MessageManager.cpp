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

class MessageManager::QuitMessage   : public MessageManager::MessageBase
{
public:
    QuitMessage() {}

    void messageCallback() override
    {
        if (MessageManager* const mm = MessageManager::instance)
            mm->quitMessageReceived = true;
    }

    JUCE_DECLARE_NON_COPYABLE (QuitMessage)
};

//==============================================================================
MessageManager::MessageManager() noexcept
  : quitMessagePosted (false),
    quitMessageReceived (false),
    messageThreadId (Thread::getCurrentThreadId()),
    threadWithLock (0)
{
    if (JUCEApplicationBase::isStandaloneApp())
        Thread::setCurrentThreadName ("Juce Message Thread");
}

MessageManager::~MessageManager() noexcept
{
    JUCE_ORIGINAL( broadcaster = nullptr; )

    doPlatformSpecificShutdown();

    jassert (instance == this);
#ifndef LE_PATCHED_JUCE
    instance = nullptr;  // do this last in case this instance is still needed by doPlatformSpecificShutdown()
#endif // LE_PATCHED_JUCE
}

MessageManager * MessageManager::instance = nullptr;

#ifdef LE_PATCHED_JUCE
union MessageManagerSingletonStorage
{
    double aligner;
    char   storage[ sizeof( MessageManager ) ];
};
static MessageManagerSingletonStorage messageManagerSingletonStorage;
void MessageManager::createSingleton()
{
    LE_PATCH_ASSUME( instance == nullptr );
    instance = new ( & messageManagerSingletonStorage ) MessageManager;
    LE_PATCH_ASSUME( instance == reinterpret_cast<MessageManager const *>( &messageManagerSingletonStorage ) );
    MessageManager::doPlatformSpecificInitialisation();
}
void MessageManager::destroySingleton()
{
    LE_PATCH_ASSUME( instance == reinterpret_cast<MessageManager const *>( &messageManagerSingletonStorage ) );
    MessageManager::instance->~MessageManager();
    MessageManager::instance = nullptr;
}
#endif // LE_PATCHED_JUCE

MessageManager* MessageManager::getInstance()
{
#ifdef LE_PATCHED_JUCE
    LE_PATCH_ASSUME( instance == reinterpret_cast<MessageManager const *>( &messageManagerSingletonStorage ) );
    return instance;
#else
    if (instance == nullptr)
    {
        instance = new MessageManager();
        doPlatformSpecificInitialisation();
    }

    return instance;
#endif // LE_PATCHED_JUCE
}

JUCE_ORIGINAL( inline ) MessageManager* MessageManager::getInstanceWithoutCreating() noexcept
{
    LE_PATCH_ASSUME( instance == reinterpret_cast<MessageManager const *>( &messageManagerSingletonStorage ) );
    return instance;
}

void MessageManager::deleteInstance()
{
#ifdef LE_PATCHED_JUCE
    // You need to ensure/force Cocoa cleanup before the static object cleanup.
#else
    deleteAndZero (instance);
#endif // LE_PATCHED_JUCE
}

//==============================================================================
void MessageManager::MessageBase::post() LE_PATCH( noexcept )
{
    MessageManager* const mm = MessageManager::instance;

    if (mm == nullptr || mm->quitMessagePosted || ! postMessageToSystemQueue (this))
        Ptr deleter (this); // (this will delete messages that were just created with a 0 ref count)
}

//==============================================================================
#if JUCE_MODAL_LOOPS_PERMITTED && ! (JUCE_MAC || JUCE_IOS)
void MessageManager::runDispatchLoop()
{
    runDispatchLoopUntil (-1);
}

void MessageManager::stopDispatchLoop()
{
    (new QuitMessage())->post();
    quitMessagePosted = true;
}

bool MessageManager::runDispatchLoopUntil (int millisecondsToRunFor)
{
    jassert (isThisTheMessageThread()); // must only be called by the message thread

    const int64 endTime = Time::currentTimeMillis() + millisecondsToRunFor;

    while (! quitMessageReceived)
    {
        JUCE_TRY
        {
            if (! dispatchNextMessageOnSystemQueue (millisecondsToRunFor >= 0))
                Thread::sleep (1);
        }
        JUCE_CATCH_EXCEPTION

        if (millisecondsToRunFor >= 0 && Time::currentTimeMillis() >= endTime)
            break;
    }

    return ! quitMessageReceived;
}

#endif

//==============================================================================
class AsyncFunctionCallback   : public MessageManager::MessageBase
{
public:
    AsyncFunctionCallback (MessageCallbackFunction* const f, void* const param)
        : result (nullptr), func (f), parameter (param)
    {}

    void messageCallback() override
    {
        result = (*func) (parameter);
        finished.signal();
    }

    WaitableEvent finished;
    void* volatile result;

private:
    MessageCallbackFunction* const func;
    void* const parameter;

    JUCE_DECLARE_NON_COPYABLE (AsyncFunctionCallback)
};

void* MessageManager::callFunctionOnMessageThread (MessageCallbackFunction* const func, void* const parameter)
{
    if (isThisTheMessageThread())
        return func (parameter);

    // If this thread has the message manager locked, then this will deadlock!
    jassert (! currentThreadHasLockedMessageManager());

    const ReferenceCountedObjectPtr<AsyncFunctionCallback> message (new AsyncFunctionCallback (func, parameter));
    message->post();
    message->finished.wait();
    return message->result;
}

//==============================================================================
void MessageManager::deliverBroadcastMessage (const String& value)
{
#ifdef LE_PATCHED_JUCE
    LE_PATCH_UNREACHABLE_CODE
#else
    if (broadcaster != nullptr)
        broadcaster->sendActionMessage (value);
#endif // LE_PATCHED_JUCE
}

void MessageManager::registerBroadcastListener (ActionListener* const listener)
{
#ifdef LE_PATCHED_JUCE
    (void)listener;
    LE_PATCH_UNREACHABLE_CODE
#else
    if (broadcaster == nullptr)
        broadcaster = new ActionBroadcaster();

    broadcaster->addActionListener (listener);
#endif // LE_PATCHED_JUCE
}

void MessageManager::deregisterBroadcastListener (ActionListener* const listener)
{
#ifdef LE_PATCHED_JUCE
    (void)listener;
    LE_PATCH_UNREACHABLE_CODE
#else
    if (broadcaster != nullptr)
        broadcaster->removeActionListener (listener);
#endif // LE_PATCHED_JUCE
}

//==============================================================================
bool MessageManager::isThisTheMessageThread() const noexcept
{
    return Thread::getCurrentThreadId() == messageThreadId;
}

void MessageManager::setCurrentThreadAsMessageThread()
{
    const Thread::ThreadID thisThread = Thread::getCurrentThreadId();

    if (messageThreadId != thisThread)
    {
        messageThreadId = thisThread;

        // This is needed on windows to make sure the message window is created by this thread
        doPlatformSpecificShutdown();
        doPlatformSpecificInitialisation();
    }
}

bool MessageManager::currentThreadHasLockedMessageManager() const noexcept
{
    const Thread::ThreadID thisThread = Thread::getCurrentThreadId();
    return thisThread == messageThreadId || thisThread == threadWithLock;
}

//==============================================================================
//==============================================================================
/*  The only safe way to lock the message thread while another thread does
    some work is by posting a special message, whose purpose is to tie up the event
    loop until the other thread has finished its business.

    Any other approach can get horribly deadlocked if the OS uses its own hidden locks which
    get locked before making an event callback, because if the same OS lock gets indirectly
    accessed from another thread inside a MM lock, you're screwed. (this is exactly what happens
    in Cocoa).
*/
class MessageManagerLock::BlockingMessage   : public MessageManager::MessageBase
{
public:
    BlockingMessage() noexcept {}

    void messageCallback() override
    {
        lockedEvent.signal();
        releaseEvent.wait();
    }

    WaitableEvent lockedEvent, releaseEvent;

    JUCE_DECLARE_NON_COPYABLE (BlockingMessage)
};

//==============================================================================
MessageManagerLock::MessageManagerLock (Thread* const threadToCheck)
    : blockingMessage(), locked (attemptLock (threadToCheck, nullptr))
{
}

#ifndef LE_PATCHED_JUCE
MessageManagerLock::MessageManagerLock (ThreadPoolJob* const jobToCheckForExitSignal)
    : blockingMessage(), locked (attemptLock (nullptr, jobToCheckForExitSignal))
{
}
#endif // LE_PATCHED_JUCE

bool MessageManagerLock::attemptLock (Thread* const threadToCheck, ThreadPoolJob* const job)
{
    LE_PATCH_ASSUME( job == nullptr );
    MessageManager* const mm = MessageManager::instance;

    if (mm == nullptr)
        return false;

    if (mm->currentThreadHasLockedMessageManager())
        return true;

    if (threadToCheck == nullptr && job == nullptr)
    {
        mm->lockingLock.enter();
    }
    else
    {
        while (! mm->lockingLock.tryEnter())
        {
            if ((threadToCheck != nullptr && threadToCheck->threadShouldExit())
                  || (job != nullptr JUCE_ORIGINAL( && job->shouldExit() )))
                return false;

            Thread::yield();
        }
    }

    blockingMessage = new BlockingMessage();
    blockingMessage->post();

    while (! blockingMessage->lockedEvent.wait (20))
    {
        if ((threadToCheck != nullptr && threadToCheck->threadShouldExit())
              || (job != nullptr JUCE_ORIGINAL( && job->shouldExit() )))
        {
            blockingMessage->releaseEvent.signal();
            blockingMessage = nullptr;
            mm->lockingLock.exit();
            return false;
        }
    }

    jassert (mm->threadWithLock == 0);

    mm->threadWithLock = Thread::getCurrentThreadId();
    return true;
}

MessageManagerLock::~MessageManagerLock() noexcept
{
    if (blockingMessage != nullptr)
    {
        MessageManager* const mm = MessageManager::instance;

        jassert (mm == nullptr || mm->currentThreadHasLockedMessageManager());

        blockingMessage->releaseEvent.signal();
        blockingMessage = nullptr;

        if (mm != nullptr)
        {
            mm->threadWithLock = 0;
            mm->lockingLock.exit();
        }
    }
}

//==============================================================================
JUCE_API void JUCE_CALLTYPE initialiseJuce_GUI();
JUCE_API void JUCE_CALLTYPE initialiseJuce_GUI()
{
    JUCE_AUTORELEASEPOOL
    {
    #ifdef LE_PATCHED_JUCE
        MessageManager::createSingleton();
        //...mrmlj...includes...Desktop::create();
    #endif // LE_PATCHED_JUCE
        MessageManager::getInstance();
    }
}

JUCE_API void JUCE_CALLTYPE shutdownJuce_GUI();
JUCE_API void JUCE_CALLTYPE shutdownJuce_GUI()
{
    JUCE_AUTORELEASEPOOL
    {
        DeletedAtShutdown::deleteAll();
        MessageManager::deleteInstance();
    #ifdef LE_PATCHED_JUCE
        Timer::destroyThread();
        //...mrmlj...includes...Desktop::destroy();
        //   MessageManager::doPlatformSpecificShutdown() has to be called
        // before static object destructors otherwise dubious crashes occur in
        // Cocoa object deallocation functions.
        //                                    (25.11.2011.) (Domagoj Saric)
        //...mrmlj...required by Desktop::destroy()...MessageManager::destroySingleton();
    #endif // LE_PATCHED_JUCE
    }
}
