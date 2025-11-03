#include "PluginEditor.h"
#include "PluginProcessor.h"

DualToneGeneratorAudioProcessorEditor::DualToneGeneratorAudioProcessorEditor(DualToneGeneratorAudioProcessor& processor)
    : juce::AudioProcessorEditor(&processor),
      processorRef(processor),
      frequencyOneAttachment(processorRef.getValueTreeState(), "freq1", frequencyOneSlider),
      frequencyTwoAttachment(processorRef.getValueTreeState(), "freq2", frequencyTwoSlider),
      panOneAttachment(processorRef.getValueTreeState(), "pan1", panOneSlider),
      panTwoAttachment(processorRef.getValueTreeState(), "pan2", panTwoSlider),
      gainAttachment(processorRef.getValueTreeState(), "gain", gainSlider)
{
    configureSlider(frequencyOneSlider, frequencyOneLabel, "Frequency 1 (Hz)", juce::Slider::RotaryVerticalDrag);
    configureSlider(frequencyTwoSlider, frequencyTwoLabel, "Frequency 2 (Hz)", juce::Slider::RotaryVerticalDrag);
    configureSlider(panOneSlider, panOneLabel, "Pan 1", juce::Slider::RotaryVerticalDrag);
    configureSlider(panTwoSlider, panTwoLabel, "Pan 2", juce::Slider::RotaryVerticalDrag);
    configureSlider(gainSlider, gainLabel, "Gain (%)", juce::Slider::LinearHorizontal);

    gainSlider.setNumDecimalPlacesToDisplay(1);
    gainSlider.textFromValueFunction = [](double value)
    {
        return juce::String(value * 100.0, 1) + " %";
    };
    gainSlider.valueFromTextFunction = [](const juce::String& text)
    {
        auto cleaned = text.retainCharacters("0123456789.,");
        cleaned = cleaned.replaceCharacter(',', '.');
        return cleaned.getDoubleValue() / 100.0;
    };

    frequencyOneSlider.setTextValueSuffix(" Hz");
    frequencyOneSlider.setNumDecimalPlacesToDisplay(1);

    frequencyTwoSlider.setTextValueSuffix(" Hz");
    frequencyTwoSlider.setNumDecimalPlacesToDisplay(1);

    panOneSlider.setNumDecimalPlacesToDisplay(2);
    panTwoSlider.setNumDecimalPlacesToDisplay(2);

    setSize(500, 320);
    startTimerHz(10);
}

void DualToneGeneratorAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void DualToneGeneratorAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(20);

    auto topRow = bounds.removeFromTop(140);
    auto middleRow = bounds.removeFromTop(140);
    auto bottomRow = bounds;

    auto topLeft = topRow.removeFromLeft(topRow.getWidth() / 2);
    frequencyOneSlider.setBounds(topLeft.reduced(10));
    frequencyOneLabel.setJustificationType(juce::Justification::centred);
    frequencyOneLabel.setBounds(frequencyOneSlider.getX(),
                                frequencyOneSlider.getBottom() + 4,
                                frequencyOneSlider.getWidth(),
                                20);

    frequencyTwoSlider.setBounds(topRow.reduced(10));
    frequencyTwoLabel.setJustificationType(juce::Justification::centred);
    frequencyTwoLabel.setBounds(frequencyTwoSlider.getX(),
                                frequencyTwoSlider.getBottom() + 4,
                                frequencyTwoSlider.getWidth(),
                                20);

    auto middleLeft = middleRow.removeFromLeft(middleRow.getWidth() / 2);
    panOneSlider.setBounds(middleLeft.reduced(10));
    panOneLabel.setJustificationType(juce::Justification::centred);
    panOneLabel.setBounds(panOneSlider.getX(),
                          panOneSlider.getBottom() + 4,
                          panOneSlider.getWidth(),
                          20);

    panTwoSlider.setBounds(middleRow.reduced(10));
    panTwoLabel.setJustificationType(juce::Justification::centred);
    panTwoLabel.setBounds(panTwoSlider.getX(),
                          panTwoSlider.getBottom() + 4,
                          panTwoSlider.getWidth(),
                          20);

    gainSlider.setBounds(bottomRow.removeFromTop(60).reduced(10, 0));
    gainLabel.setJustificationType(juce::Justification::centred);
    gainLabel.setBounds(gainSlider.getX(),
                        gainSlider.getY() - 24,
                        gainSlider.getWidth(),
                        20);
}

void DualToneGeneratorAudioProcessorEditor::timerCallback()
{
    const auto stereo = processorRef.isStereoOutput();
    panOneSlider.setEnabled(stereo);
    panTwoSlider.setEnabled(stereo);
}

void DualToneGeneratorAudioProcessorEditor::configureSlider(juce::Slider& slider,
                                                            juce::Label& label,
                                                            const juce::String& labelText,
                                                            juce::Slider::SliderStyle style)
{
    slider.setSliderStyle(style);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 72, 20);
    addAndMakeVisible(slider);

    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(14.0f, juce::Font::bold));
    addAndMakeVisible(label);
}
