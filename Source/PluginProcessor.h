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

#define PI 3.14159265
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
//    return rampUp+rampDown; // this would give linear fade
    return sin( PI* (rampUp+rampDown)/2 ); // this gives a smooth sinewave based fade
}
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

inline
float cubicInterpolate(std::vector<float> buffer, float read_pos)
{
    auto bufferSize = buffer.size();
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
        y0 = buffer[ bufferSize - 1 ];
    }
    else
    {
        y0 = buffer[ index - 1 ];
    }
    y1 = buffer[ index % bufferSize ];
    y2 = buffer[ (index + 1) % bufferSize ];
    y3 = buffer[ (index + 2) % bufferSize ];
    double a0,a1,a2,a3,mu2;
    
    mu2 = mu*mu;
    a0 = y3 - y2 - y0 + y1;
    a1 = y0 - y1 - a0;
    a2 = y2 - y0;
    a3 = y1;
    
    return (a0*mu*mu2 + a1*mu2 + a2*mu + a3);
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
inline
float rand01()
{
    return float( rand() ) / float( RAND_MAX );
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

    void setPhase(float p)
    {
        if (p < 0) {p = 0;}
        else if (p > 1){ p = 1 ;}
        position = p;
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
    
    int nSteps = 16; int nSlices = 16;
    sjf_phasor phaseRamp;
    
    
    std::vector<float> revPat, speedPat, subDivPat, subDivAmpRampPat, ampPat, stepPat;
    float revProb = 0; float speedProb = 0; float subDivProb = 0; float ampProb = 0; float stepShuffleProb = 0;
    bool canPlay = false; bool randomOnLoopFlag = false; bool syncToHostFlag = false;
    bool revFlag = false; bool speedFlag = false; bool speedRampFlag = true;
    bool subDivFlag = false; bool ampFlag = false; bool stepShuffleFlag = false;
    

    float hostSyncCompenstation = 1.0f;
    float sampleDivNoteValue = 1;
    
    juce::File samplePath;
    
private:
    juce::AudioBuffer<float> AudioSample;
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::FileChooser> chooser;
    
    float fadeInMs = 1;
    float read_pos = 0;
    float duration = 44100;
    float subDivCount = 0.0f;
    float preSubDivAmp = 0;
    float hostBPM = 120;
    float sliceLenSamps;
    int SR = 44100;
    int lastStep = -1;
    float phaseRateMultiplier = 1;
    

    
public:
//==============================================================================
    float getDuration() { return duration; };
//==============================================================================
    void loadSample()
    {
//        bool lastPlayState = canPlay;
        canPlay = false;
            chooser = std::make_unique<juce::FileChooser> ("Select a Wave file to play...",
                                                           juce::File{},
                                                           "*.wav");
            auto chooserFlags = juce::FileBrowserComponent::openMode
            | juce::FileBrowserComponent::canSelectFiles;
            
            chooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& fc)
                                  {
                                      auto file = fc.getResult();
                                      if (file == juce::File{}) { return; }
                                      std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
                                      if (reader.get() != nullptr)
                                      {
                                          bool lastPlayState = canPlay;
                                          canPlay = false;
                                          AudioSample.clear();
                                          AudioSample.setSize((int) reader->numChannels, (int) reader->lengthInSamples);
                                          duration = AudioSample.getNumSamples();
                                          reader->read (&AudioSample, 0, (int) reader->lengthInSamples, 0, true, true);
                                          read_pos = 0;
                                          sliceLenSamps = duration/nSlices;
                                          setPatterns();
                                          samplePath = file;
                                          canPlay = lastPlayState;
                                      }
                                  });
//        canPlay = lastPlayState;
    };
//==============================================================================
    void loadSample(juce::Value path)
    {
        juce::File file( path.getValue().toString() );
        if (file == juce::File{}) { return; }
        std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
        if (reader.get() != nullptr)
        {
            AudioSample.clear();
            AudioSample.setSize((int) reader->numChannels, (int) reader->lengthInSamples);
            duration = AudioSample.getNumSamples();
            reader->read (&AudioSample, 0, (int) reader->lengthInSamples, 0, true, true);
            read_pos = 0;
            sliceLenSamps = duration/nSlices;
            setPatterns();
            samplePath = file;
        }
    };
//==============================================================================
    void setFadeLenMs(float fade)
    {
        fadeInMs = fade;
    };
//==============================================================================
    float getFadeInMs()
    {
        return fadeInMs;
    };
//==============================================================================
    void setNumSlices(int slices){
        nSlices = slices;
        sliceLenSamps = duration/nSlices;
        setPatterns();
    };
