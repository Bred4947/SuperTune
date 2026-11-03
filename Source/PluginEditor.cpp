#include "PluginEditor.h"

namespace Col
{
    const juce::Colour bgTop    { 0xff0b0b0f };
    const juce::Colour bgBottom { 0xff17171f };
    const juce::Colour panel    { 0xff121218 };
    const juce::Colour edge     { 0xff2b2b36 };
    const juce::Colour red      { 0xffff2a3d };
    const juce::Colour orange   { 0xffff7a2a };
    const juce::Colour text     { 0xffe9e9f0 };
    const juce::Colour dim      { 0xff8a8a99 };
}

static juce::Font stFont (float h, bool bold = false)
{
    return juce::Font (juce::FontOptions().withHeight (h).withStyle (bold ? "Bold" : "Regular"));
}

static juce::String noteName (float midi)
{
    static const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    const int n = (int) std::lround (midi);
    return juce::String (names[((n % 12) + 12) % 12]) + juce::String (n / 12 - 1);
}

//==============================================================================
SuperTuneLookAndFeel::SuperTuneLookAndFeel()
{
    setColour (juce::ComboBox::textColourId, Col::text);
    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff15151b));
    setColour (juce::PopupMenu::textColourId, Col::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Col::red);
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    setColour (juce::Slider::textBoxTextColourId, Col::dim);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
}

void SuperTuneLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                             float startAngle, float endAngle, juce::Slider& s)
{
    using namespace juce;
    const float size = (float) jmin (w, h);
    const float cx = (float) x + (float) w * 0.5f, cy = (float) y + (float) h * 0.5f;
    const float radius = size * 0.5f - 4.0f;
    const float angle = startAngle + pos * (endAngle - startAngle);
    const bool big = size > 100.0f;
    const float thick = big ? 7.0f : 3.5f;
    const float alpha = s.isEnabled() ? 1.0f : 0.35f;

    Path track;
    track.addCentredArc (cx, cy, radius, radius, 0.0f, startAngle, endAngle, true);
    g.setColour (Col::edge);
    g.strokePath (track, PathStrokeType (thick, PathStrokeType::curved, PathStrokeType::rounded));

    Path val;
    val.addCentredArc (cx, cy, radius, radius, 0.0f, startAngle, angle, true);
    if (big)
    {
        g.setColour (Col::red.withAlpha (0.25f * alpha));
        g.strokePath (val, PathStrokeType (thick + 8.0f, PathStrokeType::curved, PathStrokeType::rounded));
    }
    g.setGradientFill (ColourGradient (Col::red.withAlpha (alpha), cx - radius, cy + radius,
                                       Col::orange.withAlpha (alpha), cx + radius, cy - radius, false));
    g.strokePath (val, PathStrokeType (thick, PathStrokeType::curved, PathStrokeType::rounded));

    // Chrome ring + dark cap
    const float capR = radius - thick - (big ? 8.0f : 4.0f);
    auto cap = Rectangle<float> (capR * 2.0f, capR * 2.0f).withCentre ({ cx, cy });
    g.setGradientFill (ColourGradient (Colour (0xffd8d8e0), cx, cap.getY(), Colour (0xff4a4a55), cx, cap.getBottom(), false));
    g.fillEllipse (cap.expanded (2.0f));
    g.setGradientFill (ColourGradient (Colour (0xff3a3a46), cx - capR * 0.3f, cy - capR * 0.5f,
                                       Colour (0xff0e0e12), cx + capR * 0.5f, cy + capR, true));
    g.fillEllipse (cap);

    // Pointer
    const float p1 = capR * 0.35f, p2 = capR * 0.85f;
    Path ptr;
    ptr.startNewSubPath (cx + p1 * std::sin (angle), cy - p1 * std::cos (angle));
    ptr.lineTo (cx + p2 * std::sin (angle), cy - p2 * std::cos (angle));
    g.setColour (Colours::white.withAlpha (alpha));
    g.strokePath (ptr, PathStrokeType (big ? 3.5f : 2.0f, PathStrokeType::curved, PathStrokeType::rounded));
}

