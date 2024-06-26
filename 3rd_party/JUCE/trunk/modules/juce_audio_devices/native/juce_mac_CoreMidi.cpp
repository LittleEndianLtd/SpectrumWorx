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

#ifndef JUCE_LOG_COREMIDI_ERRORS
 #define JUCE_LOG_COREMIDI_ERRORS 1
#endif

namespace CoreMidiHelpers
{
    static bool checkError (const OSStatus err, const int lineNum)
    {
        if (err == noErr)
            return true;

       #if JUCE_LOG_COREMIDI_ERRORS
        Logger::writeToLog ("CoreMIDI error: " + String (lineNum) + " - " + String::toHexString ((int) err));
       #endif

        (void) lineNum;
        return false;
    }

    #undef CHECK_ERROR
    #define CHECK_ERROR(a) CoreMidiHelpers::checkError (a, __LINE__)

    //==============================================================================
    static String getMidiObjectName (MIDIObjectRef entity)
    {
        String result;
        CFStringRef str = nullptr;
        MIDIObjectGetStringProperty (entity, kMIDIPropertyName, &str);

        if (str != nullptr)
        {
            result = String::fromCFString (str);
            CFRelease (str);
        }

        return result;
    }

    static String getEndpointName (MIDIEndpointRef endpoint, bool isExternal)
    {
        String result (getMidiObjectName (endpoint));

        MIDIEntityRef entity = 0;  // NB: don't attempt to use nullptr for refs - it fails in some types of build.
        MIDIEndpointGetEntity (endpoint, &entity);

        if (entity == 0)
            return result; // probably virtual

        if (result.isEmpty())
            result = getMidiObjectName (entity);  // endpoint name is empty - try the entity

        // now consider the device's name
        MIDIDeviceRef device = 0;
        MIDIEntityGetDevice (entity, &device);

        if (device != 0)
        {
            const String deviceName (getMidiObjectName (device));

            if (deviceName.isNotEmpty())
            {
                // if an external device has only one entity, throw away
                // the endpoint name and just use the device name
                if (isExternal && MIDIDeviceGetNumberOfEntities (device) < 2)
                {
                    result = deviceName;
                }
                else if (! result.startsWithIgnoreCase (deviceName))
                {
                    // prepend the device name to the entity name
                    result = (deviceName + " " + result).trimEnd();
                }
            }
        }

        return result;
    }

    static String getConnectedEndpointName (MIDIEndpointRef endpoint)
    {
        String result;

        // Does the endpoint have connections?
        CFDataRef connections = nullptr;
        int numConnections = 0;

        MIDIObjectGetDataProperty (endpoint, kMIDIPropertyConnectionUniqueID, &connections);

        if (connections != nullptr)
        {
            numConnections = ((int) CFDataGetLength (connections)) / (int) sizeof (MIDIUniqueID);

            if (numConnections > 0)
            {
                const SInt32* pid = reinterpret_cast <const SInt32*> (CFDataGetBytePtr (connections));

                for (int i = 0; i < numConnections; ++i, ++pid)
                {
                    MIDIUniqueID uid = (MIDIUniqueID) ByteOrder::swapIfLittleEndian ((uint32) *pid);
                    MIDIObjectRef connObject;
                    MIDIObjectType connObjectType;
                    OSStatus err = MIDIObjectFindByUniqueID (uid, &connObject, &connObjectType);

                    if (err == noErr)
                    {
                        String s;

                        if (connObjectType == kMIDIObjectType_ExternalSource
                             || connObjectType == kMIDIObjectType_ExternalDestination)
                        {
                            // Connected to an external device's endpoint (10.3 and later).
                            s = getEndpointName (static_cast <MIDIEndpointRef> (connObject), true);
                        }
                        else
                        {
                            // Connected to an external device (10.2) (or something else, catch-all)
                            s = getMidiObjectName (connObject);
                        }

                        if (s.isNotEmpty())
                        {
                            if (result.isNotEmpty())
                                result += ", ";

                            result += s;
                        }
                    }
                }
            }

            CFRelease (connections);
        }

        if (result.isEmpty())  // Here, either the endpoint had no connections, or we failed to obtain names for them.
            result = getEndpointName (endpoint, false);

        return result;
    }

