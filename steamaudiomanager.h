//---------------------OOP Interface for steam audio(not implemented fully yet)---------------------------
#pragma once

#include <SFML/Audio.hpp>
#include "phonon.h"
#include <vector>

class SteamAudioManager
{
public:
    SteamAudioManager();
    ~SteamAudioManager();

    void Initialize();
    void CleanUp();
    void DebugPrint() const;

    IPLSource CreateSource();
    std::vector<float> ProcessAudio(std::vector<float>& vectorBuffer, IPLVector3& dirVector);

private:
    IPLContext context;
    IPLContextSettings contextSettings;
    IPLAudioSettings audioSettings;
    IPLHRTF hrtf;
    IPLHRTFSettings hrtfSettings;
    IPLSimulator simulator;
    IPLBinauralEffect binauralEffect;
    IPLBinauralEffectSettings binauralEffectSettings;

    IPLAudioBuffer inBuffer;
    IPLAudioBuffer outBuffer;
};