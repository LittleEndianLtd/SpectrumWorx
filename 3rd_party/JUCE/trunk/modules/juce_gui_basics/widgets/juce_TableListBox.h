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

#ifndef JUCE_TABLELISTBOX_H_INCLUDED
#define JUCE_TABLELISTBOX_H_INCLUDED


//==============================================================================
/**
    One of these is used by a TableListBox as the data model for the table's contents.

    The virtual methods that you override in this class take care of drawing the
    table cells, and reacting to events.

    @see TableListBox
*/
class JUCE_API  TableListBoxModel
{
public:
    //==============================================================================
    TableListBoxModel()  {}

    /** Destructor. */
    virtual ~TableListBoxModel()  {}

    //==============================================================================
    /** This must return the number of rows currently in the table.

        If the number of rows changes, you must call TableListBox::updateContent() to
        cause it to refresh the list.
    */
    virtual int getNumRows() = 0;

    /** This must draw the background behind one of the rows in the table.

        The graphics context has its origin at the row's top-left, and your method
        should fill the area specified by the width and height parameters.
    */
    virtual void paintRowBackground (Graphics& g,
                                     int rowNumber,
                                     int width, int height,
                                     bool rowIsSelected) = 0;

    /** This must draw one of the cells.

        The graphics context's origin will already be set to the top-left of the cell,
        whose size is specified by (width, height).
    */
    virtual void paintCell (Graphics& g,
                            int rowNumber,
                            int columnId,
                            int width, int height,
                            bool rowIsSelected) = 0;

    //==============================================================================
    /** This is used to create or update a custom component to go in a cell.

        Any cell may contain a custom component, or can just be drawn with the paintCell() method
        and handle mouse clicks with cellClicked().

        This method will be called whenever a custom component might need to be updated - e.g.
        when the table is changed, or TableListBox::updateContent() is called.

        If you don't need a custom component for the specified cell, then return nullptr.
        (Bear in mind that even if you're not creating a new component, you may still need to
        delete existingComponentToUpdate if it's non-null).

        If you do want a custom component, and the existingComponentToUpdate is null, then
        this method must create a new component suitable for the cell, and return it.

        If the existingComponentToUpdate is non-null, it will be a pointer to a component previously created
        by this method. In this case, the method must either update it to make sure it's correctly representing
        the given cell (which may be different from the one that the component was created for), or it can
        delete this component and return a new one.
    */
    virtual Component* refreshComponentForCell (int rowNumber, int columnId, bool isRowSelected,
                                                Component* existingComponentToUpdate);

    //==============================================================================
    /** This callback is made when the user clicks on one of the cells in the table.

        The mouse event's coordinates will be relative to the entire table row.
        @see cellDoubleClicked, backgroundClicked
    */
    virtual void cellClicked (int rowNumber, int columnId, const MouseEvent& e);

    /** This callback is made when the user clicks on one of the cells in the table.

        The mouse event's coordinates will be relative to the entire table row.
        @see cellClicked, backgroundClicked
    */
    virtual void cellDoubleClicked (int rowNumber, int columnId, const MouseEvent& e);

    /** This can be overridden to react to the user double-clicking on a part of the list where
        there are no rows.

        @see cellClicked
    */
    virtual void backgroundClicked();

    //==============================================================================
    /** This callback is made when the table's sort order is changed.

        This could be because the user has clicked a column header, or because the
        TableHeaderComponent::setSortColumnId() method was called.

        If you implement this, your method should re-sort the table using the given
        column as the key.
    */
    virtual void sortOrderChanged (int newSortColumnId, bool isForwards);

    //==============================================================================
    /** Returns the best width for one of the columns.

        If you implement this method, you should measure the width of all the items
        in this column, and return the best size.

        Returning 0 means that the column shouldn't be changed.

        This is used by TableListBox::autoSizeColumn() and TableListBox::autoSizeAllColumns().
    */
    virtual int getColumnAutoSizeWidth (int columnId);

    /** Returns a tooltip for a particular cell in the table.
    */
    virtual String getCellTooltip (int rowNumber, int columnId);

    //==============================================================================
    /** Override this to be informed when rows are selected or deselected.

        @see ListBox::selectedRowsChanged()
    */
    virtual void selectedRowsChanged (int lastRowSelected);

    /** Override this to be informed when the delete key is pressed.

        @see ListBox::deleteKeyPressed()
    */
    virtual void deleteKeyPressed (int lastRowSelected);

    /** Override this to be informed when the return key is pressed.

        @see ListBox::returnKeyPressed()
    */
    virtual void returnKeyPressed (int lastRowSelected);

    /** Override this to be informed when the list is scrolled.

        This might be caused by the user moving the scrollbar, or by programmatic changes
        to the list position.
    */
    virtual void listWasScrolled();

