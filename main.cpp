/* This project uses the following third-party libraries:
 *
 * 1. SFML - Simple and Fast Multimedia Library
 *    Copyright (c) Laurent Gomila
 *    Licensed under the zlib/libpng license
 *    https://www.sfml-dev.org/license.php
 *
 * 2. Steam Audio
 *    Copyright (c) Valve Corporation
 *    Licensed under the BSD 3-Clause "New" or "Revised" License
 *    https://github.com/ValveSoftware/steam-audio/blob/master/LICENSE.md
 *
 * This project itself is released under CC0 1.0 Universal (CC0 1.0) Public Domain Dedication.
 * To the extent possible under law, the author(s) have dedicated all copyright and related 
 * and neighboring rights to this software to the public domain worldwide.
 * This software is distributed without any warranty.
 * You should have received a copy of the CC0 Public Domain Dedication along with this software. 
 * If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.*/

#include <iostream>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include "phonon.h"

// Screen Size
const int sW {1920};
const int sH {1080};

// Other trivial constants
const float piVal {3.14159265358979323846};
const float movementSpeed {300.f};
const float radarSpeed {1500.f};

// Keyboard input method 
void InputMovement(sf::Vector2f& ballPos, float deltaTime) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) ballPos.y -= movementSpeed * deltaTime;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) ballPos.y += movementSpeed * deltaTime;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) ballPos.x += movementSpeed * deltaTime;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) ballPos.x -= movementSpeed * deltaTime;
}

// Focus shape modifier and color initialize
void UpdateFocusShape(sf::VertexArray& shape, sf::Vector2f center, float radius, float startAngle, float endAngle, sf::Color color)
{
    shape[0].position = center;
    shape[0].color = color;

    for(size_t i = 1; i < shape.getVertexCount(); ++i)
    {
        float angle = startAngle + (endAngle - startAngle) * (i - 1) / (shape.getVertexCount() - 2);
        shape[i].position = center + sf::Vector2f(std::cos(angle) * radius, std::sin(angle) * radius);
        shape[i].color = sf::Color::Black;
    }
}

// Radar circle action
void UpdateRadarShape(sf::CircleShape& radarCircle, float& radarRadius, float maxRadarRadius, float deltaTime, bool& isRadarExpanding)
{
    if (isRadarExpanding)
    {
        radarRadius += radarSpeed * deltaTime;
        if (radarRadius >= maxRadarRadius)
        {
            isRadarExpanding = false;
            radarRadius = 10.f;
        }
        radarCircle.setRadius(radarRadius);
        radarCircle.setOrigin(radarRadius, radarRadius);
    }
}

