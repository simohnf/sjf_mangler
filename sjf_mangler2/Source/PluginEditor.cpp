/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin editor.
 
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"
#define textHeight 20
#define potSize 80
#define indent 10

#define WIDTH potSize*5+indent*2
#define HEIGHT potSize*3+textHeight*3 + indent
//==============================================================================
Sjf_Mangler2AudioProcessorEditor::Sjf_Mangler2AudioProcessorEditor (Sjf_Mangler2AudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
: AudioProcessorEditor (&p), valueTreeState (vts), audioProcessor (p)
{
    //    getLookAndFeel().setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey.withAlpha(0.2f));
    //    getLookAndFeel().setColour(juce::ComboBox::backgroundColourId, juce::Colours::darkgrey.withAlpha(0.2f));
    
    
    
    setLookAndFeel( &otherLookAndFeel );
    
    
    addAndMakeVisible (revProbSlider);
    revProbAttachment.reset(new SliderAttachment (valueTreeState, "revProb", revProbSlider));
    revProbSlider.setSliderStyle( juce::Slider::LinearBar );
    revProbSlider.setTextValueSuffix ("%");
    revProbSlider.setTooltip("This sets the likelyhood of a step being played in reverse");
    revProbSlider.sendLookAndFeelChange();
    //    addAndMakeVisible (revProbLabel);
    //    revProbLabel.setText ("reverse", juce::dontSendNotification);
    //    revProbLabel.attachToComponent (&revProbSlider, true);
    //    revProbLabel.setTooltip("This sets the likelyhood of a step being played in reverse");
    
    addAndMakeVisible (speedProbSlider);
    speedProbAttachment.reset(new SliderAttachment (valueTreeState, "speedProb", speedProbSlider));
    speedProbSlider.setSliderStyle( juce::Slider::LinearBar );
    speedProbSlider.setTextValueSuffix ("%");
    speedProbSlider.setTooltip("This sets the likelyhood of a step being played at a different speed and pitch");
    speedProbSlider.sendLookAndFeelChange();
    //    addAndMakeVisible (speedProbLabel);
    //    speedProbLabel.setText ("speed", juce::dontSendNotification);
    //    speedProbLabel.attachToComponent (&speedProbSlider, true);
    //    speedProbLabel.setTooltip("This sets the likelyhood of a step being played at a different speed and pitch");
    
    addAndMakeVisible (subDivProbSlider);
    subDivProbAttachment.reset(new SliderAttachment (valueTreeState, "divProb", subDivProbSlider));
    subDivProbSlider.setSliderStyle( juce::Slider::LinearBar );
    subDivProbSlider.setTextValueSuffix ("%");
    subDivProbSlider.setTooltip("This sets the likelyhood of a slice being subdivided (e.g. half as long) and for the first of these subdivisions being repeated with an increasing/decreasing amplitude ramp");
    subDivProbSlider.sendLookAndFeelChange();
    //    addAndMakeVisible (subDivProbLabel);
    //    subDivProbLabel.setText ("divisions", juce::dontSendNotification);
    //    subDivProbLabel.attachToComponent (&subDivProbSlider, true);
    //    subDivProbLabel.setTooltip("This sets the likelyhood of a slice being subdivided (e.g. half as long) and for the first of these subdivisions being repeated with an increasing/decreasing amplitude ramp");
    
    addAndMakeVisible (ampProbSlider);
    ampProbAttachment.reset(new SliderAttachment (valueTreeState, "ampProb", ampProbSlider));
    ampProbSlider.setSliderStyle( juce::Slider::LinearBar );
    ampProbSlider.setTextValueSuffix ("%");
    ampProbSlider.setTooltip("This sets the likelyhood of a step having a lower than normal amplitude");
    ampProbSlider.sendLookAndFeelChange();
    //    addAndMakeVisible (ampProbLabel);
    //    ampProbLabel.setText ("amplitude", juce::dontSendNotification);
    //    ampProbLabel.attachToComponent (&ampProbSlider, true);
    //    ampProbLabel.setTooltip("This sets the likelyhood of a step having a lower than normal amplitude");
    
    addAndMakeVisible (stepShuffleProbSlider);
    shuffleProbAttachment.reset(new SliderAttachment (valueTreeState, "shuffleProb", stepShuffleProbSlider));
    stepShuffleProbSlider.setSliderStyle( juce::Slider::LinearBar );
    stepShuffleProbSlider.setTextValueSuffix ("%");
    stepShuffleProbSlider.setTooltip("This sets the likelyhood of a different slice being played at any given step --> e.g. instead of playing the first slice being played on the first step the  2nd/3rd/last/etc. slice might be played instead");
    stepShuffleProbSlider.sendLookAndFeelChange();
    //    addAndMakeVisible (stepShuffleProbLabel);
    //    stepShuffleProbLabel.setText ("shuffle", juce::dontSendNotification);
    //    stepShuffleProbLabel.attachToComponent (&stepShuffleProbSlider, true);
    //    stepShuffleProbLabel.setTooltip("This sets the likelyhood of a different slice being played at any given step --> e.g. instead of playing the first slice being played on the first step the  2nd/3rd/last/etc. slice might be played instead");
    
    
    addAndMakeVisible( &sampleChoiceSlider );
    sampleChoiceSliderAttachment.reset(new SliderAttachment (valueTreeState, "sampleChoiceProb", sampleChoiceSlider));
    sampleChoiceSlider.setSliderStyle( juce::Slider::LinearBar );
    sampleChoiceSlider.setTextValueSuffix ("%");
    sampleChoiceSlider.setTooltip("This sets the likelyhood of a choosing the second sample");
    sampleChoiceSlider.sendLookAndFeelChange();
    
    addAndMakeVisible (loadButton);
    loadButton.setButtonText ("load sample");
    loadButton.onClick = [this]
    {
        int currentVoice = voiceComboBox.getSelectedId() > 0 ? voiceComboBox.getSelectedId()-1 : 0 ;
        audioProcessor.loadButtonClicked( currentVoice ) ;
    };
    loadButton.setTooltip("This allows you to select and load a new audio sample (NOTE: it will stop the playback)");
    //    loadButton.setLookAndFeel( &otherLookAndFeel );
    
    addAndMakeVisible (playButton);
    playButtonAttachment.reset(new ButtonAttachment(valueTreeState, "play", playButton));
    playButton.setButtonText ("play");
    playButton.onClick = [this] { audioProcessor.playButtonClicked( playButton.getToggleState() ) ;};
    playButton.setTooltip("This starts the audio sample playback");
    //    playButton.setLookAndFeel(&otherLookAndFeel);
    //    playButton.setLookAndFeel( &otherLookAndFeel );
    
    addAndMakeVisible (hostSyncButton);
    hostSyncAttachment.reset(new ButtonAttachment(valueTreeState, "syncToHost", hostSyncButton));
    hostSyncButton.setButtonText ("sync");
    hostSyncButton.setTooltip("This will synchronise the playback to the host's BPM and playback position");
    //    hostSyncButton.setLookAndFeel(&otherLookAndFeel);
    //    hostSyncButton.setLookAndFeel( &otherLookAndFeel );
    
    
    addAndMakeVisible (randomAllButton);
    randomAllButton.setButtonText ("random");
    randomAllButton.onClick = [this] { audioProcessor.sampleMangler2.randomiseAll() ;};
    randomAllButton.setTooltip("This will generate a new pattern of random variations based on the variation settings chosen)");
    randomAllButton.setLookAndFeel( &otherLookAndFeel );
    
    addAndMakeVisible (randomOnLoopButton);
    randomOnLoopAttachment.reset(new ButtonAttachment(valueTreeState, "randomOnLoop", randomOnLoopButton));
    randomOnLoopButton.setButtonText ("randomise on loop");
    randomOnLoopButton.setTooltip("This will generate a new pattern of random variations (based on the variation settings chosen) everytime the pattern loops");
    //    randomOnLoopButton.setLookAndFeel(&otherLookAndFeel);
    //    randomOnLoopButton.setLookAndFeel( &otherLookAndFeel );
    
    
    
    
    addAndMakeVisible (nSlicesNumBox);
    //    nSlicesAttachment.reset(new SliderAttachment (valueTreeState, "numSlices", nSlicesNumBox));
    juce::Range< double > sliceRange( 1 , 1024 );
    nSlicesNumBox.setRange( sliceRange, 1 );
    nSlicesNumBox.onValueChange = [this]
    {
        
        int currentVoice = std::max( voiceComboBox.getSelectedId() -1, 0 );
        audioProcessor.sampleMangler2.setNumSlices( nSlicesNumBox.getValue(), currentVoice );
        audioProcessor.writeSampleInfoToXML();
        DBG("SLIDER CHANGED " << currentVoice );
    };

    nSlicesNumBox.setTooltip("This determines the number of divisions the audio sample is cut up into. More slices means more divisions, therefore shorter variations. Fewer slices means longer divisions and therefore longer variations");
    nSlicesNumBox.sendLookAndFeelChange();
    
    addAndMakeVisible (nStepsNumBox);
    nStepsAttachment.reset(new SliderAttachment (valueTreeState, "numSteps", nStepsNumBox));
    //    addAndMakeVisible (nStepsLabel);
    //    nStepsLabel.setText ("steps", juce::dontSendNotification);
    //    nStepsLabel.attachToComponent (&nStepsNumBox, true);
    nStepsNumBox.setTooltip("This determines the number of slices the device will read through before going back to the beginning. \ne.g. If you choose 5 steps, the pattern will be 5 slices long.\n If you choose more steps than there are slices the pattern will loop for that many steps. e.g. if you have 8 slices and 10 steps, with no variations set the device will read the 8 slices, then read the first 2 slices again, and then go back to the start. ");
    nStepsNumBox.sendLookAndFeelChange();
    
    addAndMakeVisible (fadeLenNumBox);
    fadeLenAttachment.reset(new SliderAttachment (valueTreeState, "fade", fadeLenNumBox));
    fadeLenNumBox.setTextValueSuffix ("ms");
    //    addAndMakeVisible (fadeLenLabel);
    //    fadeLenLabel.setText ("fade", juce::dontSendNotification);
    //    fadeLenLabel.attachToComponent (&fadeLenNumBox, true);
    fadeLenNumBox.setTooltip("This sets the length of the fade at the beginning and end of each playback segment");
    fadeLenNumBox.sendLookAndFeelChange();
    
    addAndMakeVisible(phaseRateMultiplierBox);
    phaseRateMultiplierBox.addItem("*1/4", 1);
    phaseRateMultiplierBox.addItem("*1/2", 2);
    phaseRateMultiplierBox.addItem("*1", 3);
    phaseRateMultiplierBox.addItem("*2", 4);
    phaseRateMultiplierBox.addItem("*4", 5);
//    phaseRateMultiplierAttachment.reset(new ComboBoxAttachment(valueTreeState, "phaseRateMultiplier", phaseRateMultiplierBox ));
//    phaseRateMultiplierBox.setSelectedId( 3 );
    phaseRateMultiplierBox.onChange = [ this ]
    {
        audioProcessor.sampleMangler2.setPhaseRateMultiplierIndex( phaseRateMultiplierBox.getSelectedId(), std::max( voiceComboBox.getSelectedId()-1, 0 ) );
    };
    phaseRateMultiplierBox.setTooltip("This alters the playback speed");
    //    phaseRateMultiplierBox.setLookAndFeel( &otherLookAndFeel );
    
    addAndMakeVisible(interpolationTypeBox);
    interpolationTypeBox.addItem("linear", 1);
    interpolationTypeBox.addItem("cubic", 2);
    interpolationTypeBox.addItem("PD", 3);
    interpolationTypeBox.addItem("optimal", 4);
    interpolationTypeBox.addItem("godot", 5);
    interpolationTypeBox.addItem("hermite", 6);
    interpolationTypeAttachment.reset(new ComboBoxAttachment(valueTreeState, "interpolationType", interpolationTypeBox));
    interpolationTypeBox.setTooltip("This changes between different interpolation types... it might make a difference to sound quality, or it might not...");
    //    interpolationTypeBox.setLookAndFeel( &otherLookAndFeel );
    
    addAndMakeVisible(&tooltipsToggle);
    tooltipsToggle.setButtonText("Hints");
    tooltipsToggle.onStateChange = [this]
    {
        if (tooltipsToggle.getToggleState())
        {
            //            tooltipWindow.getObject().setAlpha(1.0f);
            tooltipLabel.setVisible( true );
            setSize (WIDTH, HEIGHT+tooltipLabel.getHeight());
        }
        else
        {
            tooltipLabel.setVisible( false );
            setSize (WIDTH, HEIGHT);
            //            tooltipWindow.getObject().setAlpha(0.0f);
        }
    };
    //    tooltipWindow.getObject().setAlpha(0.0f);
    //    tooltipsToggle.setLookAndFeel(&otherLookAndFeel);
    tooltipsToggle.setTooltip(MAIN_TOOLTIP);
    
    //    addAndMakeVisible(sampleNameLabel);
    //    sampleNameLabel.setText(audioProcessor.sampleMangler2.getFileName(), juce::dontSendNotification);
    //    sampleNameLabel.setJustificationType( juce::Justification::centred );
    //    sampleNameLabel.setTooltip(MAIN_TOOLTIP);
    
    addAndMakeVisible(&tooltipLabel);
    tooltipLabel.setVisible( false );
    tooltipLabel.setColour( juce::Label::backgroundColourId, otherLookAndFeel.backGroundColour.withAlpha( 0.85f ) );
    tooltipLabel.setTooltip(MAIN_TOOLTIP);
    
    
    addAndMakeVisible( &voiceComboBox );
    voiceComboBox.onChange = [ this ]
    {
        DBG("Combo box change " << std::max( voiceComboBox.getSelectedId() - 1, 0 ) ) ;
        int voiceNumber = std::max( voiceComboBox.getSelectedId() - 1, 0 );
        nSlicesNumBox.setValue( audioProcessor.sampleMangler2.getNumSlices( voiceNumber ) );
        phaseRateMultiplierBox.setSelectedId( audioProcessor.sampleMangler2.getPhaseRateMultiplierIndex( voiceNumber ) );
    };
    
    
    addAndMakeVisible( &readSampleInfoButton );
    readSampleInfoButton.setButtonText( "Sample Info" );
    readSampleInfoButton.setTooltip( "If you have loaded a sample in a previous session you can use this to load automatically load the last number of slices you set for this file" );
    readSampleInfoButton.onClick = [ this ]
    {
        int voiceNumber = std::max( voiceComboBox.getSelectedId() - 1, 0 );
        audioProcessor.readSampleInfoFromXML( voiceNumber );
        nSlicesNumBox.setValue( audioProcessor.sampleMangler2.getNumSlices( voiceNumber ) );
    };
    
    
    startTimer(500);
    
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize ( WIDTH, HEIGHT);
}

Sjf_Mangler2AudioProcessorEditor::~Sjf_Mangler2AudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}


