/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BasicDelayAudioProcessor::BasicDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts (*this, nullptr, "Parameters", createParameters())
#endif
{
    apvts.addParameterListener ("RATE", this);
    apvts.addParameterListener ("FEEDBACK", this);
    apvts.addParameterListener ("MIX", this);
}

BasicDelayAudioProcessor::~BasicDelayAudioProcessor()
{
    apvts.removeParameterListener ("RATE", this);
    apvts.removeParameterListener ("FEEDBACK", this);
    apvts.removeParameterListener ("MIX", this);
}

//==============================================================================
const juce::String BasicDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BasicDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BasicDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BasicDelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BasicDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BasicDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BasicDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BasicDelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BasicDelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void BasicDelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BasicDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;    
    
    delay.prepare (spec);
    linear.prepare (spec);
    mixer.prepare (spec);
    
    for (auto& volume : delayFeedbackVolume)
        volume.reset (spec.sampleRate, 0.05);
    
    linear.reset();
    std::fill (lastDelayOutput.begin(), lastDelayOutput.end(), 0.0f);
}

void BasicDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BasicDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void BasicDelayAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    const auto totalNumInputChannels  = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    const auto numChannels = jmax (totalNumInputChannels, totalNumOutputChannels);

    auto audioBlock = juce::dsp::AudioBlock<float> (buffer).getSubsetChannelBlock (0, (size_t) numChannels);
    auto context = juce::dsp::ProcessContextReplacing<float> (audioBlock);
    const auto& input = context.getInputBlock();
    const auto& output = context.getOutputBlock();
    
    mixer.pushDrySamples (input);
    
    for (size_t channel = 0; channel < numChannels; ++channel)
    {
        auto* samplesIn = input.getChannelPointer (channel);
        auto* samplesOut = output.getChannelPointer (channel);
        
        for (size_t sample = 0; sample < input.getNumSamples(); ++sample)
        {
            auto input = samplesIn[sample] - lastDelayOutput[channel];
            auto delayAmount = delayValue[channel];

            linear.pushSample (int (channel), input);
            linear.setDelay ((float) delayAmount);
            samplesOut[sample] = linear.popSample ((int) channel);
                        
            lastDelayOutput[channel] = samplesOut[sample] * delayFeedbackVolume[channel].getNextValue();
        }
    }
    
    mixer.mixWetSamples (output);
}

//==============================================================================
bool BasicDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BasicDelayAudioProcessor::createEditor()
{
    return new BasicDelayAudioProcessorEditor (*this);
}

//==============================================================================
void BasicDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BasicDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BasicDelayAudioProcessor();
}

void BasicDelayAudioProcessor::parameterChanged (const String& parameterID, float newValue)
{
    if (parameterID == "RATE")
        std::fill (delayValue.begin(), delayValue.end(), newValue / 1000.0 * getSampleRate());
    
    if (parameterID == "MIX")
        mixer.setWetMixProportion (newValue);
    
    if (parameterID == "FEEDBACK")
    {
        const auto feedbackGain = Decibels::decibelsToGain (newValue, -100.0f);
        
        for (auto& volume : delayFeedbackVolume)
            volume.setTargetValue (feedbackGain);
    }
}

AudioProcessorValueTreeState::ParameterLayout BasicDelayAudioProcessor::createParameters()
{
    AudioProcessorValueTreeState::ParameterLayout params;
    
    using Range = NormalisableRange<float>;
    
    params.add (std::make_unique<AudioParameterFloat>   ("RATE", "Rate", 0.01f, 1000.0f, 0));
    params.add (std::make_unique<AudioParameterFloat> ("FEEDBACK", "Feedback", -100.0f, 0.0f, -100.0f));
    params.add (std::make_unique<AudioParameterFloat> ("MIX", "Mix", Range { 0.0f, 1.0f, 0.01f }, 0.0f));
    
    return params;
}

