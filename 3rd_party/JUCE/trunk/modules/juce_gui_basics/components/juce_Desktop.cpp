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

Desktop::Desktop()
    : mouseClickCounter (0), mouseWheelCounter (0),
#ifdef LE_PATCHED_JUCE
      currentLookAndFeel( nullptr ),
#else
      kioskModeComponent (nullptr),
#endif // LE_PATCHED_JUCE
      allowedOrientations (allOrientations),
      masterScaleFactor ((float) getDefaultMasterScale())
{
    displays = new Displays (*this);
    addMouseInputSource();
}

Desktop::~Desktop()
{
    setScreenSaverEnabled (true);

#ifndef LE_PATCHED_JUCE
    jassert (instance == this);
    instance = nullptr;
#endif // LE_PATCHED_JUCE

    // doh! If you don't delete all your windows before exiting, you're going to
    // be leaking memory!
    jassert (desktopComponents.size() == 0);
}
#ifdef LE_PATCHED_JUCE
union DesktopSingletonStorage
{
    double aligner;
    char   storage[ sizeof( Desktop ) ];
};
static DesktopSingletonStorage desktopSingletonStorage;
void Desktop::create ()
{
    LE_PATCH_ASSUME( instance == nullptr );
    instance = new ( &desktopSingletonStorage ) Desktop;
    LE_PATCH_ASSUME( instance == reinterpret_cast<Desktop const *>( &desktopSingletonStorage ) );
}
void Desktop::destroy()
{
    LE_PATCH_ASSUME( instance == reinterpret_cast<Desktop const *>( &desktopSingletonStorage ) );
    instance->~Desktop();
    instance = nullptr;
}
#endif // LE_PATCHED_JUCE
Desktop& JUCE_CALLTYPE Desktop::getInstance()
{
#ifdef LE_PATCHED_JUCE
    LE_PATCH_ASSUME( instance == reinterpret_cast<Desktop const *>( &desktopSingletonStorage ) );
    return *instance;
#else
    if (instance == nullptr)
        instance = new Desktop();

    return *instance;
#endif // LE_PATCHED_JUCE
}

Desktop * Desktop::instance = nullptr;

//==============================================================================
int Desktop::getNumComponents() const noexcept
{
    return desktopComponents.size();
}

Component* Desktop::getComponent (const int index) const noexcept
{
    return desktopComponents [index];
}

Component* Desktop::findComponentAt (Point<int> LE_PATCH( const & ) screenPosition) const
{
    for (int i = desktopComponents.size(); --i >= 0;)
    {
        Component* const c = desktopComponents.getUnchecked(i);

        if (c->isVisible())
        {
            const Point<int> relative (c->getLocalPoint (nullptr, screenPosition));

            if (c->contains (relative))
                return c->getComponentAt (relative);
        }
    }

    return nullptr;
}

//==============================================================================
LookAndFeel& Desktop::getDefaultLookAndFeel() noexcept
{
#ifdef LE_PATCHED_JUCE
    jassert( currentLookAndFeel );
    return *currentLookAndFeel;
#else
    if (currentLookAndFeel == nullptr)
    {
        if (defaultLookAndFeel == nullptr)
            defaultLookAndFeel = new LookAndFeel();

        currentLookAndFeel = defaultLookAndFeel;
    }

    return *currentLookAndFeel;
#endif // LE_PATCHED_JUCE
}

void Desktop::setDefaultLookAndFeel (LookAndFeel* newDefaultLookAndFeel)
{
    currentLookAndFeel = newDefaultLookAndFeel;

    for (int i = getNumComponents(); --i >= 0;)
    {
        LE_PATCH_UNREACHABLE_CODE
        if (Component* const c = getComponent (i))
            c->sendLookAndFeelChange();
    }
}

//==============================================================================
void Desktop::addDesktopComponent (Component* const c)
{
    jassert (c != nullptr);
    jassert (! desktopComponents.contains (c));
    desktopComponents.addIfNotAlreadyThere (c);
}

void Desktop::removeDesktopComponent (Component* const c)
{
    desktopComponents.removeFirstMatchingValue (c);
}

void Desktop::componentBroughtToFront (Component* const c)
{
    const int index = desktopComponents.indexOf (c);
    jassert (index >= 0);

    if (index >= 0)
    {
        int newIndex = -1;

        if (! c->isAlwaysOnTop())
        {
            newIndex = desktopComponents.size();

            while (newIndex > 0 && desktopComponents.getUnchecked (newIndex - 1)->isAlwaysOnTop())
                --newIndex;

            --newIndex;
        }

        desktopComponents.move (index, newIndex);
    }
}

//==============================================================================
Point<int> Desktop::getMousePosition()
{
    return getInstance().getMainMouseSource().getScreenPosition();
}

void Desktop::setMousePosition (Point<int> LE_PATCH( const & ) newPosition)
{
    getInstance().getMainMouseSource().setScreenPosition (newPosition);
}

