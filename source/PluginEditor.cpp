#include "PluginEditor.h"
#include "PluginProcessor.h"

class MinimalDialLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MinimalDialLookAndFeel()
    {
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::black.withAlpha(0.85f));
        setColour(juce::Slider::thumbColourId, juce::Colours::black);
    }

    void drawRotarySlider(juce::Graphics& g,
                          int x,
                          int y,
                          int width,
                          int height,
                          float sliderPos,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
                          juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<float>(static_cast<float>(x),
                                             static_cast<float>(y),
                                             static_cast<float>(width),
                                             static_cast<float>(height))
                          .reduced(2.0f);

        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f - 4.0f;
        auto centre = bounds.getCentre();

        auto outlineColour = slider.findColour(juce::Slider::rotarySliderOutlineColourId);
        auto pointerColour = slider.findColour(juce::Slider::thumbColourId);

        g.setColour(outlineColour);
        g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.5f);

        auto drawTick = [&](float angle, float thickness)
        {
            auto outer = centre.getPointOnCircumference(radius, angle);
            auto inner = centre.getPointOnCircumference(radius - 8.0f, angle);
            g.drawLine({ inner, outer }, thickness);
        };

        drawTick(rotaryStartAngle, 1.2f);
        drawTick(rotaryEndAngle, 1.2f);
        drawTick((rotaryStartAngle + rotaryEndAngle) * 0.5f, 1.0f);

        const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto pointerInner = centre.getPointOnCircumference(radius * 0.2f, angle);
        auto pointerOuter = centre.getPointOnCircumference(radius - 6.0f, angle);

        g.setColour(pointerColour);
        juce::Path pointerPath;
        pointerPath.addLineSegment({ pointerInner, pointerOuter }, 2.4f);
        g.strokePath(pointerPath, juce::PathStrokeType(2.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        g.fillEllipse(centre.x - 2.2f, centre.y - 2.2f, 4.4f, 4.4f);
    }
};

