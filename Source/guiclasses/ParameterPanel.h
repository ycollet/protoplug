#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"

class ParamSlider : public juce::Slider
{
public:
    ParamSlider (LuaProtoplugJuceAudioProcessor* _pfx, int _index)
    {
        pfx   = _pfx;
        index = _index;
    }
    juce::String getTextFromValue (double /*value*/)
    {
        return pfx->getParameterText (index);
    }
    double getValueFromText (const juce::String& text)
    {
        double d;
        if (pfx->parameterText2Double (index, text, d))
            return d;
        return juce::Slider::getValueFromText (text);
    }
private:
    int index;
    LuaProtoplugJuceAudioProcessor* pfx;
};

class ParamPanelContent : public juce::Component
{
public:
    void paint (juce::Graphics& g) override { g.fillAll (juce::Colour (0xffffffff)); }
};

class ParameterPanel : public juce::Viewport, public juce::Slider::Listener
{
public:
    ParameterPanel (LuaProtoplugJuceAudioProcessor* _processor)
    {
        processor = _processor;
        content = std::make_unique<ParamPanelContent>();
        content->setBounds (0, 0, 220, NPARAMS * 36 + 36);
        for (int i = 0; i < NPARAMS; i++)
        {
            labels[i] = std::make_unique<juce::Label>();
            labels[i]->setEditable (false, false, false);
            labels[i]->setBounds (10, i * 36, 100, 22);
            content->addAndMakeVisible (labels[i].get());

            sliders[i] = std::make_unique<ParamSlider> (processor, i);
            sliders[i]->setSliderStyle (juce::Slider::LinearBar);
            sliders[i]->setBounds (110, i * 36, getWidth() - 130, 22);
            sliders[i]->setRange (0, 1.0);
            sliders[i]->setValue (processor->params[i], juce::dontSendNotification);
            sliders[i]->updateText();
            sliders[i]->addListener (this);
            sliders[i]->setColour (juce::Slider::ColourIds::textBoxTextColourId, juce::Colours::black);
            content->addAndMakeVisible (sliders[i].get());
        }
        updateNames();
        setViewedComponent (content.get());
    }

    void resized() override
    {
        content->setSize (std::max (getWidth() - getLookAndFeel().getDefaultScrollbarWidth(), 320), NPARAMS * 36 + 36);
        for (int i = 0; i < NPARAMS; i++)
            sliders[i]->setSize (std::max (getWidth() - 130, 200), 22);
        // work around juce layout refresh
        setViewPosition (getViewPosition().x, getViewPosition().y + 1);
        setViewPosition (getViewPosition().x, getViewPosition().y - 1);
    }

    void sliderDragStarted (juce::Slider* sliderThatWasMoved) override
    {
        for (int i = 0; i < NPARAMS; i++)
            if (sliders[i].get() == sliderThatWasMoved)
            {
                // AudioProcessor::beginParameterChangeGesture()/endParameterChangeGesture()/
                // setParameterNotifyingHost() were removed in JUCE 9 (deprecated since long
                // before); the gesture/value notification now lives on the parameter itself.
                processor->getParameters()[i]->beginChangeGesture();
                break;
            }
    }

    void sliderDragEnded (juce::Slider* sliderThatWasMoved) override
    {
        for (int i = 0; i < NPARAMS; i++)
            if (sliders[i].get() == sliderThatWasMoved)
            {
                processor->getParameters()[i]->endChangeGesture();
                break;
            }
    }

    void sliderValueChanged (juce::Slider* sliderThatWasMoved) override
    {
        for (int i = 0; i < NPARAMS; i++)
            if (sliders[i].get() == sliderThatWasMoved)
            {
                processor->getParameters()[i]->setValueNotifyingHost ((float) sliderThatWasMoved->getValue());
                sliders[i]->updateText();
                break;
            }
    }

    void updateNames()
    {
        for (int i = 0; i < NPARAMS; i++)
        {
            juce::String s = processor->luli->getParameterName (i);
            if (s.isEmpty())
            {
                s = "nameless";
                labels[i]->setColour (juce::Label::textColourId, juce::Colours::grey);
            }
            else
            {
                labels[i]->setColour (juce::Label::textColourId, juce::Colours::black);
            }
            labels[i]->setText (juce::String::formatted ("%d. ", i) + s, juce::dontSendNotification);
        }
    }

    void paramsChanged()
    {
        for (int i = 0; i < NPARAMS; i++)
        {
            sliders[i]->setValue (processor->params[i], juce::dontSendNotification);
            sliders[i]->updateText();
        }
    }

    void paint (juce::Graphics& g) override { g.fillAll (juce::Colour (0xffffffff)); }

private:
    std::unique_ptr<juce::Component>   content;
    std::unique_ptr<juce::Slider>      sliders[NPARAMS];
    std::unique_ptr<juce::Label>       labels[NPARAMS];
    LuaProtoplugJuceAudioProcessor*    processor;
};
