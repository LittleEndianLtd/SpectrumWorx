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

class Slider::Pimpl   : public AsyncUpdater,
                    #ifndef LE_PATCHED_JUCE
                        public ButtonListener,  // (can't use Button::Listener due to idiotic VC2005 bug)
                        public LabelListener,
                    #endif // LE_PATCHED_JUCE
                        public ValueListener
{
public:
    Pimpl (Slider& s, SliderStyle sliderStyle, TextEntryBoxPosition textBoxPosition)
      : owner (s),
    #ifdef LE_PATCHED_JUCE
        pListener   ( nullptr ),
        currentValue( 0.0     ),
    #endif // LE_PATCHED_JUCE
        style (sliderStyle),
        lastCurrentValue (0), lastValueMin (0), lastValueMax (0),
        minimum (0), maximum (10), interval (0), doubleClickReturnValue (0),
        skewFactor (1.0), velocityModeSensitivity (1.0),
        velocityModeOffset (0.0), velocityModeThreshold (1),
        rotaryStart (float_Pi * 1.2f),
        rotaryEnd (float_Pi * 2.8f),
        sliderRegionStart (0), sliderRegionSize (1), sliderBeingDragged (-1),
        pixelsForFullDragExtent (250),
        textBoxPos (textBoxPosition),
        numDecimalPlaces (7),
        textBoxWidth (80), textBoxHeight (20),
        incDecButtonMode (incDecButtonsNotDraggable),
        editableText (true),
        doubleClickToValue (false),
        isVelocityBased (false),
        userKeyOverridesVelocity (true),
        rotaryStop (true),
        incDecButtonsSideBySide (false),
        sendChangeOnlyOnRelease (false),
        popupDisplayEnabled (false),
        menuEnabled (false),
        useDragEvents (false),
        scrollWheelEnabled (true),
        snapsToMousePos (true)/*,*/
    #ifndef LE_PATCHED_JUCE
        ,parentForPopupDisplay (nullptr)
    #endif // LE_PATCHED_JUCE
    {
    }

    ~Pimpl()
    {
        currentValue.removeListener (this);
        valueMin.removeListener (this);
        valueMax.removeListener (this);
        JUCE_ORIGINAL( popupDisplay = nullptr; )
    }

    //==============================================================================
    void registerListeners()
    {
        currentValue.addListener (this);
        valueMin.addListener (this);
        valueMax.addListener (this);
    }

    bool isHorizontal() const noexcept
    {
        return style == LinearHorizontal
            || style == LinearBar
            || style == TwoValueHorizontal
            || style == ThreeValueHorizontal;
    }

    bool isVertical() const noexcept
    {
        return style == LinearVertical
            || style == LinearBarVertical
            || style == TwoValueVertical
            || style == ThreeValueVertical;
    }

    bool isRotary() const noexcept
    {
        return style == Rotary
            || style == RotaryHorizontalDrag
            || style == RotaryVerticalDrag
            || style == RotaryHorizontalVerticalDrag;
    }

    bool incDecDragDirectionIsHorizontal() const noexcept
    {
        return incDecButtonMode == incDecButtonsDraggable_Horizontal
                || (incDecButtonMode == incDecButtonsDraggable_AutoDirection && incDecButtonsSideBySide);
    }

    float getPositionOfValue (const double value) const
    {
        JUCE_ORIGINAL( if (isHorizontal() || isVertical()) )
        LE_PATCH( jassert( (isHorizontal() || isVertical()) ); )
            return getLinearSliderPos (value);

    #ifndef LE_PATCHED_JUCE
        jassertfalse; // not a valid call on a slider that doesn't work linearly!
        return 0.0f;
    #endif // LE_PATCHED_JUCE
    }

    void setRange (const double newMin, const double newMax, const double newInt)
    {
        if (minimum != newMin || maximum != newMax || interval != newInt)
        {
            minimum = newMin;
            maximum = newMax;
            interval = newInt;

            // figure out the number of DPs needed to display all values at this
            // interval setting.
            numDecimalPlaces = 7;

            if (newInt != 0)
            {
                int v = abs ((int) (newInt * 10000000));

                while ((v % 10) == 0)
                {
                    --numDecimalPlaces;
                    v /= 10;
                }
            }

            // keep the current values inside the new range..
            if (style != TwoValueHorizontal && style != TwoValueVertical)
            {
                setValue (getValue(), dontSendNotification);
            }
            else
            {
                setMinValue (getMinValue(), dontSendNotification, false);
                setMaxValue (getMaxValue(), dontSendNotification, false);
            }

            updateText();
        }
    }

    double  LE_PATCH( const & ) getValue() const LE_PATCH( noexcept )
    {
        // for a two-value style slider, you should use the getMinValue() and getMaxValue()
        // methods to get the two values.
        jassert (style != TwoValueHorizontal && style != TwoValueVertical);

    #ifdef LE_PATCHED_JUCE
        jassert( currentValue.getValue().isDouble() );
        struct LE_PATCH_NOVTABLE SimpleValueSource : Value::ValueSource { var value; };
        double const & result( static_cast<SimpleValueSource const &>( const_cast<Value &>( currentValue ).getValueSource() ).value.value.doubleValue );
        jassert( result == static_cast<double>( currentValue.getValue() ) );
        return result;
    #else
        return currentValue.getValue();
    #endif // LE_PATCHED_JUCE
    }

    void setValue (double newValue, const NotificationType notification)
    {
        // for a two-value style slider, you should use the setMinValue() and setMaxValue()
        // methods to set the two values.
        jassert (style != TwoValueHorizontal && style != TwoValueVertical);

        newValue = constrainedValue (newValue);

        if (style == ThreeValueHorizontal || style == ThreeValueVertical)
        {
            LE_PATCH_UNREACHABLE_CODE
            jassert ((double) valueMin.getValue() <= (double) valueMax.getValue());

            newValue = jlimit ((double) valueMin.getValue(),
                               (double) valueMax.getValue(),
                               newValue);
        }

        if (newValue != lastCurrentValue)
        {
        #ifndef LE_PATCHED_JUCE
            if (valueBox != nullptr)
                valueBox->hideEditor (true);
        #endif // LE_PATCHED_JUCE

            lastCurrentValue = newValue;

            // (need to do this comparison because the Value will use equalsWithSameType to compare
            // the new and old values, so will generate unwanted change events if the type changes)
            if (currentValue != newValue)
                currentValue = newValue;

            updateText();
            owner.repaint();

        #ifndef LE_PATCHED_JUCE
            if (popupDisplay != nullptr)
                popupDisplay->updatePosition (getTextFromValue (newValue));
        #endif // LE_PATCHED_JUCE

            triggerChangeMessage (notification);
        }
    }

    void setMinValue (double newValue, const NotificationType notification,
                      const bool allowNudgingOfOtherValues)
    {
        // The minimum value only applies to sliders that are in two- or three-value mode.
        jassert (style == TwoValueHorizontal || style == TwoValueVertical
                  || style == ThreeValueHorizontal || style == ThreeValueVertical);

        newValue = constrainedValue (newValue);

        if (style == TwoValueHorizontal || style == TwoValueVertical)
        {
            if (allowNudgingOfOtherValues && newValue > (double) valueMax.getValue())
                setMaxValue (newValue, notification, false);

            newValue = jmin ((double) valueMax.getValue(), newValue);
        }
        else
        {
            if (allowNudgingOfOtherValues && newValue > lastCurrentValue)
                setValue (newValue, notification);

            newValue = jmin (lastCurrentValue, newValue);
        }

        if (lastValueMin != newValue)
        {
            lastValueMin = newValue;
            valueMin = newValue;
            owner.repaint();

        #ifndef LE_PATCHED_JUCE
            if (popupDisplay != nullptr)
                popupDisplay->updatePosition (getTextFromValue (newValue));
        #endif // LE_PATCHED_JUCE

            triggerChangeMessage (notification);
        }
    }

    void setMaxValue (double newValue, const NotificationType notification,
                      const bool allowNudgingOfOtherValues)
    {
        // The maximum value only applies to sliders that are in two- or three-value mode.
        jassert (style == TwoValueHorizontal || style == TwoValueVertical
                  || style == ThreeValueHorizontal || style == ThreeValueVertical);

        newValue = constrainedValue (newValue);

        if (style == TwoValueHorizontal || style == TwoValueVertical)
        {
            if (allowNudgingOfOtherValues && newValue < (double) valueMin.getValue())
                setMinValue (newValue, notification, false);

            newValue = jmax ((double) valueMin.getValue(), newValue);
        }
        else
        {
            if (allowNudgingOfOtherValues && newValue < lastCurrentValue)
                setValue (newValue, notification);

            newValue = jmax (lastCurrentValue, newValue);
        }

        if (lastValueMax != newValue)
        {
            lastValueMax = newValue;
            valueMax = newValue;
            owner.repaint();

        #ifndef LE_PATCHED_JUCE
            if (popupDisplay != nullptr)
                popupDisplay->updatePosition (getTextFromValue (valueMax.getValue()));
        #endif // LE_PATCHED_JUCE

            triggerChangeMessage (notification);
        }
    }

    void setMinAndMaxValues (double newMinValue, double newMaxValue, const NotificationType notification)
    {
        // The maximum value only applies to sliders that are in two- or three-value mode.
        jassert (style == TwoValueHorizontal || style == TwoValueVertical
                  || style == ThreeValueHorizontal || style == ThreeValueVertical);

        if (newMaxValue < newMinValue)
            std::swap (newMaxValue, newMinValue);

        newMinValue = constrainedValue (newMinValue);
        newMaxValue = constrainedValue (newMaxValue);

        if (lastValueMax != newMaxValue || lastValueMin != newMinValue)
        {
            lastValueMax = newMaxValue;
            lastValueMin = newMinValue;
            valueMin = newMinValue;
            valueMax = newMaxValue;
            owner.repaint();

            triggerChangeMessage (notification);
        }
    }

    double getMinValue() const
    {
        // The minimum value only applies to sliders that are in two- or three-value mode.
        jassert (style == TwoValueHorizontal || style == TwoValueVertical
                  || style == ThreeValueHorizontal || style == ThreeValueVertical);

        return valueMin.getValue();
    }

    double getMaxValue() const
    {
        // The maximum value only applies to sliders that are in two- or three-value mode.
        jassert (style == TwoValueHorizontal || style == TwoValueVertical
                  || style == ThreeValueHorizontal || style == ThreeValueVertical);

        return valueMax.getValue();
    }

    void triggerChangeMessage (const NotificationType notification)
    {
        if (notification != dontSendNotification)
        {
            if (notification == sendNotificationSync)
                handleAsyncUpdate();
            else
                triggerAsyncUpdate();

            owner.valueChanged();
        }
    }

    void handleAsyncUpdate() override
    {
        cancelPendingUpdate();

    #ifdef LE_PATCHED_JUCE
        if ( pListener ) pListener->sliderValueChanged( &owner );
    #else
        Component::BailOutChecker checker (&owner);
        Slider* slider = &owner; // (must use an intermediate variable here to avoid a VS2005 compiler bug)
        listeners.callChecked (checker, &SliderListener::sliderValueChanged, slider);  // (can't use Slider::Listener due to idiotic VC2005 bug)
    #endif // LE_PATCHED_JUCE
    }

    void sendDragStart()
    {
        owner.startedDragging();

    #ifdef LE_PATCHED_JUCE
        if ( pListener ) pListener->sliderDragStarted( &owner );
    #else
        Component::BailOutChecker checker (&owner);
        Slider* slider = &owner; // (must use an intermediate variable here to avoid a VS2005 compiler bug)
        listeners.callChecked (checker, &SliderListener::sliderDragStarted, slider);
    #endif // LE_PATCHED_JUCE
    }

    void sendDragEnd()
    {
        owner.stoppedDragging();

        LE_PATCH( if ( &owner != getCurrentlyFocusedComponent() ) /*for FrequencyRange*/)
        sliderBeingDragged = -1;

    #ifdef LE_PATCHED_JUCE
        if ( pListener ) pListener->sliderDragEnded( &owner );
    #else
        Component::BailOutChecker checker (&owner);
        Slider* slider = &owner; // (must use an intermediate variable here to avoid a VS2005 compiler bug)
        listeners.callChecked (checker, &SliderListener::sliderDragEnded, slider);
    #endif // LE_PATCHED_JUCE
    }

    struct DragInProgress
    {
        DragInProgress (Pimpl& p)  : owner (p)      { owner.sendDragStart(); }
        ~DragInProgress()                           { owner.sendDragEnd(); }

        Pimpl& owner;

        JUCE_DECLARE_NON_COPYABLE (DragInProgress)
    };

#ifndef LE_PATCHED_JUCE
    void buttonClicked (Button* button) override
    {
        if (style == IncDecButtons)
        {
            const double delta = (button == incButton) ? interval : -interval;

            DragInProgress drag (*this);
            setValue (owner.snapValue (getValue() + delta, false), sendNotificationSync);
        }
    }
#endif // LE_PATCHED_JUCE

    void valueChanged (Value& value) LE_PATCH( noexcept ) override
    {
        if (value.refersToSameSourceAs (currentValue))
        {
            if (style != TwoValueHorizontal && style != TwoValueVertical)
                setValue (currentValue.getValue(), dontSendNotification);
        }
        else if (value.refersToSameSourceAs (valueMin))
            setMinValue (valueMin.getValue(), dontSendNotification, true);
        else if (value.refersToSameSourceAs (valueMax))
            setMaxValue (valueMax.getValue(), dontSendNotification, true);
    }

    void labelTextChanged (Label* label) JUCE_ORIGINAL( override )
    {
        const double newValue = owner.snapValue (owner.getValueFromText (label->getText()), false);

        if (newValue != (double) currentValue.getValue())
        {
            DragInProgress drag (*this);
            setValue (newValue, sendNotificationSync);
        }

        updateText(); // force a clean-up of the text, needed in case setValue() hasn't done this.
    }

    void updateText()
    {
    #ifndef LE_PATCHED_JUCE
        if (valueBox != nullptr)
            valueBox->setText (owner.getTextFromValue (currentValue.getValue()), dontSendNotification);
    #endif // LE_PATCHED_JUCE
    }

    double constrainedValue (double value) const
    {
        if (interval > 0)
            value = minimum + interval * std::floor ((value - minimum) / interval + 0.5);

        if (value <= minimum || maximum <= minimum)
            value = minimum;
        else if (value >= maximum)
            value = maximum;

        return value;
    }

    float getLinearSliderPos (const double value) const
    {
        double pos;

        if (maximum > minimum)
        {
            if (value < minimum)
            {
                pos = 0.0;
            }
            else if (value > maximum)
            {
                pos = 1.0;
            }
            else
            {
                pos = owner.valueToProportionOfLength (value);
                jassert (pos >= 0 && pos <= 1.0);
            }
        }
        else
        {
            pos = 0.5;
        }

        if (isVertical() || style == IncDecButtons)
            pos = 1.0 - pos;

        return (float) (sliderRegionStart + pos * sliderRegionSize);
    }

    void setSliderStyle (const SliderStyle newStyle)
    {
        if (style != newStyle)
        {
            style = newStyle;
            owner.repaint();
            owner.lookAndFeelChanged();
        }
    }

    void setRotaryParameters (const float startAngleRadians,
                              const float endAngleRadians,
                              const bool stopAtEnd)
    {
        // make sure the values are sensible..
        jassert (rotaryStart >= 0 && rotaryEnd >= 0);
        jassert (rotaryStart < float_Pi * 4.0f && rotaryEnd < float_Pi * 4.0f);
        jassert (rotaryStart < rotaryEnd);

        rotaryStart = startAngleRadians;
        rotaryEnd = endAngleRadians;
        rotaryStop = stopAtEnd;
    }

    void setVelocityModeParameters (const double sensitivity, const int threshold,
                                    const double offset, const bool userCanPressKeyToSwapMode)
    {
        velocityModeSensitivity = sensitivity;
        velocityModeOffset = offset;
        velocityModeThreshold = threshold;
        userKeyOverridesVelocity = userCanPressKeyToSwapMode;
    }

    void setSkewFactorFromMidPoint (const double sliderValueToShowAtMidPoint)
    {
        if (maximum > minimum)
            skewFactor = log (0.5) / log ((sliderValueToShowAtMidPoint - minimum)
                                            / (maximum - minimum));
    }

    void setIncDecButtonsMode (const IncDecButtonMode mode)
    {
        if (incDecButtonMode != mode)
        {
            incDecButtonMode = mode;
            owner.lookAndFeelChanged();
        }
    }

    void setTextBoxStyle (const TextEntryBoxPosition newPosition,
                          const bool isReadOnly,
                          const int textEntryBoxWidth,
                          const int textEntryBoxHeight)
    {
    #ifdef LE_PATCHED_JUCE
        jassert( newPosition == NoTextBox );
        jassert( isReadOnly  == true      );
        textBoxPos = newPosition;
        editableText = ! isReadOnly;
        textBoxWidth = textEntryBoxWidth;
        textBoxHeight = textEntryBoxHeight;
    #else
        if (textBoxPos != newPosition
             || editableText != (! isReadOnly)
             || textBoxWidth != textEntryBoxWidth
             || textBoxHeight != textEntryBoxHeight)
        {
            textBoxPos = newPosition;
            editableText = ! isReadOnly;
            textBoxWidth = textEntryBoxWidth;
            textBoxHeight = textEntryBoxHeight;

            owner.repaint();
            owner.lookAndFeelChanged();
        }
    #endif // LE_PATCHED_JUCE
    }

    void setTextBoxIsEditable (const bool shouldBeEditable)
    {
    #ifdef LE_PATCHED_JUCE
        LE_PATCH_UNREACHABLE_CODE
        (void)shouldBeEditable;
    #else
        editableText = shouldBeEditable;

        if (valueBox != nullptr)
            valueBox->setEditable (shouldBeEditable && owner.isEnabled());
    #endif // LE_PATCHED_JUCE
    }

    void showTextBox()
    {
    #ifdef LE_PATCHED_JUCE
        LE_PATCH_UNREACHABLE_CODE
    #else
        jassert (editableText); // this should probably be avoided in read-only sliders.

        if (valueBox != nullptr)
            valueBox->showEditor();
    #endif // LE_PATCHED_JUCE
    }

    void hideTextBox (const bool discardCurrentEditorContents)
    {
    #ifdef LE_PATCHED_JUCE
        LE_PATCH_UNREACHABLE_CODE
    #else
        if (valueBox != nullptr)
        {
            valueBox->hideEditor (discardCurrentEditorContents);

            if (discardCurrentEditorContents)
                updateText();
        }
    #endif // LE_PATCHED_JUCE
    }

    void setTextValueSuffix (const String& suffix)
    {
        if (textSuffix != suffix)
        {
            textSuffix = suffix;
            updateText();
        }
    }

    void lookAndFeelChanged (LookAndFeel& lf)
    {
        if (textBoxPos != NoTextBox)
        {
        #ifdef LE_PATCHED_JUCE
            LE_PATCH_UNREACHABLE_CODE
        #else
            const String previousTextBoxContent (valueBox != nullptr ? valueBox->getText()
                                                                     : owner.getTextFromValue (currentValue.getValue()));

            valueBox = nullptr;
            owner.addAndMakeVisible (valueBox = lf.createSliderTextBox (owner));

            valueBox->setWantsKeyboardFocus (false);
            valueBox->setText (previousTextBoxContent, dontSendNotification);

            if (valueBox->isEditable() != editableText) // (avoid overriding the single/double click flags unless we have to)
                valueBox->setEditable (editableText && owner.isEnabled());

            valueBox->addListener (this);

            if (style == LinearBar || style == LinearBarVertical)
            {
                valueBox->addMouseListener (&owner, false);
                valueBox->setMouseCursor (MouseCursor::ParentCursor);
            }
            else
            {
                valueBox->setTooltip (owner.getTooltip());
            }
        #endif // LE_PATCHED_JUCE
        }
        else
        {
        #ifndef LE_PATCHED_JUCE
            valueBox = nullptr;
        #endif // LE_PATCHED_JUCE
        }

        if (style == IncDecButtons)
        {
        #ifdef LE_PATCHED_JUCE
            LE_PATCH_UNREACHABLE_CODE
        #else
            owner.addAndMakeVisible (incButton = lf.createSliderButton (true));
            incButton->addListener (this);

            owner.addAndMakeVisible (decButton = lf.createSliderButton (false));
            decButton->addListener (this);

            if (incDecButtonMode != incDecButtonsNotDraggable)
            {
                incButton->addMouseListener (&owner, false);
                decButton->addMouseListener (&owner, false);
            }
            else
            {
                incButton->setRepeatSpeed (300, 100, 20);
                incButton->addMouseListener (decButton, false);

                decButton->setRepeatSpeed (300, 100, 20);
                decButton->addMouseListener (incButton, false);
            }

            const String tooltip (owner.getTooltip());
            incButton->setTooltip (tooltip);
            decButton->setTooltip (tooltip);
        #endif // LE_PATCHED_JUCE
        }
        else
        {
        #ifndef LE_PATCHED_JUCE
            incButton = nullptr;
            decButton = nullptr;
        #endif // LE_PATCHED_JUCE
        }

        owner.setComponentEffect (lf.getSliderEffect());

        owner.resized();
        owner.repaint();
    }

    void showPopupMenu()
    {
        PopupMenu m;
        m.setLookAndFeel (&owner.getLookAndFeel());
        m.addItem (1, TRANS ("Velocity-sensitive mode"), true, isVelocityBased);
        m.addSeparator();

        if (isRotary())
        {
            PopupMenu rotaryMenu;
            rotaryMenu.addItem (2, TRANS ("Use circular dragging"),           true, style == Rotary);
            rotaryMenu.addItem (3, TRANS ("Use left-right dragging"),         true, style == RotaryHorizontalDrag);
            rotaryMenu.addItem (4, TRANS ("Use up-down dragging"),            true, style == RotaryVerticalDrag);
            rotaryMenu.addItem (5, TRANS ("Use left-right/up-down dragging"), true, style == RotaryHorizontalVerticalDrag);

            m.addSubMenu (TRANS ("Rotary mode"), rotaryMenu);
        }

        m.showMenuAsync (PopupMenu::Options(),
                         ModalCallbackFunction::forComponent (sliderMenuCallback, &owner));
    }

    static void sliderMenuCallback (const int result, Slider* slider)
    {
        if (slider != nullptr)
        {
            switch (result)
            {
                case 1:   slider->setVelocityBasedMode (! slider->getVelocityBasedMode()); break;
                case 2:   slider->setSliderStyle (Rotary); break;
                case 3:   slider->setSliderStyle (RotaryHorizontalDrag); break;
                case 4:   slider->setSliderStyle (RotaryVerticalDrag); break;
                case 5:   slider->setSliderStyle (RotaryHorizontalVerticalDrag); break;
                default:  break;
            }
        }
    }

    int getThumbIndexAt (const MouseEvent& e)
    {
        const bool isTwoValue   = (style == TwoValueHorizontal   || style == TwoValueVertical);
        const bool isThreeValue = (style == ThreeValueHorizontal || style == ThreeValueVertical);
        //LE_PATCH_ASSUME( isTwoValue   == true  );
        LE_PATCH_ASSUME( isThreeValue == false );
        if (isTwoValue || isThreeValue)
        {
            const float mousePos = (float) (isVertical() ? e.y : e.x);

            JUCE_ORIGINAL( const float normalPosDistance = std::abs (getLinearSliderPos (currentValue.getValue()) - mousePos); )
            const float minPosDistance    = std::abs (getLinearSliderPos (valueMin.getValue()) - 0.1f - mousePos);
            const float maxPosDistance    = std::abs (getLinearSliderPos (valueMax.getValue()) + 0.1f - mousePos);

            if (isTwoValue)
                return maxPosDistance <= minPosDistance ? 2 : 1;
            LE_PATCH_UNREACHABLE_CODE
        #ifndef LE_PATCHED_JUCE
            if (normalPosDistance >= minPosDistance && maxPosDistance >= minPosDistance)
                return 1;

            if (normalPosDistance >= maxPosDistance)
                return 2;
        #endif // LE_PATCHED_JUCE
        }

        return 0;
    }

    //==============================================================================
    void handleRotaryDrag (const MouseEvent& e)
    {
        const int dx = e.x - sliderRect.getCentreX();
        const int dy = e.y - sliderRect.getCentreY();

        if (dx * dx + dy * dy > 25)
        {
            double angle = std::atan2 ((double) dx, (double) -dy);
            while (angle < 0.0)
                angle += double_Pi * 2.0;

            if (rotaryStop && ! e.mouseWasClicked())
            {
                if (std::abs (angle - lastAngle) > double_Pi)
                {
                    if (angle >= lastAngle)
                        angle -= double_Pi * 2.0;
                    else
                        angle += double_Pi * 2.0;
                }

                if (angle >= lastAngle)
                    angle = jmin (angle, (double) jmax (rotaryStart, rotaryEnd));
                else
                    angle = jmax (angle, (double) jmin (rotaryStart, rotaryEnd));
            }
            else
            {
                while (angle < rotaryStart)
                    angle += double_Pi * 2.0;

                if (angle > rotaryEnd)
                {
                    if (smallestAngleBetween (angle, rotaryStart)
                         <= smallestAngleBetween (angle, rotaryEnd))
                        angle = rotaryStart;
                    else
                        angle = rotaryEnd;
                }
            }

            const double proportion = (angle - rotaryStart) / (rotaryEnd - rotaryStart);
            valueWhenLastDragged = owner.proportionOfLengthToValue (jlimit (0.0, 1.0, proportion));
            lastAngle = angle;
        }
    }

    void handleAbsoluteDrag (const MouseEvent& e)
    {
        const int mousePos = (isHorizontal() || style == RotaryHorizontalDrag) ? e.x : e.y;
        double newPos = (mousePos - sliderRegionStart) / (double) sliderRegionSize;

        if (style == RotaryHorizontalDrag
            || style == RotaryVerticalDrag
            || style == IncDecButtons
            || ((style == LinearHorizontal || style == LinearVertical || style == LinearBar || style == LinearBarVertical)
                && ! snapsToMousePos))
        {
            const int mouseDiff = (style == RotaryHorizontalDrag
                                     || style == LinearHorizontal
                                     || style == LinearBar
                                     || (style == IncDecButtons && incDecDragDirectionIsHorizontal()))
                                    ? e.x - mouseDragStartPos.x
                                    : mouseDragStartPos.y - e.y;

            newPos = owner.valueToProportionOfLength (valueOnMouseDown)
                       + mouseDiff * (1.0 / pixelsForFullDragExtent);

            if (style == IncDecButtons)
            {
            #ifdef LE_PATCHED_JUCE
                LE_PATCH_UNREACHABLE_CODE
            #else
                incButton->setState (mouseDiff < 0 ? Button::buttonNormal : Button::buttonDown);
                decButton->setState (mouseDiff > 0 ? Button::buttonNormal : Button::buttonDown);
            #endif // LE_PATCHED_JUCE
            }
        }
        else if (style == RotaryHorizontalVerticalDrag)
        {
            const int mouseDiff = (e.x - mouseDragStartPos.x) + (mouseDragStartPos.y - e.y);

            newPos = owner.valueToProportionOfLength (valueOnMouseDown)
                       + mouseDiff * (1.0 / pixelsForFullDragExtent);
        }
        else
        {
            if (isVertical())
                newPos = 1.0 - newPos;
        }

        valueWhenLastDragged = owner.proportionOfLengthToValue (jlimit (0.0, 1.0, newPos));
    }

    void handleVelocityDrag (const MouseEvent& e)
    {
        const int mouseDiff = style == RotaryHorizontalVerticalDrag
                                ? (e.x - mousePosWhenLastDragged.x) + (mousePosWhenLastDragged.y - e.y)
                                : (isHorizontal()
                                    || style == RotaryHorizontalDrag
                                    || (style == IncDecButtons && incDecDragDirectionIsHorizontal()))
                                      ? e.x - mousePosWhenLastDragged.x
                                      : e.y - mousePosWhenLastDragged.y;

        const double maxSpeed = jmax (200, sliderRegionSize);
        double speed = jlimit (0.0, maxSpeed, (double) abs (mouseDiff));

        if (speed != 0)
        {
            speed = 0.2 * velocityModeSensitivity
                      * (1.0 + std::sin (double_Pi * (1.5 + jmin (0.5, velocityModeOffset
                                                                    + jmax (0.0, (double) (speed - velocityModeThreshold))
                                                                        / maxSpeed))));

            if (mouseDiff < 0)
                speed = -speed;

            if (isVertical() || style == RotaryVerticalDrag
                 || (style == IncDecButtons && ! incDecDragDirectionIsHorizontal()))
                speed = -speed;

            const double currentPos = owner.valueToProportionOfLength (valueWhenLastDragged);

            valueWhenLastDragged = owner.proportionOfLengthToValue (jlimit (0.0, 1.0, currentPos + speed));

            e.source.enableUnboundedMouseMovement (true, false);
            mouseWasHidden = true;
        }
    }

    void mouseDown (const MouseEvent& e)
    {
        mouseWasHidden = false;
        incDecDragged = false;
        useDragEvents = false;
        mouseDragStartPos = mousePosWhenLastDragged = e.getPosition();
        currentDrag = nullptr;

        if (owner.isEnabled())
        {
            if (e.mods.isPopupMenu() && menuEnabled)
            {
                showPopupMenu();
            }
            else if (canDoubleClickToValue() && e.mods.isAltDown())
            {
                mouseDoubleClick();
            }
            else if (maximum > minimum)
            {
                useDragEvents = true;

            #ifndef LE_PATCHED_JUCE
                if (valueBox != nullptr)
                    valueBox->hideEditor (true);
            #endif // LE_PATCHED_JUCE

                sliderBeingDragged = getThumbIndexAt (e);

                minMaxDiff = (double) valueMax.getValue() - (double) valueMin.getValue();

                lastAngle = rotaryStart + (rotaryEnd - rotaryStart)
                                            * owner.valueToProportionOfLength (currentValue.getValue());

                valueWhenLastDragged = (sliderBeingDragged == 2 ? valueMax
                                                                : (sliderBeingDragged == 1 ? valueMin
                                                                                           : currentValue)).getValue();
                valueOnMouseDown = valueWhenLastDragged;

                if (popupDisplayEnabled)
                {
                #ifdef LE_PATCHED_JUCE
                    LE_PATCH_UNREACHABLE_CODE
                #else
                    PopupDisplayComponent* const popup = new PopupDisplayComponent (owner);
                    popupDisplay = popup;

                    if (parentForPopupDisplay != nullptr)
                        parentForPopupDisplay->addChildComponent (popup);
                    else
                        popup->addToDesktop (0);

                    popup->setVisible (true);
                #endif // LE_PATCHED_JUCE
                }

                currentDrag = new DragInProgress (*this);
                mouseDrag (e);
            }
        }
    }

    void mouseDrag (const MouseEvent& e) LE_PATCH( noexcept )
    {
        if (useDragEvents
             && maximum > minimum
             && ! ((style == LinearBar || style == LinearBarVertical) && e.mouseWasClicked() JUCE_ORIGINAL( && valueBox != nullptr && valueBox->isEditable()) ))
        {
            if (style == Rotary)
            {
                handleRotaryDrag (e);
            }
            else
            {
                if (style == IncDecButtons && ! incDecDragged)
                {
                    LE_PATCH_UNREACHABLE_CODE
                    if (e.getDistanceFromDragStart() < 10 || e.mouseWasClicked())
                        return;

                    incDecDragged = true;
                    mouseDragStartPos = e.getPosition();
                }
                LE_PATCH_ASSUME( userKeyOverridesVelocity );
                if (isVelocityBased == (userKeyOverridesVelocity && e.mods.testFlags (ModifierKeys::ctrlModifier
                                                                                        | ModifierKeys::commandModifier
                                                                                        | ModifierKeys::altModifier))
                     JUCE_ORIGINAL( || (maximum - minimum) / sliderRegionSize < interval ))
                    handleAbsoluteDrag (e);
                else
                    handleVelocityDrag (e);
            }

            valueWhenLastDragged = jlimit (minimum, maximum, valueWhenLastDragged);

            if (sliderBeingDragged == 0)
            {
                setValue (owner.snapValue (valueWhenLastDragged, true),
                          sendChangeOnlyOnRelease ? dontSendNotification : sendNotificationSync);
            }
            else if (sliderBeingDragged == 1)
            {
                setMinValue (owner.snapValue (valueWhenLastDragged, true),
                             sendChangeOnlyOnRelease ? dontSendNotification : sendNotificationAsync, true);

                if (e.mods.isShiftDown())
                    setMaxValue (getMinValue() + minMaxDiff, dontSendNotification, true);
                else
                    minMaxDiff = (double) valueMax.getValue() - (double) valueMin.getValue();
            }
            else if (sliderBeingDragged == 2)
            {
                setMaxValue (owner.snapValue (valueWhenLastDragged, true),
                             sendChangeOnlyOnRelease ? dontSendNotification : sendNotificationAsync, true);

                if (e.mods.isShiftDown())
                    setMinValue (getMaxValue() - minMaxDiff, dontSendNotification, true);
                else
                    minMaxDiff = (double) valueMax.getValue() - (double) valueMin.getValue();
            }

            mousePosWhenLastDragged = e.getPosition();
        }
    }

    void mouseUp()
    {
    #ifdef LE_PATCHED_JUCE
        // ModuleKnob::mouseUp() requires this to be handled even when the slider is
        // temporarily disabled.
        jassert( maximum > minimum );
        jassert( style != IncDecButtons || incDecDragged );
        //if ( !menuShown )
    #else // LE_PATCHED_JUCE
        if (owner.isEnabled()
             && useDragEvents
             && (maximum > minimum)
             && (style != IncDecButtons || incDecDragged))
    #endif // LE_PATCHED_JUCE
        {
            restoreMouseIfHidden();

            if (sendChangeOnlyOnRelease && valueOnMouseDown != (double) currentValue.getValue())
                triggerChangeMessage (sendNotificationAsync);

            currentDrag = nullptr;
            JUCE_ORIGINAL( popupDisplay = nullptr; )

            if (style == IncDecButtons)
            {
            #ifdef LE_PATCHED_JUCE
                LE_PATCH_UNREACHABLE_CODE
            #else
                incButton->setState (Button::buttonNormal);
                decButton->setState (Button::buttonNormal);
            #endif // LE_PATCHED_JUCE
            }
        }
    #ifndef LE_PATCHED_JUCE
        else if (popupDisplay != nullptr)
        {
            popupDisplay->startTimer (2000);
        }
    #endif // LE_PATCHED_JUCE
        currentDrag = nullptr;
    }

    bool canDoubleClickToValue() const
    {
        return doubleClickToValue
                && style != IncDecButtons
                && minimum <= doubleClickReturnValue
                && maximum >= doubleClickReturnValue;
    }

    void mouseDoubleClick()
    {
        if (canDoubleClickToValue())
        {
            DragInProgress drag (*this);
            setValue (doubleClickReturnValue, sendNotificationSync);
        }
    }

    bool mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel) LE_PATCH( noexcept )
    {
        if (scrollWheelEnabled
             && style != TwoValueHorizontal
             && style != TwoValueVertical)
        {
            if (maximum > minimum && ! e.mods.isAnyMouseButtonDown())
            {
            #ifndef LE_PATCHED_JUCE
                if (valueBox != nullptr)
                    valueBox->hideEditor (false);
            #endif // LE_PATCHED_JUCE

                const double value = (double) currentValue.getValue();
                const double proportionDelta = (wheel.deltaX != 0 ? -wheel.deltaX : wheel.deltaY)
                                                   * (wheel.isReversed ? -0.15f : 0.15f);
                const double currentPos = owner.valueToProportionOfLength (value);
                const double newValue = owner.proportionOfLengthToValue (jlimit (0.0, 1.0, currentPos + proportionDelta));

                double delta = (newValue != value) ? jmax (std::abs (newValue - value), interval) : 0;
                if (value > newValue)
                    delta = -delta;

                DragInProgress drag (*this);
                setValue (owner.snapValue (value + delta, false), sendNotificationSync);
            }

            return true;
        }

        return false;
    }

    void modifierKeysChanged (const ModifierKeys& modifiers) LE_PATCH( noexcept )
    {
        if (style != IncDecButtons
             && style != Rotary
             && isVelocityBased == modifiers.isAnyModifierKeyDown())
        {
            restoreMouseIfHidden();
        }
    }

    void restoreMouseIfHidden()
    {
        if (mouseWasHidden)
        {
            mouseWasHidden = false;

            for (int i = Desktop::getInstance().getNumMouseSources(); --i >= 0;)
                Desktop::getInstance().getMouseSource(i)->enableUnboundedMouseMovement (false);

            const double pos = sliderBeingDragged == 2 ? getMaxValue()
                                                       : (sliderBeingDragged == 1 ? getMinValue()
                                                                                  : (double) currentValue.getValue());
            Point<int> mousePos;

            if (isRotary())
            {
                mousePos = Desktop::getLastMouseDownPosition();

                const int delta = roundToInt (pixelsForFullDragExtent * (owner.valueToProportionOfLength (valueOnMouseDown)
                                                                           - owner.valueToProportionOfLength (pos)));

                if (style == RotaryHorizontalDrag)      mousePos += Point<int> (-delta, 0);
                else if (style == RotaryVerticalDrag)   mousePos += Point<int> (0, delta);
                else                                    mousePos += Point<int> (delta / -2, delta / 2);
            }
            else
            {
                const int pixelPos = (int) getLinearSliderPos (pos);

                mousePos = owner.localPointToGlobal (Point<int> (isHorizontal() ? pixelPos : (owner.getWidth() / 2),
                                                                 isVertical()   ? pixelPos : (owner.getHeight() / 2)));
            }

            Desktop::setMousePosition (mousePos);
        }
    }

    //==============================================================================
    void paint (Graphics& g, LookAndFeel& lf)
    {
        LE_PATCH_ASSUME( style != IncDecButtons );
        if (style != IncDecButtons)
        {
            if (isRotary())
            {
                const float sliderPos = (float) owner.valueToProportionOfLength (lastCurrentValue);
                jassert (sliderPos >= 0 && sliderPos <= 1.0f);

                lf.drawRotarySlider (g,
                                     sliderRect.getX(), sliderRect.getY(),
                                     sliderRect.getWidth(), sliderRect.getHeight(),
                                     sliderPos, rotaryStart, rotaryEnd, owner);
            }
            else
            {
                lf.drawLinearSlider (g,
                                     sliderRect.getX(), sliderRect.getY(),
                                     sliderRect.getWidth(), sliderRect.getHeight(),
                                     getLinearSliderPos (lastCurrentValue),
                                     getLinearSliderPos (lastValueMin),
                                     getLinearSliderPos (lastValueMax),
                                     style, owner);
            }

            if ((style == LinearBar || style == LinearBarVertical) JUCE_ORIGINAL( && valueBox == nullptr ))
            {
                LE_PATCH_UNREACHABLE_CODE
                g.setColour (owner.findColour (Slider::textBoxOutlineColourId));
                g.drawRect (0, 0, owner.getWidth(), owner.getHeight(), 1);
            }
        }
    }

    void resized (const Rectangle<int>& localBounds, LookAndFeel& lf)
    {
        int minXSpace = 0;
        int minYSpace = 0;

        if (textBoxPos == TextBoxLeft || textBoxPos == TextBoxRight)
            minXSpace = 30;
        else
            minYSpace = 15;

        const int tbw = jmax (0, jmin (textBoxWidth,  localBounds.getWidth() - minXSpace));
        const int tbh = jmax (0, jmin (textBoxHeight, localBounds.getHeight() - minYSpace));

        if (style == LinearBar || style == LinearBarVertical)
        {
        #ifndef LE_PATCHED_JUCE
            if (valueBox != nullptr)
                valueBox->setBounds (localBounds);
        #endif // LE_PATCHED_JUCE
        }
        else
        {
            if (textBoxPos == NoTextBox)
            {
                sliderRect = localBounds;
            }
            else if (textBoxPos == TextBoxLeft)
            {
            #ifndef LE_PATCHED_JUCE
                valueBox->setBounds (0, (localBounds.getHeight() - tbh) / 2, tbw, tbh);
            #endif // LE_PATCHED_JUCE
                sliderRect.setBounds (tbw, 0, localBounds.getWidth() - tbw, localBounds.getHeight());
            }
            else if (textBoxPos == TextBoxRight)
            {
            #ifndef LE_PATCHED_JUCE
                valueBox->setBounds (localBounds.getWidth() - tbw, (localBounds.getHeight() - tbh) / 2, tbw, tbh);
            #endif // LE_PATCHED_JUCE
                sliderRect.setBounds (0, 0, localBounds.getWidth() - tbw, localBounds.getHeight());
            }
            else if (textBoxPos == TextBoxAbove)
            {
            #ifndef LE_PATCHED_JUCE
                valueBox->setBounds ((localBounds.getWidth() - tbw) / 2, 0, tbw, tbh);
            #endif // LE_PATCHED_JUCE
                sliderRect.setBounds (0, tbh, localBounds.getWidth(), localBounds.getHeight() - tbh);
            }
            else if (textBoxPos == TextBoxBelow)
            {
            #ifndef LE_PATCHED_JUCE
                valueBox->setBounds ((localBounds.getWidth() - tbw) / 2, localBounds.getHeight() - tbh, tbw, tbh);
            #endif // LE_PATCHED_JUCE
                sliderRect.setBounds (0, 0, localBounds.getWidth(), localBounds.getHeight() - tbh);
            }
        }

        const int indent = lf.getSliderThumbRadius (owner);

        if (style == LinearBar)
        {
            const int barIndent = 1;
            sliderRegionStart = barIndent;
            sliderRegionSize = localBounds.getWidth() - barIndent * 2;

            sliderRect.setBounds (sliderRegionStart, barIndent,
                                  sliderRegionSize, localBounds.getHeight() - barIndent * 2);
        }
        else if (style == LinearBarVertical)
        {
            const int barIndent = 1;
            sliderRegionStart = barIndent;
            sliderRegionSize = localBounds.getHeight() - barIndent * 2;

            sliderRect.setBounds (barIndent, sliderRegionStart,
                                  localBounds.getWidth() - barIndent * 2, sliderRegionSize);
        }
        else if (isHorizontal())
        {
            sliderRegionStart = sliderRect.getX() + indent;
            sliderRegionSize = jmax (1, sliderRect.getWidth() - indent * 2);

            sliderRect.setBounds (sliderRegionStart, sliderRect.getY(),
                                  sliderRegionSize, sliderRect.getHeight());
        }
        else if (isVertical())
        {
            sliderRegionStart = sliderRect.getY() + indent;
            sliderRegionSize = jmax (1, sliderRect.getHeight() - indent * 2);

            sliderRect.setBounds (sliderRect.getX(), sliderRegionStart,
                                  sliderRect.getWidth(), sliderRegionSize);
        }
        else
        {
            sliderRegionStart = 0;
            sliderRegionSize = 100;
        }

        if (style == IncDecButtons)
            resizeIncDecButtons();
    }

    void resizeIncDecButtons()
    {
    #ifdef LE_PATCHED_JUCE
        LE_PATCH_UNREACHABLE_CODE
    #else
        Rectangle<int> buttonRect (sliderRect);

        if (textBoxPos == TextBoxLeft || textBoxPos == TextBoxRight)
            buttonRect.expand (-2, 0);
        else
            buttonRect.expand (0, -2);

        incDecButtonsSideBySide = buttonRect.getWidth() > buttonRect.getHeight();

        if (incDecButtonsSideBySide)
        {
            decButton->setBounds (buttonRect.removeFromLeft (buttonRect.getWidth() / 2));
            decButton->setConnectedEdges (Button::ConnectedOnRight);
            incButton->setConnectedEdges (Button::ConnectedOnLeft);
        }
        else
        {
            decButton->setBounds (buttonRect.removeFromBottom (buttonRect.getHeight() / 2));
            decButton->setConnectedEdges (Button::ConnectedOnTop);
            incButton->setConnectedEdges (Button::ConnectedOnBottom);
        }

        incButton->setBounds (buttonRect);
    #endif // LE_PATCHED_JUCE
    }

    //==============================================================================
    Slider& owner;
    SliderStyle style;

