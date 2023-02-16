#include <JuceHeader.h>
#include "PluginProcessor.h"



FETCompressor2AudioProcessor::~FETCompressor2AudioProcessor() {}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FETCompressor2AudioProcessor();
}



