/*
  ==============================================================================

    PluginProcessor.cpp — updated for JUCE 7/8

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ProtoplugDir.h"
#include <cstring>


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
const juce::String LuaProtoplugJuceAudioProcessor::getParameterName (int index)
{
    if (index >= NPARAMS)
        return {};
    return luli->getParameterName (index);
}

const juce::String LuaProtoplugJuceAudioProcessor::getParameterText (int index)
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

    // Use exact UTF-8 byte lengths (NOT juce::String::length(), which counts
    // Unicode characters, not bytes) so that non-ASCII code/save-data content
    // cannot overflow the buffer below.
    auto  codeUtf8   = luli->code.toUTF8();
    auto  userUtf8   = luli->saveData.toUTF8();
    size_t sz_script = (size_t) luli->code.getNumBytesAsUTF8();
    size_t sz_user   = (size_t) luli->saveData.getNumBytesAsUTF8();
    size_t sz_total  = 3 * sizeof (int) + (size_t) NPARAMS * sizeof (double) + sz_script + sz_user;

    delete[] chunk;
    chunk = new char[sz_total];

    char* pc = chunk;
    int   numParams = NPARAMS;
    memcpy (pc, &numParams, sizeof (int));                  // store number of parameters
    pc += sizeof (int);
    for (int i = 0; i < NPARAMS; i++)
    {
        memcpy (pc, &params[i], sizeof (double));           // store param values
        pc += sizeof (double);
    }

    int sz_script_i = (int) sz_script;
    memcpy (pc, &sz_script_i, sizeof (int));                // store size of code
    pc += sizeof (int);
    memcpy (pc, codeUtf8.getAddress(), sz_script);           // store code (exact byte count, no NUL)
    pc += sz_script;

    int sz_user_i = (int) sz_user;
    memcpy (pc, &sz_user_i, sizeof (int));                  // store size of lua saveable string
    pc += sizeof (int);
    memcpy (pc, userUtf8.getAddress(), sz_user);             // store lua saveable string
    pc += sz_user;

    destData.append (chunk, sz_total);
}

void LuaProtoplugJuceAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // The host-supplied state blob is untrusted input (it may come from a
    // shared project/preset file), so every embedded length must be checked
    // against the actual buffer bounds before it is used to index or read.
    if (data == nullptr || sizeInBytes <= 0)
        return;

    const char* p   = (const char*) data;
    const char* end = p + (size_t) sizeInBytes;
    auto remaining  = [&] { return (size_t) (end - p); };

    if (remaining() < sizeof (int))
        return;
    int numparams;
    memcpy (&numparams, p, sizeof (int));                   // get number of parameters
    p += sizeof (int);
    if (numparams < 0 || remaining() < (size_t) numparams * sizeof (double))
        return;

    for (int i = 0; i < numparams; i++)
    {
        double v;
        memcpy (&v, p, sizeof (double));
        p += sizeof (double);
        if (i < NPARAMS)
            params[i] = v;
    }

    if (remaining() < sizeof (int))
        return;
    int sz_script;
    memcpy (&sz_script, p, sizeof (int));                   // get size of code
    p += sizeof (int);
    if (sz_script < 0 || remaining() < (size_t) sz_script)
        return;

    luli->code = juce::String::fromUTF8 (p, sz_script);     // get code (length-bounded, not NUL-terminated read)
    p += sz_script;
    luli->saveData = {};
    if (ProtoplugDir::Instance()->found)
        luli->compile();
    else
        luli->addToLog ("could not compile script because the ProtoplugFiles directory is missing or incomplete");

    if (remaining() < sizeof (int))
        return;
    int sz_user;
    memcpy (&sz_user, p, sizeof (int));                     // get size of lua saveable string
    p += sizeof (int);
    if (sz_user > 0)
    {
        if (remaining() < (size_t) sz_user)
            return;
        luli->saveData = juce::String::fromUTF8 (p, sz_user); // get lua saveable string (length-bounded)
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
