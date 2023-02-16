/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 */
class FETCompressor2AudioProcessor  : public juce::AudioProcessor
#if JucePlugin_Enable_ARA
, public juce::AudioProcessorARAExtension
#endif
{
public:
    //==============================================================================
    FETCompressor2AudioProcessor()  : AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo())
                                                      .withOutput ("Output", juce::AudioChannelSet::stereo()))
    {
        
        // Create the input and output gain parameters
        //        inputGainParam = new juce::AudioParameterFloat("input_gain", "Input Gain", 0.0f, 2.0f, 1.0f);
        //        outputGainParam = new juce::AudioParameterFloat("output_gain", "Output Gain", 0.0f, 2.0f, 1.0f);
        
        // Create the compressor parameters
        thresholdParam = new juce::AudioParameterFloat("threshold", "Threshold", -60.0f, 0.0f, -12.0f);
        ratioParam = new juce::AudioParameterFloat("ratio", "Ratio", 1.0f, 20.0f, 4.0f);
        attackParam = new juce::AudioParameterFloat("attack", "Attack", 1.0f, 1000.0f, 20.0f);
        releaseParam = new juce::AudioParameterFloat("release", "Release", 10.0f, 10000.0f, 200.0f);
        makeupGainParam = new juce::AudioParameterFloat("makeupGain", "Makeup Gain", -12.0f, 12.0f, 0.0f);
        
        // Add the parameters to the processor
        //        addParameter(inputGainParam);
        //        addParameter(outputGainParam);
        addParameter(thresholdParam);
        addParameter(ratioParam);
        addParameter(attackParam);
        addParameter(releaseParam);
        addParameter(makeupGainParam);

        
    }
    ~FETCompressor2AudioProcessor() override;
    
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    
#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif
    
    // PROCESS BLOCK //
    //    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override
    //    {
    //        const float inputGain = inputGainParam->get();
    //        const float outputGain = outputGainParam->get();
    //        const float threshold = thresholdParam->get();
    //        const float ratio = ratioParam->get();
    //        const float attack = attackParam->get();
    //        const float release = releaseParam->get();
    //
    //        const int numSamples = buffer.getNumSamples();
    //        const int numChannels = buffer.getNumChannels();
    //
    //        for (int channel = 0; channel < numChannels; ++channel)
    //        {
    //            float* channelData = buffer.getWritePointer(channel);
    //
    //            for (int i = 0; i < numSamples; ++i)
    //            {
    //                // apply input gain
    //                channelData[i] *= inputGain;
    //
    //                // calculate the envelope of the signal
    //                float envelope = std::abs(channelData[i]);
    //
    //                // apply compression if necessary
    //                if (envelope > threshold)
    //                {
    //                    // calculate gain reduction
    //                    float dbAboveThreshold = juce::Decibels::gainToDecibels(envelope / threshold);
    //                    float dbGainReduction = dbAboveThreshold * (1.0f - 1.0f / ratio);
    //                    float gainReduction = juce::Decibels::decibelsToGain(-dbGainReduction);
    //
    //                    // apply gain reduction with fast attack and release
    //                    float attackTime = std::max(0.001f, attack / 1000.0f);
    //                    float releaseTime = std::max(0.001f, release / 1000.0f);
    //                    float time = envelope > gainReduction * threshold ? attackTime : releaseTime;
    //                    float gain = std::exp(-1.0f / (time * getSampleRate()));
    //                    channelData[i] *= gainReduction * gain;
    //                }
    //
    //                // apply output gain
    //                channelData[i] *= outputGain;
    //            }
    //        }
    //    }
    
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override
    {
        const float threshold = thresholdParam->get();
        const float ratio = ratioParam->get();
        const float attack = attackParam->get();
        const float release = releaseParam->get();
        const float makeupGain = makeupGainParam->get();

        const int numSamples = buffer.getNumSamples();
        const int numChannels = buffer.getNumChannels();
        
        const float maxGainReduction = juce::Decibels::decibelsToGain(-60.0f);
        
        for (int channel = 0; channel < numChannels; ++channel)
        {
            float* channelData = buffer.getWritePointer(channel);
            float envelope = 0.0f;
            float gain = 1.0f;
            float previousGain = 1.0f;
            
            for (int i = 0; i < numSamples; ++i)
            {
                // calculate the envelope of the signal
                envelope *= 0.999f;
                envelope = std::max(envelope, std::abs(channelData[i]));
                
                // calculate gain reduction
                float dbAboveThreshold = juce::Decibels::gainToDecibels(envelope / threshold);
                dbAboveThreshold = std::max(-60.0f, dbAboveThreshold);
                float dbGainReduction = dbAboveThreshold * (1.0f - 1.0f / ratio);
                dbGainReduction = std::min(0.0f, dbGainReduction);
                float gainReduction = juce::Decibels::decibelsToGain(dbGainReduction);
                gainReduction = std::min(gainReduction, maxGainReduction);
                
                // calculate gain smoothing
                float time = envelope > gainReduction * threshold ? attack : release;
                gain = std::exp(-1.0f / (time * getSampleRate()));
                gain = std::max(gain, previousGain * 0.999f);
                previousGain = gain;
                
                // apply gain reduction and makeup gain
                channelData[i] *= gainReduction * gain * makeupGain;
            }
        }
    }

    
    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override
    {
        return new juce::GenericAudioProcessorEditor(*this);
    }
    bool hasEditor() const override;
    
    //==============================================================================
    const juce::String getName() const override;
    
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    
    //==============================================================================
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
    int getNumParameters() override { return 5; }
    
    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override
    {
        float threshold = thresholdParam->get();
        float ratio = ratioParam->get();
        float attack = attackParam->get();
        float release = releaseParam->get();
        destData.append(&threshold, sizeof(float));
        destData.append(&ratio, sizeof(float));
        destData.append(&attack, sizeof(float));
        destData.append(&release, sizeof(float));
    }
    void setStateInformation(const void* data, int size) override
    {
        if (size != 4*sizeof(float))
            return;
        
        float threshold;
        memcpy(&threshold, data, sizeof(float));
        thresholdParam->setValueNotifyingHost(threshold);
        
        float ratio;
        memcpy(&ratio, static_cast<const char*>(data) + sizeof(float), sizeof(float));
        ratioParam->setValueNotifyingHost(ratio);
        
        float attack;
        memcpy(&attack, static_cast<const char*>(data) + 2*sizeof(float), sizeof(float));
        attackParam->setValueNotifyingHost(attack);
        
        float release;
        memcpy(&release, static_cast<const char*>(data) + 3*sizeof(float), sizeof(float));
        releaseParam->setValueNotifyingHost(release);
    }
    
    