void SuperTuneLookAndFeel::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&)
{
    using namespace juce;
    auto r = Rectangle<float> (0, 0, (float) w, (float) h).reduced (1.0f);
    g.setGradientFill (ColourGradient (Colour (0xff1d1d25), 0, r.getY(), Colour (0xff111116), 0, r.getBottom(), false));
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (Col::red.withAlpha (0.7f));
    g.drawRoundedRectangle (r, 6.0f, 1.2f);
    Path a;
    const float ax = r.getRight() - 16.0f, ay = r.getCentreY();
    a.addTriangle (ax - 5, ay - 2.5f, ax + 5, ay - 2.5f, ax, ay + 3.5f);
    g.setColour (Col::red);
    g.fillPath (a);
}

juce::Font SuperTuneLookAndFeel::getComboBoxFont (juce::ComboBox&) { return stFont (14.0f, true); }

void SuperTuneLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (10, 1, box.getWidth() - 32, box.getHeight() - 2);
    label.setFont (getComboBoxFont (box));
}

juce::Label* SuperTuneLookAndFeel::createSliderTextBox (juce::Slider& s)
{
    auto* l = LookAndFeel_V4::createSliderTextBox (s);
    l->setFont (stFont (s.getWidth() > 120 ? 15.0f : 11.5f, s.getWidth() > 120));
    l->setColour (juce::Label::textColourId, s.getWidth() > 120 ? Col::text : Col::dim);
    l->setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
    l->setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    return l;
}

void SuperTuneLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool)
{
    using namespace juce;
    auto r = b.getLocalBounds().toFloat().reduced (3.0f);
    const float corner = r.getHeight() * 0.5f;
    if (b.getToggleState())
    {
        g.setColour (Col::red.withAlpha (0.35f));
        g.fillRoundedRectangle (r.expanded (3.0f), corner + 3.0f);
        g.setGradientFill (ColourGradient (Colour (0xffff5a4a), 0, r.getY(), Colour (0xffc4001a), 0, r.getBottom(), false));
        g.fillRoundedRectangle (r, corner);
        g.setColour (Colours::white.withAlpha (0.25f));
        g.fillRoundedRectangle (r.withHeight (r.getHeight() * 0.45f).reduced (6.0f, 2.0f), corner * 0.6f);
    }
    else
    {
        g.setColour (Colour (0xff15151b));
        g.fillRoundedRectangle (r, corner);
        g.setColour (Col::red.withAlpha (over ? 0.9f : 0.55f));
        g.drawRoundedRectangle (r, corner, 1.6f);
    }
}

void SuperTuneLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool)
{
    g.setFont (stFont (20.0f, true));
    g.setColour (b.getToggleState() ? juce::Colours::white : Col::red.withAlpha (0.85f));
    g.drawText (b.getButtonText(), b.getLocalBounds(), juce::Justification::centred);
}

