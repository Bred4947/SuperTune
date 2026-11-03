#pragma once
#include <vector>
#include <cstdint>
#include <cmath>

//==============================================================================
// YIN pitch detector running on a 2x-decimated signal.
class PitchDetector
{
public:
    void prepare (double sampleRate, float minHz, float maxHz);
    // Feed one input sample. Returns true when a new estimate is ready.
    bool push (float x);

    float getFrequency() const  { return freq; }   // 0 = unvoiced
    float getConfidence() const { return conf; }
    // How far behind 'now' (in input samples) the latest estimate is centred
    int getLagSamples() const { return window + tauMax + hop * 2; }

private:
    void analyse();

    double dsr = 24000.0;
    int tauMin = 20, tauMax = 320, window = 400, hop = 128;
    std::vector<float> ring, frame, diff;
    int writePos = 0, sinceLast = 0;
    float decimAcc = 0.0f; bool decimPhase = false;
    float freq = 0.0f, conf = 0.0f;
    float history[3] {};
};

//==============================================================================
// Pitch-synchronous overlap-add (PSOLA) shifter with several output "voices"
// that share one input history. Preserves formants, and lets us scale them.
class PsolaEngine
{
public:
    static constexpr int numVoices = 5;   // main, double L, double R, octave down, octave up

    void prepare (double sampleRate, int maxPeriodSamples);
    void reset();

    int getLatency() const { return latency; }
    int getLookahead() const { return lookahead; }

    // period: current detected period in samples (0 = unvoiced)
    // ratios: pitch ratio per voice; active: whether to render each voice
    // formantScale: >1 lowers formants, <1 raises them
    void process (float x, float period, const float* ratios, const bool* active,
                  float formantScale, float* outs);

private:
    struct Voice
    {
        double synth = 0.0, epoch = 0.0, lastCentre = -1.0e18;
        std::vector<float> out;
        bool wasActive = false;
    };

    void addGrain (Voice& v, float T, float ratio, float fscale);
    float readIn (double pos) const;

    static constexpr int bufSize = 1 << 15;
    static constexpr int mask = bufSize - 1;

    std::vector<float> in;
    Voice voices[numVoices];
    int64_t n = 0;
    int maxPeriod = 600, lookahead = 1500, latency = 2100;
    float defaultPeriod = 256.0f;
};
