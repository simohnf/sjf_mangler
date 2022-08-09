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
float fourPointInterpolatePD(juce::AudioBuffer<float>& buffer, int channel, float read_pos)
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
    
    auto y2minusy1 = y2-y1;
    return y1 + mu * (y2minusy1 - 0.1666667f * (1.0f - mu) * ( (y3 - y0 - 3.0f * y2minusy1) * mu + (y3 + 2.0f*y0 - 3.0f*y1) ) );
}
//==============================================================================
inline
float linearInterpolate(juce::AudioBuffer<float>& buffer, int channel, float read_pos)
{
    auto bufferSize = buffer.getNumSamples();
    double y1; // this step value
    double y2; // next step value
    double mu; // fractional part between step 1 & 2
    
    float findex = read_pos;
    if(findex < 0){ findex+= bufferSize;}
    else if(findex > bufferSize){ findex-= bufferSize;}
    
    int index = findex;
    mu = findex - index;
    
    y1 = buffer.getSample(channel, index % bufferSize);
    y2 = buffer.getSample(channel, (index + 1) % bufferSize);
    
    return y1 + mu*(y2-y1) ;
}
//==============================================================================
inline
float fourPointFourthOrderOptimal(juce::AudioBuffer<float>& buffer, int channel, float read_pos)
{
    //    Copied from Olli Niemitalo - Polynomial Interpolators for High-Quality Resampling of Oversampled Audio
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
    
    
    // Optimal 2x (4-point, 4th-order) (z-form)
    float z = mu - 1/2.0;
    float even1 = y2+y1, odd1 = y2-y1;
    float even2 = y3+y0, odd2 = y3-y0;
    float c0 = even1*0.45645918406487612 + even2*0.04354173901996461;
    float c1 = odd1*0.47236675362442071 + odd2*0.17686613581136501;
    float c2 = even1*-0.253674794204558521 + even2*0.25371918651882464;
    float c3 = odd1*-0.37917091811631082 + odd2*0.11952965967158000;
    float c4 = even1*0.04252164479749607 + even2*-0.04289144034653719;
    return (((c4*z+c3)*z+c2)*z+c1)*z+c0;
    
}
//==============================================================================
inline
float cubicInterpolateGodot(juce::AudioBuffer<float>& buffer, int channel, float read_pos)
{
    //    Copied from Olli Niemitalo - Polynomial Interpolators for High-Quality Resampling of Oversampled Audio
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
    
    a0 = 3 * y1 - 3 * y2 + y3 - y0;
    a1 = 2 * y0 - 5 * y1 + 4 * y2 - y3;
    a2 = y2 - y0;
    a3 = 2 * y1;
    
    return (a0 * mu * mu2 + a1 * mu2 + a2 * mu + a3) / 2;
}
//==============================================================================
inline
float cubicInterpolateHermite(juce::AudioBuffer<float>& buffer, int channel, float read_pos)
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
    
    a0 = y1;
    a1 = 0.5f * (y2 - y0);
    a2 = y0 - (2.5f * y1) + (2 * y2) - (0.5f * y3);
    a3 = (0.5f * (y3 - y0)) + (1.5F * (y1 - y2));
    return (((((a3 * mu) + a2) * mu) + a1) * mu) + a0;
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
        AudioSample.clear();
        formatManager.registerBasicFormats();
        setPatterns();
    };
    
    ~sjf_sampler(){};
    
    void initialise(int sampleRate){ SR  = sampleRate; srand((unsigned)time(NULL)); };
    
    int nSteps = 16; int nSlices = 16;
    
    
    std::vector<float> revPat, speedPat, subDivPat, subDivAmpRampPat, ampPat, stepPat;
    float revProb = 0; float speedProb = 0; float subDivProb = 0; float ampProb = 0; float stepShuffleProb = 0;
    bool canPlay = false; bool randomOnLoopFlag = false; bool syncToHostFlag = false;
    bool revFlag = false; bool speedFlag = false; bool speedRampFlag = true;
    bool subDivFlag = false; bool ampFlag = false; bool stepShuffleFlag = false;
    int interpolationType = 0;
    
    juce::File samplePath;
    
private:
    juce::AudioBuffer<float> AudioSample, tempBuffer;
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::FileChooser> chooser;
    
    float fadeInMs = 1;
    float duration = 44100;
    float preSubDivAmp = 0;
    float hostBPM = 120;
    float sliceLenSamps;
    int SR = 44100;
    int lastStep = -1;
    float phaseRateMultiplier = 1;
    bool sampleLoaded = false;
    float read_pos = 0;
    int stepCount = 0;
    

    
