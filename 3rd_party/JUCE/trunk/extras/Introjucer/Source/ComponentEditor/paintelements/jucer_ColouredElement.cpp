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

#include "../../jucer_Headers.h"
#include "jucer_ColouredElement.h"
#include "jucer_GradientPointComponent.h"
#include "../properties/jucer_PositionPropertyBase.h"
#include "../properties/jucer_ColourPropertyComponent.h"
#include "jucer_PaintElementUndoableAction.h"
#include "jucer_PaintElementPath.h"
#include "jucer_ImageResourceProperty.h"


//==============================================================================
class ElementFillModeProperty   : public ChoicePropertyComponent,
                                  private ElementListenerBase <ColouredElement>
{
public:
    ElementFillModeProperty (ColouredElement* const owner_, const bool isForStroke_)
        : ChoicePropertyComponent ("fill mode"),
          ElementListenerBase <ColouredElement> (owner_),
          isForStroke (isForStroke_)
    {
        choices.add ("Solid Colour");
        choices.add ("Linear Gradient");
        choices.add ("Radial Gradient");
        choices.add ("Image Brush");
    }

    void setIndex (int newIndex)
    {
        JucerFillType fill (isForStroke ? owner->getStrokeType().fill
                                        : owner->getFillType());

        switch (newIndex)
        {
            case 0:  fill.mode = JucerFillType::solidColour; break;
            case 1:  fill.mode = JucerFillType::linearGradient; break;
            case 2:  fill.mode = JucerFillType::radialGradient; break;
            case 3:  fill.mode = JucerFillType::imageBrush; break;
            default: jassertfalse; break;
        }

        if (! isForStroke)
            owner->setFillType (fill, true);
        else
            owner->setStrokeFill (fill, true);
    }

    int getIndex() const
    {
        switch (isForStroke ? owner->getStrokeType().fill.mode
                            : owner->getFillType().mode)
        {
            case JucerFillType::solidColour:    return 0;
            case JucerFillType::linearGradient: return 1;
            case JucerFillType::radialGradient: return 2;
            case JucerFillType::imageBrush:     return 3;
            default:                            jassertfalse; break;
        }

        return 0;
    }

private:
    const bool isForStroke;
};

//==============================================================================
class ElementFillColourProperty  : public JucerColourPropertyComponent,
                                   private ElementListenerBase <ColouredElement>
{
public:
    enum ColourType
    {
        solidColour,
        gradientColour1,
        gradientColour2
    };

    ElementFillColourProperty (const String& name,
                               ColouredElement* const owner_,
                               const ColourType type_,
                               const bool isForStroke_)
        : JucerColourPropertyComponent (name, false),
          ElementListenerBase <ColouredElement> (owner_),
          type (type_),
          isForStroke (isForStroke_)
    {
    }

    void setColour (const Colour& newColour)
    {
        owner->getDocument()->getUndoManager().undoCurrentTransactionOnly();

        JucerFillType fill (isForStroke ? owner->getStrokeType().fill
                                        : owner->getFillType());

        switch (type)
        {
            case solidColour:       fill.colour = newColour; break;
            case gradientColour1:   fill.gradCol1 = newColour; break;
            case gradientColour2:   fill.gradCol2 = newColour; break;
            default:                jassertfalse; break;
        }

        if (! isForStroke)
            owner->setFillType (fill, true);
        else
            owner->setStrokeFill (fill, true);
    }

    Colour getColour() const
    {
        const JucerFillType fill (isForStroke ? owner->getStrokeType().fill
                                              : owner->getFillType());

        switch (type)
        {
            case solidColour:       return fill.colour; break;
            case gradientColour1:   return fill.gradCol1; break;
            case gradientColour2:   return fill.gradCol2; break;
            default:                jassertfalse; break;
        }

        return Colours::black;
    }

    void resetToDefault()
    {
        jassertfalse; // option shouldn't be visible
    }

private:
    const ColourType type;
    const bool isForStroke;
};