#ifdef LE_PATCHED_JUCE
    SliderListener * pListener;
#else
    ListenerList <SliderListener> listeners;
#endif // LE_PATCHED_JUCE
    Value currentValue, valueMin, valueMax;
    double lastCurrentValue, lastValueMin, lastValueMax;
    double minimum, maximum, interval, doubleClickReturnValue;
    double valueWhenLastDragged, valueOnMouseDown, skewFactor, lastAngle;
    double velocityModeSensitivity, velocityModeOffset, minMaxDiff;
    int velocityModeThreshold;
    float rotaryStart, rotaryEnd;
    Point<int> mouseDragStartPos, mousePosWhenLastDragged;
    int sliderRegionStart, sliderRegionSize;
    int sliderBeingDragged;
    int pixelsForFullDragExtent;
    Rectangle<int> sliderRect;
    ScopedPointer<DragInProgress> currentDrag;

    TextEntryBoxPosition textBoxPos;
    String textSuffix;
    int numDecimalPlaces;
    int textBoxWidth, textBoxHeight;
    IncDecButtonMode incDecButtonMode;

    bool editableText;
    bool doubleClickToValue;
    bool isVelocityBased;
    bool userKeyOverridesVelocity;
    bool rotaryStop;
    bool incDecButtonsSideBySide;
    bool sendChangeOnlyOnRelease;
    bool popupDisplayEnabled;
    bool menuEnabled;
    bool useDragEvents;
    bool mouseWasHidden;
    bool incDecDragged;
    bool scrollWheelEnabled;
    bool snapsToMousePos;

