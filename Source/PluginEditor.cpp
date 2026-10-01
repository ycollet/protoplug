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
    luaProcessor = ownerFilter;
    getConstrainer()->setMinimumSize (minimumEditorWidth, minimumEditorHeight);
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
    else if (luaProcessor->popout)
    {
        content.initProtoplugDir();
        content.setSize (ownerFilter->lastUIWidth, ownerFilter->lastUIHeight);
        popOut();
    }
    else
    {
        content.initProtoplugDir();
        addAndMakeVisible (&content);
        setResizable (true, true);
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
        luaProcessor->lastPopoutX = poppedWin->getX();
        luaProcessor->lastPopoutY = poppedWin->getY();
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
    if (content.getParentComponent() == this)
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
                luaProcessor->luli->initProtoplugDir();
                locateFiles.setVisible (false);
                addAndMakeVisible (&content);
                setResizable (true, true);
                setSize (luaProcessor->lastUIWidth, luaProcessor->lastUIHeight);
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
    poppedWin = std::make_unique<ProtoPopout> (this, luaProcessor->getName(), juce::Colours::white,
                                               juce::DocumentWindow::allButtons, true);
    poppedWin->setAlwaysOnTop (luaProcessor->alwaysontop);
    poppedWin->setResizable (true, false);
    poppedWin->getConstrainer()->setMinimumSize (minimumEditorWidth, minimumEditorHeight);
    poppedWin->setUsingNativeTitleBar (true);
    poppedWin->setContentNonOwned (&content, true);
    luaProcessor->popout = true;
    poppedWin->setContentComponentSize (luaProcessor->lastUIWidth, luaProcessor->lastUIHeight);
    poppedWin->setTopLeftPosition (luaProcessor->lastPopoutX, luaProcessor->lastPopoutY);
    poppedWin->setVisible (true);
    setResizable (false, false);
    setSize (280, 130);
    yank.setVisible (true);
    content.takeFocus();
    popin.setVisible (true);
}

void LuaProtoplugJuceAudioProcessorEditor::popIn()
{
    if (poppedWin != nullptr)
    {
        luaProcessor->lastPopoutX = poppedWin->getX();
        luaProcessor->lastPopoutY = poppedWin->getY();
    }
    luaProcessor->popout = false;
    int w = luaProcessor->lastUIWidth, h = luaProcessor->lastUIHeight;
    if (poppedWin != nullptr)
        poppedWin->clearContentComponent();
    poppedWin.reset();
    addAndMakeVisible (&content);
    setResizable (true, true);
    setSize (w, h);
    content.setSize (w, h);
    yank.setVisible (false);
    content.takeFocus();
    popin.setVisible (false);
}

void LuaProtoplugJuceAudioProcessorEditor::handleCommandMessage (int com)
{
    if (com == MSG_POPOUT && ! luaProcessor->popout)
    {
        popOut();
    }
    else if (com == MSG_POPOUT && luaProcessor->popout)
    {
        popIn();
    }
    else if (com == MSG_ALWAYSONTOP)
    {
        luaProcessor->alwaysontop = ! luaProcessor->alwaysontop;
        if (poppedWin != nullptr)
            poppedWin->setAlwaysOnTop (luaProcessor->alwaysontop);
    }
}