//==============================================================================

void Sjf_Mangler2AudioProcessorEditor::timerCallback()
{
    //    sampleNameLabel.setText(audioProcessor.sampleMangler2.getFileName(), juce::dontSendNotification);
    //    auto const pos = juce::Desktop::getInstance().getMainMouseSource().getScreenPosition();
    //    tooltipWindow->hideTip();
    
    sjf_setTooltipLabel( this, MAIN_TOOLTIP, tooltipLabel );
    
//    voiceComboBox.clear();
    auto nItems = voiceComboBox.getNumItems();
    auto selected = std::max( voiceComboBox.getSelectedId(), 1 ) ;
    for ( int v = 0; v < audioProcessor.sampleMangler2.getNumVoices(); v++ )
    {
        juce::String filename = audioProcessor.sampleMangler2.getFileName( v ).isNotEmpty() ? audioProcessor.sampleMangler2.getFileName( v ) : "No sample Chosen";
        if (v+1 <= nItems) { voiceComboBox.changeItemText( v+1, filename ); }
        else { voiceComboBox.addItem( filename, v+1 ); }
    }
    voiceComboBox.setSelectedId( selected );
    
}


void Sjf_Mangler2AudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
//    juce::Rectangle<int> r = { WIDTH, HEIGHT + tooltipLabel.getHeight() };
//    sjf_makeBackground< 40 >( g, r );
    
    //    sjf_drawBackgroundImage( g, m_backgroundImage, getWidth(), getHeight() );
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText("sjf_mangler v2.0", 0, 0, getWidth(), textHeight, juce::Justification::centred, 1);
    
    g.drawFittedText("reverse", 0, revProbSlider.getY(), revProbSlider.getX(), textHeight, juce::Justification::right, 1 );
    g.drawFittedText("repeat", 0, subDivProbSlider.getY(), subDivProbSlider.getX(), textHeight, juce::Justification::right, 1 );
    g.drawFittedText("speed", 0, speedProbSlider.getY(), speedProbSlider.getX(), textHeight, juce::Justification::right, 1 );
    g.drawFittedText("amplitude", 0, ampProbSlider.getY(), ampProbSlider.getX(), textHeight, juce::Justification::right, 1 );
    g.drawFittedText("shuffle", 0, stepShuffleProbSlider.getY(), stepShuffleProbSlider.getX(), textHeight, juce::Justification::right, 1 );
    g.drawFittedText("sample", 0, sampleChoiceSlider.getY(), sampleChoiceSlider.getX(), textHeight, juce::Justification::right, 1 );
    
    g.drawFittedText("nSteps", 0, nStepsNumBox.getY(), nStepsNumBox.getX(), textHeight, juce::Justification::right, 1 );
    g.drawFittedText("fade", 0, fadeLenNumBox.getY(), fadeLenNumBox.getX(), textHeight, juce::Justification::right, 1 );
