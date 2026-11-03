#include "PitchEngine.h"
#include <algorithm>

//==============================================================================
void PitchDetector::prepare (double sampleRate, float minHz, float maxHz)
{
    dsr = sampleRate * 0.5;
    tauMin = std::max (2, (int) (dsr / maxHz));
    tauMax = (int) (dsr / minHz) + 2;
    window = (int) (tauMax * 1.15f);
    hop = std::max (32, (int) (dsr * 0.0027));          // ~2.7 ms between estimates

    ring.assign ((size_t) (window + tauMax + 4), 0.0f);
    frame.assign (ring.size(), 0.0f);
    diff.assign ((size_t) tauMax + 2, 0.0f);
    writePos = sinceLast = 0;
    decimAcc = 0.0f; decimPhase = false;
    freq = conf = 0.0f;
    std::fill (std::begin (history), std::end (history), 0.0f);
}

bool PitchDetector::push (float x)
{
    // 2x decimation (simple averaging lowpass)
    decimAcc += x;
    decimPhase = ! decimPhase;
    if (decimPhase) return false;

    ring[(size_t) writePos] = decimAcc * 0.5f;
    decimAcc = 0.0f;
    writePos = (writePos + 1) % (int) ring.size();

    if (++sinceLast < hop) return false;
    sinceLast = 0;
    analyse();
    return true;
}

void PitchDetector::analyse()
{
    const int total = (int) ring.size();
    for (int i = 0; i < total; ++i)
        frame[(size_t) i] = ring[(size_t) ((writePos + i) % total)];

    const float* xf = frame.data() + (total - window - tauMax);

    // Silence gate
    double energy = 0.0;
    for (int j = 0; j < window; ++j) energy += xf[j] * xf[j];
    const float rms = (float) std::sqrt (energy / window);

    float est = 0.0f, c = 0.0f;
    if (rms > 0.003f)
    {
        // YIN difference + cumulative mean normalised difference
        diff[0] = 1.0f;
        double running = 0.0;
        for (int tau = 1; tau <= tauMax; ++tau)
        {
            double d = 0.0;
            for (int j = 0; j < window; ++j)
            {
                const float delta = xf[j] - xf[j + tau];
                d += delta * delta;
            }
            running += d;
            diff[(size_t) tau] = running > 0.0 ? (float) (d * tau / running) : 1.0f;
        }

        int best = -1;
        for (int tau = tauMin; tau < tauMax; ++tau)
        {
            if (diff[(size_t) tau] < 0.18f)
            {
                while (tau + 1 < tauMax && diff[(size_t) tau + 1] < diff[(size_t) tau]) ++tau;
                best = tau;
                break;
            }
        }

        // Octave guard: if the dip at twice the lag is much deeper, the first dip was
        // just a strong harmonic - use the real (lower) period instead.
        while (best > 0 && best * 2 + 3 < tauMax)
        {
            int t2 = best * 2;
            for (int t = best * 2 - 3; t <= best * 2 + 3; ++t)
                if (diff[(size_t) t] < diff[(size_t) t2]) t2 = t;
            if (diff[(size_t) t2] < diff[(size_t) best] * 0.5f && diff[(size_t) best] > 0.01f)
                best = t2;
            else
                break;
        }

        if (best > 0)
        {
            // Parabolic interpolation for sub-sample accuracy
            const float a = diff[(size_t) best - 1], b = diff[(size_t) best], cc = diff[(size_t) best + 1];
            const float denom = a - 2.0f * b + cc;
            const float shift = std::abs (denom) > 1e-9f ? 0.5f * (a - cc) / denom : 0.0f;
            est = (float) (dsr / ((float) best + std::clamp (shift, -0.5f, 0.5f)));
            c = 1.0f - b;
        }
    }

    // Median of the last 3 estimates kills single-frame octave glitches
    history[0] = history[1]; history[1] = history[2]; history[2] = est;
    float s[3] = { history[0], history[1], history[2] };
    std::sort (s, s + 3);
    freq = (est > 0.0f) ? (s[1] > 0.0f ? s[1] : est) : 0.0f;
    conf = c;
}

