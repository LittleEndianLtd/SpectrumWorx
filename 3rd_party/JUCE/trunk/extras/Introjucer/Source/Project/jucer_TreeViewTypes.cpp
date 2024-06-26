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

#include "jucer_TreeViewTypes.h"
#include "jucer_ConfigPage.h"
#include "jucer_GroupInformationComponent.h"
#include "../Application/jucer_OpenDocumentManager.h"
#include "../Code Editor/jucer_SourceCodeEditor.h"
#include "jucer_NewFileWizard.h"
#include "jucer_ProjectContentComponent.h"

//==============================================================================
GroupTreeViewItem::GroupTreeViewItem (const Project::Item& i)
    : ProjectTreeViewBase (i)
{
}

GroupTreeViewItem::~GroupTreeViewItem()
{
}

void GroupTreeViewItem::addNewGroup()
{
    Project::Item newGroup (item.addNewSubGroup ("New Group", 0));
    triggerAsyncRename (newGroup);
}

bool GroupTreeViewItem::acceptsDragItems (const OwnedArray <Project::Item>& selectedNodes)
{
    for (int i = selectedNodes.size(); --i >= 0;)
        if (item.canContain (*selectedNodes.getUnchecked(i)))
            return true;

    return false;
}

void GroupTreeViewItem::addFiles (const StringArray& files, int insertIndex)
{
    for (int i = 0; i < files.size(); ++i)
    {
        const File file (files[i]);

        if (item.addFile (file, insertIndex, true))
            ++insertIndex;
    }
}

void GroupTreeViewItem::moveSelectedItemsTo (OwnedArray <Project::Item>& selectedNodes, int insertIndex)
{
    moveItems (selectedNodes, item, insertIndex);
}

void GroupTreeViewItem::checkFileStatus()
{
    for (int i = 0; i < getNumSubItems(); ++i)
        if (ProjectTreeViewBase* p = dynamic_cast <ProjectTreeViewBase*> (getSubItem(i)))
            p->checkFileStatus();
}

ProjectTreeViewBase* GroupTreeViewItem::createSubItem (const Project::Item& child)
{
    if (child.isGroup())   return new GroupTreeViewItem (child);
    if (child.isFile())    return new SourceFileTreeViewItem (child);

    jassertfalse;
    return nullptr;
}

void GroupTreeViewItem::showDocument()
{
    if (ProjectContentComponent* pcc = getProjectContentComponent())
        pcc->setEditorComponent (new GroupInformationComponent (item), nullptr);
}

static void openOrCloseAllSubGroups (TreeViewItem& item, bool shouldOpen)
{
    item.setOpen (shouldOpen);

    for (int i = item.getNumSubItems(); --i >= 0;)
        if (TreeViewItem* sub = item.getSubItem(i))
            openOrCloseAllSubGroups (*sub, shouldOpen);
}

static void setFilesToCompile (Project::Item item, const bool shouldCompile)
{
    if (item.isFile())
        item.getShouldCompileValue() = shouldCompile;

    for (int i = item.getNumChildren(); --i >= 0;)
        setFilesToCompile (item.getChild (i), shouldCompile);
}

void GroupTreeViewItem::showPopupMenu()
{
    PopupMenu m;
    addCreateFileMenuItems (m);

    m.addSeparator();

    if (isOpen())
        m.addItem (1, "Collapse all Sub-groups");
    else
        m.addItem (2, "Expand all Sub-groups");

    m.addSeparator();
    m.addItem (3, "Enable compiling of all enclosed files");
    m.addItem (4, "Disable compiling of all enclosed files");

    m.addSeparator();
    m.addItem (5, "Sort Items Alphabetically");
    m.addItem (6, "Sort Items Alphabetically (Groups first)");
    m.addSeparator();
    m.addItem (7, "Rename...");

    if (! isRoot())
        m.addItem (8, "Delete");

    launchPopupMenu (m);
}

void GroupTreeViewItem::handlePopupMenuResult (int resultCode)
{
    switch (resultCode)
    {
        case 1:     openOrCloseAllSubGroups (*this, false); break;
        case 2:     openOrCloseAllSubGroups (*this, true); break;
        case 3:     setFilesToCompile (item, true); break;
        case 4:     setFilesToCompile (item, false); break;
        case 5:     item.sortAlphabetically (false); break;
        case 6:     item.sortAlphabetically (true); break;
        case 7:     triggerAsyncRename (item); break;
        case 8:     deleteAllSelectedItems(); break;
        default:    processCreateFileMenuItem (resultCode); break;
    }
}

