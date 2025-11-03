#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>

class DualToneGeneratorAudioProcessor;

class DualToneGeneratorAudioProcessorEditor : public juce::AudioProcessorEditor,
                                              private juce::Timer
{
public:
    explicit DualToneGeneratorAudioProcessorEditor(DualToneGeneratorAudioProcessor& processor);
    ~DualToneGeneratorAudioProcessorEditor() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    void configureSlider(juce::Slider& slider,
                         juce::Label& label,
                         const juce::String& labelText,
                         juce::Slider::SliderStyle style);

    DualToneGeneratorAudioProcessor& processorRef;

    juce::Slider frequencyOneSlider;
    juce::Slider frequencyTwoSlider;
    juce::Slider panOneSlider;
    juce::Slider panTwoSlider;
    juce::Slider gainSlider;

    juce::Label frequencyOneLabel;
    juce::Label frequencyTwoLabel;
    juce::Label panOneLabel;
    juce::Label panTwoLabel;
    juce::Label gainLabel;

    juce::AudioProcessorValueTreeState::SliderAttachment frequencyOneAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment frequencyTwoAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment panOneAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment panTwoAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment gainAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DualToneGeneratorAudioProcessorEditor)
};
