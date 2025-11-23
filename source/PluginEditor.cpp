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
const juce::Colour largeDialTrackColour { 196, 72, 62 };
const juce::Colour greenTrackColour { 104, 164, 122 };
const juce::Colour blueTrackColour { 74, 132, 198 };
const juce::Colour grayTrackColour { 124, 118, 112 };

constexpr float logoSvgRectWidth = 800.0f;
constexpr float logoSvgRectHeight = 800.0f;
constexpr float logoSvgRectAspect = logoSvgRectHeight / logoSvgRectWidth;
const juce::Point<float> logoSvgRectCentre { logoSvgRectWidth * 0.5f, logoSvgRectHeight * 0.5f };
constexpr float dualVcoSvgRectX = 740.0f;
constexpr float dualVcoSvgRectY = 310.0f;
constexpr float dualVcoSvgRectWidth = 100.0f;
constexpr float dualVcoSvgRectHeight = 60.0f;
constexpr float dualVcoSvgRectAspect = dualVcoSvgRectHeight / dualVcoSvgRectWidth;
const juce::Point<float> dualVcoSvgRectCentre { dualVcoSvgRectX + dualVcoSvgRectWidth * 0.5f,
                                                dualVcoSvgRectY + dualVcoSvgRectHeight * 0.5f };

// Controls whether the circuit diagram and Dual VCO label are rendered on the UI.
constexpr bool includeDualVcoGraphic = false;
} // namespace



