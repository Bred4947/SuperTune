#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    enum P { key, scale, speed, amount, hyper, transpose, formant, dbl, width, octDown, octUp,
             drive, lofi, delay, delayTime, reverbAmt, mix, output, numP };

    const char* ids[numP] = { "key", "scale", "speed", "amount", "hyper", "transpose", "formant",
                              "double", "width", "octDown", "octUp", "drive", "lofi", "delay",
                              "delayTime", "reverb", "mix", "output" };

    // Scales as semitone masks starting from the key note
    const bool scales[][12] = {
        { 1,1,1,1,1,1,1,1,1,1,1,1 },   // Chromatic
        { 1,0,1,0,1,1,0,1,0,1,0,1 },   // Major
        { 1,0,1,1,0,1,0,1,1,0,1,0 },   // Minor
        { 1,0,1,1,0,1,0,1,1,0,0,1 },   // Harmonic Minor
        { 1,0,1,0,1,0,0,1,0,1,0,0 },   // Pentatonic Major
        { 1,0,0,1,0,1,0,1,0,0,1,0 },   // Pentatonic Minor
        { 1,0,0,1,0,1,1,1,0,0,1,0 },   // Blues
        { 1,0,1,1,0,1,0,1,0,1,1,0 },   // Dorian
        { 1,1,0,1,0,1,0,1,1,0,1,0 },   // Phrygian
    };

    // Presets set the *character* only. Key & scale stay whatever your song is in.
    // speed(ms) amount(%) hyper transpose formant | double width octDown octUp | drive lofi delay time reverb | mix out(dB)
    struct Preset { const char* name; float v[16]; };
    const Preset presets[] = {
        { "HARD TUNE",          { 0, 100, 0, 0, 0.0f, 0.25f, 12, 0.00f, 0.00f, 0.00f, 0.00f, 0.10f, 250, 0.15f, 1, 0.2f } },
        { "RAGE LEAD",          { 0, 100, 0, 0, 0.0f, 0.45f, 15, 0.00f, 0.00f, 0.35f, 0.00f, 0.15f, 300, 0.20f, 1, -4.0f } },
        { "VAMP BABY VOICE",    { 0, 100, 0, 0, 4.5f, 0.30f, 10, 0.00f, 0.00f, 0.10f, 0.00f, 0.15f, 260, 0.20f, 1, 3.6f } },
        { "LONELY FOG",         { 35, 90, 0, 0, -2.5f, 0.40f, 20, 0.20f, 0.00f, 0.10f, 0.10f, 0.35f, 520, 0.60f, 1, 1.3f } },
        { "DARK STACK",         { 15, 100, 0, 0, -1.5f, 0.35f, 18, 0.35f, 0.00f, 0.15f, 0.00f, 0.25f, 420, 0.45f, 1, 0.9f } },
        { "BLOWN OUT HYPER",    { 0, 140, 1, 0, 1.0f, 0.30f, 25, 0.00f, 0.15f, 0.85f, 0.30f, 0.10f, 150, 0.15f, 1, -0.6f } },
        { "GLITCH SIREN",       { 0, 150, 1, 0, 2.0f, 0.40f, 30, 0.00f, 0.30f, 0.70f, 0.40f, 0.20f, 180, 0.20f, 1, -2.1f } },
        { "UNDERGROUND GRIT",   { 0, 120, 1, 0, 0.5f, 0.35f, 20, 0.15f, 0.00f, 0.60f, 0.55f, 0.15f, 200, 0.15f, 1, -4.2f } },
        { "PLUGG SWEET",        { 10, 100, 0, 0, 1.0f, 0.30f, 12, 0.00f, 0.12f, 0.00f, 0.00f, 0.30f, 300, 0.30f, 1, -0.3f } },
        { "PLUGGNB SILK",       { 25, 100, 0, 0, 0.0f, 0.35f, 10, 0.00f, 0.10f, 0.05f, 0.00f, 0.25f, 375, 0.35f, 1, 0.6f } },
        { "TEEN STACK",         { 0, 100, 0, 0, -0.5f, 0.60f, 22, 0.20f, 0.00f, 0.40f, 0.00f, 0.10f, 250, 0.20f, 1, -3.3f } },
        { "TONKA TONE",         { 0, 110, 0, 0, -3.0f, 0.30f, 15, 0.00f, 0.25f, 0.30f, 0.00f, 0.10f, 280, 0.25f, 1, -2.1f } },
        { "ETHEREAL AIR",       { 20, 100, 0, 0, 3.0f, 0.45f, 14, 0.00f, 0.20f, 0.00f, 0.00f, 0.30f, 350, 0.55f, 1, 2.8f } },
        { "JERK BOUNCE",        { 5, 110, 0, 0, 1.5f, 0.35f, 16, 0.00f, 0.20f, 0.25f, 0.10f, 0.20f, 180, 0.15f, 1, -3.5f } },
        { "SIGIL HYPERPOP",     { 0, 150, 1, 0, 4.0f, 0.50f, 25, 0.00f, 0.30f, 0.50f, 0.30f, 0.20f, 160, 0.30f, 1, -2.3f } },
        { "DRILL NIGHT",        { 10, 100, 0, 0, -1.0f, 0.25f, 12, 0.25f, 0.00f, 0.35f, 0.00f, 0.15f, 330, 0.20f, 1, -3.0f } },
        { "CLOUD RAP DREAM",    { 40, 90, 0, 0, 0.5f, 0.40f, 16, 0.00f, 0.15f, 0.00f, 0.00f, 0.45f, 600, 0.70f, 1, -1.0f } },
        { "DEMON DOWN",         { 0, 100, 0, -12, -4.0f, 0.20f, 10, 0.00f, 0.00f, 0.30f, 0.00f, 0.10f, 300, 0.25f, 1, -1.2f } },
        { "CHIPMUNK UP",        { 0, 100, 0, 12, 5.0f, 0.20f, 10, 0.00f, 0.00f, 0.00f, 0.00f, 0.10f, 300, 0.20f, 1, 5.0f } },
        { "PHONE CALL",         { 15, 100, 0, 0, 0.0f, 0.00f, 10, 0.00f, 0.00f, 0.20f, 0.90f, 0.10f, 250, 0.10f, 1, -5.5f } },
        { "ROBOT CLASSIC",      { 0, 100, 0, 0, 0.0f, 0.00f, 10, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 300, 0.10f, 1, 0.3f } },
        { "STUDIO POLISH",      { 50, 100, 0, 0, 0.0f, 0.20f, 8, 0.00f, 0.00f, 0.00f, 0.00f, 0.10f, 300, 0.20f, 1, -0.2f } },
        { "NATURAL TOUCH",      { 120, 70, 0, 0, 0.0f, 0.10f, 8, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 300, 0.10f, 1, 0.1f } },
        { "INIT",               { 20, 100, 0, 0, 0.0f, 0.00f, 10, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 300, 0.00f, 1, 0 } },
    };
    constexpr int numPresets = (int) (sizeof (presets) / sizeof (presets[0]));
}

