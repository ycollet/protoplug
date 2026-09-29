#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "guiclasses/ProtoWindow.h"

class LuaProtoplugJuceAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                               public juce::Button::Listener
{
public:
    LuaProtoplugJuceAudioProcessorEditor (LuaProtoplugJuceAudioProcessor* ownerFilter);
    ~LuaProtoplugJuceAudioProcessorEditor() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void handleCommandMessage (int com) override;
    void buttonClicked (juce::Button*) override;

    void popOut();
    void popIn();
    LuaProtoplugJuceAudioProcessor* luaProcessor;

private:
    ProtoWindow content; // the actual gui is in there
    std::unique_ptr<ProtoPopout> poppedWin;
    juce::TextButton yank;
    juce::TextButton popin;
    juce::TextButton locateFiles;
};
