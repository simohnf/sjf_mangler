/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class sjf_numBox : public juce::Slider
/* Copied from https://suzuki-kengo.dev/posts/numberbox */
{
public:
    sjf_numBox()
    {
        setSliderStyle (juce::Slider::LinearBarVertical);
        setColour (juce::Slider::trackColourId, juce::Colours::transparentWhite);
        setTextBoxIsEditable (true);
        setVelocityBasedMode (true);
        setVelocityModeParameters (0.5, 1, 0.09, false);
        setDoubleClickReturnValue (true, 50.0);
        setWantsKeyboardFocus (true);
        onValueChange = [&]()
        {
            if (getValue() < 10)
                setNumDecimalPlacesToDisplay(2);
            else if (10 <= getValue() && getValue() < 100)
                setNumDecimalPlacesToDisplay(1);
            else
                setNumDecimalPlacesToDisplay(0);
        };
    };
    ~sjf_numBox(){};
    
    void paint(juce::Graphics& g) override
    {
        if (hasKeyboardFocus (false))
        {
            auto bounds = getLocalBounds().toFloat();
            auto h = bounds.getHeight();
            auto w = bounds.getWidth();
            auto len = juce::jmin (h, w) * 0.15f;
            auto thick  = len / 1.8f;
            
            g.setColour (findColour (juce::Slider::textBoxOutlineColourId));
            
            // Left top
            g.drawLine (0.0f, 0.0f, 0.0f, len, thick);
            g.drawLine (0.0f, 0.0f, len, 0.0f, thick);
            
            // Left bottom
            g.drawLine (0.0f, h, 0.0f, h - len, thick);
            g.drawLine (0.0f, h, len, h, thick);
            
            // Right top
            g.drawLine (w, 0.0f, w, len, thick);
            g.drawLine (w, 0.0f, w - len, 0.0f, thick);
            
            // Right bottom
            g.drawLine (w, h, w, h - len, thick);
            g.drawLine (w, h, w - len, h, thick);
        }
    };
};
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

class newLookAndFeel : public juce::LookAndFeel_V4
{
    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                      bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto fontSize = juce::jmin (15.0f, (float) button.getHeight() * 0.75f);
        auto offset = 0.0f;
        drawTickBox(g, button, offset, offset,
                    button.getBounds().getWidth() - offset, button.getBounds().getHeight() - offset,
                    button.getToggleState(),
                    button.isEnabled(),
                    shouldDrawButtonAsHighlighted,
                    shouldDrawButtonAsDown);

        
        g.setColour (button.findColour (juce::ToggleButton::textColourId));
        g.setFont (fontSize);
        
        if (! button.isEnabled())
            g.setOpacity (0.5f);
        g.drawFittedText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, 10);
    };
    
    void drawTickBox (juce::Graphics& g, juce::Component& component,
                      float x, float y, float w, float h,
                      const bool ticked,
                      const bool isEnabled,
                      const bool shouldDrawButtonAsHighlighted,
                      const bool shouldDrawButtonAsDown) override
    {
        juce::ignoreUnused (isEnabled, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
        
        juce::Rectangle<float> tickBounds (x, y, w, h);
        
        g.setColour (component.findColour (juce::ToggleButton::tickDisabledColourId));
        g.drawRoundedRectangle (tickBounds, 4.0f, 1.0f);
        
        if (ticked)
        {
            g.setColour (component.findColour (juce::ToggleButton::tickColourId));
            g.setOpacity(0.3);
            auto tick = getCrossShape(0.75f);
            g.fillPath (tick, tick.getTransformToScaleToFit (tickBounds.reduced (4, 5).toFloat(), false));
        }
    };
};
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
class Sjf_manglerAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    Sjf_manglerAudioProcessorEditor (Sjf_manglerAudioProcessor&, juce::AudioProcessorValueTreeState& vts);
    ~Sjf_manglerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
    typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
    
private:
    juce::AudioProcessorValueTreeState& valueTreeState;
    Sjf_manglerAudioProcessor& audioProcessor;
    
    newLookAndFeel otherLookAndFeel;

    
    juce::TextButton loadButton, randomAllButton; 
    juce::ToggleButton randomOnLoopButton, hostSyncButton, playButton;
    juce::Slider revProbSlider, speedProbSlider, subDivProbSlider, ampProbSlider, stepShuffleProbSlider;
    juce::Label revProbLabel, speedProbLabel, subDivProbLabel, ampProbLabel, stepShuffleProbLabel;
    juce::Label nSlicesLabel, nStepsLabel, fadeLenLabel;
    juce::ComboBox phaseRateMultiplierBox, interpolationTypeBox;
    
    juce::TooltipWindow tooltipWindow {this, 700};

    
    sjf_numBox fadeLenNumBox, nSlicesNumBox, nStepsNumBox;
    
    std::unique_ptr<SliderAttachment> revProbAttachment, subDivProbAttachment, speedProbAttachment, ampProbAttachment, shuffleProbAttachment, nSlicesAttachment, nStepsAttachment, fadeLenAttachment;
    std::unique_ptr<ButtonAttachment> randomOnLoopAttachment, hostSyncAttachment, playButtonAttachment;
    std::unique_ptr<ComboBoxAttachment> phaseRateMultiplierAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sjf_manglerAudioProcessorEditor)
};
