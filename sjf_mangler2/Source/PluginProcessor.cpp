/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

#define sampleInfoFile "~/Library/Audio/Plug-Ins/sjf/mangler2_fileInfo.xml"

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
    nVoicesParameter = parameters.state.getPropertyAsValue( "nVoices", nullptr, true );
    if ( (int)nVoicesParameter.getValue() < 1 )
    {
        nVoicesParameter.setValue( 1 );
    }
    DBG("Starting with " << (int)nVoicesParameter.getValue() << " Voices");
    //    *nVoicesParameter = m_nVoices;
    DBG("setting num voices in mangler");
    setNumVoices( nVoicesParameter.getValue() );
    
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
    
    phaseRateMultiplierParameter = parameters.getRawParameterValue("phaseRateMultiplier");
    
    interpolationTypeParameter = parameters.getRawParameterValue("interpolationType");
    
    

    
    for ( int v = 0; v < (int)nVoicesParameter.getValue(); v++ )
    {
        filePathParameter[ v ] = parameters.state.getPropertyAsValue("sampleFilePath" + juce::String( v ), nullptr, true);
        nSlicesParameter[ v ] = parameters.state.getPropertyAsValue( "numSlices"+juce::String( v ), nullptr, true );
//        phaseRateMultiplierParameter[ v ] = parameters.state.getPropertyAsValue( "phaseRate"+juce::String( v ), nullptr, true );
        sampleChoiceProbabilitiesParameter[ v ] = parameters.state.getPropertyAsValue("sampleChoiceProb"+juce::String( v ), nullptr, true );
    }
//    nSlicesParameter = parameters.state.getPropertyAsValue("numSlices", nullptr, true );
    
    sampleMangler2.initialise( getSampleRate() ); 
    sampleMangler2.setNumSteps( *nStepsParameter );
    ////////////////////////////////////////
    ////////////////////////////////////////
    ////////////////////////////////////////
    DBG("Ended construction");
    if( !juce::File( "~/Library/Audio/Plug-Ins/sjf" ).exists() )
    {
        juce::File( "~/Library/Audio/Plug-Ins/sjf" ).createDirectory();
    }
    if ( !juce::File( sampleInfoFile ).existsAsFile() )
    {
        juce::File( sampleInfoFile ).create();
        auto storage = juce::File( sampleInfoFile );
        juce::XmlElement sampleList ("SAMPLES");
        sampleList.writeTo( storage );
    }
