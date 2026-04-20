/*
  ==============================================================================

    Dockable.h — updated for JUCE 7/8

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"

class Dockable;

class DockablePopout : public juce::DocumentWindow
{
public:
    DockablePopout (Dockable* _dad,
                    const juce::String& name,
                    juce::Colour backgroundColour,
                    int requiredButtons,
                    bool addToDesktop = true)
        : juce::DocumentWindow (name, backgroundColour, requiredButtons, addToDesktop)
    { dad = _dad; }

    void closeButtonPressed() override;

private:
    Dockable* dad;
};


class Dockable : public juce::Component
{
public:
    Dockable (juce::Component* _content, juce::String _name, LuaProtoplugJuceAudioProcessor* _processor)
    {
        content   = _content;
        name      = _name;
        processor = _processor;
        addAndMakeVisible (content);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::white);
        if (! docwin) return;
        g.fillAll();
        g.setColour (juce::Colours::grey);
        g.drawText (name + " window popped out !", g.getClipBounds(), juce::Justification::centred, false);
    }

    void resized() override
    {
        if (! docwin)
            content->setBounds (0, 0, getWidth(), getHeight());
    }

    void handleCommandMessage (int com) override
    {
        if (com == 1 && ! docwin)
            popOut();
        else if (com == 1 && docwin)
            popIn();
    }

    void popOut()
    {
        docwin = std::make_unique<DockablePopout> (this, name, juce::Colours::white,
                                                   juce::DocumentWindow::allButtons, true);
        docwin->setAlwaysOnTop (processor->alwaysontop);
        docwin->setResizable (true, false);
        docwin->setUsingNativeTitleBar (true);
        docwin->setContentNonOwned (content, true);
        docwin->setTopLeftPosition (processor->lastPopoutX, processor->lastPopoutY);
        docwin->setVisible (true);
        resized();
    }

    void popIn()
    {
        addAndMakeVisible (content);
        content->setSize (getWidth(), getHeight());
        docwin.reset();
        resized();
    }

    void setAlwaysOnTop (bool aot)
    {
        if (docwin)
            docwin->setAlwaysOnTop (aot);
    }

    bool isPoppedOut()    { return docwin != nullptr; }

    void bringWindowToFront()
    {
        if (docwin)
            docwin->toFront (true);
    }

private:
    juce::Component*                        content;
    std::unique_ptr<DockablePopout>         docwin;
    juce::String                            name;
    LuaProtoplugJuceAudioProcessor*         processor;
};