//    g.drawFittedText("interpolation", 0, interpolationTypeBox.getY(), interpolationTypeBox.getX(), textHeight, juce::Justification::right, 1 );
    
    g.drawFittedText("nSlices", 0, nSlicesNumBox.getY(), nSlicesNumBox.getX(), textHeight, juce::Justification::right, 1 );
    
    g.setColour( otherLookAndFeel.outlineColour );
    g.drawLine( voiceComboBox.getX(), voiceComboBox.getBottom(), voiceComboBox.getX(), nSlicesNumBox.getBottom() );
    g.drawLine( voiceComboBox.getX(), nSlicesNumBox.getBottom(), nSlicesNumBox.getX(), nSlicesNumBox.getBottom() );
    
    g.setColour( otherLookAndFeel.backGroundColour.withAlpha( 0.7f ) );
    g.fillRect( voiceComboBox.getX(), voiceComboBox.getY(), voiceComboBox.getWidth() + loadButton.getWidth(), textHeight*2 );
    //    g.drawFittedText(audioProcessor.sampleMangler2.getFileName(), indent, interpolationTypeBox.getBottom(), getWidth(), textHeight, juce::Justification::centred, 1 );
    
}

void Sjf_Mangler2AudioProcessorEditor::resized()
{
    //    auto leftIndent = 65.0f;
    //    auto topIndent = 40.0f;
    //    auto sliderHeight = 20.0f;
    //    auto buttonSize = 20.0f;
    auto slideLength = potSize*4;
    //    auto sliderInternalOffset = 8.0f;
    
    revProbSlider.setBounds(potSize, textHeight, slideLength, textHeight);
    speedProbSlider.setBounds(revProbSlider.getX(), revProbSlider.getBottom(), slideLength, textHeight);
    subDivProbSlider.setBounds(speedProbSlider.getX(), speedProbSlider.getBottom(), slideLength, textHeight);
    ampProbSlider.setBounds(subDivProbSlider.getX(), subDivProbSlider.getBottom(), slideLength, textHeight);
    stepShuffleProbSlider.setBounds(ampProbSlider.getX(), ampProbSlider.getBottom(), slideLength, textHeight);
    sampleChoiceSlider.setBounds(stepShuffleProbSlider.getX(), stepShuffleProbSlider.getBottom(), slideLength, textHeight);
    
    randomAllButton.setBounds(sampleChoiceSlider.getX(), sampleChoiceSlider.getBottom() + indent, potSize, textHeight*2);
    randomOnLoopButton.setBounds(randomAllButton.getRight() + indent, randomAllButton.getBounds().getY(), potSize*3 - indent, textHeight*2);
    
    
    nStepsNumBox.setBounds(randomAllButton.getX(), randomAllButton.getBottom(), potSize, textHeight);
    fadeLenNumBox.setBounds(nStepsNumBox.getX(), nStepsNumBox.getBottom(), potSize, textHeight);
    interpolationTypeBox.setBounds(fadeLenNumBox.getX(), fadeLenNumBox.getBottom(), potSize, textHeight);
    
    playButton.setBounds( randomOnLoopButton.getX(), randomOnLoopButton.getBottom(), randomOnLoopButton.getWidth()/2.0f, textHeight*3);
    hostSyncButton.setBounds( playButton.getRight(), playButton.getY(), playButton.getWidth(), playButton.getHeight() );
    
    voiceComboBox.setBounds( indent, interpolationTypeBox.getBottom() + indent, hostSyncButton.getRight() - indent - 2*potSize, textHeight );
    loadButton.setBounds( voiceComboBox.getRight(), voiceComboBox.getY(), potSize*2, textHeight );
    nSlicesNumBox.setBounds(randomAllButton.getX(), voiceComboBox.getBottom(), potSize, textHeight);
    phaseRateMultiplierBox.setBounds( nSlicesNumBox.getRight(), nSlicesNumBox.getY(), nSlicesNumBox.getWidth(), textHeight );
    readSampleInfoButton.setBounds( phaseRateMultiplierBox.getRight(), phaseRateMultiplierBox.getY(), phaseRateMultiplierBox.getWidth(), textHeight );
    
    tooltipsToggle.setBounds( readSampleInfoButton.getRight(), readSampleInfoButton.getY(), phaseRateMultiplierBox.getWidth(), textHeight );
    //    sampleNameLabel.setBounds( indent, interpolationTypeBox.getBottom(), getWidth(), textHeight);
    tooltipLabel.setBounds( 0, HEIGHT, getWidth(), textHeight*6);
}
