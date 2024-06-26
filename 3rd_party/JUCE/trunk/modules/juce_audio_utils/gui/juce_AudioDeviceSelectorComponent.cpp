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

class SimpleDeviceManagerInputLevelMeter  : public Component,
                                            public Timer
{
public:
    SimpleDeviceManagerInputLevelMeter (AudioDeviceManager* const manager_)
        : manager (manager_),
          level (0)
    {
        startTimer (50);
        manager->enableInputLevelMeasurement (true);
    }

    ~SimpleDeviceManagerInputLevelMeter()
    {
        manager->enableInputLevelMeasurement (false);
    }

    void timerCallback() override
    {
        const float newLevel = (float) manager->getCurrentInputLevel();

        if (std::abs (level - newLevel) > 0.005f)
        {
            level = newLevel;
            repaint();
        }
    }

    void paint (Graphics& g) override
    {
        getLookAndFeel().drawLevelMeter (g, getWidth(), getHeight(),
                                         (float) exp (log (level) / 3.0)); // (add a bit of a skew to make the level more obvious)
    }

private:
    AudioDeviceManager* const manager;
    float level;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleDeviceManagerInputLevelMeter)
};


//==============================================================================
class AudioDeviceSelectorComponent::MidiInputSelectorComponentListBox  : public ListBox,
                                                                         private ListBoxModel
{
public:
    MidiInputSelectorComponentListBox (AudioDeviceManager& dm,
                                       const String& noItemsMessage_)
        : ListBox (String::empty, nullptr),
          deviceManager (dm),
          noItemsMessage (noItemsMessage_)
    {
        items = MidiInput::getDevices();

        setModel (this);
        setOutlineThickness (1);
    }

    int getNumRows()
    {
        return items.size();
    }

    void paintListBoxItem (int row,
                           Graphics& g,
                           int width, int height,
                           bool rowIsSelected)
    {
        if (isPositiveAndBelow (row, items.size()))
        {
            if (rowIsSelected)
                g.fillAll (findColour (TextEditor::highlightColourId)
                               .withMultipliedAlpha (0.3f));

            const String item (items [row]);
            bool enabled = deviceManager.isMidiInputEnabled (item);

            const int x = getTickX();
            const float tickW = height * 0.75f;

            getLookAndFeel().drawTickBox (g, *this, x - tickW, (height - tickW) / 2, tickW, tickW,
                                          enabled, true, true, false);

            g.setFont (height * 0.6f);
            g.setColour (findColour (ListBox::textColourId, true).withMultipliedAlpha (enabled ? 1.0f : 0.6f));
            g.drawText (item, x, 0, width - x - 2, height, Justification::centredLeft, true);
        }
    }

    void listBoxItemClicked (int row, const MouseEvent& e)
    {
        selectRow (row);

        if (e.x < getTickX())
            flipEnablement (row);
    }

    void listBoxItemDoubleClicked (int row, const MouseEvent&)
    {
        flipEnablement (row);
    }

    void returnKeyPressed (int row)
    {
        flipEnablement (row);
    }

    void paint (Graphics& g) override
    {
        ListBox::paint (g);

        if (items.size() == 0)
        {
            g.setColour (Colours::grey);
            g.setFont (13.0f);
            g.drawText (noItemsMessage,
                        0, 0, getWidth(), getHeight() / 2,
                        Justification::centred, true);
        }
    }

    int getBestHeight (const int preferredHeight)
    {
        const int extra = getOutlineThickness() * 2;

        return jmax (getRowHeight() * 2 + extra,
                     jmin (getRowHeight() * getNumRows() + extra,
                           preferredHeight));
    }

private:
    //==============================================================================
    AudioDeviceManager& deviceManager;
    const String noItemsMessage;
    StringArray items;

    void flipEnablement (const int row)
    {
        if (isPositiveAndBelow (row, items.size()))
        {
            const String item (items [row]);
            deviceManager.setMidiInputEnabled (item, ! deviceManager.isMidiInputEnabled (item));
        }
    }

    int getTickX() const
    {
        return getRowHeight() + 5;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiInputSelectorComponentListBox)
};


