#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"

class DualToneGeneratorAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
    DualToneGeneratorAudioProcessorEditor(DualToneGeneratorAudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    DualToneGeneratorAudioProcessor& processor;
    juce::Slider freq1Slider, freq2Slider, gainSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DualToneGeneratorAudioProcessorEditor)
};
