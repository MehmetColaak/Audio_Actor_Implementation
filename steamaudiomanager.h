#pragma once

#include <SFML/Audio.hpp>
#include "phonon.h"

class SteamAudioManager
{
public:
    SteamAudioManager();
    ~SteamAudioManager();

    bool Initialize();
    void CleanUp();
    void DebugPrint() const;

private:
    IPLContext context;
    IPLHRTF hrtf;
    IPLBinauralEffect binauralEffect;
};