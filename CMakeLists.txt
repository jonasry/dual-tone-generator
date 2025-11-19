cmake_minimum_required(VERSION 3.15)
project(DualToneGenerator VERSION 0.6.1)

add_subdirectory(extern/JUCE)

set(JUCE_VST3_CAN_REPLACE_VST2 OFF CACHE BOOL "" FORCE)

juce_add_plugin(DualToneGenerator
    PLUGIN_MANUFACTURER_CODE Jona
    PLUGIN_CODE Dtgn
    FORMATS AU Standalone
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    PRODUCT_NAME "Dual Tone Generator"
    COMPANY_NAME "Jona"
)

target_sources(DualToneGenerator PRIVATE
    source/PluginProcessor.cpp
    source/PluginEditor.cpp
    source/SvgDialLookAndFeel.cpp
)

juce_add_binary_data(DualToneGeneratorData
    SOURCES
        assets/cog_knob_large.svg
        assets/cog_knob_blue.svg
        assets/cog_knob_green.svg
        assets/vco_circuit.svg
)

target_compile_features(DualToneGenerator PRIVATE cxx_std_17)

# Ensure AU target links with the shared code
target_link_libraries(DualToneGenerator_AU PRIVATE DualToneGenerator)

target_link_libraries(DualToneGenerator PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    DualToneGeneratorData
)

target_link_libraries(DualToneGenerator_AU PRIVATE DualToneGeneratorData)