//==============================================================================
struct AudioDeviceSetupDetails
{
    AudioDeviceManager* manager;
    int minNumInputChannels, maxNumInputChannels;
    int minNumOutputChannels, maxNumOutputChannels;
    bool useStereoPairs;
};

static String getNoDeviceString()   { return "<< " + TRANS("none") + " >>"; }

//==============================================================================
class AudioDeviceSettingsPanel : public Component,
                                 private ChangeListener,
                                 private ComboBoxListener,  // (can't use ComboBox::Listener due to idiotic VC2005 bug)
                                 private ButtonListener
{
public:
    AudioDeviceSettingsPanel (AudioIODeviceType* type_,
                              AudioDeviceSetupDetails& setup_,
                              const bool hideAdvancedOptionsWithButton)
        : type (type_),
          setup (setup_)
    {
        if (hideAdvancedOptionsWithButton)
        {
            addAndMakeVisible (showAdvancedSettingsButton = new TextButton (TRANS("Show advanced settings...")));
            showAdvancedSettingsButton->addListener (this);
        }

        type->scanForDevices();

        setup.manager->addChangeListener (this);
        updateAllControls();
    }

    ~AudioDeviceSettingsPanel()
    {
        setup.manager->removeChangeListener (this);
    }

    void resized() override
    {
        const int lx = proportionOfWidth (0.35f);
        const int w = proportionOfWidth (0.4f);
        const int h = 24;
        const int space = 6;
        const int dh = h + space;
        int y = 0;

        if (outputDeviceDropDown != nullptr)
        {
            outputDeviceDropDown->setBounds (lx, y, w, h);

            if (testButton != nullptr)
                testButton->setBounds (proportionOfWidth (0.77f),
                                       outputDeviceDropDown->getY(),
                                       proportionOfWidth (0.18f),
                                       h);
            y += dh;
        }

        if (inputDeviceDropDown != nullptr)
        {
            inputDeviceDropDown->setBounds (lx, y, w, h);

            inputLevelMeter->setBounds (proportionOfWidth (0.77f),
                                        inputDeviceDropDown->getY(),
                                        proportionOfWidth (0.18f),
                                        h);
            y += dh;
        }

        const int maxBoxHeight = 100;

        if (outputChanList != nullptr)
        {
            const int bh = outputChanList->getBestHeight (maxBoxHeight);
            outputChanList->setBounds (lx, y, proportionOfWidth (0.55f), bh);
            y += bh + space;
        }

        if (inputChanList != nullptr)
        {
            const int bh = inputChanList->getBestHeight (maxBoxHeight);
            inputChanList->setBounds (lx, y, proportionOfWidth (0.55f), bh);
            y += bh + space;
        }

        y += space * 2;

        if (showAdvancedSettingsButton != nullptr)
        {
            showAdvancedSettingsButton->changeWidthToFitText (h);
            showAdvancedSettingsButton->setTopLeftPosition (lx, y);
        }

        if (sampleRateDropDown != nullptr)
        {
            sampleRateDropDown->setVisible (showAdvancedSettingsButton == nullptr
                                             || ! showAdvancedSettingsButton->isVisible());

            sampleRateDropDown->setBounds (lx, y, w, h);
            y += dh;
        }

        if (bufferSizeDropDown != nullptr)
        {
            bufferSizeDropDown->setVisible (showAdvancedSettingsButton == nullptr
                                             || ! showAdvancedSettingsButton->isVisible());
            bufferSizeDropDown->setBounds (lx, y, w, h);
            y += dh;
        }

        if (showUIButton != nullptr)
        {
            showUIButton->setVisible (showAdvancedSettingsButton == nullptr
                                        || ! showAdvancedSettingsButton->isVisible());
            showUIButton->changeWidthToFitText (h);
            showUIButton->setTopLeftPosition (lx, y);
        }
    }

    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override
    {
        if (comboBoxThatHasChanged == nullptr)
            return;

        AudioDeviceManager::AudioDeviceSetup config;
        setup.manager->getAudioDeviceSetup (config);
        String error;

        if (comboBoxThatHasChanged == outputDeviceDropDown
              || comboBoxThatHasChanged == inputDeviceDropDown)
        {
            if (outputDeviceDropDown != nullptr)
                config.outputDeviceName = outputDeviceDropDown->getSelectedId() < 0 ? String::empty
                                                                                    : outputDeviceDropDown->getText();

            if (inputDeviceDropDown != nullptr)
                config.inputDeviceName = inputDeviceDropDown->getSelectedId() < 0 ? String::empty
                                                                                  : inputDeviceDropDown->getText();

            if (! type->hasSeparateInputsAndOutputs())
                config.inputDeviceName = config.outputDeviceName;

            if (comboBoxThatHasChanged == inputDeviceDropDown)
                config.useDefaultInputChannels = true;
            else
                config.useDefaultOutputChannels = true;

            error = setup.manager->setAudioDeviceSetup (config, true);

            showCorrectDeviceName (inputDeviceDropDown, true);
            showCorrectDeviceName (outputDeviceDropDown, false);

            updateControlPanelButton();
            resized();
        }
        else if (comboBoxThatHasChanged == sampleRateDropDown)
        {
            if (sampleRateDropDown->getSelectedId() > 0)
            {
                config.sampleRate = sampleRateDropDown->getSelectedId();
                error = setup.manager->setAudioDeviceSetup (config, true);
            }
        }
        else if (comboBoxThatHasChanged == bufferSizeDropDown)
        {
            if (bufferSizeDropDown->getSelectedId() > 0)
            {
                config.bufferSize = bufferSizeDropDown->getSelectedId();
                error = setup.manager->setAudioDeviceSetup (config, true);
            }
        }

        if (error.isNotEmpty())
        {
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "Error when trying to open audio device!",
                                              error);
        }
    }

    bool showDeviceControlPanel()
    {
        AudioIODevice* const device = setup.manager->getCurrentAudioDevice();

        if (device == nullptr)
            return false;

        Component modalWindow (String::empty);
        modalWindow.setOpaque (true);
        modalWindow.addToDesktop (0);
        modalWindow.enterModalState();

        return device->showControlPanel();
    }

    void buttonClicked (Button* button) override
    {
        if (button == showAdvancedSettingsButton)
        {
            showAdvancedSettingsButton->setVisible (false);
            resized();
        }
        else if (button == showUIButton)
        {
            if (showDeviceControlPanel())
            {
                setup.manager->closeAudioDevice();
                setup.manager->restartLastAudioDevice();
                getTopLevelComponent()->toFront (true);
            }
        }
        else if (button == testButton && testButton != nullptr)
        {
            setup.manager->playTestSound();
        }
    }

    void updateAllControls()
    {
        updateOutputsComboBox();
        updateInputsComboBox();

        updateControlPanelButton();

        if (AudioIODevice* const currentDevice = setup.manager->getCurrentAudioDevice())
        {
            if (setup.maxNumOutputChannels > 0
                 && setup.minNumOutputChannels < setup.manager->getCurrentAudioDevice()->getOutputChannelNames().size())
            {
                if (outputChanList == nullptr)
                {
                    addAndMakeVisible (outputChanList
                        = new ChannelSelectorListBox (setup, ChannelSelectorListBox::audioOutputType,
                                                      TRANS ("(no audio output channels found)")));
                    outputChanLabel = new Label (String::empty, TRANS ("active output channels:"));
                    outputChanLabel->attachToComponent (outputChanList, true);
                }

                outputChanList->refresh();
            }
            else
            {
                outputChanLabel = nullptr;
                outputChanList = nullptr;
            }

            if (setup.maxNumInputChannels > 0
                 && setup.minNumInputChannels < setup.manager->getCurrentAudioDevice()->getInputChannelNames().size())
            {
                if (inputChanList == nullptr)
                {
                    addAndMakeVisible (inputChanList
                        = new ChannelSelectorListBox (setup, ChannelSelectorListBox::audioInputType,
                                                      TRANS ("(no audio input channels found)")));
                    inputChanLabel = new Label (String::empty, TRANS ("active input channels:"));
                    inputChanLabel->attachToComponent (inputChanList, true);
                }

                inputChanList->refresh();
            }
            else
            {
                inputChanLabel = nullptr;
                inputChanList = nullptr;
            }

            updateSampleRateComboBox (currentDevice);
            updateBufferSizeComboBox (currentDevice);
        }
        else
        {
            jassert (setup.manager->getCurrentAudioDevice() == nullptr); // not the correct device type!

            sampleRateLabel = nullptr;
            bufferSizeLabel = nullptr;
            sampleRateDropDown = nullptr;
            bufferSizeDropDown = nullptr;

            if (outputDeviceDropDown != nullptr)
                outputDeviceDropDown->setSelectedId (-1, dontSendNotification);

            if (inputDeviceDropDown != nullptr)
                inputDeviceDropDown->setSelectedId (-1, dontSendNotification);
        }

        resized();
        setSize (getWidth(), getLowestY() + 4);
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        updateAllControls();
    }

