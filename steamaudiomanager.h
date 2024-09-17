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

    //IPLContext GetContext() const { return context; }

    //IPLSource CreateSource();
    //IPLAudioBuffer CreateAudioBuffer(const sf::Sound& sfmlBuffer);
    //void PlaySound(IPLSource source, const sf::Vector3f& position);

private:
    IPLContext context;
    IPLHRTF hrtf;
    IPLBinauralEffect binauralEffect;
    //std::vector<IPLSource> sources;
};