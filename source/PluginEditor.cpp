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
    const auto rowHeight = bounds.getHeight() / 3;

    auto topRow = bounds.removeFromTop(rowHeight);
    auto middleRow = bounds.removeFromTop(rowHeight);
    auto bottomRow = bounds;

    auto placeRotary = [](juce::Slider& slider, juce::Label& label, juce::Rectangle<int> area)
    {
        slider.setBounds(area);
        label.setBounds(area.withY(area.getBottom() + 4).withHeight(20));
    };

    auto freqOneArea = topRow.removeFromLeft(topRow.getWidth() / 2).reduced(10);
    auto freqTwoArea = topRow.reduced(10);
    placeRotary(frequencyOneSlider, frequencyOneLabel, freqOneArea);
    placeRotary(frequencyTwoSlider, frequencyTwoLabel, freqTwoArea);

    auto panOneArea = middleRow.removeFromLeft(middleRow.getWidth() / 2).reduced(10);
    auto panTwoArea = middleRow.reduced(10);
    placeRotary(panOneSlider, panOneLabel, panOneArea);
    placeRotary(panTwoSlider, panTwoLabel, panTwoArea);

    auto gainArea = bottomRow.reduced(10);
    auto gainLabelArea = gainArea.removeFromTop(24);
    gainLabel.setBounds(gainLabelArea);
    gainSlider.setBounds(gainArea);
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
