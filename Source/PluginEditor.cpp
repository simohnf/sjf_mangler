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
    openButton.setTooltip("This allows you to select a new audio sample (NOTE: it will stop the playback)");
    
    addAndMakeVisible (playButton);
    playButton.setButtonText ("Play");
    playButton.onClick = [this] { audioProcessor.playButtonClicked( playButton.getToggleState() ) ;};
    playButton.setTooltip("This starts the audio sample playback");
    
    addAndMakeVisible (randomAllButton);
    randomAllButton.setButtonText ("Randomise All");
    randomAllButton.onClick = [this] { audioProcessor.sampleMangler.randomiseAll() ;};
    randomAllButton.setTooltip("This will generate a new pattern of random variations (based on the variation settings chosen)");
    
    addAndMakeVisible (randomOnLoopButton);
    randomOnLoopAttachment.reset(new ButtonAttachment(valueTreeState, "randomOnLoop", randomOnLoopButton));
    randomOnLoopButton.setButtonText ("Randomise All On Loop");
    randomOnLoopButton.setTooltip("This will generate a new pattern of random variations (based on the variation settings chosen) everytime the pattern loops");
    
    addAndMakeVisible (hostSyncButton);
    hostSyncAttachment.reset(new ButtonAttachment(valueTreeState, "syncToHost", hostSyncButton));
    hostSyncButton.setButtonText ("Sync");
    hostSyncButton.setTooltip("This will synchronise the playback to the host's BPM and playback position");
    
    addAndMakeVisible (revProbSlider);
    revProbAttachment.reset(new SliderAttachment (valueTreeState, "revProb", revProbSlider));
    revProbSlider.setTooltip("This sets the likelyhood of a step being played in reverse");
    
    addAndMakeVisible (revProbLabel);
    revProbLabel.setText ("RevProb", juce::dontSendNotification);
    revProbLabel.attachToComponent (&revProbSlider, true);
    
    addAndMakeVisible (speedProbSlider);
    speedProbAttachment.reset(new SliderAttachment (valueTreeState, "speedProb", speedProbSlider));
    speedProbSlider.setTooltip("This sets the likelyhood of a step being played at a different speed and pitch");
    
    addAndMakeVisible (speedProbLabel);
    speedProbLabel.setText ("SpeedProb", juce::dontSendNotification);
    speedProbLabel.attachToComponent (&speedProbSlider, true);
    
    addAndMakeVisible (subDivProbSlider);
    subDivProbAttachment.reset(new SliderAttachment (valueTreeState, "divProb", subDivProbSlider));
    subDivProbSlider.setTooltip("This sets the likelyhood of a slice being subdivided (e.g. half as long) and for the first of these subdivisions being repeated with an increasing/decreasing amplitude ramp");
    
    addAndMakeVisible (subDivProbLabel);
    subDivProbLabel.setText ("subDivProb", juce::dontSendNotification);
    subDivProbLabel.attachToComponent (&subDivProbSlider, true);
    
    addAndMakeVisible (ampProbSlider);
    ampProbAttachment.reset(new SliderAttachment (valueTreeState, "ampProb", ampProbSlider));
    ampProbSlider.setTooltip("This sets the likelyhood of a step having a lower than normal amplitude");
    
    addAndMakeVisible (ampProbLabel);
    ampProbLabel.setText ("ampProb", juce::dontSendNotification);
    ampProbLabel.attachToComponent (&ampProbSlider, true);
    
    addAndMakeVisible (stepShuffleProbSlider);
    shuffleProbAttachment.reset(new SliderAttachment (valueTreeState, "shuffleProb", stepShuffleProbSlider));
    stepShuffleProbSlider.setTooltip("This sets the likelyhood of a different slice being played at any given step --> e.g. instead of playing the first slice being played on the first step the  2nd/3rd/last/etc. slice might be played instead");
    
    addAndMakeVisible (stepShuffleProbLabel);
    stepShuffleProbLabel.setText ("shuffleProb", juce::dontSendNotification);
    stepShuffleProbLabel.attachToComponent (&stepShuffleProbSlider, true);
    
    
    addAndMakeVisible (nSlicesSlider);
