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

#ifndef JUCE_TABBEDBUTTONBAR_H_INCLUDED
#define JUCE_TABBEDBUTTONBAR_H_INCLUDED

class TabbedButtonBar;


//==============================================================================
/** In a TabbedButtonBar, this component is used for each of the buttons.

    If you want to create a TabbedButtonBar with custom tab components, derive
    your component from this class, and override the TabbedButtonBar::createTabButton()
    method to create it instead of the default one.

    @see TabbedButtonBar
*/
class JUCE_API  TabBarButton  : public Button
{
public:
    //==============================================================================
    /** Creates the tab button. */
    TabBarButton (const String& name, TabbedButtonBar& ownerBar);

    /** Destructor. */
    JUCE_ORIGINAL( ~TabBarButton(); )

    /** Returns the bar that contains this button. */
    TabbedButtonBar& getTabbedButtonBar() const   { return owner; }

    //==============================================================================
    /** When adding an extra component to the tab, this indicates which side of
        the text it should be placed on. */
    enum ExtraComponentPlacement
    {
        beforeText,
        afterText
    };

    /** Sets an extra component that will be shown in the tab.

        This optional component will be positioned inside the tab, either to the left or right
        of the text. You could use this to implement things like a close button or a graphical
        status indicator. If a non-null component is passed-in, the TabbedButtonBar will take
        ownership of it and delete it when required.
     */
    void setExtraComponent (Component* extraTabComponent,
                            ExtraComponentPlacement extraComponentPlacement);

    /** Returns the custom component, if there is one. */
    Component* getExtraComponent() const noexcept                           { return extraComponent; }

    /** Returns the placement of the custom component, if there is one. */
    ExtraComponentPlacement getExtraComponentPlacement() const noexcept     { return extraCompPlacement; }

    /** Returns an area of the component that's safe to draw in.

        This deals with the orientation of the tabs, which affects which side is
        touching the tabbed box's content component.
    */
    Rectangle<int> getActiveArea() const;

    /** Returns the area of the component that should contain its text. */
    Rectangle<int> getTextArea() const;

    /** Returns this tab's index in its tab bar. */
    int getIndex() const;

    /** Returns the colour of the tab. */
    Colour getTabBackgroundColour() const;

    /** Returns true if this is the frontmost (selected) tab. */
    bool isFrontTab() const;

    //==============================================================================
    /** Chooses the best length for the tab, given the specified depth.

        If the tab is horizontal, this should return its width, and the depth
        specifies its height. If it's vertical, it should return the height, and
        the depth is actually its width.
    */
    virtual int getBestTabLength (int depth);

    //==============================================================================
    /** @internal */
    void paintButton (Graphics&, bool isMouseOverButton, bool isButtonDown) override;
    /** @internal */
    void clicked (const ModifierKeys&) LE_PATCH( noexcept ) override;
    /** @internal */
    bool hitTest (int x, int y) override;
    /** @internal */
    void resized() override;
    /** @internal */
    void childBoundsChanged (Component*) override;

protected:
    friend class TabbedButtonBar;
    TabbedButtonBar& owner;
    int overlapPixels;

    ScopedPointer<Component> extraComponent;
    ExtraComponentPlacement extraCompPlacement;

private:
    void calcAreas (Rectangle<int>&, Rectangle<int>&) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TabBarButton)
};


