/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BasicDelayAudioProcessorEditor::BasicDelayAudioProcessorEditor (BasicDelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    getLookAndFeel().setColour (Slider::ColourIds::thumbColourId, Colour::fromRGB (242, 202, 16));
    getLookAndFeel().setColour (Slider::ColourIds::rotarySliderFillColourId, Colour::fromRGB (115, 155, 184));
    getLookAndFeel().setColour (Slider::ColourIds::rotarySliderOutlineColourId, Colour::fromRGB (44, 53, 57));
    
    using SliderStyle    = Slider::SliderStyle;
    using Attachment     = SliderParameterAttachment;
    const auto boxWidth  = 35;
    const auto boxHeight = 15;
    
    rateSlider.setSliderStyle (SliderStyle::RotaryVerticalDrag);
    rateSlider.setTextBoxStyle (Slider::TextBoxBelow, true, boxWidth, boxHeight);
    addAndMakeVisible (rateSlider);
    
    feedbackSlider.setSliderStyle (SliderStyle::RotaryVerticalDrag);
    feedbackSlider.setTextBoxStyle (Slider::TextBoxBelow, true, boxWidth, boxHeight);
    addAndMakeVisible (feedbackSlider);
    
    mixSlider.setSliderStyle (juce::Slider::SliderStyle::RotaryVerticalDrag);
    mixSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, boxWidth, boxHeight);
    addAndMakeVisible (mixSlider);
    
    rateLabel.setJustificationType (Justification::centred);
    addAndMakeVisible (rateLabel);
    feedbackLabel.setJustificationType (Justification::centred);
    addAndMakeVisible (feedbackLabel);
    mixLabel.setJustificationType (Justification::centred);
    addAndMakeVisible (mixLabel);
    
    pluginTitle.setFont (currentFont.boldened().withHeight (60.0f));
    pluginTitle.setColour (Label::ColourIds::textColourId, Colours::white);
    addAndMakeVisible (pluginTitle);
    
    auto& apvts = audioProcessor.apvts;
    
    rateSliderAttachment        = std::make_unique<Attachment>(*apvts.getParameter ("RATE"), rateSlider);
    feedbackSliderAttachment    = std::make_unique<Attachment>(*apvts.getParameter ("FEEDBACK"), feedbackSlider);
    mixSliderAttachment         = std::make_unique<Attachment>(*apvts.getParameter ("MIX"), mixSlider);
    
    addAndMakeVisible (tapImage);
    
    setSize (400, 350);
}

BasicDelayAudioProcessorEditor::~BasicDelayAudioProcessorEditor()
{
}

//==============================================================================
void BasicDelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (Colours::black);
}

void BasicDelayAudioProcessorEditor::resized()
{
    const auto column0     = 0.05f;
    const auto column1     = 0.28f;
    const auto column2     = 0.52f;
    const auto row0        = 0.27f;
    const auto row1        = 0.67f;
    const auto row2        = 0.40f;
    const auto dialSize    = 0.30f;
    const auto labelSpace  = 0.03f;
    const auto labelHeight = 0.05f;
    
    tapImage.setBoundsRelative (0.005f, 0.005f, 0.45f, 0.25f);
    pluginTitle.setBoundsRelative (0.45f, 0.04f, 0.5f, 0.15f);
    
    feedbackLabel.setBoundsRelative (column0, row0 - labelSpace, dialSize, labelHeight);
    feedbackSlider.setBoundsRelative (column0, row0, dialSize, dialSize);
    
    rateLabel.setBoundsRelative  (column1, row0 - labelSpace, dialSize, labelHeight);
    rateSlider.setBoundsRelative (column1, row0, dialSize, dialSize);
    
    mixLabel.setBoundsRelative (column2, row0 - labelSpace, dialSize, labelHeight);
    mixSlider.setBoundsRelative (column2, row0, dialSize, dialSize);
}
