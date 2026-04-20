/*
  ==============================================================================

    PluginProcessor.cpp — updated for JUCE 7/8

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ProtoplugDir.h"


//==============================================================================
LuaProtoplugJuceAudioProcessor::LuaProtoplugJuceAudioProcessor()
{
    lastUIWidth   = 670;
    lastUIHeight  = 455;
    lastUISplit   = 455 - 46;
    lastUIPanel   = 0;
    lastPopoutX   = lastPopoutY = 60;
    lastUIFontSize = -1;
    popout = alwaysontop = liveMode = false;
    chunk = nullptr;
    lastOpenedEditor = nullptr;

    for (int i = 0; i < NPARAMS; i++)
    {
        params[i] = 0.5;
        auto* p = new ProtoParam (i, &params[i]);
        p->addListener (this);
        addParameter (p);
    }

    luli = new LuaLink (this);
}

LuaProtoplugJuceAudioProcessor::~LuaProtoplugJuceAudioProcessor()
{
    delete luli;
    delete[] chunk;
}

//==============================================================================
// AudioProcessorParameter::Listener — called when the host (or setParameterNotifyingHost)
// changes a parameter value.
void LuaProtoplugJuceAudioProcessor::parameterValueChanged (int parameterIndex, float newValue)
{
    if (parameterIndex < 0 || parameterIndex >= NPARAMS)
        return;

    params[parameterIndex] = (double) newValue;

    if (luli)
        luli->paramChanged (parameterIndex);

    if (getActiveEditor() && lastOpenedEditor)
        lastOpenedEditor->msg_ParamsChanged = true;
}

//==============================================================================
juce::String LuaProtoplugJuceAudioProcessor::getParameterName (int index)
{
    if (index >= NPARAMS)
        return {};
    return luli->getParameterName (index);
}

juce::String LuaProtoplugJuceAudioProcessor::getParameterText (int index)
{
    if (index >= NPARAMS)
        return {};
    juce::String s = luli->getParameterText (index);
    if (s.isEmpty())
        s = juce::String (params[index], 4);
    return s;
}

bool LuaProtoplugJuceAudioProcessor::parameterText2Double (int index, juce::String text, double& d)
{
    if (index >= NPARAMS)
        return false;
    return luli->parameterText2Double (index, text, d);
}

double LuaProtoplugJuceAudioProcessor::getTailLengthSeconds() const
{
    return luli->getTailLengthSeconds();
}

//==============================================================================
void LuaProtoplugJuceAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                    juce::MidiBuffer& midiMessages)
{
    luli->processBlock (buffer, midiMessages, getPlayHead());
}

juce::AudioProcessorEditor* LuaProtoplugJuceAudioProcessor::createEditor()
{
    return new LuaProtoplugJuceAudioProcessorEditor (this);
}

//==============================================================================
void LuaProtoplugJuceAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // if code editor is open, flush its code to luli buffer
    if (getActiveEditor())
        lastOpenedEditor->saveCode();

    // if lua save() is overridden, call it and store the script's custom data
    luli->saveData = luli->save();

    int sz_script = luli->code.length() * 2;
    int sz_user   = luli->saveData.length() * 2;
    int sz_total  = 3 * 4 + 8 * NPARAMS + sz_script + sz_user + 8;

    delete[] chunk;
    chunk = new char[sz_total];

    int*   pi = (int*) chunk;
    *pi++ = NPARAMS;                                        // store number of parameters
    double* pd = (double*) pi;
    for (int i = 0; i < NPARAMS; i++)
        *pd++ = params[i];                                  // store param values
    pi = (int*) pd;
    *pi++ = sz_script;                                      // store size of code
    char* pc = (char*) pi;
    strcpy (pc, luli->code.getCharPointer());               // store code
    pc += sz_script;
    pi = (int*) pc;
    *pi++ = sz_user;                                        // store size of lua saveable string
    pc = (char*) pi;
    strcpy (pc, luli->saveData.getCharPointer());           // store lua saveable string

    destData.append (chunk, sz_total);
}

void LuaProtoplugJuceAudioProcessor::setStateInformation (const void* data, int /*sizeInBytes*/)
{
    const int* pi = (const int*) data;
    int numparams  = *pi++;                                 // get number of parameters
    const double* pd = (const double*) pi;
    for (int i = 0; i < numparams; i++)
    {
        if (i < NPARAMS)
            params[i] = *pd++;
        else
            ++pd;
    }
    pi = (const int*) pd;
    int sz_script  = *pi++;                                 // get size of code
    const char* pc = (const char*) pi;
    luli->code     = pc;                                    // get code
    luli->saveData = {};
    if (ProtoplugDir::Instance()->found)
        luli->compile();
    else
        luli->addToLog ("could not compile script because the ProtoplugFiles directory is missing or incomplete");

    pc += sz_script;
    pi  = (const int*) pc;
    int sz_user = *pi++;                                    // get size of lua saveable string
    pc = (const char*) pi;
    if (sz_user > 0)
    {
        luli->saveData = pc;                                // get lua saveable string
        luli->load (luli->saveData);
    }
}

//==============================================================================
// This creates new instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LuaProtoplugJuceAudioProcessor();
}

ProtoWindow* LuaProtoplugJuceAudioProcessor::getProtoEditor()
{
    if (getActiveEditor())
        return lastOpenedEditor;
    return nullptr;
}

void LuaProtoplugJuceAudioProcessor::setProtoEditor (ProtoWindow* _ed)
{
    lastOpenedEditor = _ed;
}
