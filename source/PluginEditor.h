#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include <memory>

class DualToneGeneratorAudioProcessor;
class SvgDialLookAndFeel;

class DualToneGeneratorAudioProcessorEditor : public juce::AudioProcessorEditor,
                                              private juce::Timer
{
public:
    explicit DualToneGeneratorAudioProcessorEditor(DualToneGeneratorAudioProcessor& processor);
    ~DualToneGeneratorAudioProcessorEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    void configureSlider(juce::Slider& slider,
                         juce::Label& label,
                         const juce::String& labelText,
                         juce::Slider::SliderStyle style);
    void updateScaledStyles(float scale);

    DualToneGeneratorAudioProcessor& processorRef;

    juce::Slider centerSlider;
    juce::Slider spreadSlider;
    juce::Slider panOneSlider;
    juce::Slider panTwoSlider;
    juce::Slider attenuationOneSlider;
    juce::Slider attenuationTwoSlider;

    juce::Label toneOneTitleLabel;
    juce::Label toneTwoTitleLabel;
    juce::Label centerLabel;
    juce::Label spreadLabel;
    juce::Label panOneLabel;
    juce::Label panTwoLabel;
    juce::Label attenuationOneLabel;
    juce::Label attenuationTwoLabel;
    juce::Label centerUnitLabel;
    juce::Label spreadUnitLabel;
    juce::Label centerMinLabel;
    juce::Label centerMaxLabel;
    juce::Label spreadMinLabel;
    juce::Label spreadMaxLabel;

    juce::AudioProcessorValueTreeState::SliderAttachment centerAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment spreadAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment panOneAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment panTwoAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment attenuationOneAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment attenuationTwoAttachment;

    std::unique_ptr<SvgDialLookAndFeel> redDialLookAndFeel;
    std::unique_ptr<SvgDialLookAndFeel> greenDialLookAndFeel;
    std::unique_ptr<SvgDialLookAndFeel> blueDialLookAndFeel;
    juce::Rectangle<int> contentPanelBounds;
    juce::Rectangle<int> centerPanelBounds;
    juce::Rectangle<int> spreadPanelBounds;
    juce::Rectangle<int> toneOnePanelBounds;
    juce::Rectangle<int> toneTwoPanelBounds;
    juce::Line<float> toneOneDividerLine;
    juce::Line<float> toneTwoDividerLine;
    float toneDividerThickness = 1.0f;
    float toneDividerSeparation = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DualToneGeneratorAudioProcessorEditor)
};