private:
    AudioIODeviceType* const type;
    const AudioDeviceSetupDetails setup;

    ScopedPointer<ComboBox> outputDeviceDropDown, inputDeviceDropDown, sampleRateDropDown, bufferSizeDropDown;
    ScopedPointer<Label> outputDeviceLabel, inputDeviceLabel, sampleRateLabel, bufferSizeLabel, inputChanLabel, outputChanLabel;
    ScopedPointer<TextButton> testButton;
    ScopedPointer<Component> inputLevelMeter;
    ScopedPointer<TextButton> showUIButton, showAdvancedSettingsButton;

    void showCorrectDeviceName (ComboBox* const box, const bool isInput)
    {
        if (box != nullptr)
        {
            AudioIODevice* const currentDevice = setup.manager->getCurrentAudioDevice();

            const int index = type->getIndexOfDevice (currentDevice, isInput);

            box->setSelectedId (index + 1, dontSendNotification);

            if (testButton != nullptr && ! isInput)
                testButton->setEnabled (index >= 0);
        }
    }

    void addNamesToDeviceBox (ComboBox& combo, bool isInputs)
    {
        const StringArray devs (type->getDeviceNames (isInputs));

        combo.clear (dontSendNotification);

        for (int i = 0; i < devs.size(); ++i)
            combo.addItem (devs[i], i + 1);

        combo.addItem (getNoDeviceString(), -1);
        combo.setSelectedId (-1, dontSendNotification);
    }

    int getLowestY() const
    {
        int y = 0;

        for (int i = getNumChildComponents(); --i >= 0;)
            y = jmax (y, getChildComponent (i)->getBottom());

        return y;
    }

    void updateControlPanelButton()
    {
        AudioIODevice* const currentDevice = setup.manager->getCurrentAudioDevice();
        showUIButton = nullptr;

        if (currentDevice != nullptr && currentDevice->hasControlPanel())
        {
            addAndMakeVisible (showUIButton = new TextButton (TRANS ("show this device's control panel"),
                                                              TRANS ("opens the device's own control panel")));
            showUIButton->addListener (this);
        }

        resized();
    }

    void updateOutputsComboBox()
    {
        if (setup.maxNumOutputChannels > 0 || ! type->hasSeparateInputsAndOutputs())
        {
            if (outputDeviceDropDown == nullptr)
            {
                outputDeviceDropDown = new ComboBox (String::empty);
                outputDeviceDropDown->addListener (this);
                addAndMakeVisible (outputDeviceDropDown);

                outputDeviceLabel = new Label (String::empty,
                                               type->hasSeparateInputsAndOutputs() ? TRANS ("output:")
                                                                                   : TRANS ("device:"));
                outputDeviceLabel->attachToComponent (outputDeviceDropDown, true);

                if (setup.maxNumOutputChannels > 0)
                {
                    addAndMakeVisible (testButton = new TextButton (TRANS ("Test")));
                    testButton->addListener (this);
                }
            }

            addNamesToDeviceBox (*outputDeviceDropDown, false);
        }

        showCorrectDeviceName (outputDeviceDropDown, false);
    }

    void updateInputsComboBox()
    {
        if (setup.maxNumInputChannels > 0 && type->hasSeparateInputsAndOutputs())
        {
            if (inputDeviceDropDown == nullptr)
            {
                inputDeviceDropDown = new ComboBox (String::empty);
                inputDeviceDropDown->addListener (this);
                addAndMakeVisible (inputDeviceDropDown);

                inputDeviceLabel = new Label (String::empty, TRANS ("input:"));
                inputDeviceLabel->attachToComponent (inputDeviceDropDown, true);

                addAndMakeVisible (inputLevelMeter
                    = new SimpleDeviceManagerInputLevelMeter (setup.manager));
            }

            addNamesToDeviceBox (*inputDeviceDropDown, true);
        }

        showCorrectDeviceName (inputDeviceDropDown, true);
    }

    void updateSampleRateComboBox (AudioIODevice* currentDevice)
    {
        if (sampleRateDropDown == nullptr)
        {
            addAndMakeVisible (sampleRateDropDown = new ComboBox (String::empty));

            sampleRateLabel = new Label (String::empty, TRANS ("sample rate:"));
            sampleRateLabel->attachToComponent (sampleRateDropDown, true);
        }
        else
        {
            sampleRateDropDown->clear();
            sampleRateDropDown->removeListener (this);
        }

        const int numRates = currentDevice->getNumSampleRates();

        for (int i = 0; i < numRates; ++i)
        {
            const int rate = roundToInt (currentDevice->getSampleRate (i));
            sampleRateDropDown->addItem (String (rate) + " Hz", rate);
        }

        sampleRateDropDown->setSelectedId (roundToInt (currentDevice->getCurrentSampleRate()), dontSendNotification);
        sampleRateDropDown->addListener (this);
    }

    void updateBufferSizeComboBox (AudioIODevice* currentDevice)
    {
        if (bufferSizeDropDown == nullptr)
        {
            addAndMakeVisible (bufferSizeDropDown = new ComboBox (String::empty));

            bufferSizeLabel = new Label (String::empty, TRANS ("audio buffer size:"));
            bufferSizeLabel->attachToComponent (bufferSizeDropDown, true);
        }
        else
        {
            bufferSizeDropDown->clear();
            bufferSizeDropDown->removeListener (this);
        }

        const int numBufferSizes = currentDevice->getNumBufferSizesAvailable();
        double currentRate = currentDevice->getCurrentSampleRate();
        if (currentRate == 0)
            currentRate = 48000.0;

        for (int i = 0; i < numBufferSizes; ++i)
        {
            const int bs = currentDevice->getBufferSizeSamples (i);
            bufferSizeDropDown->addItem (String (bs)
                                          + " samples ("
                                          + String (bs * 1000.0 / currentRate, 1)
                                          + " ms)",
                                         bs);
        }

        bufferSizeDropDown->setSelectedId (currentDevice->getCurrentBufferSizeSamples(), dontSendNotification);
        bufferSizeDropDown->addListener (this);
    }