#ifndef LE_PATCHED_JUCE
    ScopedPointer<Label> valueBox;
    ScopedPointer<Button> incButton, decButton;
#endif // LE_PATCHED_JUCE

    //==============================================================================
#ifndef LE_PATCHED_JUCE
    class PopupDisplayComponent  : public BubbleComponent,
                                   public Timer
    {
    public:
        PopupDisplayComponent (Slider& s)
            : owner (s),
              font (s.getLookAndFeel().getSliderPopupFont())
        {
            setAlwaysOnTop (true);
            setAllowedPlacement (owner.getLookAndFeel().getSliderPopupPlacement());
        }

        void paintContent (Graphics& g, int w, int h)
        {
            g.setFont (font);
            g.setColour (findColour (TooltipWindow::textColourId, true));
            g.drawFittedText (text, Rectangle<int> (w, h), Justification::centred, 1);
        }

        void getContentSize (int& w, int& h)
        {
            w = font.getStringWidth (text) + 18;
            h = (int) (font.getHeight() * 1.6f);
        }

        void updatePosition (const String& newText)
        {
            text = newText;
            BubbleComponent::setPosition (&owner);
            repaint();
        }

        void timerCallback() override
        {
            owner.pimpl->popupDisplay = nullptr;
        }

    private:
        Slider& owner;
        Font font;
        String text;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PopupDisplayComponent)
    };

    ScopedPointer <PopupDisplayComponent> popupDisplay;
    Component* parentForPopupDisplay;
