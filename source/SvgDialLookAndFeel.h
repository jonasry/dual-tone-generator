#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class SvgDialLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SvgDialLookAndFeel(const void* svgData,
                       size_t svgSize,
                       juce::Colour trackColour,
                       juce::Colour outlineColour);

    void drawRotarySlider(juce::Graphics& g,
                          int x,
                          int y,
                          int width,
                          int height,
                          float sliderPos,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
                          juce::Slider& slider) override;

private:
    std::unique_ptr<juce::Drawable> knobDrawable;
};
