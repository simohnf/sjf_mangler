/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../sjf_audio/sjf_numBox.h"
#include "../sjf_audio/sjf_LookAndFeel.h"

//==============================================================================
/**
*/

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
class Sjf_manglerAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
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
    
    sjf_lookAndFeel otherLookAndFeel;
    
    void timerCallback() override;


    
    juce::TextButton loadButton, randomAllButton; 
    juce::ToggleButton randomOnLoopButton, hostSyncButton, playButton, tooltipsToggle;
    juce::Slider revProbSlider, speedProbSlider, subDivProbSlider, ampProbSlider, stepShuffleProbSlider;
    juce::Label /*revProbLabel, speedProbLabel, subDivProbLabel, ampProbLabel, stepShuffleProbLabel, nSlicesLabel, nStepsLabel, fadeLenLabel, sampleNameLabel,*/ tooltipLabel;
    juce::ComboBox phaseRateMultiplierBox, interpolationTypeBox;
    

    
    sjf_numBox fadeLenNumBox, nSlicesNumBox, nStepsNumBox;
    
    std::unique_ptr<SliderAttachment> revProbAttachment, subDivProbAttachment, speedProbAttachment, ampProbAttachment, shuffleProbAttachment, nSlicesAttachment, nStepsAttachment, fadeLenAttachment;
    std::unique_ptr<ButtonAttachment> randomOnLoopAttachment, hostSyncAttachment, playButtonAttachment;
    std::unique_ptr<ComboBoxAttachment> phaseRateMultiplierAttachment, interpolationTypeAttachment;
    
    juce::String MAIN_TOOLTIP = "sjf_mangler: \nRandomised sample editor\n\n1) load a sample \n2) turn on play (NB if sync is set you need to start the transport) \n3) play with the controls to get randomised variations";
    
    //    juce::SharedResourcePointer<juce::TooltipWindow> tooltipWindow;
    
//    juce::Image m_backgroundImage = juce::ImageCache::getFromMemory (BinaryData::primes1_png, BinaryData::primes1_pngSize );
//    juce::Image m_backgroundImage = juce::ImageCache::getFromMemory (BinaryData::primes2_png, BinaryData::primes2_pngSize );
//    juce::Image m_backgroundImage = juce::ImageCache::getFromMemory (BinaryData::primes3_png, BinaryData::primes3_pngSize );
//    juce::Image m_backgroundImage = juce::ImageCache::getFromMemory (BinaryData::primes4_png, BinaryData::primes4_pngSize );
//    juce::Image m_backgroundImage = juce::ImageCache::getFromMemory (BinaryData::Tree1_png, BinaryData::Tree1_pngSize );
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sjf_manglerAudioProcessorEditor)
};