DualToneGeneratorAudioProcessorEditor::DualToneGeneratorAudioProcessorEditor(DualToneGeneratorAudioProcessor& processor)
    : juce::AudioProcessorEditor(&processor),
      processorRef(processor),
      centerAttachment(processorRef.getValueTreeState(), "centerFreq", centerSlider),
      spreadAttachment(processorRef.getValueTreeState(), "spread", spreadSlider),
      panOneAttachment(processorRef.getValueTreeState(), "pan1", panOneSlider),
      panTwoAttachment(processorRef.getValueTreeState(), "pan2", panTwoSlider),
      attenuationOneAttachment(processorRef.getValueTreeState(), "atten1", attenuationOneSlider),
      attenuationTwoAttachment(processorRef.getValueTreeState(), "atten2", attenuationTwoSlider),
      gainAttachment(processorRef.getValueTreeState(), "gain", gainSlider),
      largeDialLookAndFeel(std::make_unique<SvgDialLookAndFeel>(BinaryData::cog_knob_large_svg,
                                                                BinaryData::cog_knob_large_svgSize,
                                                                largeDialTrackColour,
                                                                dialOutlineColour)),
      greenDialLookAndFeel(std::make_unique<SvgDialLookAndFeel>(BinaryData::cog_knob_green_svg,
                                                                BinaryData::cog_knob_green_svgSize,
                                                                greenTrackColour,
                                                                dialOutlineColour)),
      blueDialLookAndFeel(std::make_unique<SvgDialLookAndFeel>(BinaryData::cog_knob_blue_svg,
                                                               BinaryData::cog_knob_blue_svgSize,
                                                               blueTrackColour,
                                                               dialOutlineColour)),
      grayDialLookAndFeel(std::make_unique<SvgDialLookAndFeel>(BinaryData::cog_knob_gray_svg,
                                                               BinaryData::cog_knob_gray_svgSize,
                                                               grayTrackColour,
                                                               dialOutlineColour))
{
    configureSlider(centerSlider, centerLabel, "CENTER", juce::Slider::RotaryVerticalDrag);
    configureSlider(spreadSlider, spreadLabel, "SPREAD", juce::Slider::RotaryVerticalDrag);
    configureSlider(panOneSlider, panOneLabel, "PAN", juce::Slider::RotaryVerticalDrag);
    configureSlider(attenuationOneSlider, attenuationOneLabel, "ATTN", juce::Slider::RotaryVerticalDrag);
    configureSlider(attenuationTwoSlider, attenuationTwoLabel, "ATTN", juce::Slider::RotaryVerticalDrag);
    configureSlider(panTwoSlider, panTwoLabel, "PAN", juce::Slider::RotaryVerticalDrag);
    configureSlider(gainSlider, gainLabel, "GAIN", juce::Slider::RotaryVerticalDrag);

    const auto startAngle = juce::degreesToRadians(225.0f);
    const auto endAngle = juce::degreesToRadians(495.0f);

    for (auto* slider : { &centerSlider, &spreadSlider, &panOneSlider, &panTwoSlider, &attenuationOneSlider, &attenuationTwoSlider, &gainSlider })
    {
        slider->setRotaryParameters(startAngle, endAngle, true);
        slider->setPaintingIsUnclipped(true);
        slider->setPopupDisplayEnabled(true, false, this);
        slider->setColour(juce::Slider::textBoxTextColourId, labelActiveColour);
        slider->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        slider->setColour(juce::Slider::textBoxBackgroundColourId, panelBaseColour);
    }

    centerSlider.setLookAndFeel(largeDialLookAndFeel.get());
    spreadSlider.setLookAndFeel(largeDialLookAndFeel.get());
    panOneSlider.setLookAndFeel(greenDialLookAndFeel.get());
    panTwoSlider.setLookAndFeel(greenDialLookAndFeel.get());
    attenuationOneSlider.setLookAndFeel(blueDialLookAndFeel.get());
    attenuationTwoSlider.setLookAndFeel(blueDialLookAndFeel.get());
    gainSlider.setLookAndFeel(grayDialLookAndFeel.get());

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

    gainSlider.setTextValueSuffix(" dB");
    gainSlider.setNumDecimalPlacesToDisplay(1);

    logoDrawable = juce::Drawable::createFromImageData(BinaryData::logo_svg,
                                                       BinaryData::logo_svgSize);
    if (includeDualVcoGraphic)
        dualVcoDrawable = juce::Drawable::createFromImageData(BinaryData::vco_circuit_svg,
                                                              BinaryData::vco_circuit_svgSize);

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
    initStaticLabel(gainMinLabel, "-12", 18.0f, false, labelActiveColour.withMultipliedAlpha(0.7f));
    gainMinLabel.setJustificationType(juce::Justification::centredRight);
    initStaticLabel(gainMaxLabel, "+12", 18.0f, false, labelActiveColour.withMultipliedAlpha(0.7f));
    initStaticLabel(gainLabel, "dB", 18.0f, false, labelActiveColour.withMultipliedAlpha(0.7f));
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

    for (auto* slider : { &centerSlider, &spreadSlider, &panOneSlider, &panTwoSlider, &attenuationOneSlider, &attenuationTwoSlider, &gainSlider })
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

    const auto highlightColour = backgroundBottomColour.brighter(0.55f);
    const auto shadowColour = backgroundBottomColour.darker(0.25f);

    auto drawDividerGroove = [&](const juce::Line<float>& line)
    {
        if (line.getLength() <= 0.0f)
            return;

        g.setColour(highlightColour);
        g.drawLine(line, toneDividerThickness);

        const auto offset = toneDividerSeparation;
        juce::Line<float> shadowLine({ line.getStartX(), line.getStartY() + offset },
                                     { line.getEndX(), line.getEndY() + offset });
        g.setColour(shadowColour);
        g.drawLine(shadowLine, toneDividerThickness);
    };

    if (!logoBounds.isEmpty())
    {
        if (logoDrawable != nullptr && logoScale > 0.0f)
        {
            juce::Graphics::ScopedSaveState state(g);
            logoDrawable->draw(g, 1.0f, logoTransform);
        }
    }

    if (includeDualVcoGraphic && !dualVcoBounds.isEmpty())
    {
        if (dualVcoDrawable != nullptr && dualVcoScale > 0.0f)
        {
            juce::Graphics::ScopedSaveState state(g);
            dualVcoDrawable->draw(g, 1.0f, dualVcoTransform);
        }

        g.setColour(juce::Colours::black);
        g.setFont(juce::Font(dualVcoLabelFontHeight, juce::Font::bold));

        const auto textInset = juce::jmax(1, juce::roundToInt(static_cast<float>(dualVcoBounds.getHeight()) * 0.15f));
        g.drawFittedText("Dual\nVCO",
                         dualVcoBounds.reduced(textInset),
                         juce::Justification::centred,
                         2);
    }

    drawDividerGroove(toneOneDividerLine);
    drawDividerGroove(toneTwoDividerLine);
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

    auto centerArea = topArea.removeFromLeft(topArea.getWidth() / 2);
    auto spreadArea = topArea;
    layoutLargeDial(centerSlider, centerArea, centerLabel, centerUnitLabel, centerMinLabel, centerMaxLabel, scale);
    layoutLargeDial(spreadSlider, spreadArea, spreadLabel, spreadUnitLabel, spreadMinLabel, spreadMaxLabel, scale);

    const auto dualVcoWidth = juce::roundToInt(70.0f * scale);
    const auto dualVcoHeight = dualVcoWidth > 0
                                   ? juce::jmax(1, juce::roundToInt(static_cast<float>(dualVcoWidth) * dualVcoSvgRectAspect))
                                   : 0;
    dualVcoScale = dualVcoWidth > 0 ? static_cast<float>(dualVcoWidth) / dualVcoSvgRectWidth : 0.0f;
    dualVcoLabelFontHeight = juce::jlimit(8.0f, 24.0f, 14.0f * scale);

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

    logoBounds = {};
    dualVcoBounds = {};
    logoTransform = juce::AffineTransform();
    dualVcoTransform = juce::AffineTransform();
    logoScale = 0.0f;

    if (dualVcoScale > 0.0f && dualVcoHeight > 0)
    {
        const auto centerBounds = centerSlider.getBounds();
        const auto spreadBounds = spreadSlider.getBounds();

        if (!centerBounds.isEmpty() && !spreadBounds.isEmpty())
        {
            const auto midX = juce::roundToInt((centerBounds.getRight() + spreadBounds.getX()) * 0.5f);
            const auto midY = juce::roundToInt((centerBounds.getCentreY() + spreadBounds.getCentreY()) * 0.5f);
            dualVcoBounds = juce::Rectangle<int>(dualVcoWidth, dualVcoHeight)
                                .withCentre({ midX, midY });

            const auto targetCentre = dualVcoBounds.toFloat().getCentre();
            dualVcoTransform = juce::AffineTransform::translation(-dualVcoSvgRectCentre.x, -dualVcoSvgRectCentre.y)
                                   .scaled(dualVcoScale)
                                   .translated(targetCentre.x, targetCentre.y);
        }

        const auto logoWidth = dualVcoWidth > 0 ? juce::roundToInt(static_cast<float>(dualVcoWidth) * 2.0f * 0.8f) : 0;
        const auto logoHeight = logoWidth > 0
                                    ? juce::jmax(1, juce::roundToInt(static_cast<float>(logoWidth) * logoSvgRectAspect))
                                    : 0;
        logoScale = logoWidth > 0 ? static_cast<float>(logoWidth) / logoSvgRectWidth : 0.0f;

        if (logoScale > 0.0f && logoHeight > 0)
        {
            const auto logoGap = juce::roundToInt(18.0f * scale);
            const auto logoDownShift = juce::roundToInt(10.0f * scale);
            const auto logoCentreX = contentPanelBounds.getCentreX();
            const auto targetCentreY = dualVcoBounds.getY() - logoGap - logoHeight / 2 + logoDownShift;
            const auto minCentreY = contentPanelBounds.getY() + logoHeight / 2;
            const auto logoCentre = juce::Point<int>(logoCentreX, juce::jmax(minCentreY, targetCentreY));

            logoBounds = juce::Rectangle<int>(logoWidth, logoHeight).withCentre(logoCentre);

            const auto logoTargetCentre = logoBounds.toFloat().getCentre();
            logoTransform = juce::AffineTransform::translation(-logoSvgRectCentre.x, -logoSvgRectCentre.y)
                                .scaled(logoScale)
                                .translated(logoTargetCentre.x, logoTargetCentre.y);
        }
    }

    const auto toneBlockGap = juce::roundToInt(72.0f * scale);
    auto toneOneArea = bottomArea.removeFromLeft((bottomArea.getWidth() - toneBlockGap) / 2);
    bottomArea.removeFromLeft(toneBlockGap);
    auto toneTwoArea = bottomArea;
    layoutToneSection(toneOneArea, toneOneTitleLabel, panOneSlider, panOneLabel, attenuationOneSlider, attenuationOneLabel, scale);
    layoutToneSection(toneTwoArea, toneTwoTitleLabel, attenuationTwoSlider, attenuationTwoLabel, panTwoSlider, panTwoLabel, scale);

    const auto topDialsCentreY = (centerSlider.getBounds().getCentreY() + spreadSlider.getBounds().getCentreY()) / 2;
    const auto toneDialsCentreY = (panOneSlider.getBounds().getCentreY()
                                   + panTwoSlider.getBounds().getCentreY()
                                   + attenuationOneSlider.getBounds().getCentreY()
                                   + attenuationTwoSlider.getBounds().getCentreY()) / 4;
    const auto gainCentreY = juce::roundToInt(static_cast<float>(topDialsCentreY + toneDialsCentreY) * 0.5f);
    const auto gainAreaWidth = juce::roundToInt(210.0f * scale);
    const auto gainAreaHeight = juce::roundToInt(200.0f * scale);
    auto gainArea = juce::Rectangle<int>(gainAreaWidth, gainAreaHeight)
                        .withCentre({ contentPanelBounds.getCentreX(), gainCentreY })
                        .getIntersection(contentPanelBounds);
    layoutGainDial(gainArea, scale);

    toneOnePanelBounds = expandPanel(toneOneArea, tonePanelExpandTop, tonePanelExpandBottom);
    toneTwoPanelBounds = expandPanel(toneTwoArea, tonePanelExpandTop, tonePanelExpandBottom);

    toneDividerThickness = juce::jmax(1.0f, scale * 0.9f);
    toneDividerSeparation = juce::jmax(1.0f, scale * 0.75f);

    toneOneDividerLine = computeToneDivider(panOneSlider, attenuationOneSlider, toneOneTitleLabel, scale);
    toneTwoDividerLine = computeToneDivider(panTwoSlider, attenuationTwoSlider, toneTwoTitleLabel, scale);

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
    setLabelFont(gainMinLabel, 18.0f);
    setLabelFont(gainMaxLabel, 18.0f);
    setLabelFont(gainLabel, 18.0f);
    setLabelFont(toneOneTitleLabel, 18.0f, true);
    setLabelFont(toneTwoTitleLabel, 18.0f, true);
    setLabelFont(centerUnitLabel, 18.0f);
    setLabelFont(spreadUnitLabel, 18.0f);
    setLabelFont(centerMinLabel, 18.0f);
    setLabelFont(centerMaxLabel, 18.0f);
    setLabelFont(spreadMinLabel, 18.0f);
    setLabelFont(spreadMaxLabel, 18.0f);
}

