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

#include "jucer_ProjectTreeViewBase.h"
#include "../Application/jucer_Application.h"


//==============================================================================
ProjectTreeViewBase::ProjectTreeViewBase (const Project::Item& item_)
    : item (item_), isFileMissing (false)
{
    item.state.addListener (this);
}

ProjectTreeViewBase::~ProjectTreeViewBase()
{
    item.state.removeListener (this);
}

//==============================================================================
String ProjectTreeViewBase::getDisplayName() const
{
    return item.getName();
}

void ProjectTreeViewBase::setName (const String& newName)
{
    if (item.isMainGroup())
        item.project.setTitle (newName);
    else
        item.getNameValue() = newName;
}

//==============================================================================
File ProjectTreeViewBase::getFile() const
{
    return item.getFile();
}

void ProjectTreeViewBase::browseToAddExistingFiles()
{
    const File location (item.isGroup() ? item.determineGroupFolder() : getFile());
    FileChooser fc ("Add Files to Jucer Project", location, String::empty, false);

    if (fc.browseForMultipleFilesOrDirectories())
    {
        StringArray files;

        for (int i = 0; i < fc.getResults().size(); ++i)
            files.add (fc.getResults().getReference(i).getFullPathName());

        addFiles (files, 0);
    }
}

void ProjectTreeViewBase::addFiles (const StringArray& files, int insertIndex)
{
    if (ProjectTreeViewBase* p = dynamic_cast <ProjectTreeViewBase*> (getParentItem()))
        p->addFiles (files, insertIndex);
}

void ProjectTreeViewBase::moveSelectedItemsTo (OwnedArray <Project::Item>&, int /*insertIndex*/)
{
    jassertfalse;
}

//==============================================================================
ProjectTreeViewBase* ProjectTreeViewBase::findTreeViewItem (const Project::Item& itemToFind)
{
    if (item == itemToFind)
        return this;

    const bool wasOpen = isOpen();
    setOpen (true);

    for (int i = getNumSubItems(); --i >= 0;)
    {
        if (ProjectTreeViewBase* pg = dynamic_cast <ProjectTreeViewBase*> (getSubItem(i)))
            if (ProjectTreeViewBase* found = pg->findTreeViewItem (itemToFind))
                return found;
    }

    setOpen (wasOpen);
    return nullptr;
}

//==============================================================================
void ProjectTreeViewBase::triggerAsyncRename (const Project::Item& itemToRename)
{
    class RenameMessage  : public CallbackMessage
    {
    public:
        RenameMessage (TreeView* const t, const Project::Item& i)
            : tree (t), itemToRename (i)  {}

        void messageCallback() override
        {
            if (tree != nullptr)
                if (ProjectTreeViewBase* root = dynamic_cast <ProjectTreeViewBase*> (tree->getRootItem()))
                    if (ProjectTreeViewBase* found = root->findTreeViewItem (itemToRename))
                        found->showRenameBox();
        }

    private:
        Component::SafePointer<TreeView> tree;
        Project::Item itemToRename;
    };

    (new RenameMessage (getOwnerView(), itemToRename))->post();
}

//==============================================================================
void ProjectTreeViewBase::checkFileStatus()
{
    const File file (getFile());
    const bool nowMissing = file != File::nonexistent && ! file.exists();

    if (nowMissing != isFileMissing)
    {
        isFileMissing = nowMissing;
        repaintItem();
    }
}

void ProjectTreeViewBase::revealInFinder() const
{
    getFile().revealToUser();
}

void ProjectTreeViewBase::deleteItem()
{
    item.removeItemFromProject();
}

