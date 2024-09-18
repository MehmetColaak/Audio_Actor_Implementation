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

    bool Initialize();
    void CleanUp();
    void DebugPrint() const;

    IPLSource CreateSource();

private:
    IPLContext context;
    IPLHRTF hrtf;
    IPLSimulator simulator;
    IPLBinauralEffect binauralEffect;
};