public:
    //==============================================================================
    class ChannelSelectorListBox  : public ListBox,
                                    private ListBoxModel
    {
    public:
        enum BoxType
        {
            audioInputType,
            audioOutputType
        };

        //==============================================================================
        ChannelSelectorListBox (const AudioDeviceSetupDetails& setup_,
                                const BoxType type_,
                                const String& noItemsMessage_)
            : ListBox (String::empty, nullptr),
              setup (setup_),
              type (type_),
              noItemsMessage (noItemsMessage_)
        {
            refresh();
            setModel (this);
            setOutlineThickness (1);
        }

        void refresh()
        {
            items.clear();

            if (AudioIODevice* const currentDevice = setup.manager->getCurrentAudioDevice())
            {
                if (type == audioInputType)
                    items = currentDevice->getInputChannelNames();
                else if (type == audioOutputType)
                    items = currentDevice->getOutputChannelNames();

                if (setup.useStereoPairs)
                {
                    StringArray pairs;

                    for (int i = 0; i < items.size(); i += 2)
                    {
                        const String& name = items[i];

                        if (i + 1 >= items.size())
                            pairs.add (name.trim());
                        else
                            pairs.add (getNameForChannelPair (name, items[i + 1]));
                    }

                    items = pairs;
                }
            }

            updateContent();
            repaint();
        }

        int getNumRows() override
        {
            return items.size();
        }

        void paintListBoxItem (int row, Graphics& g, int width, int height, bool rowIsSelected) override
        {
            if (isPositiveAndBelow (row, items.size()))
            {
                if (rowIsSelected)
                    g.fillAll (findColour (TextEditor::highlightColourId)
                                   .withMultipliedAlpha (0.3f));

                const String item (items [row]);
                bool enabled = false;

                AudioDeviceManager::AudioDeviceSetup config;
                setup.manager->getAudioDeviceSetup (config);

                if (setup.useStereoPairs)
                {
                    if (type == audioInputType)
                        enabled = config.inputChannels [row * 2] || config.inputChannels [row * 2 + 1];
                    else if (type == audioOutputType)
                        enabled = config.outputChannels [row * 2] || config.outputChannels [row * 2 + 1];
                }
                else
                {
                    if (type == audioInputType)
                        enabled = config.inputChannels [row];
                    else if (type == audioOutputType)
                        enabled = config.outputChannels [row];
                }

                const int x = getTickX();
                const float tickW = height * 0.75f;

                getLookAndFeel().drawTickBox (g, *this, x - tickW, (height - tickW) / 2, tickW, tickW,
                                              enabled, true, true, false);

                g.setFont (height * 0.6f);
                g.setColour (findColour (ListBox::textColourId, true).withMultipliedAlpha (enabled ? 1.0f : 0.6f));
                g.drawText (item, x, 0, width - x - 2, height, Justification::centredLeft, true);
            }
        }

        void listBoxItemClicked (int row, const MouseEvent& e) override
        {
            selectRow (row);

            if (e.x < getTickX())
                flipEnablement (row);
        }

        void listBoxItemDoubleClicked (int row, const MouseEvent&) override
        {
            flipEnablement (row);
        }

        void returnKeyPressed (int row) override
        {
            flipEnablement (row);
        }

        void paint (Graphics& g) override
        {
            ListBox::paint (g);

            if (items.size() == 0)
            {
                g.setColour (Colours::grey);
                g.setFont (13.0f);
                g.drawText (noItemsMessage,
                            0, 0, getWidth(), getHeight() / 2,
                            Justification::centred, true);
            }
        }

        int getBestHeight (int maxHeight)
        {
            return getRowHeight() * jlimit (2, jmax (2, maxHeight / getRowHeight()),
                                            getNumRows())
                       + getOutlineThickness() * 2;
        }

    private:
        //==============================================================================
        const AudioDeviceSetupDetails setup;
        const BoxType type;
        const String noItemsMessage;
        StringArray items;

        static String getNameForChannelPair (const String& name1, const String& name2)
        {
            String commonBit;

            for (int j = 0; j < name1.length(); ++j)
                if (name1.substring (0, j).equalsIgnoreCase (name2.substring (0, j)))
                    commonBit = name1.substring (0, j);

            // Make sure we only split the name at a space, because otherwise, things
            // like "input 11" + "input 12" would become "input 11 + 2"
            while (commonBit.isNotEmpty() && ! CharacterFunctions::isWhitespace (commonBit.getLastCharacter()))
                commonBit = commonBit.dropLastCharacters (1);

            return name1.trim() + " + " + name2.substring (commonBit.length()).trim();
        }

        void flipEnablement (const int row)
        {
            jassert (type == audioInputType || type == audioOutputType);

            if (isPositiveAndBelow (row, items.size()))
            {
                AudioDeviceManager::AudioDeviceSetup config;
                setup.manager->getAudioDeviceSetup (config);

                if (setup.useStereoPairs)
                {
                    BigInteger bits;
                    BigInteger& original = (type == audioInputType ? config.inputChannels
                                                                   : config.outputChannels);

                    for (int i = 0; i < 256; i += 2)
                        bits.setBit (i / 2, original [i] || original [i + 1]);

                    if (type == audioInputType)
                    {
                        config.useDefaultInputChannels = false;
                        flipBit (bits, row, setup.minNumInputChannels / 2, setup.maxNumInputChannels / 2);
                    }
                    else
                    {
                        config.useDefaultOutputChannels = false;
                        flipBit (bits, row, setup.minNumOutputChannels / 2, setup.maxNumOutputChannels / 2);
                    }

                    for (int i = 0; i < 256; ++i)
                        original.setBit (i, bits [i / 2]);
                }
                else
                {
                    if (type == audioInputType)
                    {
                        config.useDefaultInputChannels = false;
                        flipBit (config.inputChannels, row, setup.minNumInputChannels, setup.maxNumInputChannels);
                    }
                    else
                    {
                        config.useDefaultOutputChannels = false;
                        flipBit (config.outputChannels, row, setup.minNumOutputChannels, setup.maxNumOutputChannels);
                    }
                }

                String error (setup.manager->setAudioDeviceSetup (config, true));

                if (error.isNotEmpty())
                {
                    //xxx
                }
            }
        }

        static void flipBit (BigInteger& chans, int index, int minNumber, int maxNumber)
        {
            const int numActive = chans.countNumberOfSetBits();

            if (chans [index])
            {
                if (numActive > minNumber)
                    chans.setBit (index, false);
            }
            else
            {
                if (numActive >= maxNumber)
                {
                    const int firstActiveChan = chans.findNextSetBit (0);

                    chans.setBit (index > firstActiveChan
                                     ? firstActiveChan : chans.getHighestBit(),
                                  false);
                }

                chans.setBit (index, true);
            }
        }

        int getTickX() const
        {
            return getRowHeight() + 5;
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelSelectorListBox)
    };

