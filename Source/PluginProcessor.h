#pragma once
#include <JuceHeader.h>
#include "LuaLink.h"


#define NPARAMS 127
class ProtoWindow;
class LuaProtoplugJuceAudioProcessor;
class ProtoPopout;

//==============================================================================
// Custom AudioProcessorParameter that mirrors the raw params[] double array
// so that Lua scripts can read/write params[] directly while the host
// still receives proper parameter notifications.
class ProtoParam : public juce::AudioProcessorParameter
{
public:
    ProtoParam (int idx, double* val)
        : index (idx), value (val)
    {}

    float getValue() const override                         { return (float) *value; }
    void  setValue (float v) override                       { *value = (double) v; }
    float getDefaultValue() const override                  { return 0.5f; }
    juce::String getName (int maxLen) const override        { return juce::String (index).substring (0, maxLen); }
    juce::String getLabel() const override                  { return {}; }
    float getValueForText (const juce::String& t) const override { return t.getFloatValue(); }

private:
    int     index;
    double* value;
};

//==============================================================================
class LuaProtoplugJuceAudioProcessor  : public juce::AudioProcessor,
                                        private juce::AudioProcessorParameter::Listener
{
public:
    LuaProtoplugJuceAudioProcessor();
    ~LuaProtoplugJuceAudioProcessor();

    // AudioProcessor overrides
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    juce::AudioProcessorEditor* createEditor() override;
    double getTailLengthSeconds() const override;
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Inlined AudioProcessor overrides
    bool acceptsMidi() const override                                                    { return true; }
    bool producesMidi() const override                                                   { return true; }
    const juce::String getName() const override                                          { return JucePlugin_Name; }
    bool hasEditor() const override                                                      { return true; }
    int  getNumPrograms() override                                                       { return 1; }
    int  getCurrentProgram() override                                                    { return 0; }
    void setCurrentProgram (int) override                                                {}
    const juce::String getProgramName (int) override                                     { return {}; }
    void changeProgramName (int, const juce::String&) override                           {}
    void prepareToPlay (double, int) override                                            {}
    void releaseResources() override                                                     {}

    // Helpers for the GUI (not AudioProcessor overrides)
    const juce::String getParameterName (int index);
    const juce::String getParameterText (int index);
    bool parameterText2Double (int index, juce::String text, double& d);

    // Editor management
    ProtoWindow* getProtoEditor();
    void setProtoEditor (ProtoWindow* _ed);

    int   lastUIWidth, lastUIHeight, lastUISplit, lastUIPanel;
    int   lastPopoutX, lastPopoutY;
    float lastUIFontSize;
    bool  popout, alwaysontop, liveMode;
    LuaLink* luli;
    double params[NPARAMS];

private:
    // AudioProcessorParameter::Listener — called when the host changes a parameter
    void parameterValueChanged (int parameterIndex, float newValue) override;
    void parameterGestureChanged (int, bool) override {}

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LuaProtoplugJuceAudioProcessor)
    ProtoWindow* lastOpenedEditor;
    char* chunk;
};
