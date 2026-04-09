#include "PluginEditor.h"

//==============================================================================
PanzerGGEditor::PanzerGGEditor (PanzerGGProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (kPluginWidth, kPluginHeight);
    setResizable (false, false);
}

PanzerGGEditor::~PanzerGGEditor() {}

//==============================================================================
void PanzerGGEditor::paint (juce::Graphics& g)
{
    drawChassis (g);
    drawTopStrip (g);
    drawCornerScrews (g);
}

void PanzerGGEditor::resized()
{
    // Task 8 montará o layout completo de componentes aqui
}

//==============================================================================
void PanzerGGEditor::drawChassis (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    juce::ColourGradient chassisGrad (
        juce::Colour (0xff3a9080),
        0.f, bounds.getY(),
        juce::Colour (0xff1e5a50),
        0.f, bounds.getBottom(),
        false);

    g.setGradientFill (chassisGrad);
    g.fillRoundedRectangle (bounds, 6.f);

    g.setColour (juce::Colour (0xff0a2e28).withAlpha (0.6f));
    g.drawRoundedRectangle (bounds.reduced (0.5f), 6.f, 1.f);

    g.setColour (juce::Colours::black.withAlpha (0.04f));
    for (float x = 0; x < bounds.getWidth(); x += 4.f)
        g.drawVerticalLine (static_cast<int> (x), bounds.getY(), bounds.getBottom());
}

//==============================================================================
void PanzerGGEditor::drawTopStrip (juce::Graphics& g)
{
    const float stripH = 28.f;
    const auto  bounds = getLocalBounds().toFloat();

    juce::Rectangle<float> strip (bounds.getX(), bounds.getY(),
                                  bounds.getWidth(), stripH);

    juce::ColourGradient stripGrad (
        juce::Colour (0xff0e2420), 0.f, strip.getY(),
        juce::Colour (0xff0a1e1a), 0.f, strip.getBottom(),
        false);
    g.setGradientFill (stripGrad);
    g.fillRoundedRectangle (strip.withHeight (stripH + 4.f), 6.f);
    g.fillRect (strip.withTrimmedTop (4.f));

    g.setColour (juce::Colour (0xffe8a020));
    g.fillRect (bounds.getX(), bounds.getY() + stripH, bounds.getWidth(), 2.f);

    g.setColour (juce::Colour (0xffa0c8c0));
    g.setFont (juce::Font (juce::Font::getDefaultMonospacedFontName(), 11.f,
                           juce::Font::bold));
    g.drawText ("M\xc2\xb7VAVE", strip.withWidth (80.f).translated (14.f, 0.f),
                juce::Justification::centredLeft);

    g.setColour (juce::Colour (0xff78c8b8));
    g.setFont (juce::Font (juce::Font::getDefaultMonospacedFontName(), 13.f,
                           juce::Font::bold));
    g.drawText ("TANK-G", strip.withWidth (90.f).translated (70.f, 0.f),
                juce::Justification::centredLeft);

    g.setColour (juce::Colour (0xff3a7068));
    g.setFont (juce::Font (juce::Font::getDefaultSansSerifFontName(), 8.5f,
                           juce::Font::plain));
    g.drawText ("GUITAR MULTI-FX  |  VST3",
                strip.withTrimmedLeft (bounds.getWidth() - 200.f).reduced (8.f, 0.f),
                juce::Justification::centredRight);
}

//==============================================================================
void PanzerGGEditor::drawCornerScrews (juce::Graphics& g)
{
    const float r      = 8.f;
    const float margin = 10.f;
    const auto  bounds = getLocalBounds().toFloat();

    drawScrewAt (g, bounds.getX()      + margin, bounds.getY()      + margin, r);
    drawScrewAt (g, bounds.getRight()  - margin, bounds.getY()      + margin, r);
    drawScrewAt (g, bounds.getX()      + margin, bounds.getBottom() - margin, r);
    drawScrewAt (g, bounds.getRight()  - margin, bounds.getBottom() - margin, r);
}

void PanzerGGEditor::drawScrewAt (juce::Graphics& g, float cx, float cy, float r)
{
    juce::Rectangle<float> area (cx - r, cy - r, r * 2.f, r * 2.f);

    juce::ColourGradient screwGrad (
        juce::Colour (0xff5a7a74), cx - r * 0.3f, cy - r * 0.3f,
        juce::Colour (0xff1a2e2c), cx, cy,
        true);
    g.setGradientFill (screwGrad);
    g.fillEllipse (area);

    g.setColour (juce::Colour (0xff0a1e1c));
    g.drawEllipse (area, 1.f);

    g.setColour (juce::Colour (0xff0d2220));
    const float half = r * 0.65f;
    g.drawLine (cx - half, cy - half, cx + half, cy + half, 1.5f);
}
