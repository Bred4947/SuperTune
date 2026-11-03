#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "PitchEngine.h"
#include <array>
#include <atomic>

class SuperTuneProcessor : public juce::AudioProcessor
{
public:
    SuperTuneProcessor();
    ~SuperTuneProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    using juce::AudioProcessor::processBlock;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "SuperTune"; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    juce::AudioProcessorValueTreeState apvts;

    // ---- Live data for the pitch display (written by audio thread) ----
    static constexpr int historySize = 260;
    std::array<std::atomic<float>, historySize> historyIn {}, historyOut {};
    std::atomic<int> historyPos { 0 };
    std::atomic<float> liveInMidi { 0.0f }, liveTargetMidi { 0.0f }, liveOutMidi { 0.0f };
    std::atomic<bool> liveVoiced { false };

private:
    void updateTarget (float midi);
    float processDrive (float x, float d);

    PitchDetector detector;
    PsolaEngine engine;
    double sr = 44100.0;
    int currentProgram = 0;

    // correction state
    float desiredCorr = 0.0f, corr = 0.0f, kick = 0.0f, currentTarget = -100.0f, inputMidi = 0.0f;
    float period = 0.0f;
    bool voiced = false;
    int unvoicedCount = 0;

    std::vector<float> shiftBuf; int shiftPos = 0, shiftDelay = 0;

    // dry delay (latency compensation for Mix)
    std::vector<float> dryL, dryR; int dryPos = 0;

    // doubler delays, ping-pong delay
    std::vector<float> dblBufL, dblBufR, dlyL, dlyR;
    int dblPos = 0, dlyPos = 0;
    float dlyLp[2] {};
    juce::SmoothedValue<float> dlySamples;

    // lo-fi state
    float hp1[2] {}, hpPrev[2] {}, lp1[2] {}, lp2[2] {}, hold[2] {}; float holdCount = 0.0f;
    float driveLp[2] {};

    juce::dsp::Reverb reverb;

    std::atomic<float>* p[19] {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SuperTuneProcessor)
};
