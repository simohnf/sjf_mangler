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
class sjf_numBox :public juce::Slider, juce::Label
{
public:
    sjf_numBox(){};
    ~sjf_numBox(){};
    
    
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
    
    

    
    juce::TextButton loadButton, randomAllButton; 
    juce::ToggleButton randomOnLoopButton, hostSyncButton, playButton;
    juce::Slider revProbSlider, speedProbSlider, subDivProbSlider, ampProbSlider, stepShuffleProbSlider;
    juce::Label revProbLabel, speedProbLabel, subDivProbLabel, ampProbLabel, stepShuffleProbLabel;
    juce::Slider nSlicesSlider, nStepsSlider, fadeLenSlider;
    juce::Label nSlicesLabel, nStepsLabel, fadeLenLabel;
    juce::ComboBox phaseRateMultiplierBox;
    
//    juce::TextEditor nStepsLabel2;
    juce::TooltipWindow tooltipWindow;

//    sjf_numBox numBoxTest;
    
    std::unique_ptr<SliderAttachment> revProbAttachment, subDivProbAttachment, speedProbAttachment, ampProbAttachment, shuffleProbAttachment, nSlicesAttachment, nStepsAttachment, fadeLenAttachment;
    std::unique_ptr<ButtonAttachment> randomOnLoopAttachment, hostSyncAttachment;
    std::unique_ptr<ComboBoxAttachment> phaseRateMultiplierAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sjf_manglerAudioProcessorEditor)
};