//==============================================================================
class ElementFillPositionProperty   : public PositionPropertyBase,
                                      private ElementListenerBase <ColouredElement>
{
public:
    ElementFillPositionProperty (ColouredElement* const owner_,
                                 const String& name,
                                 ComponentPositionDimension dimension_,
                                 const bool isStart_,
                                 const bool isForStroke_)
     : PositionPropertyBase (owner_, name, dimension_, false, false,
                             owner_->getDocument()->getComponentLayout()),
       ElementListenerBase <ColouredElement> (owner_),
       isStart (isStart_),
       isForStroke (isForStroke_)
    {
    }

    void setPosition (const RelativePositionedRectangle& newPos)
    {
        JucerFillType fill (isForStroke ? owner->getStrokeType().fill
                                        : owner->getFillType());

        if (isStart)
            fill.gradPos1 = newPos;
        else
            fill.gradPos2 = newPos;

        if (! isForStroke)
            owner->setFillType (fill, true);
        else
            owner->setStrokeFill (fill, true);
    }

    RelativePositionedRectangle getPosition() const
    {
        const JucerFillType fill (isForStroke ? owner->getStrokeType().fill
                                              : owner->getFillType());

        return isStart ? fill.gradPos1
                       : fill.gradPos2;
    }

private:
    const bool isStart, isForStroke;
};

//==============================================================================
class EnableStrokeProperty : public BooleanPropertyComponent,
                             private ElementListenerBase <ColouredElement>
{
public:
    EnableStrokeProperty (ColouredElement* const owner_)
        : BooleanPropertyComponent ("outline", "Outline enabled", "No outline"),
          ElementListenerBase <ColouredElement> (owner_)
    {
    }

    //==============================================================================
    void setState (bool newState)           { owner->enableStroke (newState, true); }
    bool getState() const                   { return owner->isStrokeEnabled(); }
};

//==============================================================================
class StrokeThicknessProperty   : public SliderPropertyComponent,
                                  private ElementListenerBase <ColouredElement>
{
public:
    StrokeThicknessProperty (ColouredElement* const owner_)
        : SliderPropertyComponent ("outline thickness", 0.1, 200.0, 0.1, 0.3),
          ElementListenerBase <ColouredElement> (owner_)
    {
    }

    void setValue (double newValue)
    {
        owner->getDocument()->getUndoManager().undoCurrentTransactionOnly();

        owner->setStrokeType (PathStrokeType ((float) newValue,
                                              owner->getStrokeType().stroke.getJointStyle(),
                                              owner->getStrokeType().stroke.getEndStyle()),
                              true);
    }

    double getValue() const                 { return owner->getStrokeType().stroke.getStrokeThickness(); }
};

//==============================================================================
class StrokeJointProperty : public ChoicePropertyComponent,
                            private ElementListenerBase <ColouredElement>
{
public:
    StrokeJointProperty (ColouredElement* const owner_)
        : ChoicePropertyComponent ("joint style"),
          ElementListenerBase <ColouredElement> (owner_)
    {
        choices.add ("mitered");
        choices.add ("curved");
        choices.add ("beveled");
    }

    void setIndex (int newIndex)
    {
        const PathStrokeType::JointStyle joints[] = { PathStrokeType::mitered,
                                                      PathStrokeType::curved,
                                                      PathStrokeType::beveled };

        jassert (newIndex >= 0 && newIndex < 3);

        owner->setStrokeType (PathStrokeType (owner->getStrokeType().stroke.getStrokeThickness(),
                                              joints [newIndex],
                                              owner->getStrokeType().stroke.getEndStyle()),
                              true);
    }

    int getIndex() const
    {
        switch (owner->getStrokeType().stroke.getJointStyle())
        {
            case PathStrokeType::mitered:   return 0;
            case PathStrokeType::curved:    return 1;
            case PathStrokeType::beveled:   return 2;
            default:                        jassertfalse; break;
        }

        return 0;
    }
};