void ProjectTreeViewBase::deleteAllSelectedItems()
{
    TreeView* tree = getOwnerView();
    const int numSelected = tree->getNumSelectedItems();
    OwnedArray <File> filesToTrash;
    OwnedArray <Project::Item> itemsToRemove;

    for (int i = 0; i < numSelected; ++i)
    {
        if (const ProjectTreeViewBase* const p = dynamic_cast <ProjectTreeViewBase*> (tree->getSelectedItem (i)))
        {
            itemsToRemove.add (new Project::Item (p->item));

            if (p->getFile().existsAsFile())
                filesToTrash.add (new File (p->getFile()));
        }
    }

    if (filesToTrash.size() > 0)
    {
        String fileList;
        const int maxFilesToList = 10;
        for (int i = jmin (maxFilesToList, filesToTrash.size()); --i >= 0;)
            fileList << filesToTrash.getUnchecked(i)->getFullPathName() << "\n";

        if (filesToTrash.size() > maxFilesToList)
            fileList << "\n...plus " << (filesToTrash.size() - maxFilesToList) << " more files...";

        int r = AlertWindow::showYesNoCancelBox (AlertWindow::NoIcon, "Delete Project Items",
                                                 "As well as removing the selected item(s) from the project, do you also want to move their files to the trash:\n\n"
                                                   + fileList,
                                                 "Just remove references",
                                                 "Also move files to Trash",
                                                 "Cancel",
                                                 tree->getTopLevelComponent());

        if (r == 0)
            return;

        if (r != 2)
            filesToTrash.clear();
    }

    if (ProjectTreeViewBase* treeRootItem = dynamic_cast <ProjectTreeViewBase*> (tree->getRootItem()))
    {
        OpenDocumentManager& om = IntrojucerApp::getApp().openDocumentManager;

        for (int i = filesToTrash.size(); --i >= 0;)
        {
            const File f (*filesToTrash.getUnchecked(i));

            om.closeFile (f, false);

            if (! f.moveToTrash())
            {
                // xxx
            }
        }

        for (int i = itemsToRemove.size(); --i >= 0;)
        {
            if (ProjectTreeViewBase* itemToRemove = treeRootItem->findTreeViewItem (*itemsToRemove.getUnchecked(i)))
            {
                om.closeFile (itemToRemove->getFile(), false);
                itemToRemove->deleteItem();
            }
        }
    }
    else
    {
        jassertfalse;
    }
}

static int indexOfNode (const ValueTree& parent, const ValueTree& child)
{
    for (int i = parent.getNumChildren(); --i >= 0;)
        if (parent.getChild (i) == child)
            return i;

    return -1;
}

void ProjectTreeViewBase::moveItems (OwnedArray <Project::Item>& selectedNodes,
                                     Project::Item destNode, int insertIndex)
{
    for (int i = selectedNodes.size(); --i >= 0;)
    {
        Project::Item* const n = selectedNodes.getUnchecked(i);

        if (destNode == *n || destNode.state.isAChildOf (n->state)) // Check for recursion.
            return;

        if (! destNode.canContain (*n))
            selectedNodes.remove (i);
    }

    // Don't include any nodes that are children of other selected nodes..
    for (int i = selectedNodes.size(); --i >= 0;)
    {
        Project::Item* const n = selectedNodes.getUnchecked(i);

        for (int j = selectedNodes.size(); --j >= 0;)
        {
            if (j != i && n->state.isAChildOf (selectedNodes.getUnchecked(j)->state))
            {
                selectedNodes.remove (i);
                break;
            }
        }
    }

    // Remove and re-insert them one at a time..
    for (int i = 0; i < selectedNodes.size(); ++i)
    {
        Project::Item* selectedNode = selectedNodes.getUnchecked(i);

        if (selectedNode->state.getParent() == destNode.state
              && indexOfNode (destNode.state, selectedNode->state) < insertIndex)
            --insertIndex;

        selectedNode->removeItemFromProject();
        destNode.addChild (*selectedNode, insertIndex++);
    }
}

//==============================================================================
bool ProjectTreeViewBase::isInterestedInFileDrag (const StringArray& files)
{
    return acceptsFileDrop (files);
}

void ProjectTreeViewBase::filesDropped (const StringArray& files, int insertIndex)
{
    addFiles (files, insertIndex);
}

void ProjectTreeViewBase::getAllSelectedNodesInTree (Component* componentInTree, OwnedArray <Project::Item>& selectedNodes)
{
    TreeView* tree = dynamic_cast <TreeView*> (componentInTree);

    if (tree == nullptr)
        tree = componentInTree->findParentComponentOfClass<TreeView>();

    if (tree != nullptr)
    {
        const int numSelected = tree->getNumSelectedItems();

        for (int i = 0; i < numSelected; ++i)
            if (const ProjectTreeViewBase* const p = dynamic_cast <ProjectTreeViewBase*> (tree->getSelectedItem (i)))
                selectedNodes.add (new Project::Item (p->item));
    }
}

