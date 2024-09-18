//---------------------OOP Interface for steam audio(not implemented fully yet)---------------------------
#include "steamaudiomanager.h"
#include <iostream>

SteamAudioManager::SteamAudioManager() : context(nullptr), hrtf(nullptr), binauralEffect(nullptr)
{
    std::cout << "SteamAudioManager constructor called" << std::endl;
}

SteamAudioManager::~SteamAudioManager()
{
    std::cout << "SteamAudioManager destructor called" << std::endl;
    CleanUp();
}

bool SteamAudioManager::Initialize()
{
    std::cout << "Initializing Steam Audio..." << std::endl;
    
    // Initialize context
    IPLContextSettings contextSettings{};
    contextSettings.version = STEAMAUDIO_VERSION;
    IPLerror error = iplContextCreate(&contextSettings, &context);
    if(error != IPL_STATUS_SUCCESS)
    {
        std::cerr << "Failed to create Steam Audio Context." << std::endl;
        return false;
    }
    std::cout << "Steam Audio context initialized successfully" << std::endl;

    // Set up audio settings
    IPLAudioSettings audioSettings{};
    audioSettings.samplingRate = 44100;
    audioSettings.frameSize = 1024;

    // Initialize HRTF
    std::cout << "Initializing HRTF..." << std::endl;
    IPLHRTFSettings hrtfSettings{};
    hrtfSettings.type = IPL_HRTFTYPE_DEFAULT;

    error = iplHRTFCreate(context, &audioSettings, &hrtfSettings, &hrtf);
    if (error != IPL_STATUS_SUCCESS)
    {
        std::cerr << "Failed to create HRTF." << std::endl;
        iplContextRelease(&context);
        return false;
    }
    std::cout << "HRTF initialized successfully" << std::endl;

    // Initialize binaural effect
    std::cout << "Initializing binaural effect..." << std::endl;
    IPLBinauralEffectSettings effectSettings{};
    effectSettings.hrtf = hrtf;

    error = iplBinauralEffectCreate(context, &audioSettings, &effectSettings, &binauralEffect);
    if (error != IPL_STATUS_SUCCESS)
    {
        std::cerr << "Failed to create binaural effect." << std::endl;
        iplHRTFRelease(&hrtf);
        iplContextRelease(&context);
        return false;
    }
    std::cout << "Binaural effect initialized successfully" << std::endl;

    return true;
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
