/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <time.h>
#include <math.h>

//==============================================================================
/**
*/
//==============================================================================
inline
float phaseEnv( float phase, float period, float envLen){
    auto nSegments = period / envLen;
    auto segmentPhase = phase * nSegments;
    auto rampUp = segmentPhase;
    if (rampUp > 1) {rampUp = 1;}
    else if (rampUp < 0) {rampUp = 0;}
    
    float rampDown = segmentPhase - (nSegments - 1);
    if (rampDown > 1) {rampDown = 1;}
    else if (rampDown < 0) {rampDown = 0;}
    rampDown *= -1;
    
    return rampUp + rampDown;
    
}
//==============================================================================
inline
float cubicInterpolate(juce::AudioBuffer<float>& buffer, int channel, float read_pos)
{
    auto bufferSize = buffer.getNumSamples();
    double y0; // previous step value
    double y1; // this step value
    double y2; // next step value
    double y3; // next next step value
    double mu; // fractional part between step 1 & 2
    
    float findex = read_pos;
    if(findex < 0){ findex+= bufferSize;}
    else if(findex > bufferSize){ findex-= bufferSize;}
    
    int index = findex;
    mu = findex - index;
    
    if (index == 0)
    {
        y0 = buffer.getSample(channel, bufferSize - 1);
    }
    else
    {
        y0 = buffer.getSample(channel, index - 1);
    }
    y1 = buffer.getSample(channel, index % bufferSize);
    y2 = buffer.getSample(channel, (index + 1) % bufferSize);
    y3 = buffer.getSample(channel, (index + 2) % bufferSize);
    double a0,a1,a2,a3,mu2;
    
    mu2 = mu*mu;
    a0 = y3 - y2 - y0 + y1;
    a1 = y0 - y1 - a0;
    a2 = y2 - y0;
    a3 = y1;
    
    return (a0*mu*mu2 + a1*mu2 + a2*mu + a3);
}
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

class sjf_phasor{
public:
    sjf_phasor() { calculateIncrement() ; };
    sjf_phasor(float sample_rate, float f) { initialise(sample_rate, f); };
    
    ~sjf_phasor() {};
    
    void initialise(float sample_rate, float f)
    {
        SR = sample_rate;
        setFrequency( f );
    };
    
    void setSampleRate( float sample_rate)
    {
        SR = sample_rate;
        calculateIncrement();
    }
    void setFrequency(float f)
    {
        frequency = f;
        increment = 1 / (SR / frequency);
    };
    
    float getFrequency(){ return frequency ;};
    
    float output()
    {
        float p = position;
        position += increment;
        if (position >= 1){ position -= 1; }
        return p;
    };

private:
    void calculateIncrement(){ increment = 1 / (SR / frequency); };
    
    float frequency = 440;
    float SR = 44100;
    float increment;
    float position = 0;
};


//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
class sjf_sampler{

public:
    sjf_sampler()
    {
        AudioSample.setSize(2, 44100);
        AudioSample.clear();
        
        formatManager.registerBasicFormats();
    };
    
    ~sjf_sampler(){};
    
    void initialise(int sampleRate){ SR  = sampleRate; phaseRamp.setSampleRate( SR ); srand((unsigned)time(NULL)); };
    
    int nSteps = 16;
    int nSlices = 16;
    bool canPlay = false;
    sjf_phasor phaseRamp;
    
    float getDuration() { return duration; };
    std::vector<float> revPat, speedPat, subDivPat, subDivAmpRampPat, ampPat, stepPat;
    
private:
    juce::AudioBuffer<float> AudioSample;
    std::unique_ptr<juce::FileChooser> chooser;
    juce::AudioFormatManager formatManager;
    float read_pos = 0;
    float duration = 44100;
    int SR = 44100;
    float sliceLenSamps;
    int lastStep = -1;
    float subDivCount = 0.0f;
    float preSubDivAmp = 0;
    
    
    
public:
    void loadSample()
    {
        canPlay = false;
            chooser = std::make_unique<juce::FileChooser> ("Select a Wave file shorter than 2 seconds to play...",
                                                           juce::File{},
                                                           "*.wav");
            auto chooserFlags = juce::FileBrowserComponent::openMode
            | juce::FileBrowserComponent::canSelectFiles;
            
            chooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& fc)
                                  {
                                      auto file = fc.getResult();
                                      
                                      if (file == juce::File{})
                                          return;
                                      
                                      std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
                                      
                                      if (reader.get() != nullptr)
                                      {
                                          AudioSample.clear();
                                          AudioSample.setSize((int) reader->numChannels, (int) reader->lengthInSamples);
                                          duration = AudioSample.getNumSamples();
                                          reader->read (&AudioSample, 0, (int) reader->lengthInSamples, 0, true, true);
                                          read_pos = 0;
                                          sliceLenSamps = duration/nSlices;
                                      }
                                      setPatterns();
                                  });
    };