//==============================================================================
class StrokeEndCapProperty   : public ChoicePropertyComponent,
                               private ElementListenerBase <ColouredElement>
{
public:
    StrokeEndCapProperty (ColouredElement* const owner_)
        : ChoicePropertyComponent ("end-cap style"),
          ElementListenerBase <ColouredElement> (owner_)
    {
        choices.add ("butt");
        choices.add ("square");
        choices.add ("round");
    }

    void setIndex (int newIndex)
    {
        const PathStrokeType::EndCapStyle ends[] = { PathStrokeType::butt,
                                                     PathStrokeType::square,
                                                     PathStrokeType::rounded };

        jassert (newIndex >= 0 && newIndex < 3);

        owner->setStrokeType (PathStrokeType (owner->getStrokeType().stroke.getStrokeThickness(),
                                              owner->getStrokeType().stroke.getJointStyle(),
                                              ends [newIndex]),
                              true);
    }

    int getIndex() const
    {
        switch (owner->getStrokeType().stroke.getEndStyle())
        {
            case PathStrokeType::butt:      return 0;
            case PathStrokeType::square:    return 1;
            case PathStrokeType::rounded:   return 2;
            default:                        jassertfalse; break;
        }

        return 0;
    }
};

//==============================================================================
class ImageBrushResourceProperty    : public ImageResourceProperty <ColouredElement>
{
public:
    ImageBrushResourceProperty (ColouredElement* const e, const bool isForStroke_)
        : ImageResourceProperty <ColouredElement> (e, isForStroke_ ? "stroke image"
                                                                   : "fill image"),
          isForStroke (isForStroke_)
    {
    }

    //==============================================================================
    void setResource (const String& newName)
    {
        if (element != nullptr)
        {
            if (isForStroke)
            {
                JucerFillType type (element->getStrokeType().fill);
                type.imageResourceName = newName;

                element->setStrokeFill (type, true);
            }
            else
            {
                JucerFillType type (element->getFillType());
                type.imageResourceName = newName;

                element->setFillType (type, true);
            }
        }
    }

    String getResource() const
    {
        if (element == nullptr)
            return String::empty;

        if (isForStroke)
            return element->getStrokeType().fill.imageResourceName;

        return element->getFillType().imageResourceName;
    }

private:
    bool isForStroke;
};

//==============================================================================
class ImageBrushPositionProperty    : public PositionPropertyBase,
                                      private ElementListenerBase <ColouredElement>
{
public:
    ImageBrushPositionProperty (ColouredElement* const owner_,
                                const String& name,
                                ComponentPositionDimension dimension_,
                                const bool isForStroke_)
        : PositionPropertyBase (owner_, name, dimension_, false, false,
                                owner_->getDocument()->getComponentLayout()),
          ElementListenerBase <ColouredElement> (owner_),
          isForStroke (isForStroke_)
    {
    }

    void setPosition (const RelativePositionedRectangle& newPos)
    {
        if (isForStroke)
        {
            JucerFillType type (owner->getStrokeType().fill);
            type.imageAnchor = newPos;
            owner->setStrokeFill (type, true);
        }
        else
        {
            JucerFillType type (owner->getFillType());
            type.imageAnchor = newPos;
            owner->setFillType (type, true);
        }
    }

    RelativePositionedRectangle getPosition() const
    {
        if (isForStroke)
            return owner->getStrokeType().fill.imageAnchor;

        return owner->getFillType().imageAnchor;
    }

private:
    const bool isForStroke;
};

//==============================================================================
class ImageBrushOpacityProperty  : public SliderPropertyComponent,
                                   private ElementListenerBase <ColouredElement>
{
public:
    ImageBrushOpacityProperty (ColouredElement* const e, const bool isForStroke_)
        : SliderPropertyComponent ("opacity", 0.0, 1.0, 0.001),
          ElementListenerBase <ColouredElement> (e),
          isForStroke (isForStroke_)
    {
    }

    void setValue (double newValue)
    {
        if (owner != nullptr)
        {
            owner->getDocument()->getUndoManager().undoCurrentTransactionOnly();

            if (isForStroke)
            {
                JucerFillType type (owner->getStrokeType().fill);
                type.imageOpacity = newValue;

                owner->setStrokeFill (type, true);
            }
            else
            {
                JucerFillType type (owner->getFillType());
                type.imageOpacity = newValue;

                owner->setFillType (type, true);
            }
        }
    }

    double getValue() const
    {
        if (owner == nullptr)
            return 0;

        if (isForStroke)
            return owner->getStrokeType().fill.imageOpacity;

        return owner->getFillType().imageOpacity;
    }

private:
    bool isForStroke;
};