juce::AudioProcessorValueTreeState::ParameterLayout SuperTuneProcessor::createLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> ps;

    auto fmt = [] (String suffix, int decimals)
    {
        return AudioParameterFloatAttributes().withStringFromValueFunction ([=] (float v, int)
        {
            return String (v, decimals) + (suffix.isEmpty() ? "" : " " + suffix);
        });
    };
    auto pct = AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int)
    {
        return String (roundToInt (v * 100.0f)) + "%";
    });

    ps.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "key", 1 }, "Key",
                    StringArray { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 0));
    ps.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "scale", 1 }, "Scale",
                    StringArray { "Chromatic", "Major", "Minor", "Harmonic Minor", "Pentatonic Maj",
                                  "Pentatonic Min", "Blues", "Dorian", "Phrygian" }, 2));

    NormalisableRange<float> speedR (0.0f, 400.0f); speedR.setSkewForCentre (40.0f);
    ps.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "speed", 1 }, "Retune Speed", speedR, 20.0f,
                    AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int)
                    { return v < 0.5f ? String ("INSTANT") : String (roundToInt (v)) + " ms"; })));
    ps.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "amount", 1 }, "Amount",
                    NormalisableRange<float> (0.0f, 150.0f), 100.0f, fmt ("%", 0)));
    ps.push_back (std::make_unique<AudioParameterBool> (ParameterID { "hyper", 1 }, "HYPER", false));
    ps.push_back (std::make_unique<AudioParameterInt> (ParameterID { "transpose", 1 }, "Transpose", -12, 12, 0));
    ps.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "formant", 1 }, "Formant",
                    NormalisableRange<float> (-7.0f, 7.0f), 0.0f, fmt ("st", 1)));

    ps.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "double", 1 }, "Double", NormalisableRange<float> (0, 1), 0.0f, pct));
    ps.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "width", 1 }, "Width",
                    NormalisableRange<float> (0.0f, 40.0f), 12.0f, fmt ("ct", 0)));
    ps.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "octDown", 1 }, "Oct Down", NormalisableRange<float> (0, 1), 0.0f, pct));
    ps.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "octUp", 1 }, "Oct Up", NormalisableRange<float> (0, 1), 0.0f, pct));

    ps.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "drive", 1 }, "Drive", NormalisableRange<float> (0, 1), 0.0f, pct));
    ps.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "lofi", 1 }, "Lo-Fi", NormalisableRange<float> (0, 1), 0.0f, pct));
    ps.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "delay", 1 }, "Delay", NormalisableRange<float> (0, 1), 0.0f, pct));
    NormalisableRange<float> dtR (40.0f, 1000.0f); dtR.setSkewForCentre (250.0f);
    ps.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "delayTime", 1 }, "Delay Time", dtR, 300.0f, fmt ("ms", 0)));
    ps.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "reverb", 1 }, "Reverb", NormalisableRange<float> (0, 1), 0.0f, pct));
    ps.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "mix", 1 }, "Mix", NormalisableRange<float> (0, 1), 1.0f, pct));
    ps.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "output", 1 }, "Output",
                    NormalisableRange<float> (-24.0f, 12.0f), 0.0f, fmt ("dB", 1)));

    return { ps.begin(), ps.end() };
}

