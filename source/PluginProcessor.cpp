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
    centerFrequencyParam = parameters.getRawParameterValue("centerFreq");
    spreadParam = parameters.getRawParameterValue("spread");
    panOneParam = parameters.getRawParameterValue("pan1");
    panTwoParam = parameters.getRawParameterValue("pan2");
    attenuationOneParam = parameters.getRawParameterValue("atten1");
    attenuationTwoParam = parameters.getRawParameterValue("atten2");
    gainParam = parameters.getRawParameterValue("gain");
    driveParam = parameters.getRawParameterValue("drive");
    shapeTypeParam = parameters.getRawParameterValue("shapeType");
}

juce::AudioProcessorValueTreeState::ParameterLayout DualToneGeneratorAudioProcessor::createParameterLayout()
{
    using juce::AudioParameterFloat;
    using juce::NormalisableRange;

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto centerRange = NormalisableRange<float>(60.0f, 600.0f, 0.001f, 0.3f);
    auto spreadRange = NormalisableRange<float>(0.0f, 20.0f, 0.001f, 0.6f);
    layout.add(std::make_unique<AudioParameterFloat>("centerFreq", "Center (Hz)", centerRange, 100.0f));
    layout.add(std::make_unique<AudioParameterFloat>("spread", "Spread (Hz)", spreadRange, 2.0f));
    layout.add(std::make_unique<AudioParameterFloat>("pan1", "Pan 1", -1.0f, 1.0f, -1.0f));
    layout.add(std::make_unique<AudioParameterFloat>("pan2", "Pan 2", -1.0f, 1.0f, 1.0f));
    auto attenuationRange = NormalisableRange<float>(-24.0f, 0.0f, 0.01f);
    auto gainRange = NormalisableRange<float>(-12.0f, 12.0f, 0.01f);
    const auto defaultGainDb = 0.0f;
    layout.add(std::make_unique<AudioParameterFloat>("atten1", "Attenuation 1", attenuationRange, 0.0f, "dB"));
    layout.add(std::make_unique<AudioParameterFloat>("atten2", "Attenuation 2", attenuationRange, 0.0f, "dB"));
    layout.add(std::make_unique<AudioParameterFloat>("gain", "Gain", gainRange, defaultGainDb, "dB"));
    auto driveRange = NormalisableRange<float>(-24.0f, 12.0f, 0.01f);
    layout.add(std::make_unique<AudioParameterFloat>("drive", "Drive", driveRange, -24.0f, "dB"));
    auto shapeTypeRange = NormalisableRange<float>(0.0f, 1.0f, 0.001f);
    auto shapeTypeAttributes = juce::AudioParameterFloatAttributes()
                                   .withStringFromValueFunction([](float value, int)
                                                                {
                                                                    return juce::String(juce::roundToInt(value * 100.0f)) + "%";
                                                                })
                                   .withValueFromStringFunction([](const juce::String& text)
                                                                {
                                                                    return text.trimCharactersAtEnd("%").getFloatValue() * 0.01f;
                                                                });
    layout.add(std::make_unique<AudioParameterFloat>("shapeType",
                                                     "Shape Type",
                                                     shapeTypeRange,
                                                     0.0f,
                                                     shapeTypeAttributes));

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

    const auto centerFrequency = static_cast<double>(centerFrequencyParam->load());
    const auto spread = static_cast<double>(spreadParam->load());
    const auto freq1 = juce::jmax(0.0, centerFrequency - spread);
    const auto freq2 = juce::jmax(0.0, centerFrequency + spread);
    const auto attenuationOne = juce::Decibels::decibelsToGain(attenuationOneParam != nullptr ? attenuationOneParam->load()
                                                                                              : 0.0f);
    const auto attenuationTwo = juce::Decibels::decibelsToGain(attenuationTwoParam != nullptr ? attenuationTwoParam->load()
                                                                                              : 0.0f);
    const auto gainDb = gainParam != nullptr ? gainParam->load() : 0.0f;
    const auto gain = juce::Decibels::decibelsToGain(gainDb);
    const auto driveDb = static_cast<double>(driveParam != nullptr ? driveParam->load() : -24.0f);
    const auto driveAmount = std::pow(10.0, driveDb / 20.0);
    const auto tanhDenominator = std::tanh(driveAmount);
    const auto tanhScale = tanhDenominator != 0.0 ? (1.0 / tanhDenominator) : 1.0;
    const auto atanDenominator = std::atan(driveAmount);
    const auto atanScale = atanDenominator != 0.0 ? (1.0 / atanDenominator) : 1.0;
    const auto typeMix = juce::jlimit(0.0f, 1.0f, shapeTypeParam != nullptr ? shapeTypeParam->load() : 0.0f);
    const bool stereo = (numChannels >= 2);

    const auto increment1 = (juce::MathConstants<double>::twoPi * freq1) / currentSampleRate;
    const auto increment2 = (juce::MathConstants<double>::twoPi * freq2) / currentSampleRate;

    const auto baseGain = juce::Decibels::decibelsToGain(-12.0f);
    const auto toneGain = baseGain * gain;

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

        const auto tanhWave1 = std::tanh(static_cast<double>(wave1) * driveAmount) * tanhScale;
        const auto tanhWave2 = std::tanh(static_cast<double>(wave2) * driveAmount) * tanhScale;
        const auto atanWave1 = std::atan(static_cast<double>(wave1) * driveAmount) * atanScale;
        const auto atanWave2 = std::atan(static_cast<double>(wave2) * driveAmount) * atanScale;
        const auto shapedWave1 = static_cast<SampleType>((1.0 - static_cast<double>(typeMix)) * tanhWave1
                                                         + static_cast<double>(typeMix) * atanWave1);
        const auto shapedWave2 = static_cast<SampleType>((1.0 - static_cast<double>(typeMix)) * tanhWave2
                                                         + static_cast<double>(typeMix) * atanWave2);

        auto tone1 = static_cast<SampleType>(shapedWave1 * toneGain);
        auto tone2 = static_cast<SampleType>(shapedWave2 * toneGain);

        tone1 = static_cast<SampleType>(tone1 * attenuationOne);
        tone2 = static_cast<SampleType>(tone2 * attenuationTwo);

        if (stereo)
        {
            const auto leftValue = (tone1 * static_cast<SampleType>(leftGain1))
                                   + (tone2 * static_cast<SampleType>(leftGain2));
            const auto rightValue = (tone1 * static_cast<SampleType>(rightGain1))
                                    + (tone2 * static_cast<SampleType>(rightGain2));

            buffer.setSample(0, sample, static_cast<SampleType>(leftValue));
            buffer.setSample(1, sample, static_cast<SampleType>(rightValue));
        }
        else
        {
            const auto monoValue = (tone1 + tone2) * static_cast<SampleType>(0.5);
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
