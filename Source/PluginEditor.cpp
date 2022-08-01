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
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void Sjf_manglerAudioProcessorEditor::resized()
{
    openButton.setBounds(100, 100, 100, 20);
    playButton.setBounds(100, 140, 100, 20);
    stopButton.setBounds(100, 180, 100, 20);
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
