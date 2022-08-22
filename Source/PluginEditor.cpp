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
    addAndMakeVisible (loadButton);
    loadButton.setButtonText ("Load\nAudio\nSample");
    loadButton.onClick = [this] { audioProcessor.loadButtonClicked() ; };
    loadButton.setTooltip("This allows you to select and load a new audio sample (NOTE: it will stop the playback)");
    
    addAndMakeVisible (playButton);
    playButtonAttachment.reset(new ButtonAttachment(valueTreeState, "play", playButton));
    playButton.setButtonText ("Play");
    playButton.onClick = [this] { audioProcessor.playButtonClicked( playButton.getToggleState() ) ;};
    playButton.setTooltip("This starts the audio sample playback");
    playButton.setLookAndFeel(&otherLookAndFeel);
    
    
    addAndMakeVisible (hostSyncButton);
    hostSyncAttachment.reset(new ButtonAttachment(valueTreeState, "syncToHost", hostSyncButton));
    hostSyncButton.setButtonText ("Sync");
    hostSyncButton.setTooltip("This will synchronise the playback to the host's BPM and playback position");
    hostSyncButton.setLookAndFeel(&otherLookAndFeel);
    
    
    
    addAndMakeVisible (randomAllButton);
    randomAllButton.setButtonText ("Random");
    randomAllButton.onClick = [this] { audioProcessor.sampleMangler.randomiseAll() ;};
    randomAllButton.setTooltip("This will generate a new pattern of random variations (based on the variation settings chosen)");
    
    addAndMakeVisible (randomOnLoopButton);
    randomOnLoopAttachment.reset(new ButtonAttachment(valueTreeState, "randomOnLoop", randomOnLoopButton));
    randomOnLoopButton.setButtonText ("Randomise On Loop");
    randomOnLoopButton.setTooltip("This will generate a new pattern of random variations (based on the variation settings chosen) everytime the pattern loops");
    randomOnLoopButton.setLookAndFeel(&otherLookAndFeel);
    
    
    addAndMakeVisible (revProbSlider);
    revProbAttachment.reset(new SliderAttachment (valueTreeState, "revProb", revProbSlider));
    revProbSlider.setTextValueSuffix ("%");
    revProbSlider.setTooltip("This sets the likelyhood of a step being played in reverse");
    addAndMakeVisible (revProbLabel);
    revProbLabel.setText ("RevProb", juce::dontSendNotification);
    revProbLabel.attachToComponent (&revProbSlider, true);
    revProbLabel.setTooltip("This sets the likelyhood of a step being played in reverse");
    
    addAndMakeVisible (speedProbSlider);
    speedProbAttachment.reset(new SliderAttachment (valueTreeState, "speedProb", speedProbSlider));
    speedProbSlider.setTextValueSuffix ("%");
    speedProbSlider.setTooltip("This sets the likelyhood of a step being played at a different speed and pitch");
    addAndMakeVisible (speedProbLabel);
    speedProbLabel.setText ("SpeedProb", juce::dontSendNotification);
    speedProbLabel.attachToComponent (&speedProbSlider, true);
    speedProbLabel.setTooltip("This sets the likelyhood of a step being played at a different speed and pitch");
    
    addAndMakeVisible (subDivProbSlider);
    subDivProbAttachment.reset(new SliderAttachment (valueTreeState, "divProb", subDivProbSlider));
    subDivProbSlider.setTextValueSuffix ("%");
    subDivProbSlider.setTooltip("This sets the likelyhood of a slice being subdivided (e.g. half as long) and for the first of these subdivisions being repeated with an increasing/decreasing amplitude ramp");
    addAndMakeVisible (subDivProbLabel);
    subDivProbLabel.setText ("subDivProb", juce::dontSendNotification);
    subDivProbLabel.attachToComponent (&subDivProbSlider, true);
    subDivProbLabel.setTooltip("This sets the likelyhood of a slice being subdivided (e.g. half as long) and for the first of these subdivisions being repeated with an increasing/decreasing amplitude ramp");
    
    addAndMakeVisible (ampProbSlider);
    ampProbAttachment.reset(new SliderAttachment (valueTreeState, "ampProb", ampProbSlider));
    ampProbSlider.setTextValueSuffix ("%");
    ampProbSlider.setTooltip("This sets the likelyhood of a step having a lower than normal amplitude");
    addAndMakeVisible (ampProbLabel);
    ampProbLabel.setText ("ampProb", juce::dontSendNotification);
    ampProbLabel.attachToComponent (&ampProbSlider, true);
    ampProbLabel.setTooltip("This sets the likelyhood of a step having a lower than normal amplitude");
    
    addAndMakeVisible (stepShuffleProbSlider);
    shuffleProbAttachment.reset(new SliderAttachment (valueTreeState, "shuffleProb", stepShuffleProbSlider));
    stepShuffleProbSlider.setTextValueSuffix ("%");
    stepShuffleProbSlider.setTooltip("This sets the likelyhood of a different slice being played at any given step --> e.g. instead of playing the first slice being played on the first step the  2nd/3rd/last/etc. slice might be played instead");
    addAndMakeVisible (stepShuffleProbLabel);
    stepShuffleProbLabel.setText ("shuffleProb", juce::dontSendNotification);
    stepShuffleProbLabel.attachToComponent (&stepShuffleProbSlider, true);
    stepShuffleProbLabel.setTooltip("This sets the likelyhood of a different slice being played at any given step --> e.g. instead of playing the first slice being played on the first step the  2nd/3rd/last/etc. slice might be played instead");
    
    
    addAndMakeVisible (nSlicesNumBox);
    nSlicesAttachment.reset(new SliderAttachment (valueTreeState, "numSlices", nSlicesNumBox));
    addAndMakeVisible (nSlicesLabel);
    nSlicesLabel.setText ("slices", juce::dontSendNotification);
    nSlicesLabel.attachToComponent (&nSlicesNumBox, true);
    nSlicesLabel.setTooltip("This determines the number of divisions the audio sample is cut up into. \n\nMore slices means more divisions, therefore shorter variations. Fewer slices means longer divisions ad therefore longer variations");
    
    addAndMakeVisible (nStepsNumBox);
    nStepsAttachment.reset(new SliderAttachment (valueTreeState, "numSteps", nStepsNumBox));
    addAndMakeVisible (nStepsLabel);
    nStepsLabel.setText ("steps", juce::dontSendNotification);
    nStepsLabel.attachToComponent (&nStepsNumBox, true);
    nStepsLabel.setTooltip("This determines the number of slices the device will read through before going back to the beginning. \n\n e.g. If you choose 5 steps, the pattern will be 5 slices long.\n If you choose more steps than there are slices the pattern will loop for that many steps. e.g. if you have 8 slices and 10 steps, with no variations set the device will read the 8 slices, then read the first 2 slices again, and then go back to the start. ");    
    
    addAndMakeVisible (fadeLenNumBox);
    fadeLenAttachment.reset(new SliderAttachment (valueTreeState, "fade", fadeLenNumBox));
    fadeLenNumBox.setTextValueSuffix ("ms");
    addAndMakeVisible (fadeLenLabel);
    fadeLenLabel.setText ("fade", juce::dontSendNotification);
    fadeLenLabel.attachToComponent (&fadeLenNumBox, true);
    fadeLenLabel.setTooltip("This sets the length of the fade at the beginning and end of each playback segment");
    
    addAndMakeVisible(phaseRateMultiplierBox);
    phaseRateMultiplierBox.addItem("*1/4", 1);
    phaseRateMultiplierBox.addItem("*1/2", 2);
    phaseRateMultiplierBox.addItem("*1", 3);
    phaseRateMultiplierBox.addItem("*2", 4);
    phaseRateMultiplierBox.addItem("*4", 5);
    phaseRateMultiplierAttachment.reset(new ComboBoxAttachment(valueTreeState, "phaseRateMultiplier", phaseRateMultiplierBox ));
    phaseRateMultiplierBox.setTooltip("This alters the playback speed");
    
    addAndMakeVisible(interpolationTypeBox);
    interpolationTypeBox.addItem("linear", 1);
    interpolationTypeBox.addItem("cubic", 2);
    interpolationTypeBox.addItem("PD", 3);
    interpolationTypeBox.addItem("optimal", 4);
    interpolationTypeBox.addItem("godot", 5);
    interpolationTypeBox.addItem("Hermite", 6);
    interpolationTypeAttachment.reset(new ComboBoxAttachment(valueTreeState, "interpolationType", interpolationTypeBox));
    interpolationTypeBox.setTooltip("This changes between different interpolation types... it might make a difference to sound quality, or it might not...");
    

    addAndMakeVisible(sampleNameLabel);
    sampleNameLabel.setText(audioProcessor.sampleMangler.samplePath.getFileName(), juce::dontSendNotification);
    startTimer(500);
    
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

