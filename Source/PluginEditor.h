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
    
private:
    juce::AudioProcessorValueTreeState& valueTreeState;
    Sjf_manglerAudioProcessor& audioProcessor;
    
    

    
    juce::TextButton openButton, randomAllButton; 
    juce::ToggleButton randomOnLoopButton, hostSyncButton, playButton;
    juce::Slider revProbSlider, speedProbSlider, subDivProbSlider, ampProbSlider, stepShuffleProbSlider;
    juce::Label revProbLabel, speedProbLabel, subDivProbLabel, ampProbLabel, stepShuffleProbLabel;
    juce::Slider nSlicesSlider, nStepsSlider;
    juce::Label nSlicesLabel, nStepsLabel;
    juce::ComboBox speedChangeBox;
    

    std::unique_ptr<SliderAttachment> revProbAttachment, subDivProbAttachment, speedProbAttachment, ampProbAttachment, shuffleProbAttachment, nSlicesAttachment, nStepsAttachment;
    std::unique_ptr<ButtonAttachment> randomOnLoopAttachment, hostSyncAttachment;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sjf_manglerAudioProcessorEditor)
};
