# Project Description
This project builds a dual-tone generator plugin for AU-compatible hosts. It is written in C++ using the JUCE framework.

# Overall Guidelines

## Context7
Make use of the Context7 MPC server to find technical documentatation when needed.

# Repository Guidelines

## Project Structure & Module Organization
The audio processor lives under `source/`, with shared DSP logic in `PluginProcessor.*` and UI code in `PluginEditor.*`. Assets are collected in `assets/`. External dependencies reside in `extern/`; run `git submodule update --init --recursive` to pull JUCE (and FUSE when present). 

CMake creates build products and intermediates under `build/`, including the final AU bundle inside `build/DualToneGenerator_artefacts/Release/AU/` and a standalone app in `build/DualToneGenerator_artefacts/Release/Standalone/`.

## Build, Test, and Development Commands
Configure once with `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug` (swap `Release` when packaging). Rebuild after code changes via `cmake --build build --target DualToneGenerator_AU --config Debug`, which also refreshes the standalone target. Use `cmake --build build --target DualToneGenerator_Standalone --config Debug` if you only need the desktop app for quick auditioning. Clean by deleting the `build/` directory; no custom clean target is defined.

## Coding Style & Naming Conventions
Follow modern C++17 conventions already in the tree: four-space indentation, brace-on-same-line for functions, and JUCE-friendly PascalCase for classes (`DualToneGeneratorAudioProcessor`) with camelCase methods (`prepareToPlay`). Prefer `auto` only when the type is obvious, and rely on `juce::ScopedValueSetter`, `juce::AudioBuffer`, and other JUCE utilities instead of reinventing helpers. Keep processor logic deterministic—avoid hidden static state—and document non-trivial processing blocks with concise comments.

## Testing Guidelines
No automated unit harness currently exists; validate changes by building the standalone app or loading the AU in Logic Pro/MainStage. Exercise both mono and stereo buses to confirm pan behaviour, verify default frequencies (98 Hz/102 Hz), and sweep the gain for artifacts. Capture short screen recordings or audio exports when describing regressions in pull requests to speed up reviews.

## Commit & Pull Request Guidelines
Match the history’s concise, imperative commits (e.g., `Add standalone app`, `Make it stereo`) and keep them scoped to a single concern. Each PR should include: a summary of the sonic or UI impact, reproduction steps, and references to related issues or tickets. If the change affects audio behaviour, attach test audio or measurement notes and mention which host/version you used for verification.