//==============================================================================
void PitchDisplay::paint (juce::Graphics& g)
{
    using namespace juce;
    auto b = getLocalBounds().toFloat();
    g.setColour (Colour (0xff0c0c10));
    g.fillRoundedRectangle (b, 10.0f);

    auto plot = b.reduced (10.0f).withTrimmedLeft (34.0f).withTrimmedTop (4.0f);
    const int N = SuperTuneProcessor::historySize;
    const int pos = proc.historyPos.load();

    // Follow the singer: centre the view on recent pitch
    float sum = 0.0f; int cnt = 0;
    for (int i = N - 60; i < N; ++i)
    {
        const float v = proc.historyOut[(size_t) ((pos + i) % N)].load();
        if (v > 0.0f) { sum += v; ++cnt; }
    }
    if (cnt > 0)
    {
        const float want = sum / (float) cnt;
        centre = std::abs (want - centre) > 5.0f ? want : centre + (want - centre) * 0.08f;
    }
    const float span = 14.0f;
    const float lo = centre - span * 0.5f;
    auto yOf = [&] (float m) { return plot.getBottom() - (m - lo) / span * plot.getHeight(); };

    // Semitone grid
    const int keyIdx = (int) proc.apvts.getRawParameterValue ("key")->load();
    const int scaleIdx = (int) proc.apvts.getRawParameterValue ("scale")->load();
    static const bool scales[][12] = {
        { 1,1,1,1,1,1,1,1,1,1,1,1 }, { 1,0,1,0,1,1,0,1,0,1,0,1 }, { 1,0,1,1,0,1,0,1,1,0,1,0 },
        { 1,0,1,1,0,1,0,1,1,0,0,1 }, { 1,0,1,0,1,0,0,1,0,1,0,0 }, { 1,0,0,1,0,1,0,1,0,0,1,0 },
        { 1,0,0,1,0,1,1,1,0,0,1,0 }, { 1,0,1,1,0,1,0,1,0,1,1,0 }, { 1,1,0,1,0,1,0,1,1,0,1,0 } };
    const float target = proc.liveTargetMidi.load();
    const bool voiced = proc.liveVoiced.load();

    for (int m = (int) std::floor (lo); m <= (int) std::ceil (lo + span); ++m)
    {
        const float yy = yOf ((float) m);
        if (yy < plot.getY() - 2 || yy > plot.getBottom() + 2) continue;
        const bool inScale = scales[jlimit (0, 8, scaleIdx)][((m - keyIdx) % 12 + 12) % 12];
        if (voiced && m == (int) std::lround (target))
        {
            g.setColour (Col::red.withAlpha (0.14f));
            g.fillRect (plot.getX(), yy - plot.getHeight() / span * 0.5f, plot.getWidth(), plot.getHeight() / span);
        }
        g.setColour (inScale ? Colour (0xff2c2c38) : Colour (0xff17171e));
        g.drawHorizontalLine ((int) yy, plot.getX(), plot.getRight());
        g.setFont (stFont (10.5f, inScale));
        g.setColour (inScale ? Col::text.withAlpha (0.8f) : Col::dim.withAlpha (0.35f));
        g.drawText (noteName ((float) m), (int) b.getX() + 6, (int) yy - 7, 34, 14, Justification::centredLeft);
    }

    // Pitch traces
    auto trace = [&] (std::array<std::atomic<float>, SuperTuneProcessor::historySize>& h, Colour c, float thick, bool glow)
    {
        Path path; bool drawing = false;
        for (int i = 0; i < N; ++i)
        {
            const float v = h[(size_t) ((pos + i) % N)].load();
            const float xx = plot.getX() + plot.getWidth() * (float) i / (float) (N - 1);
            if (v <= 0.0f) { drawing = false; continue; }
            const float yy = jlimit (plot.getY(), plot.getBottom(), yOf (v));
            if (! drawing) { path.startNewSubPath (xx, yy); drawing = true; }
            else path.lineTo (xx, yy);
        }
        if (glow)
        {
            g.setColour (c.withAlpha (0.18f));
            g.strokePath (path, PathStrokeType (thick + 7.0f, PathStrokeType::curved, PathStrokeType::rounded));
        }
        g.setColour (c);
        g.strokePath (path, PathStrokeType (thick, PathStrokeType::curved, PathStrokeType::rounded));
    };
    trace (proc.historyIn, Col::dim.withAlpha (0.7f), 1.5f, false);
    trace (proc.historyOut, Col::red, 2.6f, true);

    // Readout
    auto ro = b.reduced (16.0f).removeFromTop (60.0f).removeFromRight (200.0f);
    g.setFont (stFont (40.0f, true));
    g.setColour (voiced ? Col::text : Col::dim.withAlpha (0.4f));
    g.drawText (voiced ? noteName (target) : String ("--"), ro.removeFromTop (42.0f), Justification::centredRight);
    if (voiced)
    {
        // How far off you actually sang, before correction
        const float trans = proc.apvts.getRawParameterValue ("transpose")->load();
        const int cents = roundToInt ((proc.liveInMidi.load() + trans - target) * 100.0f);
        g.setFont (stFont (13.0f, true));
        g.setColour (std::abs (cents) < 10 ? Colour (0xff5dff8a) : Col::orange);
        g.drawText ("YOU: " + String (cents > 0 ? "+" : "") + String (cents) + " cents",
                    ro.removeFromTop (18.0f), Justification::centredRight);
    }

    // Border (pulses red in HYPER mode)
    g.setColour (Col::red.withAlpha (0.25f + 0.6f * pulse));
    g.drawRoundedRectangle (b.reduced (0.5f), 10.0f, 1.0f + 2.0f * pulse);
}