    static StringArray findDevices (const bool forInput)
    {
        const ItemCount num = forInput ? MIDIGetNumberOfSources()
                                       : MIDIGetNumberOfDestinations();
        StringArray s;

        for (ItemCount i = 0; i < num; ++i)
        {
            MIDIEndpointRef dest = forInput ? MIDIGetSource (i)
                                            : MIDIGetDestination (i);
            String name;

            if (dest != 0)
                name = getConnectedEndpointName (dest);

            if (name.isEmpty())
                name = "<error>";

            s.add (name);
        }

        return s;
    }

    static void globalSystemChangeCallback (const MIDINotification*, void*)
    {
        // TODO.. Should pass-on this notification..
    }

    static String getGlobalMidiClientName()
    {
        if (JUCEApplicationBase* const app = JUCEApplicationBase::getInstance())
            return app->getApplicationName();

        return "JUCE";
    }

    static MIDIClientRef getGlobalMidiClient()
    {
        static MIDIClientRef globalMidiClient = 0;

        if (globalMidiClient == 0)
        {
            // Since OSX 10.6, the MIDIClientCreate function will only work
            // correctly when called from the message thread!
            jassert (MessageManager::getInstance()->isThisTheMessageThread());

            CFStringRef name = getGlobalMidiClientName().toCFString();
            CHECK_ERROR (MIDIClientCreate (name, &globalSystemChangeCallback, nullptr, &globalMidiClient));
            CFRelease (name);
        }

        return globalMidiClient;
    }

    //==============================================================================
    class MidiPortAndEndpoint
    {
    public:
        MidiPortAndEndpoint (MIDIPortRef p, MIDIEndpointRef ep)
            : port (p), endPoint (ep)
        {
        }

        ~MidiPortAndEndpoint()
        {
            if (port != 0)
                MIDIPortDispose (port);

            if (port == 0 && endPoint != 0) // if port == nullptr, it means we created the endpoint, so it's safe to delete it
                MIDIEndpointDispose (endPoint);
        }

        void send (const MIDIPacketList* const packets)
        {
            if (port != 0)
                MIDISend (port, endPoint, packets);
            else
                MIDIReceived (endPoint, packets);
        }

        MIDIPortRef port;
        MIDIEndpointRef endPoint;
    };

    //==============================================================================
    class MidiPortAndCallback;
    LE_PATCH( LE_WEAK_SYMBOL ) CriticalSection callbackLock;
    LE_PATCH( LE_WEAK_SYMBOL ) Array<MidiPortAndCallback*> activeCallbacks;

    class MidiPortAndCallback
    {
    public:
        MidiPortAndCallback (MidiInputCallback& cb)
            : input (nullptr), active (false), callback (cb), concatenator (2048)
        {
        }

        ~MidiPortAndCallback()
        {
            active = false;

            {
                const ScopedLock sl (callbackLock);
                activeCallbacks.removeFirstMatchingValue (this);
            }

            if (portAndEndpoint != 0 && portAndEndpoint->port != 0)
                CHECK_ERROR (MIDIPortDisconnectSource (portAndEndpoint->port, portAndEndpoint->endPoint));
        }

        void handlePackets (const MIDIPacketList* const pktlist)
        {
            const double time = Time::getMillisecondCounterHiRes() * 0.001;

            const ScopedLock sl (callbackLock);
            if (activeCallbacks.contains (this) && active)
            {
                const MIDIPacket* packet = &pktlist->packet[0];

                for (unsigned int i = 0; i < pktlist->numPackets; ++i)
                {
                    concatenator.pushMidiData (packet->data, (int) packet->length, time,
                                               input, callback);

                    packet = MIDIPacketNext (packet);
                }
            }
        }