#endif // LE_PATCHED_JUCE

    //==============================================================================
    static double smallestAngleBetween (const double a1, const double a2) noexcept
    {
        return jmin (std::abs (a1 - a2),
                     std::abs (a1 + double_Pi * 2.0 - a2),
                     std::abs (a2 + double_Pi * 2.0 - a1));
    }
};


//==============================================================================
Slider::Slider()
{
#ifdef LE_PATCHED_JUCE
    init (RotaryVerticalDrag, NoTextBox);
#else
    init (LinearHorizontal, TextBoxLeft);
#endif // LE_PATCHED_JUCE
}

Slider::Slider (const String& name)  : Component (name)
{
#ifdef LE_PATCHED_JUCE
    init (RotaryVerticalDrag, NoTextBox);
#else
    init (LinearHorizontal, TextBoxLeft);
#endif // LE_PATCHED_JUCE
}

Slider::Slider (SliderStyle style, TextEntryBoxPosition textBoxPos)
{
    init (style, textBoxPos);
}

void Slider::init (SliderStyle style, TextEntryBoxPosition textBoxPos)
{
    setWantsKeyboardFocus (false);
    setRepaintsOnMouseActivity (true);

    pimpl = new Pimpl (*this, style, textBoxPos);

    Slider::lookAndFeelChanged();
    updateText();

    pimpl->registerListeners();
}

