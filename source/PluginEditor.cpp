#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "BinaryData.h"

namespace
{
constexpr int defaultEditorWidth = 720;
constexpr int defaultEditorHeight = 540;
constexpr float minEditorScale = 0.5f;
constexpr float maxEditorScale = 2.0f;

const juce::Colour backgroundMidColour { 228, 214, 202 };
const juce::Colour backgroundTopColour = backgroundMidColour.darker(0.2f);
const juce::Colour backgroundBottomColour = backgroundMidColour.darker(0.4f);
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

        const auto shadowRadius = juce::jmax(juce::roundToInt(knobRadius * 0.18f), 4);
        const auto shadowOffsetY = juce::jmax(juce::roundToInt(knobRadius * 0.10f), 1);

        juce::Path knobShadowPath;
        knobShadowPath.addEllipse(knobBounds);

        juce::DropShadow knobShadow { juce::Colours::black.withAlpha(0.16f),
                                      shadowRadius,
                                      { 0, shadowOffsetY } };
        knobShadow.drawForPath(g, knobShadowPath);

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
        slider->setPaintingIsUnclipped(true);
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

    const auto minWidth = juce::roundToInt(static_cast<float>(defaultEditorWidth) * minEditorScale);
    const auto minHeight = juce::roundToInt(static_cast<float>(defaultEditorHeight) * minEditorScale);
    const auto maxWidth = juce::roundToInt(static_cast<float>(defaultEditorWidth) * maxEditorScale);
    const auto maxHeight = juce::roundToInt(static_cast<float>(defaultEditorHeight) * maxEditorScale);

    setResizeLimits(minWidth, minHeight, maxWidth, maxHeight);
    setResizable(true, true);

    if (auto* constrainer = getConstrainer())
        constrainer->setFixedAspectRatio(static_cast<double>(defaultEditorWidth) / static_cast<double>(defaultEditorHeight));

    setSize(defaultEditorWidth, defaultEditorHeight);
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
}