        MidiInput* input;
        ScopedPointer<MidiPortAndEndpoint> portAndEndpoint;
        volatile bool active;

    private:
        MidiInputCallback& callback;
        MidiDataConcatenator concatenator;
    };

    static void midiInputProc (const MIDIPacketList* pktlist, void* readProcRefCon, void* /*srcConnRefCon*/)
    {
        static_cast <MidiPortAndCallback*> (readProcRefCon)->handlePackets (pktlist);
    }
}

//==============================================================================
StringArray MidiOutput::getDevices()        { return CoreMidiHelpers::findDevices (false); }
int MidiOutput::getDefaultDeviceIndex()     { return 0; }

MidiOutput* MidiOutput::openDevice (int index)
{
    MidiOutput* mo = nullptr;

    if (isPositiveAndBelow (index, (int) MIDIGetNumberOfDestinations()))
    {
        MIDIEndpointRef endPoint = MIDIGetDestination ((ItemCount) index);

        CFStringRef pname;
        if (CHECK_ERROR (MIDIObjectGetStringProperty (endPoint, kMIDIPropertyName, &pname)))
        {
            MIDIClientRef client = CoreMidiHelpers::getGlobalMidiClient();
            MIDIPortRef port;

            if (client != 0 && CHECK_ERROR (MIDIOutputPortCreate (client, pname, &port)))
            {
                mo = new MidiOutput();
                mo->internal = new CoreMidiHelpers::MidiPortAndEndpoint (port, endPoint);
            }

            CFRelease (pname);
        }
    }

    return mo;
}

MidiOutput* MidiOutput::createNewDevice (const String& deviceName)
{
    MidiOutput* mo = nullptr;
    MIDIClientRef client = CoreMidiHelpers::getGlobalMidiClient();

    MIDIEndpointRef endPoint;
    CFStringRef name = deviceName.toCFString();

    if (client != 0 && CHECK_ERROR (MIDISourceCreate (client, name, &endPoint)))
    {
        mo = new MidiOutput();
        mo->internal = new CoreMidiHelpers::MidiPortAndEndpoint (0, endPoint);
    }

    CFRelease (name);
    return mo;
}

MidiOutput::~MidiOutput()
{
    delete static_cast<CoreMidiHelpers::MidiPortAndEndpoint*> (internal);
}

void MidiOutput::sendMessageNow (const MidiMessage& message)
{
   #if JUCE_IOS
    const MIDITimeStamp timeStamp = mach_absolute_time();
   #else
    const MIDITimeStamp timeStamp = AudioGetCurrentHostTime();
   #endif

    HeapBlock <MIDIPacketList> allocatedPackets;
    MIDIPacketList stackPacket;
    MIDIPacketList* packetToSend = &stackPacket;
    const size_t dataSize = (size_t) message.getRawDataSize();

    if (message.isSysEx())
    {
        const int maxPacketSize = 256;
        int pos = 0, bytesLeft = (int) dataSize;
        const int numPackets = (bytesLeft + maxPacketSize - 1) / maxPacketSize;
        allocatedPackets.malloc ((size_t) (32 * (size_t) numPackets + dataSize), 1);
        packetToSend = allocatedPackets;
        packetToSend->numPackets = (UInt32) numPackets;

        MIDIPacket* p = packetToSend->packet;

        for (int i = 0; i < numPackets; ++i)
        {
            p->timeStamp = timeStamp;
            p->length = (UInt16) jmin (maxPacketSize, bytesLeft);
            memcpy (p->data, message.getRawData() + pos, p->length);
            pos += p->length;
            bytesLeft -= p->length;
            p = MIDIPacketNext (p);
        }
    }
    else if (dataSize < 65536) // max packet size
    {
        const size_t stackCapacity = sizeof (stackPacket.packet->data);

        if (dataSize > stackCapacity)
        {
            allocatedPackets.malloc ((sizeof (MIDIPacketList) - stackCapacity) + dataSize, 1);
            packetToSend = allocatedPackets;
        }

        packetToSend->numPackets = 1;
        MIDIPacket& p = *(packetToSend->packet);
        p.timeStamp = timeStamp;
        p.length = (UInt16) dataSize;
        memcpy (p.data, message.getRawData(), dataSize);
    }
    else
    {
        jassertfalse; // packet too large to send!
        return;
    }

    static_cast<CoreMidiHelpers::MidiPortAndEndpoint*> (internal)->send (packetToSend);
}

