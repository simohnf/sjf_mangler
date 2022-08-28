/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
Sjf_manglerAudioProcessor::Sjf_manglerAudioProcessor()
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
, parameters(*this, nullptr, juce::Identifier("sjf_mangler"),
             {
                 std::make_unique<juce::AudioParameterFloat> ("revProb", "Rev", 0.0f, 100.0f, 0.0f),
                 std::make_unique<juce::AudioParameterFloat> ("speedProb", "Speed", 0.0f, 100.0f, 0.0f),
                 std::make_unique<juce::AudioParameterFloat> ("divProb", "Div", 0.0f, 100.0f, 0.0f),
                 std::make_unique<juce::AudioParameterFloat> ("ampProb", "Amp", 0.0f, 100.0f, 0.0f),
                 std::make_unique<juce::AudioParameterFloat> ("shuffleProb", "Shuffle", 0.0f, 100.0f, 0.0f),
                 std::make_unique<juce::AudioParameterInt> ("numSteps", "NumSteps", 4, 1024, 16),
                 std::make_unique<juce::AudioParameterInt> ("numSlices", "NumSlices", 4, 1024, 16),
                 std::make_unique<juce::AudioParameterBool> ("randomOnLoop", "RandomOnLoop", true),
                 std::make_unique<juce::AudioParameterBool> ("play", "Play", true),
                 std::make_unique<juce::AudioParameterBool> ("syncToHost", "SyncToHost", true),
                 std::make_unique<juce::AudioParameterInt> ("phaseRateMultiplier", "PhaseRateMultiplier", 1, 5, 3),
                 std::make_unique<juce::AudioParameterFloat> ("fade", "Fade", 0.01, 100, 1),
                 std::make_unique<juce::AudioParameterInt> ("interpolationType", "InterpolationType", 1, 6, 2)
             })
{
    revParameter = parameters.getRawParameterValue("revProb");
    speedParameter = parameters.getRawParameterValue("speedProb");
    divParameter = parameters.getRawParameterValue("divProb");
    ampParameter = parameters.getRawParameterValue("ampProb");;
    shuffleParameter = parameters.getRawParameterValue("shuffleProb");
    
    nSlicesParameter = parameters.getRawParameterValue("numSlices");
    nStepsParameter = parameters.getRawParameterValue("numSteps");
    fadeParameter = parameters.getRawParameterValue("fade");
    
    randOnLoopParameter = parameters.getRawParameterValue("randomOnLoop");
    syncToHostParameter = parameters.getRawParameterValue("syncToHost");
    playStateParameter = parameters.getRawParameterValue("play");
    
    phaseRateMultiplierParameter = parameters.getRawParameterValue("phaseRateMultiplier");
    
    interpolationTypeParameter = parameters.getRawParameterValue("interpolationType");
    
    filePathParameter = parameters.state.getPropertyAsValue("sampleFilePath", nullptr, true);
    
    sampleMangler.initialise( getSampleRate() );
}

Sjf_manglerAudioProcessor::~Sjf_manglerAudioProcessor()
{
}

//==============================================================================
const juce::String Sjf_manglerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Sjf_manglerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Sjf_manglerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Sjf_manglerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Sjf_manglerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Sjf_manglerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Sjf_manglerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Sjf_manglerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Sjf_manglerAudioProcessor::getProgramName (int index)
{
    return {};
}

void Sjf_manglerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Sjf_manglerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sampleMangler.initialise( getSampleRate() );
}

void Sjf_manglerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Sjf_manglerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void Sjf_manglerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    
    checkParameters();
    
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    if (sampleMangler.m_canPlayFlag)
    {
        if (!sampleMangler.m_syncToHostFlag){ sampleMangler.play(buffer); }
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
                        sampleMangler.play(buffer, bpm, pos);
                    }
                }
                else { sampleMangler.play(buffer); }
            }
            else { sampleMangler.play(buffer); }
        }
    }
}

//==============================================================================
bool Sjf_manglerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Sjf_manglerAudioProcessor::createEditor()
{
    return new Sjf_manglerAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void Sjf_manglerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    filePathParameter.setValue(sampleMangler.m_samplePath.getFullPathName());
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void Sjf_manglerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType())){
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
            filePathParameter.referTo(parameters.state.getPropertyAsValue("sampleFilePath", nullptr) );
            if (filePathParameter == juce::Value{}){ return; }
            sampleMangler.loadSample( filePathParameter );
            
        }
    
}


void Sjf_manglerAudioProcessor::checkParameters()
{
    if (sampleMangler.m_revProb != *revParameter){
        sampleMangler.m_revProb = *revParameter;
        sampleMangler.m_revFlag = true;
    }
    if (sampleMangler.m_speedProb != *speedParameter){
        sampleMangler.m_speedProb = *speedParameter;
        sampleMangler.m_speedFlag = true;
    }
    if (sampleMangler.m_subDivProb != *divParameter){
        sampleMangler.m_subDivProb = *divParameter;
        sampleMangler.m_subDivFlag = true;
    }
    if (sampleMangler.m_ampProb != *ampParameter){
        sampleMangler.m_ampProb = *ampParameter;
        sampleMangler.m_ampFlag = true;
    }
    if (sampleMangler.m_stepShuffleProb != *shuffleParameter){
        sampleMangler.m_stepShuffleProb = *shuffleParameter;
        sampleMangler.m_stepShuffleFlag = true;
    }
    if (sampleMangler.getNumSlices() != *nSlicesParameter){
        sampleMangler.setNumSlices( *nSlicesParameter );
    }
    if (sampleMangler.getNumSteps() != *nStepsParameter){
        sampleMangler.setNumSteps( *nStepsParameter );
    }
    if (sampleMangler.getFadeInMs() != *fadeParameter){
        sampleMangler.setFadeLenMs( *fadeParameter );
    }
    if (sampleMangler.m_randomOnLoopFlag != *randOnLoopParameter){
        sampleMangler.m_randomOnLoopFlag = *randOnLoopParameter;
    }
    if (sampleMangler.m_syncToHostFlag != *syncToHostParameter){
        sampleMangler.m_syncToHostFlag = *syncToHostParameter;
    }
    if (sampleMangler.getPhaseRateMultiplierIndex() != *phaseRateMultiplierParameter){
        sampleMangler.setPhaseRateMultiplierIndex(*phaseRateMultiplierParameter);
    }
    if (sampleMangler.m_canPlayFlag != *playStateParameter){
        sampleMangler.m_canPlayFlag = *playStateParameter;
    }
    if (sampleMangler.m_interpolationType != *interpolationTypeParameter){
        sampleMangler.m_interpolationType = *interpolationTypeParameter;
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Sjf_manglerAudioProcessor();
}




