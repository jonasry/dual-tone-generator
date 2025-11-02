#include "PluginEditor.h"

DualToneGeneratorAudioProcessorEditor::DualToneGeneratorAudioProcessorEditor(DualToneGeneratorAudioProcessor& p)
: AudioProcessorEditor(&p), processor(p)
{
    addAndMakeVisible(freq1Slider);
    freq1Slider.setRange(20.0, 20000.0, 1.0);
    freq1Slider.setValue(440.0);
    freq1Slider.setTextValueSuffix(" Hz");

    addAndMakeVisible(freq2Slider);
    freq2Slider.setRange(20.0, 20000.0, 1.0);
    freq2Slider.setValue(660.0);
    freq2Slider.setTextValueSuffix(" Hz");

    addAndMakeVisible(gainSlider);
    gainSlider.setRange(0.0, 1.0, 0.01);
    gainSlider.setValue(0.2);
    gainSlider.setTextValueSuffix(" Gain");

    setSize(400, 300);
}

void DualToneGeneratorAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.drawText("Dual Tone Generator", getLocalBounds(), juce::Justification::centredTop);
}

void DualToneGeneratorAudioProcessorEditor::resized() {
    auto area = getLocalBounds().reduced(20);
    freq1Slider.setBounds(area.removeFromTop(80));
    freq2Slider.setBounds(area.removeFromTop(80));
    gainSlider.setBounds(area.removeFromTop(80));
}