/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>
//#include <time.h>
//#include <math.h>
#include "/Users/simonfay/Programming_Stuff/sjf_audio/sjf_samplerPoly.h"

//#define PI 3.14159265


//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
class Sjf_Mangler2AudioProcessor  : public juce::AudioProcessor
#if JucePlugin_Enable_ARA
, public juce::AudioProcessorARAExtension
#endif
{
public:
    //==============================================================================
    Sjf_Mangler2AudioProcessor();
    ~Sjf_Mangler2AudioProcessor() override;
    
    
    void loadButtonClicked ( const int voiceNumber );// { sampleMangler2.loadSample( voiceNumber ) ; };
    void playButtonClicked (bool playFlag) { sampleMangler2.m_canPlayFlag = playFlag; };
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
    
    void setNumVoices( const int nVoices );
    
    void writeSampleInfoToXML();
    void readSampleInfoFromXML( const int voiceNumber );
    void loadFolderOfSamples ();
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
public:
    sjf_samplerPoly sampleMangler2;
//    int m_nVoices = 2;
    
private:
    juce::AudioFormatManager m_formatManager;
    std::unique_ptr<juce::FileChooser> m_chooser;
    
    juce::AudioPlayHead* playHead;
    juce::AudioPlayHead::PositionInfo positionInfo;
    
    juce::AudioProcessorValueTreeState parameters;
    
    std::atomic<float>* revParameter = nullptr;
    std::atomic<float>* divParameter = nullptr;
    std::atomic<float>* speedParameter = nullptr;
    std::atomic<float>* ampParameter = nullptr;
    std::atomic<float>* shuffleParameter = nullptr;
    std::atomic<float>* sampleChoiceParameter = nullptr;
    
    
    std::atomic<float>* nStepsParameter = nullptr;
    std::atomic<float>* fadeParameter = nullptr;
    
    std::atomic<float>* randOnLoopParameter = nullptr;
    std::atomic<float>* syncToHostParameter = nullptr;
    std::atomic<float>* phaseRateMultiplierParameter = nullptr;
    std::atomic<float>* interpolationTypeParameter = nullptr;
    
    std::atomic<float>* playStateParameter = nullptr;
    
//    std::atomic<float>* nVoicesParameter = nullptr;
    juce::Value nVoicesParameter;
    std::vector< juce::Value > filePathParameter;
    std::vector< juce::Value > nSlicesParameter;
    std::vector< juce::Value > sampleChoiceProbabilitiesParameter;
//    std::vector< juce::Value > phaseRateMultiplierParameter;
//    std::vector< std::atomic<float>* > nSlicesParameter;
    //    std::atomic<int>*
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sjf_Mangler2AudioProcessor)
};