SuperTuneProcessor::SuperTuneProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "SUPERTUNE", createLayout())
{
    for (int i = 0; i < numP; ++i)
        p[i] = apvts.getRawParameterValue (ids[i]);
    setCurrentProgram (0);
}

bool SuperTuneProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    const auto in  = layouts.getMainInputChannelSet();
    return (out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono())
        && (in == out || in == juce::AudioChannelSet::mono());
}

void SuperTuneProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    constexpr float minHz = 75.0f, maxHz = 1000.0f;
    detector.prepare (sampleRate, minHz, maxHz);
    engine.prepare (sampleRate, (int) (sampleRate / minHz) + 2);
    setLatencySamples (engine.getLatency());

    // Line the correction curve up in time with the audio the grains are reading
    shiftDelay = juce::jmax (0, engine.getLookahead() - detector.getLagSamples());
    shiftBuf.assign ((size_t) shiftDelay + 1, 0.0f);
    shiftPos = 0;

    dryL.assign ((size_t) engine.getLatency() + 1, 0.0f);
    dryR.assign ((size_t) engine.getLatency() + 1, 0.0f);
    dryPos = 0;

    dblBufL.assign ((size_t) (sampleRate * 0.05) + 2, 0.0f);
    dblBufR.assign (dblBufL.size(), 0.0f);
    dblPos = 0;
    dlyL.assign ((size_t) (sampleRate * 1.1) + 2, 0.0f);
    dlyR.assign (dlyL.size(), 0.0f);
    dlyPos = 0;
    dlySamples.reset (sampleRate, 0.2);
    dlySamples.setCurrentAndTargetValue (p[delayTime]->load() * 0.001f * (float) sampleRate);

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 2 };
    reverb.prepare (spec);
    reverb.reset();

    desiredCorr = corr = kick = 0.0f; currentTarget = -100.0f; voiced = false; period = 0.0f;
}

