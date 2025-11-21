#include "SvgDialLookAndFeel.h"

SvgDialLookAndFeel::SvgDialLookAndFeel(const void* svgData,
                                       size_t svgSize,
                                       juce::Colour trackColour,
                                       juce::Colour outlineColour)
{
    setColour(juce::Slider::rotarySliderOutlineColourId, outlineColour);
    setColour(juce::Slider::trackColourId, trackColour);
    setColour(juce::Slider::thumbColourId, trackColour.brighter(0.15f));

    knobDrawable = juce::Drawable::createFromImageData(svgData, svgSize);
}

void SvgDialLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                          int x,
                                          int y,
                                          int width,
                                          int height,
                                          float sliderPos,
                                          float rotaryStartAngle,
                                          float rotaryEndAngle,
                                          juce::Slider& slider)
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

    const auto uiScale = static_cast<float>(slider.getProperties().getWithDefault("uiScale", 1.0f));
    const auto baseShadowRadius = 16.0f * uiScale;
    const auto baseShadowOffsetY = 9.0f * uiScale;

    const auto shadowRadius = juce::jmax(juce::roundToInt(baseShadowRadius),
                                         juce::roundToInt(knobRadius * 0.18f));
    const auto shadowOffsetY = juce::jmax(juce::roundToInt(baseShadowOffsetY),
                                          juce::roundToInt(knobRadius * 0.10f));

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