Sjf_manglerAudioProcessorEditor::~Sjf_manglerAudioProcessorEditor()
{
}


//==============================================================================

void Sjf_manglerAudioProcessorEditor::timerCallback()
{
    sampleNameLabel.setText(audioProcessor.sampleMangler.samplePath.getFileName(), juce::dontSendNotification);
}


void Sjf_manglerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText("sjf_mangler", (getWidth()/2.0f) - 50, 0, 100, 20, juce::Justification::centred, 1);
}

void Sjf_manglerAudioProcessorEditor::resized()
{
    auto leftIndent = 65.0f;
    auto topIndent = 40.0f;
    auto sliderHeight = 20.0f;
    auto buttonSize = 20.0f;
    auto slideLength = 330.0f;
    auto sliderInternalOffset = 8.0f;
    
    revProbSlider.setBounds(leftIndent, topIndent, slideLength, sliderHeight);
    speedProbSlider.setBounds(leftIndent, topIndent + 1*sliderHeight, slideLength, sliderHeight);
    subDivProbSlider.setBounds(leftIndent, topIndent + 2*sliderHeight, slideLength, sliderHeight);
    ampProbSlider.setBounds(leftIndent, topIndent + 3*sliderHeight, slideLength, sliderHeight);
    stepShuffleProbSlider.setBounds(leftIndent, topIndent + 4*sliderHeight, slideLength, sliderHeight);
    
    
    randomAllButton.setBounds(leftIndent, stepShuffleProbSlider.getBounds().getY()+stepShuffleProbSlider.getBounds().getHeight(), buttonSize*4, sliderHeight*2);
    randomOnLoopButton.setBounds(randomAllButton.getBounds().getX() + randomAllButton.getBounds().getWidth() + sliderInternalOffset, randomAllButton.getBounds().getY(), stepShuffleProbSlider.getBounds().getWidth() - randomAllButton.getBounds().getWidth() - 2*sliderInternalOffset, sliderHeight*2);
    
    
    nSlicesNumBox.setBounds(leftIndent, topIndent + 8*sliderHeight, sliderHeight*4, sliderHeight);
    nStepsNumBox.setBounds(leftIndent, topIndent + 9*sliderHeight, sliderHeight*4, sliderHeight);
    fadeLenNumBox.setBounds(leftIndent, topIndent + 10*sliderHeight, sliderHeight*4, sliderHeight);
    
    
    
    playButton.setBounds(nSlicesNumBox.getBounds().getX() + nSlicesNumBox.getBounds().getWidth() + sliderHeight, nSlicesNumBox.getBounds().getY(), sliderHeight*4, sliderHeight*4);
    hostSyncButton.setBounds(playButton.getBounds().getX() + playButton.getBounds().getWidth(), playButton.getBounds().getY(), sliderHeight*3, sliderHeight*3);
    phaseRateMultiplierBox.setBounds(hostSyncButton.getBounds().getX(), hostSyncButton.getBounds().getY()+hostSyncButton.getBounds().getHeight(), sliderHeight*3, sliderHeight);
    loadButton.setBounds(hostSyncButton.getBounds().getX()+hostSyncButton.getBounds().getWidth(), hostSyncButton.getBounds().getY(), playButton.getBounds().getWidth(), playButton.getBounds().getHeight());
    
    interpolationTypeBox.setBounds(0, getHeight()-20, 120, 20);
    sampleNameLabel.setBounds(interpolationTypeBox.getBounds().getX()+interpolationTypeBox.getBounds().getWidth(), interpolationTypeBox.getBounds().getY(), getWidth() - sampleNameLabel.getBounds().getX(), 20);
}