private:
    ScopedPointer<ChannelSelectorListBox> inputChanList, outputChanList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioDeviceSettingsPanel)
};


//==============================================================================
AudioDeviceSelectorComponent::AudioDeviceSelectorComponent (AudioDeviceManager& dm,
                                                            const int minInputChannels_,
                                                            const int maxInputChannels_,
                                                            const int minOutputChannels_,
                                                            const int maxOutputChannels_,
                                                            const bool showMidiInputOptions,
                                                            const bool showMidiOutputSelector,
                                                            const bool showChannelsAsStereoPairs_,
                                                            const bool hideAdvancedOptionsWithButton_)
    : deviceManager (dm),
      minOutputChannels (minOutputChannels_),
      maxOutputChannels (maxOutputChannels_),
      minInputChannels (minInputChannels_),
      maxInputChannels (maxInputChannels_),
      showChannelsAsStereoPairs (showChannelsAsStereoPairs_),
      hideAdvancedOptionsWithButton (hideAdvancedOptionsWithButton_)
{
    jassert (minOutputChannels >= 0 && minOutputChannels <= maxOutputChannels);
    jassert (minInputChannels >= 0 && minInputChannels <= maxInputChannels);

    const OwnedArray<AudioIODeviceType>& types = deviceManager.getAvailableDeviceTypes();

    if (types.size() > 1)
    {
        deviceTypeDropDown = new ComboBox (String::empty);

        for (int i = 0; i < types.size(); ++i)
            deviceTypeDropDown->addItem (types.getUnchecked(i)->getTypeName(), i + 1);

        addAndMakeVisible (deviceTypeDropDown);
        deviceTypeDropDown->addListener (this);

        deviceTypeDropDownLabel = new Label (String::empty, TRANS ("Audio device type:"));
        deviceTypeDropDownLabel->setJustificationType (Justification::centredRight);
        deviceTypeDropDownLabel->attachToComponent (deviceTypeDropDown, true);
    }

    if (showMidiInputOptions)
    {
        addAndMakeVisible (midiInputsList
                            = new MidiInputSelectorComponentListBox (deviceManager,
                                                                     "(" + TRANS("No MIDI inputs available") + ")"));

        midiInputsLabel = new Label (String::empty, TRANS ("Active MIDI inputs:"));
        midiInputsLabel->setJustificationType (Justification::topRight);
        midiInputsLabel->attachToComponent (midiInputsList, true);
    }
    else
    {
        midiInputsList = nullptr;
        midiInputsLabel = nullptr;
    }

    if (showMidiOutputSelector)
    {
        addAndMakeVisible (midiOutputSelector = new ComboBox (String::empty));
        midiOutputSelector->addListener (this);

        midiOutputLabel = new Label ("lm", TRANS("MIDI Output:"));
        midiOutputLabel->attachToComponent (midiOutputSelector, true);
    }
    else
    {
        midiOutputSelector = nullptr;
        midiOutputLabel = nullptr;
    }

    deviceManager.addChangeListener (this);
    updateAllControls();
}