public:
    //==============================================================================
    float getDuration() { return duration; };
    //==============================================================================
    void loadSample()
    {
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
                                          tempBuffer.clear();
                                          tempBuffer.setSize((int) reader->numChannels, (int) reader->lengthInSamples);
                                          duration = tempBuffer.getNumSamples();
                                          reader->read (&tempBuffer, 0, (int) reader->lengthInSamples, 0, true, true);
//                                          AudioSample.clear();
                                          AudioSample.makeCopyOf(tempBuffer);
                                          sliceLenSamps = duration/(float)nSlices;
//                                          setPatterns();
                                          samplePath = file;
                                          tempBuffer.setSize(0, 0);
                                          sampleLoaded = true;
                                          canPlay = lastPlayState;
                                      }
                                  });
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
            sliceLenSamps = duration/nSlices;
            setPatterns();
            samplePath = file;
            sampleLoaded = true;
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
    void setNumSlices(int slices)
    {
        nSlices = slices;
        sliceLenSamps = duration/nSlices;
        setPatterns();
    };
    //==============================================================================
    int getNumSlices()
    {
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
    int getNumSteps()
    {
        return nSteps;
    };
    //==============================================================================
    void setPhaseRateMultiplierIndex(int i)
    {
        phaseRateMultiplier = pow(2, i-3);
    };
    //==============================================================================
    int getPhaseRateMultiplierIndex()
    {
        return 3 + log10(phaseRateMultiplier)/log10(2);
    };
    //==============================================================================
    void play(juce::AudioBuffer<float> &buffer)
    {
        if (!sampleLoaded) { return; }
        auto envLen = phaseRateMultiplier * fadeInMs * SR / 1000;
        envLen = round( envLen ) ;
        
        auto bufferSize = buffer.getNumSamples();
        auto numChannels = buffer.getNumChannels();
        
        auto increment = phaseRateMultiplier;
        
        for (int index = 0; index < bufferSize; index++)
        {
            if (read_pos >= sliceLenSamps) {stepCount++; stepCount %= nSteps; }
            while (read_pos >= sliceLenSamps){ read_pos -= sliceLenSamps; }
            
            checkForChangeOfBeat( stepCount );
            auto pos = read_pos;
            auto subDiv = floor(subDivPat[stepCount] * 8.0f) + 1.0f ;
            auto subDivLenSamps = sliceLenSamps / subDiv ;
            auto subDivAmp = calculateSubDivAmp( stepCount, subDiv, int(pos / subDivLenSamps) );
            auto speedVal = calculateSpeedVal( stepCount, (read_pos / sliceLenSamps) );
            while (pos >= subDivLenSamps){ pos -= subDivLenSamps; }
            auto env = envelope( pos , subDivLenSamps, envLen );
            auto amp = calculateAmpValue( stepCount );
            pos = calculateReverse(stepCount, pos, subDivLenSamps);
            pos *= speedVal;
            pos += stepPat[stepCount] * sliceLenSamps;
            
            for (int channel = 0; channel < numChannels; channel ++)
            {
                auto val = calculateSampleValue(AudioSample, channel % AudioSample.getNumChannels(), pos);
                val *= subDivAmp * amp;
                val *= env;
                buffer.setSample(channel, index, val);
            }
            read_pos += increment;
        }
    };
    //==============================================================================
    void play(juce::AudioBuffer<float> &buffer, float bpm, double hostPosition)
    {
        if (!sampleLoaded) { return; }
        auto hostSyncCompenstation =  calculateHostCompensation( bpm );
        auto envLen = hostSyncCompenstation * phaseRateMultiplier * fadeInMs * SR / 1000;
        envLen = round( envLen );
        
        auto bufferSize = buffer.getNumSamples();
        auto numChannels = buffer.getNumChannels();
        
        auto hostSampQuarter = 60.0f*SR/bpm;
        hostPosition *= hostSampQuarter * hostSyncCompenstation * phaseRateMultiplier;
        auto leftOver = hostPosition / sliceLenSamps;
        stepCount = (int)leftOver;
        leftOver = hostPosition - stepCount*sliceLenSamps;
        
        read_pos = leftOver;
        stepCount %= nSteps;
        auto increment = phaseRateMultiplier * hostSyncCompenstation;
        
        for (int index = 0; index < bufferSize; index++)
        {
            if (read_pos >= sliceLenSamps) {stepCount++; stepCount %= nSteps; }
            while (read_pos >= sliceLenSamps){ read_pos -= sliceLenSamps; }
            
            checkForChangeOfBeat( stepCount );
            auto pos = read_pos;
            auto subDiv = floor(subDivPat[stepCount] * 8.0f) + 1.0f ;
            auto subDivLenSamps = sliceLenSamps / subDiv ;
            auto subDivAmp = calculateSubDivAmp( stepCount, subDiv, int(pos / subDivLenSamps) );
            auto speedVal = calculateSpeedVal( stepCount, (read_pos / sliceLenSamps) );
            while (pos >= subDivLenSamps){ pos -= subDivLenSamps; }
            auto env = envelope( pos , subDivLenSamps, envLen ); // Check this out more
            auto amp = calculateAmpValue( stepCount );
            pos = calculateReverse(stepCount, pos, subDivLenSamps);
            pos *= speedVal;
            pos += stepPat[stepCount] * sliceLenSamps;

            for (int channel = 0; channel < numChannels; channel ++)
            {
                auto val = calculateSampleValue(AudioSample, channel % AudioSample.getNumChannels(), pos);
                val *= subDivAmp * amp;
                val *= env;
                buffer.setSample(channel, index, val);
            }
            read_pos +=  increment;
        }
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
    bool checkForChangeOfBeat(int currentStep)
    {
        bool newStep = false;
        if ( currentStep != lastStep )
        {   // This is just to randomise patterns at the start of each loop
            if (currentStep == 0 && randomOnLoopFlag)
            {
                randomiseAll();
            }
            checkPatternRandomisationFlags();
            newStep = true;
        }
        lastStep = currentStep;
        return newStep;
    };
    //==============================================================================
    float envelope( float pos, float period, float envLen)
    {
        if (pos < 0){ pos = 0; }
        else if (pos > period ){ pos = period; }
        auto outVal = 1.0f;
        auto sampsAtEnd = period - envLen;
        if (pos < envLen) { outVal = pos/envLen; }
        else if(pos > sampsAtEnd)
        {
            outVal = 1 - (pos - sampsAtEnd)/envLen;
        }
        //        return outVal; // this would give linear fade
        return sin( PI* (outVal)/2 ); // this gives a smooth sinewave based fade
    };
    //==============================================================================
    float calculatePosition(int stepCount, int readStep, float pos, float subDivLenSamps){
        auto posOut = (readStep * sliceLenSamps) + pos;
        
        //            JUST RECHECK POSITION FOR SAFETY
        while (posOut < 0 ) { posOut += duration; }
        while (posOut >= duration) { posOut -= duration; }
        return posOut;
    }
    //==============================================================================
    float calculateReverse(int stepCount, float pos, float subDivLenSamps)
    {
        //            REVERSE LOGIC
        if ( revPat[stepCount] >= 0.25 )
        {
            pos =  subDivLenSamps - pos;
        }
        return pos;
    };
    //==============================================================================
    float calculateSampleValue(juce::AudioBuffer<float>& buffer, int channel, float pos)
    {
        if (interpolationType < 0) { interpolationType = 0; }
        else if (interpolationType > 5) { interpolationType = 5; }
        switch(interpolationType)
        {
            case 0:
                return linearInterpolate(buffer, channel, pos);
                break;
            case 1:
                return cubicInterpolate(buffer, channel, pos);
                break;
            case 2:
                return fourPointInterpolatePD(buffer, channel, pos);
                break;
            case 3:
                return fourPointFourthOrderOptimal(buffer, channel, pos);
                break;
            case 4:
                return cubicInterpolateGodot(buffer, channel, pos);
                break;
            case 5:
                return cubicInterpolateHermite(buffer, channel, pos);
                break;
        }
    };
    //==============================================================================
    float calculateHostCompensation(float bpm){
        if (!sampleLoaded ) { return 1; }
        hostBPM = bpm;
        auto hostQuarterNoteSamps = SR*60.0f/bpm;
        auto multiplier = 1.0f;
        if (!syncToHostFlag) { return 1; }
        if (sliceLenSamps == hostQuarterNoteSamps){
            return 1;
        }
        else if (sliceLenSamps < hostQuarterNoteSamps){
            while (sliceLenSamps * multiplier < hostQuarterNoteSamps){ multiplier *= 2; }
            auto lastDiff = abs(hostQuarterNoteSamps - sliceLenSamps*multiplier);
            auto halfMultiplier = multiplier * 0.5;
            auto newDiff = abs(hostQuarterNoteSamps - sliceLenSamps*halfMultiplier);
            if ( newDiff < lastDiff) { multiplier = halfMultiplier; }
        }
        else
        {
            while (sliceLenSamps > hostQuarterNoteSamps *multiplier){ multiplier *= 2; }
            float lastDiff = abs(hostQuarterNoteSamps*multiplier - sliceLenSamps);
            auto halfMultiplier = multiplier * 0.5f;
            auto newDiff = abs(hostQuarterNoteSamps*halfMultiplier - sliceLenSamps);
            if (newDiff < lastDiff ) { multiplier = halfMultiplier;}
            multiplier = 1/multiplier;
        }
        return (sliceLenSamps*multiplier) / hostQuarterNoteSamps ;
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
    float calculateSpeedVal( int currentStep, float stepPhase )
    {
        float speedVal = 0;
        if (!speedRampFlag) { speedVal = speedPat[currentStep] ; }
        else { speedVal = 0 + (speedPat[currentStep] * stepPhase) ; }
        speedVal = pow(2, (speedVal*12)/12);
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

    
    void loadButtonClicked () { sampleMangler.loadSample() ; };
    void playButtonClicked (bool playFlag) { sampleMangler.canPlay = playFlag; };
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
    
    std::atomic<float>* playStateParameter = nullptr;
    
    juce::Value filePathParameter;
//    std::atomic<int>*
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sjf_manglerAudioProcessor)
};
