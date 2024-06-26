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

InterprocessConnection::InterprocessConnection (const bool callbacksOnMessageThread,
                                                const uint32 magicMessageHeaderNumber)
    : Thread ("Juce IPC connection"),
      callbackConnectionState (false),
      useMessageThread (callbacksOnMessageThread),
      magicMessageHeader (magicMessageHeaderNumber),
      pipeReceiveMessageTimeout (-1)
#ifdef LE_PATCHED_JUCE
      ,masterReference( *this )
#endif // LE_PATCHED_JUCE
{
}

InterprocessConnection::~InterprocessConnection()
{
    callbackConnectionState = false;
    disconnect();
    masterReference.clear();
}


//==============================================================================
bool InterprocessConnection::connectToSocket (const String& hostName,
                                              const int portNumber,
                                              const int timeOutMillisecs)
{
    disconnect();

    const ScopedLock sl (pipeAndSocketLock);
    socket = new StreamingSocket();

    if (socket->connect (hostName, portNumber, timeOutMillisecs))
    {
        connectionMadeInt();
        startThread();
        return true;
    }
    else
    {
        socket = nullptr;
        return false;
    }
}

bool InterprocessConnection::connectToPipe (const String& pipeName, const int timeoutMs)
{
    disconnect();

    ScopedPointer <NamedPipe> newPipe (new NamedPipe());

    if (newPipe->openExisting (pipeName))
    {
        const ScopedLock sl (pipeAndSocketLock);
        pipeReceiveMessageTimeout = timeoutMs;
        initialiseWithPipe (newPipe.release());
        return true;
    }

    return false;
}

bool InterprocessConnection::createPipe (const String& pipeName, const int timeoutMs)
{
    disconnect();

    ScopedPointer <NamedPipe> newPipe (new NamedPipe());

    if (newPipe->createNewPipe (pipeName))
    {
        const ScopedLock sl (pipeAndSocketLock);
        pipeReceiveMessageTimeout = timeoutMs;
        initialiseWithPipe (newPipe.release());
        return true;
    }

    return false;
}

void InterprocessConnection::disconnect()
{
    if (socket != nullptr)
        socket->close();

    if (pipe != nullptr)
        pipe->close();

    stopThread (4000);

    {
        const ScopedLock sl (pipeAndSocketLock);
        socket = nullptr;
        pipe = nullptr;
    }

    connectionLostInt();
}

bool InterprocessConnection::isConnected() const
{
    const ScopedLock sl (pipeAndSocketLock);

    return ((socket != nullptr && socket->isConnected())
              || (pipe != nullptr && pipe->isOpen()))
            && isThreadRunning();
}

String InterprocessConnection::getConnectedHostName() const
{
    if (pipe != nullptr)
        return "localhost";

    if (socket != nullptr)
    {
        if (! socket->isLocal())
            return socket->getHostName();

        return "localhost";
    }

    return String::empty;
}

//==============================================================================
bool InterprocessConnection::sendMessage (const MemoryBlock& message)
{
    uint32 messageHeader[2];
    messageHeader [0] = ByteOrder::swapIfBigEndian (magicMessageHeader);
    messageHeader [1] = ByteOrder::swapIfBigEndian ((uint32) message.getSize());

    MemoryBlock messageData (sizeof (messageHeader) + message.getSize());
    messageData.copyFrom (messageHeader, 0, sizeof (messageHeader));
    messageData.copyFrom (message.getData(), sizeof (messageHeader), message.getSize());

    int bytesWritten = 0;

    const ScopedLock sl (pipeAndSocketLock);

    if (socket != nullptr)
        bytesWritten = socket->write (messageData.getData(), (int) messageData.getSize());
    else if (pipe != nullptr)
        bytesWritten = pipe->write (messageData.getData(), (int) messageData.getSize(), pipeReceiveMessageTimeout);

    return bytesWritten == (int) messageData.getSize();
}

//==============================================================================
void InterprocessConnection::initialiseWithSocket (StreamingSocket* const socket_)
{
    jassert (socket == nullptr);
    socket = socket_;
    connectionMadeInt();
    startThread();
}