//==============================================================================
    void play(juce::AudioBuffer<float> &buffer)
    {
        auto envLen = SR / 1000; // samples in one millisecond
        phaseRamp.setFrequency( 1 / ( nSteps * sliceLenSamps/SR ) );
        
        auto bufferSize = buffer.getNumSamples();
        auto numChannels = buffer.getNumChannels();
        
        std::vector<float> phaseOut;
        for (int index = 0; index < bufferSize; index++)
        {
            phaseOut.push_back( phaseRamp.output() * nSteps ); // phase Out goes 0 -> nSteps
        }

        
        for (int index = 0; index < bufferSize; index++)
        {
            int currentStep = floor(phaseOut[index]);

            if ( currentStep != lastStep ){
                lastStep = currentStep;
                subDivCount = 0;
                //            This is just to randomise patterns at the start of each loop
                if (currentStep == 0 )
                {
                    setPatterns();
                }
            }
            
            
            
            float phaseWrap = phaseOut[index] - currentStep;
            
//            SUBDIVISION & TRAIL LOGIC
            float subDiv = floor(subDivPat[currentStep] * 8.0f) + 1.0f ;
            phaseWrap *= subDiv;
            if (phaseWrap >= 1)
            {
                int phaseInt = phaseWrap;
                phaseWrap -= phaseInt;
                subDivCount = phaseInt;
                
            }
            auto subDivLenSamps = sliceLenSamps/ subDiv;
            
            auto subDivAmp = calculateSubDivAmp( currentStep, subDiv, subDivCount );
//            auto subDivAmp = 1.0f;

            auto speedVal = calculateSpeedVal( currentStep );
//            STEP LOGIC
            auto readStep = stepPat[currentStep] ;

            float pos = calculateReverseAndPosition( currentStep,  readStep,  phaseWrap,  subDivLenSamps,  speedVal);
//            AMPLITUDE Pattern LOGIC
            float amp = calculateAmpValue( currentStep );
            
            for (int channel = 0; channel < numChannels; channel ++)
            {
                auto val = cubicInterpolate(AudioSample, channel % AudioSample.getNumChannels(), pos);
                val *= subDivAmp;
                val *= amp;
                val *= phaseEnv(phaseWrap, subDivLenSamps, envLen); // apply amplitude envelope
                buffer.setSample(channel, index, val);
            }
        }
    }; 
//==============================================================================
    float calculateSubDivAmp( int currentStep, float subDiv, float subDivCount )
    {
        auto subDivAmp = 1.0f;
        
        // if subDiv is one, subDiv amp is 1
        if (subDiv == 1)
        {
            subDivAmp = 1.0f;
        }
        // else if subDivPatAmp for current
        else if ( subDivAmpRampPat[currentStep] < 0.5 )
        {
            // start subdivision at max amplitude, drop to a fraction of that
            subDivAmp = 1.0f / (subDivCount + 1.0f);
        }
        else
        {
            // start subdivision at a fraction of max, raise to maximum for last division
            subDivAmp = 1.0f / (subDiv - subDivCount);
        }
        if (preSubDivAmp != subDivAmp){
            preSubDivAmp = subDivAmp;
            DBG( "SUBDIVAMP" << subDivAmp) ;
            DBG("subDiv " << subDiv);
            DBG("subDivCOunt " << subDivCount);
        }
        return subDivAmp;
    };
//==============================================================================

    float calculateAmpValue(int currentStep) { return ampPat[currentStep]; };
//==============================================================================
    float calculateReverseAndPosition(int currentStep, int readStep, float phaseWrap, float subDivLenSamps, float speedVal)
    {
        float pos;
        //            REVERSE LOGIC
        if ( revPat[currentStep] < 0.25 )
        {
            pos =  (readStep + (phaseWrap * speedVal)) * subDivLenSamps;
        }
        else
        {
            pos = ( (readStep + 1) - (phaseWrap * speedVal) ) * subDivLenSamps;
        }
        
        //            JUST RECHECK POSITION FOR SAFETY
        if (pos < 0 ) { pos += duration; }
        else if (pos >= duration) { pos -= duration; }
        
        return pos;
    };
//==============================================================================
    float calculateSpeedVal( int currentStep )
    {
        auto speedVal = speedPat[currentStep] ;
        speedVal *= 5;
        if ( speedVal < 0.5 ) { speedVal = 1; }
        return speedVal;
    };
//==============================================================================
    void setPatterns()
    {
        revPat.resize(nSteps);
        speedPat.resize(nSteps);
        subDivPat.resize(nSteps);
        subDivAmpRampPat.resize(nSteps);
        ampPat.resize(nSteps);
        stepPat.resize(nSteps);
        for (int index = 0; index < nSteps; index++)
        {
            revPat[index] = pow( rand01(), 3 );
//            revPat[index] = 0;
            speedPat[index] = pow( rand01(), 3 );
//            speedPat[index] = 0;
            subDivPat[index] = pow( rand01(), 3 );
//            subDivPat[index] = 0;
            subDivAmpRampPat[index] = rand01();
            ampPat[index] = 1.0f - pow( rand01(), 10 );
//            ampPat[index] = 1.0f;
            stepPat[index] = floor(rand01() * nSteps);
//            stepPat[index] = index;
        }
    }
//==============================================================================
    void play2(juce::AudioBuffer<float> &buffer)
    {
        auto bufferSize = buffer.getNumSamples();
        auto numChannels = buffer.getNumChannels();
        
        
        for (int index = 0; index < bufferSize; index++)
        {
            for (int channel = 0; channel < numChannels; channel ++)
            {
                auto pos = read_pos + index;
                if (pos >= duration) { pos -= duration ; }
                if (pos < 0) { pos += duration; }
                
                auto ch = channel % AudioSample.getNumChannels();
                auto val = AudioSample.getSample( ch, pos );
                
                buffer.setSample(channel, index, val);
            }
        }
        read_pos += bufferSize;
        if (read_pos >= duration) { read_pos -= duration; }
        if (read_pos < 0) { read_pos += duration; }
    };
    
    float rand01() { return float( rand() ) / float( RAND_MAX ); }
};

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

    
    void openButtonClicked ()
    {
        sampleMangler.canPlay = false;
        sampleMangler.loadSample() ;
    };
    void playButtonClicked () { sampleMangler.canPlay = true; ; };
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

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    
private:
        sjf_sampler sampleMangler;
    
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sjf_manglerAudioProcessor)
};
