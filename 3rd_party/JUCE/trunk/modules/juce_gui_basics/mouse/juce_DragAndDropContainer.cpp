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

bool juce_performDragDropFiles (const StringArray&, const bool copyFiles, bool& shouldStop);
bool juce_performDragDropText (const String&, bool& shouldStop);


//==============================================================================
class DragAndDropContainer::DragImageComponent  : public Component,
                                                  private Timer
{
public:
    DragImageComponent (const Image& im,
                        const var& desc,
                        Component* const sourceComponent,
                        Component* const mouseDragSource_,
                        DragAndDropContainer& owner_,
                        Point<int> imageOffset_)
        : sourceDetails (desc, sourceComponent, Point<int>()),
          image (im),
          owner (owner_),
          mouseDragSource (mouseDragSource_),
          imageOffset (imageOffset_),
          hasCheckedForExternalDrag (false)
    {
        setSize (im.getWidth(), im.getHeight());

        if (mouseDragSource == nullptr)
            mouseDragSource = sourceComponent;

        mouseDragSource->addMouseListener (this, false);

        startTimer (200);

        setInterceptsMouseClicks (false, false);
        setAlwaysOnTop (true);
    }

    ~DragImageComponent()
    {
        if (owner.dragImageComponent == this)
            owner.dragImageComponent.release();

        if (mouseDragSource != nullptr)
        {
            mouseDragSource->removeMouseListener (this);

            if (DragAndDropTarget* const current = getCurrentlyOver())
                if (current->isInterestedInDragSource (sourceDetails))
                    current->itemDragExit (sourceDetails);
        }
    }

    void paint (Graphics& g) override
    {
        if (isOpaque())
            g.fillAll (Colours::white);

        g.setOpacity (1.0f);
        g.drawImageAt (image, 0, 0);
    }

    void mouseUp (const MouseEvent& e) override
    {
        if (e.originalComponent != this)
        {
            if (mouseDragSource != nullptr)
                mouseDragSource->removeMouseListener (this);

            // (note: use a local copy of this in case the callback runs
            // a modal loop and deletes this object before the method completes)
            DragAndDropTarget::SourceDetails details (sourceDetails);
            DragAndDropTarget* finalTarget = nullptr;

            const bool wasVisible = isVisible();
            setVisible (false);
            Component* unused;
            finalTarget = findTarget (e.getScreenPosition(), details.localPosition, unused);

            if (wasVisible) // fade the component and remove it - it'll be deleted later by the timer callback
                dismissWithAnimation (finalTarget == nullptr);

            if (Component* parent = getParentComponent())
                parent->removeChildComponent (this);

            if (finalTarget != nullptr)
            {
                currentlyOverComp = nullptr;
                finalTarget->itemDropped (details);
            }

            // careful - this object could now be deleted..
        }
    }

    void mouseDrag (const MouseEvent& e) override
    {
        if (e.originalComponent != this)
            updateLocation (true, e.getScreenPosition());
    }

    void updateLocation (const bool canDoExternalDrag, Point<int> LE_PATCH( const & ) screenPos)
    {
        DragAndDropTarget::SourceDetails details (sourceDetails);

        setNewScreenPos (screenPos);

        Component* newTargetComp;
        DragAndDropTarget* const newTarget = findTarget (screenPos, details.localPosition, newTargetComp);

        setVisible (newTarget == nullptr || newTarget->shouldDrawDragImageWhenOver());

        if (newTargetComp != currentlyOverComp)
        {
            if (DragAndDropTarget* const lastTarget = getCurrentlyOver())
                if (details.sourceComponent != nullptr && lastTarget->isInterestedInDragSource (details))
                    lastTarget->itemDragExit (details);

            currentlyOverComp = newTargetComp;

            if (newTarget != nullptr
                  && newTarget->isInterestedInDragSource (details))
                newTarget->itemDragEnter (details);
        }

        sendDragMove (details);

    #ifdef LE_PATCHED_JUCE
        #ifndef NDEBUG
            StringArray files;
            bool canMoveFiles( false );
            jassert( !owner.shouldDropFilesWhenDraggedExternally (details, files, canMoveFiles) );
        #endif // NDEBUG
    #else
        if (canDoExternalDrag)
        {
            const Time now (Time::getCurrentTime());

            if (getCurrentlyOver() != nullptr)
                lastTimeOverTarget = now;
            else if (now > lastTimeOverTarget + RelativeTime::milliseconds (700))
                checkForExternalDrag (details, screenPos);
        }
    #endif // LE_PATCHED_JUCE
    }

    void timerCallback() override
    {
        if (sourceDetails.sourceComponent == nullptr)
        {
            delete this;
        }
        else if (! isMouseButtonDownAnywhere())
        {
            if (mouseDragSource != nullptr)
                mouseDragSource->removeMouseListener (this);

            delete this;
        }
    }

private:
    DragAndDropTarget::SourceDetails sourceDetails;
    Image image;
    DragAndDropContainer& owner;
    WeakReference<Component> mouseDragSource, currentlyOverComp;
    const Point<int> imageOffset;
    bool hasCheckedForExternalDrag;
    Time lastTimeOverTarget;

    DragAndDropTarget* getCurrentlyOver() const noexcept
    {
    #ifdef LE_PATCHED_JUCE
        return nullptr;
    #else
        return dynamic_cast <DragAndDropTarget*> (currentlyOverComp.get());
    #endif // LE_PATCHED_JUCE
    }

    DragAndDropTarget* findTarget (Point<int> screenPos, Point<int>& relativePos,
                                   Component*& resultComponent) const
    {
    #ifdef LE_PATCHED_JUCE
        resultComponent = nullptr;
        return nullptr;
    #else
        Component* hit = getParentComponent();

        if (hit == nullptr)
            hit = Desktop::getInstance().findComponentAt (screenPos);
        else
            hit = hit->getComponentAt (hit->getLocalPoint (nullptr, screenPos));

        // (note: use a local copy of this in case the callback runs
        // a modal loop and deletes this object before the method completes)
        const DragAndDropTarget::SourceDetails details (sourceDetails);

        while (hit != nullptr)
        {
            if (DragAndDropTarget* const ddt = dynamic_cast <DragAndDropTarget*> (hit))
            {
                if (ddt->isInterestedInDragSource (details))
                {
                    relativePos = hit->getLocalPoint (nullptr, screenPos);
                    resultComponent = hit;
                    return ddt;
                }
            }

            hit = hit->getParentComponent();
        }

        resultComponent = nullptr;
        return nullptr;
    #endif // LE_PATCHED_JUCE
    }

    void setNewScreenPos (Point<int> screenPos)
    {
        Point<int> newPos (screenPos - imageOffset);

        if (Component* p = getParentComponent())
            newPos = p->getLocalPoint (nullptr, newPos);

        setTopLeftPosition (newPos);
    }

    void sendDragMove (DragAndDropTarget::SourceDetails& details) const
    {
        if (DragAndDropTarget* const target = getCurrentlyOver())
            if (target->isInterestedInDragSource (details))
                target->itemDragMove (details);
    }

    struct ExternalDragAndDropMessage  : public CallbackMessage
    {
        ExternalDragAndDropMessage (const StringArray& f, bool canMove)
            : files (f), canMoveFiles (canMove)
        {}

        void messageCallback() override
        {
            DragAndDropContainer::performExternalDragDropOfFiles (files, canMoveFiles);
        }

    private:
        StringArray files;
        bool canMoveFiles;
    };

    void checkForExternalDrag (DragAndDropTarget::SourceDetails& details, Point<int> screenPos)
    {
        LE_PATCH_UNREACHABLE_CODE
        if (! hasCheckedForExternalDrag)
        {
            if (Desktop::getInstance().findComponentAt (screenPos) == nullptr)
            {
                hasCheckedForExternalDrag = true;
                StringArray files;
                bool canMoveFiles = false;

                if (owner.shouldDropFilesWhenDraggedExternally (details, files, canMoveFiles)
                      && files.size() > 0
                      && ModifierKeys::getCurrentModifiersRealtime().isAnyMouseButtonDown())
                {
                    (new ExternalDragAndDropMessage (files, canMoveFiles))->post();
                    delete this;
                }
            }
        }
    }

    void dismissWithAnimation (const bool shouldSnapBack)
    {
        setVisible (true);
        ComponentAnimator& animator = Desktop::getInstance().getAnimator();

        if (shouldSnapBack && sourceDetails.sourceComponent != nullptr)
        {
            const Point<int> target (sourceDetails.sourceComponent->localPointToGlobal (sourceDetails.sourceComponent->getLocalBounds().getCentre()));
            const Point<int> ourCentre (localPointToGlobal (getLocalBounds().getCentre()));

            animator.animateComponent (this,
                                       getBounds() + (target - ourCentre),
                                       0.0f, 120,
                                       true, 1.0, 1.0);
        }
        else
        {
            animator.fadeOut (this, 120);
        }
    }

    JUCE_DECLARE_NON_COPYABLE (DragImageComponent)
};