Slider::~Slider() LE_PATCH( noexcept ) {}

//==============================================================================
#ifdef LE_PATCHED_JUCE
void Slider::addListener (SliderListener* const listener)       { jassert( pimpl->pListener == nullptr  ); pimpl->pListener = listener; }
void Slider::removeListener (SliderListener* const listener)    { jassert( pimpl->pListener == listener ); pimpl->pListener = nullptr ; }
#else
void Slider::addListener (SliderListener* const listener)       { pimpl->listeners.add (listener); }
void Slider::removeListener (SliderListener* const listener)    { pimpl->listeners.remove (listener); }
#endif // LE_PATCHED_JUCE

//==============================================================================
Slider::SliderStyle Slider::getSliderStyle() const noexcept     { return pimpl->style; }
void Slider::setSliderStyle (const SliderStyle newStyle)        { pimpl->setSliderStyle (newStyle); }

void Slider::setRotaryParameters (const float startAngleRadians, const float endAngleRadians, const bool stopAtEnd)
{
    pimpl->setRotaryParameters (startAngleRadians, endAngleRadians, stopAtEnd);
}

void Slider::setVelocityBasedMode (bool vb)                 { pimpl->isVelocityBased = vb; }
bool Slider::getVelocityBasedMode() const noexcept          { return pimpl->isVelocityBased; }
bool Slider::getVelocityModeIsSwappable() const noexcept    { return pimpl->userKeyOverridesVelocity; }
int Slider::getVelocityThreshold() const noexcept           { return pimpl->velocityModeThreshold; }
double Slider::getVelocitySensitivity() const noexcept      { return pimpl->velocityModeSensitivity; }
double Slider::getVelocityOffset() const noexcept           { return pimpl->velocityModeOffset; }