void SuperTuneProcessor::updateTarget (float midi)
{
    const int k = (int) p[key]->load();
    const int sc = juce::jlimit (0, 8, (int) p[scale]->load());
    const bool isHyper = p[hyper]->load() > 0.5f;

    auto allowed = [&] (int note) { return scales[sc][((note - k) % 12 + 12) % 12]; };

    // Hysteresis: stick with the current note unless we've clearly moved away
    const float hyst = isHyper ? 0.0f : 0.22f;
    const int cur = (int) std::lround (currentTarget);
    if (currentTarget > -50.0f && allowed (cur) && std::abs (midi - currentTarget) < 0.5f + hyst)
        return;

    const int base = (int) std::lround (midi);
    float best = (float) base; float bestDist = 1e9f;
    for (int d = -6; d <= 6; ++d)
        if (allowed (base + d))
        {
            const float dist = std::abs ((float) (base + d) - midi);
            if (dist < bestDist) { bestDist = dist; best = (float) (base + d); }
        }
    currentTarget = best;
}

float SuperTuneProcessor::processDrive (float x, float d)
{
    const float g = 1.0f + d * d * 30.0f;
    const float y = std::tanh (x * g + 0.15f * d) - std::tanh (0.15f * d);
    const float makeup = 0.32f / (0.32f + d * d * 1.6f + d * 0.25f);
    return x + juce::jmin (1.0f, d * 4.0f) * (y * makeup - x);
}

void SuperTuneProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    const int numCh = buffer.getNumChannels();
    auto* L = buffer.getWritePointer (0);
    auto* R = numCh > 1 ? buffer.getWritePointer (1) : nullptr;

    // ---- read params ----
    const bool  isHyper   = p[hyper]->load() > 0.5f;
    const float speedMs   = isHyper ? 0.0f : p[speed]->load();
    const float amt       = p[amount]->load() * 0.01f * (isHyper ? 1.4f : 1.0f);
    const float trans     = p[transpose]->load();
    const float fscale    = std::pow (2.0f, -p[formant]->load() / 12.0f);
    const float dblAmt    = p[dbl]->load();
    const float widthCt   = p[width]->load();
    const float octDn     = p[octDown]->load();
    const float octU      = p[octUp]->load();
    const float driveAmt  = p[drive]->load();
    const float lofiAmt   = p[lofi]->load();
    const float dlyAmt    = p[delay]->load();
    const float revAmt    = p[reverbAmt]->load();
    const float mixAmt    = p[mix]->load();
    const float outGain   = juce::Decibels::decibelsToGain (p[output]->load());

    // Keep loudness steady as layers are added
    const float stackNorm = 1.0f / std::sqrt (1.0f + 1.4f * dblAmt * dblAmt + 0.8f * octDn * octDn + 0.36f * octU * octU);
    const float kickDecay = std::exp (-1.0f / ((isHyper ? 0.045f : 0.03f) * (float) sr));
    const float alpha = speedMs < 0.5f ? 1.0f : 1.0f - std::exp (-1.0f / (speedMs * 0.001f * (float) sr));
    const bool active[PsolaEngine::numVoices] = { true, dblAmt > 0.001f, dblAmt > 0.001f, octDn > 0.001f, octU > 0.001f };
    dlySamples.setTargetValue (p[delayTime]->load() * 0.001f * (float) sr);

    const int dblDelayL = (int) (sr * 0.011), dblDelayR = (int) (sr * 0.017);
    const int dryLen = (int) dryL.size();

    // lo-fi filter coefficients (telephone band)
    const float hpC = std::exp (-2.0f * juce::MathConstants<float>::pi * 380.0f / (float) sr);
    const float lpC = 1.0f - std::exp (-2.0f * juce::MathConstants<float>::pi * 3200.0f / (float) sr);
    const float crushLevels = std::pow (2.0f, 16.0f - lofiAmt * lofiAmt * 10.0f) * 0.5f;
    const float holdLen = 1.0f + lofiAmt * lofiAmt * 3.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        const float inL = L[i];
        const float inR = R ? R[i] : inL;
        const float mono = 0.5f * (inL + inR);

        // ---- pitch detection ----
        if (detector.push (mono))
        {
            const float f = detector.getFrequency();
            if (f > 0.0f)
            {
                voiced = true; unvoicedCount = 0;
                period = (float) sr / f;
                inputMidi = 69.0f + 12.0f * std::log2 (f / 440.0f);
                const float prevTarget = currentTarget;
                updateTarget (inputMidi);
                // Amount up to 100% = how far we pull you onto the note.
                // Anything above 100% becomes overshoot on note changes ("yelp" / glitch flips).
                desiredCorr = (currentTarget - inputMidi) * juce::jmin (amt, 1.0f);
                if (prevTarget > -50.0f && std::abs (currentTarget - prevTarget) > 0.5f && amt > 1.0f)
                    kick += (currentTarget - prevTarget) * (amt - 1.0f) * (isHyper ? 1.6f : 1.0f);
            }
            else if (++unvoicedCount > 3)
            {
                voiced = false;
                period = 0.0f;
                desiredCorr = 0.0f;
                currentTarget = -100.0f;
                kick = 0.0f;
            }

            const int hp = historyPos.load (std::memory_order_relaxed);
            historyIn[(size_t) hp].store (voiced ? inputMidi : 0.0f, std::memory_order_relaxed);
            historyOut[(size_t) hp].store (voiced ? inputMidi + corr + kick + trans : 0.0f, std::memory_order_relaxed);
            historyPos.store ((hp + 1) % historySize, std::memory_order_relaxed);
            liveVoiced.store (voiced);
            liveInMidi.store (inputMidi);
            liveTargetMidi.store (currentTarget + trans);
            liveOutMidi.store (inputMidi + corr + kick + trans);
        }

        corr += (desiredCorr - corr) * alpha;
        kick *= kickDecay;
        kick = juce::jlimit (-7.0f, 7.0f, kick);

        // ---- pitch shifting ----
        shiftBuf[(size_t) shiftPos] = corr + kick;
        shiftPos = (shiftPos + 1) % (int) shiftBuf.size();
        const float shift = shiftBuf[(size_t) shiftPos];   // oldest = aligned with grain time
        const float r = std::pow (2.0f, (shift + trans) / 12.0f);
        const float detune = std::pow (2.0f, widthCt / 1200.0f);
        const float ratios[PsolaEngine::numVoices] = { r, r * detune, r / detune, r * 0.5f, r * 2.0f };
        float v[PsolaEngine::numVoices];
        engine.process (mono, voiced ? period : 0.0f, ratios, active, fscale, v);

        // ---- doubles (short delays, hard panned) ----
        dblBufL[(size_t) dblPos] = v[1];
        dblBufR[(size_t) dblPos] = v[2];
        const int dsz = (int) dblBufL.size();
        const float dL = dblBufL[(size_t) ((dblPos - dblDelayL + dsz) % dsz)];
        const float dR = dblBufR[(size_t) ((dblPos - dblDelayR + dsz) % dsz)];
        dblPos = (dblPos + 1) % dsz;

        const float centre = v[0] + v[3] * octDn * 0.9f + v[4] * octU * 0.6f;
        float wl = (centre + dL * dblAmt * 0.85f) * stackNorm;
        float wr = (centre + dR * dblAmt * 0.85f) * stackNorm;

        // ---- drive ----
        if (driveAmt > 0.001f)
        {
            wl = processDrive (wl, driveAmt);
            wr = processDrive (wr, driveAmt);
            driveLp[0] += 0.55f * (wl - driveLp[0]); wl = driveLp[0];
            driveLp[1] += 0.55f * (wr - driveLp[1]); wr = driveLp[1];
        }

        // ---- lo-fi (phone band + crush) ----
        if (lofiAmt > 0.001f)
        {
            const bool take = (holdCount += 1.0f) >= holdLen;
            if (take) holdCount -= holdLen;
            float* ch[2] = { &wl, &wr };
            for (int c = 0; c < 2; ++c)
            {
                const float x = *ch[c];
                hp1[c] = hpC * (hp1[c] + x - hpPrev[c]); hpPrev[c] = x;
                lp1[c] += lpC * (hp1[c] - lp1[c]);
                lp2[c] += lpC * (lp1[c] - lp2[c]);
                float y = lp2[c] * 1.8f;
                if (take) hold[c] = std::round (y * crushLevels) / crushLevels;
                y = hold[c];
                *ch[c] = x + lofiAmt * (y - x);
            }
        }

        // ---- ping-pong delay ----
        // (always runs so repeats ring out naturally when Delay is turned down)
        {
            const int sz = (int) dlyL.size();
            const float ds = juce::jlimit (1.0f, (float) sz - 2.0f, dlySamples.getNextValue());
            float rp = (float) dlyPos - ds; if (rp < 0) rp += (float) sz;
            const int i0 = (int) rp, i1 = (i0 + 1) % sz; const float fr = rp - (float) i0;
            const float yl = dlyL[(size_t) i0] + fr * (dlyL[(size_t) i1] - dlyL[(size_t) i0]);
            const float yr = dlyR[(size_t) i0] + fr * (dlyR[(size_t) i1] - dlyR[(size_t) i0]);
            dlyLp[0] += 0.4f * (yl - dlyLp[0]);
            dlyLp[1] += 0.4f * (yr - dlyLp[1]);
            dlyL[(size_t) dlyPos] = 0.5f * (wl + wr) * dlyAmt + dlyLp[1] * 0.38f;
            dlyR[(size_t) dlyPos] = dlyLp[0] * 0.38f;
            dlyPos = (dlyPos + 1) % sz;
            wl += yl; wr += yr;
        }

        // ---- dry, delayed to line up with the tuned signal ----
        const float dryOutL = dryL[(size_t) dryPos], dryOutR = dryR[(size_t) dryPos];
        dryL[(size_t) dryPos] = inL; dryR[(size_t) dryPos] = inR;
        dryPos = (dryPos + 1) % dryLen;

        L[i] = dryOutL * (1.0f - mixAmt) + wl * mixAmt;
        if (R) R[i] = dryOutR * (1.0f - mixAmt) + wr * mixAmt;
    }

    // ---- reverb ----
    if (revAmt > 0.001f && R != nullptr)
    {
        juce::dsp::Reverb::Parameters rp;
        rp.roomSize = 0.78f; rp.damping = 0.45f; rp.width = 1.0f;
        rp.wetLevel = revAmt * 0.33f * mixAmt;
        rp.dryLevel = (1.0f - revAmt * 0.25f * mixAmt) * 0.5f;   // JUCE doubles the dry path internally
        reverb.setParameters (rp);
        juce::dsp::AudioBlock<float> block (buffer);
        auto stereo = block.getSubsetChannelBlock (0, 2);
        juce::dsp::ProcessContextReplacing<float> ctx (stereo);
        reverb.process (ctx);
    }

    for (int c = 0; c < numCh; ++c)
    {
        auto* d = buffer.getWritePointer (c);
        for (int i = 0; i < numSamples; ++i)
            d[i] = std::tanh (d[i] * outGain);
    }
}

int SuperTuneProcessor::getNumPrograms() { return numPresets; }

const juce::String SuperTuneProcessor::getProgramName (int index)
{
    return juce::isPositiveAndBelow (index, numPresets) ? presets[index].name : "";
}

void SuperTuneProcessor::setCurrentProgram (int index)
{
    if (! juce::isPositiveAndBelow (index, numPresets)) return;
    currentProgram = index;
    for (int i = 0; i < 16; ++i)
        if (auto* param = apvts.getParameter (ids[i + 2]))   // skip key & scale
            param->setValueNotifyingHost (param->convertTo0to1 (presets[index].v[i]));
}

void SuperTuneProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("program", currentProgram, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void SuperTuneProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto tree = juce::ValueTree::fromXml (*xml);
            currentProgram = tree.getProperty ("program", 0);
            apvts.replaceState (tree);
        }
}

juce::AudioProcessorEditor* SuperTuneProcessor::createEditor() { return new SuperTuneEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SuperTuneProcessor(); }
