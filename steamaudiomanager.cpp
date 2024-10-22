//---------------------OOP Interface for steam audio(not implemented fully yet)---------------------------
#include "steamaudiomanager.h"
#include <iostream>

SteamAudioManager::SteamAudioManager() : 
    context(nullptr), 
    contextSettings({}), 
    hrtf(nullptr), 
    binauralEffect(nullptr),
    binauralEffectSettings({}), 
    audioSettings({}), 
    hrtfSettings({}),
    inBuffer({}),
    outBuffer({})
{
    std::cout << "SteamAudioManager constructor called" << std::endl;
}

SteamAudioManager::~SteamAudioManager()
{
    std::cout << "SteamAudioManager destructor called" << std::endl;
    CleanUp();
}

void SteamAudioManager::Initialize()
{
    contextSettings.version = STEAMAUDIO_VERSION;
    iplContextCreate(&contextSettings, &context);

    audioSettings.samplingRate = 44100;
    audioSettings.frameSize = 1024;

    hrtfSettings.type = IPL_HRTFTYPE_DEFAULT;
    hrtfSettings.volume = 1.0f;

    iplHRTFCreate(context, &audioSettings, &hrtfSettings, &hrtf);

    binauralEffectSettings.hrtf = hrtf;

    iplBinauralEffectCreate(context, &audioSettings, &binauralEffectSettings, &binauralEffect);
}

void SteamAudioManager::CleanUp()
{
    std::cout << "Cleaning up Steam Audio..." << std::endl;

    if(simulator)
    {
        iplSimulatorRelease(&simulator);
        std::cout << "Simulator released" << std::endl;

    }
    if (binauralEffect)
    {
        iplBinauralEffectRelease(&binauralEffect);
        std::cout << "Binaural effect released" << std::endl;
    }
    if (hrtf)
    {
        iplHRTFRelease(&hrtf);
        std::cout << "HRTF released" << std::endl;
    }
    if(context)
    {
        iplAudioBufferFree(context, &outBuffer);
        iplContextRelease(&context);
        context = nullptr;
        std::cout << "Steam Audio context destroyed" << std::endl;
    } 
    else
    {
        std::cout << "No Steam Audio context to clean up" << std::endl;
    }
}

void SteamAudioManager::DebugPrint() const
{
    std::cout << "SteamAudioManager Debug Information:" << std::endl;
    std::cout << "Context: " << (context ? "Initialized" : "Not initialized") << std::endl;
    std::cout << "Steam Audio Version: " 
              << STEAMAUDIO_VERSION_MAJOR << "."
              << STEAMAUDIO_VERSION_MINOR << "."
              << STEAMAUDIO_VERSION_PATCH << std::endl;
}

IPLSource SteamAudioManager::CreateSource()
{
    IPLSource source = nullptr;
    IPLSourceSettings sourceSettings = {};
    sourceSettings.flags = IPL_SIMULATIONFLAGS_DIRECT;

    IPLerror error = iplSourceCreate(simulator, &sourceSettings, &source);
    if(error != IPL_STATUS_SUCCESS)
    {
        std::cerr << "Failed to create Steam Audio Source." << std::endl;
        return nullptr;
    }
    return source;
}

std::vector<float> SteamAudioManager::ProcessAudio(std::vector<float>& vectorBuffer, IPLVector3& dirVector)
{
    // Prepare audio buffers
    std::vector<float> inputBuffer(vectorBuffer.begin(), vectorBuffer.end());
    std::vector<float> outputBuffer(inputBuffer.size() * 2);  // Stereo output

    inBuffer.numChannels = 1;
    inBuffer.numSamples = audioSettings.frameSize;
    float* inData[] = { inputBuffer.data() };
    inBuffer.data = inData;

    iplAudioBufferAllocate(context, 2, audioSettings.frameSize, &outBuffer);

    size_t numFrames = inputBuffer.size() / audioSettings.frameSize;
    for (size_t frame = 0; frame < numFrames; ++frame)
    {
        IPLBinauralEffectParams params{};
        params.direction = dirVector; 
        params.hrtf = hrtf;
        params.interpolation = IPL_HRTFINTERPOLATION_NEAREST;
        params.spatialBlend = 1.0f;

        iplBinauralEffectApply(binauralEffect, &params, &inBuffer, &outBuffer);

        iplAudioBufferInterleave(context, &outBuffer, outputBuffer.data() + frame * audioSettings.frameSize * 2);

        inData[0] += audioSettings.frameSize;
    }
    return outputBuffer;
}