//    nSlicesSlider.setValue(audioProcessor.sampleMangler.getNumSlices());
    nSlicesAttachment.reset(new SliderAttachment (valueTreeState, "numSlices", nSlicesSlider));
    nSlicesSlider.setTooltip("This determines the number of divisions the audio sample is cut up into. \n\nMore slices means more divisions, therefore shorter variations. Fewer slices means longer divisions ad therefore longer variations");
    
    addAndMakeVisible (nSlicesLabel);
    nSlicesLabel.setText ("slices", juce::dontSendNotification);
    nSlicesLabel.attachToComponent (&nSlicesSlider, true);
    
    addAndMakeVisible (nStepsSlider);
//    nStepsSlider.setValue(audioProcessor.sampleMangler.getNumSteps());
    nStepsAttachment.reset(new SliderAttachment (valueTreeState, "numSteps", nStepsSlider));
    nStepsSlider.setTooltip("This determines the number of slices th device will read through before going back to the beginning. \n\n e.g. If you choose 5 steps, the pattern will be 5 slices long.\n If you choose more steps than there are slices the pattern will loop for that many steps. e.g. if you have 8 slices and 10 steps, with no variations set the device will read the 8 slices, then read the first 2 slices again, and then go back to the start. ");
    
    addAndMakeVisible (nStepsLabel);
    nStepsLabel.setText ("steps", juce::dontSendNotification);
    nStepsLabel.attachToComponent (&nStepsSlider, true);
    
    addAndMakeVisible (fadeLenSlider);
    fadeLenAttachment.reset(new SliderAttachment (valueTreeState, "fade", fadeLenSlider));
    fadeLenSlider.setTooltip("This sets the length of the fade at the beginning and end of each playback segment");
    
    addAndMakeVisible (fadeLenLabel);
    fadeLenLabel.setText ("fade", juce::dontSendNotification);
    fadeLenLabel.attachToComponent (&fadeLenSlider, true);
    
    addAndMakeVisible(phaseRateMultiplierBox);
    phaseRateMultiplierBox.addItem("*1/4", 1);
    phaseRateMultiplierBox.addItem("*1/2", 2);
    phaseRateMultiplierBox.addItem("*1", 3);
    phaseRateMultiplierBox.addItem("*2", 4);
    phaseRateMultiplierBox.addItem("*4", 5);
//    phaseRateMultiplierBox.setSelectedId(3);
    phaseRateMultiplierAttachment.reset(new ComboBoxAttachment(valueTreeState, "phaseRateMultiplier", phaseRateMultiplierBox ));
    phaseRateMultiplierBox.setTooltip("This alters the playback speed");
    
    
    
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
    revProbSlider.setBounds(60, 20, 325, 20);
    speedProbSlider.setBounds(60, 40, 325, 20);
    subDivProbSlider.setBounds(60, 60, 325, 20);
    ampProbSlider.setBounds(60, 80, 325, 20);
    stepShuffleProbSlider.setBounds(60, 100, 325, 20);
    
    randomAllButton.setBounds(60, 120, 100, 20);
    randomOnLoopButton.setBounds(160, 120, 100, 20);
    
    
    nSlicesSlider.setBounds(60, 160, 200, 20);
    nStepsSlider.setBounds(60, 180, 200, 20);
    fadeLenSlider.setBounds(60, 200, 200, 20);
    
    
    
    playButton.setBounds(255, 160, 100, 20);
    hostSyncButton.setBounds(320, 160, 100, 20);
    phaseRateMultiplierBox.setBounds(255, 180, 60, 20);
    
    openButton.setBounds(getWidth()/2-50, getHeight()-30, 100, 20);

    
}