void InterprocessConnection::initialiseWithPipe (NamedPipe* const pipe_)
{
    jassert (pipe == nullptr);
    pipe = pipe_;
    connectionMadeInt();
    startThread();
}

//==============================================================================
struct ConnectionStateMessage  : public MessageManager::MessageBase
{
    ConnectionStateMessage (InterprocessConnection* ipc, bool connected) noexcept
        : owner (ipc), connectionMade (connected)
    {}

    void messageCallback() override
    {
        if (InterprocessConnection* const ipc = owner)
        {
            if (connectionMade)
                ipc->connectionMade();
            else
                ipc->connectionLost();
        }
    }

    WeakReference<InterprocessConnection> owner;
    bool connectionMade;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConnectionStateMessage)
};

void InterprocessConnection::connectionMadeInt()
{
    if (! callbackConnectionState)
    {
        callbackConnectionState = true;

        if (useMessageThread)
            (new ConnectionStateMessage (this, true))->post();
        else
            connectionMade();
    }
}

void InterprocessConnection::connectionLostInt()
{
    if (callbackConnectionState)
    {
        callbackConnectionState = false;

        if (useMessageThread)
            (new ConnectionStateMessage (this, false))->post();
        else
            connectionLost();
    }
}

struct DataDeliveryMessage  : public Message
{
    DataDeliveryMessage (InterprocessConnection* ipc, const MemoryBlock& d)
        : owner (ipc), data (d)
    {}

    void messageCallback() override
    {
        if (InterprocessConnection* const ipc = owner)
            ipc->messageReceived (data);
    }

    WeakReference<InterprocessConnection> owner;
    MemoryBlock data;
};

void InterprocessConnection::deliverDataInt (const MemoryBlock& data)
{
    jassert (callbackConnectionState);

    if (useMessageThread)
        (new DataDeliveryMessage (this, data))->post();
    else
        messageReceived (data);
}

//==============================================================================
bool InterprocessConnection::readNextMessageInt()
{
    uint32 messageHeader[2];
    const int bytes = socket != nullptr ? socket->read (messageHeader, sizeof (messageHeader), true)
                                        : pipe  ->read (messageHeader, sizeof (messageHeader), -1);

    if (bytes == sizeof (messageHeader)
         && ByteOrder::swapIfBigEndian (messageHeader[0]) == magicMessageHeader)
    {
        int bytesInMessage = (int) ByteOrder::swapIfBigEndian (messageHeader[1]);

        if (bytesInMessage > 0)
        {
            MemoryBlock messageData ((size_t) bytesInMessage, true);
            int bytesRead = 0;

            while (bytesInMessage > 0)
            {
                if (threadShouldExit())
                    return false;

                const int numThisTime = jmin (bytesInMessage, 65536);
                void* const data = addBytesToPointer (messageData.getData(), bytesRead);

                const int bytesIn = socket != nullptr ? socket->read (data, numThisTime, true)
                                                      : pipe  ->read (data, numThisTime, -1);

                if (bytesIn <= 0)
                    break;

                bytesRead += bytesIn;
                bytesInMessage -= bytesIn;
            }

            if (bytesRead >= 0)
                deliverDataInt (messageData);
        }
    }
    else if (bytes < 0)
    {
        if (socket != nullptr)
        {
            const ScopedLock sl (pipeAndSocketLock);
            socket = nullptr;
        }

        connectionLostInt();
        return false;
    }

    return true;
}

void InterprocessConnection::run()
{
    while (! threadShouldExit())
    {
        if (socket != nullptr)
        {
            const int ready = socket->waitUntilReady (true, 0);

            if (ready < 0)
            {
                {
                    const ScopedLock sl (pipeAndSocketLock);
                    socket = nullptr;
                }

                connectionLostInt();
                break;
            }
            else if (ready > 0)
            {
                if (! readNextMessageInt())
                    break;
            }
            else
            {
                Thread::sleep (1);
            }
        }
        else if (pipe != nullptr)
        {
            if (! pipe->isOpen())
            {
                {
                    const ScopedLock sl (pipeAndSocketLock);
                    pipe = nullptr;
                }

                connectionLostInt();
                break;
            }
            else
            {
                if (! readNextMessageInt())
                    break;
            }
        }
        else
        {
            break;
        }
    }
}
