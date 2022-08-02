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
                 std::make_unique<juce::AudioParameterInt> ("numSteps", "NumSteps", 4, 256, 16),
                 std::make_unique<juce::AudioParameterInt> ("numSlices", "NumSlices", 4, 256, 16),
                 std::make_unique<juce::AudioParameterBool> ("randomOnLoop", "RandomOnLoop", false),
                 std::make_unique<juce::AudioParameterBool> ("syncToHost", "SyncToHost", false)
             })
{
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
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());


    

    playHead = this->getPlayHead();
    if (playHead != nullptr){
        positionInfo = *playHead->getPosition();
        if(positionInfo.getBpm()){
            float bpm = *positionInfo.getBpm();
            if (sampleMangler.canPlay){ sampleMangler.syncToHost(bpm); }
            if (positionInfo.getIsPlaying())
            {
                auto pos = *positionInfo.getPpqPosition();
                pos *= sampleMangler.sampleDivNoteValue;
                int posInt = (int)pos;
                auto leftOver = pos - posInt;
                posInt %=sampleMangler.nSteps;
                pos = (posInt + leftOver) / sampleMangler.nSteps;
                sampleMangler.phaseRamp.setPhase(pos);
            }
        }
    }
    if (sampleMangler.canPlay){ sampleMangler.play(buffer); }
    
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
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void Sjf_manglerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Sjf_manglerAudioProcessor();
}