//==============================================================================
SuperTuneEditor::SuperTuneEditor (SuperTuneProcessor& p)
    : AudioProcessorEditor (p), proc (p), display (p)
{
    setLookAndFeel (&lnf);

    speedKnob = makeKnob ("speed", "RETUNE SPEED");
    speedKnob->slider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 120, 22);
    speedKnob->label.setFont (stFont (13.0f, true));

    sections.push_back ({ "VOICE", {}, { makeKnob ("amount", "Amount"), makeKnob ("transpose", "Transpose"), makeKnob ("formant", "Formant") } });
    sections.push_back ({ "STACK", {}, { makeKnob ("double", "Double"), makeKnob ("width", "Width"),
                                         makeKnob ("octDown", "Oct Down"), makeKnob ("octUp", "Oct Up") } });
    sections.push_back ({ "FX", {}, { makeKnob ("drive", "Drive"), makeKnob ("lofi", "Lo-Fi"), makeKnob ("delay", "Delay"),
                                      makeKnob ("delayTime", "Time"), makeKnob ("reverb", "Reverb") } });
    sections.push_back ({ "OUT", {}, { makeKnob ("mix", "Mix"), makeKnob ("output", "Output") } });

    auto setupLabel = [this] (juce::Label& l, const juce::String& t)
    {
        l.setText (t, juce::dontSendNotification);
        l.setFont (stFont (12.0f, true));
        l.setColour (juce::Label::textColourId, Col::dim);
        addAndMakeVisible (l);
    };
    setupLabel (keyLabel, "KEY");
    setupLabel (scaleLabel, "SCALE");

    keyBox.addItemList ({ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 1);
    scaleBox.addItemList ({ "Chromatic", "Major", "Minor", "Harmonic Minor", "Pentatonic Maj",
                            "Pentatonic Min", "Blues", "Dorian", "Phrygian" }, 1);
    addAndMakeVisible (keyBox);
    addAndMakeVisible (scaleBox);
    keyAttach   = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "key", keyBox);
    scaleAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "scale", scaleBox);

    hyperButton.setClickingTogglesState (true);
    addAndMakeVisible (hyperButton);
    hyperAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "hyper", hyperButton);

    for (int i = 0; i < proc.getNumPrograms(); ++i)
        presetBox.addItem (proc.getProgramName (i), i + 1);
    presetBox.setSelectedId (proc.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this] { proc.setCurrentProgram (presetBox.getSelectedId() - 1); };
    addAndMakeVisible (presetBox);

    addAndMakeVisible (display);

    setSize (1000, 620);
    startTimerHz (30);
}

SuperTuneEditor::~SuperTuneEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

SuperTuneEditor::Knob* SuperTuneEditor::makeKnob (const juce::String& id, const juce::String& name)
{
    auto k = std::make_unique<Knob>();
    k->slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k->slider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 58, 16);
    k->slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    k->attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, id, k->slider);
    if (auto* param = proc.apvts.getParameter (id))
        k->slider.setDoubleClickReturnValue (true, param->convertFrom0to1 (param->getDefaultValue()));

    k->label.setText (name, juce::dontSendNotification);
    k->label.setJustificationType (juce::Justification::centred);
    k->label.setFont (stFont (11.5f, true));
    k->label.setColour (juce::Label::textColourId, Col::text.withAlpha (0.85f));
    addAndMakeVisible (k->slider);
    addAndMakeVisible (k->label);
    knobs.push_back (std::move (k));
    return knobs.back().get();
}

void SuperTuneEditor::timerCallback()
{
    time += 1.0f / 30.0f;
    const bool hyper = hyperButton.getToggleState();
    speedKnob->slider.setEnabled (! hyper);
    display.pulse = hyper ? 0.5f + 0.5f * std::sin (time * 9.0f) : 0.0f;
    display.repaint();
}