//==============================================================================
StringArray MidiInput::getDevices()     { return CoreMidiHelpers::findDevices (true); }
int MidiInput::getDefaultDeviceIndex()  { return 0; }

MidiInput* MidiInput::openDevice (int index, MidiInputCallback* callback)
{
    jassert (callback != nullptr);

    using namespace CoreMidiHelpers;
    MidiInput* newInput = nullptr;

    if (isPositiveAndBelow (index, (int) MIDIGetNumberOfSources()))
    {
        if (MIDIEndpointRef endPoint = MIDIGetSource ((ItemCount) index))
        {
            CFStringRef name;

            if (CHECK_ERROR (MIDIObjectGetStringProperty (endPoint, kMIDIPropertyName, &name)))
            {
                if (MIDIClientRef client = getGlobalMidiClient())
                {
                    MIDIPortRef port;
                    ScopedPointer <MidiPortAndCallback> mpc (new MidiPortAndCallback (*callback));

                    if (CHECK_ERROR (MIDIInputPortCreate (client, name, midiInputProc, mpc, &port)))
                    {
                        if (CHECK_ERROR (MIDIPortConnectSource (port, endPoint, nullptr)))
                        {
                            mpc->portAndEndpoint = new MidiPortAndEndpoint (port, endPoint);

                            newInput = new MidiInput (getDevices() [index]);
                            mpc->input = newInput;
                            newInput->internal = mpc;

                            const ScopedLock sl (callbackLock);
                            activeCallbacks.add (mpc.release());
                        }
                        else
                        {
                            CHECK_ERROR (MIDIPortDispose (port));
                        }
                    }
                }
            }

            CFRelease (name);
        }
    }

    return newInput;
}

MidiInput* MidiInput::createNewDevice (const String& deviceName, MidiInputCallback* callback)
{
    jassert (callback != nullptr);

    using namespace CoreMidiHelpers;
    MidiInput* mi = nullptr;

    if (MIDIClientRef client = getGlobalMidiClient())
    {
        ScopedPointer <MidiPortAndCallback> mpc (new MidiPortAndCallback (*callback));
        mpc->active = false;

        MIDIEndpointRef endPoint;
        CFStringRef name = deviceName.toCFString();

        if (CHECK_ERROR (MIDIDestinationCreate (client, name, midiInputProc, mpc, &endPoint)))
        {
            mpc->portAndEndpoint = new MidiPortAndEndpoint (0, endPoint);

            mi = new MidiInput (deviceName);
            mpc->input = mi;
            mi->internal = mpc;

            const ScopedLock sl (callbackLock);
            activeCallbacks.add (mpc.release());
        }

        CFRelease (name);
    }

    return mi;
}

MidiInput::MidiInput (const String& nm)  : name (nm)
{
}

MidiInput::~MidiInput()
{
    delete static_cast<CoreMidiHelpers::MidiPortAndCallback*> (internal);
}

void MidiInput::start()
{
    const ScopedLock sl (CoreMidiHelpers::callbackLock);
    static_cast<CoreMidiHelpers::MidiPortAndCallback*> (internal)->active = true;
}

void MidiInput::stop()
{
    const ScopedLock sl (CoreMidiHelpers::callbackLock);
    static_cast<CoreMidiHelpers::MidiPortAndCallback*> (internal)->active = false;
}

#undef CHECK_ERROR
