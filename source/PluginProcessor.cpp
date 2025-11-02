#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <juce_core/juce_core.h>

DualToneGeneratorAudioProcessor::DualToneGeneratorAudioProcessor() {
    addParameter(freq1Param = new juce::AudioParameterFloat("freq1", "Frequency 1", 20.0f, 20000.0f, 440.0f));
    addParameter(freq2Param = new juce::AudioParameterFloat("freq2", "Frequency 2", 20.0f, 20000.0f, 660.0f));
    addParameter(gainParam  = new juce::AudioParameterFloat("gain",  "Gain",        0.0f, 1.0f,      0.2f));
}

void DualToneGeneratorAudioProcessor::prepareToPlay(double newSampleRate, int) {
    sampleRate = newSampleRate;
}

void DualToneGeneratorAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    auto* left  = buffer.getWritePointer(0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
    int numSamples = buffer.getNumSamples();

    double f1 = *freq1Param;
    double f2 = *freq2Param;
    double g  = *gainParam;

    for (int i = 0; i < numSamples; ++i) {
        float sample = std::sin(phase1) * 0.5f + std::sin(phase2) * 0.5f;
        sample *= g;

        left[i] = sample;
        if (right) right[i] = sample;

        phase1 += 2.0 * juce::MathConstants<double>::pi * f1 / sampleRate;
        phase2 += 2.0 * juce::MathConstants<double>::pi * f2 / sampleRate;
        if (phase1 >= juce::MathConstants<double>::twoPi) phase1 -= juce::MathConstants<double>::twoPi;
        if (phase2 >= juce::MathConstants<double>::twoPi) phase2 -= juce::MathConstants<double>::twoPi;
    }
}

juce::AudioProcessorEditor* DualToneGeneratorAudioProcessor::createEditor() {
    return new DualToneGeneratorAudioProcessorEditor(*this);
}

void DualToneGeneratorAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    juce::MemoryOutputStream stream(destData, false);
    stream.writeFloat(freq1Param->get());
    stream.writeFloat(freq2Param->get());
    stream.writeFloat(gainParam->get());
}

void DualToneGeneratorAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
    if (stream.getTotalLength() < static_cast<size_t>(3 * sizeof(float)))
        return;

    const auto applyValue = [](juce::AudioParameterFloat* param, float value) {
        const auto range = param->getNormalisableRange();
        const auto clamped = juce::jlimit(range.start, range.end, value);
        param->setValueNotifyingHost(range.convertTo0to1(clamped));
    };

    applyValue(freq1Param, stream.readFloat());
    applyValue(freq2Param, stream.readFloat());
    applyValue(gainParam,  stream.readFloat());
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new DualToneGeneratorAudioProcessor();
}
