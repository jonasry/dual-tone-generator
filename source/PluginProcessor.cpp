#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>
#include <utility>

namespace
{
inline std::pair<float, float> calculatePanGains(float pan)
{
    const auto clipped = juce::jlimit(-1.0f, 1.0f, pan);
    const auto angle = (clipped + 1.0f) * 0.5f * juce::MathConstants<float>::halfPi;
    const auto left = std::cos(angle);
    const auto right = std::sin(angle);
    return { left, right };
}
} // namespace

DualToneGeneratorAudioProcessor::DualToneGeneratorAudioProcessor()
    : juce::AudioProcessor(
          BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    frequencyOneParam = parameters.getRawParameterValue("freq1");
    frequencyTwoParam = parameters.getRawParameterValue("freq2");
    panOneParam = parameters.getRawParameterValue("pan1");
    panTwoParam = parameters.getRawParameterValue("pan2");
    gainParam = parameters.getRawParameterValue("gain");
}

juce::AudioProcessorValueTreeState::ParameterLayout DualToneGeneratorAudioProcessor::createParameterLayout()
{
    using juce::AudioParameterFloat;
    using juce::NormalisableRange;

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto frequencyRange = NormalisableRange<float>(30.0f, 400.0f, 0.001f, 0.3f);
    layout.add(std::make_unique<AudioParameterFloat>("freq1", "Frequency 1", frequencyRange, 98.0f));
    layout.add(std::make_unique<AudioParameterFloat>("freq2", "Frequency 2", frequencyRange, 102.0f));
    layout.add(std::make_unique<AudioParameterFloat>("pan1", "Pan 1", -1.0f, 1.0f, -1.0f));
    layout.add(std::make_unique<AudioParameterFloat>("pan2", "Pan 2", -1.0f, 1.0f, 1.0f));
    layout.add(std::make_unique<AudioParameterFloat>("gain", "Gain", 0.0f, 1.0f, 1.0f));

    return layout;
}

const juce::String DualToneGeneratorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DualToneGeneratorAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* DualToneGeneratorAudioProcessor::createEditor()
{
    return new DualToneGeneratorAudioProcessorEditor(*this);
}

bool DualToneGeneratorAudioProcessor::isStereoOutput() const
{
    return getTotalNumOutputChannels() >= 2;
}

void DualToneGeneratorAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate;
}

void DualToneGeneratorAudioProcessor::releaseResources()
{
}

bool DualToneGeneratorAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Only allow mono or stereo outputs, no inputs needed for a synth
    if (layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo())
        return true;

    return false;
}

template <typename SampleType>
void DualToneGeneratorAudioProcessor::processBlockInternal(juce::AudioBuffer<SampleType>& buffer)
{
    juce::ScopedNoDenormals disableDenormals;
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    buffer.clear();

    if (numSamples == 0)
        return;

    const auto freq1 = static_cast<double>(frequencyOneParam->load());
    const auto freq2 = static_cast<double>(frequencyTwoParam->load());
    const auto gain = static_cast<float>(gainParam->load());
    const bool stereo = (numChannels >= 2);

    const auto increment1 = (juce::MathConstants<double>::twoPi * freq1) / currentSampleRate;
    const auto increment2 = (juce::MathConstants<double>::twoPi * freq2) / currentSampleRate;

    const float halfGain = gain * 0.5f;

    float leftGain1 = 1.0f, rightGain1 = stereo ? 0.0f : 1.0f;
    float leftGain2 = 1.0f, rightGain2 = stereo ? 0.0f : 1.0f;

    if (stereo)
    {
        const auto [l1, r1] = calculatePanGains(panOneParam->load());
        const auto [l2, r2] = calculatePanGains(panTwoParam->load());
        leftGain1 = l1;
        rightGain1 = r1;
        leftGain2 = l2;
        rightGain2 = r2;
    }

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto wave1 = static_cast<float>(std::sin(phaseOne));
        const auto wave2 = static_cast<float>(std::sin(phaseTwo));

        phaseOne += increment1;
        phaseTwo += increment2;

        if (phaseOne >= juce::MathConstants<double>::twoPi)
            phaseOne -= juce::MathConstants<double>::twoPi;
        if (phaseTwo >= juce::MathConstants<double>::twoPi)
            phaseTwo -= juce::MathConstants<double>::twoPi;

        if (stereo)
        {
            const auto leftValue = ((wave1 * leftGain1) + (wave2 * leftGain2)) * halfGain;
            const auto rightValue = ((wave1 * rightGain1) + (wave2 * rightGain2)) * halfGain;

            buffer.setSample(0, sample, static_cast<SampleType>(leftValue));
            buffer.setSample(1, sample, static_cast<SampleType>(rightValue));
        }
        else
        {
            const auto monoValue = (wave1 + wave2) * halfGain;
            buffer.setSample(0, sample, static_cast<SampleType>(monoValue));
        }
    }
}

void DualToneGeneratorAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    midiMessages.clear();
    processBlockInternal(buffer);
}

void DualToneGeneratorAudioProcessor::processBlock(juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages)
{
    midiMessages.clear();
    processBlockInternal(buffer);
}

void DualToneGeneratorAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void DualToneGeneratorAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        if (xml->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DualToneGeneratorAudioProcessor();
}