void DualToneGeneratorAudioProcessorEditor::layoutLargeDial(juce::Slider& slider,
                                                            juce::Rectangle<int> area,
                                                            juce::Label& title,
                                                            juce::Label& unit,
                                                            juce::Label& minLabel,
                                                            juce::Label& maxLabel,
                                                            float scale)
{
    const auto largeDialPadding = juce::roundToInt(10.0f * scale);
    const auto largeDialSpacing = juce::roundToInt(12.0f * scale);
    const auto largeDialInset = juce::roundToInt(20.0f * scale);
    const auto largeTitleHeight = juce::roundToInt(40.0f * scale);
    const auto unitWidth = juce::roundToInt(60.0f * scale);
    const auto unitHeight = juce::roundToInt(24.0f * scale);
    const auto labelYOffset = juce::roundToInt(14.0f * scale);
    const auto minMaxHeight = juce::roundToInt(20.0f * scale);

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
    slider.getProperties().set("uiScale", scale);

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
}

void DualToneGeneratorAudioProcessorEditor::layoutSmallDial(juce::Slider& slider,
                                                            juce::Label& label,
                                                            juce::Rectangle<int> area,
                                                            float scale)
{
    const auto smallDialMargin = juce::roundToInt(24.0f * scale);
    const auto smallDialInset = juce::roundToInt(10.0f * scale);
    const auto smallLabelHeight = juce::roundToInt(18.0f * scale);
    const auto smallLabelExtraWidth = juce::roundToInt(20.0f * scale);
    const auto smallDialMinSize = juce::roundToInt(90.0f * scale);

    auto slot = area.reduced(smallDialMargin, 0);
    const int available = juce::jmin(slot.getWidth(), juce::jmax(0, slot.getHeight() - smallLabelHeight));
    const int maxDial = juce::jmax(slot.getWidth(), 0);
    const int minDial = juce::jmin(smallDialMinSize, maxDial);
    const int desiredDial = juce::jmax(0, available - smallDialInset);
    const int dialSize = juce::jlimit(minDial, desiredDial, maxDial);

    auto dialBounds = juce::Rectangle<int>(dialSize, dialSize).withCentre(slot.getCentre());
    dialBounds.setY(slot.getY());
    slider.setBounds(dialBounds);
    slider.getProperties().set("uiScale", scale);

    auto labelBounds = juce::Rectangle<int>(dialBounds.getWidth() + smallLabelExtraWidth, smallLabelHeight);
    labelBounds.setCentre({ dialBounds.getCentreX(), dialBounds.getBottom() + smallLabelHeight });
    label.setBounds(labelBounds);
}

