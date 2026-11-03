#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class SuperTuneLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SuperTuneLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool down, int, int, int, int, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    juce::Label* createSliderTextBox (juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool over, bool down) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool over, bool down) override;
};

// Scrolling pitch graph: grey = your voice, red = tuned output.
class PitchDisplay : public juce::Component
{
public:
    explicit PitchDisplay (SuperTuneProcessor& p) : proc (p) {}
    void paint (juce::Graphics&) override;
    float pulse = 0.0f;

private:
    SuperTuneProcessor& proc;
    float centre = 57.0f;
};

class SuperTuneEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit SuperTuneEditor (SuperTuneProcessor&);
    ~SuperTuneEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Knob
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
    };
    struct Section { juce::String title; juce::Rectangle<int> bounds; std::vector<Knob*> knobs; };

    void timerCallback() override;
    Knob* makeKnob (const juce::String& id, const juce::String& name);
    void drawPanel (juce::Graphics&, juce::Rectangle<float>, const juce::String& title);

    SuperTuneProcessor& proc;
    SuperTuneLookAndFeel lnf;

    std::vector<std::unique_ptr<Knob>> knobs;
    std::vector<Section> sections;
    Knob* speedKnob = nullptr;

    juce::ComboBox presetBox, keyBox, scaleBox;
    juce::Label keyLabel, scaleLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> keyAttach, scaleAttach;
    juce::TextButton hyperButton { "HYPER" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> hyperAttach;

    PitchDisplay display;
    juce::Rectangle<int> leftPanel;
    float time = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SuperTuneEditor)
};
