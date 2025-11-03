#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class DualToneGeneratorAudioProcessor : public juce::AudioProcessor
{
public:
    DualToneGeneratorAudioProcessor();
    ~DualToneGeneratorAudioProcessor() override = default;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlock(juce::AudioBuffer<double>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters; }
    bool isStereoOutput() const;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    template <typename SampleType>
    void processBlockInternal(juce::AudioBuffer<SampleType>& buffer);

    juce::AudioProcessorValueTreeState parameters;

    std::atomic<float>* centerFrequencyParam = nullptr;
    std::atomic<float>* spreadParam = nullptr;
    std::atomic<float>* panOneParam = nullptr;
    std::atomic<float>* panTwoParam = nullptr;
    std::atomic<float>* gainParam = nullptr;

    double currentSampleRate = 44100.0;
    double phaseOne = 0.0;
    double phaseTwo = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DualToneGeneratorAudioProcessor)
};