//==============================================================================
ColouredElement::ColouredElement (PaintRoutine* owner_,
                                  const String& name,
                                  const bool showOutline_,
                                  const bool showJointAndEnd_)
    : PaintElement (owner_, name),
      isStrokePresent (false),
      showOutline (showOutline_),
      showJointAndEnd (showJointAndEnd_)
{
}

ColouredElement::~ColouredElement()
{
}

//==============================================================================
void ColouredElement::getEditableProperties (Array <PropertyComponent*>& properties)
{
    PaintElement::getEditableProperties (properties);
    getColourSpecificProperties (properties);
}

void ColouredElement::getColourSpecificProperties (Array <PropertyComponent*>& properties)
{
    properties.add (new ElementFillModeProperty (this, false));

    switch (getFillType().mode)
    {
    case JucerFillType::solidColour:
        properties.add (new ElementFillColourProperty ("colour", this, ElementFillColourProperty::solidColour, false));
        break;

    case JucerFillType::linearGradient:
    case JucerFillType::radialGradient:
        properties.add (new ElementFillColourProperty ("colour 1", this, ElementFillColourProperty::gradientColour1, false));
        properties.add (new ElementFillPositionProperty (this, "x1", PositionPropertyBase::componentX, true, false));
        properties.add (new ElementFillPositionProperty (this, "y1", PositionPropertyBase::componentY, true, false));
        properties.add (new ElementFillColourProperty ("colour 2", this, ElementFillColourProperty::gradientColour2, false));
        properties.add (new ElementFillPositionProperty (this, "x2", PositionPropertyBase::componentX, false, false));
        properties.add (new ElementFillPositionProperty (this, "y2", PositionPropertyBase::componentY, false, false));
        break;

    case JucerFillType::imageBrush:
        properties.add (new ImageBrushResourceProperty (this, false));
        properties.add (new ImageBrushPositionProperty (this, "anchor x", PositionPropertyBase::componentX, false));
        properties.add (new ImageBrushPositionProperty (this, "anchor y", PositionPropertyBase::componentY, false));
        properties.add (new ImageBrushOpacityProperty (this, false));
        break;

    default:
        jassertfalse;
        break;
    }

    if (showOutline)
    {
        properties.add (new EnableStrokeProperty (this));

        if (isStrokePresent)
        {
            properties.add (new StrokeThicknessProperty (this));

            if (showJointAndEnd)
            {
                properties.add (new StrokeJointProperty (this));
                properties.add (new StrokeEndCapProperty (this));
            }

            properties.add (new ElementFillModeProperty (this, true));

            switch (getStrokeType().fill.mode)
            {
                case JucerFillType::solidColour:
                    properties.add (new ElementFillColourProperty ("colour", this, ElementFillColourProperty::solidColour, true));
                    break;

                case JucerFillType::linearGradient:
                case JucerFillType::radialGradient:
                    properties.add (new ElementFillColourProperty ("colour 1", this, ElementFillColourProperty::gradientColour1, true));
                    properties.add (new ElementFillPositionProperty (this, "x1", PositionPropertyBase::componentX, true, true));
                    properties.add (new ElementFillPositionProperty (this, "y1", PositionPropertyBase::componentY, true, true));
                    properties.add (new ElementFillColourProperty ("colour 2", this, ElementFillColourProperty::gradientColour2, true));
                    properties.add (new ElementFillPositionProperty (this, "x2", PositionPropertyBase::componentX, false, true));
                    properties.add (new ElementFillPositionProperty (this, "y2", PositionPropertyBase::componentY, false, true));
                    break;

                case JucerFillType::imageBrush:
                    properties.add (new ImageBrushResourceProperty (this, true));
                    properties.add (new ImageBrushPositionProperty (this, "stroke anchor x", PositionPropertyBase::componentX, true));
                    properties.add (new ImageBrushPositionProperty (this, "stroke anchor y", PositionPropertyBase::componentY, true));
                    properties.add (new ImageBrushOpacityProperty (this, true));
                    break;

                default:
                    jassertfalse;
                    break;
            }
        }
    }
}

//==============================================================================
const JucerFillType& ColouredElement::getFillType() noexcept
{
    return fillType;
}

