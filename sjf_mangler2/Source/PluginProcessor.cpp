/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
Sjf_Mangler2AudioProcessor::Sjf_Mangler2AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
: AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                  .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
                  .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
                  )
#endif
, parameters(*this, nullptr, juce::Identifier("sjf_Mangler2"), createParameterLayout() )//, m_nVoices( 2 )
{
    DBG("Started construction");
    revParameter = parameters.getRawParameterValue("revProb");
    speedParameter = parameters.getRawParameterValue("speedProb");
    divParameter = parameters.getRawParameterValue("divProb");
    ampParameter = parameters.getRawParameterValue("ampProb");;
    shuffleParameter = parameters.getRawParameterValue("shuffleProb");
    sampleChoiceParameter = parameters.getRawParameterValue("sampleChoiceProb");
    
    nStepsParameter = parameters.getRawParameterValue("numSteps");
    fadeParameter = parameters.getRawParameterValue("fade");
    
    randOnLoopParameter = parameters.getRawParameterValue("randomOnLoop");
    syncToHostParameter = parameters.getRawParameterValue("syncToHost");
    playStateParameter = parameters.getRawParameterValue("play");
    
//    phaseRateMultiplierParameter = parameters.getRawParameterValue("phaseRateMultiplier");
    
    interpolationTypeParameter = parameters.getRawParameterValue("interpolationType");
    
    
    nVoicesParameter = parameters.getRawParameterValue( "nVoices" );
    DBG("Starting with nVoices");
//    *nVoicesParameter = m_nVoices;
    DBG("setting num voices in mangler");
    setNumVoices( *nVoicesParameter );
    
    for ( int v = 0; v < *nVoicesParameter; v++ )
    {
        filePathParameter[ v ] = parameters.state.getPropertyAsValue("sampleFilePath" + juce::String( v ), nullptr, true);
        nSlicesParameter[ v ] = parameters.state.getPropertyAsValue( "numSlices"+juce::String( v ), nullptr, true );
        phaseRateMultiplierParameter[ v ] = parameters.state.getPropertyAsValue( "phaseRate"+juce::String( v ), nullptr, true );
    }
//    nSlicesParameter = parameters.state.getPropertyAsValue("numSlices", nullptr, true );
    
    sampleMangler2.initialise( getSampleRate() );
    DBG("Ended construction");
}

Sjf_Mangler2AudioProcessor::~Sjf_Mangler2AudioProcessor()
{
}

//==============================================================================
const juce::String Sjf_Mangler2AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Sjf_Mangler2AudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool Sjf_Mangler2AudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool Sjf_Mangler2AudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double Sjf_Mangler2AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Sjf_Mangler2AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int Sjf_Mangler2AudioProcessor::getCurrentProgram()
{
    return 0;
}

void Sjf_Mangler2AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Sjf_Mangler2AudioProcessor::getProgramName (int index)
{
    return {};
}

void Sjf_Mangler2AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Sjf_Mangler2AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sampleMangler2.initialise( getSampleRate() );
}

void Sjf_Mangler2AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Sjf_Mangler2AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif
    
    return true;
#endif
}
#endif

void Sjf_Mangler2AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    
    checkParameters();
    
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    if (sampleMangler2.m_canPlayFlag)
    {
        if (!sampleMangler2.m_syncToHostFlag){ sampleMangler2.play(buffer); }
        else{
            playHead = this->getPlayHead();
            if (playHead != nullptr)
            {
                positionInfo = *playHead->getPosition();
                if( positionInfo.getBpm() )
                {
                    if ( positionInfo.getIsPlaying())
                    {
                        auto pos = *positionInfo.getPpqPosition();
                        auto bpm = *positionInfo.getBpm();
                        sampleMangler2.play(buffer, bpm, pos);
                    }
                }
                else { sampleMangler2.play(buffer); }
            }
            else { sampleMangler2.play(buffer); }
        }
    }
}

//==============================================================================
bool Sjf_Mangler2AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Sjf_Mangler2AudioProcessor::createEditor()
{
    return new Sjf_Mangler2AudioProcessorEditor (*this, parameters);
}

//==============================================================================
void Sjf_Mangler2AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    setNumVoices( *nVoicesParameter );
    for ( int v = 0; v < *nVoicesParameter; v++ )
    {
        filePathParameter[ v ].setValue( sampleMangler2.getFilePath( v ) );
        nSlicesParameter[ v ].setValue( sampleMangler2.getNumSlices( v ) );
        phaseRateMultiplierParameter[ v ].setValue( sampleMangler2.getPhaseRateMultiplierIndex( v ) );
    }
    
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}
//==============================================================================
void Sjf_Mangler2AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
            setNumVoices( *nVoicesParameter );
            for ( int v = 0; v < *nVoicesParameter; v++ )
            {
                filePathParameter[ v ].referTo( parameters.state.getPropertyAsValue("sampleFilePath"+juce::String( v ), nullptr ) );
                if (filePathParameter[ v ] != juce::Value{})
                {
                    sampleMangler2.loadSample( filePathParameter[ v ], v );
                }
                nSlicesParameter[ v ].referTo( parameters.state.getPropertyAsValue("numSlices"+juce::String( v ), nullptr ) );
                int slices = nSlicesParameter[ v ].getValue();
                sampleMangler2.setNumSlices( slices, v );
                phaseRateMultiplierParameter[ v ].referTo( parameters.state.getPropertyAsValue("phaseRate"+juce::String( v ), nullptr ) );
                int phaseRate = phaseRateMultiplierParameter[ v ].getValue();
                sampleMangler2.setPhaseRateMultiplierIndex( phaseRate , v );
                
            }
        }
}
//==============================================================================