//==============================================================================
/**
    A vertical or horizontal bar containing tabs that you can select.

    You can use one of these to generate things like a dialog box that has
    tabbed pages you can flip between. Attach a ChangeListener to the
    button bar to be told when the user changes the page.

    An easier method than doing this is to use a TabbedComponent, which
    contains its own TabbedButtonBar and which takes care of the layout
    and other housekeeping.

    @see TabbedComponent
*/
class JUCE_API  TabbedButtonBar  : public Component,
                                   public ChangeBroadcaster
{
public:
    //==============================================================================
    /** The placement of the tab-bar
        @see setOrientation, getOrientation
    */
    enum Orientation
    {
        TabsAtTop,
        TabsAtBottom,
        TabsAtLeft,
        TabsAtRight
    };

    //==============================================================================
    /** Creates a TabbedButtonBar with a given orientation.
        You can change the orientation later if you need to.
    */
    TabbedButtonBar (Orientation orientation);

    /** Destructor. */
    ~TabbedButtonBar();

    //==============================================================================
    /** Changes the bar's orientation.

        This won't change the bar's actual size - you'll need to do that yourself,
        but this determines which direction the tabs go in, and which side they're
        stuck to.
    */
    void setOrientation (Orientation orientation);

    /** Returns the bar's current orientation.
        @see setOrientation
    */
    Orientation getOrientation() const noexcept         { return orientation; }

    /** Returns true if the orientation is TabsAtLeft or TabsAtRight. */
    bool isVertical() const noexcept                    { return orientation == TabsAtLeft || orientation == TabsAtRight; }

    /** Returns the thickness of the bar, which may be its width or height, depending on the orientation. */
    int getThickness() const noexcept                   { return isVertical() ? getWidth() : getHeight(); }

    /** Changes the minimum scale factor to which the tabs can be compressed when trying to
        fit a lot of tabs on-screen.
    */
    void setMinimumTabScaleFactor (double newMinimumScale);

    //==============================================================================
    /** Deletes all the tabs from the bar.
        @see addTab
    */
    void clearTabs();

    /** Adds a tab to the bar.
        Tabs are added in left-to-right reading order.
        If this is the first tab added, it'll also be automatically selected.
    */
    void addTab (const String& tabName,
                 Colour tabBackgroundColour,
                 int insertIndex);

    /** Changes the name of one of the tabs. */
    void setTabName (int tabIndex, const String& newName);

    /** Gets rid of one of the tabs. */
    void removeTab (int tabIndex);

    /** Moves a tab to a new index in the list.
        Pass -1 as the index to move it to the end of the list.
    */
    void moveTab (int currentIndex, int newIndex);

    /** Returns the number of tabs in the bar. */
    int getNumTabs() const;

    /** Returns a list of all the tab names in the bar. */
    StringArray getTabNames() const;

    /** Changes the currently selected tab.
        This will send a change message and cause a synchronous callback to
        the currentTabChanged() method. (But if the given tab is already selected,
        nothing will be done).

        To deselect all the tabs, use an index of -1.
    */
    void setCurrentTabIndex (int newTabIndex, bool sendChangeMessage = true);

    /** Returns the name of the currently selected tab.
        This could be an empty string if none are selected.
    */
    String getCurrentTabName() const;

    /** Returns the index of the currently selected tab.
        This could return -1 if none are selected.
    */
    int getCurrentTabIndex() const noexcept             { return currentTabIndex; }

    /** Returns the button for a specific tab.
        The button that is returned may be deleted later by this component, so don't hang
        on to the pointer that is returned. A null pointer may be returned if the index is
        out of range.
    */
    TabBarButton* getTabButton (int index) const;

    /** Returns the index of a TabBarButton if it belongs to this bar. */
    int indexOfTabButton (const TabBarButton* button) const;

    //==============================================================================
    /** Callback method to indicate the selected tab has been changed.
        @see setCurrentTabIndex
    */
    virtual void currentTabChanged (int newCurrentTabIndex,
                                    const String& newCurrentTabName);

    /** Callback method to indicate that the user has right-clicked on a tab. */
    virtual void popupMenuClickOnTab (int tabIndex, const String& tabName);

    /** Returns the colour of a tab.
        This is the colour that was specified in addTab().
    */
    Colour getTabBackgroundColour (int tabIndex);

    /** Changes the background colour of a tab.
        @see addTab, getTabBackgroundColour
    */
    void setTabBackgroundColour (int tabIndex, Colour newColour);

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the component.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        tabOutlineColourId              = 0x1005812,    /**< The colour to use to draw an outline around the tabs.  */
        tabTextColourId                 = 0x1005813,    /**< The colour to use to draw the tab names. If this isn't specified,
                                                             the look and feel will choose an appropriate colour. */
        frontOutlineColourId            = 0x1005814,    /**< The colour to use to draw an outline around the currently-selected tab.  */
        frontTextColourId               = 0x1005815,    /**< The colour to use to draw the currently-selected tab name. If
                                                             this isn't specified, the look and feel will choose an appropriate
                                                             colour. */
    };

    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    void resized() override;
    /** @internal */
    void lookAndFeelChanged() LE_PATCH( noexcept ) override;

protected:
    //==============================================================================
    /** This creates one of the tabs.

        If you need to use custom tab components, you can override this method and
        return your own class instead of the default.
    */
    virtual TabBarButton* createTabButton (const String& tabName, int tabIndex);

private:
    Orientation orientation;

    struct TabInfo
    {
        ScopedPointer<TabBarButton> button;
        String name;
        Colour colour;
    };

    OwnedArray <TabInfo> tabs;

    double minimumScale;
    int currentTabIndex;

    class BehindFrontTabComp;
    friend class BehindFrontTabComp;
    friend struct ContainerDeletePolicy<BehindFrontTabComp>;
    ScopedPointer<BehindFrontTabComp> behindFrontTab;
    ScopedPointer<Button> extraTabsButton;

    void showExtraItemsMenu();
    static void extraItemsMenuCallback (int, TabbedButtonBar*);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TabbedButtonBar)
};


#endif   // JUCE_TABBEDBUTTONBAR_H_INCLUDED