AudioDeviceSelectorComponent::~AudioDeviceSelectorComponent()
{
    deviceManager.removeChangeListener (this);
}

void AudioDeviceSelectorComponent::resized()
{
    const int lx = proportionOfWidth (0.35f);
    const int w = proportionOfWidth (0.4f);
    const int h = 24;
    const int space = 6;
    const int dh = h + space;
    int y = 15;

    if (deviceTypeDropDown != nullptr)
    {
        deviceTypeDropDown->setBounds (lx, y, proportionOfWidth (0.3f), h);
        y += dh + space * 2;
    }

    if (audioDeviceSettingsComp != nullptr)
    {
        audioDeviceSettingsComp->setBounds (0, y, getWidth(), audioDeviceSettingsComp->getHeight());
        y += audioDeviceSettingsComp->getHeight() + space;
    }

    if (midiInputsList != nullptr)
    {
        const int bh = midiInputsList->getBestHeight (jmin (h * 8, getHeight() - y - space - h));
        midiInputsList->setBounds (lx, y, w, bh);
        y += bh + space;
    }

    if (midiOutputSelector != nullptr)
        midiOutputSelector->setBounds (lx, y, w, h);
}

void AudioDeviceSelectorComponent::childBoundsChanged (Component* child)
{
    if (child == audioDeviceSettingsComp)
        resized();
}