void Sjf_Mangler2AudioProcessor::checkParameters()
{
    if (sampleMangler2.m_revProb != *revParameter){
        sampleMangler2.m_revProb = *revParameter;
        sampleMangler2.m_revFlag = true;
    }
    if (sampleMangler2.m_speedProb != *speedParameter){
        sampleMangler2.m_speedProb = *speedParameter;
        sampleMangler2.m_speedFlag = true;
    }
    if (sampleMangler2.m_subDivProb != *divParameter){
        sampleMangler2.m_subDivProb = *divParameter;
        sampleMangler2.m_subDivFlag = true;
    }
    if (sampleMangler2.m_ampProb != *ampParameter){
        sampleMangler2.m_ampProb = *ampParameter;
        sampleMangler2.m_ampFlag = true;
    }
    if (sampleMangler2.m_stepShuffleProb != *shuffleParameter){
        sampleMangler2.m_stepShuffleProb = *shuffleParameter;
        sampleMangler2.m_stepShuffleFlag = true;
    }
    if (sampleMangler2.m_sampleChoiceProb != *sampleChoiceParameter){
        sampleMangler2.m_sampleChoiceProb = *sampleChoiceParameter;
        sampleMangler2.m_sampleChoiceFlag = true;
    }
//    for ( int v = 0; v < *nVoicesParameter; v++ )
//    {
//        if (sampleMangler2.getNumSlices( v ) != nSlicesParameter[ v ] )
//        {
//            sampleMangler2.setNumSlices( nSlicesParameter[ v ], v );
//        }
//    }
    
    if (sampleMangler2.getNumSteps() != *nStepsParameter){
        sampleMangler2.setNumSteps( *nStepsParameter );
    }
    if (sampleMangler2.getFadeInMs() != *fadeParameter){
        sampleMangler2.setFadeLenMs( *fadeParameter );
    }
    if (sampleMangler2.m_randomOnLoopFlag != *randOnLoopParameter){
        sampleMangler2.m_randomOnLoopFlag = *randOnLoopParameter;
    }
    if (sampleMangler2.m_syncToHostFlag != *syncToHostParameter){
        sampleMangler2.m_syncToHostFlag = *syncToHostParameter;
    }
//    if (sampleMangler2.getPhaseRateMultiplierIndex() != *phaseRateMultiplierParameter){
//        sampleMangler2.setPhaseRateMultiplierIndex(*phaseRateMultiplierParameter);
//    }
    if (sampleMangler2.m_canPlayFlag != *playStateParameter){
        sampleMangler2.m_canPlayFlag = *playStateParameter;
    }
    if (sampleMangler2.m_interpolationType != *interpolationTypeParameter){
        sampleMangler2.m_interpolationType = *interpolationTypeParameter;
    }
}


void Sjf_Mangler2AudioProcessor::setNumVoices( const int nVoices )
{
    *nVoicesParameter = nVoices;
//    nVoicesParameter->setValue( nVoices );
    sampleMangler2.setNumVoices( nVoices );
    filePathParameter.resize( nVoices );
    nSlicesParameter.resize( nVoices );
    phaseRateMultiplierParameter.resize( nVoices );
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout Sjf_Mangler2AudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout params;
    
    static constexpr int pIDVersionNumber = 1;
    
    params.add( std::make_unique<juce::AudioParameterFloat> ( juce::ParameterID{ "revProb", pIDVersionNumber }, "Rev", 0.0f, 100.0f, 0.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> ( juce::ParameterID{ "speedProb", pIDVersionNumber }, "Speed", 0.0f, 100.0f, 0.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> ( juce::ParameterID{ "divProb", pIDVersionNumber }, "Div", 0.0f, 100.0f, 0.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> ( juce::ParameterID{ "ampProb", pIDVersionNumber }, "Amp", 0.0f, 100.0f, 0.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> ( juce::ParameterID{ "shuffleProb", pIDVersionNumber }, "Shuffle", 0.0f, 100.0f, 0.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> ( juce::ParameterID{ "sampleChoiceProb", pIDVersionNumber }, "SampleChoiceProb", 0.0f, 100.0f, 0.0f ) );
    params.add( std::make_unique<juce::AudioParameterInt> ( juce::ParameterID{ "numSteps", pIDVersionNumber }, "NumSteps", 4, 1024, 16) );
    params.add( std::make_unique<juce::AudioParameterBool> ( juce::ParameterID{ "randomOnLoop", pIDVersionNumber }, "RandomOnLoop", true) );
    params.add( std::make_unique<juce::AudioParameterBool> ( juce::ParameterID{ "play", pIDVersionNumber }, "Play", false) );
    params.add( std::make_unique<juce::AudioParameterBool> ( juce::ParameterID{ "syncToHost", pIDVersionNumber }, "SyncToHost", true) );
//    params.add( std::make_unique<juce::AudioParameterInt> ( juce::ParameterID{ "phaseRateMultiplier", pIDVersionNumber }, "PhaseRateMultiplier", 1, 5, 3) );
    params.add( std::make_unique<juce::AudioParameterFloat> ( juce::ParameterID{ "fade", pIDVersionNumber }, "Fade", 0.01, 100, 1) );
    params.add( std::make_unique<juce::AudioParameterInt> ( juce::ParameterID{ "interpolationType", pIDVersionNumber }, "InterpolationType", 1, 6, 2) );
    
    params.add( std::make_unique<juce::AudioParameterInt> ( juce::ParameterID{ "nVoices", pIDVersionNumber }, "NVoices", 1, 32, 2) );
    
    return params;
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Sjf_Mangler2AudioProcessor();
}