//    auto storage = juce::File( "~/Library/Audio/Plug-Ins/sjf" ).getChildFile("mangler2_fileInfo.txt");
//    storage.create();
//    juce::File storage( juce::String(juce::File::getSpecialLocation( juce::File::SpecialLocationType::currentApplicationFile )));
//    DBG("FILENAME " << storage.getFileName());
//    // create an outer node called "ANIMALS"
//    juce::XmlElement animalsList ("ANIMALS");
//
//    // create an inner element..
//    juce::XmlElement* giraffe = new juce::XmlElement ("GIRAFFE");
//
//    giraffe->setAttribute ("name", "nigel");
//    giraffe->setAttribute ("age", 10);
//    giraffe->setAttribute ("friendly", true);
//
//    // ..and add our new element to the parent node
//    animalsList.addChildElement (giraffe);
//    animalsList.writeTo( storage );
    ////////////////////////////////////////
    ////////////////////////////////////////
    ////////////////////////////////////////
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
    DBG( "nVoicesParameter " << (int)nVoicesParameter.getValue() );
    setNumVoices( (int)nVoicesParameter.getValue() );
    DBG( "nVoicesParameter " << (int)nVoicesParameter.getValue() );
    for ( int v = 0; v < (int)nVoicesParameter.getValue(); v++ )
    {
        filePathParameter[ v ].setValue( sampleMangler2.getFilePath( v ) );
        nSlicesParameter[ v ].setValue( sampleMangler2.getNumSlices( v ) );
//        phaseRateMultiplierParameter[ v ].setValue( sampleMangler2.getPhaseRateMultiplierIndex( v ) );
        sampleChoiceProbabilitiesParameter[ v ].setValue( sampleMangler2.getSampleChoiceProbability( v ) );
    }
    
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
    
    
    writeSampleInfoToXML();
}
//==============================================================================
void Sjf_Mangler2AudioProcessor::writeSampleInfoToXML()
{
    auto storage = juce::File( sampleInfoFile );
    juce::XmlDocument mainDoc ( storage );
    if ( auto mainElement = mainDoc.getDocumentElement() )
    {
        if( mainElement->getTagName() == "SAMPLES" )
        {
            DBG( "MAIN ELEMENT: "<< mainElement->getNumChildElements() << " CHILDREN" );
            for( int i = 0; i < mainElement->getNumChildElements(); i++ )
            {
                DBG("MAIN ELEMENT: CHILD " << i << " " << mainElement->getChildElement( i )->getTagName() );
            }
            
            DBG("SAMPLES EXISTS --> DO SOMETHING");
            for ( int v = 0; v < (int)nVoicesParameter.getValue(); v++ )
            {
                if( sampleMangler2.getFilePath( v ).isNotEmpty() )
                {
                    auto samplesElement = mainElement->getChildByAttribute( "path", sampleMangler2.getFilePath( v ) );
                    if ( samplesElement != nullptr )
                    {
                        //                        auto samplesElement = mainElement->getChildByName( sampleMangler2.getFilePath( v ) );
                        DBG( "ELEMENT EXISTS");
                        samplesElement->setAttribute ("nSlices", sampleMangler2.getNumSlices( v ) );
                    }
                    else
                    {
                        DBG( "ELEMENT DOESN'T EXIST" );
                        juce::XmlElement* sample = new juce::XmlElement ( "sample"+juce::String( mainElement->getNumChildElements() ) );
                        sample->setAttribute ("path", sampleMangler2.getFilePath( v ) );
                        sample->setAttribute ("nSlices", sampleMangler2.getNumSlices( v ) );
                        // ..and add our new element to the parent node
                        mainElement->addChildElement (sample);
                    }
                }
            }
            mainElement->writeTo( storage );
        }
        else
        {
            DBG("SAMPLES DOESNT EXIST");
        }
    }
    else
    {
        DBG("CREATING SAMPLES");
        //        juce::XmlElement sampleList ("SAMPLES");
        //        // create an inner element..
        //        for ( int v = 0; v < *nVoicesParameter; v++ )
        //        {
        //            if( sampleMangler2.getFilePath( v ) != juce::String{} )
        //            {
        //                juce::XmlElement* sample = new juce::XmlElement ( sampleMangler2.getFilePath( v ) );
        //                sample->setAttribute ("nSlices", sampleMangler2.getNumSlices( v ) );
        //                // ..and add our new element to the parent node
        //                sampleList.addChildElement (sample);
        //            }
        //        }
        //        sampleList.writeTo( storage );
    }
}
//==============================================================================
void Sjf_Mangler2AudioProcessor::loadFolderOfSamples ()
{
    
//    juce::FileChooser::browseForDirectory();
    m_chooser = std::make_unique<juce::FileChooser> ("Select a folder of Wave/Aiff files to load..." ,
                                                     juce::File{}, "*.aif, *.wav");
//    m_chooser->browseForDirectory();
    auto chooserFlags = juce::FileBrowserComponent::openMode
    | juce::FileBrowserComponent::canSelectDirectories
    | juce::FileBrowserComponent::canSelectFiles
    | juce::FileBrowserComponent::canSelectMultipleItems;

    
    bool wasPlaying = *playStateParameter;
    
    m_chooser->launchAsync (chooserFlags, [ this, wasPlaying ] (const juce::FileChooser& fc)
                            {
                                sampleMangler2.m_canPlayFlag = *playStateParameter = false;
                                auto results = fc.getResults();
                                if ( results.size() == 0 )
                                {
                                    *playStateParameter = wasPlaying;
                                    return;
                                }
//                                if ( results == juce::File{} ) { return; }
                                if ( results.getFirst().isDirectory() )
                                {
                                    auto directory = results.getFirst(); 
                                    auto nFiles = directory.getNumberOfChildFiles( juce::File::findFiles, "*.aif, *.wav" );
                                    if ( nFiles <= 0 )
                                    {
                                        *playStateParameter = wasPlaying;
                                        return;
                                    }
                                    setNumVoices( nFiles );
                                    DBG( "Directory Name " << directory.getFileName() << " " <<  nFiles << " audio files" );
                                    auto samples = directory.findChildFiles( juce::File::findFiles, true, "*.aif, *.wav", juce::File::FollowSymlinks::no );
                                    for ( int s = 0; s < nFiles; s++ )
                                    {
                                        DBG( samples[ s ].getFileName() );
                                        juce::Value path;
                                        path.setValue( samples[ s ].getFullPathName() );
                                        sampleMangler2.loadSample( path, s );
                                    }
                                }
                                else
                                {
                                    auto nFiles = results.size();
                                    setNumVoices( nFiles );
                                    for ( int s = 0; s < nFiles; s++ )
                                    {
                                        DBG( results[ s ].getFileName() );
                                        juce::Value path;
                                        path.setValue( results[ s ].getFullPathName() );
                                        sampleMangler2.loadSample( path, s );
                                    }
                                }
                                for ( int v = 0; v < (int)nVoicesParameter.getValue(); v++ )
                                {
                                    readSampleInfoFromXML( v );
                                }
                                *playStateParameter = wasPlaying;
                            });
    
}
//==============================================================================
void Sjf_Mangler2AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
            nVoicesParameter.referTo( parameters.state.getPropertyAsValue( "nVoices", nullptr ) );
            setNumVoices( (int)nVoicesParameter.getValue() );
            for ( int v = 0; v < (int)nVoicesParameter.getValue(); v++ )
            {
                filePathParameter[ v ].referTo( parameters.state.getPropertyAsValue("sampleFilePath"+juce::String( v ), nullptr ) );
                if (filePathParameter[ v ] != juce::Value{})
                {
                    if ( sampleMangler2.loadSample( filePathParameter[ v ], v ) )
                    {
                        readSampleInfoFromXML( v );
                    }
//                    sampleMangler2.loadSample( filePathParameter[ v ], v );
                }
                nSlicesParameter[ v ].referTo( parameters.state.getPropertyAsValue( "numSlices"+juce::String( v ), nullptr ) );
//                int slices = nSlicesParameter[ v ].getValue();
                sampleMangler2.setNumSlices( nSlicesParameter[ v ].getValue(), v );
//                phaseRateMultiplierParameter[ v ].referTo( parameters.state.getPropertyAsValue( "phaseRate"+juce::String( v ), nullptr ) );
//                int phaseRate = phaseRateMultiplierParameter[ v ].getValue();
//                sampleMangler2.setPhaseRateMultiplierIndex( phaseRateMultiplierParameter[ v ].getValue() , v );
                sampleChoiceProbabilitiesParameter[ v ].referTo( parameters.state.getPropertyAsValue( "sampleChoiceProb"+juce::String( v ), nullptr ) );
                sampleMangler2.setSampleChoiceProbabilities( sampleChoiceProbabilitiesParameter[ v ].getValue(), v );
            }
        }
}
//==============================================================================
void Sjf_Mangler2AudioProcessor::readSampleInfoFromXML( const int voiceNumber )
{
    auto storage = juce::File( sampleInfoFile );
    juce::XmlDocument mainDoc ( storage );
    if ( auto mainElement = mainDoc.getDocumentElement() )
    {
        if( mainElement->getTagName() == "SAMPLES" )
        {
            DBG( "MAIN ELEMENT: "<< mainElement->getNumChildElements() << " CHILDREN" );
            for( int i = 0; i < mainElement->getNumChildElements(); i++ )
            {
                DBG("MAIN ELEMENT: CHILD " << i << " " << mainElement->getChildElement( i )->getTagName() );
            }
            
            DBG("SAMPLES EXISTS --> DO SOMETHING");
            if( sampleMangler2.getFilePath( voiceNumber ).isNotEmpty() )
            {
                auto samplesElement = mainElement->getChildByAttribute( "path", sampleMangler2.getFilePath( voiceNumber ) );
                if ( samplesElement != nullptr )
                {
                    //                        auto samplesElement = mainElement->getChildByName( sampleMangler2.getFilePath( v ) );
                    DBG( "ELEMENT EXISTS");
                    sampleMangler2.setNumSlices( samplesElement->getIntAttribute ("nSlices"), voiceNumber );
                }
                else { DBG( "ELEMENT DOESN'T EXIST" ); }
            }
        }
    }
}
//==============================================================================
void Sjf_Mangler2AudioProcessor::loadButtonClicked ( const int voiceNumber )
{
    if ( sampleMangler2.loadSample( voiceNumber ) ) 
    {
        readSampleInfoFromXML( voiceNumber );
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
    if (sampleMangler2.getPhaseRateMultiplierIndex() != *phaseRateMultiplierParameter){
        sampleMangler2.setPhaseRateMultiplierIndex(*phaseRateMultiplierParameter);
    }
    if (sampleMangler2.m_canPlayFlag != *playStateParameter){
        sampleMangler2.m_canPlayFlag = *playStateParameter;
    }
    if (sampleMangler2.m_interpolationType != *interpolationTypeParameter){
        sampleMangler2.m_interpolationType = *interpolationTypeParameter;
    }
}


void Sjf_Mangler2AudioProcessor::setNumVoices( const int nVoices )
{
//    *nVoicesParameter = nVoices;
    nVoicesParameter.setValue( nVoices );
//    nVoicesParameter->setValue( nVoices );
    sampleMangler2.setNumVoices( nVoices );
    filePathParameter.resize( nVoices );
    nSlicesParameter.resize( nVoices );
//    phaseRateMultiplierParameter.resize( nVoices );
    sampleChoiceProbabilitiesParameter.resize( nVoices );
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
    params.add( std::make_unique<juce::AudioParameterInt> ( juce::ParameterID{ "phaseRateMultiplier", pIDVersionNumber }, "PhaseRateMultiplier", 1, 5, 3) );
    params.add( std::make_unique<juce::AudioParameterFloat> ( juce::ParameterID{ "fade", pIDVersionNumber }, "Fade", 0.01, 100, 1) );
    params.add( std::make_unique<juce::AudioParameterInt> ( juce::ParameterID{ "interpolationType", pIDVersionNumber }, "InterpolationType", 1, 6, 6) );
    
//    params.add( std::make_unique<juce::AudioParameterInt> ( juce::ParameterID{ "nVoices", pIDVersionNumber }, "NVoices", 1, 32, 2) );
    
    return params;
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Sjf_Mangler2AudioProcessor();
}