int main()
{
    // Sfml window initialization and frame limit
    sf::RenderWindow window(sf::VideoMode(sW, sH), "Audio Actor Test!");
    window.setFramerateLimit(144);

    // Sound buffer initialization and wave sound file load with sfml interface
    sf::SoundBuffer radarBuffer;
    if(!radarBuffer.loadFromFile("assets/audiofiles/radarSFX.wav"))
    {
        std::cerr << "Failed to load audio file." << std::endl;
        return 1;
    }

    std::vector<float> radarFloatBuffer;
    radarFloatBuffer.reserve(radarBuffer.getSampleCount() * radarBuffer.getChannelCount());
    for (size_t i = 0; i < radarBuffer.getSampleCount() * radarBuffer.getChannelCount(); ++i)
    {
        radarFloatBuffer.push_back(static_cast<float>(radarBuffer.getSamples()[i]) / 32767.f);
    }

    // ---------------------------------Steam Audio Implemantation Start(DRAFT)-----------------------------------------
    
    IPLContextSettings conSettings{};
    conSettings.version = STEAMAUDIO_VERSION;
    IPLContext context = nullptr;
    iplContextCreate(&conSettings, &context);

    IPLAudioSettings audioSettings{};
    audioSettings.samplingRate = 48000;
    audioSettings.frameSize = 1024;

    IPLHRTFSettings hrtfSettings{};
    hrtfSettings.type = IPL_HRTFTYPE_DEFAULT;
    hrtfSettings.volume = 1.0f;

    IPLHRTF hrtf = nullptr;
    iplHRTFCreate(context, &audioSettings, &hrtfSettings, &hrtf);

    IPLBinauralEffectSettings effectSettings{};
    effectSettings.hrtf = hrtf;

    IPLBinauralEffect effect = nullptr;
    iplBinauralEffectCreate(context, &audioSettings, &effectSettings, &effect);

    std::vector<float> inputBuffer(radarFloatBuffer.begin(), radarFloatBuffer.end());
    std::vector<float> outputBuffer(inputBuffer.size() * 2);

    IPLAudioBuffer inBuffer{};
    inBuffer.numChannels = 1;
    inBuffer.numSamples = static_cast<int>(inputBuffer.size());
    float* inData[] = { inputBuffer.data() };
    inBuffer.data = inData;

    IPLAudioBuffer outBuffer{};
    iplAudioBufferAllocate(context, 2, inBuffer.numSamples, &outBuffer);

    IPLBinauralEffectParams params{};
    params.direction = IPLVector3{1.0f, 1.0f, 1.0f}; 
    params.hrtf = hrtf;
    params.interpolation = IPL_HRTFINTERPOLATION_NEAREST; 
    params.spatialBlend = 1.0f; 
    params.peakDelays = nullptr;

    iplBinauralEffectApply(effect, &params, &inBuffer, &outBuffer);

    iplAudioBufferInterleave(context, &outBuffer, &outputBuffer[0]);

    sf::SoundBuffer processedBuffer;
    std::vector<sf::Int16> processedInt16(outputBuffer.size());
    for (size_t i = 0; i < outputBuffer.size(); ++i) {
        processedInt16[i] = static_cast<sf::Int16>(outputBuffer[i] * 32767.f);
    }
    processedBuffer.loadFromSamples(processedInt16.data(), processedInt16.size(), 2, 48000);
    sf::Sound processedSound(processedBuffer);

    // ---------------------------------Steam Audio Implemantation End(DRAFT)-----------------------------------------

    sf::Sound radarSound(radarBuffer);
    radarSound.setVolume(100);

    // Base clock and fps counter variables
    sf::Clock clock;
    float currentElapsedTime = 0.0f;
    int frameCount = 0;
    float fpsUpdateTime = 0.0f;
    float fpsUpdateInterval = 0.2f;
    float fpsVal = 0.f;

    sf::Vector2f ballPos = {sW/2, sH/2};

    // Sfml font load
    sf::Font font;
    if(!font.loadFromFile("assets/fonts/ARIAL.TTF"))
    {
        std::cerr << "Failed to load font file." << std::endl;
        return -1;
    }

    // Drawable shape colors definitions
    sf::Color circleColor(100, 100, 100);
    sf::Color focusColor(140, 10, 60);
    sf::Color radarColor(255, 255, 255, 30);

    // Informative text
    sf::Text timeText;
    timeText.setFont(font);
    timeText.setCharacterSize(20);
    timeText.setFillColor(sf::Color::White);
    timeText.setPosition(20, 20);

    sf::Text mousePosText;
    mousePosText.setFont(font);
    mousePosText.setCharacterSize(20);
    mousePosText.setFillColor(sf::Color::White);
    mousePosText.setPosition(20, 50);

    sf::Text radianText;
    radianText.setFont(font);
    radianText.setCharacterSize(20);
    radianText.setFillColor(sf::Color::White);
    radianText.setPosition(20, 80);

    sf::Text fpsText;
    fpsText.setFont(font);
    fpsText.setCharacterSize(20);
    fpsText.setFillColor(sf::Color::White); 
    fpsText.setPosition(1750, 20);

    // Main Actor shape
    sf::CircleShape circleShape(20.f);
    circleShape.setFillColor(circleColor);
    circleShape.setOrigin(20.f, 20.f);

    // Focus shape vertex array variable declarations
    float outerRadius = 200.f;
    float maxRadius = 1000.f;
    float minRadius = 60.f;
    float startAngle;
    float endAngle;
    int   segments = 100;

    sf::VertexArray focusShape(sf::PrimitiveType::TriangleFan, segments);

    // Radar shape
    float radarRadius = 10.f;
    float maxRadarRadius = 600.f;
    sf::CircleShape radarCircle(radarRadius);
    radarCircle.setFillColor(radarColor);
    radarCircle.setOrigin(radarRadius, radarRadius);
    bool isRadarExpanding = false;

    sf::Mouse mouse;

    // ----------------- MAIN GAME LOOP ----------------------
    while(window.isOpen())
    {
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
            // Keyboard click event check and calls
            if(event.type == sf::Event::KeyPressed)
            {
                if(event.key.code == sf::Keyboard::F)
                {
                    processedSound.play();
                    //radarSound.play();
                    std::cout << "Radar Pulse played." << std::endl;
                    isRadarExpanding = true;
                    radarRadius = 10.f;
                }
            }
        }

        // Delta time and frame per second calculation
        float deltaTime = clock.restart().asSeconds();
        frameCount++;
        currentElapsedTime += deltaTime;

        if (currentElapsedTime - fpsUpdateTime >= fpsUpdateInterval)
        {
            fpsVal = frameCount / (currentElapsedTime - fpsUpdateTime);
            fpsUpdateTime = currentElapsedTime;
            frameCount = 0;
        }

        // Mouse position and angle calculation with main actor
        sf::Vector2i mousePos = mouse.getPosition(window);

        float mouseBallDistance = std::sqrt(std::pow(mousePos.x - ballPos.x, 2) + std::pow(mousePos.y - ballPos.y, 2));        
        float maxDistance = 800.f;

        float focusRadian = std::atan2f(mousePos.y - ballPos.y, mousePos.x - ballPos.x);

        float normalizedDistance = std::min(mouseBallDistance, maxDistance) / maxDistance;

        float invertedNormalizedDistance = 1.0f - normalizedDistance;

        // Focus shape sector width 
        outerRadius = minRadius + normalizedDistance * (maxRadius - minRadius);

        float minSectorWidth = 5.0f * (piVal / 180.0f);
        float maxSectorWidth = 240.0f * (piVal / 180.0f); 
        float sectorWidth = minSectorWidth + invertedNormalizedDistance * (maxSectorWidth - minSectorWidth);

        startAngle = focusRadian - (sectorWidth * 0.5f);
        endAngle = focusRadian + (sectorWidth * 0.5f);

        // Defined method calls for every iteration
        InputMovement(ballPos, deltaTime);
        UpdateFocusShape(focusShape, (sf::Vector2f){ballPos}, outerRadius, startAngle, endAngle, focusColor);
        UpdateRadarShape(radarCircle, radarRadius, maxRadarRadius, deltaTime, isRadarExpanding);

        window.clear(sf::Color::Black);

        // Screen text insert
        timeText.setString("Elapsed Time: " + std::to_string(currentElapsedTime));
        mousePosText.setString("Mouse Position: x = " + std::to_string(mousePos.x) + " y = " + std::to_string(mousePos.y));
        radianText.setString("Mouse Radian: " + std::to_string(focusRadian));
        fpsText.setString("FPS: " + std::to_string(fpsVal));

        // Main actor position change
        circleShape.setPosition(ballPos);
        radarCircle.setPosition(ballPos);

        // Sfml draw calls
        window.draw(focusShape);
        window.draw(radarCircle);
        window.draw(circleShape);
        window.draw(timeText);
        window.draw(mousePosText);
        window.draw(radianText);
        window.draw(fpsText);

        window.display();
    }

    iplAudioBufferFree(context, &outBuffer);
    iplBinauralEffectRelease(&effect);
    iplHRTFRelease(&hrtf);
    iplContextRelease(&context);

    return 0;
}