    /** To allow rows from your table to be dragged-and-dropped, implement this method.

        If this returns a non-null variant then when the user drags a row, the table will try to
        find a DragAndDropContainer in its parent hierarchy, and will use it to trigger a
        drag-and-drop operation, using this string as the source description, and the listbox
        itself as the source component.

        @see getDragSourceCustomData, DragAndDropContainer::startDragging
    */
    virtual var getDragSourceDescription (const SparseSet<int>& currentlySelectedRows);
};


//==============================================================================
/**
    A table of cells, using a TableHeaderComponent as its header.

    This component makes it easy to create a table by providing a TableListBoxModel as
    the data source.


    @see TableListBoxModel, TableHeaderComponent
*/
class JUCE_API  TableListBox   : public ListBox,
                                 private ListBoxModel,
                                 private TableHeaderComponent::Listener
{
public:
    //==============================================================================
    /** Creates a TableListBox.

        The model pointer passed-in can be null, in which case you can set it later
        with setModel().
    */
    TableListBox (const String& componentName = String::empty,
                  TableListBoxModel* model = 0);

    /** Destructor. */
    ~TableListBox();

    //==============================================================================
    /** Changes the TableListBoxModel that is being used for this table.
    */
    void setModel (TableListBoxModel* newModel);

    /** Returns the model currently in use. */
    TableListBoxModel* getModel() const                             { return model; }

    //==============================================================================
    /** Returns the header component being used in this table. */
    TableHeaderComponent& getHeader() const                         { return *header; }

    /** Sets the header component to use for the table.
        The table will take ownership of the component that you pass in, and will delete it
        when it's no longer needed.
    */
    void setHeader (TableHeaderComponent* newHeader);

    /** Changes the height of the table header component.
        @see getHeaderHeight
    */
    void setHeaderHeight (int newHeight);

    /** Returns the height of the table header.
        @see setHeaderHeight
    */
    int getHeaderHeight() const;

    //==============================================================================
    /** Resizes a column to fit its contents.

        This uses TableListBoxModel::getColumnAutoSizeWidth() to find the best width,
        and applies that to the column.

        @see autoSizeAllColumns, TableHeaderComponent::setColumnWidth
    */
    void autoSizeColumn (int columnId);

    /** Calls autoSizeColumn() for all columns in the table. */
    void autoSizeAllColumns();

    /** Enables or disables the auto size options on the popup menu.

        By default, these are enabled.
    */
    void setAutoSizeMenuOptionShown (bool shouldBeShown);

    /** True if the auto-size options should be shown on the menu.
        @see setAutoSizeMenuOptionsShown
    */
    bool isAutoSizeMenuOptionShown() const;

    /** Returns the position of one of the cells in the table.

        If relativeToComponentTopLeft is true, the coordinates are relative to
        the table component's top-left. The row number isn't checked to see if it's
        in-range, but the column ID must exist or this will return an empty rectangle.

        If relativeToComponentTopLeft is false, the coordinates are relative to the
        top-left of the table's top-left cell.
    */
    Rectangle<int> getCellPosition (int columnId, int rowNumber,
                                    bool relativeToComponentTopLeft) const;

    /** Returns the component that currently represents a given cell.
        If the component for this cell is off-screen or if the position is out-of-range,
        this may return 0.
        @see getCellPosition
    */
    Component* getCellComponent (int columnId, int rowNumber) const;

    /** Scrolls horizontally if necessary to make sure that a particular column is visible.

        @see ListBox::scrollToEnsureRowIsOnscreen
    */
    void scrollToEnsureColumnIsOnscreen (int columnId);

    //==============================================================================
    /** @internal */
    int getNumRows() LE_PATCH( noexcept );
    /** @internal */
    void paintListBoxItem (int, Graphics&, int, int, bool);
    /** @internal */
    Component* refreshComponentForRow (int rowNumber, bool isRowSelected, Component* existingComponentToUpdate);
    /** @internal */
    void selectedRowsChanged (int lastRowSelected);
    /** @internal */
    void deleteKeyPressed (int currentSelectedRow) LE_PATCH( noexcept );
    /** @internal */
    void returnKeyPressed (int currentSelectedRow);
    /** @internal */
    void backgroundClicked() LE_PATCH( noexcept );
    /** @internal */
    void listWasScrolled() LE_PATCH( noexcept );
    /** @internal */
    void tableColumnsChanged (TableHeaderComponent*);
    /** @internal */
    void tableColumnsResized (TableHeaderComponent*);
    /** @internal */
    void tableSortOrderChanged (TableHeaderComponent*);
    /** @internal */
    void tableColumnDraggingChanged (TableHeaderComponent*, int);
    /** @internal */
    void resized();


private:
    //==============================================================================
    class Header;
    class RowComp;

    TableHeaderComponent* header;
    TableListBoxModel* model;
    int columnIdNowBeingDragged;
    bool autoSizeOptionsShown;

    void updateColumnComponents() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TableListBox)
};


#endif   // JUCE_TABLELISTBOX_H_INCLUDED
