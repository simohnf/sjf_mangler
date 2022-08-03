/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Sjf_manglerAudioProcessorEditor::Sjf_manglerAudioProcessorEditor (Sjf_manglerAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), valueTreeState (vts), audioProcessor (p)
{
    
    
    addAndMakeVisible (openButton);
    openButton.setButtonText ("Open...");
    openButton.onClick = [this] { audioProcessor.openButtonClicked() ; playButton.setToggleState(false, juce::dontSendNotification) ;};
    
    
    addAndMakeVisible (playButton);
    playButton.setButtonText ("Play");
    playButton.onClick = [this] { audioProcessor.playButtonClicked( playButton.getToggleState() ) ;};
    
    addAndMakeVisible (randomAllButton);
    randomAllButton.setButtonText ("Randomise All");
    randomAllButton.onClick = [this] { audioProcessor.sampleMangler.randomiseAll() ;};
    
    addAndMakeVisible (randomOnLoopButton);
    randomOnLoopAttachment.reset(new ButtonAttachment(valueTreeState, "randomOnLoop", randomOnLoopButton));
    randomOnLoopButton.setButtonText ("Randomise All On Loop");
    
    addAndMakeVisible (hostSyncButton);
    hostSyncAttachment.reset(new ButtonAttachment(valueTreeState, "syncToHost", hostSyncButton));
    hostSyncButton.setButtonText ("Sync To Host");
    
    addAndMakeVisible (revProbSlider);
    revProbAttachment.reset(new SliderAttachment (valueTreeState, "revProb", revProbSlider));
    
    addAndMakeVisible (revProbLabel);
    revProbLabel.setText ("RevProb", juce::dontSendNotification);
    revProbLabel.attachToComponent (&revProbSlider, true);
    
    addAndMakeVisible (speedProbSlider);
    speedProbAttachment.reset(new SliderAttachment (valueTreeState, "speedProb", speedProbSlider));
    
    addAndMakeVisible (speedProbLabel);
    speedProbLabel.setText ("SpeedProb", juce::dontSendNotification);
    speedProbLabel.attachToComponent (&speedProbSlider, true);
    
    addAndMakeVisible (subDivProbSlider);
    subDivProbAttachment.reset(new SliderAttachment (valueTreeState, "divProb", subDivProbSlider));
    subDivProbSlider.setRange (0.0f, 100.0f);
    
    addAndMakeVisible (subDivProbLabel);
    subDivProbLabel.setText ("subDivProb", juce::dontSendNotification);
    subDivProbLabel.attachToComponent (&subDivProbSlider, true);
    
    addAndMakeVisible (ampProbSlider);
    ampProbAttachment.reset(new SliderAttachment (valueTreeState, "ampProb", ampProbSlider));
    
    addAndMakeVisible (ampProbLabel);
    ampProbLabel.setText ("ampProb", juce::dontSendNotification);
    ampProbLabel.attachToComponent (&ampProbSlider, true);
    
    addAndMakeVisible (stepShuffleProbSlider);
    shuffleProbAttachment.reset(new SliderAttachment (valueTreeState, "shuffleProb", stepShuffleProbSlider));
    
    addAndMakeVisible (stepShuffleProbLabel);
    stepShuffleProbLabel.setText ("shuffleProb", juce::dontSendNotification);
    stepShuffleProbLabel.attachToComponent (&stepShuffleProbSlider, true);
    
    
    addAndMakeVisible (nSlicesSlider);
    nSlicesSlider.setValue(audioProcessor.sampleMangler.getNumSlices());
    nSlicesAttachment.reset(new SliderAttachment (valueTreeState, "numSlices", nSlicesSlider));
    
    addAndMakeVisible (nSlicesLabel);
    nSlicesLabel.setText ("slices", juce::dontSendNotification);
    nSlicesLabel.attachToComponent (&nSlicesSlider, true);
    
    
    addAndMakeVisible (nStepsSlider);
    nStepsSlider.setValue(audioProcessor.sampleMangler.getNumSteps());
    nStepsAttachment.reset(new SliderAttachment (valueTreeState, "numSteps", nStepsSlider));
    
    addAndMakeVisible (nStepsLabel);
    nStepsLabel.setText ("steps", juce::dontSendNotification);
    nStepsLabel.attachToComponent (&nStepsSlider, true);
    
    addAndMakeVisible(speedChangeBox);
    speedChangeBox.addItem("*1/4", 1);
    speedChangeBox.addItem("*1/2", 2);
    speedChangeBox.addItem("*1", 3);
    speedChangeBox.addItem("*2", 4);
    speedChangeBox.addItem("*4", 5);
    speedChangeBox.setSelectedId(3);
    speedChangeBox.onChange = [this] { audioProcessor.sampleMangler.setPhaseRateMultiplierIndex(speedChangeBox.getSelectedId() - 1) ; };
    
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

Sjf_manglerAudioProcessorEditor::~Sjf_manglerAudioProcessorEditor()
{
}


//==============================================================================
void Sjf_manglerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
//    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
    g.drawFittedText("sjf_mangler", (getWidth()/2.0f) - 50, 0, 100, 20, juce::Justification::centred, 1);
}

void Sjf_manglerAudioProcessorEditor::resized()
{
    revProbSlider.setBounds(60, 20, 200, 20);
    speedProbSlider.setBounds(60, 40, 200, 20);
    subDivProbSlider.setBounds(60, 60, 200, 20);
    ampProbSlider.setBounds(60, 80, 200, 20);
    stepShuffleProbSlider.setBounds(60, 100, 200, 20);
    
    nSlicesSlider.setBounds(60, 130, 200, 20);
    nStepsSlider.setBounds(60, 150, 200, 20);
    
    speedChangeBox.setBounds(260, 100, 40, 20);
    
    openButton.setBounds(100, 180, 100, 20);
    playButton.setBounds(100, 200, 100, 20);
    randomAllButton.setBounds(100, 240, 100, 20);
    randomOnLoopButton.setBounds(200, 240, 100, 20);
    hostSyncButton.setBounds(200, 260, 100, 20);
}
