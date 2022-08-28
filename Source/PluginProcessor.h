/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
//#include <time.h>
//#include <math.h>
#include "/Users/simonfay/Programming_Stuff/sjf_audio/sjf_sampler.h"

//#define PI 3.14159265


//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
class Sjf_manglerAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    Sjf_manglerAudioProcessor();
    ~Sjf_manglerAudioProcessor() override;

    
    void loadButtonClicked () { sampleMangler.loadSample() ; };
    void playButtonClicked (bool playFlag) { sampleMangler.m_canPlayFlag = playFlag; };
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void checkParameters();
    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    
public:
    sjf_sampler sampleMangler;
    
private:
    juce::AudioPlayHead* playHead;
    juce::AudioPlayHead::PositionInfo positionInfo;
    
    juce::AudioProcessorValueTreeState parameters;
    
    std::atomic<float>* revParameter = nullptr;
    std::atomic<float>* divParameter = nullptr;
    std::atomic<float>* speedParameter = nullptr;
    std::atomic<float>* ampParameter = nullptr;
    std::atomic<float>* shuffleParameter = nullptr;
    
    std::atomic<float>* nSlicesParameter = nullptr;
    std::atomic<float>* nStepsParameter = nullptr;
    std::atomic<float>* fadeParameter = nullptr;

    std::atomic<float>* randOnLoopParameter = nullptr;
    std::atomic<float>* syncToHostParameter = nullptr;
    std::atomic<float>* phaseRateMultiplierParameter = nullptr;
    std::atomic<float>* interpolationTypeParameter = nullptr;
    
    std::atomic<float>* playStateParameter = nullptr;
    
    juce::Value filePathParameter;
//    std::atomic<int>*
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sjf_manglerAudioProcessor)
};