void Slider::setVelocityModeParameters (const double sensitivity, const int threshold,
                                        const double offset, const bool userCanPressKeyToSwapMode)
{
    jassert (threshold >= 0);
    jassert (sensitivity > 0);
    jassert (offset >= 0);

    pimpl->setVelocityModeParameters (sensitivity, threshold, offset, userCanPressKeyToSwapMode);
}

double Slider::getSkewFactor() const noexcept               { return pimpl->skewFactor; }
void Slider::setSkewFactor (const double factor)            { pimpl->skewFactor = factor; }

void Slider::setSkewFactorFromMidPoint (const double sliderValueToShowAtMidPoint)
{
    pimpl->setSkewFactorFromMidPoint (sliderValueToShowAtMidPoint);
}

int Slider::getMouseDragSensitivity() const noexcept        { return pimpl->pixelsForFullDragExtent; }

void Slider::setMouseDragSensitivity (const int distanceForFullScaleDrag)
{
    jassert (distanceForFullScaleDrag > 0);

    pimpl->pixelsForFullDragExtent = distanceForFullScaleDrag;
}

void Slider::setIncDecButtonsMode (const IncDecButtonMode mode)             { pimpl->setIncDecButtonsMode (mode); }

Slider::TextEntryBoxPosition Slider::getTextBoxPosition() const noexcept    { return pimpl->textBoxPos; }
int Slider::getTextBoxWidth() const noexcept                                { return pimpl->textBoxWidth; }
int Slider::getTextBoxHeight() const noexcept                               { return pimpl->textBoxHeight; }

