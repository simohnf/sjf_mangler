/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Sjf_manglerAudioProcessorEditor::Sjf_manglerAudioProcessorEditor (Sjf_manglerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    
    
    addAndMakeVisible (openButton);
    openButton.setButtonText ("Open...");
    openButton.onClick = [this] { audioProcessor.openButtonClicked() ;};
    
    
    addAndMakeVisible (playButton);
    playButton.setButtonText ("Play");
    playButton.onClick = [this] { audioProcessor.playButtonClicked() ;};
    
    addAndMakeVisible (stopButton);
    stopButton.setButtonText ("Stop");
    stopButton.onClick = [this] { audioProcessor.stopButtonClicked() ;};
    
    addAndMakeVisible (randomAllButton);
    randomAllButton.setButtonText ("Randomise All");
    randomAllButton.onClick = [this] { audioProcessor.sampleMangler.randomiseAll() ;};
    
    addAndMakeVisible (randomOnLoopButton);
    randomOnLoopButton.setButtonText ("Randomise All On Loop");
    randomOnLoopButton.onClick = [this] { audioProcessor.sampleMangler.randomOnLoopFlag = randomOnLoopButton.getToggleState(); };
    
    addAndMakeVisible (hostSyncButton);
    hostSyncButton.setButtonText ("Sync To Host");
    hostSyncButton.onClick = [this] { audioProcessor.sampleMangler.syncToHostFlag = hostSyncButton.getToggleState(); };
    
    addAndMakeVisible (revProbSlider);
    revProbSlider.setRange (0.0f, 1.0f);
    revProbSlider.onValueChange = [this] { audioProcessor.sampleMangler.revProb = revProbSlider.getValue(); audioProcessor.sampleMangler.revFlag = true; };
    
    addAndMakeVisible (revProbLabel);
    revProbLabel.setText ("RevProb", juce::dontSendNotification);
    revProbLabel.attachToComponent (&revProbSlider, true);
    
    addAndMakeVisible (speedProbSlider);
    speedProbSlider.setRange (0.0f, 1.0f);
    speedProbSlider.onValueChange = [this] { audioProcessor.sampleMangler.speedProb = speedProbSlider.getValue(); audioProcessor.sampleMangler.speedFlag = true; };
    
    addAndMakeVisible (speedProbLabel);
    speedProbLabel.setText ("SpeedProb", juce::dontSendNotification);
    speedProbLabel.attachToComponent (&speedProbSlider, true);
    
    addAndMakeVisible (subDivProbSlider);
    subDivProbSlider.setRange (0.0f, 1.0f);
    subDivProbSlider.onValueChange = [this] { audioProcessor.sampleMangler.subDivProb = subDivProbSlider.getValue(); audioProcessor.sampleMangler.subDivFlag = true; };
    
    addAndMakeVisible (subDivProbLabel);
    subDivProbLabel.setText ("subDivProb", juce::dontSendNotification);
    subDivProbLabel.attachToComponent (&subDivProbSlider, true);
    
    addAndMakeVisible (ampProbSlider);
    ampProbSlider.setRange (0.0f, 1.0f);
    ampProbSlider.onValueChange = [this] { audioProcessor.sampleMangler.ampProb = ampProbSlider.getValue(); audioProcessor.sampleMangler.ampFlag = true; };
    ampProbSlider.onValueChange = [this] { audioProcessor.sampleMangler.ampProb = ampProbSlider.getValue(); audioProcessor.sampleMangler.ampFlag = true; };
    
    addAndMakeVisible (ampProbLabel);
    ampProbLabel.setText ("ampProb", juce::dontSendNotification);
    ampProbLabel.attachToComponent (&ampProbSlider, true);
    
    addAndMakeVisible (stepShuffleProbSlider);
    stepShuffleProbSlider.setRange (0.0f, 1.0f);
    stepShuffleProbSlider.onValueChange = [this] { audioProcessor.sampleMangler.stepShuffleProb = stepShuffleProbSlider.getValue(); audioProcessor.sampleMangler.stepShuffleFlag = true; };
    
    addAndMakeVisible (stepShuffleProbLabel);
    stepShuffleProbLabel.setText ("shuffleProb", juce::dontSendNotification);
    stepShuffleProbLabel.attachToComponent (&stepShuffleProbSlider, true);
    
    
    
    
    
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
    
    openButton.setBounds(100, 140, 100, 20);
    playButton.setBounds(100, 160, 100, 20);
    stopButton.setBounds(100, 180, 100, 20);
    randomAllButton.setBounds(100, 200, 100, 20);
    randomOnLoopButton.setBounds(200, 200, 100, 20);
    hostSyncButton.setBounds(200, 240, 100, 20);
}
