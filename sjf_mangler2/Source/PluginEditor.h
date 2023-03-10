/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin editor.
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "/Users/simonfay/Programming_Stuff/sjf_audio/sjf_numBox.h"
#include "/Users/simonfay/Programming_Stuff/sjf_audio/sjf_lookAndFeel.h"
#include "/Users/simonfay/Programming_Stuff/sjf_audio/sjf_multislider.h"

//==============================================================================
/**
 */

//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
class Sjf_Mangler2AudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    Sjf_Mangler2AudioProcessorEditor (Sjf_Mangler2AudioProcessor&, juce::AudioProcessorValueTreeState& vts);
    ~Sjf_Mangler2AudioProcessorEditor() override;
    
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
    typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
    
private:
    
    
    juce::AudioProcessorValueTreeState& valueTreeState;
    Sjf_Mangler2AudioProcessor& audioProcessor;
    
    sjf_lookAndFeel otherLookAndFeel;
    
    void timerCallback() override;
    
    
    juce::TextButton loadButton, randomAllButton, readSampleInfoButton, loadFolderButton;
    juce::ToggleButton randomOnLoopButton, hostSyncButton, playButton, tooltipsToggle;
    juce::Slider revProbSlider, speedProbSlider, subDivProbSlider, ampProbSlider, stepShuffleProbSlider, sampleChoiceSlider;
    juce::Label /*revProbLabel, speedProbLabel, subDivProbLabel, ampProbLabel, stepShuffleProbLabel, nSlicesLabel, nStepsLabel, fadeLenLabel, sampleNameLabel,*/ tooltipLabel;
    juce::ComboBox phaseRateMultiplierBox, interpolationTypeBox, voiceComboBox;
    
    
    
    sjf_numBox fadeLenNumBox, nSlicesNumBox, nStepsNumBox;
    sjf_multislider sampleProbMultiSlider;
    
    std::unique_ptr<SliderAttachment> revProbAttachment, subDivProbAttachment, speedProbAttachment, ampProbAttachment, shuffleProbAttachment, nSlicesAttachment, nStepsAttachment, fadeLenAttachment, sampleChoiceSliderAttachment;
    
    std::unique_ptr<ButtonAttachment> randomOnLoopAttachment, hostSyncAttachment, playButtonAttachment;
    std::unique_ptr<ComboBoxAttachment> phaseRateMultiplierAttachment, interpolationTypeAttachment;
    
    juce::String MAIN_TOOLTIP = "sjf_mangler2: \nRandomised sample editor\n\n1) load a sample \n2) turn on play (NB if sync is set you need to start the transport) \n3) play with the controls to get randomised variations\nmangler2 allows you to load multiple samples and have the random variations can include segments from each of these samples";
    
    //    juce::SharedResourcePointer<juce::TooltipWindow> tooltipWindow;
    
    //    juce::Image m_backgroundImage = juce::ImageCache::getFromMemory (BinaryData::primes1_png, BinaryData::primes1_pngSize );
    //    juce::Image m_backgroundImage = juce::ImageCache::getFromMemory (BinaryData::primes2_png, BinaryData::primes2_pngSize );
    //    juce::Image m_backgroundImage = juce::ImageCache::getFromMemory (BinaryData::primes3_png, BinaryData::primes3_pngSize );
    //    juce::Image m_backgroundImage = juce::ImageCache::getFromMemory (BinaryData::primes4_png, BinaryData::primes4_pngSize );
    //    juce::Image m_backgroundImage = juce::ImageCache::getFromMemory (BinaryData::Tree1_png, BinaryData::Tree1_pngSize );
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sjf_Mangler2AudioProcessorEditor)
};
