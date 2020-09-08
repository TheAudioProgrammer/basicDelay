/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Assets.h"

class TapImage : public ImageComponent
{
public:
    TapImage()
    {
        tapImage = ImageCache::getFromMemory (Assets::Lockup_3_Curves_png, Assets::Lockup_3_Curves_pngSize);
        jassert (tapImage.isValid());
        setImage (tapImage, RectanglePlacement::stretchToFit);
        addAndMakeVisible (websiteButton);
    }
    
    void resized() override
    {
        websiteButton.setBounds (getLocalBounds());
    }
    
private:
    Image tapImage;
    HyperlinkButton websiteButton { "", URL ("https://theaudioprogrammer.com") };
};

//==============================================================================
/**
*/
class BasicDelayAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    BasicDelayAudioProcessorEditor (BasicDelayAudioProcessor&);
    ~BasicDelayAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    Slider rateSlider;
    Slider feedbackSlider;
    Slider mixSlider;
    Label rateLabel     { "Rate", "Rate" };
    Label feedbackLabel { "Feedback", "Feedback" };
    Label mixLabel      { "Mix", "Mix" };
    Label pluginTitle   { "Plug-in Title", "Delay" };
    
    using Attachment = std::unique_ptr<SliderParameterAttachment>;
    
    Attachment rateSliderAttachment;
    Attachment feedbackSliderAttachment;
    Attachment mixSliderAttachment;
    
    TapImage tapImage;
    Font currentFont;
    
    BasicDelayAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BasicDelayAudioProcessorEditor)
};