class FillTypeChangeAction  : public PaintElementUndoableAction <ColouredElement>
{
public:
    FillTypeChangeAction (ColouredElement* const element, const JucerFillType& newState_)
        : PaintElementUndoableAction <ColouredElement> (element),
          newState (newState_)
    {
        oldState = element->getFillType();
    }

    bool perform()
    {
        showCorrectTab();
        getElement()->setFillType (newState, false);
        return true;
    }

    bool undo()
    {
        showCorrectTab();
        getElement()->setFillType (oldState, false);
        return true;
    }

private:
    JucerFillType newState, oldState;
};

void ColouredElement::setFillType (const JucerFillType& newType, const bool undoable)
{
    if (fillType != newType)
    {
        if (undoable)
        {
            perform (new FillTypeChangeAction (this, newType),
                     "Change fill type");
        }
        else
        {
            repaint();

            if (fillType.mode != newType.mode)
            {
                owner->getSelectedElements().changed();
                siblingComponentsChanged();
            }

            fillType = newType;
            changed();
        }
    }
}

//==============================================================================
bool ColouredElement::isStrokeEnabled() const noexcept
{
    return isStrokePresent && showOutline;
}

class StrokeEnableChangeAction  : public PaintElementUndoableAction <ColouredElement>
{
public:
    StrokeEnableChangeAction (ColouredElement* const element, const bool newState_)
        : PaintElementUndoableAction <ColouredElement> (element),
          newState (newState_)
    {
        oldState = element->isStrokeEnabled();
    }

    bool perform()
    {
        showCorrectTab();
        getElement()->enableStroke (newState, false);
        return true;
    }

    bool undo()
    {
        showCorrectTab();
        getElement()->enableStroke (oldState, false);
        return true;
    }

private:
    bool newState, oldState;
};

void ColouredElement::enableStroke (bool enable, const bool undoable)
{
    enable = enable && showOutline;

    if (isStrokePresent != enable)
    {
        if (undoable)
        {
            perform (new StrokeEnableChangeAction (this, enable),
                     "Change stroke mode");
        }
        else
        {
            repaint();
            isStrokePresent = enable;

            siblingComponentsChanged();
            owner->changed();
            owner->getSelectedElements().changed();
        }
    }
}

//==============================================================================
const StrokeType& ColouredElement::getStrokeType() noexcept
{
    return strokeType;
}

class StrokeTypeChangeAction  : public PaintElementUndoableAction <ColouredElement>
{
public:
    StrokeTypeChangeAction (ColouredElement* const element, const PathStrokeType& newState_)
        : PaintElementUndoableAction <ColouredElement> (element),
          newState (newState_),
          oldState (element->getStrokeType().stroke)
    {
    }

    bool perform()
    {
        showCorrectTab();
        getElement()->setStrokeType (newState, false);
        return true;
    }

    bool undo()
    {
        showCorrectTab();
        getElement()->setStrokeType (oldState, false);
        return true;
    }

private:
    PathStrokeType newState, oldState;
};

void ColouredElement::setStrokeType (const PathStrokeType& newType, const bool undoable)
{
    if (strokeType.stroke != newType)
    {
        if (undoable)
        {
            perform (new StrokeTypeChangeAction (this, newType),
                     "Change stroke type");
        }
        else
        {
            repaint();
            strokeType.stroke = newType;
            changed();
        }
    }
}

class StrokeFillTypeChangeAction  : public PaintElementUndoableAction <ColouredElement>
{
public:
    StrokeFillTypeChangeAction (ColouredElement* const element, const JucerFillType& newState_)
        : PaintElementUndoableAction <ColouredElement> (element),
          newState (newState_)
    {
        oldState = element->getStrokeType().fill;
    }

    bool perform()
    {
        showCorrectTab();
        getElement()->setStrokeFill (newState, false);
        return true;
    }

    bool undo()
    {
        showCorrectTab();
        getElement()->setStrokeFill (oldState, false);
        return true;
    }

private:
    JucerFillType newState, oldState;
};