//==============================================================================
    int getNumSlices(){ 
        return nSlices;
    };
//==============================================================================
    void setNumSteps(int steps){
        revPat.resize(steps);
        speedPat.resize(steps);
        subDivPat.resize(steps);
        subDivAmpRampPat.resize(steps);
        ampPat.resize(steps);
        stepPat.resize(steps);
        if (steps > nSteps)
        for (int index = nSteps; index < steps; index++)
        {
            revPat[index] = 0;
            speedPat[index] = 0;
            subDivPat[index] = 0;
            subDivAmpRampPat[index] = rand01();
            ampPat[index] = 1.0f;
            stepPat[index] = index % nSlices;
        }
        nSteps = steps;
    };
//==============================================================================
    int getNumSteps(){
        return nSteps;
    };
//==============================================================================
    void setPhaseRateMultiplierIndex(int i)
    {
        phaseRateMultiplier = pow(2, i-3);
        hostSyncCompenstation *= phaseRateMultiplier;
//        sliceLenSamps *= phaseRateMultiplier;
//        nSlices /= phaseRateMultiplier;
    };
//==============================================================================
    int getPhaseRateMultiplierIndex()
    {
        return 3 + log10(phaseRateMultiplier)/log10(2);
    };
//==============================================================================
    void play(juce::AudioBuffer<float> &buffer)
    {
        auto envLen = fadeInMs * SR / 1000; // samples in one millisecond
        auto rampFrequency = 1.0f / ( nSteps * sliceLenSamps/SR );
        rampFrequency *= hostSyncCompenstation;
        phaseRamp.setFrequency( rampFrequency );
        
        auto bufferSize = buffer.getNumSamples();
        auto numChannels = buffer.getNumChannels();
        
        std::vector<float> phaseOut;
        for (int index = 0; index < bufferSize; index++)
        {
            phaseOut.push_back( phaseRamp.output() * nSteps ); // phase Out goes 0 -> nSteps
        }

        for (int index = 0; index < bufferSize; index++)
        {
            auto currentStep = checkForChangeOfBeat( floor(phaseOut[index]) );
            auto subDiv = floor(subDivPat[currentStep] * 8.0f) + 1.0f ;
            auto phaseWrap = calculatePhaseWrap( (phaseOut[index] - currentStep), subDiv );
            auto subDivLenSamps = sliceLenSamps / subDiv;
            auto subDivAmp = calculateSubDivAmp( currentStep, subDiv, subDivCount );
            auto speedVal = calculateSpeedVal( currentStep, phaseOut[index] );
            auto readStep = stepPat[currentStep] ;
            auto pos = calculateReverseAndPosition( currentStep,  readStep,  phaseWrap,  subDivLenSamps,  speedVal);
            auto amp = calculateAmpValue( currentStep );
            for (int channel = 0; channel < numChannels; channel ++)
            {
                auto val = cubicInterpolate(AudioSample, channel % AudioSample.getNumChannels(), pos);
                val *= subDivAmp * amp * phaseEnv(phaseWrap, subDivLenSamps, envLen);
                val *= phaseEnv( (phaseOut[index] - currentStep), sliceLenSamps, 2* envLen); // this extra envelope is just to fade on each step
                buffer.setSample(channel, index, val);
            }
        }
    }; 

//==============================================================================
    void syncToHost(float bpm){
        hostBPM = bpm;
        
        if (!syncToHostFlag) { hostSyncCompenstation = phaseRateMultiplier ; return; }
        
        auto sliceLenSec = sliceLenSamps/SR;
        auto sampleBPM = 60000.0f / sliceLenSec;
        if ( sampleBPM == hostBPM ){
            hostSyncCompenstation = 1;
            sampleDivNoteValue = 1;
            return;
        }
        else if ( sampleBPM < hostBPM )
        {
            while (sampleBPM < hostBPM){ sampleBPM *= 2; }
            float lastDiff = abs(hostBPM - sampleBPM);
            auto halfSampleBPM = sampleBPM * 0.5;
            
            auto newDiff = abs(halfSampleBPM - hostBPM);
            if ( newDiff < lastDiff) { sampleBPM = halfSampleBPM;}
        }
        else
        {
            while (sampleBPM > hostBPM){
                sampleBPM *= 0.5;
            }
            float lastDiff = abs(sampleBPM - hostBPM);
            auto twiceSampleBPM = sampleBPM * 2;
            auto newDiff = abs(hostBPM - twiceSampleBPM);
            if (newDiff < lastDiff ) { sampleBPM = twiceSampleBPM;}
        }
        sampleBPM /= phaseRateMultiplier;
        
        auto sampleQuarterLenSamps = 60.0 * (double)SR/sampleBPM;
        if (sampleQuarterLenSamps > sliceLenSamps)
        {
            sampleDivNoteValue = round( sampleQuarterLenSamps / sliceLenSamps );
        }
        else
        {
            sampleDivNoteValue = round( sliceLenSamps / sampleQuarterLenSamps );
            sampleDivNoteValue = 1.0f / (float)sampleDivNoteValue;
        }
        
        hostSyncCompenstation = hostBPM / ( sampleBPM );
    };