void DualToneGeneratorAudioProcessorEditor::layoutGainDial(juce::Rectangle<int> area,
                                                           float scale)
{
    const auto labelWidth = juce::roundToInt(110.0f * scale);
    const auto labelHeight = juce::roundToInt(22.0f * scale);
    const auto labelDistance = juce::roundToInt(20.0f * scale);
    const auto labelHorizontalOffset = juce::roundToInt(18.0f * scale);
    const auto labelVerticalLift = juce::roundToInt(10.0f * scale);
    const auto gainVerticalOffset = juce::roundToInt(20.0f * scale);
    const auto gainUnitHeight = juce::roundToInt(22.0f * scale);
    const auto gainUnitGap = juce::roundToInt(4.0f * scale);

    const auto referenceSize = panOneSlider.getBounds().getWidth();
    const auto fallbackSize = juce::roundToInt(90.0f * scale);
    const auto dialSize = referenceSize > 0 ? referenceSize : fallbackSize;

    auto dialBounds = juce::Rectangle<int>(dialSize, dialSize).withCentre(area.getCentre());
    dialBounds.translate(0, -gainVerticalOffset);
    gainSlider.setBounds(dialBounds);
    gainSlider.getProperties().set("uiScale", scale);

    const auto unitLabelWidth = juce::roundToInt(static_cast<float>(dialBounds.getWidth()) * 0.8f);
    auto unitLabelBounds = juce::Rectangle<int>(unitLabelWidth, gainUnitHeight)
                               .withCentre({ dialBounds.getCentreX(), dialBounds.getY() - gainUnitGap - gainUnitHeight / 2 });
    gainLabel.setBounds(unitLabelBounds);

    const auto radius = static_cast<float>(dialBounds.getWidth()) * 0.5f;
    const auto centre = juce::Point<float>(static_cast<float>(dialBounds.getCentreX()),
                                           static_cast<float>(dialBounds.getCentreY()));
    const auto params = gainSlider.getRotaryParameters();

    const auto minPoint = centre.getPointOnCircumference(radius + static_cast<float>(labelDistance),
                                                         params.startAngleRadians);
    const auto maxPoint = centre.getPointOnCircumference(radius + static_cast<float>(labelDistance),
                                                         params.endAngleRadians);

    auto minCentre = juce::Point<int>(juce::roundToInt(minPoint.x), juce::roundToInt(minPoint.y) - labelVerticalLift);
    auto minBounds = juce::Rectangle<int>(labelWidth, labelHeight).withCentre(minCentre);
    minBounds.setRight(juce::roundToInt(minPoint.x) + labelHorizontalOffset);
    gainMinLabel.setBounds(minBounds);

    auto maxCentre = juce::Point<int>(juce::roundToInt(maxPoint.x), juce::roundToInt(maxPoint.y) - labelVerticalLift);
    auto maxBounds = juce::Rectangle<int>(labelWidth, labelHeight).withCentre(maxCentre);
    gainMaxLabel.setBounds(maxBounds);
}

