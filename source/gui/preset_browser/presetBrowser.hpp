////////////////////////////////////////////////////////////////////////////////
///
/// \file presetBrowser.hpp
/// -----------------------
///
/// SpectrumWorx preset browser implementation.
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef presetBrowser_hpp__228370D3_4C4C_46B8_8544_9273C3AAEB61A
#define presetBrowser_hpp__228370D3_4C4C_46B8_8544_9273C3AAEB61A
#pragma once
//------------------------------------------------------------------------------
#include "gui/gui.hpp"

#include "le/utility/platformSpecifics.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

class SpectrumWorx;

//------------------------------------------------------------------------------
namespace GUI
{
//------------------------------------------------------------------------------

class SpectrumWorxEditor;

class PresetBrowser LE_SEALED
    :
    public  BackgroundImage           ,
    private juce::ListBoxModel        ,
    private juce::ButtonListener      ,
    private juce::TextEditorListener  ,
    public  OwnedWindow<PresetBrowser>
{
public:
     PresetBrowser();
    ~PresetBrowser();

    void authorize();

    juce::Component & window() { return *this; }

    SpectrumWorxEditor       & editor()      ;
    SpectrumWorxEditor const & editor() const;

private: // JUCE Component overrides.
    void paint( juce::Graphics & ) LE_OVERRIDE;

private: // JUCE ButtonListener overrides.
    void buttonClicked( juce::Button * ) LE_OVERRIDE;

private: // JUCE TextEditorListener overrides.
    void textEditorTextChanged     ( juce::TextEditor & ) LE_OVERRIDE;
    void textEditorReturnKeyPressed( juce::TextEditor & ) LE_OVERRIDE;
    void textEditorEscapeKeyPressed( juce::TextEditor & ) LE_OVERRIDE;
    void textEditorFocusLost       ( juce::TextEditor & ) LE_OVERRIDE;

private: // JUCE ListBoxModel overrides.
    LE_NOTHROW int  getNumRows() noexcept LE_OVERRIDE;
               void paintListBoxItem( int rowNumber, juce::Graphics &, int width, int height, bool rowIsSelected ) LE_OVERRIDE;
               void listBoxItemDoubleClicked( int row, juce::MouseEvent const & ) LE_OVERRIDE;
               void deleteKeyPressed   ( int lastRowSelected ) noexcept LE_OVERRIDE;
               void returnKeyPressed   ( int lastRowSelected ) noexcept LE_OVERRIDE;
               void selectedRowsChanged( int lastRowSelected )          LE_OVERRIDE;

private:
    struct Item
    {
        juce::String name;
        bool         isDirectory;

        bool operator==( Item const & other ) const;
        bool operator< ( Item const & other ) const;
    };

private:
    void setNewFolder( juce::File const & );

    void refresh();

    void refreshAndSelectPreset( juce::String const & presetName );

    void showFilenameEditBox( juce::String const & presetName, unsigned int atRow );
    void hideFilenameEditBox();

    void saveCurrentPreset( juce::String const & presetName, juce::File const & targetFile );

    void saveDirtyComment();
    void presetSelectionChanged();

    void deselectAllRows();

    void addOneRow( bool const value ) { addOneRow_ = value; }

    static bool askForOverwrite();

    bool enablePresetSaving() const;

    unsigned int selectedIndex() const;

    Item const & item        ( unsigned int index ) const;
    Item const & selectedItem(                    ) const;
    juce::File   file        ( unsigned int index ) const;
    juce::File   selectedFile(                    ) const;

    Item const * findPreset( juce::String const & presetName ) const;

    juce::TextEditor & comment   () { return commentBox_; }
    BackgroundImage  & background() { return *this      ; }

private:// friend class Detail::BackgroundWithCurrentFolder;
    juce::TextEditor presetNameEditBox_    ;
    juce::TextEditor commentBox_           ;
    juce::ListBox    listBox_              ;
    BitmapButton     save_                 ;
    BitmapButton     saveAs_               ;
    BitmapButton     delete_               ;
    BitmapButton     browseArrow_          ;
    LEDTextButton    ignoreExternalSamples_;

    bool ignoreSelectionChange_;
    bool addOneRow_            ;
    bool newPresetPending_     ;

    int dirtyCommentPresetIndex_;

    juce::File        currentDirectory_;
    juce::Array<Item> files_;

    juce::String originalComment_;
}; // class PresetBrowser

//------------------------------------------------------------------------------
} // namespace GUI
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // presetBrowser_hpp