void AudioDeviceSelectorComponent::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == deviceTypeDropDown)
    {
        if (AudioIODeviceType* const type = deviceManager.getAvailableDeviceTypes() [deviceTypeDropDown->getSelectedId() - 1])
        {
            audioDeviceSettingsComp = nullptr;

            deviceManager.setCurrentAudioDeviceType (type->getTypeName(), true);

            updateAllControls(); // needed in case the type hasn't actally changed
        }
    }
    else if (comboBoxThatHasChanged == midiOutputSelector)
    {
        deviceManager.setDefaultMidiOutput (midiOutputSelector->getText());
    }
}

void AudioDeviceSelectorComponent::changeListenerCallback (ChangeBroadcaster*)
{
    updateAllControls();
}

void AudioDeviceSelectorComponent::updateAllControls()
{
    if (deviceTypeDropDown != nullptr)
        deviceTypeDropDown->setText (deviceManager.getCurrentAudioDeviceType(), dontSendNotification);

    if (audioDeviceSettingsComp == nullptr
         || audioDeviceSettingsCompType != deviceManager.getCurrentAudioDeviceType())
    {
        audioDeviceSettingsCompType = deviceManager.getCurrentAudioDeviceType();
        audioDeviceSettingsComp = nullptr;

        if (AudioIODeviceType* const type
                = deviceManager.getAvailableDeviceTypes() [deviceTypeDropDown == nullptr
                                                            ? 0 : deviceTypeDropDown->getSelectedId() - 1])
        {
            AudioDeviceSetupDetails details;
            details.manager = &deviceManager;
            details.minNumInputChannels = minInputChannels;
            details.maxNumInputChannels = maxInputChannels;
            details.minNumOutputChannels = minOutputChannels;
            details.maxNumOutputChannels = maxOutputChannels;
            details.useStereoPairs = showChannelsAsStereoPairs;

            audioDeviceSettingsComp = new AudioDeviceSettingsPanel (type, details, hideAdvancedOptionsWithButton);

            if (audioDeviceSettingsComp != nullptr)
            {
                addAndMakeVisible (audioDeviceSettingsComp);
                audioDeviceSettingsComp->resized();
            }
        }
    }

    if (midiInputsList != nullptr)
    {
        midiInputsList->updateContent();
        midiInputsList->repaint();
    }

    if (midiOutputSelector != nullptr)
    {
        midiOutputSelector->clear();

        const StringArray midiOuts (MidiOutput::getDevices());

        midiOutputSelector->addItem (getNoDeviceString(), -1);
        midiOutputSelector->addSeparator();

        for (int i = 0; i < midiOuts.size(); ++i)
            midiOutputSelector->addItem (midiOuts[i], i + 1);

        int current = -1;

        if (deviceManager.getDefaultMidiOutput() != nullptr)
            current = 1 + midiOuts.indexOf (deviceManager.getDefaultMidiOutputName());

        midiOutputSelector->setSelectedId (current, dontSendNotification);
    }

    resized();
}
