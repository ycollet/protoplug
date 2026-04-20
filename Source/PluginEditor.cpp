/*
  ==============================================================================

    PluginEditor.cpp — updated for JUCE 7/8

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "guiclasses/ProtoPopout.h"
#include "ProtoplugDir.h"


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4355) // 'this' used in base member initializer list
#endif
//==============================================================================
LuaProtoplugJuceAudioProcessorEditor::LuaProtoplugJuceAudioProcessorEditor (LuaProtoplugJuceAudioProcessor* ownerFilter)
    : AudioProcessorEditor (ownerFilter),
      content (this, ownerFilter),
      yank ("bring to front"),
      popin ("pop back in"),
      locateFiles ("locate directory...")
{
    processor = ownerFilter;
    ownerFilter->setProtoEditor (&content);
    addChildComponent (&yank);
    addChildComponent (&popin);
    addChildComponent (&locateFiles);
    yank.addListener (this);
    popin.addListener (this);
    locateFiles.addListener (this);
    yank.setBounds (20, 50, 150, 30);
    yank.setCentrePosition (140, 60);
    popin.setBounds (20, 95, 150, 30);
    popin.setCentrePosition (140, 102);
    locateFiles.setBounds (45, 95, 150, 30);
    locateFiles.setCentrePosition (190, 95);

    if (! ProtoplugDir::Instance()->found)
    {
        setSize (380, 130);
        locateFiles.setVisible (true);
    }
    else if (processor->popout)
    {
        content.initProtoplugDir();
        content.setSize (ownerFilter->lastUIWidth, ownerFilter->lastUIHeight);
        popOut();
    }
    else
    {
        content.initProtoplugDir();
        addAndMakeVisible (&content);
        content.takeFocus();
        setSize (ownerFilter->lastUIWidth, ownerFilter->lastUIHeight);
    }
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

LuaProtoplugJuceAudioProcessorEditor::~LuaProtoplugJuceAudioProcessorEditor()
{
    if (poppedWin != nullptr)
    {
        processor->lastPopoutX = poppedWin->getX();
        processor->lastPopoutY = poppedWin->getY();
    }
    content.saveCode();
}

void LuaProtoplugJuceAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::white);
    if (! ProtoplugDir::Instance()->found)
    {
        g.setColour (juce::Colours::black);
        g.setFont (15.0f);
        g.drawFittedText (
            "ProtoplugFiles not found! Please locate the \"ProtoplugFiles\" directory "
            " (which should contain \"generators\", \"effects\", \"themes\"...)",
            10, 0, 360, 80,
            juce::Justification::centred, 3);
    }
    else if (poppedWin)
    {
        g.setColour (juce::Colours::black);
        g.setFont (15.0f);
        g.drawFittedText ("interface popped out",
                          0, 0, 280, 50,
                          juce::Justification::centred, 1);
    }
}

void LuaProtoplugJuceAudioProcessorEditor::resized()
{
    if (! poppedWin)
        content.setBounds (0, 0, getWidth(), getHeight());
}

void LuaProtoplugJuceAudioProcessorEditor::buttonClicked (juce::Button* b)
{
    if (b == &yank && poppedWin)
    {
        poppedWin->toFront (true);
    }
    else if (b == &popin && poppedWin)
    {
        popIn();
    }
    else if (b == &locateFiles)
    {
        juce::FileChooser fileOpen (
            "Where did you put my ProtoplugFiles directory:",
            juce::File::getSpecialLocation (juce::File::currentApplicationFile).getParentDirectory());
        if (fileOpen.browseForDirectory())
        {
            juce::File chosen = fileOpen.getResult();
            juce::String missing;
            if (ProtoplugDir::Instance()->checkDir (chosen, missing))
            {
                ProtoplugDir::Instance()->setDir (chosen);
                if (ProtoplugDir::Instance()->getDirTextFile().create().wasOk())
                    ProtoplugDir::Instance()->getDirTextFile().replaceWithText (chosen.getFullPathName());
                setSize (670, 455);
                processor->luli->initProtoplugDir();
                locateFiles.setVisible (false);
                addAndMakeVisible (&content);
                content.takeFocus();
                content.initProtoplugDir();
            }
            else
            {
                juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon,
                    "Protoplug",
                    "Wrong directory: \"" + missing + "\" was not found in the given directory.");
            }
        }
    }
}

void LuaProtoplugJuceAudioProcessorEditor::popOut()
{
    poppedWin = std::make_unique<ProtoPopout> (this, processor->getName(), juce::Colours::white,
                                               juce::DocumentWindow::allButtons, true);
    poppedWin->setAlwaysOnTop (processor->alwaysontop);
    poppedWin->setResizable (true, false);
    poppedWin->setUsingNativeTitleBar (true);
    poppedWin->setContentNonOwned (&content, true);
    processor->popout = true;
    poppedWin->setContentComponentSize (processor->lastUIWidth, processor->lastUIHeight);
    poppedWin->setTopLeftPosition (processor->lastPopoutX, processor->lastPopoutY);
    content.setPoppedOut (true);
    poppedWin->setVisible (true);
    setSize (280, 130);
    yank.setVisible (true);
    content.takeFocus();
    popin.setVisible (true);
}

void LuaProtoplugJuceAudioProcessorEditor::popIn()
{
    processor->lastUIWidth  = 670;
    processor->lastUIHeight = 455;
    if (poppedWin != nullptr)
    {
        processor->lastPopoutX = poppedWin->getX();
        processor->lastPopoutY = poppedWin->getY();
    }
    processor->popout = false;
    int w = processor->lastUIWidth, h = processor->lastUIHeight;
    addAndMakeVisible (&content);
    content.setPoppedOut (false);
    setSize (w, h);
    content.setSize (w, h);
    poppedWin.reset();
    yank.setVisible (false);
    content.takeFocus();
    popin.setVisible (false);
}

void LuaProtoplugJuceAudioProcessorEditor::handleCommandMessage (int com)
{
    if (com == MSG_POPOUT && ! processor->popout)
    {
        popOut();
    }
    else if (com == MSG_POPOUT && processor->popout)
    {
        popIn();
    }
    else if (com == MSG_ALWAYSONTOP)
    {
        processor->alwaysontop = ! processor->alwaysontop;
        if (poppedWin != nullptr)
            poppedWin->setAlwaysOnTop (processor->alwaysontop);
    }
}