Point<int> Desktop::getLastMouseDownPosition()
{
    return getInstance().getMainMouseSource().getLastMouseDownPosition();
}

int Desktop::getMouseButtonClickCounter() const noexcept    { return mouseClickCounter; }
int Desktop::getMouseWheelMoveCounter() const noexcept      { return mouseWheelCounter; }

void Desktop::incrementMouseClickCounter() noexcept         { ++mouseClickCounter; }
void Desktop::incrementMouseWheelCounter() noexcept         { ++mouseWheelCounter; }

int Desktop::getNumDraggingMouseSources() const noexcept
{
    int num = 0;
    for (int i = mouseSources.size(); --i >= 0;)
        if (mouseSources.getUnchecked(i)->isDragging())
            ++num;

    return num;
}

MouseInputSource* Desktop::getDraggingMouseSource (int index) const noexcept
{
    int num = 0;
    for (int i = mouseSources.size(); --i >= 0;)
    {
        MouseInputSource* const mi = mouseSources.getUnchecked(i);

        if (mi->isDragging())
        {
            if (index == num)
                return mi;

            ++num;
        }
    }

    return nullptr;
}

MouseInputSource* Desktop::getOrCreateMouseInputSource (int touchIndex)
{
    jassert (touchIndex >= 0 && touchIndex < 100); // sanity-check on number of fingers

    for (;;)
    {
        if (MouseInputSource* mouse = getMouseSource (touchIndex))
            return mouse;

        if (! addMouseInputSource())
        {
            jassertfalse; // not enough mouse sources!
            return nullptr;
        }
    }
}

//==============================================================================
class MouseDragAutoRepeater  : public Timer
{
public:
    MouseDragAutoRepeater() {}

    void timerCallback() override
    {
        Desktop& desktop = Desktop::getInstance();
        int numMiceDown = 0;

        for (int i = desktop.getNumMouseSources(); --i >= 0;)
        {
            MouseInputSource& source = *desktop.getMouseSource(i);

            if (source.isDragging())
            {
                source.triggerFakeMove();
                ++numMiceDown;
            }
        }

        if (numMiceDown == 0)
            desktop.beginDragAutoRepeat (0);
    }

private:
    JUCE_DECLARE_NON_COPYABLE (MouseDragAutoRepeater)
};

void Desktop::beginDragAutoRepeat (const int interval)
{
#ifdef LE_PATCHED_JUCE
    (void)interval;
#else
    if (interval > 0)
    {
        if (dragRepeater == nullptr)
            dragRepeater = new MouseDragAutoRepeater();

        if (dragRepeater->getTimerInterval() != interval)
            dragRepeater->startTimer (interval);
    }
    else
    {
        dragRepeater = nullptr;
    }
#endif // LE_PATCHED_JUCE
}

//==============================================================================
void Desktop::addFocusChangeListener (FocusChangeListener* const listener)
{
    focusListeners.add (listener);
}

void Desktop::removeFocusChangeListener (FocusChangeListener* const listener)
{
    focusListeners.remove (listener);
}

void Desktop::triggerFocusCallback()
{
    triggerAsyncUpdate();
}

void Desktop::handleAsyncUpdate()
{
    // The component may be deleted during this operation, but we'll use a SafePointer rather than a
    // BailOutChecker so that any remaining listeners will still get a callback (with a null pointer).
    WeakReference<Component> currentFocus (Component::getCurrentlyFocusedComponent());
    focusListeners.call (&FocusChangeListener::globalFocusChanged, currentFocus);
}

//==============================================================================
void Desktop::resetTimer()
{
#ifndef LE_PATCHED_JUCE
    if (mouseListeners.size() == 0)
        stopTimer();
    else
        startTimer (100);

    lastFakeMouseMove = getMousePosition();
#endif // LE_PATCHED_JUCE
}

ListenerList <MouseListener>& Desktop::getMouseListeners()
{
    resetTimer();
    return mouseListeners;
}

void Desktop::addGlobalMouseListener (MouseListener* const listener)
{
    mouseListeners.add (listener);
    resetTimer();
}

void Desktop::removeGlobalMouseListener (MouseListener* const listener)
{
    mouseListeners.remove (listener);
    resetTimer();
}

void Desktop::timerCallback()
{
#ifndef LE_PATCHED_JUCE
    if (lastFakeMouseMove != getMousePosition())
        sendMouseMove();
#endif // LE_PATCHED_JUCE
}

void Desktop::sendMouseMove()
{
    if (! mouseListeners.isEmpty())
    {
        JUCE_ORIGINAL( startTimer (20); )

        LE_PATCH( Point<int> const ) lastFakeMouseMove = getMousePosition();

        if (Component* const target = findComponentAt (lastFakeMouseMove))
        {
            Component::BailOutChecker checker (target);
            const Point<int> pos (target->getLocalPoint (nullptr, lastFakeMouseMove));
            const Time now (Time::getCurrentTime());

            const MouseEvent me (getMainMouseSource(), pos, ModifierKeys::getCurrentModifiers(),
                                 target, target, now, pos, now, 0, false);

            if (me.mods.isAnyMouseButtonDown())
                mouseListeners.callChecked (checker, &MouseListener::mouseDrag, me);
            else
                mouseListeners.callChecked (checker, &MouseListener::mouseMove, me);
        }
    }
}


