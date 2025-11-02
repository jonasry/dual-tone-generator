#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

class DualToneGeneratorAudioProcessor : public juce::AudioProcessor {
public:
    DualToneGeneratorAudioProcessor();
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    const juce::String getName() const override { return "DualToneGenerator"; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    double getTailLengthSeconds() const override { return 0.0; }
    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

private:
    double sampleRate = 44100.0;
    double phase1 = 0.0, phase2 = 0.0;
    juce::AudioParameterFloat* freq1Param = nullptr;
    juce::AudioParameterFloat* freq2Param = nullptr;
    juce::AudioParameterFloat* gainParam = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DualToneGeneratorAudioProcessor)
};
