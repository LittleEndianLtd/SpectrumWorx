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

#ifndef JUCE_MENUBARMODEL_H_INCLUDED
#define JUCE_MENUBARMODEL_H_INCLUDED


//==============================================================================
/**
    A class for controlling MenuBar components.

    This class is used to tell a MenuBar what menus to show, and to respond
    to a menu being selected.

    @see MenuBarModel::Listener, MenuBarComponent, PopupMenu
*/
class JUCE_API  MenuBarModel      : private AsyncUpdater JUCE_ORIGINAL( , )
                                    JUCE_ORIGINAL( private ApplicationCommandManagerListener )
{
public:
    //==============================================================================
    MenuBarModel() noexcept;

    /** Destructor. */
    virtual ~MenuBarModel();

    //==============================================================================
    /** Call this when some of your menu items have changed.

        This method will cause a callback to any MenuBarListener objects that
        are registered with this model.

        If this model is displaying items from an ApplicationCommandManager, you
        can use the setApplicationCommandManagerToWatch() method to cause
        change messages to be sent automatically when the ApplicationCommandManager
        is changed.

        @see addListener, removeListener, MenuBarListener
    */
    void menuItemsChanged();

    /** Tells the menu bar to listen to the specified command manager, and to update
        itself when the commands change.

        This will also allow it to flash a menu name when a command from that menu
        is invoked using a keystroke.
    */
    void setApplicationCommandManagerToWatch (ApplicationCommandManager* manager) noexcept;

    //==============================================================================
    /** A class to receive callbacks when a MenuBarModel changes.

        @see MenuBarModel::addListener, MenuBarModel::removeListener, MenuBarModel::menuItemsChanged
    */
    class JUCE_API  Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() {}

        //==============================================================================
        /** This callback is made when items are changed in the menu bar model.
        */
        virtual void menuBarItemsChanged (MenuBarModel* menuBarModel) = 0;

        /** This callback is made when an application command is invoked that
            is represented by one of the items in the menu bar model.
        */
        virtual void menuCommandInvoked (MenuBarModel* menuBarModel,
                                         const ApplicationCommandTarget::InvocationInfo& info) = 0;
    };

    /** Registers a listener for callbacks when the menu items in this model change.

        The listener object will get callbacks when this object's menuItemsChanged()
        method is called.

        @see removeListener
    */
    void addListener (Listener* listenerToAdd) noexcept;

    /** Removes a listener.

        @see addListener
    */
    void removeListener (Listener* listenerToRemove) noexcept;

    //==============================================================================
    /** This method must return a list of the names of the menus. */
    virtual StringArray getMenuBarNames() = 0;

    /** This should return the popup menu to display for a given top-level menu.

        @param topLevelMenuIndex    the index of the top-level menu to show
        @param menuName             the name of the top-level menu item to show
    */
    virtual PopupMenu getMenuForIndex (int topLevelMenuIndex,
                                       const String& menuName) = 0;

    /** This is called when a menu item has been clicked on.

        @param menuItemID           the item ID of the PopupMenu item that was selected
        @param topLevelMenuIndex    the index of the top-level menu from which the item was
                                    chosen (just in case you've used duplicate ID numbers
                                    on more than one of the popup menus)
    */
    virtual void menuItemSelected (int menuItemID,
                                   int topLevelMenuIndex) = 0;

    //==============================================================================
   #if JUCE_MAC || DOXYGEN
    /** MAC ONLY - Sets the model that is currently being shown as the main
        menu bar at the top of the screen on the Mac.

        You can pass 0 to stop the current model being displayed. Be careful
        not to delete a model while it is being used.

        An optional extra menu can be specified, containing items to add to the top of
        the apple menu. (Confusingly, the 'apple' menu isn't the one with a picture of
        an apple, it's the one next to it, with your application's name at the top
        and the services menu etc on it). When one of these items is selected, the
        menu bar model will be used to invoke it, and in the menuItemSelected() callback
        the topLevelMenuIndex parameter will be -1. If you pass in an extraAppleMenuItems
        object then newMenuBarModel must be non-null.

        If the recentItemsMenuName parameter is non-empty, then any sub-menus with this
        name will be replaced by OSX's special recent-files menu.
    */
    static void setMacMainMenu (MenuBarModel* newMenuBarModel,
                                const PopupMenu* extraAppleMenuItems = nullptr,
                                const String& recentItemsMenuName = String::empty);

    /** MAC ONLY - Returns the menu model that is currently being shown as
        the main menu bar.
    */
    static MenuBarModel* getMacMainMenu();

    /** MAC ONLY - Returns the menu that was last passed as the extraAppleMenuItems
        argument to setMacMainMenu(), or nullptr if none was specified.
    */
    static const PopupMenu* getMacExtraAppleItemsMenu();
   #endif

    //==============================================================================
    /** @internal */
    void applicationCommandInvoked (const ApplicationCommandTarget::InvocationInfo& info) override;
    /** @internal */
    void applicationCommandListChanged() override;
    /** @internal */
    void handleAsyncUpdate() override;

private:
    ApplicationCommandManager* manager;
    ListenerList <Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MenuBarModel)
};

/** This typedef is just for compatibility with old code - newer code should use the MenuBarModel::Listener class directly. */
typedef MenuBarModel::Listener MenuBarModelListener;


#endif   // JUCE_MENUBARMODEL_H_INCLUDED
