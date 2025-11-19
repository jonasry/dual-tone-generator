#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

TEST_CASE("DualToneGeneratorAudioProcessor Frequency Test", "[processor]")
{
    // Initialize JUCE MessageManager for APVTS timers
    juce::ScopedJuceInitialiser_GUI juceInitialiser;

    DualToneGeneratorAudioProcessor processor;

    // Prepare the processor
    double sampleRate = 44100.0;
    int samplesPerBlock = 44100; // 1 second for better frequency resolution
    processor.prepareToPlay(sampleRate, samplesPerBlock);

    // Set parameters
    auto& params = processor.getValueTreeState();
    
    // Center = 440 Hz, Spread = 10 Hz
    // Tone 1 = 430 Hz, Tone 2 = 450 Hz
    *params.getRawParameterValue("centerFreq") = 440.0f;
    *params.getRawParameterValue("spread") = 10.0f;
    
    // Hard pan: Tone 1 Left (-1.0), Tone 2 Right (1.0)
    *params.getRawParameterValue("pan1") = -1.0f;
    *params.getRawParameterValue("pan2") = 1.0f;
    
    // Ensure full gain and no attenuation
    *params.getRawParameterValue("atten1") = 0.0f;
    *params.getRawParameterValue("atten2") = 0.0f;
    *params.getRawParameterValue("gain") = 1.0f;

    // Create buffers
    juce::AudioBuffer<float> buffer(2, samplesPerBlock);
    juce::MidiBuffer midiBuffer;

    // Process a few blocks to let things settle (though there's no smoothing on these params currently, good practice)
    processor.processBlock(buffer, midiBuffer);
    processor.processBlock(buffer, midiBuffer);

    // Analyze the output
    // We expect Left channel to be 430 Hz sine wave
    // We expect Right channel to be 450 Hz sine wave
    
    auto* leftChannel = buffer.getReadPointer(0);
    auto* rightChannel = buffer.getReadPointer(1);

    auto calculateFrequency = [&](const float* data, int numSamples) -> double {
        // Autocorrelation lag 1 method
        // R(1) = sum(x[i] * x[i+1])
        // R(0) = sum(x[i] * x[i])
        // cos(w) = R(1) / R(0)
        
        double r0 = 0.0;
        double r1 = 0.0;
        
        // Use the same range for both sums to be consistent
        // Sum from 0 to N-2
        for (int i = 0; i < numSamples - 1; ++i)
        {
            r0 += data[i] * data[i];
            r1 += data[i] * data[i+1];
        }

        if (r0 < 1e-9) return 0.0; // Silence

        double cosW = r1 / r0;
        // Clamp to valid range for acos
        cosW = std::max(-1.0, std::min(1.0, cosW));
        
        double w = std::acos(cosW);
        return w * sampleRate / (2 * juce::MathConstants<double>::pi);
    };

    double measuredFreqLeft = calculateFrequency(leftChannel, samplesPerBlock);
    double measuredFreqRight = calculateFrequency(rightChannel, samplesPerBlock);

    // Tolerances might be needed due to block size and precision
    REQUIRE(measuredFreqLeft == Catch::Approx(430.0).margin(1.0));
    REQUIRE(measuredFreqRight == Catch::Approx(450.0).margin(1.0));
}
