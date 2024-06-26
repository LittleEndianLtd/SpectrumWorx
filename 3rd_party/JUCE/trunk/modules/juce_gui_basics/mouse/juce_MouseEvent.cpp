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

MouseEvent::MouseEvent (MouseInputSource& inputSource,
                        Point<int> LE_PATCH( const & ) position,
                        ModifierKeys modKeys,
                        Component* const eventComp,
                        Component* const originator,
                        Time LE_PATCH( const & ) time,
                        Point<int> LE_PATCH( const & ) downPos,
                        Time LE_PATCH( const & ) downTime,
                        const int numClicks,
                        const bool mouseWasDragged) noexcept
    : x (position.x),
      y (position.y),
      mods (modKeys),
      eventComponent (eventComp),
      originalComponent (originator),
      eventTime (time),
      mouseDownTime (downTime),
      source (inputSource),
      mouseDownPos (downPos),
      numberOfClicks ((uint8) numClicks),
      wasMovedSinceMouseDown ((uint8) (mouseWasDragged ? 1 : 0))
{
}

#ifndef LE_PATCHED_JUCE
MouseEvent::~MouseEvent() noexcept
{
}
#endif // LE_PATCHED_JUCE

//==============================================================================
MouseEvent MouseEvent::getEventRelativeTo (Component* const otherComponent) const noexcept
{
    jassert (otherComponent != nullptr);

    return MouseEvent (source, otherComponent->getLocalPoint (eventComponent, getPosition()),
                       mods, otherComponent, originalComponent, eventTime,
                       otherComponent->getLocalPoint (eventComponent, mouseDownPos),
                       mouseDownTime, numberOfClicks, wasMovedSinceMouseDown != 0);
}

MouseEvent MouseEvent::withNewPosition (Point<int> LE_PATCH( const & ) newPosition) const noexcept
{
    return MouseEvent (source, newPosition, mods, eventComponent, originalComponent,
                       eventTime, mouseDownPos, mouseDownTime,
                       numberOfClicks, wasMovedSinceMouseDown != 0);
}

//==============================================================================
bool MouseEvent::mouseWasClicked() const noexcept
{
    return wasMovedSinceMouseDown == 0;
}

int MouseEvent::getLengthOfMousePress() const noexcept
{
    if (mouseDownTime.toMilliseconds() > 0)
        return jmax (0, (int) (eventTime - mouseDownTime).inMilliseconds());

    return 0;
}

//==============================================================================
Point<int> MouseEvent::getPosition() const noexcept             { return Point<int> (x, y); }
Point<int> MouseEvent::getScreenPosition() const                { return eventComponent->localPointToGlobal (getPosition()); }

Point<int> MouseEvent::getMouseDownPosition() const noexcept    { return mouseDownPos; }
Point<int> MouseEvent::getMouseDownScreenPosition() const       { return eventComponent->localPointToGlobal (mouseDownPos); }

Point<int> MouseEvent::getOffsetFromDragStart() const noexcept  { return getPosition() - mouseDownPos; }
int MouseEvent::getDistanceFromDragStart() const noexcept       { return mouseDownPos.getDistanceFrom (getPosition()); }

int MouseEvent::getMouseDownX() const noexcept                  { return mouseDownPos.x; }
int MouseEvent::getMouseDownY() const noexcept                  { return mouseDownPos.y; }

int MouseEvent::getDistanceFromDragStartX() const noexcept      { return x - mouseDownPos.x; }
int MouseEvent::getDistanceFromDragStartY() const noexcept      { return y - mouseDownPos.y; }

int MouseEvent::getScreenX() const                              { return getScreenPosition().x; }
int MouseEvent::getScreenY() const                              { return getScreenPosition().y; }

int MouseEvent::getMouseDownScreenX() const                     { return getMouseDownScreenPosition().x; }
int MouseEvent::getMouseDownScreenY() const                     { return getMouseDownScreenPosition().y; }

//==============================================================================
static int doubleClickTimeOutMs = 400;

int MouseEvent::getDoubleClickTimeout() noexcept                        { return doubleClickTimeOutMs; }
void MouseEvent::setDoubleClickTimeout (const int newTime) noexcept     { doubleClickTimeOutMs = newTime; }