void DualToneGeneratorAudioProcessorEditor::layoutToneSection(juce::Rectangle<int> area,
                                                              juce::Label& title,
                                                              juce::Slider& firstSlider,
                                                              juce::Label& firstLabel,
                                                              juce::Slider& secondSlider,
                                                              juce::Label& secondLabel,
                                                              float scale)
{
    const auto toneTitleHeight = juce::roundToInt(26.0f * scale);
    const auto toneSectionInset = juce::roundToInt(8.0f * scale);
    const auto toneSectionLift = juce::roundToInt(6.0f * scale);

    auto section = area;
    title.setBounds(section.removeFromTop(toneTitleHeight));
    section = section.reduced(toneSectionInset, 0);
    section.translate(0, -toneSectionLift);

    auto firstArea = section.removeFromLeft(section.getWidth() / 2);
    auto secondArea = section;

    layoutSmallDial(firstSlider, firstLabel, firstArea, scale);
    layoutSmallDial(secondSlider, secondLabel, secondArea, scale);
}

juce::Line<float> DualToneGeneratorAudioProcessorEditor::computeToneDivider(juce::Slider& panSlider,
                                                                            juce::Slider& attenuationSlider,
                                                                            juce::Label& titleLabel,
                                                                            float scale)
{
    const auto grooveOffset = juce::roundToInt(8.0f * scale);
    const auto panBounds = panSlider.getBounds();
    const auto attenuationBounds = attenuationSlider.getBounds();
    const auto left = static_cast<float>(juce::jmin(panBounds.getX(), attenuationBounds.getX()));
    const auto right = static_cast<float>(juce::jmax(panBounds.getRight(), attenuationBounds.getRight()));
    if (right <= left)
        return juce::Line<float>();

    auto y = static_cast<float>(titleLabel.getY() - grooveOffset);
    const auto minY = static_cast<float>(contentPanelBounds.getY());
    const auto maxY = static_cast<float>(contentPanelBounds.getBottom());
    y = juce::jlimit(minY, maxY, y);
    return juce::Line<float>({ left, y }, { right, y });
}