bool ProjectTreeViewBase::isInterestedInDragSource (const DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    if (dragSourceDetails.description != projectItemDragType)
        return false;

    OwnedArray <Project::Item> selectedNodes;
    getAllSelectedNodesInTree (dragSourceDetails.sourceComponent, selectedNodes);

    return selectedNodes.size() > 0 && acceptsDragItems (selectedNodes);
}

void ProjectTreeViewBase::itemDropped (const DragAndDropTarget::SourceDetails& dragSourceDetails, int insertIndex)
{
    OwnedArray <Project::Item> selectedNodes;
    getAllSelectedNodesInTree (dragSourceDetails.sourceComponent, selectedNodes);

    if (selectedNodes.size() > 0)
    {
        TreeView* tree = getOwnerView();
        ScopedPointer <XmlElement> oldOpenness (tree->getOpennessState (false));

        moveSelectedItemsTo (selectedNodes, insertIndex);

        if (oldOpenness != nullptr)
            tree->restoreOpennessState (*oldOpenness, false);
    }
}

//==============================================================================
void ProjectTreeViewBase::treeChildrenChanged (const ValueTree& parentTree)
{
    if (parentTree == item.state)
    {
        refreshSubItems();
        treeHasChanged();
        setOpen (true);
    }
}

void ProjectTreeViewBase::valueTreePropertyChanged (ValueTree& tree, const Identifier&)
{
    if (tree == item.state)
        repaintItem();
}

void ProjectTreeViewBase::valueTreeChildAdded (ValueTree& parentTree, ValueTree&)
{
    treeChildrenChanged (parentTree);
}

void ProjectTreeViewBase::valueTreeChildRemoved (ValueTree& parentTree, ValueTree&)
{
    treeChildrenChanged (parentTree);
}

void ProjectTreeViewBase::valueTreeChildOrderChanged (ValueTree& parentTree)
{
    treeChildrenChanged (parentTree);
}

void ProjectTreeViewBase::valueTreeParentChanged (ValueTree&)
{
}

//==============================================================================
bool ProjectTreeViewBase::mightContainSubItems()
{
    return item.getNumChildren() > 0;
}

String ProjectTreeViewBase::getUniqueName() const
{
    jassert (item.getID().isNotEmpty());
    return item.getID();
}

void ProjectTreeViewBase::itemOpennessChanged (bool isNowOpen)
{
    if (isNowOpen)
        refreshSubItems();
}

void ProjectTreeViewBase::addSubItems()
{
    for (int i = 0; i < item.getNumChildren(); ++i)
        if (ProjectTreeViewBase* p = createSubItem (item.getChild(i)))
            addSubItem (p);
}

static void treeViewMultiSelectItemChosen (int resultCode, ProjectTreeViewBase* item)
{
    switch (resultCode)
    {
        case 1:     item->deleteAllSelectedItems(); break;
        default:    break;
    }
}

void ProjectTreeViewBase::showMultiSelectionPopupMenu()
{
    PopupMenu m;
    m.addItem (1, "Delete");

    m.showMenuAsync (PopupMenu::Options(),
                     ModalCallbackFunction::create (treeViewMultiSelectItemChosen, this));
}

String ProjectTreeViewBase::getTooltip()
{
    return String::empty;
}

var ProjectTreeViewBase::getDragSourceDescription()
{
    cancelDelayedSelectionTimer();
    return projectItemDragType;
}

int ProjectTreeViewBase::getMillisecsAllowedForDragGesture()
{
    // for images, give the user longer to start dragging before assuming they're
    // clicking to select it for previewing..
    return item.isImageFile() ? 250 : JucerTreeViewBase::getMillisecsAllowedForDragGesture();
}

//==============================================================================
ProjectTreeViewBase* ProjectTreeViewBase::getParentProjectItem() const
{
    return dynamic_cast <ProjectTreeViewBase*> (getParentItem());
}