void GroupTreeViewItem::addCreateFileMenuItems (PopupMenu& m)
{
    m.addItem (1001, "Add New Group");
    m.addItem (1002, "Add Existing Files...");

    m.addSeparator();
    NewFileWizard().addWizardsToMenu (m);
}

void GroupTreeViewItem::processCreateFileMenuItem (int menuID)
{
    switch (menuID)
    {
        case 1001:  addNewGroup(); break;
        case 1002:  browseToAddExistingFiles(); break;

        default:
            NewFileWizard().runWizardFromMenu (menuID, item);
            break;
    }
}

//==============================================================================
//==============================================================================
SourceFileTreeViewItem::SourceFileTreeViewItem (const Project::Item& item_)
    : ProjectTreeViewBase (item_)
{
}

SourceFileTreeViewItem::~SourceFileTreeViewItem()
{
}

String SourceFileTreeViewItem::getDisplayName() const
{
    return getFile().getFileName();
}

static File findCorrespondingHeaderOrCpp (const File& f)
{
    if (f.hasFileExtension (sourceFileExtensions))
        return f.withFileExtension (".h");
    else if (f.hasFileExtension (headerFileExtensions))
        return f.withFileExtension (".cpp");

    return File::nonexistent;
}

void SourceFileTreeViewItem::setName (const String& newName)
{
    if (newName != File::createLegalFileName (newName))
    {
        AlertWindow::showMessageBox (AlertWindow::WarningIcon, "File Rename",
                                     "That filename contained some illegal characters!");
        triggerAsyncRename (item);
        return;
    }

    File oldFile (getFile());
    File newFile (oldFile.getSiblingFile (newName));
    File correspondingFile (findCorrespondingHeaderOrCpp (oldFile));

    if (correspondingFile.exists() && newFile.hasFileExtension (oldFile.getFileExtension()))
    {
        Project::Item correspondingItem (item.project.getMainGroup().findItemForFile (correspondingFile));

        if (correspondingItem.isValid())
        {
            if (AlertWindow::showOkCancelBox (AlertWindow::NoIcon, "File Rename",
                                              "Do you also want to rename the corresponding file \"" + correspondingFile.getFileName()
                                                + "\" to match?"))
            {
                if (! item.renameFile (newFile))
                {
                    AlertWindow::showMessageBox (AlertWindow::WarningIcon, "File Rename",
                                                 "Failed to rename \"" + oldFile.getFullPathName() + "\"!\n\nCheck your file permissions!");
                    return;
                }

                if (! correspondingItem.renameFile (newFile.withFileExtension (correspondingFile.getFileExtension())))
                {
                    AlertWindow::showMessageBox (AlertWindow::WarningIcon, "File Rename",
                                                 "Failed to rename \"" + correspondingFile.getFullPathName() + "\"!\n\nCheck your file permissions!");
                }
            }
        }
    }

    if (! item.renameFile (newFile))
    {
        AlertWindow::showMessageBox (AlertWindow::WarningIcon, "File Rename",
                                     "Failed to rename the file!\n\nCheck your file permissions!");
    }
}

ProjectTreeViewBase* SourceFileTreeViewItem::createSubItem (const Project::Item&)
{
    jassertfalse;
    return nullptr;
}

void SourceFileTreeViewItem::showDocument()
{
    const File f (getFile());

    if (f.exists())
        if (ProjectContentComponent* pcc = getProjectContentComponent())
            pcc->showEditorForFile (f, false);
}

void SourceFileTreeViewItem::showPopupMenu()
{
    PopupMenu m;

    if (GroupTreeViewItem* parentGroup = dynamic_cast <GroupTreeViewItem*> (getParentProjectItem()))
    {
        parentGroup->addCreateFileMenuItems (m);
        m.addSeparator();
    }

    m.addItem (1, "Open in external editor");
    m.addItem (2,
                 #if JUCE_MAC
                  "Reveal in Finder");
                 #else
                  "Reveal in Explorer");
                 #endif

    m.addItem (4, "Rename File...");
    m.addSeparator();
    m.addItem (3, "Delete");

    launchPopupMenu (m);
}

void SourceFileTreeViewItem::handlePopupMenuResult (int resultCode)
{
    switch (resultCode)
    {
        case 1:     getFile().startAsProcess(); break;
        case 2:     revealInFinder(); break;
        case 3:     deleteAllSelectedItems(); break;
        case 4:     triggerAsyncRename (item); break;

        default:
            if (GroupTreeViewItem* parentGroup = dynamic_cast <GroupTreeViewItem*> (getParentProjectItem()))
                parentGroup->processCreateFileMenuItem (resultCode);

            break;
    }
}
