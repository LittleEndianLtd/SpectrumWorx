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

DirectoryContentsDisplayComponent::DirectoryContentsDisplayComponent (DirectoryContentsList& listToShow)
    : fileList (listToShow)
{
}

#ifndef LE_PATCHED_JUCE
DirectoryContentsDisplayComponent::~DirectoryContentsDisplayComponent()
{
}
#endif // LE_PATCHED_JUCE

//==============================================================================
FileBrowserListener::~FileBrowserListener()
{
}

void DirectoryContentsDisplayComponent::addListener (FileBrowserListener* const listener)
{
    listeners.add (listener);
}

void DirectoryContentsDisplayComponent::removeListener (FileBrowserListener* const listener)
{
    listeners.remove (listener);
}

void DirectoryContentsDisplayComponent::sendSelectionChangeMessage()
{
#ifdef LE_PATCHED_JUCE
    listeners.call( &FileBrowserListener::selectionChanged );
#else
    Component::BailOutChecker checker (dynamic_cast <Component*> (this));
    listeners.callChecked (checker, &FileBrowserListener::selectionChanged);
#endif // LE_PATCHED_JUCE
}

void DirectoryContentsDisplayComponent::sendMouseClickMessage (const File& file, const MouseEvent& e)
{
    if (fileList.getDirectory().exists())
    {
    #ifdef LE_PATCHED_JUCE
        listeners.call( &FileBrowserListener::fileClicked, file, e );
    #else
        Component::BailOutChecker checker (dynamic_cast <Component*> (this));
        listeners.callChecked (checker, &FileBrowserListener::fileClicked, file, e);
    #endif // LE_PATCHED_JUCE
    }
}

void DirectoryContentsDisplayComponent::sendDoubleClickMessage (const File& file)
{
    if (fileList.getDirectory().exists())
    {
    #ifdef LE_PATCHED_JUCE
        listeners.call( &FileBrowserListener::fileDoubleClicked, file );
    #else
        Component::BailOutChecker checker (dynamic_cast <Component*> (this));
        listeners.callChecked (checker, &FileBrowserListener::fileDoubleClicked, file);
    #endif // LE_PATCHED_JUCE
    }
}
