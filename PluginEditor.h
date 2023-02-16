/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class FETCompressor2AudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    FETCompressor2AudioProcessorEditor (FETCompressor2AudioProcessor&);
    ~FETCompressor2AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    FETCompressor2AudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FETCompressor2AudioProcessorEditor)
};