//==============================================================================
DragAndDropContainer::DragAndDropContainer()
{
}
#ifndef LE_PATCHED_JUCE
DragAndDropContainer::~DragAndDropContainer()
{
    dragImageComponent = nullptr;
}
#endif // LE_PATCHED_JUCE

void DragAndDropContainer::startDragging (const var& sourceDescription,
                                          Component* sourceComponent,
                                          Image LE_PATCH( const & dragImageParam ) JUCE_ORIGINAL( dragImage ),
                                          const bool allowDraggingToExternalWindows,
                                          const Point<int>* imageOffsetFromMouse)
{
    if (dragImageComponent == nullptr)
    {
        MouseInputSource* const draggingSource = Desktop::getInstance().getDraggingMouseSource (0);

        if (draggingSource == nullptr || ! draggingSource->isDragging())
        {
            LE_PATCH_UNREACHABLE_CODE
            jassertfalse;   // You must call startDragging() from within a mouseDown or mouseDrag callback!
            return;
        }

        const Point<int> lastMouseDown (draggingSource->getLastMouseDownPosition());
        Point<int> imageOffset;
        
        LE_PATCH( Image dragImage; )
        if (LE_PATCH( dragImageParam ) JUCE_ORIGINAL( dragImage ).isNull())
        {
            dragImage = sourceComponent->createComponentSnapshot (sourceComponent->getLocalBounds())
                            .convertedToFormat (Image::ARGB);

            dragImage.multiplyAllAlphas (0.6f);

            const int lo = 150;
            const int hi = 400;

            Point<int> relPos (sourceComponent->getLocalPoint (nullptr, lastMouseDown));
            Point<int> clipped (dragImage.getBounds().getConstrainedPoint (relPos));
            Random random;

            for (int y = dragImage.getHeight(); --y >= 0;)
            {
                const double dy = (y - clipped.getY()) * (y - clipped.getY());

                for (int x = dragImage.getWidth(); --x >= 0;)
                {
                    const int dx = x - clipped.getX();
                    const int distance = roundToInt (std::sqrt (dx * dx + dy));

                    if (distance > lo)
                    {
                        const float alpha = (distance > hi) ? 0
                                                            : (hi - distance) / (float) (hi - lo)
                                                               + random.nextFloat() * 0.008f;

                        dragImage.multiplyAlphaAt (x, y, alpha);
                    }
                }
            }

            imageOffset = clipped;
        }
        else
        {
            LE_PATCH_UNREACHABLE_CODE
            LE_PATCH( dragImage = dragImageParam; )
            if (imageOffsetFromMouse == nullptr)
                imageOffset = dragImage.getBounds().getCentre();
            else
                imageOffset = dragImage.getBounds().getConstrainedPoint (-*imageOffsetFromMouse);
        }

        dragImageComponent = new DragImageComponent (dragImage, sourceDescription, sourceComponent,
                                                     draggingSource->getComponentUnderMouse(), *this, imageOffset);

        currentDragDesc = sourceDescription;

        if (allowDraggingToExternalWindows)
        {
            LE_PATCH_UNREACHABLE_CODE
            if (! Desktop::canUseSemiTransparentWindows())
                dragImageComponent->setOpaque (true);

            dragImageComponent->addToDesktop (ComponentPeer::windowIgnoresMouseClicks
                                               | ComponentPeer::windowIsTemporary
                                               | ComponentPeer::windowIgnoresKeyPresses);
        }
        else
        {
        #ifdef LE_PATCHED_JUCE
            Component * const thisComp( sourceComponent->getParentComponent() ); //...mrmlj...ugh...SW specific...
            if ( thisComp )
        #else
            if (Component* const thisComp = dynamic_cast <Component*> (this))
        #endif // LE_PATCHED_JUCE
            {
                thisComp->addChildComponent (dragImageComponent);
            }
            else
            {
                jassertfalse;   // Your DragAndDropContainer needs to be a Component!
                LE_PATCH_UNREACHABLE_CODE
                return;
            }
        }

        static_cast <DragImageComponent*> (dragImageComponent.get())->updateLocation (false, lastMouseDown);
        dragImageComponent->setVisible (true);

       #if JUCE_WINDOWS
        // Under heavy load, the layered window's paint callback can often be lost by the OS,
        // so forcing a repaint at least once makes sure that the window becomes visible..
        if (ComponentPeer* const peer = dragImageComponent->getPeer())
            peer->performAnyPendingRepaintsNow();
       #endif
    }
}

