////////////////////////////////////////////////////////////////////////////////
///
/// presetBrowser.cpp
/// -----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "presetBrowser.hpp"

#include "configuration/versionConfiguration.hpp"
#include "gui/editor/spectrumWorxEditor.hpp"

#include "le/parameters/uiElements.hpp"
#include "le/spectrumworx/presets.hpp"
#include "le/utility/countof.hpp"
#include "le/utility/tchar.hpp"

#include "juce/beginIncludes.hpp"
    //...mrmlj...juce missing includes...
    #include "juce/juce_core/files/juce_DirectoryIterator.h"
    #include "juce/juce_core/threads/juce_TimeSliceThread.h"

    #include "juce/juce_gui_basics/filebrowser/juce_DirectoryContentsDisplayComponent.h"
    #include "juce/juce_gui_basics/filebrowser/juce_FileChooser.h"
#include "juce/endIncludes.hpp"

// Boost sandbox
#include "boost/filesystem/directory_iterator.hpp"
#include "boost/mmap/mappble_objects/file/utility.hpp"

#include "boost/assert.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
namespace GUI
{
//------------------------------------------------------------------------------

namespace
{
    typedef juce::String::CharPointerType::CharType char_t;

    static char_t       const presetExtension[]     = _T( ".swp" );
           unsigned int const presetExtensionLength = _countof( presetExtension ) - 1;
}

#pragma warning( push )
#pragma warning( disable : 4355 ) // 'this' used in base member initializer list.

PresetBrowser::PresetBrowser()
    :
    BackgroundImage         ( resourceBitmap<PresetBackground>() ),
    save_                   ( *this, resourceBitmap<PresetSaveDown  >(), resourceBitmap<PresetSaveUp  >(), juce::Colours::transparentWhite       , false ),
    saveAs_                 ( *this, resourceBitmap<PresetSaveAsDown>(), resourceBitmap<PresetSaveAsUp>(), juce::Colours::transparentWhite       , false ),
    delete_                 ( *this, resourceBitmap<PresetDeleteDown>(), resourceBitmap<PresetDeleteUp>(), juce::Colours::transparentWhite       , false ),
    browseArrow_            ( *this, resourceBitmap<ChangeWaveform  >(), resourceBitmap<ChangeWaveform>(), juce::Colours::white.withAlpha( 0.5f ), false ),
    ignoreExternalSamples_  ( *this, 15, 58, "Ignore external audio"                                                                                     ),
    ignoreSelectionChange_  ( false ),
    addOneRow_              ( false ),
    newPresetPending_       ( false ),
    dirtyCommentPresetIndex_( - 1   )
{
    listBox_.setModel( this );

    setSizeFromImage( *this, this->image() );

    save_  .setEnabled( false                );
    saveAs_.setEnabled( enablePresetSaving() );
    delete_.setEnabled( false                );
    save_  .addListener( this );
    saveAs_.addListener( this );
    delete_.addListener( this );

    browseArrow_.addListener( this );

    setNewFolder( GUI::presetsFolder() );

    browseArrow_.setTopLeftPosition( 174     ,  10                       );
    save_       .setTopLeftPosition( 17      ,  33                       );
    saveAs_     .setTopLeftPosition( 17+55   ,  33                       );
    delete_     .setTopLeftPosition( 17+55+55,  33                       );
    listBox_    .setBounds         ( 11      ,  79, getWidth() - 22, 234 );
    comment()   .setBounds         (  8      , 322, getWidth() - 13,  29 );

    addChildComponent( &presetNameEditBox_ );
    presetNameEditBox_.setAlwaysOnTop( true );
    presetNameEditBox_.setFont  ( Theme::singleton().whiteFont() );
    presetNameEditBox_.setColour( juce::TextEditor::backgroundColourId    , juce::Colours::black            );
    presetNameEditBox_.setColour( juce::TextEditor::focusedOutlineColourId, Theme::singleton().blueColour() );
    presetNameEditBox_.addListener( this );

    listBox_.setOpaque   ( false );
    listBox_.setRowHeight( 16    );
    listBox_.getViewport()->setScrollBarThickness( 12 );

    comment().setMultiLine             ( true );
    comment().setReturnKeyStartsNewLine( true );
    comment().setPopupMenuEnabled      ( true );
    comment().setColour( juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack );
    comment().setColour( juce::TextEditor::textColourId      , Theme::singleton().blueColour() );
    comment().setColour( juce::TextEditor::highlightColourId , juce::Colours::lightgrey        );
    comment().addListener( this );
    {
        juce::Font font( Theme::singleton().Theme::getPopupMenuFont() );
        font.setHeight( 11 );
        comment().setFont( font );
    }

    //  Required so that a preset can be deselected by clicking anywhere outside
    // the file list box.
    this->setWantsKeyboardFocus( true );

    // Implementation note:
    //   We enable the comment box only when a preset is selected (in other
    // words to create a new preset with a comment you first need to create a
    // preset and then add a comment to the new/existing preset).
    //                                        (27.05.2010.) (Domagoj Saric)
    comment().setEnabled          ( false                              );
    comment().setInputRestrictions( PresetHeader::maxCommentLength - 1 );

    BOOST_ASSERT( !save_  .getMouseClickGrabsKeyboardFocus() );
    BOOST_ASSERT( !saveAs_.getMouseClickGrabsKeyboardFocus() );
    BOOST_ASSERT( !delete_.getMouseClickGrabsKeyboardFocus() );

    addToParentAndShow( *this, comment() );
    addToParentAndShow( *this, listBox_  );

    OwnedWindow<PresetBrowser>::attach();

#ifdef LE_SW_DISABLE_SIDE_CHANNEL
    ignoreExternalSamples_.setValue  ( true  );
    ignoreExternalSamples_.setEnabled( false );
#endif // LE_SW_DISABLE_SIDE_CHANNEL
}

#pragma warning( pop )


PresetBrowser::~PresetBrowser()
{
    //...mrmlj...fade out does not work for 'on desktop components'
    //this->fadeOutComponent( 600, 0, 0, 0.2f );
    //juce::Point<int> const centre( this->getBounds().getCentre() );
    //juce::Desktop::getInstance().getAnimator().animateComponent( this, juce::Rectangle<int>( centre, centre ), 0, 600, true, 0, 0 );
    GUI::presetsFolder() = currentDirectory_;
}


void PresetBrowser::authorize()
{
    BOOST_ASSERT( save_.isEnabled() == saveAs_.isEnabled() );
    BOOST_ASSERT_MSG( !save_.isEnabled(), "Already authorised" );
    saveAs_.setEnabled( true );
}


bool PresetBrowser::enablePresetSaving() const
{
#if LE_SW_AUTHORISATION_REQUIRED
    return SW_IS_RETAIL
            ? editor().authorised() != 0
            : true;
#else
    return true;
#endif // LE_SW_AUTHORISATION_REQUIRED
}


void PresetBrowser::presetSelectionChanged()
{
    Item const & item( selectedItem() );
    if ( item.isDirectory )
    {
        save_  .setEnabled( false );
        delete_.setEnabled( false );
        return;
    }

    bool const enablePresetSaving( this->enablePresetSaving() );

    save_  .setEnabled( enablePresetSaving );
    delete_.setEnabled( enablePresetSaving );

    bool const succeeded
    (
        editor().loadPreset
        (
            selectedFile(),
            ignoreExternalSamples_.getToggleState(),
            originalComment_,
            item.name
        )
    );

    if ( !succeeded )
    {
        Preset::reportPresetLoadingError();
        return;
    }

    comment().setText   ( originalComment_, false );
    comment().setEnabled( enablePresetSaving      );
    BOOST_ASSERT( comment().getWantsKeyboardFocus() || !comment().isEnabled() );
}


unsigned int PresetBrowser::selectedIndex() const
{
    int const index( listBox_.getLastRowSelected() );
    BOOST_ASSERT_MSG( index >= 0, "Nothing selected" );
    return index;
}


PresetBrowser::Item const & PresetBrowser::item( unsigned int const index ) const
{
    Item const & item( files_.getReference( index ) );
    BOOST_ASSERT( currentDirectory_.getChildFile( item.name + ( item.isDirectory ? _T( "" ) : presetExtension ) ).exists() );
    return item;
}


PresetBrowser::Item const & PresetBrowser::selectedItem() const
{
    return item( selectedIndex() );
}


juce::File PresetBrowser::file( unsigned int const index ) const
{
    Item const & item( this->item( index ) );
    BOOST_ASSERT( !item.isDirectory );
    return currentDirectory_.getChildFile( item.name + presetExtension );
}


juce::File PresetBrowser::selectedFile() const
{
    return file( selectedIndex() );
}


PresetBrowser::Item const * PresetBrowser::findPreset( juce::String const & presetName ) const
{
    return std::find_if
    (
        files_.begin(),
        files_.end  (),
        [&]( Item const & item ) { return presetName == item.name; }
    );
}


void PresetBrowser::listBoxItemDoubleClicked( int const row, juce::MouseEvent const & )
{
    Item const & item( this->item( row ) );
    if ( item.isDirectory )
        setNewFolder( currentDirectory_.getChildFile( item.name ) );
    else
        showFilenameEditBox( item.name, listBox_.getLastRowSelected() );
}


void PresetBrowser::paintListBoxItem( int const rowNumber, juce::Graphics & graphics, int const width, int const height, bool const rowIsSelected )
{
#ifdef __APPLE__
    // Implementation note:
    //   Under OS X this also gets called when the user double-clicks on the
    // last preset in the browser (on double-click we bring up the filename
    // edit box and add the bogus extra row for which this paint method gets
    // invoked).
    //                                        (25.11.2011.) (Domagoj Saric)
    if ( rowNumber >= files_.size() )
    {
        BOOST_ASSERT( addOneRow_ == true          );
        BOOST_ASSERT( rowNumber  == files_.size() );
        BOOST_ASSERT( !rowIsSelected              );
        return;
    }
#endif // __APPLE__

    BOOST_ASSERT( rowNumber < files_.size() );

    if ( rowIsSelected )
        graphics.fillAll( Theme::singleton().Theme::findColour( juce::DirectoryContentsDisplayComponent::highlightColourId ) );

    Item const & item( this->item( rowNumber ) );

    bool const isDirectory( item.isDirectory );

    unsigned int const x( isDirectory ? 16 : 4 );
    
    graphics.setColour( juce::Colours::black );

    if ( isDirectory )
    {
        //...mrmlj...
        //paintImage
        //(
        //    graphics,
        //    Theme::singleton().Theme::getDefaultFolderImage(),
        //    2, 2
        //);
        graphics.drawImageWithin
        (
            Theme::singleton().Theme::getDefaultFolderImage(),
            2, 2,
            x - 4, height - 4,
            juce::RectanglePlacement::xLeft | juce::RectanglePlacement::xRight | juce::RectanglePlacement::onlyReduceInSize,
            false
        );
    }

    graphics.setColour( Theme::singleton().Theme::findColour( juce::DirectoryContentsDisplayComponent::textColourId ) );
    graphics.setFont  ( height * 0.7f );

    graphics.drawFittedText
    (
        item.name,
        x, 0,
        width - x, height,
        juce::Justification::centredLeft,
        1
    );
}

void PresetBrowser::deleteKeyPressed( int /*lastRowSelected*/ ) noexcept
{
}


void PresetBrowser::returnKeyPressed( int /*lastRowSelected*/ ) noexcept
{
}


void PresetBrowser::textEditorTextChanged( juce::TextEditor & editor )
{
    BOOST_ASSERT( ( &editor == &this->presetNameEditBox_ ) || ( &editor == &this->comment() ) );
    if ( &editor == &comment() )
        dirtyCommentPresetIndex_ = listBox_.getLastRowSelected();
}


#pragma warning( push )
#pragma warning( disable : 4706 ) // Assignment within conditional expression.

void PresetBrowser::textEditorReturnKeyPressed( juce::TextEditor & editor )
{
    BOOST_ASSERT( listBox_.getViewport()->isVerticalScrollBarShown() || listBox_.getViewport()->isHorizontalScrollBarShown() );
    BOOST_ASSERT( &editor == &this->presetNameEditBox_ );

    // Implementation note:
    //   We simply do not accept empty input. The user either has to cancel the
    // operation or enter something meaningful.
    //                                        (14.12.2009.) (Domagoj Saric)
    juce::String const userEntry( editor.getText() );
    if ( userEntry.isEmpty() )
        return;

    juce::File const targetFile( currentDirectory_.getChildFile( userEntry + presetExtension ) );

    if ( newPresetPending_ )
    {
        if ( targetFile.exists() )
        {
            if ( !askForOverwrite() )
                return;
            targetFile.deleteFile();
        }
        newPresetPending_ = false;
        hideFilenameEditBox();
        saveCurrentPreset( userEntry, targetFile );
    }
    else
    {
        juce::File const sourceFile( selectedFile() );
        if
        (
            sourceFile != targetFile &&
            (
                !targetFile.exists() ||
                askForOverwrite()
            )
        )
        {
            bool canceled( false );
            while
            (
                !sourceFile.moveFileTo( targetFile ) &&
                !( canceled = !GUI::warningOkCancelBox( _T( "Error writing." ), _T( "Retry?" ) ) )
            ) {}
            if ( canceled )
                return;
            hideFilenameEditBox();
            refreshAndSelectPreset( userEntry );
        }
    }
}

#pragma warning( pop )


void PresetBrowser::textEditorEscapeKeyPressed( juce::TextEditor & editor )
{
    if ( &editor == &this->presetNameEditBox_ )
    {
        hideFilenameEditBox();
    }
    else
    {
        BOOST_ASSERT( &editor == &this->comment() );
        comment().setText( originalComment_, false );
    }
}


void PresetBrowser::textEditorFocusLost( juce::TextEditor & editor )
{
    if ( &editor == &this->presetNameEditBox_ )
    {
        hideFilenameEditBox();
    }
    else
    {
        BOOST_ASSERT( &editor == &this->comment() );
        // Implementation note:
        //   No need to do anything here: already handled in focusLost().
        //                                    (15.03.2010.) (Domagoj Saric)
    }
}


////////////////////////////////////////////////////////////////////////////////
//
// PresetBrowser::saveDirtyComment()
// ---------------------------------
//
////////////////////////////////////////////////////////////////////////////////
// Implementation note:
//   Focus change monitoring and handling logic failed to work as desired
// when a juce::TextEditorListener was used to 'listen' to the comment
// 'TextEditor' because JUCE sends notifications through listeners
// asynchronously so a focus lost notification was received after the
// ListBoxModel::selectedRowsChanged() notification was received when all
// the required information on the current preset and a possibly non-saved
// comment are lost. For this reason a little utility class was used that simply
// derives from juce::TextEditor and override its focusLost() method to handle
// it directly (and synchronously). This approach however required RTTI on OSX
// (because JUCE converts between TextEditorListener and Component instance
// pointers with dynamic_cast) so the "dirty comment tracking" technique is
// currently used.
//                                            (16.12.2010.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////////

void PresetBrowser::saveDirtyComment()
{
    if ( dirtyCommentPresetIndex_ < 0 )
        return;

    juce::File const dirtyPreset( this->file( dirtyCommentPresetIndex_ ) );

    dirtyCommentPresetIndex_ = -1;

    if ( dirtyPreset.existsAsFile() )
    {
        juce::String const newComment( comment().getText() );

        //...assert that the file/preset on disk is the same as the one in selectedPreset...
        using namespace boost;
        Preset::InMemoryPresetBuffer newPresetData;
        unsigned int newPresetDataSize( 0 );
        {
            auto const pPresetData( Preset::loadIntoMemory( dirtyPreset ) );
            if ( !pPresetData.get() )
                return;
            PresetHeader const presetHeader( newComment );
            Preset preset( pPresetData.get() );
            preset.setHeader( presetHeader );
            newPresetDataSize = preset.saveTo( &newPresetData[ 0 ] );
        }
        mmap::basic_mapped_view const mappedPreset( mmap::map_file( dirtyPreset.getFullPathName().getCharPointer(), newPresetDataSize ) );
        if ( !mappedPreset )
            return;
        BOOST_ASSERT( mappedPreset.size() == newPresetDataSize );
        std::memcpy( mappedPreset.begin(), &newPresetData[ 0 ], newPresetDataSize );
    }
}


void PresetBrowser::saveCurrentPreset( juce::String const & presetName, juce::File const & targetFile )
{
    bool const shouldRefresh( !targetFile.exists() );

    originalComment_ = comment().getText();
    editor().savePreset( targetFile, ignoreExternalSamples_.getToggleState(), originalComment_ );

    if ( shouldRefresh )
        refreshAndSelectPreset( presetName );
}


void PresetBrowser::buttonClicked( juce::Button * const pButton )
{
    if ( pButton == &save_ )
    {
        saveCurrentPreset( selectedItem().name, selectedFile() );
    }
    else
    if ( pButton == &delete_ )
    {
        selectedFile().deleteFile();
        refresh();
        delete_.setEnabled( false );
        deselectAllRows();
    }
    else
    if ( pButton == &saveAs_ )
    {
        juce::String newPreset( editor().currentProgramName() );
        unsigned int const newPresetNameLength( newPreset.length() );

        Item const *       pPreset;
        Item const * const pPresetsEnd( files_.end() );
        unsigned int counter( 0 );
        while ( ( pPreset = findPreset( newPreset ) ) != pPresetsEnd ) 
        {
            unsigned int const suffixLength( 1 + 1 + 2 + counter / 100 + 1 );
            newPreset.preallocateBytes( ( newPresetNameLength + suffixLength ) * sizeof( char_t ) );
            char_t * const pSuffix( newPreset.getCharPointer().getAddress() + newPresetNameLength );
            BOOST_VERIFY( LE_INT_SPRINTF( pSuffix, _T( " (%02u)" ), ++counter ) <= signed( suffixLength ) );
        }

        // Implementation note:
        //   Creating a new preset is done asynchronously (as it waits for
        // user input) so we return here immediately (delegating further
        // processing to the textEditorReturnKeyPressed() callback) and skip
        // the comment().moveKeyboardFocusToSibling() call as this would
        // cause the popup preset name edit box to disappear immediately.
        //                                    (17.12.2009.) (Domagoj Saric)

        newPresetPending_ = true;

        showFilenameEditBox( newPreset, PresetBrowser::getNumRows() );
        return;
    }
    else
    {
        BOOST_ASSERT( pButton == &browseArrow_ );
        juce::FileChooser folderChooser
        (
            "Please select a folder with SW presets...",
            currentDirectory_
        );
        if ( folderChooser.browseForDirectory() )
        {
            BOOST_ASSERT( folderChooser.getResults().size() == 1 );
            setNewFolder( folderChooser.getResults().getReference( 0 ) );
        }
    }

    // Implementation note:
    //   Force the comment box to loose focus when a button is clicked (we do
    // not want the buttons to do so automatically because this messes up our
    // other related logic).
    //                                        (27.05.2010.) (Domagoj Saric)
    comment().moveKeyboardFocusToSibling( false );
}


void PresetBrowser::showFilenameEditBox( juce::String const & presetName, unsigned int atRow )
{
    BOOST_ASSERT( presetNameEditBox_.getParentComponent() == this );

    listBox_.scrollToEnsureRowIsOnscreen( atRow );

    juce::Rectangle<int> rowRect( listBox_.getRowPosition( atRow, true ).translated( listBox_.getX(), listBox_.getY() ) );
    rowRect.setTop   ( rowRect.getY     () - 3 );
    rowRect.setBottom( rowRect.getBottom() + 2 );
    //rowRect.setRight ( rowRect.getRight() - 1 );//...mrmlj...for testing...
    rowRect.setLeft  ( rowRect.getX     () - 2 );
    int const verticalOverflow( rowRect.getBottom() - listBox_.getBounds().getBottom() );
    if ( verticalOverflow > 0 )
    {
        rowRect.setPosition( rowRect.getX(), rowRect.getY() - verticalOverflow );
        addOneRow( true );
        ++atRow;
    }

    presetNameEditBox_.setSelectAllWhenFocused( true       );
    presetNameEditBox_.setBounds              ( rowRect    );
    presetNameEditBox_.setText                ( presetName );
    presetNameEditBox_.setVisible             ( true       );
    presetNameEditBox_.grabKeyboardFocus      (            );
    presetNameEditBox_.setSelectAllWhenFocused( false      );

    listBox_.updateContent();
    listBox_.scrollToEnsureRowIsOnscreen( atRow );
}


void PresetBrowser::hideFilenameEditBox()
{
    // Implementation note:
    //   The edit box should not be hidden if it lost focus due to a modal
    // component pop up (like a message box).
    //                                        (22.03.2010.) (Domagoj Saric)
    if
    (
        !presetNameEditBox_.isVisible                     () ||
         presetNameEditBox_.getNumCurrentlyModalComponents() != 0
    )
        return;

    fadeOutComponent( presetNameEditBox_, 1, 200, false );

    listBox_.grabKeyboardFocus(       );
    this->addOneRow           ( false );
    listBox_.updateContent    (       );
}


bool PresetBrowser::askForOverwrite()
{
    return GUI::warningOkCancelBox( _T( "File already exists." ), _T( "Overwrite?" ) );
}


void PresetBrowser::setNewFolder( juce::File const & file )
{
    unsigned int const length    (   file.getFullPathName().length() );
    bool         const goToParent( ( file.getFullPathName()[ length - 1 ] == '.' ) && ( file.getFullPathName()[ length - 2 ] == '.' ) );
    currentDirectory_ = goToParent ? file.getParentDirectory().getParentDirectory() : file;
    deselectAllRows();
    refresh();
    background().repaint();
}


SpectrumWorxEditor & PresetBrowser::editor()
{
    SpectrumWorxEditor * LE_RESTRICT const pEditor( &SpectrumWorxEditor::fromPresetBrowser( *this ) );
    return *pEditor;
}

SpectrumWorxEditor const & PresetBrowser::editor() const
{
    return const_cast<PresetBrowser &>( *this ).editor();
}


void PresetBrowser::selectedRowsChanged( int const lastRowSelected )
{
    saveDirtyComment();

    if ( ignoreSelectionChange_ || ( lastRowSelected == -1 ) )
        return;

    ignoreSelectionChange_ = true;
    presetSelectionChanged();
    ignoreSelectionChange_ = false;
}


LE_NOTHROW
int PresetBrowser::getNumRows() noexcept
{
    return files_.size() + addOneRow_;
}


void PresetBrowser::refresh()
{
    files_.clearQuick();

    boost::filesystem::directory_iterator directoryIterator
    (//...mrmlj...
    #ifdef _WIN32
        ( currentDirectory_.getFullPathName() + BOOST_FS_DIRECTORY_ITERATOR_SUFFIX() ).getCharPointer()
    #else
        currentDirectory_.getFullPathName().getCharPointer()
    #endif // _WIN32
    );

    typedef boost::filesystem::directory_iterator::entry Entry;

    Item item;
    while ( !!directoryIterator )
    {
        Entry const & entry( *directoryIterator );
        if ( entry.name()[ 0 ] != '.' || entry.name()[ 1 ] != '\0' )
        {
            bool const isDirectory( entry.is_directory() );
            unsigned int const fullNameLength( static_cast<unsigned int>( std::_tcslen( entry.name() ) ) );
            unsigned int const nameLength    ( isDirectory ? fullNameLength : ( fullNameLength - std::min( fullNameLength, presetExtensionLength ) ) );

            if ( isDirectory || std::_tcscmp( entry.name() + nameLength, presetExtension ) == 0 )
            {
                item.isDirectory = isDirectory;
                item.name        = juce::String( entry.name(), nameLength );
                files_.add( item );
            }
        }
            
        ++directoryIterator;
    }

    std::sort( files_.begin(), files_.end() );

    listBox_.updateContent();
}


void PresetBrowser::refreshAndSelectPreset( juce::String const & presetName )
{
    refresh();

    Item const * const pItem( findPreset( presetName ) );
    unsigned int const indexToSelect( static_cast<unsigned int>( pItem - files_.begin() ) );
    BOOST_ASSERT( indexToSelect < unsigned( files_.size() ) );

    comment().grabKeyboardFocus();

    listBox_.scrollToEnsureRowIsOnscreen( indexToSelect );
    ignoreSelectionChange_ = true;
    listBox_.selectRow                  ( indexToSelect );
    ignoreSelectionChange_ = false;
    bool const enablePresetSaving( this->enablePresetSaving() );
    delete_  .setEnabled( enablePresetSaving );
    comment().setEnabled( enablePresetSaving );
}


void PresetBrowser::deselectAllRows()
{
    listBox_.deselectAllRows();
    // Implementation note:
    //   A single place to ensure that whenever the preset selection is cleared
    // the comment box is also cleared. In the special case when the selection
    // is only switched from one preset to another this will be taken care of
    // implicitly (the comment will be automatically set to the one of the newly
    // selected preset).
    //                                    (23.03.2010.) (Domagoj Saric)
    comment().clear();
    comment().setEnabled( false );

    save_.setEnabled( false );
}


void PresetBrowser::paint( juce::Graphics & graphics )
{
    BackgroundImage::paint( graphics );
    graphics.setColour( juce::Colours::white );
    graphics.drawFittedText
    (
        currentDirectory_.getFullPathName(),
          9,  8,
        162, 17,
        juce::Justification::centredLeft, 1
    );
}


bool PresetBrowser::Item::operator==( Item const & other ) const
{
    return name == other.name;
}


bool PresetBrowser::Item::operator<( Item const & other ) const
{
    if ( this->isDirectory != other.isDirectory )
        return this->isDirectory;
    return this->name < other.name;
}

//------------------------------------------------------------------------------
} // namespace GUI
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
