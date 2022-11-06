/*
  ==============================================================================
                            Guilherme Barros 2022
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimplePhaserAudioProcessor::SimplePhaserAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
treeState(*this, nullptr, "Parameters", createParameterLayout())

#endif
{
    treeState.addParameterListener(rateID, this);
    treeState.addParameterListener(depthID, this);
    treeState.addParameterListener(centreFrequencyID, this);
    treeState.addParameterListener(feedbackID, this);
    treeState.addParameterListener(mixID, this);
}

SimplePhaserAudioProcessor::~SimplePhaserAudioProcessor()
{
    treeState.removeParameterListener(rateID, this);
    treeState.removeParameterListener(depthID, this);
    treeState.removeParameterListener(centreFrequencyID, this);
    treeState.removeParameterListener(feedbackID, this);
    treeState.removeParameterListener(mixID, this);
}

//==============================================================================

juce::AudioProcessorValueTreeState::ParameterLayout SimplePhaserAudioProcessor::createParameterLayout()
{
    // parameters vector (params)
    
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
        
    // chorus parameters
        
    auto pRate = std::make_unique<juce::AudioParameterFloat>(rateID, rateName, 0.1f, 100.0f, 1.0f);
    auto pDepth = std::make_unique<juce::AudioParameterFloat>(depthID, depthName, 0.0f, 1.0f, 0.0f);
    auto pCentreFrequency = std::make_unique<juce::AudioParameterFloat>(centreFrequencyID, centreFrequencyName, 1.0f, 1000.0f, 1.0f);
    auto pFeedback = std::make_unique<juce::AudioParameterFloat>(feedbackID, feedbackName, -1.0f, 1.0f, 1.0f);
    auto pMix = std::make_unique<juce::AudioParameterFloat>(mixID, mixName, 0.0f, 1.0f, 0.5f);

    params.push_back(std::move(pRate));
    params.push_back(std::move(pDepth));
    params.push_back(std::move(pCentreFrequency));
    params.push_back(std::move(pFeedback));
    params.push_back(std::move(pMix));

    return { params.begin(), params.end() };
}


void SimplePhaserAudioProcessor::updateParameters()
{
    phaserModule.setRate(treeState.getRawParameterValue(rateID)->load());
    phaserModule.setDepth(treeState.getRawParameterValue(depthID)->load());
    phaserModule.setCentreFrequency(treeState.getRawParameterValue(centreFrequencyID)->load());
    phaserModule.setFeedback(treeState.getRawParameterValue(feedbackID)->load());
    phaserModule.setMix(treeState.getRawParameterValue(mixID)->load());
}

void SimplePhaserAudioProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    updateParameters();
}

//==============================================================================


const juce::String SimplePhaserAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimplePhaserAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimplePhaserAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimplePhaserAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimplePhaserAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimplePhaserAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimplePhaserAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimplePhaserAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimplePhaserAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimplePhaserAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimplePhaserAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();
    
    phaserModule.prepare(spec);
    
    updateParameters();
}

void SimplePhaserAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimplePhaserAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SimplePhaserAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    juce::dsp::AudioBlock<float> block { buffer };
    phaserModule.process(juce::dsp::ProcessContextReplacing<float>(block));
    
}

//==============================================================================
bool SimplePhaserAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimplePhaserAudioProcessor::createEditor()
{
    //return new SimplePhaserAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void SimplePhaserAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, false);
    treeState.state.writeToStream(stream);
}

void SimplePhaserAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree  = juce::ValueTree::readFromData(data, size_t(sizeInBytes));
    if (tree.isValid())
    {
        treeState.state = tree;
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimplePhaserAudioProcessor();
}
