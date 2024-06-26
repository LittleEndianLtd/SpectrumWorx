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

Message::Message() noexcept {}
Message::~Message() LE_PATCH( noexcept ) {}

void Message::messageCallback()
{
    if (MessageListener* const r = recipient)
        r->handleMessage (*this);
}

MessageListener::MessageListener() noexcept
#ifdef LE_PATCHED_JUCE
    : masterReference( *this )
#endif // LE_PATCHED_JUCE
{
    // Are you trying to create a messagelistener before or after juce has been intialised??
    jassert (MessageManager::getInstanceWithoutCreating() != nullptr);
}

MessageListener::~MessageListener() LE_PATCH( noexcept )
{
    masterReference.clear();
}

void MessageListener::postMessage (Message* const message) const
{
    message->recipient = const_cast <MessageListener*> (this);
    message->post();
}