void ColouredElement::setStrokeFill (const JucerFillType& newType, const bool undoable)
{
    if (strokeType.fill != newType)
    {
        if (undoable)
        {
            perform (new StrokeFillTypeChangeAction (this, newType),
                     "Change stroke fill type");
        }
        else
        {
            repaint();

            if (strokeType.fill.mode != newType.mode)
            {
                siblingComponentsChanged();
                owner->getSelectedElements().changed();
            }

            strokeType.fill = newType;
            changed();
        }
    }
}

//==============================================================================
void ColouredElement::createSiblingComponents()
{
    {
        GradientPointComponent* g1 = new GradientPointComponent (this, false, true);
        siblingComponents.add (g1);

        GradientPointComponent* g2 = new GradientPointComponent (this, false, false);
        siblingComponents.add (g2);

        getParentComponent()->addAndMakeVisible (g1);
        getParentComponent()->addAndMakeVisible (g2);

        g1->updatePosition();
        g2->updatePosition();
    }

    if (isStrokePresent && showOutline)
    {
        GradientPointComponent* g1 = new GradientPointComponent (this, true, true);
        siblingComponents.add (g1);

        GradientPointComponent* g2 = new GradientPointComponent (this, true, false);
        siblingComponents.add (g2);

        getParentComponent()->addAndMakeVisible (g1);
        getParentComponent()->addAndMakeVisible (g2);

        g1->updatePosition();
        g2->updatePosition();
    }
}

Rectangle<int> ColouredElement::getCurrentBounds (const Rectangle<int>& parentArea) const
{
    int border = 0;

    if (isStrokePresent)
        border = (int) strokeType.stroke.getStrokeThickness() / 2 + 1;

    return position.getRectangle (parentArea, getDocument()->getComponentLayout())
                   .expanded (border, border);
}

void ColouredElement::setCurrentBounds (const Rectangle<int>& newBounds,
                                        const Rectangle<int>& parentArea,
                                        const bool undoable)
{
    Rectangle<int> r (newBounds);

    if (isStrokePresent)
    {
        const int border = (int) strokeType.stroke.getStrokeThickness() / 2 + 1;
        r = r.expanded (-border, -border);

        r.setSize (jmax (1, r.getWidth()), jmax (1, r.getHeight()));
    }

    RelativePositionedRectangle pr (position);
    pr.updateFrom (r.getX() - parentArea.getX(),
                   r.getY() - parentArea.getY(),
                   r.getWidth(), r.getHeight(),
                   Rectangle<int> (0, 0, parentArea.getWidth(), parentArea.getHeight()),
                   getDocument()->getComponentLayout());
    setPosition (pr, undoable);

    updateBounds (parentArea);
}

//==============================================================================
void ColouredElement::addColourAttributes (XmlElement* const e) const
{
    e->setAttribute ("fill", fillType.toString());
    e->setAttribute ("hasStroke", isStrokePresent);

    if (isStrokePresent && showOutline)
    {
        e->setAttribute ("stroke", strokeType.toString());
        e->setAttribute ("strokeColour", strokeType.fill.toString());
    }
}

bool ColouredElement::loadColourAttributes (const XmlElement& xml)
{
    fillType.restoreFromString (xml.getStringAttribute ("fill", String::empty));

    isStrokePresent = showOutline && xml.getBoolAttribute ("hasStroke", false);

    strokeType.restoreFromString (xml.getStringAttribute ("stroke", String::empty));
    strokeType.fill.restoreFromString (xml.getStringAttribute ("strokeColour", String::empty));

    return true;
}

//==============================================================================
void ColouredElement::convertToNewPathElement (const Path& path)
{
    if (! path.isEmpty())
    {
        PaintElementPath newElement (getOwner());
        newElement.setToPath (path);
        newElement.setFillType (fillType, false);
        newElement.enableStroke (isStrokeEnabled(), false);
        newElement.setStrokeType (getStrokeType().stroke, false);
        newElement.setStrokeFill (getStrokeType().fill, false);

        ScopedPointer<XmlElement> xml (newElement.createXml());

        PaintElement* e = getOwner()->addElementFromXml (*xml, getOwner()->indexOfElement (this), true);

        getOwner()->getSelectedElements().selectOnly (e);
        getOwner()->removeElement (this, true);
    }
}
