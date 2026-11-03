# SuperTune

Pitch correction for FL Studio and any other DAW (VST3 + AU), built for underground rap vocals.

## Features
- **Retune Speed**: from 0 ms (instant, hard-tuned) to 400 ms (natural)
- **HYPER mode**: instant retune, no note stickiness, and over-the-top overshoot flips on every note change
- **Amount**: 0–150%. Anything over 100% adds overshoot ("yelp") when you jump between notes
- **Key + Scale**: Chromatic, Major, Minor, Harmonic Minor, Pentatonic Major/Minor, Blues, Dorian, Phrygian
- **Formant** (±7 semitones) for baby-voice or demon tones, plus **Transpose** (±12)
- **Stack**: stereo double (with Width), octave-down and octave-up layers
- **FX**: Drive, Lo-Fi (phone band + crush), ping-pong Delay, Reverb, Mix, Output
- **Live pitch graph**: grey = what you sang, red = what comes out
- **24 presets**, level-matched so switching between them doesn't change your volume

Presets only change the sound character. Your Key and Scale stay where you set them.

## Presets
HARD TUNE, RAGE LEAD, VAMP BABY VOICE, LONELY FOG, DARK STACK, BLOWN OUT HYPER, GLITCH SIREN,
UNDERGROUND GRIT, PLUGG SWEET, PLUGGNB SILK, TEEN STACK, TONKA TONE, ETHEREAL AIR, JERK BOUNCE,
SIGIL HYPERPOP, DRILL NIGHT, CLOUD RAP DREAM, DEMON DOWN, CHIPMUNK UP, PHONE CALL, ROBOT CLASSIC,
STUDIO POLISH, NATURAL TOUCH, INIT

## Building
Upload this folder to GitHub, including the hidden `.github` folder. GitHub Actions then builds
`SuperTune-Mac.zip` and `SuperTune-Windows.zip`, each with a double-click installer.
They show up under **Releases**.

To build on a Mac yourself (needs Xcode Command Line Tools and CMake):
```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j 8
```

## Latency
SuperTune reports about 45 ms of latency to your DAW. FL Studio compensates for it automatically during
playback. When you're recording through it live, you'll hear that delay.

## Where to tweak things
| Want to change… | File |
|---|---|
| Presets | `Source/PluginProcessor.cpp` → the `presets[]` table |
| Pitch detection / shifting | `Source/PitchEngine.cpp` |
| Correction logic, HYPER, FX | `Source/PluginProcessor.cpp` → `processBlock` |
| Look and layout | `Source/PluginEditor.cpp` |