private:
    juce::AudioParameterFloat* inputGainParam; // input gain parameter
    juce::AudioParameterFloat* outputGainParam; // output gain parameter
    juce::AudioParameterFloat*  thresholdParam;
    juce::AudioParameterFloat*  ratioParam;
    juce::AudioParameterFloat*  attackParam;
    juce::AudioParameterFloat* releaseParam;
    juce::AudioParameterFloat* makeupGainParam;
    
    float gainComputer;
    float targetGain;
    
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FETCompressor2AudioProcessor)
};


inline bool FETCompressor2AudioProcessor::hasEditor() const
{
    return true;
}

void FETCompressor2AudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // This method is called when the audio device is starting up or has changed.
    // You can use it to perform any initialization or setup that depends on the audio device settings.
    // Here, we'll just reset the compressor so it's ready to go.
    
    // Reset the compressor
    for (auto i = 0; i < 2; ++i)
    {
        gainComputer = 0.0f;
        targetGain = 0.0f;
        
        
        // Set the target gain to 1.0
        targetGain = 1.0f;
    }
}

const juce::String FETCompressor2AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FETCompressor2AudioProcessor::acceptsMidi() const
{
    return false; // Or true, depending on whether your plugin accepts MIDI input
}

bool FETCompressor2AudioProcessor::producesMidi() const
{
    return false; // Or true, depending on whether your plugin produces MIDI output
}

bool FETCompressor2AudioProcessor::isMidiEffect() const
{
    return false; // Or true, depending on whether your plugin is a MIDI effect
}

double FETCompressor2AudioProcessor::getTailLengthSeconds() const
{
    return 0.0; // Or return the length of the tail in seconds
}

void FETCompressor2AudioProcessor::setCurrentProgram(int index)
{
    // Implement this method to set the current program to the one at the specified index.
    // You might want to store the current program index as a member variable in your processor class.
}

const juce::String FETCompressor2AudioProcessor::getProgramName(int index)
{
    // Implement this method to return the name of the program at the specified index.
    // You might want to store the program names as an array of strings in your processor class.
    return "";
}

void FETCompressor2AudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    // Implement this method to change the name of the program at the specified index to the new name.
    // You might want to store the program names as an array of strings in your processor class.
}

void FETCompressor2AudioProcessor::releaseResources()
{
    // Implement this method to release any resources that were allocated during prepareToPlay.
    // For example, you might want to free any memory that was allocated for your processing algorithm.
}

bool FETCompressor2AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Implement this method to indicate whether your plugin supports the specified input and output bus layouts.
    // You can use the layouts.inputBuses.size() and layouts.outputBuses.size() to determine the number of input and output buses.
    // You might also want to check the channel configurations of the buses using the getChannelSet() method.
    return true; // Or return false if your plugin does not support the specified bus layout.
}