void SuperTuneEditor::drawPanel (juce::Graphics& g, juce::Rectangle<float> r, const juce::String& title)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff16161d), 0, r.getY(), juce::Colour (0xff101015), 0, r.getBottom(), false));
    g.fillRoundedRectangle (r, 10.0f);
    g.setColour (Col::edge);
    g.drawRoundedRectangle (r.reduced (0.5f), 10.0f, 1.0f);
    if (title.isNotEmpty())
    {
        g.setColour (Col::red);
        g.fillRect (r.getX() + 14.0f, r.getY() + 13.0f, 3.0f, 11.0f);
        g.setFont (stFont (12.0f, true));
        g.setColour (Col::text.withAlpha (0.9f));
        g.drawText (title, r.reduced (22.0f, 9.0f).withHeight (18.0f), juce::Justification::topLeft);
    }
}

void SuperTuneEditor::paint (juce::Graphics& g)
{
    using namespace juce;
    g.setGradientFill (ColourGradient (Col::bgTop, 0, 0, Col::bgBottom, 0, (float) getHeight(), false));
    g.fillAll();

    // Red glow behind the display
    auto glowC = display.getBounds().toFloat().getCentre();
    g.setGradientFill (ColourGradient (Col::red.withAlpha (0.10f), glowC.x, glowC.y,
                                       Col::red.withAlpha (0.0f), glowC.x + 520.0f, glowC.y, true));
    g.fillRect (getLocalBounds());

    // Subtle diagonal texture
    g.setColour (Colours::white.withAlpha (0.015f));
    for (int x = -getHeight(); x < getWidth(); x += 6)
        g.drawLine ((float) x, (float) getHeight(), (float) (x + getHeight()), 0.0f, 1.0f);

    // Title: chrome SUPER + red TUNE
    g.setFont (stFont (40.0f, true));
    const int superW = (int) std::ceil (GlyphArrangement::getStringWidth (g.getCurrentFont(), "SUPER"));
    g.setGradientFill (ColourGradient (Colours::white, 0, 16, Colour (0xff8a8a96), 0, 56, false));
    g.drawText ("SUPER", 26, 14, superW + 4, 44, Justification::centredLeft);
    g.setGradientFill (ColourGradient (Colour (0xffff5a4a), 0, 16, Colour (0xffc4001a), 0, 56, false));
    g.drawText ("TUNE", 26 + superW, 14, 200, 44, Justification::centredLeft);
    g.setFont (stFont (12.0f));
    g.setColour (Col::dim);
    g.drawText ("pitch correction  //  underground edition  //  v1.0", 28, 54, 400, 16, Justification::centredLeft);

    drawPanel (g, leftPanel.toFloat(), {});
    for (auto& s : sections)
        drawPanel (g, s.bounds.toFloat(), s.title);
}

void SuperTuneEditor::resized()
{
    presetBox.setBounds (getWidth() - 310, 22, 280, 34);

    leftPanel = { 20, 82, 230, 338 };
    keyLabel.setBounds (34, 92, 60, 18);
    keyBox.setBounds (34, 110, 70, 30);
    scaleLabel.setBounds (114, 92, 120, 18);
    scaleBox.setBounds (114, 110, 122, 30);
    speedKnob->label.setBounds (20, 152, 230, 18);
    speedKnob->slider.setBounds (45, 170, 180, 182);
    hyperButton.setBounds (45, 360, 180, 48);

    display.setBounds (262, 82, 718, 338);

    const int cellW = 58, pad = 12, gap = 12, panelY = 434, panelH = 168;
    int total = 0;
    for (auto& s : sections) total += (int) s.knobs.size() * cellW + pad * 2;
    total += gap * ((int) sections.size() - 1);
    int x = (getWidth() - total) / 2;
    for (auto& s : sections)
    {
        const int pw = (int) s.knobs.size() * cellW + pad * 2;
        s.bounds = { x, panelY, pw, panelH };
        int kx = x + pad;
        for (auto* k : s.knobs)
        {
            k->label.setBounds (kx - 4, panelY + 40, cellW + 8, 16);
            k->slider.setBounds (kx, panelY + 60, cellW, 92);
            kx += cellW;
        }
        x += pw + gap;
    }
}