void Slider::setTextBoxStyle (const TextEntryBoxPosition newPosition, const bool isReadOnly,
                              const int textEntryBoxWidth, const int textEntryBoxHeight)
{
    pimpl->setTextBoxStyle (newPosition, isReadOnly, textEntryBoxWidth, textEntryBoxHeight);
}

bool Slider::isTextBoxEditable() const noexcept                     { return pimpl->editableText; }
void Slider::setTextBoxIsEditable (const bool shouldBeEditable)     { pimpl->setTextBoxIsEditable (shouldBeEditable); }
void Slider::showTextBox()                                          { pimpl->showTextBox(); }
void Slider::hideTextBox (const bool discardCurrentEditorContents)  { pimpl->hideTextBox (discardCurrentEditorContents); }

void Slider::setChangeNotificationOnlyOnRelease (bool onlyNotifyOnRelease)
{
    pimpl->sendChangeOnlyOnRelease = onlyNotifyOnRelease;
}

bool Slider::getSliderSnapsToMousePosition() const noexcept                 { return pimpl->snapsToMousePos; }
void Slider::setSliderSnapsToMousePosition (const bool shouldSnapToMouse)   { pimpl->snapsToMousePos = shouldSnapToMouse; }

void Slider::setPopupDisplayEnabled (const bool enabled, Component* const parentComponentToUse)
{
    LE_PATCH_ASSUME( !enabled              );
    LE_PATCH_ASSUME( !parentComponentToUse );
    pimpl->popupDisplayEnabled = enabled;
    JUCE_ORIGINAL( pimpl->parentForPopupDisplay = parentComponentToUse );
}