//==============================================================================
Desktop::Displays::Displays (Desktop& desktop)   { init (desktop); }
Desktop::Displays::~Displays()  {}

const Desktop::Displays::Display& Desktop::Displays::getMainDisplay() const noexcept
{
    jassert (displays.getReference(0).isMain);
    return displays.getReference(0);
}

const Desktop::Displays::Display& Desktop::Displays::getDisplayContaining (Point<int> position) const noexcept
{
    const Display* best = &displays.getReference(0);
    double bestDistance = 1.0e10;

    for (int i = displays.size(); --i >= 0;)
    {
        const Display& d = displays.getReference(i);

        if (d.totalArea.contains (position))
        {
            best = &d;
            break;
        }

        const double distance = d.totalArea.getCentre().getDistanceFrom (position);

        if (distance < bestDistance)
        {
            bestDistance = distance;
            best = &d;
        }
    }

    return *best;
}

RectangleList<int> Desktop::Displays::getRectangleList (bool userAreasOnly) const
{
    RectangleList<int> rl;

    for (int i = 0; i < displays.size(); ++i)
    {
        const Display& d = displays.getReference(i);
        rl.addWithoutMerging (userAreasOnly ? d.userArea : d.totalArea);
    }

    return rl;
}

Rectangle<int> Desktop::Displays::getTotalBounds (bool userAreasOnly) const
{
    return getRectangleList (userAreasOnly).getBounds();
}

bool operator== (const Desktop::Displays::Display& d1, const Desktop::Displays::Display& d2) noexcept;
bool operator== (const Desktop::Displays::Display& d1, const Desktop::Displays::Display& d2) noexcept
{
    return d1.userArea == d2.userArea
        && d1.totalArea == d2.totalArea
        && d1.scale == d2.scale
        && d1.isMain == d2.isMain;
}

bool operator!= (const Desktop::Displays::Display& d1, const Desktop::Displays::Display& d2) noexcept;
bool operator!= (const Desktop::Displays::Display& d1, const Desktop::Displays::Display& d2) noexcept
{
    return ! (d1 == d2);
}

void Desktop::Displays::init (Desktop& desktop)
{
    findDisplays (desktop.getGlobalScaleFactor());
    jassert (displays.size() > 0);
}

void Desktop::Displays::refresh()
{
    Array<Display> oldDisplays;
    oldDisplays.swapWith (displays);

    init (Desktop::getInstance());

    if (oldDisplays != displays)
    {
        for (int i = ComponentPeer::getNumPeers(); --i >= 0;)
            if (ComponentPeer* const peer = ComponentPeer::getPeer (i))
                peer->handleScreenSizeChange();
    }
}

//==============================================================================
void Desktop::setKioskModeComponent (Component* componentToUse, const bool allowMenusAndBars)
{
#ifdef LE_PATCHED_JUCE
    jassert( !componentToUse );
    (void)componentToUse; (void)allowMenusAndBars;
#else
    if (kioskModeComponent != componentToUse)
    {
        // agh! Don't delete or remove a component from the desktop while it's still the kiosk component!
        jassert (kioskModeComponent == nullptr || ComponentPeer::getPeerFor (kioskModeComponent) != nullptr);

        if (kioskModeComponent != nullptr)
        {
            setKioskComponent (kioskModeComponent, false, allowMenusAndBars);

            kioskModeComponent->setBounds (kioskComponentOriginalBounds);
        }

        kioskModeComponent = componentToUse;

        if (kioskModeComponent != nullptr)
        {
            // Only components that are already on the desktop can be put into kiosk mode!
            jassert (ComponentPeer::getPeerFor (kioskModeComponent) != nullptr);

            kioskComponentOriginalBounds = kioskModeComponent->getBounds();

            setKioskComponent (kioskModeComponent, true, allowMenusAndBars);
        }
    }
#endif // LE_PATCHED_JUCE
}

//==============================================================================
void Desktop::setOrientationsEnabled (const int newOrientations)
{
    // Dodgy set of flags being passed here! Make sure you specify at least one permitted orientation.
    jassert (newOrientations != 0 && (newOrientations & ~allOrientations) == 0);

    allowedOrientations = newOrientations;
}

bool Desktop::isOrientationEnabled (const DisplayOrientation orientation) const noexcept
{
    // Make sure you only pass one valid flag in here...
    jassert (orientation == upright || orientation == upsideDown || orientation == rotatedClockwise || orientation ==  rotatedAntiClockwise);

    return (allowedOrientations & orientation) != 0;
}

void Desktop::setGlobalScaleFactor (float newScaleFactor) noexcept
{
    if (masterScaleFactor != newScaleFactor)
    {
        masterScaleFactor = newScaleFactor;
        displays->refresh();
    }
}