DualToneGeneratorAudioProcessorEditor::DualToneGeneratorAudioProcessorEditor(DualToneGeneratorAudioProcessor& processor)
    : juce::AudioProcessorEditor(&processor),
      processorRef(processor),
      centerAttachment(processorRef.getValueTreeState(), "centerFreq", centerSlider),
      spreadAttachment(processorRef.getValueTreeState(), "spread", spreadSlider),
      panOneAttachment(processorRef.getValueTreeState(), "pan1", panOneSlider),
      panTwoAttachment(processorRef.getValueTreeState(), "pan2", panTwoSlider),
      attenuationOneAttachment(processorRef.getValueTreeState(), "atten1", attenuationOneSlider),
      attenuationTwoAttachment(processorRef.getValueTreeState(), "atten2", attenuationTwoSlider),
      dialLookAndFeel(std::make_unique<MinimalDialLookAndFeel>())
{
    configureSlider(centerSlider, centerLabel, "CENTER", juce::Slider::RotaryVerticalDrag);
    configureSlider(spreadSlider, spreadLabel, "SPREAD", juce::Slider::RotaryVerticalDrag);
    configureSlider(panOneSlider, panOneLabel, "PAN", juce::Slider::RotaryVerticalDrag);
    configureSlider(attenuationOneSlider, attenuationOneLabel, "ATTN", juce::Slider::RotaryVerticalDrag);
    configureSlider(attenuationTwoSlider, attenuationTwoLabel, "ATTN", juce::Slider::RotaryVerticalDrag);
    configureSlider(panTwoSlider, panTwoLabel, "PAN", juce::Slider::RotaryVerticalDrag);

    const auto startAngle = juce::degreesToRadians(210.0f);
    const auto endAngle = juce::degreesToRadians(510.0f);

    for (auto* slider : { &centerSlider, &spreadSlider, &panOneSlider, &panTwoSlider, &attenuationOneSlider, &attenuationTwoSlider })
    {
        slider->setLookAndFeel(dialLookAndFeel.get());
        slider->setRotaryParameters(startAngle, endAngle, true);
        slider->setPopupDisplayEnabled(true, false, this);
    }

    centerSlider.setTextValueSuffix(" Hz");
    centerSlider.setNumDecimalPlacesToDisplay(1);

    spreadSlider.setTextValueSuffix(" Hz");
    spreadSlider.setNumDecimalPlacesToDisplay(2);
    spreadSlider.textFromValueFunction = [](double value)
    {
        return juce::String(value * 2.0, 2) + " Hz";
    };
    spreadSlider.valueFromTextFunction = [](const juce::String& text)
    {
        auto cleaned = text.retainCharacters("0123456789.,");
        cleaned = cleaned.replaceCharacter(',', '.');
        return cleaned.getDoubleValue() * 0.5;
    };

    panOneSlider.setNumDecimalPlacesToDisplay(2);
    panTwoSlider.setNumDecimalPlacesToDisplay(2);

    attenuationOneSlider.setTextValueSuffix(" dB");
    attenuationTwoSlider.setTextValueSuffix(" dB");
    attenuationOneSlider.setNumDecimalPlacesToDisplay(1);
    attenuationTwoSlider.setNumDecimalPlacesToDisplay(1);

    auto initStaticLabel = [this](juce::Label& label, const juce::String& text, float fontSize, bool bold = false)
    {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colours::black);
        label.setFont(juce::Font(fontSize, bold ? juce::Font::bold : juce::Font::plain));

        if (label.getParentComponent() == nullptr)
            addAndMakeVisible(label);
        else
            label.setVisible(true);
    };

    initStaticLabel(centerLabel, "CENTER", 26.0f, true);
    initStaticLabel(spreadLabel, "SPREAD", 26.0f, true);
    initStaticLabel(panOneLabel, "PAN", 13.0f);
    initStaticLabel(panTwoLabel, "PAN", 13.0f);
    initStaticLabel(attenuationOneLabel, "ATTN", 13.0f);
    initStaticLabel(attenuationTwoLabel, "ATTN", 13.0f);
    initStaticLabel(toneOneTitleLabel, "TONE 1", 15.0f, true);
    initStaticLabel(toneTwoTitleLabel, "TONE 2", 15.0f, true);
    initStaticLabel(centerUnitLabel, "Hz", 15.0f);
    initStaticLabel(spreadUnitLabel, "Hz", 15.0f);
    initStaticLabel(centerMinLabel, "60", 15.0f);
    initStaticLabel(centerMaxLabel, "600", 15.0f);
    initStaticLabel(spreadMinLabel, "0", 15.0f);
    initStaticLabel(spreadMaxLabel, "20", 15.0f);

    setSize(720, 540);
    startTimerHz(10);
}

DualToneGeneratorAudioProcessorEditor::~DualToneGeneratorAudioProcessorEditor()
{
    stopTimer();

    for (auto* slider : { &centerSlider, &spreadSlider, &panOneSlider, &panTwoSlider, &attenuationOneSlider, &attenuationTwoSlider })
        slider->setLookAndFeel(nullptr);
}

void DualToneGeneratorAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);
}

void DualToneGeneratorAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(36);

    auto topArea = bounds.removeFromTop(static_cast<int>(bounds.getHeight() * 0.58f));
    bounds.removeFromTop(60);
    auto bottomArea = bounds;

    auto layoutLargeDial = [](juce::Slider& slider,
                              juce::Rectangle<int> area,
                              juce::Label& title,
                              juce::Label& unit,
                              juce::Label& minLabel,
                              juce::Label& maxLabel)
    {
        auto section = area.reduced(10, 0);
        const int titleHeight = 40;
        title.setBounds(section.removeFromTop(titleHeight));
        section.removeFromTop(12);

        const int available = juce::jmin(section.getWidth(), section.getHeight());
        const int dialSize = juce::jmax(available - 20, 180);
        auto dialBounds = section.withSizeKeepingCentre(dialSize, dialSize);
        slider.setBounds(dialBounds);

        unit.setBounds(juce::Rectangle<int>(60, 24)
                           .withCentre({ dialBounds.getCentreX(), dialBounds.getY() - 10 }));

        const auto radius = static_cast<float>(dialBounds.getWidth()) * 0.5f;
        const auto centre = juce::Point<float>(static_cast<float>(dialBounds.getCentreX()),
                                               static_cast<float>(dialBounds.getCentreY()));
        const auto params = slider.getRotaryParameters();

        const auto minPoint = centre.getPointOnCircumference(radius + 2.0f, params.startAngleRadians);
        const auto maxPoint = centre.getPointOnCircumference(radius + 2.0f, params.endAngleRadians);

        auto minBounds = juce::Rectangle<int>(60, 20);
        minBounds.setCentre(minPoint.toInt());
        minBounds.translate(0, 14);

        auto maxBounds = juce::Rectangle<int>(60, 20);
        maxBounds.setCentre(maxPoint.toInt());
        maxBounds.translate(0, 14);

        minLabel.setBounds(minBounds);
        maxLabel.setBounds(maxBounds);
    };

    auto layoutSmallDial = [](juce::Slider& slider, juce::Label& label, juce::Rectangle<int> area)
    {
        auto slot = area.reduced(24, 0);
        const int labelHeight = 24;
        const int dialSize = juce::jmax(juce::jmin(slot.getWidth(), slot.getHeight() - labelHeight) - 10, 90);
        auto dialBounds = juce::Rectangle<int>(dialSize, dialSize).withCentre(slot.getCentre());
        dialBounds.setY(slot.getY());
        slider.setBounds(dialBounds);

        auto labelBounds = juce::Rectangle<int>(dialBounds.getWidth() + 20, labelHeight);
        labelBounds.setCentre({ dialBounds.getCentreX(), dialBounds.getBottom() + labelHeight });
        label.setBounds(labelBounds);
    };

    auto layoutToneSection = [&](juce::Rectangle<int> area,
                                 juce::Label& title,
                                 juce::Slider& firstSlider,
                                 juce::Label& firstLabel,
                                 juce::Slider& secondSlider,
                                 juce::Label& secondLabel)
    {
        auto section = area;
        const int titleHeight = 26;
        title.setBounds(section.removeFromTop(titleHeight));
        section.removeFromTop(6);
        section = section.reduced(8, 0);

        auto firstArea = section.removeFromLeft(section.getWidth() / 2);
        auto secondArea = section;

        layoutSmallDial(firstSlider, firstLabel, firstArea);
        layoutSmallDial(secondSlider, secondLabel, secondArea);
    };

    auto centerArea = topArea.removeFromLeft(topArea.getWidth() / 2);
    auto spreadArea = topArea;
    layoutLargeDial(centerSlider, centerArea, centerLabel, centerUnitLabel, centerMinLabel, centerMaxLabel);
    layoutLargeDial(spreadSlider, spreadArea, spreadLabel, spreadUnitLabel, spreadMinLabel, spreadMaxLabel);

    const int toneBlockGap = 72;
    auto toneOneArea = bottomArea.removeFromLeft((bottomArea.getWidth() - toneBlockGap) / 2);
    bottomArea.removeFromLeft(toneBlockGap);
    auto toneTwoArea = bottomArea;
    layoutToneSection(toneOneArea, toneOneTitleLabel, panOneSlider, panOneLabel, attenuationOneSlider, attenuationOneLabel);
    layoutToneSection(toneTwoArea, toneTwoTitleLabel, attenuationTwoSlider, attenuationTwoLabel, panTwoSlider, panTwoLabel);
}

void DualToneGeneratorAudioProcessorEditor::timerCallback()
{
    const auto stereo = processorRef.isStereoOutput();
    panOneSlider.setEnabled(stereo);
    panTwoSlider.setEnabled(stereo);

    const auto panLabelColour = stereo ? juce::Colours::black : juce::Colours::grey;
    panOneLabel.setColour(juce::Label::textColourId, panLabelColour);
    panTwoLabel.setColour(juce::Label::textColourId, panLabelColour);
}

void DualToneGeneratorAudioProcessorEditor::configureSlider(juce::Slider& slider,
                                                            juce::Label& label,
                                                            const juce::String& labelText,
                                                            juce::Slider::SliderStyle style)
{
    slider.setSliderStyle(style);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(slider);

    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(14.0f, juce::Font::bold));
    addAndMakeVisible(label);
}