Component* Slider::getCurrentPopupDisplay() const noexcept      { return LE_PATCH( nullptr ) JUCE_ORIGINAL( pimpl->popupDisplay.get() ); }

//==============================================================================
void Slider::colourChanged()        { lookAndFeelChanged(); }
void Slider::lookAndFeelChanged()   { pimpl->lookAndFeelChanged (getLookAndFeel()); }
void Slider::enablementChanged() LE_PATCH( noexcept )    { repaint(); }

//==============================================================================
double Slider::getMaximum() const noexcept      { return pimpl->maximum; }
double Slider::getMinimum() const noexcept      { return pimpl->minimum; }
double Slider::getInterval() const noexcept     { return pimpl->interval; }

void Slider::setRange (double newMin, double newMax, double newInt)
{
    pimpl->setRange (newMin, newMax, newInt);
}

Value& Slider::getValueObject() noexcept        { return pimpl->currentValue; }
Value& Slider::getMinValueObject() noexcept     { return pimpl->valueMin; }
Value& Slider::getMaxValueObject() noexcept     { return pimpl->valueMax; }

double LE_PATCH( const & ) Slider::getValue() const LE_PATCH( noexcept )                 { return pimpl->getValue(); }

void Slider::setValue (double newValue, const NotificationType notification)
{
    pimpl->setValue (newValue, notification);
}

double Slider::getMinValue() const      { return pimpl->getMinValue(); }
double Slider::getMaxValue() const      { return pimpl->getMaxValue(); }

void Slider::setMinValue (double newValue, const NotificationType notification, bool allowNudgingOfOtherValues)
{
    pimpl->setMinValue (newValue, notification, allowNudgingOfOtherValues);
}

void Slider::setMaxValue (double newValue, const NotificationType notification, bool allowNudgingOfOtherValues)
{
    pimpl->setMaxValue (newValue, notification, allowNudgingOfOtherValues);
}

void Slider::setMinAndMaxValues (double newMinValue, double newMaxValue, const NotificationType notification)
{
    pimpl->setMinAndMaxValues (newMinValue, newMaxValue, notification);
}

void Slider::setDoubleClickReturnValue (bool isDoubleClickEnabled,  double valueToSetOnDoubleClick)
{
    pimpl->doubleClickToValue = isDoubleClickEnabled;
    pimpl->doubleClickReturnValue = valueToSetOnDoubleClick;
}

double Slider::getDoubleClickReturnValue (bool& isEnabledResult) const
{
    isEnabledResult = pimpl->doubleClickToValue;
    return pimpl->doubleClickReturnValue;
}

void Slider::updateText()
{
    pimpl->updateText();
}

void Slider::setTextValueSuffix (const String& suffix)
{
    pimpl->setTextValueSuffix (suffix);
}

String LE_PATCH( const & ) Slider::getTextValueSuffix() const
{
    return pimpl->textSuffix;
}

String Slider::getTextFromValue (double v)
{
    if (getNumDecimalPlacesToDisplay() > 0)
        return String (v, getNumDecimalPlacesToDisplay()) + getTextValueSuffix();

    return String (roundToInt (v)) + getTextValueSuffix();
}

double Slider::getValueFromText (const String& text)
{
    String t (text.trimStart());

    if (t.endsWith (getTextValueSuffix()))
        t = t.substring (0, t.length() - getTextValueSuffix().length());

    while (t.startsWithChar ('+'))
        t = t.substring (1).trimStart();

    return t.initialSectionContainingOnly ("0123456789.,-")
            .getDoubleValue();
}

double Slider::proportionOfLengthToValue (double proportion)
{
    const double skew = getSkewFactor();

    if (skew != 1.0 && proportion > 0.0)
        proportion = exp (log (proportion) / skew);

    return getMinimum() + (getMaximum() - getMinimum()) * proportion;
}

double Slider::valueToProportionOfLength (double value)
{
    const double n = (value - getMinimum()) / (getMaximum() - getMinimum());
    const double skew = getSkewFactor();

    return skew == 1.0 ? n : pow (n, skew);
}

double Slider::snapValue (double attemptedValue, const bool) LE_PATCH( noexcept )
{
    return attemptedValue;
}

int Slider::getNumDecimalPlacesToDisplay() const noexcept    { return pimpl->numDecimalPlaces; }

//==============================================================================
int Slider::getThumbBeingDragged() const noexcept            { return pimpl->sliderBeingDragged; }

void Slider::startedDragging() LE_PATCH( noexcept ) {}
void Slider::stoppedDragging() LE_PATCH( noexcept ) {}
void Slider::valueChanged() LE_PATCH( noexcept ) {}

//==============================================================================
void Slider::setPopupMenuEnabled (const bool menuEnabled)   { pimpl->menuEnabled = menuEnabled; }
void Slider::setScrollWheelEnabled (const bool enabled)     { pimpl->scrollWheelEnabled = enabled; }

bool Slider::isHorizontal() const noexcept   { return pimpl->isHorizontal(); }
bool Slider::isVertical() const noexcept     { return pimpl->isVertical(); }

float Slider::getPositionOfValue (const double value)   { return pimpl->getPositionOfValue (value); }

//==============================================================================
void Slider::paint (Graphics& g)        { pimpl->paint (g, getLookAndFeel()); }
void Slider::resized()                  { pimpl->resized (getLocalBounds(), getLookAndFeel()); }

void Slider::focusOfChildComponentChanged (FocusChangeType)     { repaint(); }

void Slider::mouseDown (const MouseEvent& e)    { pimpl->mouseDown (e); }
void Slider::mouseUp (const MouseEvent&)        { pimpl->mouseUp(); }

void Slider::modifierKeysChanged (const ModifierKeys& modifiers) LE_PATCH( noexcept )
{
    if (isEnabled())
        pimpl->modifierKeysChanged (modifiers);
}

void Slider::mouseDrag (const MouseEvent& e) LE_PATCH( noexcept )
{
    if (isEnabled())
        pimpl->mouseDrag (e);
}

void Slider::mouseDoubleClick (const MouseEvent&)
{
    if (isEnabled())
        pimpl->mouseDoubleClick();
}

void Slider::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel) LE_PATCH( noexcept )
{
    if (! (isEnabled() && pimpl->mouseWheelMove (e, wheel)))
        Component::mouseWheelMove (e, wheel);
}

#ifdef LE_PATCHED_JUCE
int           & Slider::sliderBeingDragged() noexcept { return pimpl->sliderBeingDragged; }
ValueListener & Slider::valueListener     () noexcept { return *pimpl; }
#endif // LE_PATCHED_JUCE
