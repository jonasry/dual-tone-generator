#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include <memory>

class DualToneGeneratorAudioProcessor;
#include "SvgDialLookAndFeel.h"

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

    void layoutLargeDial(juce::Slider& slider,
                         juce::Rectangle<int> area,
                         juce::Label& title,
                         juce::Label& unit,
                         juce::Label& minLabel,
                         juce::Label& maxLabel,
                         float scale);

    void layoutSmallDial(juce::Slider& slider,
                         juce::Label& label,
                         juce::Rectangle<int> area,
                         float scale);

    void layoutGainDial(juce::Rectangle<int> area,
                        float scale);

    void layoutToneSection(juce::Rectangle<int> area,
                           juce::Label& title,
                           juce::Slider& firstSlider,
                           juce::Label& firstLabel,
                           juce::Slider& secondSlider,
                           juce::Label& secondLabel,
                           float scale);

    juce::Line<float> computeToneDivider(juce::Slider& panSlider,
                                         juce::Slider& attenuationSlider,
                                         juce::Label& titleLabel,
                                         float scale);

    DualToneGeneratorAudioProcessor& processorRef;

    juce::Slider centerSlider;
    juce::Slider spreadSlider;
    juce::Slider panOneSlider;
    juce::Slider panTwoSlider;
    juce::Slider attenuationOneSlider;
    juce::Slider attenuationTwoSlider;
    juce::Slider gainSlider;
    juce::Slider driveSlider;
    juce::Slider typeSlider;

    juce::Label gainMinLabel;
    juce::Label gainMaxLabel;
    juce::Label toneOneTitleLabel;
    juce::Label toneTwoTitleLabel;
    juce::Label centerLabel;
    juce::Label spreadLabel;
    juce::Label panOneLabel;
    juce::Label panTwoLabel;
    juce::Label attenuationOneLabel;
    juce::Label attenuationTwoLabel;
    juce::Label gainLabel;
    juce::Label driveLabel;
    juce::Label typeLabel;
    juce::Label typeMinLabel;
    juce::Label typeMaxLabel;
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
    juce::AudioProcessorValueTreeState::SliderAttachment gainAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment driveAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment typeAttachment;

    std::unique_ptr<SvgDialLookAndFeel> largeDialLookAndFeel;
    std::unique_ptr<SvgDialLookAndFeel> greenDialLookAndFeel;
    std::unique_ptr<SvgDialLookAndFeel> blueDialLookAndFeel;
    std::unique_ptr<SvgDialLookAndFeel> grayDialLookAndFeel;
    std::unique_ptr<SvgDialLookAndFeel> darkDialLookAndFeel;
    std::unique_ptr<juce::Drawable> logoDrawable;
    std::unique_ptr<juce::Drawable> dualVcoDrawable;
    juce::AffineTransform logoTransform;
    juce::AffineTransform dualVcoTransform;
    juce::Rectangle<int> contentPanelBounds;
    juce::Rectangle<int> centerPanelBounds;
    juce::Rectangle<int> spreadPanelBounds;
    juce::Rectangle<int> toneOnePanelBounds;
    juce::Rectangle<int> toneTwoPanelBounds;
    juce::Rectangle<int> logoBounds;
    juce::Rectangle<int> dualVcoBounds;
    juce::Line<float> toneOneDividerLine;
    juce::Line<float> toneTwoDividerLine;
    float toneDividerThickness = 1.0f;
    float toneDividerSeparation = 1.0f;
    float logoScale = 1.0f;
    float dualVcoScale = 1.0f;
    float dualVcoLabelFontHeight = 14.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DualToneGeneratorAudioProcessorEditor)
};