//==============================================================================
void PsolaEngine::prepare (double sampleRate, int maxPeriodSamples)
{
    maxPeriod = maxPeriodSamples;
    // Grain reads reach up to ~2.25 periods ahead of the synthesis point
    // (epoch search + formant stretch), so look that far ahead.
    lookahead = (int) (maxPeriod * 2.3f) + 4;
    latency = lookahead + maxPeriod + 2;
    defaultPeriod = (float) (sampleRate * 0.005);   // 5 ms grains for unvoiced sounds

    in.assign (bufSize, 0.0f);
    for (auto& v : voices) v.out.assign (bufSize, 0.0f);
    reset();
}

void PsolaEngine::reset()
{
    std::fill (in.begin(), in.end(), 0.0f);
    n = (int64_t) bufSize * 4;
    for (auto& v : voices)
    {
        std::fill (v.out.begin(), v.out.end(), 0.0f);
        v.synth = v.epoch = (double) (n - lookahead);
        v.wasActive = false;
    }
}

float PsolaEngine::readIn (double pos) const
{
    const int64_t i0 = (int64_t) std::floor (pos);
    const float frac = (float) (pos - (double) i0);
    const float a = in[(size_t) (i0 & mask)];
    const float b = in[(size_t) ((i0 + 1) & mask)];
    return a + frac * (b - a);
}

void PsolaEngine::addGrain (Voice& v, float T, float ratio, float fscale)
{
    const double s = v.synth;
    const bool voiced = T > 0.0f;
    if (! voiced) T = defaultPeriod;
    T = std::min (T, (float) maxPeriod);

    // Keep the analysis epoch locked near the synthesis time, stepping by whole periods
    if (std::abs (v.epoch - s) > 3.0 * T) v.epoch = s;
    while (v.epoch + T * 0.5 < s) v.epoch += T;
    while (v.epoch - T * 0.5 > s) v.epoch -= T;

    double centre = v.epoch;
    if (voiced && v.lastCentre > -1.0e17)
    {
        // WSOLA-style refinement: nudge the grain so it lines up with where the previous
        // grain's waveform naturally continues (prev + T). Much steadier than peak-picking.
        const int search = std::max (2, (int) (T * 0.15f));
        const int len = (int) T;
        const double ref = v.lastCentre + T;
        double bestScore = -1e30; int bestD = 0;
        for (int d = -search; d <= search; ++d)
        {
            double score = 0.0;
            const double c = v.epoch + d;
            for (int k = -len / 2; k < len / 2; k += 2)
                score += (double) readIn (c + k) * (double) readIn (ref + k);
            if (score > bestScore) { bestScore = score; bestD = d; }
        }
        centre = v.epoch + bestD;
        v.epoch = centre;
    }
    v.lastCentre = voiced ? centre : -1.0e18;

    const int Ti = (int) T;
    const float gain = std::min (1.0f, 1.0f / ratio);
    const int64_t sc = (int64_t) std::llround (s);
    const float invT = 1.0f / (float) Ti;

    for (int k = -Ti; k <= Ti; ++k)
    {
        const float w = 0.5f * (1.0f + std::cos (3.14159265f * (float) k * invT));
        const float x = readIn (centre + (double) k * fscale);
        v.out[(size_t) ((sc + k) & mask)] += gain * w * x;
    }

    v.synth += (double) T / (double) ratio;
}

void PsolaEngine::process (float x, float period, const float* ratios, const bool* active,
                           float formantScale, float* outs)
{
    in[(size_t) (n & mask)] = x;

    const double analysisPoint = (double) (n - lookahead);
    const int64_t readPos = n - lookahead - maxPeriod - 1;

    for (int i = 0; i < numVoices; ++i)
    {
        auto& v = voices[i];
        if (! active[i])
        {
            if (v.wasActive)
                std::fill (v.out.begin(), v.out.end(), 0.0f);
            v.wasActive = false;
            v.synth = v.epoch = analysisPoint;
            outs[i] = 0.0f;
            continue;
        }
        v.wasActive = true;

        while (v.synth <= analysisPoint)
            addGrain (v, period, ratios[i], formantScale);

        float& o = v.out[(size_t) (readPos & mask)];
        outs[i] = o;
        o = 0.0f;
    }

    ++n;
}