//==============================================================================
    void randomiseAll()
    {
        revFlag = true;
        speedFlag = true;
        subDivFlag = true;
        ampFlag = true;
        stepShuffleFlag = true;
    };
//==============================================================================
private:
    int checkForChangeOfBeat(int currentStep)
    {
        if ( currentStep != lastStep )
        {   // This is just to randomise patterns at the start of each loop
            if (currentStep == 0 && randomOnLoopFlag)
            {
                randomiseAll();
            }
            lastStep = currentStep;
            subDivCount = 0;
            checkPatternRandomisationFlags();
        }
        return currentStep;
    };
//==============================================================================
    float calculatePhaseWrap(float phaseWrap, float subDiv){

        //            SUBDIVISION & TRAIL LOGIC
        phaseWrap *= subDiv;
        if (phaseWrap >= 1)
        {
            int phaseInt = phaseWrap;
            phaseWrap -= phaseInt;
            subDivCount = phaseInt;
        }
        return phaseWrap;
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
        while (pos < 0 ) { pos += duration; }
        while (pos >= duration) { pos -= duration; }
        
        return pos;
    };
//==============================================================================
    float calculateSpeedVal( int currentStep, float stepPhase )
    {
        float speedVal = 1;
        if (!speedRampFlag) { speedVal = speedPat[currentStep] ; }
        else { speedVal = cubicInterpolate(speedPat, stepPhase); }
        speedVal = pow(2, (speedVal*12)/12);
//        if ( abs(speedVal) < 0.5 ) { speedVal = 1; }
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
            revPat[index] = 0;
            speedPat[index] = 0;
            subDivPat[index] = 0;
            subDivAmpRampPat[index] = rand01();
            ampPat[index] = 1.0f;
            stepPat[index] = index % nSlices;
        }
    };
//==============================================================================
    void checkPatternRandomisationFlags(){
        if(revFlag){ setRandRevPat(); revFlag = false;}
        if(speedFlag) { setRandSpeedPat(); speedFlag = false;}
        if(subDivFlag) { setRandSubDivPat(); subDivFlag = false;}
        if(ampFlag) { setRandAmpPat(); ampFlag = false;}
        if(stepShuffleFlag) { setRandStepPat(); stepShuffleFlag = false;}
    };
//==============================================================================
    void setRandRevPat()
    {
        setRandPat(revPat, revProb/100.0f);
    };
//==============================================================================
    void setRandSpeedPat()
    {
        setRandPat(speedPat, speedProb/100.0f);
        
        for (int index = 0; index < nSteps; index++)
        {
            if (rand01() >=0.5) { speedPat[index]  *= -1.0f; }
        }
            
    };
//==============================================================================
    void setRandSubDivPat()
    {
        setRandPat(subDivPat, subDivProb/100.0f);
        for (int index = 0; index < nSteps; index++)
        {
            subDivAmpRampPat[index] = rand01();
        }
    };
//==============================================================================
    void setRandAmpPat()
    {
        setRandPat(ampPat, 1 - pow(ampProb/100.0f, 4));
    };
//==============================================================================
    void setRandStepPat()
    {
        stepPat.resize(nSteps);
        for (int index = 0; index < nSteps; index++)
        {
            if(  stepShuffleProb/100.0f <= rand01() )
            {
                stepPat[index] = index;
            }
            else
            {
                int step = floor(rand01() * nSteps);
                stepPat[index] = step;
            }
        }
    };
//==============================================================================
    void setRandPat(std::vector<float>& pattern, float prob)
    {
        pattern.resize(nSteps);
        auto exponent = (1 - prob) * 20;
        for (int index = 0; index < nSteps; index++)
        {
            if (prob == 0) {pattern[index] = 0;}
            else {pattern[index] = pow( rand01(), exponent );}
        }
    };

//    END OF sjf_sampler CLASS
//==============================================================================
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
//        sampleMangler.canPlay = false;
        sampleMangler.loadSample() ;
    };
    void playButtonClicked (bool play) { sampleMangler.canPlay = play; };
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
    
    juce::Value filePathParameter;
//    std::atomic<int>*
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sjf_manglerAudioProcessor)
};
