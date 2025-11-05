#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "BinaryData.h"

namespace
{
const juce::Colour backgroundTopColour { 42, 36, 33 };
const juce::Colour backgroundMidColour { 66, 59, 45 };
const juce::Colour backgroundBottomColour { 26, 21, 19 };
const juce::Colour outerFrameColour { 18, 15, 13 };
const juce::Colour panelBaseColour { 228, 214, 202 };
const juce::Colour labelActiveColour { 73, 56, 44 };
const juce::Colour labelInactiveColour = labelActiveColour.withMultipliedAlpha(0.35f);
const juce::Colour toneAccentColour = labelActiveColour.darker(0.6f);
const juce::Colour dialOutlineColour { 116, 96, 80 };
const juce::Colour redTrackColour { 196, 72, 62 };
const juce::Colour greenTrackColour { 104, 164, 122 };
const juce::Colour blueTrackColour { 74, 132, 198 };
} // namespace

class SvgDialLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SvgDialLookAndFeel(const void* svgData,
                       size_t svgSize,
                       juce::Colour trackColour,
                       juce::Colour outlineColour = dialOutlineColour)
    {
        setColour(juce::Slider::rotarySliderOutlineColourId, outlineColour);
        setColour(juce::Slider::trackColourId, trackColour);
        setColour(juce::Slider::thumbColourId, trackColour.brighter(0.15f));

        knobDrawable = juce::Drawable::createFromImageData(svgData, svgSize);
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
                          .reduced(6.0f);

        const auto diameter = juce::jmin(bounds.getWidth(), bounds.getHeight());
        auto knobBounds = juce::Rectangle<float>(diameter, diameter).withCentre(bounds.getCentre());
        const auto centre = knobBounds.getCentre();
        const auto knobRadius = knobBounds.getWidth() * 0.5f;

        const auto trackRadius = knobRadius - 14.0f;
        const auto trackThickness = juce::jmax(knobBounds.getWidth() * 0.045f, 3.0f);

        const auto outlineColour = slider.findColour(juce::Slider::rotarySliderOutlineColourId);
        const auto accentColour = slider.findColour(juce::Slider::trackColourId);

        const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        const auto knobAngle = angle;

        juce::Path track;
        track.addCentredArc(centre.x, centre.y, trackRadius, trackRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(outlineColour.withAlpha(0.20f));
        g.strokePath(track, juce::PathStrokeType(trackThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path valueArc;
        valueArc.addCentredArc(centre.x, centre.y, trackRadius, trackRadius, 0.0f, rotaryStartAngle, angle, true);
        g.setColour(accentColour.withAlpha(0.75f));
        g.strokePath(valueArc, juce::PathStrokeType(trackThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        auto drawTick = [&](float tickAngle, float thickness, float alpha)
        {
            const auto outer = centre.getPointOnCircumference(trackRadius + 10.0f, tickAngle);
            const auto inner = centre.getPointOnCircumference(trackRadius - 6.0f, tickAngle);
            g.setColour(outlineColour.withAlpha(alpha));
            g.drawLine({ inner, outer }, thickness);
        };

        drawTick(rotaryStartAngle, 1.6f, 0.35f);
        drawTick(rotaryEndAngle, 1.6f, 0.35f);
        drawTick((rotaryStartAngle + rotaryEndAngle) * 0.5f, 1.8f, 0.45f);

        if (knobDrawable != nullptr)
        {
            juce::Graphics::ScopedSaveState state(g);
            g.addTransform(juce::AffineTransform::rotation(knobAngle, centre.x, centre.y));
            knobDrawable->drawWithin(g, knobBounds, juce::RectanglePlacement::stretchToFit, 1.0f);
        }
        else
        {
            g.setColour(juce::Colours::darkgrey);
            g.fillEllipse(knobBounds);
        }
    }

private:
    std::unique_ptr<juce::Drawable> knobDrawable;
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
      redDialLookAndFeel(std::make_unique<SvgDialLookAndFeel>(BinaryData::cog_knob_red_svg,
                                                              BinaryData::cog_knob_red_svgSize,
                                                              redTrackColour)),
      greenDialLookAndFeel(std::make_unique<SvgDialLookAndFeel>(BinaryData::cog_knob_green_svg,
                                                                BinaryData::cog_knob_green_svgSize,
                                                                greenTrackColour)),
      blueDialLookAndFeel(std::make_unique<SvgDialLookAndFeel>(BinaryData::cog_knob_blue_svg,
                                                               BinaryData::cog_knob_blue_svgSize,
                                                               blueTrackColour))
{
    configureSlider(centerSlider, centerLabel, "CENTER", juce::Slider::RotaryVerticalDrag);
    configureSlider(spreadSlider, spreadLabel, "SPREAD", juce::Slider::RotaryVerticalDrag);
    configureSlider(panOneSlider, panOneLabel, "PAN", juce::Slider::RotaryVerticalDrag);
    configureSlider(attenuationOneSlider, attenuationOneLabel, "ATTN", juce::Slider::RotaryVerticalDrag);
    configureSlider(attenuationTwoSlider, attenuationTwoLabel, "ATTN", juce::Slider::RotaryVerticalDrag);
    configureSlider(panTwoSlider, panTwoLabel, "PAN", juce::Slider::RotaryVerticalDrag);

    const auto startAngle = juce::degreesToRadians(225.0f);
    const auto endAngle = juce::degreesToRadians(495.0f);

    for (auto* slider : { &centerSlider, &spreadSlider, &panOneSlider, &panTwoSlider, &attenuationOneSlider, &attenuationTwoSlider })
    {
        slider->setRotaryParameters(startAngle, endAngle, true);
        slider->setPopupDisplayEnabled(true, false, this);
        slider->setColour(juce::Slider::textBoxTextColourId, labelActiveColour);
        slider->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        slider->setColour(juce::Slider::textBoxBackgroundColourId, panelBaseColour);
    }

    centerSlider.setLookAndFeel(redDialLookAndFeel.get());
    spreadSlider.setLookAndFeel(redDialLookAndFeel.get());
    panOneSlider.setLookAndFeel(greenDialLookAndFeel.get());
    panTwoSlider.setLookAndFeel(greenDialLookAndFeel.get());
    attenuationOneSlider.setLookAndFeel(blueDialLookAndFeel.get());
    attenuationTwoSlider.setLookAndFeel(blueDialLookAndFeel.get());

    centerSlider.setTextValueSuffix(" Hz");
    centerSlider.setNumDecimalPlacesToDisplay(1);

    spreadSlider.setTextValueSuffix(" Hz");
    spreadSlider.setNumDecimalPlacesToDisplay(2);
    spreadSlider.textFromValueFunction = [](double value)
    {
        return juce::String(value * 2.0, 2);
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

    auto initStaticLabel = [this](juce::Label& label,
                                  const juce::String& text,
                                  float fontSize,
                                  bool bold = false,
                                  juce::Colour colour = labelActiveColour)
    {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, colour);
        label.setFont(juce::Font(fontSize, bold ? juce::Font::bold : juce::Font::plain));

        if (label.getParentComponent() == nullptr)
            addAndMakeVisible(label);
        else
            label.setVisible(true);
    };

    initStaticLabel(centerLabel, "CENTER", 26.0f, true, toneAccentColour.darker(0.05f));
    initStaticLabel(spreadLabel, "SPREAD", 26.0f, true, toneAccentColour.darker(0.05f));
    initStaticLabel(panOneLabel, "PAN", 18.0f);
    initStaticLabel(panTwoLabel, "PAN", 18.0f);
    initStaticLabel(attenuationOneLabel, "ATTN", 18.0f);
    initStaticLabel(attenuationTwoLabel, "ATTN", 18.0f);
    initStaticLabel(toneOneTitleLabel, "TONE 1", 18.0f, true, toneAccentColour);
    initStaticLabel(toneTwoTitleLabel, "TONE 2", 18.0f, true, toneAccentColour);
    initStaticLabel(centerUnitLabel, "Hz", 18.0f, false, labelActiveColour.withMultipliedAlpha(0.7f));
    initStaticLabel(spreadUnitLabel, "Hz", 18.0f, false, labelActiveColour.withMultipliedAlpha(0.7f));
    initStaticLabel(centerMinLabel, "60", 18.0f, false, labelActiveColour.withMultipliedAlpha(0.55f));
    initStaticLabel(centerMaxLabel, "600", 18.0f, false, labelActiveColour.withMultipliedAlpha(0.55f));
    initStaticLabel(spreadMinLabel, "0", 18.0f, false, labelActiveColour.withMultipliedAlpha(0.55f));
    initStaticLabel(spreadMaxLabel, "40", 18.0f, false, labelActiveColour.withMultipliedAlpha(0.55f));

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
    const auto fullBounds = getLocalBounds().toFloat();

    juce::ColourGradient backgroundGradient(backgroundTopColour,
                                            fullBounds.getCentreX(),
                                            fullBounds.getY(),
                                            backgroundBottomColour,
                                            fullBounds.getCentreX(),
                                            fullBounds.getBottom(),
                                            false);
    backgroundGradient.addColour(0.35f, backgroundMidColour);
    g.setGradientFill(backgroundGradient);
    g.fillRect(fullBounds);

    auto frameBounds = fullBounds.reduced(8.0f);
    g.setColour(outerFrameColour.withAlpha(0.65f));
    g.drawRoundedRectangle(frameBounds, 28.0f, 2.0f);

    g.setColour(juce::Colours::black.withAlpha(0.12f));
    g.drawRoundedRectangle(frameBounds.reduced(6.0f), 24.0f, 1.5f);

    auto drawPanel = [&](const juce::Rectangle<int>& area)
    {
        if (area.isEmpty())
            return;

        juce::DropShadow panelShadow(juce::Colour(0x26000000), 18, { 0, 8 });
        panelShadow.drawForRectangle(g, area);

        auto panelBounds = area.toFloat();
        if (panelBounds.getWidth() <= 0.0f || panelBounds.getHeight() <= 0.0f)
            return;

        juce::ColourGradient panelGradient(panelBaseColour.brighter(0.12f),
                                           panelBounds.getX(),
                                           panelBounds.getY(),
                                           panelBaseColour.darker(0.08f),
                                           panelBounds.getX(),
                                           panelBounds.getBottom(),
                                           false);
        panelGradient.addColour(0.48f, panelBaseColour);
        g.setGradientFill(panelGradient);
        g.fillRoundedRectangle(panelBounds, 22.0f);

        g.setColour(panelBaseColour.brighter(0.2f).withAlpha(0.55f));
        g.drawRoundedRectangle(panelBounds.reduced(3.5f), 18.0f, 1.2f);

        g.setColour(panelBaseColour.darker(0.35f).withAlpha(0.45f));
        g.drawRoundedRectangle(panelBounds, 22.0f, 1.6f);
    };

    auto drawInset = [&](const juce::Rectangle<int>& area, float cornerSize, juce::Colour tint)
    {
        if (area.isEmpty())
            return;

        auto insetBounds = area.reduced(18, 16).toFloat();
        if (insetBounds.getWidth() <= 0.0f || insetBounds.getHeight() <= 0.0f)
            return;

        juce::ColourGradient insetGradient(panelBaseColour.darker(0.04f),
                                           insetBounds.getX(),
                                           insetBounds.getY(),
                                           panelBaseColour.brighter(0.08f),
                                           insetBounds.getX(),
                                           insetBounds.getBottom(),
                                           false);
        insetGradient.addColour(0.25f, panelBaseColour.brighter(0.04f));
        insetGradient.addColour(0.75f, panelBaseColour.darker(0.02f));
        g.setGradientFill(insetGradient);
        g.fillRoundedRectangle(insetBounds, cornerSize);

        g.setColour(tint.withAlpha(0.08f));
        g.fillRoundedRectangle(insetBounds, cornerSize);

        g.setColour(panelBaseColour.brighter(0.25f).withAlpha(0.4f));
        g.drawRoundedRectangle(insetBounds.reduced(2.0f), cornerSize - 2.0f, 1.0f);

        g.setColour(panelBaseColour.darker(0.35f).withAlpha(0.35f));
        g.drawRoundedRectangle(insetBounds, cornerSize, 1.2f);
    };

    drawPanel(contentPanelBounds);

    drawInset(centerPanelBounds, 18.0f, toneAccentColour.withMultipliedAlpha(0.3f));
    drawInset(spreadPanelBounds, 18.0f, toneAccentColour.withMultipliedAlpha(0.2f));

    drawInset(toneOnePanelBounds, 14.0f, toneAccentColour.withMultipliedAlpha(0.18f));
    drawInset(toneTwoPanelBounds, 14.0f, toneAccentColour.withMultipliedAlpha(0.18f));
}

void DualToneGeneratorAudioProcessorEditor::resized()
{
    auto rootBounds = getLocalBounds();
    auto contentArea = rootBounds.reduced(36);

    auto expandedContent = contentArea;
    expandedContent.setTop(juce::jmax(rootBounds.getY(), expandedContent.getY() - 20));
    expandedContent.setBottom(juce::jmin(rootBounds.getBottom(), expandedContent.getBottom() + 60));
    contentPanelBounds = expandedContent;

    auto workingArea = contentArea;
    auto topArea = workingArea.removeFromTop(static_cast<int>(workingArea.getHeight() * 0.58f));
    workingArea.removeFromTop(60);
    auto bottomArea = workingArea;

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
                           .withCentre({ dialBounds.getCentreX(), dialBounds.getY() - 14 }));

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
        const int labelHeight = 18;
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
        section = section.reduced(8, 0);
        section.translate(0, -6);

        auto firstArea = section.removeFromLeft(section.getWidth() / 2);
        auto secondArea = section;

        layoutSmallDial(firstSlider, firstLabel, firstArea);
        layoutSmallDial(secondSlider, secondLabel, secondArea);
    };

    auto centerArea = topArea.removeFromLeft(topArea.getWidth() / 2);
    auto spreadArea = topArea;
    layoutLargeDial(centerSlider, centerArea, centerLabel, centerUnitLabel, centerMinLabel, centerMaxLabel);
    layoutLargeDial(spreadSlider, spreadArea, spreadLabel, spreadUnitLabel, spreadMinLabel, spreadMaxLabel);

    auto expandPanel = [&](juce::Rectangle<int> area, int extraTop, int extraBottom)
    {
        auto result = area;
        result.setTop(juce::jmax(contentPanelBounds.getY(), result.getY() - extraTop));
        result.setBottom(juce::jmin(contentPanelBounds.getBottom(), result.getBottom() + extraBottom));
        return result;
    };

    centerPanelBounds = expandPanel(centerArea, 20, 20);
    spreadPanelBounds = expandPanel(spreadArea, 20, 20);

    const int toneBlockGap = 72;
    auto toneOneArea = bottomArea.removeFromLeft((bottomArea.getWidth() - toneBlockGap) / 2);
    bottomArea.removeFromLeft(toneBlockGap);
    auto toneTwoArea = bottomArea;
    layoutToneSection(toneOneArea, toneOneTitleLabel, panOneSlider, panOneLabel, attenuationOneSlider, attenuationOneLabel);
    layoutToneSection(toneTwoArea, toneTwoTitleLabel, attenuationTwoSlider, attenuationTwoLabel, panTwoSlider, panTwoLabel);

    toneOnePanelBounds = expandPanel(toneOneArea, 40, 60);
    toneTwoPanelBounds = expandPanel(toneTwoArea, 40, 60);
}

void DualToneGeneratorAudioProcessorEditor::timerCallback()
{
    const auto stereo = processorRef.isStereoOutput();
    panOneSlider.setEnabled(stereo);
    panTwoSlider.setEnabled(stereo);

    const auto panLabelColour = stereo ? labelActiveColour : labelInactiveColour;
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