bool DragAndDropContainer::isDragAndDropActive() const
{
    return dragImageComponent != nullptr;
}

var LE_PATCH( const & ) DragAndDropContainer::getCurrentDragDescription() const
{
    return dragImageComponent != nullptr ? currentDragDesc
                                         : LE_PATCH( var::null ) JUCE_ORIGINAL( var() );
}

DragAndDropContainer* DragAndDropContainer::findParentDragContainerFor (Component* c)
{
    return c != nullptr ? c->findParentComponentOfClass<DragAndDropContainer>() : nullptr;
}

bool DragAndDropContainer::shouldDropFilesWhenDraggedExternally (const DragAndDropTarget::SourceDetails&, StringArray&, bool&)
{
    return false;
}

//==============================================================================
DragAndDropTarget::SourceDetails::SourceDetails (const var& desc, Component* comp, Point<int> pos) noexcept
    : description (desc),
      sourceComponent (comp),
      localPosition (pos)
{
}

void DragAndDropTarget::itemDragEnter (const SourceDetails&)  {}
void DragAndDropTarget::itemDragMove  (const SourceDetails&)  {}
void DragAndDropTarget::itemDragExit  (const SourceDetails&)  {}
bool DragAndDropTarget::shouldDrawDragImageWhenOver()         { return true; }

//==============================================================================
void FileDragAndDropTarget::fileDragEnter (const StringArray&, int, int)  {}
void FileDragAndDropTarget::fileDragMove  (const StringArray&, int, int)  {}
void FileDragAndDropTarget::fileDragExit  (const StringArray&)            {}

void TextDragAndDropTarget::textDragEnter (const String&, int, int)  {}
void TextDragAndDropTarget::textDragMove  (const String&, int, int)  {}
void TextDragAndDropTarget::textDragExit  (const String&)            {}
