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

#ifndef JUCE_LASSOCOMPONENT_H_INCLUDED
#define JUCE_LASSOCOMPONENT_H_INCLUDED


//==============================================================================
/**
    A class used by the LassoComponent to manage the things that it selects.

    This allows the LassoComponent to find out which items are within the lasso,
    and to change the list of selected items.

    @see LassoComponent, SelectedItemSet
*/
template <class SelectableItemType>
class LassoSource
{
public:
    /** Destructor. */
    virtual ~LassoSource() {}

    /** Returns the set of items that lie within a given lassoable region.

        Your implementation of this method must find all the relevent items that lie
        within the given rectangle. and add them to the itemsFound array.

        The coordinates are relative to the top-left of the lasso component's parent
        component. (i.e. they are the same as the size and position of the lasso
        component itself).
    */
    virtual void findLassoItemsInArea (Array <SelectableItemType>& itemsFound,
                                       const Rectangle<int>& area) = 0;

    /** Returns the SelectedItemSet that the lasso should update.

        This set will be continuously updated by the LassoComponent as it gets
        dragged around, so make sure that you've got a ChangeListener attached to
        the set so that your UI objects will know when the selection changes and
        be able to update themselves appropriately.
    */
    virtual SelectedItemSet <SelectableItemType>& getLassoSelection() = 0;
};


//==============================================================================
/**
    A component that acts as a rectangular selection region, which you drag with
    the mouse to select groups of objects (in conjunction with a SelectedItemSet).

    To use one of these:

    - In your mouseDown or mouseDrag event, add the LassoComponent to your parent
      component, and call its beginLasso() method, giving it a
      suitable LassoSource object that it can use to find out which items are in
      the active area.

    - Each time your parent component gets a mouseDrag event, call dragLasso()
      to update the lasso's position - it will use its LassoSource to calculate and
      update the current selection.

    - After the drag has finished and you get a mouseUp callback, you should call
      endLasso() to clean up. This will make the lasso component invisible, and you
      can remove it from the parent component, or delete it.

    The class takes into account the modifier keys that are being held down while
    the lasso is being dragged, so if shift is pressed, then any lassoed items will
    be added to the original selection; if ctrl or command is pressed, they will be
    xor'ed with any previously selected items.

    @see LassoSource, SelectedItemSet
*/
template <class SelectableItemType>
class LassoComponent  : public Component
{
public:
    //==============================================================================
    /** Creates a Lasso component. */
    LassoComponent()  : source (nullptr)
    {
    }

    /** Destructor. */
    ~LassoComponent()
    {
    }

    //==============================================================================
    /** Call this in your mouseDown event, to initialise a drag.

        Pass in a suitable LassoSource object which the lasso will use to find
        the items and change the selection.

        After using this method to initialise the lasso, repeatedly call dragLasso()
        in your component's mouseDrag callback.

        @see dragLasso, endLasso, LassoSource
    */
    void beginLasso (const MouseEvent& e,
                     LassoSource <SelectableItemType>* const lassoSource)
    {
        jassert (source == nullptr);  // this suggests that you didn't call endLasso() after the last drag...
        jassert (lassoSource != nullptr); // the source can't be null!
        jassert (getParentComponent() != nullptr);  // you need to add this to a parent component for it to work!

        source = lassoSource;

        if (lassoSource != nullptr)
            originalSelection = lassoSource->getLassoSelection().getItemArray();

        setSize (0, 0);
        dragStartPos = e.getMouseDownPosition();
    }

    /** Call this in your mouseDrag event, to update the lasso's position.

        This must be repeatedly calling when the mouse is dragged, after you've
        first initialised the lasso with beginLasso().

        This method takes into account the modifier keys that are being held down, so
        if shift is pressed, then the lassoed items will be added to any that were
        previously selected; if ctrl or command is pressed, then they will be xor'ed
        with previously selected items.

        @see beginLasso, endLasso
    */
    void dragLasso (const MouseEvent& e)
    {
        if (source != nullptr)
        {
            setBounds (Rectangle<int> (dragStartPos, e.getPosition()));
            setVisible (true);

            Array <SelectableItemType> itemsInLasso;
            source->findLassoItemsInArea (itemsInLasso, getBounds());

            if (e.mods.isShiftDown())
            {
                itemsInLasso.removeValuesIn (originalSelection); //  to avoid duplicates
                itemsInLasso.addArray (originalSelection);
            }
            else if (e.mods.isCommandDown() || e.mods.isAltDown())
            {
                Array <SelectableItemType> originalMinusNew (originalSelection);
                originalMinusNew.removeValuesIn (itemsInLasso);

                itemsInLasso.removeValuesIn (originalSelection);
                itemsInLasso.addArray (originalMinusNew);
            }

            source->getLassoSelection() = SelectedItemSet <SelectableItemType> (itemsInLasso);
        }
    }

    /** Call this in your mouseUp event, after the lasso has been dragged.

        @see beginLasso, dragLasso
    */
    void endLasso()
    {
        source = nullptr;
        originalSelection.clear();
        setVisible (false);
    }

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the label.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        Note that you can also use the constants from TextEditor::ColourIds to change the
        colour of the text editor that is opened when a label is editable.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        lassoFillColourId       = 0x1000440, /**< The colour to fill the lasso rectangle with. */
        lassoOutlineColourId    = 0x1000441, /**< The colour to draw the outline with. */
    };

    //==============================================================================
    /** @internal */
    void paint (Graphics& g) override
    {
        getLookAndFeel().drawLasso (g, *this);

        // this suggests that you've left a lasso comp lying around after the
        // mouse drag has finished.. Be careful to call endLasso() when you get a
        // mouse-up event.
        jassert (isMouseButtonDownAnywhere());
    }

    /** @internal */
    bool hitTest (int, int) override        { return false; }

private:
    //==============================================================================
    Array <SelectableItemType> originalSelection;
    LassoSource <SelectableItemType>* source;
    Point<int> dragStartPos;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LassoComponent)
};


#endif   // JUCE_LASSOCOMPONENT_H_INCLUDED