void DualToneGeneratorAudioProcessorEditor::resized()
{
    const auto width = getWidth();
    const auto height = getHeight();
    auto scale = juce::jmin(static_cast<float>(width) / static_cast<float>(defaultEditorWidth),
                            static_cast<float>(height) / static_cast<float>(defaultEditorHeight));
    scale = juce::jlimit(minEditorScale, maxEditorScale, scale);

    updateScaledStyles(scale);

    auto rootBounds = getLocalBounds();

    const auto margin = juce::roundToInt(36.0f * scale);
    auto contentArea = rootBounds.reduced(margin);

    auto expandedContent = contentArea;
    const auto expandTop = juce::roundToInt(20.0f * scale);
    const auto expandBottom = juce::roundToInt(60.0f * scale);
    expandedContent.setTop(juce::jmax(rootBounds.getY(), expandedContent.getY() - expandTop));
    expandedContent.setBottom(juce::jmin(rootBounds.getBottom(), expandedContent.getBottom() + expandBottom));
    contentPanelBounds = expandedContent;

    auto workingArea = contentArea;
    auto topArea = workingArea.removeFromTop(static_cast<int>(workingArea.getHeight() * 0.58f));
    workingArea.removeFromTop(juce::roundToInt(60.0f * scale));
    auto bottomArea = workingArea;

    const auto largeDialPadding = juce::roundToInt(10.0f * scale);
    const auto largeDialSpacing = juce::roundToInt(12.0f * scale);
    const auto largeDialInset = juce::roundToInt(20.0f * scale);
    const auto largeTitleHeight = juce::roundToInt(40.0f * scale);
    const auto unitWidth = juce::roundToInt(60.0f * scale);
    const auto unitHeight = juce::roundToInt(24.0f * scale);
    const auto labelYOffset = juce::roundToInt(14.0f * scale);
    const auto minMaxHeight = juce::roundToInt(20.0f * scale);

    auto layoutLargeDial = [&](juce::Slider& slider,
                               juce::Rectangle<int> area,
                               juce::Label& title,
                               juce::Label& unit,
                               juce::Label& minLabel,
                               juce::Label& maxLabel)
    {
        auto section = area.reduced(largeDialPadding, 0);
        title.setBounds(section.removeFromTop(largeTitleHeight));
        section.removeFromTop(largeDialSpacing);

        const int available = juce::jmin(section.getWidth(), section.getHeight());
        const int maxDial = juce::jmax(available, 0);
        const int minDial = juce::jmin(juce::roundToInt(180.0f * scale), maxDial);
        const int desiredDial = juce::jmax(0, available - largeDialInset);
        const int dialSize = juce::jlimit(minDial, desiredDial, maxDial);

        auto dialBounds = section.withSizeKeepingCentre(dialSize, dialSize);
        slider.setBounds(dialBounds);

        unit.setBounds(juce::Rectangle<int>(unitWidth, unitHeight)
                           .withCentre({ dialBounds.getCentreX(), dialBounds.getY() - labelYOffset }));

        const auto radius = static_cast<float>(dialBounds.getWidth()) * 0.5f;
        const auto centre = juce::Point<float>(static_cast<float>(dialBounds.getCentreX()),
                                               static_cast<float>(dialBounds.getCentreY()));
        const auto params = slider.getRotaryParameters();

        const auto minPoint = centre.getPointOnCircumference(radius + 2.0f, params.startAngleRadians);
        const auto maxPoint = centre.getPointOnCircumference(radius + 2.0f, params.endAngleRadians);

        auto minBounds = juce::Rectangle<int>(unitWidth, minMaxHeight);
        minBounds.setCentre(minPoint.toInt());
        minBounds.translate(0, labelYOffset);

        auto maxBounds = juce::Rectangle<int>(unitWidth, minMaxHeight);
        maxBounds.setCentre(maxPoint.toInt());
        maxBounds.translate(0, labelYOffset);

        minLabel.setBounds(minBounds);
        maxLabel.setBounds(maxBounds);
    };

    const auto smallDialMargin = juce::roundToInt(24.0f * scale);
    const auto smallDialInset = juce::roundToInt(10.0f * scale);
    const auto smallLabelHeight = juce::roundToInt(18.0f * scale);
    const auto smallLabelExtraWidth = juce::roundToInt(20.0f * scale);
    const auto smallDialMinSize = juce::roundToInt(90.0f * scale);

    auto layoutSmallDial = [&](juce::Slider& slider, juce::Label& label, juce::Rectangle<int> area)
    {
        auto slot = area.reduced(smallDialMargin, 0);
        const int available = juce::jmin(slot.getWidth(), juce::jmax(0, slot.getHeight() - smallLabelHeight));
        const int maxDial = juce::jmax(slot.getWidth(), 0);
        const int minDial = juce::jmin(smallDialMinSize, maxDial);
        const int desiredDial = juce::jmax(0, available - smallDialInset);
        const int dialSize = juce::jlimit(minDial, desiredDial, maxDial);

        auto dialBounds = juce::Rectangle<int>(dialSize, dialSize).withCentre(slot.getCentre());
        dialBounds.setY(slot.getY());
        slider.setBounds(dialBounds);

        auto labelBounds = juce::Rectangle<int>(dialBounds.getWidth() + smallLabelExtraWidth, smallLabelHeight);
        labelBounds.setCentre({ dialBounds.getCentreX(), dialBounds.getBottom() + smallLabelHeight });
        label.setBounds(labelBounds);
    };

    const auto toneTitleHeight = juce::roundToInt(26.0f * scale);
    const auto toneSectionInset = juce::roundToInt(8.0f * scale);
    const auto toneSectionLift = juce::roundToInt(6.0f * scale);

    auto layoutToneSection = [&](juce::Rectangle<int> area,
                                 juce::Label& title,
                                 juce::Slider& firstSlider,
                                 juce::Label& firstLabel,
                                 juce::Slider& secondSlider,
                                 juce::Label& secondLabel)
    {
        auto section = area;
        title.setBounds(section.removeFromTop(toneTitleHeight));
        section = section.reduced(toneSectionInset, 0);
        section.translate(0, -toneSectionLift);

        auto firstArea = section.removeFromLeft(section.getWidth() / 2);
        auto secondArea = section;

        layoutSmallDial(firstSlider, firstLabel, firstArea);
        layoutSmallDial(secondSlider, secondLabel, secondArea);
    };

    auto centerArea = topArea.removeFromLeft(topArea.getWidth() / 2);
    auto spreadArea = topArea;
    layoutLargeDial(centerSlider, centerArea, centerLabel, centerUnitLabel, centerMinLabel, centerMaxLabel);
    layoutLargeDial(spreadSlider, spreadArea, spreadLabel, spreadUnitLabel, spreadMinLabel, spreadMaxLabel);

    const auto panelExpandSmall = juce::roundToInt(20.0f * scale);
    const auto tonePanelExpandTop = juce::roundToInt(40.0f * scale);
    const auto tonePanelExpandBottom = juce::roundToInt(60.0f * scale);

    auto expandPanel = [&](juce::Rectangle<int> area, int extraTop, int extraBottom)
    {
        auto result = area;
        result.setTop(juce::jmax(contentPanelBounds.getY(), result.getY() - extraTop));
        result.setBottom(juce::jmin(contentPanelBounds.getBottom(), result.getBottom() + extraBottom));
        return result;
    };

    centerPanelBounds = expandPanel(centerArea, panelExpandSmall, panelExpandSmall);
    spreadPanelBounds = expandPanel(spreadArea, panelExpandSmall, panelExpandSmall);

    const auto toneBlockGap = juce::roundToInt(72.0f * scale);
    auto toneOneArea = bottomArea.removeFromLeft((bottomArea.getWidth() - toneBlockGap) / 2);
    bottomArea.removeFromLeft(toneBlockGap);
    auto toneTwoArea = bottomArea;
    layoutToneSection(toneOneArea, toneOneTitleLabel, panOneSlider, panOneLabel, attenuationOneSlider, attenuationOneLabel);
    layoutToneSection(toneTwoArea, toneTwoTitleLabel, attenuationTwoSlider, attenuationTwoLabel, panTwoSlider, panTwoLabel);

    toneOnePanelBounds = expandPanel(toneOneArea, tonePanelExpandTop, tonePanelExpandBottom);
    toneTwoPanelBounds = expandPanel(toneTwoArea, tonePanelExpandTop, tonePanelExpandBottom);
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

void DualToneGeneratorAudioProcessorEditor::updateScaledStyles(float scale)
{
    auto setLabelFont = [scale](juce::Label& label, float baseSize, bool bold = false)
    {
        label.setFont(juce::Font(baseSize * scale, bold ? juce::Font::bold : juce::Font::plain));
    };

    setLabelFont(centerLabel, 26.0f, true);
    setLabelFont(spreadLabel, 26.0f, true);
    setLabelFont(panOneLabel, 18.0f);
    setLabelFont(panTwoLabel, 18.0f);
    setLabelFont(attenuationOneLabel, 18.0f);
    setLabelFont(attenuationTwoLabel, 18.0f);
    setLabelFont(toneOneTitleLabel, 18.0f, true);
    setLabelFont(toneTwoTitleLabel, 18.0f, true);
    setLabelFont(centerUnitLabel, 18.0f);
    setLabelFont(spreadUnitLabel, 18.0f);
    setLabelFont(centerMinLabel, 18.0f);
    setLabelFont(centerMaxLabel, 18.0f);
    setLabelFont(spreadMinLabel, 18.0f);
    setLabelFont(spreadMaxLabel, 18.0f);
}
